#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <cstdint>
#include <iostream>

using namespace std;


int CharToInt(char letter);
char IntToChar(int val);
int UpperPowOfTwo(int val);
void InitTabZeros(int32_t size, int *table);
void ReadIntFile(string file, int *size, int **table);

#endif  // TOOLS_HPP
