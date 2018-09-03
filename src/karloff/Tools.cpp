/* Copyright : ???
Author : Tatiana Rocher, tatiana.rocher@gmail.com

This file contains some functions used by LCP.cpp 
and HammingDistance.cpp
*/
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

template<typename T> void InitTabZeros(int32_t size, T *tab) {
	for (int32_t i = 0; i < size; i++)
		tab[i] = 0;
}

void InitTabZeros(int32_t size, int *tab) {
	for (int32_t i = 0; i < size; i++)
		tab[i] = 0;
}

void PrintTable(int32_t size, int *table) {
	for (int32_t i = 0; i < size; ++i)
		cout << table[i] << " ";
	cout << endl;
}
