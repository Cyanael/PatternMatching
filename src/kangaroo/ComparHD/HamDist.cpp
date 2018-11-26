/* Copyright : GNU GPL V3
Author : Tatiana Rocher, tatiana.rocher@gmail.com

Compilation :
install the fftw3 library
g++ -std=c++11 -O3 hamDist.cpp Fft_wak.cpp -o hd -lfftw3 -lm

Execution :
./hd text.in pattern.in optional.out
The pattern/text input file must contain its lenght then the pattern/text
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cassert>
#include <cstdint>
#include <unistd.h>

extern "C" {
	#include "../../../Lib/fftw3/fftw-3.3.7/api/fftw3.h"
}

#include "Fft_wak.hpp"

using namespace std;

int k_nb_letters = 128;


int UpperPowOfTwo(int val) {
	val--;
	val |= val >> 1;
	val |= val >> 2;
	val |= val >> 4;
	val |= val >> 8;
	val |= val >> 16;
	val++;
	return val;
}

void InitTabZeros(int32_t size, int32_t *tab) {
	for (int32_t i = 0; i < size; i++)
		tab[i] = 0;
}

int CharToInt(char letter) {
	return (int) letter;
}

char IntToChar(int val) {
	return (char) val;
}


void SortfreqInfreqCaract(int32_t size_pattern, char *pattern, float limit,
						vector<char> *freq, vector<int32_t> *infreq) {
	for (int32_t i = 0; i < size_pattern; ++i) {
		if (infreq[CharToInt(pattern[i])].size() == 0) {
			vector<int32_t> v = {i};
			infreq[CharToInt(pattern[i])] = v;
		}
		else
			infreq[CharToInt(pattern[i])].push_back(i);
	}
	for (int32_t i = 0; i < k_nb_letters; ++i) {
		if (infreq[i].size() >= limit) {
			freq->push_back(IntToChar(i));
			infreq[i].clear();
		}
	}
}

bool IsInfreq(char letter, vector<int32_t> *infreq) {
	if (infreq[CharToInt(letter)].size() > 0)
		return true;
	return false;
}

// Computes the boolean vector of the text
// when there is a 1 where the matching letter is "letter"
void MatchLetterText(int32_t size, char *text, char letter,
					FFT_wak *fft_text) {
	for (int32_t i = 0; i < size; ++i) {
		if(text[i] == letter)
			fft_text->setVal(i, 1);
		else
			fft_text->setVal(i, 0);
	}
	if (size < fft_text->getSize()) {
		for (int32_t i = size; i < fft_text->getSize(); ++i)
			fft_text->setVal(i, 0);
	}
}

void ReversePattern(int32_t size, FFT_wak *fft_pattern) {
	double tmp;
	for (int32_t i = 0; i < size/2; ++i) {
		tmp = fft_pattern->getVal(i);
		fft_pattern->setVal(i, fft_pattern->getVal(size-i-1));
		fft_pattern->setVal(size-i-1, tmp);
	}
}

void ComputeFreq(int32_t size_pattern, int32_t size_text, int32_t size_res,
				char *text, char *pattern, vector<char> *frequent,
				FFT_wak *fft_text, FFT_wak *fft_pattern, FFT_wak *fft_res,
				int *res) {
	InitTabZeros(size_res, res);

	char current_char;
	for (auto j = frequent->begin(); j != frequent->end(); ++j) {
		current_char = *j;

		MatchLetterText(size_text, text, current_char, fft_text);
		MatchLetterText(size_pattern, pattern, current_char, fft_pattern);
		ReversePattern(size_pattern, fft_pattern);

		fft_text->ExecFFT();
		fft_pattern->ExecFFT();
		fft_res->FFTMultiplication(fft_text, fft_pattern);
		fft_res->ExecFFT();

		for (int32_t i = 0; i < size_res; ++i) {
			// sometime, the double variable is close to an integer
			// but a little inferior, the +0,5 corrects the cast into an integer
			res[i]+= (fft_res->getVal(i+size_pattern-1)+0.5);
		}
	}
}

void ComputeInfreq(int32_t size_text, char *text, int32_t size_res,
				vector<int32_t> *infreq, int *res) {
	char current_char;
	for (int32_t i = 0; i < size_text; ++i) {
		if (IsInfreq(text[i], infreq)) {
			current_char = CharToInt(text[i]);
			for (int32_t j = 0; j < infreq[current_char].size(); ++j) {
				if (i >= infreq[current_char][j]
				&& i - infreq[current_char][j] < size_res) {
					res[i-infreq[current_char][j]]++;
				}
			}
		}
	}
}


int HD(int32_t size_text, char* text, int32_t size_pattern, char* pattern, int32_t size_res, int* res) {
	
	// Init size_fft: the lenght used for the FFTs
	// It is a power of 2 (see FFTW documentation)
	int32_t size_fft = UpperPowOfTwo(size_text);

	// Initialise the fftw_plan. Should be done before initialising the in/output
	FFT_wak *fft_text = new FFT_wak(size_fft);
	FFT_wak *fft_pattern = new FFT_wak(size_fft);
	FFT_wak *fft_tmp = new FFT_wak(size_fft, false);
	// false indicates that we make a plan for a FFT inverse

	// Begin of computation
	// Sort P's caracteres in frequent/infrequent caractere
	int32_t threshold_freq = sqrt(size_pattern *log2(size_text));
	vector<char> frequent;
	vector<int32_t> *infrequent = new vector<int32_t>[k_nb_letters];

	SortfreqInfreqCaract(size_pattern, pattern, threshold_freq,
						&frequent, infrequent);


	ComputeFreq(size_pattern, size_text, size_res, text, pattern, &frequent,
				fft_text, fft_pattern, fft_tmp, res);

	ComputeInfreq(size_text, text, size_res, infrequent, res);

	frequent.clear();
	for (int i=0; i<k_nb_letters; ++i)
		infrequent[i].clear();
	delete [] infrequent;

	delete fft_pattern;
	delete fft_text;
	delete fft_tmp;
}
