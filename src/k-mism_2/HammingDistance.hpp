/* Copyright : ???
Author : Tatiana Rocher, tatiana.rocher@gmail.com

This file contains the Hamming distance algorithm 
used when there is at most 2*sqrt(k) frequent symbols.
*/

#ifndef HAMMINGDISTANCE_H
#define HAMMINGDISTANCE_H

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <chrono>
#include <unistd.h>

extern "C" {
	#include "../../Lib/fftw3/fftw-3.3.7/api/fftw3.h"
}

#include "Fft_wak.hpp"

using namespace std;



int UpperPowOfTwo(int val);

void InitTabZeros(int32_t size, int32_t *tab);

int CharToInt(char letter);

char IntToChar(int val);


void SortfreqInfreqCaract(int32_t size_pattern, char *pattern, float limit,
						vector<char> *freq, vector<int32_t> *infreq);


bool IsInfreq(char letter, vector<int32_t> *infreq);

// Computes the boolean vector of the text
// when there is a 1 where the matching letter is "letter"
void MatchLetterText(int32_t size, char *text, char letter,
					FFT_wak *fft_text);

void ReversePattern(int32_t size, FFT_wak *fft_pattern);

void SortfreqInfreqCaract(int32_t size_pattern, char *pattern, int32_t limit,
						vector<char> *freq, vector<int32_t> *infreq, int size_alphabet);

void ComputeFreq(int32_t size_pattern, int32_t size_text, int32_t size_res,
				char *text, char *pattern, vector<char> *frequent,
				FFT_wak *fft_text, FFT_wak *fft_pattern, FFT_wak *fft_res,
				int *res);

void ComputeInfreq(int32_t size_text, char *text, int32_t size_res,
				vector<int32_t> *infreq, int *res);


void ComputeHD(int32_t size_text, char *text, int32_t size_pattern, 
						char *pattern, vector<int32_t> *infrequent, 
						float threshold_freq, int size_alphabet, 
						int32_t size_res, int *res);

#endif  // HAMMINGDISTANCE_K
