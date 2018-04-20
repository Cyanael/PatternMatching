/* Copyright: ??
Author: Tatiana Rocher tatiana.rocher@gmail.com

This file is used to make and save FFTs plans.
Loading the output file later will help and algorithm compute faster.
You need to enter the min and max powers of 2 you will need.
*/

#include <iostream>
#include <cmath>
#include <chrono>

extern "C" {
	#include "../../Lib/fftw3/fftw-3.3.7/api/fftw3.h"
}


using namespace std;

int main(int argc, char* argv[]) {
	if (argc < 4) {
		cout << "Usage: ./exec min max savingFile opt_flag" << endl;
		cout << "The program computes the FFT and iFFT plans for tables of size ";
		cout << " pow(2, min) to pow(2, max)." << endl;
		cout << "You have 3 possible flags for the plan: ";
		cout << "0 = MEASURE, 1 = PATIENT, 2 = EXHAUSTIVE." << endl;
		cout << "Caution: For max > 16, this program needs some time, ";
		cout << "have a look on the documentation for more information." << endl;
		return 0;
	}

	int min = atoi(argv[1]);
	int max = atoi(argv[2]);

	unsigned flag = FFTW_MEASURE;
	if (argc > 4){
		int f = atoi(argv[4]);
		if (f == 0)
			flag = FFTW_MEASURE;
		else if (f == 1)
			flag = FFTW_PATIENT;
		else if (f == 2)
			flag = FFTW_EXHAUSTIVE;
		else {
			cout << "I did not understand the flag you want, please use:";
			cout << "0 for MEASURE, 1 for PATIENT, 2 for EXHAUSTIVE." << endl;
			return 0;
		}
	}

    int ret, size;
	fftw_plan plan;
	fftw_complex *fft_res = NULL;
	double *table = NULL;

	chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> texec;
    start = chrono::system_clock::now();

    for (int i = min; i <= max; ++i) {
        size = pow(2, i);

        table = (double *) malloc(size*sizeof(double));
        fft_res = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*(size/2+1));

        plan = fftw_plan_dft_r2c_1d(size, table, fft_res, flag);
        plan = fftw_plan_dft_c2r_1d(size, fft_res, table, flag);
    }
	ret = fftw_export_wisdom_to_filename(argv[3]);

	end = chrono::system_clock::now();
    texec = end-start;
    cout << "Total time : " << texec.count() << "s" << endl;

	cout << "size : " << size << endl;
	if (ret != 0)
        cout << "Saving plans in " << argv[3] << " succed."<< endl;
    else
        cout << "Error while saving in " << argv[3] << endl;

		delete [] table;
    	fftw_free(fft_res);
    	fftw_destroy_plan(plan);
}
