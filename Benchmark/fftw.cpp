/*
Author: Tatiana Rocher tatiana.rocher@gmail.com

Test the FFT execution of FFTW library,
using the plan fftw_plan_dft_r2c_1d 
Only the FFT's creation, from the double tab to the real tab, is considered
*/


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fstream>
#include <math.h>
#include <ctime> 
#include <chrono>

extern "C"{	
	#include "../fftw-3.3.7/api/fftw3.h"
}	 

using namespace std;

int UpperPowOfTwo(int32_t val) {
	val--;
	val |= val >> 1;
	val |= val >> 2;
	val |= val >> 4;
	val |= val >> 8;
	val |= val >> 16;
	val++;
	return val;
}

int main(int argc, char* argv[]) {
	if (argc < 2)
		cout << "Usage : ./exec textFile" << endl;

	string file_text = argv[1];
	ifstream stream_text(file_text.c_str(), ios::in);

	if (!stream_text) {
		cout << "Can't open text file." << endl;
		return 0;
	}

	int size_text, size_ffts;
	stream_text >> size_text;
	size_ffts = UpperPowOfTwo(size_text);
	
	double *text = new double[size_ffts]();

	fftw_complex *fft_text;
	fftw_plan plan_text;
	fft_text = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*(size_ffts/2+1));
	plan_text = fftw_plan_dft_r2c_1d(size_ffts, text, fft_text, FFTW_ESTIMATE);


	for (int32_t i = 0; i < size_text; i++) {
		stream_text >> text[i];
	}

	chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> texec;
    start = chrono::system_clock::now();

	fftw_execute(plan_text);

	end = chrono::system_clock::now();
    texec = end-start;
    cout << "Total time : " << texec.count() << "s" << endl;

return 0;
}