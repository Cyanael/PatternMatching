/* Copyright : ???
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

void RunRle::PrintRun() const {
	std::cout << "(" << size_ << ", " << letter_ << ")";
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
	//  TODO
	// for (int i = 0; i < period_; ++i) {
	// 	for (std::list<RunRle*>::iterator it = rle_[i].begin(); it != rle_[i].end(); ++it)
	// 		delete (*it);
	// 	rle_[i].clear();
	// }
	// delete [] rle_;
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
	//  TODO
	// for (int i = 0; i < period_; ++i) {
	// 	for (std::list<RunRle*>::iterator it = rle_[i].begin(); it != rle_[i].end(); ++it)
	// 		delete (*it);
	// 	rle_[i].clear();

	// 	for (std::list<RunRle*>::iterator it = t_sec_[i].begin(); it != t_sec_[i].end(); ++it)
	// 		delete (*it);
	// 	t_sec_[i].clear();
	// }
	// delete [] rle_;
	// delete [] t_sec_;
}


void RleText::MakeRle(int32_t size_pattern, int32_t size_text, char *text,
					int nb_errors, int32_t *i_l, int32_t *i_r, int max_runs) {
	int32_t p, i = size_pattern-1;
	int nb_runs = 0;

	//  extend the text from m to the left : gives us i_l
	while (i >= 0 && nb_runs <= nb_errors * max_runs) {
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
		if (nb_runs >= (nb_errors * 2 * max_runs) && (i_r - i_l + 1) % period_ == 0)
			continu = false;
	}
	(*i_r) = i - 1;
	cout << "nb runs : " << nb_runs << endl;
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


void Rle::PrintRle() const {
	cout << "period = " << period_;
	cout << " nb of letters = " << size_ << endl;
	for (int i = 0; i < period_; ++i) {
		cout << i << " " ;
  		for (list<RunRle*>::iterator it = rle_[i].begin(); it != rle_[i].end(); it++) {
    		(*it)->PrintRun();
    		cout << " ";
    	}
    	cout << endl;
	}
}

void RleText::PrintRle() const {
	Rle::PrintRle();
	for (int i = 0; i < period_; ++i) {
		cout << i << " " ;
  		for (list<RunRle*>::iterator it = t_sec_[i].begin(); it != t_sec_[i].end(); it++) {
    		(*it)->PrintRun();
    		cout << " ";
    	}
    	cout << endl;
	}
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

void Small8kPeriod(int32_t size_text, char *text, int32_t size_pattern, int error_k, int approx_period,
					int32_t *i_l, int32_t *i_r, int nb_runs) {

	// init i_l and i_r to find T*
    int32_t m1;
    RleText *t_rle = new RleText(approx_period);  // T* stored as RLE blocks
    t_rle->MakeRle(size_pattern, size_text, text, error_k, i_l, i_r, nb_runs);
}
