/* Copyright : GNU GPL V3
Author : Tatiana Rocher, tatiana.rocher@gmail.com

This file contains the Hamming distance algorithm
used when there is at most 2*sqrt(k) frequent symbols.
*/

#include <unistd.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>

extern "C" {
	#include "../../../Lib/fftw3/fftw-3.3.7/api/fftw3.h"
}

#include "Fft_wak.hpp"
#include "HammingDistance.hpp"
#include "Tools.hpp"

using namespace std;


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

void SortfreqInfreqCaract(int32_t size_pattern, char *pattern,
						float threshold_freq, vector<char> *freq,
						vector<int32_t> *infreq, int size_alphabet) {
	for (int32_t i = 0; i < size_alphabet; ++i) {
		if (infreq[i].size() >= threshold_freq) {
			freq->push_back(IntToChar(i));
			infreq[i].clear();
		}
	}
}


void ComputeFreq(int32_t size_pattern, int32_t size_text, int32_t size_res,
				char *text, char *pattern, vector<char> *frequent,
				FFT_wak *fft_text, FFT_wak *fft_pattern, FFT_wak *fft_res,
				int *res) {
	for (int32_t i = 0; i < size_res; i++)
		res[i] = 0;

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
	int current_char;
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


void ComputeHD(int32_t size_text, char *text, int32_t size_pattern,
						char *pattern, vector<int32_t> *infrequent, float threshold_freq,
						int size_alphabet, int32_t size_res, int *res) {

	// Init size_fft: the lenght used for the FFTs
	// It is a power of 2 (see FFTW documentation)
    int32_t size_fft = UpperPowOfTwo(size_text);

	// Initialise the fftw_plan. Should be done before initialising the in/output
    FFT_wak *fft_text = new FFT_wak(size_fft);
    FFT_wak *fft_pattern = new FFT_wak(size_fft);
    FFT_wak *fft_tmp = new FFT_wak(size_fft, false);
	// false indicates that we make a plan for a FFT inverse

    vector<char> frequent;

    SortfreqInfreqCaract(size_pattern, pattern, threshold_freq,
						&frequent, infrequent, size_alphabet);


    ComputeFreq(size_pattern, size_text, size_res, text, pattern, &frequent,
				fft_text, fft_pattern, fft_tmp, res);

    ComputeInfreq(size_text, text, size_res, infrequent, res);

    delete [] infrequent;
    delete fft_pattern;
    delete fft_text;
    delete fft_tmp;
}
