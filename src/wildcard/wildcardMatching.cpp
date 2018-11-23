/* Copyright : GNU GPL V3

Author : Tatiana Rocher, tatiana.rocher@gmail.com
From the paper: "Simple deterministic wildcard matching" from Peter Clifford & Raphaël Clifford

Compilation :
g++ -std=c++11 wildcardMatching.cpp -o wcm -lfftw3 -lm

Execution :
./wcm text.in pattern.in optional.out
The pattern (text) input file must contain its lenght then the pattern (text)
*/

#include <math.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdint>
#include <ctime>

extern "C" {
	#include "../../Lib/fftw3/fftw-3.3.7/api/fftw3.h"
}

using namespace std;

// Under this threshold, we assume that the computing of doubles made approximation errors
double THRESHOLD = 0.001;
int LIMIT = 1000;  // size of the output buffer

bool Usage(int argc) {
	if (argc < 3) {
		cout << endl << "How to run: ./exec text pattern -o optionalOutput";
		cout << "-p optionalPlan" << endl;
		cout << "/!\\ The text (or pattern) input file must ";
		cout << "contain its lenght then the text (or pattern)." << endl;
		cout << "This algorithm works with text and pattern ";
		cout << "composed of integers." << endl;
		cout << "The size of the text is limited to 2^32/2 bits. ";
		cout << "If your text is longer, consider spliting it."<< endl << endl;
		return false;
	}
	return true;
}

void LoadSavedPlan(char* file) {
	int res=0;
	res = fftw_import_wisdom_from_filename(file);
	if (res != 0)
		cout << "Loading plans from " << file << " succeed."<< endl;
	else
		cout << "Error while loading plans from " << file << endl;
}

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

void FFTmultiplication(int32_t size, const fftw_complex *a,
						const fftw_complex *b, fftw_complex *res) {
	double real, im;
	for (int32_t i = 0; i < size/2+1; i++) {
		real = a[i][0] * b[i][0] - a[i][1]* b[i][1];
		im = a[i][0] * b[i][1] + a[i][1] * b[i][0];
		res[i][0] = real;
		res[i][1] = im;
	}
}

// Computes the square and cube of every coeff
void SquaredAndCubedCoeff(int32_t size, const double *text,
						double *square, double *cube) {
	for (int32_t i = 0; i < size; ++i) {
		square[i] = pow(text[i], 2);
		cube[i] = square[i]*text[i];
	}
}

// Compute the 2^power of every coeff
void PoweredTextCoeff(int32_t size, const double *text,
					double *powered, int power) {
	for (int32_t i = 0; i < size; i++)
		powered[i] = pow(text[i], power);
}


// Computes the FFT of the pattern
void FFTPattern(int32_t size_ffts, double *pattern, fftw_complex **fft_pat) {
	(*fft_pat) = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*(size_ffts/2+1));
	fftw_plan plan_pattern;
	plan_pattern = fftw_plan_dft_r2c_1d(size_ffts, pattern, (*fft_pat), FFTW_ESTIMATE);
	fftw_execute(plan_pattern);
	fftw_destroy_plan(plan_pattern);
}


// Intialise the pattern, size_pattern and size_text
void ReadPattern(string file, int32_t *size_pattern, int32_t size_text,
	int32_t size_ffts, double **pattern) {
	ifstream fileInPattern(file.c_str(), ios::in);
	if (fileInPattern) {
		int size_tmp;
		fileInPattern >> size_tmp;
		(*size_pattern) = size_tmp;
		assert(size_text >= (*size_pattern) &&
			"The text's length must be longer or equal to the pattern's. Did you invert the text and pattern calls?");

		(*pattern) = new double[size_ffts]();

		for (int i = 0; i < size_tmp; i++)
			fileInPattern >> (*pattern)[size_tmp-i-1];

		fileInPattern.close();
	}
	else
		cout << "Can't open pattern file." << endl;
}


void Computation(int32_t size_pattern, int32_t size_ffts, int32_t size_res,
				fftw_plan plan_text, fftw_plan plan_ifft, fftw_complex *fft_text,
				fftw_complex *fft_pat, fftw_complex *fft_tmp, double *res,
				double *tmp, int mult) {
	fftw_execute(plan_text);
	FFTmultiplication(size_ffts, fft_text, fft_pat, fft_tmp);
	fftw_execute(plan_ifft);

	// Add the tmp results
	for (int32_t i = 0; i < size_res; i++) {
		res[i]+= (int)round(mult*tmp[i+size_pattern-1]/size_ffts);
	}
}

void WriteOuput(int32_t size_res, double *res, ofstream &file_out) {
	string buffer;
	buffer.reserve(LIMIT);
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
	if (!Usage(argc)) return 0;

	string file_text = argv[1];
	string inPattern = argv[2];

	string out = "out.out";
	char c;
	while((c = getopt(argc, argv, "p:o:")) !=EOF) {
		switch (c) {
			case 'p':
				LoadSavedPlan(optarg);
				break;
			case 'o':
				out = optarg;
				break;
			default:
				Usage(argc);
				break;
		}
	}

	int32_t size_pattern, size_ffts, size_text;
	// size_ffts indicates the lenght a tab should have if we want
	// to do the multiplication between P and T,
	// it's a power of 2 (as advised in FFTW's doc)

	// Open file containing the text
	ifstream stream_text(file_text.c_str(), ios::in);

	if (!stream_text) {
		cout << "Can't open text file." << endl;
		return 0;
	}

	stream_text >> size_text;
	size_ffts = UpperPowOfTwo(size_text);

	// Open and read the file containing the pattern
	double *pattern;
	ReadPattern(inPattern, &size_pattern, size_text, size_ffts, &pattern);

	// Open output file
	ofstream stream_out(out.c_str(), ios::out | ios::trunc);
	if (!stream_out) {
		cout << "Can't open output file." << endl;
		return 0;
	}

	int32_t size_res = size_text - size_pattern + 1;
	double *text = new double[size_ffts]();
	double *tmp = new double[size_ffts]();
	double *res = new double[size_res]();

	// Initialise the fftw_plan. Should be done before initialising the in/output
	fftw_complex *fft_text, *fft_tmp;
	fftw_plan plan_text, plan_tmp, plan_ifft;

	fft_text = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*(size_ffts/2+1));
	fft_tmp = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*(size_ffts/2+1));

	plan_text = fftw_plan_dft_r2c_1d(size_ffts, text, fft_text, FFTW_ESTIMATE);
	plan_tmp = fftw_plan_dft_r2c_1d(size_ffts, tmp, fft_tmp, FFTW_ESTIMATE);
	plan_ifft = fftw_plan_dft_c2r_1d(size_ffts, fft_tmp, tmp, FFTW_ESTIMATE);

	// Compute the squared and cubed coeff of the pattern
	double *squared_coeff_pattern = new double[size_ffts]();
	double *cubed_coeff_pattern = new double[size_ffts]();
	SquaredAndCubedCoeff(size_ffts, pattern, squared_coeff_pattern,
						cubed_coeff_pattern);

	// Build the FFTs of: Pattern, squared_coeff_pattern and cubed_coeff_pattern
	fftw_complex *fft_pat, *fft_pat2, *fft_pat3;
	FFTPattern(size_ffts, pattern, &fft_pat);
	FFTPattern(size_ffts, squared_coeff_pattern, &fft_pat2);
	FFTPattern(size_ffts, cubed_coeff_pattern, &fft_pat3);

	for (int32_t i = 0; i < size_text; i++) {
		stream_text >> text[i];
	}

	// Sigma T*P^3
	fftw_execute(plan_text);
	FFTmultiplication(size_ffts, fft_text, fft_pat3, fft_tmp);
	fftw_execute(plan_ifft);

	// Add the tmp results
	for (int32_t i = 0; i < size_res; i++)
		res[i] = (int)round(tmp[i+size_pattern-1]/size_ffts);

	// T² * P²
	PoweredTextCoeff(size_ffts, text, tmp, 2);
	Computation(size_pattern, size_ffts, size_res, plan_tmp, plan_ifft, fft_tmp,
				fft_pat2, fft_tmp, res, tmp, -2);

	// T^3 * P
	PoweredTextCoeff(size_ffts, text, tmp, 3);
	Computation(size_pattern, size_ffts, size_res, plan_tmp, plan_ifft, fft_tmp,
				fft_pat, fft_tmp, res, tmp, 1);

    cout << "Writing results in output file: " << out << endl;
	WriteOuput(size_res, res, stream_out);

	stream_text.close();
	stream_out.close();

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
    cout << "Total : " << texec.count() << "s" << endl;

	return 0;
}
