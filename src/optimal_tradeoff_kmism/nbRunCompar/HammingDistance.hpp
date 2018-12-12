/* Copyright : ???
Author : Tatiana Rocher, tatiana.rocher@gmail.com

Compilation :
install the fftw3 library
g++ -std=c++11 approxHamDist.cpp Fft_wak.cpp -o ahd -lfftw3 -lm

Execution :
./hd text.in pattern.in errorDistance -o optional.out -p plan -c constante_loop
The pattern/text input file must contain its lenght then the pattern/text
*/


#ifndef ApproxHD_HPP
#define ApproxHD_HPP

#include <vector>
#include <algorithm>

#include "Fft_wak.hpp"

using namespace std;


void MapLetters(int k_nb_letters, int size_alphabet, int size_prime_number,
                int *prime_numbers, int *map);

// char CharMap(char letter, int *map);

bool SortVectorOfPair(pair<int, int> a, pair<int, int> b);

void SortfreqInfreqCaract(int32_t size_pattern, int *pattern, float limit,
            int *map, int size_alphabet, vector<int> *freq,
                        vector<int32_t> *infreq);

void SortFreqCaract(int32_t size_pattern, int *pattern, int k_nb_letters, float limit,
            int *map, int size_alphabet, vector<int> *freq);

bool IsInfreq(int letter, vector<int32_t> *infreq) ;

// Computes the boolean vector of the text
// when there is a 1 where the matching letter is "letter"
void MatchLetterTextApprox(int32_t size, int *text, int letter,
          int *map, FFT_wak *fft_text);

// Computes the boolean vector of the text
// when there is a 1 where the matching letter is "letter"
void MatchLetterText(int32_t size, int *text, int letter,
          FFT_wak *fft_text);

void ReversePattern(int32_t size, FFT_wak *fft_pattern);

void ComputeFreqApprox(int32_t size_pattern, int32_t size_text, int32_t size_res,
        int *text, int *pattern, vector<int> *frequent, int *map,
        FFT_wak *fft_text, FFT_wak *fft_pattern, FFT_wak *fft_res,
        int *res);

void ComputeInfreqApprox(int32_t size_text, int *text, int32_t size_res,
        int *map, vector<int32_t> *infreq, int *res);

void ComputeFreqHD(int32_t size_text, int *text, int32_t size_pattern, int *pattern,
         vector<int> *frequent, int32_t size_res, int *res);

void ComputeInfreqHD(int32_t size_text, int *text, int32_t size_res,
        vector<int32_t> *infreq, int *res);

void KeepSmaller(int32_t size_res, int *tmp_res, int *res);

void ApproxHD(int32_t size_text, int *text, int32_t size_pattern,
            int *pattern, int k_nb_letters, float epsilon,
            int32_t size_res, int *res);

int findApproximatePeriod(int32_t size_pattern, int *pattern,  int k_nb_letters,
                          float epsilon, int position_max, int value_max);

void NoSmall4kPeriod(int32_t size_text, int *text, int32_t size_pattern,
                      int *pattern, int k_nb_letters, int error_k,
                      int32_t size_res, int *res) ;

int NaiveHD(int *text, int32_t size_pattern, int *pattern, int32_t pos);



#endif  // ApproxHD_HPP
