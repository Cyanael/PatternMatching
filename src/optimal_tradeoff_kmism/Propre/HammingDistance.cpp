/* Copyright : ???
Author : Tatiana Rocher, tatiana.rocher@gmail.com
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
#include "Tools.hpp"
#include "LCP.hpp"

using namespace std;


void MapLetters(int k_nb_letters, int size_alphabet, int size_prime_number, int *prime_numbers, int *map) {
  struct timespec ts;
  if (timespec_get(&ts, TIME_UTC) == 0) {
    cout << "error seed." << endl;
  }
  srandom(ts.tv_nsec ^ ts.tv_sec); // compute seed

  int p = 0;
  int r;
  while(p<=size_alphabet){
    r = random()%size_prime_number;
    p = prime_numbers[r];
  }
  int a = random()%p;
  int b = random()%p;

  for (int i = 0; i < k_nb_letters; ++i)
    map[i] = ((a*i + b) % p) % size_alphabet;
}

char CharMap(char letter, int *map) {
    return IntToChar(map[CharToInt(letter)]);
}

bool SortVectorOfPair(pair<char, int> a, pair<char, int> b) {
  return a.first < b.first;
}

void SortfreqInfreqCaract(int32_t size_pattern, char *pattern, float limit,
            int *map, int size_alphabet, vector<char> *freq,
                        vector<int32_t> *infreq) {
    freq->clear();
    infreq->clear();
  for (int32_t i = 0; i < size_pattern; ++i) {
        int current_val = map[CharToInt(pattern[i])];
    if (infreq[current_val].size() == 0) {
      vector<int32_t> v = {i};
      infreq[current_val] = v;
    }
    else {
      infreq[current_val].push_back(i);
    }
  }
  for (int32_t i = 0; i < size_alphabet; ++i) {
    if (infreq[i].size() >= limit) {
      freq->push_back(IntToChar(i));
      infreq[i].clear();
    }
  }
}

void SortFreqCaract(int32_t size_pattern, char *pattern, int k_nb_letters,
            float limit, int *map, int size_alphabet, vector<char> *freq) {
    freq->clear();
    int *infreq = new int[k_nb_letters]();
  for (int32_t i = 0; i < size_pattern; ++i) {
        int current_val = map[CharToInt(pattern[i])];
    if (infreq[current_val] == 0) {
      infreq[current_val] = 1;
      freq->push_back(current_val);
    }
  }
  delete [] infreq;
}



bool IsInfreq(char letter, vector<int32_t> *infreq) {
  if (infreq[CharToInt(letter)].size() > 0)
    return true;
  return false;
}

// Computes the boolean vector of the text
// when there is a 1 where the matching letter is "letter"
// use the map
void MatchLetterTextApprox(int32_t size, char *text, char letter,
          int *map, FFT_wak *fft_text) {
  for (int32_t i = 0; i < size; ++i) {
    if(CharMap(text[i], map) == letter)
      fft_text->setVal(i, 1);
    else
      fft_text->setVal(i, 0);
  }
  if (size < fft_text->getSize()) {
    for (int32_t i = size; i < fft_text->getSize(); ++i)
      fft_text->setVal(i, 0);
  }
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
        char *text, char *pattern, vector<char> *frequent, int *map,
        FFT_wak *fft_text, FFT_wak *fft_pattern, FFT_wak *fft_res,
        int *res) {
  InitTabZeros(size_res, res);

  char current_char;
  for (auto j = frequent->begin(); j != frequent->end(); ++j) {
    current_char = *j;
    MatchLetterTextApprox(size_text, text, current_char, map, fft_text);
    MatchLetterTextApprox(size_pattern, pattern, current_char, map, fft_pattern);

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
        int *map, vector<int32_t> *infreq, int *res) {
  int current_val;
  for (int32_t i = 0; i < size_text; ++i) {
    if (IsInfreq(CharMap(text[i], map), infreq)) {
      current_val = map[CharToInt(text[i])];
      for (int32_t j = 0; j < infreq[current_val].size(); ++j) {
        if ((i >= infreq[current_val][j]) && (i-infreq[current_val][j] < size_res)) {
          res[i-infreq[current_val][j]]++;
        }
      }
    }
  }
}


void KeepSmaller(int32_t size_res, int *tmp_res, int *res) {
    for (int32_t i = 0; i < size_res; ++i)
    if (res[i] > tmp_res[i])
            res[i] = tmp_res[i];
}


void ApproxHD(int32_t size_text, char *text, int32_t size_pattern,
            char *pattern, int k_nb_letters, float epsilon,
            int32_t size_res, int *res) {
  int cst_loop = 1;
  int *prime_numbers;
  int size_prime_number;
  ReadIntFile("prime_numbers.txt", &size_prime_number, &prime_numbers);

  // Init size_fft: the lenght used for the FFTs
  // It is a power of 2 (see FFTW documentation)
  int32_t size_fft = UpperPowOfTwo(size_text);

  // Initialise the fftw_plan. Should be done before initialising the in/output
  FFT_wak *fft_text = new FFT_wak(size_fft);
  FFT_wak *fft_pattern = new FFT_wak(size_fft);
  FFT_wak *fft_tmp = new FFT_wak(size_fft, false);
  // false indicates that we make a plan for a FFT inverse

  // Init size_res : length of the res table which indicates the hamm dist
  int *tmp_res = new int[size_res]();
  for (int32_t i = 0; i<size_res; ++i)
    res[i] = size_pattern;

  int32_t threshold_freq = sqrt(size_pattern *log2(size_text));
  vector<char> frequent;
  vector<int32_t> *infrequent = new vector<int32_t>[k_nb_letters];

  int32_t cpt_loop = cst_loop * log2(size_text);
  int *map = new int[k_nb_letters];
  int size_alphabet = min(round(2 / epsilon), (float)k_nb_letters);
  if (size_alphabet == k_nb_letters)
    cpt_loop = 1;

  for (int i = 0; i < cpt_loop; ++i) {
    MapLetters(k_nb_letters, size_alphabet, size_prime_number, prime_numbers, map);

    // Sort P's caracteres in frequent/infrequent caractere
    SortfreqInfreqCaract(size_pattern, pattern, threshold_freq, map,
                            size_alphabet, &frequent, infrequent);

    ComputeFreq(size_pattern, size_text, size_res, text, pattern, &frequent,
            map, fft_text, fft_pattern, fft_tmp, tmp_res);

    ComputeInfreq(size_text, text, size_res, map, infrequent, tmp_res);

    KeepSmaller(size_res, tmp_res, res);

    frequent.clear();
  for (int i = 0; i < k_nb_letters; ++i)
    infrequent[i].clear();
  }

  for (int i = 0; i < size_res; ++i)
    res[i] = size_pattern - res[i];

  delete [] tmp_res;
  delete [] map;
  delete [] prime_numbers;

  delete [] infrequent;

  delete fft_pattern;
  delete fft_text;
  delete fft_tmp;

}

int findApproximatePeriod(int32_t size_pattern, char *pattern, int k_nb_letters,
                          float epsilon, int position_max, int value_max) {
  int32_t size_pattern2 = size_pattern * 2 - 1;
  char * pattern2 = new char[size_pattern2];
  int32_t size_res = size_pattern;
  int* res = new int[size_res];

  //  create a second string containing the pattern followed by m '$' symbols
  for (int32_t i = 0; i < size_pattern; ++i)
    pattern2[i] = pattern[i];
  for (int32_t i = size_pattern; i < size_pattern2; ++i)
    pattern2[i] = '$';

  ApproxHD(size_pattern2, pattern2, size_pattern, pattern, k_nb_letters, (float)1, size_res, res);

  for (int i = 1; i <= position_max; ++i){
    if (res[i] < value_max)
      return i;
  }
  return 0;
}


void NoSmall4kPeriod(int32_t size_text, char *text, int32_t size_pattern,
                      char *pattern, int k_nb_letters, int error_k,
                      int32_t size_res, int *res) {
  float epsilon = 1;
  ApproxHD(size_text, text, size_pattern, pattern, k_nb_letters, epsilon, size_res, res);

  vector<int32_t> pos_to_search;
  for (int32_t i= 0; i < size_res; ++i)
    if(res[i] <= error_k)
      pos_to_search.push_back(i);
  LCP(size_text, text, size_pattern, pattern, k_nb_letters, error_k, 
      pos_to_search, size_res, res);
}




void HD(int32_t size_text, char *text, int32_t size_pattern, char *pattern, 
         vector<char> *frequent, int32_t size_res, int *res) {
  InitTabZeros(size_res, res);

  int32_t size_fft = UpperPowOfTwo(size_text);
  FFT_wak *fft_text = new FFT_wak(size_fft);
  FFT_wak *fft_pattern = new FFT_wak(size_fft);
  FFT_wak *fft_res = new FFT_wak(size_fft, false);
  
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

    delete fft_text;
    delete fft_pattern;
    delete fft_res;
}

int NaiveHD(char *text, int32_t size_pattern, char *pattern, int32_t pos) {
  int res = 0;
  for (int i = 0; i < size_pattern; ++i)
    if (pattern[i] != text[pos + i])
      res++;
  return res;
}
