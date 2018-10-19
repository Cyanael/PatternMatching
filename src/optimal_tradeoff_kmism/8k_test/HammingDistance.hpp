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

char CharMap(char letter, int *map);

bool SortVectorOfPair(pair<char, int> a, pair<char, int> b);

void SortfreqInfreqCaract(int32_t size_pattern, char *pattern, float limit,
            int *map, int size_alphabet, vector<char> *freq,
                        vector<int32_t> *infreq);

void SortFreqCaract(int32_t size_pattern, char *pattern, int k_nb_letters, float limit,
            int *map, int size_alphabet, vector<char> *freq);

bool IsInfreq(char letter, vector<int32_t> *infreq) ;

// Computes the boolean vector of the text
// when there is a 1 where the matching letter is "letter"
void MatchLetterTextApprox(int32_t size, char *text, char letter,
          int *map, FFT_wak *fft_text);

// Computes the boolean vector of the text
// when there is a 1 where the matching letter is "letter"
void MatchLetterText(int32_t size, char *text, char letter,
          FFT_wak *fft_text);

void ReversePattern(int32_t size, FFT_wak *fft_pattern);

void ComputeFreq(int32_t size_pattern, int32_t size_text, int32_t size_res,
        char *text, char *pattern, vector<char> *frequent, int *map,
        FFT_wak *fft_text, FFT_wak *fft_pattern, FFT_wak *fft_res,
        int *res);

void ComputeInfreq(int32_t size_text, char *text, int32_t size_res,
        int *map, vector<int32_t> *infreq, int *res);

void KeepSmaller(int32_t size_res, int *tmp_res, int *res);

void ApproxHD(int32_t size_text, char *text, int32_t size_pattern,
            char *pattern, int k_nb_letters, float epsilon,
            int32_t size_res, int *res);

void FindApproximatePeriod(int32_t size_pattern, char *pattern,  int k_nb_letters,
                          float epsilon, int position_max, int value_max, int *appr_res);

void NoSmall4kPeriod(int32_t size_text, char *text, int32_t size_pattern,
                      char *pattern, int k_nb_letters, int error_k,
                      int32_t size_res, int *res) ;

void HD(int32_t size_text, char *text, int32_t size_pattern, char *pattern, 
         vector<char> *frequent, int32_t size_res, int *res);

int NaiveHD(char *text, int32_t size_pattern, char *pattern, int32_t pos);



#endif  // ApproxHD_HPP
