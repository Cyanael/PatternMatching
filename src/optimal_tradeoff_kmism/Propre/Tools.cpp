/* Copyright : GNU GPL V3
Author : Tatiana Rocher, tatiana.rocher@gmail.com

This file contains some functions used by LCP.cpp
and HammingDistance.cpp
*/

#include <fstream>

#include "Tools.hpp"

int CharToInt(char letter) {
    return (int) letter;
}

char IntToChar(int val) {
    return (char) val;
}

int UpperPowOfTwo(int val) {
	val--;
	val |= val >> 1;
	val |= val >> 2;
	val |= val >> 4;
	val |= val >> 8;
	val |= val >> 16;
	val++;
	return val;
}

void InitTabZeros(int32_t size, int32_t *tab) {
	for (int32_t i = 0; i < size; i++)
		tab[i] = 0;
}

// Initialise a table from a file
// Used for the prime number table
void ReadIntFile(string file, int *size, int **table) {
ifstream stream_file(file.c_str(), ios::in);
	if (stream_file) {
		int size_tmp;
		stream_file >> size_tmp;
		(*size) = size_tmp;

		(*table) = new int[size_tmp]();
		for (int32_t i = 0; i < size_tmp; ++i)
			stream_file >> (*table)[i];
		stream_file.close();
	}
	else
		cout << endl <<"	/!\\ Can't open prime numbers file." << endl << endl;
}
