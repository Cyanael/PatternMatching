/* Copyright : ???
Author : Tatiana Rocher, tatiana.rocher@gmail.com

*/
#include <iostream>

#include "Rle.hpp"
#include "Tools.hpp"

using namespace std;


RunRle::RunRle(int p, char l) {
	position_ = p;
	size_ = 1;
	letter_ = l;
}

// RunRle::~RunRle() {
// 	;
// }

void RunRle::ExtendRunBegin(int pos) {
	position_ = pos;
	size_ ++;
}

void RunRle::ExtendRunEnd() {
	size_++;
}

char RunRle::GetChar() {
	return letter_;
}

int RunRle::GetSize() {
	return size_;
}

void RunRle::PrintRun() {
	std::cout << "(" << size_ << ", " << letter_ << ")";
}




/************ Rle Class ************/


Rle::Rle(int p, int nb_letters) {
	nb_runs_ = 0;
	begin_ = 0;
	period_ = p;
	size_ = 0;
	list<RunRle*> *rle = new list<RunRle*>[p]();
	for (int i = 0; i < period_; ++i) {
		list<RunRle*> *list_run = new list<RunRle*>();
		rle[i] = (*list_run);
	}
	rle_ = rle;
	cpt_letter_run_ = new int[nb_letters];
} 

Rle::~Rle() {
	// TODO
}

void Rle::AddLetterBegin(int pos, int period, char letter) {
	size_ ++;
	begin_ = period;
	if (rle_[period].size() == 0) {
		RunRle *run = new RunRle(pos, letter);
		rle_[period].push_front(run);
		cpt_letter_run_[CharToInt(letter)]++;
	}
	else {
		char c = rle_[period].front()->GetChar();
		if (c == letter)
			rle_[period].front()->ExtendRunBegin(pos);
		else {
			RunRle *run = new RunRle(pos, letter);
			rle_[period].push_front(run);
			cpt_letter_run_[CharToInt(letter)]++;
		}
	}
}

void Rle::AddLetterEnd(int pos, int period, char letter) {
	size_ ++;
	if (rle_[period].size() == 0) {
		RunRle *run = new RunRle(pos, letter);
		rle_[period].push_back(run);
		cpt_letter_run_[CharToInt(letter)]++;
	}
	else {
		char c = rle_[period].back()->GetChar();
		if (c == letter)
			rle_[period].back()->ExtendRunEnd();
		else {
			RunRle *run = new RunRle(pos, letter);
			rle_[period].push_back(run);
			cpt_letter_run_[CharToInt(letter)]++;
		}
	}
}

void Rle::RleText(int32_t size_pattern, int32_t size_text, char *text,
					int nb_errors, int32_t *i_l, int32_t *i_r) {
	int32_t p, i = size_pattern-1;

	// search for i_l
	while (i >= 0 && nb_runs_ <= nb_errors * 6) {
		p = i % period_;
		AddLetterBegin(i, p, text[i]);
		i--;
	}
	(*i_l) = i + 1;
	// cout << "i_l = " << (*i_l) << endl;
	i = size_pattern;

	// search for i_r
	bool continu = true;
	while ( i < size_text && continu) {
		p = i % period_;
		AddLetterEnd(i, p, text[i]);
		i++;
		if (nb_runs_ >= nb_errors * 12 && i_r - i_l + 1 % period_ == 0)
			continu = false;
	}
	// cout << "i = " << i << endl;
	// cout << "modulo " << period_ << " = " << (i - (*i_l)) % period_ << endl;
	//pad with # until having a lenght divisible by period
	while ((i - (*i_l)) % period_ != 0) {
		// cout << "pad " << i << endl;
		p = i % period_;
		AddLetterEnd(i, p, '#');
		i++;
		// cout << "modulo " << period_ << " = " << (i - (*i_l)) % period_ << endl;
	}
	(*i_r) = i - 1;
	// cout << "i_r = " << (*i_r) << endl;
}

void Rle::RlePattern(int32_t size_pattern, char *pattern, int32_t size_padding) {
	int p, i = 0;
	while (i<size_pattern) {
		p = i % period_;
		AddLetterEnd(i, p, pattern[i]);
		i++;
	}
	int32_t size_total = size_pattern + size_padding;
	while(i< size_total) {
		p = i % period_;
		AddLetterEnd(i, p, '$');
		i++;
	}
}


int Rle::NbRuns() {
	return nb_runs_;
}

int Rle::NbLettersRuns(char letter) {
	return cpt_letter_run_[CharToInt(letter)];
}

int32_t Rle::Size() {
	return size_;
}

void Rle::PrintRle() {
	for (int i=0; i<period_; ++i) {
  		for (list<RunRle*>::iterator it = rle_[i].begin(); it != rle_[i].end(); it++) {
    		(*it)->PrintRun();
    		cout << " ";
    	}
	}
	cout << endl;
}

void Rle::ToString(int32_t *size, char **str) {
	(*size) = size_;
	(*str) = new char[size_];
	int pos = 0;
	char letter;
	int i = begin_;

	do {
		for (list<RunRle*>::iterator it=rle_[i].begin(); it != rle_[i].end(); ++it) {
			letter = (*it)->GetChar();
    		for (int j = 0; j < (*it)->GetSize(); ++j) {
				(*str)[pos] = letter;
				pos++;
    		}
    	}
   	i++;
	}

	while ((i % period_) != (begin_)% period_);
}	