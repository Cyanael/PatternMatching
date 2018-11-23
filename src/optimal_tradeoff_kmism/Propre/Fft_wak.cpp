/* Copyright : GNU GPL V3
Author : Tatiana Rocher, tatiana.rocher@gmail.com
*/

#include <math.h>
#include <iostream>

#include "Fft_wak.hpp"


FFT_wak::FFT_wak(unsigned int size) {
	size_ = size;
	polynome_ = new double[size];
	fft_str_ = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*(size/2+1));
	plan_ = fftw_plan_dft_r2c_1d(size_, polynome_, fft_str_, FFTW_ESTIMATE);
	signe_ = true;
}


FFT_wak::FFT_wak(unsigned int size, bool s) {
	size_ = size;
	signe_ = s;
	polynome_ = new double[size];
	fft_str_ = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*(size/2+1));
	if (s == true)
		plan_ = fftw_plan_dft_r2c_1d(size_, polynome_, fft_str_, FFTW_ESTIMATE);
	else
		plan_ = fftw_plan_dft_c2r_1d(size_, fft_str_, polynome_, FFTW_ESTIMATE);
}


FFT_wak::~FFT_wak() {
	delete [] polynome_;
	fftw_free(fft_str_);
	fftw_destroy_plan(plan_);
}


void FFT_wak::MatchLetters(char letter, int32_t size, char *text) {
	for (int32_t i = 0; i < size; ++i) {
		if(text[i] == letter)
			polynome_[i] = 1;
		else
			polynome_[i] = 0;
	}
	if (size < size_) {
		for (int32_t i = size; i < size_; ++i)
			polynome_[i] = 0;
	}
}

void FFT_wak::ReversePolynome(int32_t size) {
	double tmp;
	for (int32_t i = 0; i < size/2; ++i) {
		tmp = polynome_[i];
		polynome_[i] = polynome_[size-i-1];
		polynome_[size-i-1] = tmp;
	}
}


/*****************************/

void FFT_wak::FFTMultiplication(FFT_wak *a, FFT_wak *b) {
	for (unsigned int i = 0; i < size_/2+1; i++) {
		fft_str_[i][0] = a->getReal(i) * b->getReal(i)
							- a->getIm(i)* b->getIm(i);
		fft_str_[i][1] = a->getReal(i) * b->getIm(i)
							+ a->getIm(i) * b->getReal(i);
	}
}


void FFT_wak::Rounding() {
	double threshold = 0.001;
	// Under this threshold, we assume that the computing made approximation errors
	for (unsigned int i = 0; i < size_; i++) {
		if (fabs(polynome_[i]) < threshold)
			polynome_[i] = 0;
	}
}


void FFT_wak::ExecFFT() {
	fftw_execute(plan_);
	if (!signe_) {
		for (unsigned int i = 0; i < size_; i++)
			polynome_[i] = polynome_[i]/size_;
	}
	this->Rounding();
}



/************* Getter / Setter ***************/

unsigned int FFT_wak::getSize() {
	return size_;
}

double FFT_wak::getVal(unsigned int i) const {
	return polynome_[i];
}

void FFT_wak::setVal(unsigned int i, double val) {
	polynome_[i] = val;
}

double FFT_wak::getReal(unsigned int i) const {
	return fft_str_[i][0];
}

double FFT_wak::getIm(unsigned int i) const {
	return fft_str_[i][1];
}

void FFT_wak::setReal(unsigned int i, double val) {
	fft_str_[i][0] = val;
}

void FFT_wak::setIm(unsigned int i, double val) {
	fft_str_[i][1] = val;
}


/********* Prints **********/

void FFT_wak::strToString() const {
	cout << "[";
	for (unsigned int i = 0; i < size_-1; i++)
		cout << polynome_[i] << ", ";
	cout << polynome_[size_-1] << "]"<< endl;
}

void FFT_wak::fftToString() const {
	for (unsigned int i = 0; i < size_/2+1; i++)
		cout << fft_str_[i][0] << "	" << fft_str_[i][1] << endl;
}
