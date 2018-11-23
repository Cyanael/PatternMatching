/* Copyright: GNU GPL V3
Author: Tatiana Rocher tatiana.rocher@gmail.com

This file is used to make and save FFTs plans.
Loading the output file later will help and algorithm compute faster.
You need to enter the min and max powers of 2 you will need.
*/

#include <iostream>
#include <cmath>
#include <chrono>
#include <sstream>

extern "C" {
	#include "../../Lib/fftw3/fftw-3.3.7/api/fftw3.h"
}


using namespace std;

int main(int argc, char* argv[]) {
	if (argc < 3) {
		cout << "Usage: ./exec min max optional_flag" << endl;
		cout << "The program computes the FFT and iFFT plans for tables of size ";
		cout << " pow(2, min) to pow(2, max)." << endl;
		cout << "You have 3 possible flags plan: ";
		cout << "0 = ESTIMATE, 1 = MEASURE, 2 = PATIENT, 3 = EXHAUSTIVE." << endl;
		cout << "As the computation of plan can be long, every time a new plan";
		cout << "is computed, we save all the generated plans in a new file.";
		cout << "This allow you to stop the execution at any time." << endl;
		cout << "Caution: For max > 16, this program needs some time, ";
		cout << "have a look on the documentation for more information." << endl;
		return 0;
	}

	int min = atoi(argv[1]);
	int max = atoi(argv[2]);

	unsigned flag = FFTW_ESTIMATE;
	if (argc >=4) {
		int f = atoi(argv[3]);
		if (f == 1)
			flag = FFTW_MEASURE;
		else if (f == 2)
			flag = FFTW_PATIENT;
		else if (f == 3)
			flag = FFTW_EXHAUSTIVE;
		else {
			cout << "I did not understand the flag you want, please use:";
			cout << "0 for ESTIMATE, 1 for MEASURE, 2 for PATIENT, 3 for EXHAUSTIVE." << endl;
			return 0;
		}
	}

    int ret, size;
	fftw_plan plan;
	fftw_complex *fft_res = NULL;
	double *table = NULL;


    for (int i = min; i <= max; ++i) {
		stringstream name_file;
		name_file << min << "_to_" << i;
		switch (flag) {
			case FFTW_ESTIMATE:
				name_file << "_estimate.plan";
				break;
			case FFTW_MEASURE:
				name_file << "_measure.plan";
				break;
			case FFTW_PATIENT:
				name_file << "_patient.plan";
				break;
			case FFTW_EXHAUSTIVE:
				name_file << "_exhaustive.plan";
				break;
		}

		chrono::time_point<chrono::system_clock> start, end;
		chrono::duration<double> texec;
		start = chrono::system_clock::now();
        size = pow(2, i);

        table = (double *) malloc(size*sizeof(double));
        fft_res = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*(size/2+1));

        plan = fftw_plan_dft_r2c_1d(size, table, fft_res, flag);

		end = chrono::system_clock::now();
		texec = end-start;
		cout << i <<" FFT time : " << texec.count() << "s" << endl;
		start = chrono::system_clock::now();

		plan = fftw_plan_dft_c2r_1d(size, fft_res, table, flag);

		end = chrono::system_clock::now();
		texec = end-start;
		cout << i << " IFFT Time : " << texec.count() << "s" << endl << endl;
		ret = fftw_export_wisdom_to_filename(name_file.str().c_str());

		cout << "write FFTs in file " << name_file.str() << endl;
    }

		delete [] table;
    	fftw_free(fft_res);
    	fftw_destroy_plan(plan);
}
