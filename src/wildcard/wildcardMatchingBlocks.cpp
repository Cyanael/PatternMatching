/* Author : Tatiana Rocher, tatiana.rocher@gmail.com
From the paper: "Simple deterministic wildcard matching" from Peter Clifford & Raphaël Clifford
*/


/*
Compilation :
g++ -std=c++11 wildcardMatching.cpp -o wcm -lfftw3 -lm

Execution :
./wcm text.in pattern.in optional.out
The pattern/text input file must contain its lenght then the pattern/text
*/


#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <ctime>
#include <chrono>

extern "C"{
	#include "../fftw-3.3.7/api/fftw3.h"
}

using namespace std;


double THRESHOLD = 0.001;
int LIMIT = 1000; // size of the output buffer

bool usage(int argc){
	if (argc < 3){
		cout << "How to run: ./exec text pattern optionalOutput" << endl;
		cout << "/!\\ The pattern/text input file must contain its lenght then the pattern/text" << endl;
		return false;
	}
	return true;
}

int UpperPowOfTwo(int val){
	val--;
	val |= val >> 1;
	val |= val >> 2;
	val |= val >> 4;
	val |= val >> 8;
	val |= val >> 16;
	val++;
	return val;
}

void rounding(unsigned int size, double *res){
	for (unsigned int i=0; i<size; i++){
		if (fabs(res[i])< THRESHOLD)
			res[i] = 0;
	}
}

void FFTmultiplication (unsigned int size, fftw_complex *a, fftw_complex *b, fftw_complex *res){
	double real, im;
	for (unsigned int i=0; i<size/2+1; i++){
		real = a[i][0] * b[i][0] - a[i][1]* b[i][1];
		im = a[i][0] * b[i][1] + a[i][1] * b[i][0];
		res[i][0] = real;
		res[i][1] = im;
	}
}

// Computes the square and cube of every coeff
void squaredAndCubedCoeff(unsigned int size, double *text, double *square, double *cube){
	for (unsigned int i=0; i<size; i++){
		square[i] = pow(text[i], 2);
		cube[i] = square[i]*text[i];
	}
}

// Compute the 2^power of every coeff
void poweredTextCoeff(unsigned int size, double *text, double *powered, int power){
	for (unsigned int i=0; i<size; i++)
		powered[i] = pow(text[i], power);
}

// Computes the FFT of the pattern
void FFTPattern(unsigned int size_fft, double *pattern, fftw_complex **fft_pat){
	(*fft_pat) = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*(size_fft/2+1));
	fftw_plan fftP;
	fftP = fftw_plan_dft_r2c_1d(size_fft, pattern, (*fft_pat), FFTW_ESTIMATE);
	fftw_execute(fftP);

	fftw_destroy_plan(fftP);
}

//intialise the pattern, size_pattern and size_text
void readPattern(string file, unsigned int *size_pattern, double **pattern, unsigned int *size_fft){
	ifstream stream_pattern(file.c_str(), ios::in);

	if (stream_pattern){
		int size_tmp;
		stream_pattern >> size_tmp;
		(*size_pattern) = size_tmp;
		(*size_fft) = UpperPowOfTwo(2 * size_tmp);

		// (*pattern) = (double *) malloc((*size_fft)*sizeof(double));
		(*pattern) = new double[(*size_fft)]();

		for (int i=0; i<size_tmp ;i++)
			stream_pattern >> (*pattern)[size_tmp-i-1];

		stream_pattern.close();
	}
	else
		cout << "Can't open pattern file." << endl;
}


void computation (unsigned int size_pattern, unsigned int size_fft, fftw_plan plan_text,
					fftw_plan plan_ifft, fftw_complex *fft_text, fftw_complex *fft_pat,
					fftw_complex *fft_tmp, double *res, double *tmp, int mult){

	fftw_execute(plan_text);
	FFTmultiplication(size_fft, fft_text, fft_pat, fft_tmp);
	fftw_execute(plan_ifft);


	//add the tmp results
	for (unsigned int i=0; i<size_pattern; i++){
		res[i]+= (int)round(mult*tmp[i+size_pattern-1]/size_fft);
	}
}

void WriteOuput(int32_t size_res, double *res, string buffer, ofstream &file_out) {
	string res_i_str;
	for (int32_t i = 0; i < size_res; ++i) {
    	res_i_str = to_string((int)res[i]);
    	if (buffer.length() + res_i_str.length() + 1 >= LIMIT) {
        	file_out << buffer;
        	buffer.resize(0);
    	}
    	buffer.append(res_i_str);
    	buffer.append("\n");
		}
	file_out << buffer;
}



int main(int argc, char* argv[]) {
	chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> texec;
    start = chrono::system_clock::now();

	if (!usage(argc)) return 0;

	unsigned int size_pattern, size_fft, size_text;
	/* size_pattern indicates the lenght of the pattern
	size_text is the number of text's letters we still have to read/analyse
	size_fft indicates the lenght a table should have if we want
	to do the multiplication between P an a bloc of T of lenght 2*size_pattern,
	it's a power of 2, as advised in FFTW's doc.
	*/
	double *pattern;

	// Open and read the file containing the pattern
	string file_pattern = argv[2];
	readPattern(file_pattern, &size_pattern, &pattern, &size_fft);

	// Open file containing the text
	string file_text = argv[1];
	ifstream stream_text(file_text.c_str(), ios::in);

	if (!stream_text){
		cout << "Can't open text file." << endl;
		return 0;
	}

	stream_text >> size_text;

	// Open output file
	string out;
	if (argc<4)
		out = "out.out";
	else
		out = argv[3];

	ofstream stream_output(out.c_str(), ios::out | ios::trunc);
	if (!stream_output){
		cout << "Can't open output file." << endl;
		return 0;
	}

	double *text = new double[size_fft]();
	double *tmp = new double[size_fft]();
	double *res = new double[size_pattern]();

	//Initialise the fftw_plan. Should be done before initialising the in/output
	fftw_complex *fft_text, *fft_tmp;
	fftw_plan plan_text, plan_tmp, plan_ifft;

	fft_text = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*(size_fft/2+1));
	fft_tmp = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*(size_fft/2+1));

	plan_text = fftw_plan_dft_r2c_1d(size_fft, text, fft_text, FFTW_ESTIMATE);
	plan_tmp = fftw_plan_dft_r2c_1d(size_fft, tmp, fft_tmp, FFTW_ESTIMATE);
	plan_ifft = fftw_plan_dft_c2r_1d(size_fft, fft_tmp, tmp, FFTW_ESTIMATE);


	//Begin computation
	//Compute the squared and cubed coeff of the pattern
	double *squared_coeff_pattern = new double[size_fft]();
	double *cubed_coeff_pattern = new double[size_fft]();
	squaredAndCubedCoeff(size_fft, pattern, squared_coeff_pattern, cubed_coeff_pattern);


	// Build the FFTs of: Pattern, squaredPattern and cubedPattern
	fftw_complex *fft_pat, *fft_pat2, *fft_pat3;
	FFTPattern(size_fft, pattern, &fft_pat);
	FFTPattern(size_fft, squared_coeff_pattern, &fft_pat2);
	FFTPattern(size_fft, cubed_coeff_pattern, &fft_pat3);

	// Read size_pattern letters of the text
	for (unsigned int i=0; i<size_pattern; i++){
		stream_text >> text[i];
	}
	size_text -= size_pattern;

	string buffer; // to write in output with less memory access than size_text
	buffer.reserve(LIMIT);

	while(size_text >= size_pattern){

		//Read size_pattern letters of the text
		for (unsigned int i=0; i < size_pattern; i++){
			stream_text >> text[i+size_pattern];
		}

		// Sigma T*P^3
		fftw_execute(plan_text);
		FFTmultiplication(size_fft, fft_text, fft_pat3, fft_tmp);
		fftw_execute(plan_ifft);

		// for (unsigned int i=0; i<size_fft; i++){
		// 	tmp[i] = tmp[i]/size_fft;
		// }
		// rounding(size_fft, tmp);

		//add the tmp results
		for (unsigned int i=0; i<size_pattern; i++){
			res[i] = (int)round(tmp[i+size_pattern-1]/size_fft);
		}

		//T²
		poweredTextCoeff(size_fft, text, tmp, 2);
		computation (size_pattern, size_fft, plan_tmp, plan_ifft, fft_tmp, fft_pat2, fft_tmp, res, tmp, -2);

		//T^3
		poweredTextCoeff(size_fft, text, tmp, 3);
		computation (size_pattern, size_fft, plan_tmp, plan_ifft, fft_tmp, fft_pat, fft_tmp, res, tmp, 1);

		rounding(size_pattern, res);

		//Write in output file
		//for (unsigned int i=0; i<size_pattern; ++i)
		//	stream_output << res[i] << endl;
		WriteOuput(size_pattern, res, buffer, stream_output);

		// Slide of m+1 to 2m text's letter to the 0 to m positions
		size_text -= size_pattern;
		for (unsigned int i = 0; i < size_pattern; ++i){
			text[i] = text[i+size_pattern];
		}

		}

	if (size_text > 0){
		for (unsigned int i = 0; i < size_text; ++i)
			stream_text >> text[i+size_pattern];
		for (unsigned int i = size_text; i < size_pattern; ++i)
			text[i+size_pattern] = 0;

		fftw_execute(plan_text);
		FFTmultiplication(size_fft, fft_text, fft_pat3, fft_tmp);
		fftw_execute(plan_ifft);
		rounding(size_fft, tmp);

		for (unsigned int i = 0; i < size_fft; ++i){
			tmp[i] = tmp[i]/size_fft;
		}

		//add the tmp results
		for (unsigned int i = 0; i < size_fft; ++i){
			res[i]= tmp[i+size_pattern-1];
		}

		poweredTextCoeff(size_fft, text, tmp, 2);  //T : coeff²
		computation (size_pattern, size_fft, plan_tmp, plan_ifft, fft_tmp, fft_pat2, fft_tmp, res, tmp, -2);

		poweredTextCoeff(size_fft, text, tmp, 3);  //T : coeff^3
		computation (size_pattern, size_fft, plan_tmp, plan_ifft, fft_tmp, fft_pat, fft_tmp, res, tmp, 1);

		for (unsigned int i = 0; i < size_text+1; ++i)
			stream_output << res[i] << endl;
		WriteOuput(size_pattern, res, buffer, stream_output);
	}

	stream_text.close();
	stream_output.close();

	delete [] pattern;
	delete [] text;
	delete [] tmp;
	delete [] res;
	delete [] squared_coeff_pattern;
	delete [] cubed_coeff_pattern;
	
	fftw_free(fft_pat);
	fftw_free(fft_pat2);
	fftw_free(fft_pat3);
	fftw_free(fft_text);
	fftw_free(fft_tmp);

	fftw_destroy_plan(plan_text);
	fftw_destroy_plan(plan_tmp);
	fftw_destroy_plan(plan_ifft);


	end = chrono::system_clock::now();
  texec = end-start;
  cout << "Total time : " << texec.count() << "s" << endl;

	return 0;
}
