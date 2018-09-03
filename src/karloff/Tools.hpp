#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <cstdint>
#include <iostream>

using namespace std;

// int k_nb_letters = 128;


int CharToInt(char letter);
char IntToChar(int val);
int UpperPowOfTwo(int val);
template<typename T> void InitTabZeros(int32_t size, T *tab);
void InitTabZeros(int32_t size, int *tab);
void PrintTable(int32_t size, int *table);

#endif  // TOOLS_HPP
