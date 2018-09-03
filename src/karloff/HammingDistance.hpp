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


void LoadSavedPlan(char* file);

// Computes the boolean vector of the text
// using the values of the matrix
void MakePair(int32_t size, char *str, bool **matrix, int l,
					FFT_wak *fft_str);

void ReversePattern(int32_t size, FFT_wak *fft_pattern);

void HD(int32_t size_pattern, int32_t size_text, int32_t size_res,
				char *text, char *pattern, FFT_wak *fft_text,
				FFT_wak *fft_pattern, FFT_wak *fft_res,
				bool **matrix, int l, int *res);


#endif  // HAMMINGDISTANCE_K
