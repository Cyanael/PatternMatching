/* Copyright : ???
Author : Tatiana Rocher, tatiana.rocher@gmail.com

This file contains the Hamming distance algorithm
used when there is at most 2*sqrt(k) frequent symbols.
*/

#include <unistd.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <chrono>

extern "C" {
	#include "../../Lib/fftw3/fftw-3.3.7/api/fftw3.h"
}

#include "Fft_wak.hpp"
#include "HammingDistance.hpp"
#include "Tools.hpp"

using namespace std;



void LoadSavedPlan(char* file) {
	int res = 0;
	res = fftw_import_wisdom_from_filename(file);
	if (res != 0)
		cout << "Loading plans from " << file << " succeed."<< endl;
	else
		cout << "Error while loading plans from " << file << endl;
}


// Computes the boolean vector of the text
// when there is a 1 where the matching letter is "letter"
void MakePair(int32_t size, char *str, bool **matrix, int l,
					FFT_wak *fft_str) {
	for (int32_t i = 0; i < size; ++i)
		fft_str->setVal(i, matrix[l][CharToInt(str[i])]);
	for (int32_t i = size; i < fft_str->getSize(); ++i)
		fft_str->setVal(i, 0);
}


void ReversePattern(int32_t size, FFT_wak *fft_pattern) {
	double tmp;
	for (int32_t i = 0; i < size/2; ++i) {
		tmp = fft_pattern->getVal(i);
		fft_pattern->setVal(i, fft_pattern->getVal(size-i-1));
		fft_pattern->setVal(size-i-1, tmp);
	}
}


void HD(int32_t size_pattern, int32_t size_text, int32_t size_res,
				char *text, char *pattern, FFT_wak *fft_text,
				FFT_wak *fft_pattern, FFT_wak *fft_res,
				bool **matrix, int l, int *res) {
	InitTabZeros(size_res, res);

	chrono::time_point<chrono::system_clock> start, mid, end;
	chrono::duration<double> texec;
	
	start = chrono::system_clock::now();
	MakePair(size_text, text, matrix, l, fft_text);
	MakePair(size_pattern, pattern, matrix, l, fft_pattern);
	end = chrono::system_clock::now();
	texec = end-mid;
	// cout << "	bit vect : " << texec.count() << "s" << endl;
	mid = end;

	// cout << "Paire T : " ;
	// for (int i = 0; i < fft_text->getSize(); ++i)
	// 	cout << fft_text->getVal(i) << " ";
	// cout << endl;
	
	// cout << "Paire P : ";
	// for (int i = 0; i < fft_pattern->getSize(); ++i)	
	// 	cout << fft_pattern->getVal(i) << " ";
	// cout << endl;

	ReversePattern(size_pattern, fft_pattern);
	fft_text->ExecFFT();
	fft_pattern->ExecFFT();
	end = chrono::system_clock::now();
	texec = end-mid;
	// cout << "	fft exec : " << texec.count() << "s" << endl;
	mid = end;

	// cout << "FFT T : " << endl;
	// for (int i = 0; i < fft_text->getSize(); ++i)
	// 	cout << fft_text->getReal(i) << "	" << fft_text->getIm(i) << endl;
	// cout << endl;

	// cout << "FFT P : " << endl;
	// for (int i = 0; i < fft_pattern->getSize(); ++i)
	// 	cout << fft_pattern->getReal(i) << "	" << fft_pattern->getIm(i) << endl;
	// cout << endl;


	fft_res->FFTMultiplication(fft_text, fft_pattern);
	end = chrono::system_clock::now();
	texec = end-mid;
	// cout << "	mult + iexec : " << texec.count() << "s" << endl;
	mid = end;

	// cout << "FFT Res : " << endl;
	// for (int i = 0; i < fft_res->getSize(); ++i)
	// 	cout << fft_res->getReal(i) << "	" << fft_res->getIm(i) << endl;
	// cout << endl;

	fft_res->ExecFFT();

	for (int32_t i = 0; i < size_res; ++i) {
		// sometime, the double variable is close to an integer
		// but a little inferior, the +0,5 corrects the cast into an integer
		res[i]+= (fft_res->getVal(i+size_pattern-1)+0.5);
	}

	// cout << "Tab Res : ";
	// for (int i = 0; i < fft_res->getSize(); ++i)
	// 	cout << fft_res->getVal(i) << " ";
	// cout << endl;

		end = chrono::system_clock::now();
		texec = end-mid;
		// cout << "	write res : " << texec.count() << "s" << endl;
		mid = end;
}
