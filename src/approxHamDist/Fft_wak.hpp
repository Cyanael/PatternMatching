/* Copyright : ???
Author : Tatiana Rocher, tatiana.rocher@gmail.com

Compilation :
install the fftw3 library
g++ -std=c++11 hamDist.cpp Fft_wak.cpp -o hd -lfftw3 -lm

Execution :
./hd text.in pattern.in optional.out
The pattern/text input file must contain its lenght then the pattern/text
*/

#ifndef FFT_WAK_H
#define FFT_WAK_H


extern "C" {
	#include "../../Lib/fftw3/fftw-3.3.7/api/fftw3.h"
}

using std::cout;
using std::endl;


class FFT_wak{
 private:
	unsigned int size_;
	double *polynome_;
	fftw_complex *fft_str_;
	fftw_plan plan_;
	// used to make the plan:
	// true if we make the fft from the polynome
	// false if we make the polynome from the fft
	bool signe_;

 public:
	explicit FFT_wak(unsigned int size);
	FFT_wak(unsigned int size, bool signe);
	~FFT_wak();

	void ExecFFT();
	void FFTMultiplication(FFT_wak *a, FFT_wak *b);
	void Rounding();

	unsigned int getSize();
	double getVal(unsigned int i) const;
	void setVal(unsigned int i, double val);
	double getReal(unsigned int i) const;
	double getIm(unsigned int i) const;
	void setReal(unsigned int i, double val);
	void setIm(unsigned int i, double val);

	void strToString() const;
	void fftToString() const;
};

#endif  // FFT_WAK_K
