/* Copyright : ???
Author : Tatiana Rocher, tatiana.rocher@gmail.com
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cassert>
#include <cstdint>
#include <chrono>
#include <unistd.h>

extern "C" {
  #include "../../../Lib/fftw3/fftw-3.3.7/api/fftw3.h"
}

#include "Fft_wak.hpp"
#include "Tools.hpp"
#include "LCP.hpp"

using namespace std;

// int k_nb_lettint k_nb_letters, ers = 128;


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

  if (size_alphabet <= 90) // number of character we can read ('!' (33) to 'z' (122))
    for (int i = 0; i < k_nb_letters; ++i)
      map[i] = ((a*i + b) % p) % size_alphabet + 33; // we add 33 to have characters we can read in tests
  else
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

void SortfreqInfreqCaract2(int32_t size_pattern, char *pattern, float limit,
            vector<char> *freq, vector<int32_t> *infreq, int k_nb_letters) {
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

void ComputeFreq2(int32_t size_pattern, int32_t size_text, int32_t size_res,
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

void ComputeInfreq2(int32_t size_text, char *text, int32_t size_res,
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


void KeepSmaller(int32_t size_res, int *tmp_res, int *res) {
    for (int32_t i = 0; i < size_res; ++i)
    if (res[i] > tmp_res[i])
            res[i] = tmp_res[i];
}


void ApproxHD(int32_t size_text, char *text, int32_t size_pattern,
            char *pattern, int k_nb_letters, float epsilon,
            int32_t size_res, int *res) {
  chrono::time_point<chrono::system_clock> start, mid, end;
  chrono::duration<double> texec;
  start = chrono::system_clock::now();
  mid = start;

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
  for (int32_t i = 0; i < size_res; ++i)
    res[i] = size_pattern;

  int32_t threshold_freq = sqrt(size_pattern *log2(size_text));
  vector<char> frequent;
  vector<int32_t> *infrequent = new vector<int32_t>[k_nb_letters];

  int32_t cpt_loop = 5;
  int *map = new int[k_nb_letters];
  int size_alphabet = min(round(2 / epsilon), (float)k_nb_letters);
  if (size_alphabet == k_nb_letters)
    cpt_loop = 1;
    cout << "nb lettre alphabet " << size_alphabet << endl;
    // cout << "Number of loop iterations: " << cpt_loop << endl;

  end = chrono::system_clock::now();
  texec = end-mid;
  // cout << "init : " << texec.count() << "s" << endl;
  mid= end;

  for (int i = 0; i < cpt_loop; ++i) {
    MapLetters(k_nb_letters, size_alphabet, size_prime_number, prime_numbers, map);

    end = chrono::system_clock::now();
    texec = end-mid;
    // cout << endl << "  map letters : " << texec.count() << "s" << endl;
    mid = end;

    // Sort P's caracteres in frequent/infrequent caractere
    SortfreqInfreqCaract(size_pattern, pattern, threshold_freq, map,
                            size_alphabet, &frequent, infrequent);

    SortFreqCaract(size_pattern, pattern, k_nb_letters, threshold_freq, map,
                              size_alphabet, &frequent);

    end = chrono::system_clock::now();
    texec = end-mid;
    // cout << " sort freq / infreq : " << texec.count() << "s" << endl;
    mid = end;

    ComputeFreq(size_pattern, size_text, size_res, text, pattern, &frequent,
            map, fft_text, fft_pattern, fft_tmp, tmp_res);

    end = chrono::system_clock::now();
    texec = end-mid;
    // cout << " freq : " << texec.count() << "s   nb freq :" << frequent.size() << endl;
    mid = end;

    ComputeInfreq(size_text, text, size_res, map, infrequent, tmp_res);

    end = chrono::system_clock::now();
    texec = end-mid;
    // cout << " infreq : " << texec.count() << "s" << endl;
    mid= end;

    KeepSmaller(size_res, tmp_res, res);

    end = chrono::system_clock::now();
    texec = end-mid;
    // cout << "  res : " << texec.count() << "s" << endl;
    mid= end;
  }

  for (int i = 0; i < size_res; ++i)
    res[i] = size_pattern - res[i];

  delete [] tmp_res;
  delete [] map;
  delete [] prime_numbers;

  frequent.clear();
  for (int i = 0; i < k_nb_letters; ++i)
    infrequent[i].clear();
  delete [] infrequent;

  delete fft_pattern;
  delete fft_text;
  delete fft_tmp;

  end = chrono::system_clock::now();
  texec = end-start;
  // cout << "Total time : " << texec.count() << "s" << endl;
}


void HD(int32_t size_text, char *text, int32_t size_pattern, char *pattern,
         vector<char> *frequent, int32_t size_res, int *res) {
  InitTabZeros(size_res, res);

  int32_t size_fft = UpperPowOfTwo(size_text);
  FFT_wak *fft_text = new FFT_wak(size_fft);
  FFT_wak *fft_pattern = new FFT_wak(size_fft);
  FFT_wak *fft_res = new FFT_wak(size_fft, false);

  chrono::time_point<chrono::system_clock> start, mid, end;
  chrono::duration<double> texec;
  char current_char;

  for (auto j = frequent->begin(); j != frequent->end(); ++j) {
    current_char = *j;

    start = chrono::system_clock::now();
    MatchLetterText(size_text, text, current_char, fft_text);
    MatchLetterText(size_pattern, pattern, current_char, fft_pattern);
    end = chrono::system_clock::now();
    texec = end-mid;
    // cout << " bit vect : " << texec.count() << "s" << endl;
    mid = end;

    ReversePattern(size_pattern, fft_pattern);
    fft_text->ExecFFT();
    fft_pattern->ExecFFT();
    end = chrono::system_clock::now();
    texec = end-mid;
    // cout << " fft exec : " << texec.count() << "s" << endl;
    mid = end;

    fft_res->FFTMultiplication(fft_text, fft_pattern);
    end = chrono::system_clock::now();
    texec = end-mid;
    // cout << " mult + iexec : " << texec.count() << "s" << endl;
    mid = end;

    fft_res->ExecFFT();

    for (int32_t i = 0; i < size_res; ++i) {
      // sometime, the double variable is close to an integer
      // but a little inferior, the +0,5 corrects the cast into an integer
      res[i]+= (fft_res->getVal(i+size_pattern-1)+0.5);
    }

      end = chrono::system_clock::now();
      texec = end-mid;
      // cout << " write res : " << texec.count() << "s" << endl;
      mid = end;
  }

    delete fft_text;
    delete fft_pattern;
    delete fft_res;
}


int HD2(int32_t size_text, char* text, int32_t size_pattern, char* pattern, int k_nb_letters, int32_t size_res, int* res) {
  
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

  SortfreqInfreqCaract2(size_pattern, pattern, threshold_freq,
            &frequent, infrequent, k_nb_letters);


  ComputeFreq2(size_pattern, size_text, size_res, text, pattern, &frequent,
        fft_text, fft_pattern, fft_tmp, res);

  ComputeInfreq2(size_text, text, size_res, infrequent, res);

  frequent.clear();
  for (int i=0; i<k_nb_letters; ++i)
    infrequent[i].clear();
  delete [] infrequent;

  delete fft_pattern;
  delete fft_text;
  delete fft_tmp;
  frequent.clear();
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

  vector<char> freq;
	vector<int32_t> *infreq = new vector<int32_t>[k_nb_letters];

  for (int32_t i = 0; i < size_pattern; ++i) {
  	if (infreq[CharToInt(pattern[i])].size() == 0) {
  		vector<int32_t> v = {i};
  		infreq[CharToInt(pattern[i])] = v;
  	}
  	else
  		infreq[CharToInt(pattern[i])].push_back(i);
	}
	for (int32_t i = 0; i < k_nb_letters; ++i) {
		if (infreq[i].size() >= sqrt(size_pattern *log2(size_pattern2))) {
			freq.push_back(IntToChar(i));
			infreq[i].clear();
		}
	}

  HD(size_pattern2, pattern2, size_pattern, pattern, &freq, size_res, res);
  // ApproxHD(size_pattern2, pattern2, size_pattern, pattern, k_nb_letters, (float)1, size_res, res);

  bool stop = false;
  int i = 1, ret = 0;

  while ( (i <= position_max) && (i <= size_res) && (!stop)) {
  // for (int i = 1;; ++i){
    if (res[i] < value_max) {
      stop = true;
      ret = i;
    }
    ++i;
  }
  if (!stop) 
    ret = 0;

  delete [] pattern2;
  delete [] res;
  freq.clear();
  for (int i=0; i<k_nb_letters; ++i)
      infreq[i].clear();
  delete [] infreq;

  return ret;
}

void NoSmall4kPeriod(int32_t size_text, char *text, int32_t size_pattern,
                      char *pattern, int k_nb_letters, int error_k,
                      int32_t size_res, int *res) {
  chrono::time_point<chrono::system_clock> start, mid, end;
  chrono::duration<double> texec;
  start = chrono::system_clock::now();

  float epsilon = 1;
  ApproxHD(size_text, text, size_pattern, pattern, k_nb_letters, epsilon, size_res, res);
  // cout << "approx res" << endl;
  // PrintTable(size_res, res);

  end = chrono::system_clock::now();
  texec = end-start;
  cout << " temps approx : " << texec.count() << "s" << endl;

  vector<int32_t> pos_to_search;
  for (int32_t i= 0; i < size_res; ++i)
    if(res[i] <= error_k)
      pos_to_search.push_back(i);
  LCP(size_text, text, size_pattern, pattern, k_nb_letters, error_k,
      pos_to_search, size_res, res);
}

int NaiveHD(char *text, int32_t size_pattern, char *pattern, int32_t pos) {
  int res = 0;
  for (int i = 0; i < size_pattern; ++i)
    if (pattern[i] != text[pos + i])
      res++;
  return res;
}
