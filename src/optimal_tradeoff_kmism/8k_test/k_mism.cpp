/* Copyright : ???
Author : Tatiana Rocher, tatiana.rocher@gmail.com

Compilation :
install the fftw3 library
make

Execution :
./opti text.in pattern.in error_max -o optionalOutput.out -p optionalPlan
The pattern/text input file must contain its lenght then the pattern/text
*/

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cassert>
#include <cstdint>
#include <chrono>


#include "HammingDistance.hpp"
#include "Rle.hpp"
#include "Tools.hpp"

using namespace std;

int LIMIT = 1048576;  // size of the output buffer
int k_nb_letters = 128;


bool Usage() {
	cout << endl << "How to run: ./exec text pattern error optionalOutput ";
	cout << "-p optinalPlan" << endl;
	cout << "/!\\ The text (or pattern) input file must ";
	cout << "contain its lenght first, then the text (or pattern)." << endl;
	cout << "The size of the text is limited to 2^32/2 bits. ";
	cout << "If your text is longer, consider spliting it."<< endl;
	cout << "Be carefull to have only ascii characters ";
	cout << "in the text and pattern" << endl;
	cout << "Characters \'$\' and \'#\' are used in the algorithm. ";
	cout << "Differencies may occur if they are present in your text." << endl;
	cout << endl;
}

void LoadSavedPlan(char* file) {
	int res = 0;
	res = fftw_import_wisdom_from_filename(file);
	if (res != 0)
		cout << "Loading plans from " << file << " succeed."<< endl;
	else
		cout << "Error while loading plans from " << file << endl;
}

// Intialise the pattern or text, size_pattern and size_text  from a file
void ReadFile(string file, int32_t *size_pattern, char **pattern) {
	ifstream stream_file(file.c_str(), ios::in);
	char character;

	if (stream_file) {
		int size_tmp;
		stream_file >> size_tmp;
		(*size_pattern) = size_tmp;

		stream_file.get(character);  // eliminate the \n character
		(*pattern) = new char[size_tmp]();
		for (int32_t i = 0; i < size_tmp; ++i) {
			stream_file.get(character);
			(*pattern)[i] = character;
		}

		stream_file.close();
	}
	else
		cout << "Can't open file." << endl;
}


int main(int argc, char* argv[]) {
	if (argc < 4) {
		Usage();
		return 0;
	}

	string file_text = argv[1];
	string file_pattern = argv[2];
	int error_k = atoi(argv[3]);
	string file_out = "out.out";

	char c;
	while((c = getopt(argc, argv, "p:o:")) !=EOF) {
		switch (c) {
			case 'p':
				LoadSavedPlan(optarg);
				break;
			case 'o':
				file_out = optarg;
				break;
			default:
				Usage();
				break;
		}
	}

    int32_t size_pattern, size_text;
	char *pattern, *text;

	// Open and read the files containing the text and pattern
	ReadFile(file_text, &size_text, &text);
	ReadFile(file_pattern, &size_pattern, &pattern);
	assert(size_text >= size_pattern &&
			"The text's length must be longer or equal to the pattern's. Did you invert the text and pattern calls?");

    // Open output file
	ofstream stream_out(file_out.c_str(), ios::out | ios::trunc);
	if (!stream_out) {
		cout << "Can't open output file." << endl;
		return 0;
	}

	int *appr_res = new int[error_k];

    // Search for an approximate period
	FindApproximatePeriod(size_pattern, pattern,
						k_nb_letters, 1, error_k, 8*error_k, appr_res);
	int approx_period = 0;
	int32_t i_l, i_r;


	int naif_res;
	int32_t min = size_text, max = 0;
	for (int i = 0; i < size_text - size_pattern; ++i) {
		naif_res = 0;
		for (int j = 0; j < size_pattern && naif_res <= error_k; ++j) {
			if (pattern[j] != text[i + j]) {
				naif_res++;
			}
		}
		if (naif_res <= error_k) {
			if (min > i)
				min = i;
			max = i;
		}
	}
	cout << "min : " << min << "	max : " << max << endl;


	for (int i = 8; i >= 1; --i) {

		for (int j = 1; j < error_k; ++j) {
			if (appr_res[j] < error_k * i) {
				approx_period = j;
				break;
			}
		}

		for (int j= 11; j >= 1; --j) {
			if (approx_period != 0) {  // There is a 8k-period <= k, case 2 in the paper
				Small8kPeriod(size_text, text, size_pattern, error_k, 
								approx_period, &i_l, &i_r, j);
			}

			if ((min < i_l) || (max > i_r)) {
				cout << "fouchette i = " << i << "	j = " << j << endl; 
			cout << "min = " << min << " i_l = " << i_l; 
			cout << "	max = " << max << " i_r = " << i_r << endl;
				cout << "Fourchette trop petite----------------------------" << endl << endl;
				break;
			}
		}
	}

    delete [] text;
    delete [] pattern;

    return 0;
}
