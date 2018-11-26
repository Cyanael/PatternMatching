/* Copyright : GNU GPL V3
Author : Tatiana Rocher, tatiana.rocher@gmail.com

Compilation :
install the fftw3 library
g++ -std=c++11 hamDist.cpp Fft_wak.cpp -o hd -lfftw3 -lm

Execution :
./hd text.in pattern.in optional.out
The pattern/text input file must contain its lenght then the pattern/text
*/

#ifndef HAMDIST_H
#define HAMDIST_H


extern "C" {
	#include "../../../Lib/fftw3/fftw-3.3.7/api/fftw3.h"
}
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cassert>
#include <cstdint>
#include <unistd.h>

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

void ComputeFreq(int32_t size_pattern, int32_t size_text, int32_t size_res,
				char *text, char *pattern, vector<char> *frequent,
				FFT_wak *fft_text, FFT_wak *fft_pattern, FFT_wak *fft_res,
				int *res);

void ComputeInfreq(int32_t size_text, char *text, int32_t size_res,
				vector<int32_t> *infreq, int *res);


int HD(int32_t size_text, char* text, int32_t size_pattern, char* pattern, int32_t size_res, int* res);

#endif  // HAMDIST_H
