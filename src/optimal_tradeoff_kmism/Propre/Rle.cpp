/* Copyright : GNU GPL V3
Author : Tatiana Rocher, tatiana.rocher@gmail.com

*/

#include <iostream>
#include <vector>

#include "Rle.hpp"
#include "Tools.hpp"
#include "Fft_wak.hpp"
#include "HammingDistance.hpp"

using namespace std;


RunRle::RunRle(char l) {
	position_ = -1;
	size_ = 1;
	letter_ = l;
}

RunRle::RunRle(RunRle *run) {
	position_ = run->GetPosition();
	size_ = run->GetSize();
	letter_ = run->GetChar();
}

void RunRle::DeleteRunBegin(int period) {
	position_ += period;
	size_--;
}

void RunRle::ExtendRunBegin(int pos) {
	position_ = pos;
	size_ ++;
}

void RunRle::ExtendRunEnd() {
	size_++;
}

bool RunRle::ConcatRuns(RunRle *r) {
	if (letter_ != r->GetChar())
		return false;
	size_ += r->GetSize();
}



/******** Getter***********/

int RunRle::GetPosition() const {
	return position_;
}

void RunRle::SetPosition(int32_t p) {
	position_ = p;
}


char RunRle::GetChar() const {
	return letter_;
}

int RunRle::GetSize() const {
	return size_;
}



/************ Rle Class ************/

Rle::Rle() {
	size_ = 0;
}

Rle::Rle(int p) {
	period_ = p;
	size_ = 0;
	list<RunRle*> *rle = new list<RunRle*>[p]();
	for (int i = 0; i < period_; ++i) {
		list<RunRle*> *list_run = new list<RunRle*>();
		rle[i] = (*list_run);
	}
	rle_ = rle;
}

Rle::~Rle() {
	for (int i = 0; i < period_; ++i)
		rle_[i].clear();
	delete [] rle_;
}

int RleText::AddLetterBegin(int pos, int period, char letter) {
	size_ ++;
	if (rle_[period].size() == 0) {
		RunRle *run = new RunRle(letter);
		rle_[period].push_front(run);
		return 1;
	}
	else {
		char c = rle_[period].front()->GetChar();
		if (c == letter) {
			rle_[period].front()->ExtendRunBegin(pos);
			return 0;
		}
		else {
			RunRle *run = new RunRle(letter);
			rle_[period].push_front(run);
			return 1;
		}
	}
}

int Rle::AddLetterEnd(int pos, int period, char letter) {
	size_ ++;
	if (rle_[period].size() == 0) {
		RunRle *run = new RunRle(letter);
		rle_[period].push_back(run);
		return 1;
	}
	else {
		char c = rle_[period].back()->GetChar();
		if (c == letter) {
			rle_[period].back()->ExtendRunEnd();
			return 0;
		}
		else {
			RunRle *run = new RunRle(letter);
			rle_[period].push_back(run);
			return 1;
		}
	}
}

int RleText::AddLetterEndTSec(int pos, int period, char letter) {
	if (t_sec_[period].size() == 0) {
		RunRle *run = new RunRle(letter);
		t_sec_[period].push_back(run);
		return 1;
	}
	else {
		char c = rle_[period].back()->GetChar();
		if (c == letter) {
			t_sec_[period].back()->ExtendRunEnd();
			return 0;
		}
		else {
			RunRle *run = new RunRle(letter);
			t_sec_[period].push_back(run);
			return 1;
		}
	}
}

void Rle::SetPositionRuns() {
	int32_t pos = 0;
	for (int i = 0; i < period_; ++i) {
		for (std::list<RunRle*>::iterator it = rle_[i].begin(); it != rle_[i].end(); ++it) {
			(*it)->SetPosition(pos);
			pos += (*it)->GetSize();
		}
	}
}


void Rle::MakeRle(int32_t size_pattern, char *pattern, int32_t size_total) {
	int p, i = 0;
	while (i<size_pattern) {
		p = i % period_;
		AddLetterEnd(i, p, pattern[i]);
		i++;
	}
	while(i< size_total) {
		p = i % period_;
		AddLetterEnd(i, p, '$');
		i++;
	}
	SetPositionRuns();
}


/************** Rle for the text *******************/


RleText::RleText(int p) : Rle(p) {
	list<RunRle*> *t_sec = new list<RunRle*>[p]();
	for (int i = 0; i < period_; ++i) {
		list<RunRle*> *liste = new list<RunRle*>();
		t_sec[i] = (*liste);
	}
	t_sec_ = t_sec;
}

RleText::~RleText() {
	for (int i = 0; i < period_; ++i) {
		rle_[i].clear();
		t_sec_[i].clear();
	}
	delete [] rle_;
	delete [] t_sec_;
}

void RleText::ReorganiseListRun(int32_t step) {
	if ((step == 0) || (period_ == 1))
		return;

	list<RunRle*> tmp_list = rle_[0];
	int pos = 0;

	if (step%2 == 0) {
		list<RunRle*> tmp_list1 = rle_[1];
		for (int i = 0; i < period_/2-1; ++i) {
			rle_[pos] = rle_[(pos+step)%period_];
			rle_[(pos+1)%period_] = rle_[(pos+step+1)%period_];
			pos += step;
			pos = pos%period_;
		}
		rle_[pos+1] = tmp_list1;
	}
	else {

		for (int i= 0; i < period_-1; ++i) {
			rle_[pos] = rle_[(pos+step)%period_];
			pos += step;
			pos = pos%period_;
		}
	}
		rle_[pos] = tmp_list;
}

void RleText::ConcatTSec() {
	for (int i = 0; i < period_; ++i) {
		RunRle *run = new RunRle(*(rle_[i].begin()));
		if (run->GetSize() > 1) {
			run->DeleteRunBegin(period_);
			t_sec_[i].push_back(run);
		}
    	for (std::list<RunRle*>::iterator it = next(rle_[i].begin()); it != rle_[i].end(); ++it) {
    		RunRle *r = new RunRle(*it);
    		t_sec_[i].push_back(r);
    	}
    }
    // add the last letter '#' for T"
    for (int i = 0; i < period_; ++i) {
    	AddLetterEndTSec(i, i, '#');
    }
    size_ *= 2;
}

void RleText::SetPositionRuns() {
	int32_t pos = 0;
	for (int i = 0; i < period_; ++i) {
		for (std::list<RunRle*>::iterator it = rle_[i].begin(); it != rle_[i].end(); ++it) {
			(*it)->SetPosition(pos);
			pos += (*it)->GetSize();
		}
	}
	for (int i = 0; i < period_; ++i) {
		for (std::list<RunRle*>::iterator it = t_sec_[i].begin(); it != t_sec_[i].end(); ++it) {
			(*it)->SetPosition(pos);
			pos += (*it)->GetSize();
		}
	}
}

void RleText::MakeRle(int32_t size_pattern, int32_t size_text, char *text,
					int nb_errors, int32_t *i_l, int32_t *i_r, int32_t *m1) {
	int32_t p, i = size_pattern-1;
	int nb_runs = 0;

	//  extend the text from m to the left : gives us i_l
	while (i >= 0 && nb_runs <= nb_errors * 11) {
		p = i % period_;
		nb_runs += AddLetterBegin(i, p, text[i]);
		i--;
	}
	(*i_l) = i + 1;
	i = size_pattern;

	// extend the text from m to the right : gives us i_r
	bool continu = true;
	while ( i < size_text && continu) {
		p = i % period_;
		nb_runs += AddLetterEnd(i, p, text[i]);
		i++;
		if (nb_runs >= nb_errors * 22 && i_r - i_l + 1 % period_ == 0)
			continu = false;
	}
	(*i_r) = i - 1;
	//pad with # until having a lenght divisible by period
	while ((i - (*i_l)) % period_ != 0) {
		p = i % period_;
		AddLetterEnd(i, p, '#');
		i++;
	}
	(*m1) = size_ / period_;

	//put the first run at the first position of the table
	ReorganiseListRun(*i_l);
	// concatenate T"
	ConcatTSec();
	//  puts the position where the run starts in T*
	SetPositionRuns();
}



/********** Getter / Starter *********/


int Rle::GetPeriod() const {
	return period_;
}


list<RunRle*>* Rle::GetList(int period) const {
	return &rle_[period];
}


list<RunRle*>* RleText::GetListTSec(int period) const {
	return &t_sec_[period];
}


int32_t Rle::GetSize() const {
	return size_;
}


int Rle::GetSizeList(int pos_period) const {
	return rle_[pos_period].size();
}


void Rle::DoString(int32_t *size, char **str) const {
	(*size) = size_;
	(*str) = new char[size_];
	int pos = 0;
	char letter;

	for (int i = 0; i < period_; ++i) {
		for (list<RunRle*>::iterator it=rle_[i].begin(); it != rle_[i].end(); ++it) {
			letter = (*it)->GetChar();
    		for (int j = 0; j < (*it)->GetSize(); ++j) {
				(*str)[pos] = letter;
				pos++;
    		}
    	}
	}
}

void RleText::DoString(int32_t *size, char **str) const {
	(*size) = size_;
	(*str) = new char[size_];
	int pos = 0;
	char letter;

	for (int i = 0; i < period_; ++i) {
		for (list<RunRle*>::iterator it=rle_[i].begin(); it != rle_[i].end(); ++it) {
			letter = (*it)->GetChar();
    		for (int j = 0; j < (*it)->GetSize(); ++j) {
				(*str)[pos] = letter;
				pos++;
    		}
    	}
	}
	for (int i = 0; i < period_; ++i) {
		for (list<RunRle*>::iterator it=t_sec_[i].begin(); it != t_sec_[i].end(); ++it) {
			letter = (*it)->GetChar();
    		for (int j = 0; j < (*it)->GetSize(); ++j) {
				(*str)[pos] = letter;
				pos++;
    		}
    	}
	}
}


/*************** Case 2 ***************/

void UpdateDeriv(RunRle *runT, RunRle *runP, int32_t size_deriv, int *deriv) {
	int u, v, y, z;
	u = runT->GetPosition();
	v = u + runT->GetSize()-1;
	y = runP->GetPosition();
	z = y + runP->GetSize()-1;

	if ((u - z >= 0) && ((u - z) < size_deriv)) deriv[u - z]++;
	if ((v - z + 1 >= 0) && ((v - z + 1) < size_deriv)) deriv[v - z + 1]--;
	if ((u - y + 1 >= 0) && ((u - y + 1) < size_deriv)) deriv[u - y + 1]--;
	if ((v - y + 2 >= 0) && ((v - y + 2) < size_deriv)) deriv[v -y + 2]++;
}

void ComputeResStar(char* t_star, int32_t size_p_star, char* p_star,
					int nb_freq_letter, int nb_$,
					int32_t size_deriv, int* deriv, int* res_hd,
					int32_t size_res, int* res) {
	res[0] = NaiveHD(t_star, size_p_star, p_star, 0) - nb_freq_letter + res_hd[0] - nb_$;
	res[1] = NaiveHD(t_star, size_p_star, p_star, 1) - nb_freq_letter + res_hd[1] - nb_$;
	for (int i = 2; i < size_res; ++i) {
		res[i] = -deriv[i] + 2 * res[i-1] - res[i-2];
	}
	for (int i= 0; i < size_res; ++i)
		res[i] += nb_freq_letter - res_hd[i];
}

void ReorganiseRes(int32_t i_l, int32_t i_r, int period,
					int32_t size_res_star, int* res_star, int32_t size_res, int* res) {
	for (int i = 0; i < i_l; ++i)
		res[i] = -1;

	int32_t x, y, m1 = size_res_star / period;
	for (int  i = i_l; i <= i_r && i < size_res; ++i) {
		x = i /period;
		y = i % period;
		res[i] = res_star[x + y * m1];
	}

	if (i_r < size_res)
		for (int i = i_r + 1; i < size_res; ++i)
			res[i] = -1;
}



void Small8kPeriod(int32_t size_text, char *text, int32_t size_pattern,
					char *pattern, int k_nb_letters, int error_k, int approx_period,
					int32_t size_res, int *res) {

	// init i_l and i_r to find T*
    int32_t i_l, i_r, m1;
    RleText *t_rle = new RleText(approx_period);  // T* stored as RLE blocks
    t_rle->MakeRle(size_pattern, size_text, text, error_k, &i_l, &i_r, &m1);

    Rle *p_rle = new Rle(approx_period);  // P* stored as RLE blocks
    p_rle->MakeRle(size_pattern, pattern, m1 * approx_period);

	vector<RunRle*> *infreq;
	infreq = new vector<RunRle*>[k_nb_letters];
	for (int i = 0; i < k_nb_letters; ++i) {
		vector<RunRle*> l;
		infreq[i] = l;
	}

	char c;
	// sort the pattern runs by letter, put them all in infreq
    for (int i = 0; i < p_rle->GetPeriod(); ++i) {
    	for (std::list<RunRle*>::iterator it = p_rle->GetList(i)->begin(); it != p_rle->GetList(i)->end(); ++it) {
    		c = (*it)->GetChar();
    		infreq[CharToInt(c)].push_back(*it);
    	}
    }

    // we don't want to compute the $ symbol
    infreq[CharToInt('$')].clear();

    int threshold = size_text*25;
	vector<char> freq;

	int nb_freq_letter = 0;
    // sort the freq/infreq letters
    for (int i = 0; i < k_nb_letters; ++i){
    	if (infreq[i].size() > threshold) { //  heavy letter
    		for (int j = 0; j < infreq[i].size(); ++j)
    			nb_freq_letter += infreq[i][j]->GetSize();
	    	freq.push_back(IntToChar(i));
	    	infreq[i].clear();
    	}
    	// else : the vector stays in infreq and we have all the runs of every infreq letter
    }

	//init T* and P*
    char *t_star, *p_star;
    int32_t size_t_star, size_p_star;
    t_rle->DoString(&size_t_star, &t_star);
    p_rle->DoString(&size_p_star, &p_star);

    int32_t size_res_star = t_rle->GetSize() - p_rle->GetSize() + 1;
    int *res_hd = new int[size_res_star];

    // Frequent letters
    HD(size_t_star, t_star, size_p_star,  p_star, &freq,
		size_res_star, res_hd);

    // Infrequent letters
	RunRle *runP;
	int32_t size_deriv = size_res_star;
    int *deriv = new int[size_deriv];
    InitTabZeros(size_deriv, deriv);
    int *res_star = new int[size_res_star];
    InitTabZeros(size_res_star, res_star);

    // compute pair of runs in T' and P*
    for (int i = 0; i < t_rle->GetPeriod(); ++i) {
    	for (std::list<RunRle*>::iterator it = t_rle->GetList(i)->begin(); it != t_rle->GetList(i)->end(); ++it) {
    		c = (*it)->GetChar();
    		for (int k = 0; k < infreq[CharToInt(c)].size(); ++k) {
    			runP = infreq[CharToInt(c)][k];
    			UpdateDeriv((*it), runP, size_deriv, deriv);
    		}
    	}
    }
    //  do the same with T" and P*
    for (int i = 0; i < t_rle->GetPeriod(); ++i) {
    	for (std::list<RunRle*>::iterator it = t_rle->GetListTSec(i)->begin(); it != t_rle->GetListTSec(i)->end(); ++it) {
    		c = (*it)->GetChar();
    		for (int k = 0; k < infreq[CharToInt(c)].size(); ++k) {
    			runP = infreq[CharToInt(c)][k];
    			UpdateDeriv((*it), runP, size_deriv, deriv);
    		}
    	}
    }

    int nb_$ = m1 * approx_period - size_pattern;

    ComputeResStar(t_star, p_rle->GetSize(), p_star, nb_freq_letter, nb_$,
    				size_deriv, deriv, res_hd, size_res_star, res_star);

    ReorganiseRes(i_l, i_r, t_rle->GetPeriod(),
    				size_res_star, res_star, size_res, res);

    delete [] t_star;
    delete [] p_star;
    delete [] res_star;
    delete [] deriv;
    delete [] res_hd;
    freq.clear();
}
