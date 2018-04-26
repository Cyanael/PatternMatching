
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fstream>
#include <math.h>
#include <ctime>
#include <chrono>

extern "C"{
	#include "../Lib/fftw3/fftw-3.3.7/api/fftw3.h"
}

using namespace std;

int main(int argc, char* argv[]) {
	if (argc < 2)
		cout << "Usage: ./exec textFile" << endl;

	string file_text = argv[1];
	ifstream stream_text(file_text.c_str(), ios::in);
	if (!stream_text) {
		cout << "Can't open text file." << endl;
		return 0;
	}

	int N;
	stream_text >> N;
	fftw_complex *text, *fft_text;
	fftw_plan my_plan;

	text = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
	fft_text = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
	my_plan = fftw_plan_dft_1d(N, text, fft_text, FFTW_FORWARD, FFTW_ESTIMATE);

	for (int i=0; i<N; i++)
		stream_text >> text[i][0];


	chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> texec;
    start = chrono::system_clock::now();

	fftw_execute(my_plan);


	end = chrono::system_clock::now();
    texec = end-start;
    cout << "Total time : " << texec.count() << "s" << endl;

	fftw_destroy_plan(my_plan);
	fftw_free(text);
	fftw_free(fft_text);
	return 0;
}
