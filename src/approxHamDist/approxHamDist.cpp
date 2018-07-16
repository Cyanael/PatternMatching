/* Copyright : ???
Author : Tatiana Rocher, tatiana.rocher@gmail.com

Compilation :
install the fftw3 library
g++ -std=c++11 approxHamDist.cpp Fft_wak.cpp -o ahd -lfftw3 -lm

Execution :
./hd text.in pattern.in errorDistance -o optional.out -p plan -c constante_loop
The pattern/text input file must contain its lenght then the pattern/text
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cassert>
#include <cstdint>
#include <unistd.h>

extern "C" {
	#include "../../Lib/fftw3/fftw-3.3.7/api/fftw3.h"
}

#include "Fft_wak.hpp"

using namespace std;

int k_nb_letters = 128;
int LIMIT = 524288;  // size of the output buffer

void Usage() {
	cout << endl << "How to run: ./exec text pattern error_distance [-o optional_output] ";
	cout << "[-p plan_to_load] [-c constante_loop] [-x]" << endl;
	cout << "The -x option use the Hamming distance algorithm running in ";
	cout << "O(Sigma n log n)." << endl;
	cout << "/!\\ The text (or pattern) input file must ";
	cout << "contain its lenght first, then the text (or pattern)." << endl;
	cout << "The size of the text is limited to 2^32/2 bits. ";
	cout << "If your text is longer, consider spliting it."<< endl;
	cout << "Be carefull to have only ascii characters ";
	cout << "in the text and pattern or add your own mapping." << endl;
	cout << endl;

}

void LoadSavedPlan(char* file) {
	int res=0;

	res = fftw_import_wisdom_from_filename(file);
	if (res != 0)
		cout << "Loading plans from " << file << " succeed."<< endl;
	else
		cout << "Error while loading plans from " << file << endl;
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

int CharToInt(char letter) {
	return (int) letter;
}

char IntToChar(int val) {
	return (char) val;
}


void MapLetters(int size_alphabet, int size_prime_number, int *prime_numbers, int *map) {
	struct timespec ts;
	if (timespec_get(&ts, TIME_UTC) == 0) {
		cout << "error seed." << endl;
	}
	srandom(ts.tv_nsec ^ ts.tv_sec); // compute seed

	int p = 0;
	int r;
	while(p<=size_alphabet){
		r = random()%size_prime_number;
		p = prime_numbers[r];
	}
	int a = random()%p;
	int b = random()%p;

	for (int i = 0; i < k_nb_letters; ++i)
		map[i] = ((a*i + b) % p) % size_alphabet;
}

char CharMap(char letter, int *map) {
    return IntToChar(map[CharToInt(letter)]);
}

// Initialise a table from a file
// Used for the pattern
void ReadCharFile(string file, int32_t *size_pattern, char **pattern) {
	ifstream file_pattern(file.c_str(), ios::in);
	char character;

	if (file_pattern) {
		int size_tmp;
		file_pattern >> size_tmp;
		(*size_pattern) = size_tmp;

		file_pattern.get(character);  // eliminate the \n character
		(*pattern) = new char[size_tmp]();
		for (int32_t i = 0; i < size_tmp; ++i) {
			file_pattern.get(character);
			(*pattern)[i] = character;
		}

		file_pattern.close();
	}
	else
		cout << "Can't open pattern file." << endl;
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
		cout << "Can't open prime numbers file." << endl;
}

bool SortVectorOfPair(pair<char, int> a, pair<char, int> b) {
	return a.first < b.first;
}

void SortfreqInfreqCaract(int32_t size_pattern, char *pattern, float limit,
						int *map, int size_alphabet, vector<char> *freq,
                        vector<int32_t> *infreq) {
    freq->clear();
    infreq->clear();
	for (int32_t i = 0; i < size_pattern; ++i) {
        int current_val = map[CharToInt(pattern[i])];
		if (infreq[current_val].size() == 0) {
			vector<int32_t> v = {i};
			infreq[current_val] = v;
		}
		else {
			infreq[current_val].push_back(i);
		}
	}
	for (int32_t i = 0; i < size_alphabet; ++i) {
		if (infreq[i].size() >= limit) {
			freq->push_back(IntToChar(i));
			infreq[i].clear();
		}
	}
}

void SortFreqCaract(int32_t size_pattern, char *pattern, float limit,
						int *map, int size_alphabet, vector<char> *freq) {
    freq->clear();
    int *infreq = new int[k_nb_letters]();
	for (int32_t i = 0; i < size_pattern; ++i) {
        int current_val = map[CharToInt(pattern[i])];
		if (infreq[current_val] == 0) {
			infreq[current_val] = 1;
			freq->push_back(current_val);
		}
	}
	delete [] infreq;
}

bool IsInfreq(char letter, vector<int32_t> *infreq) {
	if (infreq[CharToInt(letter)].size() > 0)
		return true;
	return false;
}

// Computes the boolean vector of the text
// when there is a 1 where the matching letter is "letter"
void MatchLetterText(int32_t size, char *text, char letter,
					int *map, FFT_wak *fft_text) {
	for (int32_t i = 0; i < size; ++i) {
		if(CharMap(text[i], map) == letter)
			fft_text->setVal(i, 1);
		else
			fft_text->setVal(i, 0);
	}
	if (size < fft_text->getSize()) {
		for (int32_t i = size; i < fft_text->getSize(); ++i)
			fft_text->setVal(i, 0);
	}
}


void ReversePattern(int32_t size, FFT_wak *fft_pattern) {
	double tmp;
	for (int32_t i = 0; i < size/2; ++i) {
		tmp = fft_pattern->getVal(i);
		fft_pattern->setVal(i, fft_pattern->getVal(size-i-1));
		fft_pattern->setVal(size-i-1, tmp);
	}
}


void ComputeFreq(int32_t size_pattern, int32_t size_text, int32_t size_res,
				char *text, char *pattern, vector<char> *frequent, int *map,
				FFT_wak *fft_text, FFT_wak *fft_pattern, FFT_wak *fft_res,
				int *res) {
	InitTabZeros(size_res, res);

	char current_char;
	for (auto j = frequent->begin(); j != frequent->end(); ++j) {
		current_char = *j;
		MatchLetterText(size_text, text, current_char, map, fft_text);
		MatchLetterText(size_pattern, pattern, current_char, map, fft_pattern);

		ReversePattern(size_pattern, fft_pattern);
		fft_text->ExecFFT();
		fft_pattern->ExecFFT();
		fft_res->FFTMultiplication(fft_text, fft_pattern);
		fft_res->ExecFFT();

		for (int32_t i = 0; i < size_res; ++i) {
			// sometime, the double variable is close to an integer
			// but a little inferior, the +0,5 corrects the cast into an integer
			res[i]+= (fft_res->getVal(i+size_pattern-1)+0.5);
		}
	}
}

void ComputeInfreq(int32_t size_text, char *text, int32_t size_res,
				int *map, vector<int32_t> *infreq, int *res) {
	int current_val;
	for (int32_t i = 0; i < size_text; ++i) {
		if (IsInfreq(CharMap(text[i], map), infreq)) {
			current_val = map[CharToInt(text[i])];
			for (int32_t j = 0; j < infreq[current_val].size(); ++j) {
				if (i >= infreq[current_val][j]) {
					res[i-infreq[current_val][j]]++;
				}
			}
		}
	}
}

void KeepSmaller(int32_t size_res, int *tmp_res, int *res) {
    for (int32_t i = 0; i < size_res; ++i)
		if (res[i] > tmp_res[i])
            res[i] = tmp_res[i];
}

void WriteOuput(int32_t size_pattern, int32_t size_res, int *res, ofstream &file_out) {
	string buffer;
	buffer.reserve(LIMIT);
	string res_i_str;
	for (int32_t i = 0; i < size_res; ++i) {
    	res_i_str = to_string(size_pattern - res[i]);
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
	if (argc < 4) {
		Usage();
		return 0;
	}
	chrono::time_point<chrono::system_clock> start, mid, end;
    chrono::duration<double> texec;
    start = chrono::system_clock::now();
	mid = start;


    string file_text = argv[1];
	string file_pattern = argv[2];
    double error_dist = atof(argv[3]);
    string file_out = "out.out";
    int cst_loop = 1;
	int *prime_numbers;
	int size_prime_number;
	ReadIntFile("prime_numbers.txt", &size_prime_number, &prime_numbers);
	bool HDopt = true;

    char c;
	while((c = getopt(argc, argv, "c:o:p:x")) !=EOF) {
		switch (c) {
			case 'c':
				cst_loop = atoi(optarg);
				break;
			case 'p':
				LoadSavedPlan(optarg);
				break;
			case 'o':
				file_out = optarg;
				break;
			case 'x':
				HDopt = false;
				break;
			default:
				Usage();
				break;
		}
	}

	int32_t size_pattern, size_text;
	char *pattern;

	// Open and read the file containing the pattern
	ReadCharFile(file_pattern, &size_pattern, &pattern);

	// Open file containing the text
	ifstream stream_text(file_text.c_str(), ios::in);

	if (!stream_text) {
		cout << "Can't open text file." << endl;
		return 0;
	}

	stream_text >> size_text;
	assert(size_text >= size_pattern &&
			"The text's length must be longer or equal to the pattern's. Did you invert the text and pattern calls?");

	ofstream stream_out(file_out.c_str(), ios::out | ios::trunc);
	if (!stream_out) {
		cout << "Can't open output file." << endl;
		return 0;
	}

	char *text = new char[size_text]();

	// Init size_fft: the lenght used for the FFTs
	// It is a power of 2 (see FFTW documentation)
	int32_t size_fft = UpperPowOfTwo(size_text);

	// Initialise the fftw_plan. Should be done before initialising the in/output
	FFT_wak *fft_text = new FFT_wak(size_fft);
	FFT_wak *fft_pattern = new FFT_wak(size_fft);
	FFT_wak *fft_tmp = new FFT_wak(size_fft, false);
	// false indicates that we make a plan for a FFT inverse

	// Init size_res : length of the res table which indicates the hamm dist
	int32_t size_res = size_text - size_pattern + 1;
	int *tmp_res = new int[size_res]();
    int *res = new int[size_res]();
	for (int32_t i = 0; i<size_res; ++i)
		res[i] = size_pattern;

	// Read the text
	char character;
    stream_text.get(character);  // eliminate the \n character
	for (int32_t i = 0; i < size_text; ++i) {
		stream_text.get(character);
		text[i] = character;
	}

    int32_t threshold_freq = sqrt(size_pattern *log2(size_text));
    vector<char> frequent;
    vector<int32_t> *infrequent = new vector<int32_t>[k_nb_letters];

    int32_t cpt_loop = cst_loop * log2(size_text)/4;
	// int32_t cpt_loop = 1;
    int *map = new int[k_nb_letters];
    int size_alphabet = min(round(2 / error_dist), (double)k_nb_letters);
	if (size_alphabet == k_nb_letters)
		cpt_loop = 1;
    cout << "Number of loop iterations: " << cpt_loop << endl;

    for (int i = 0; i < cpt_loop; ++i) {
        MapLetters(size_alphabet, size_prime_number, prime_numbers, map);


		if (HDopt) {

	        // Sort P's caracteres in frequent/infrequent caractere
	        SortfreqInfreqCaract(size_pattern, pattern, threshold_freq, map,
	                            size_alphabet, &frequent, infrequent);

		    ComputeFreq(size_pattern, size_text, size_res, text, pattern, &frequent,
					    map, fft_text, fft_pattern, fft_tmp, tmp_res);

		    ComputeInfreq(size_text, text, size_res, map, infrequent, tmp_res);

		}
		else {
			// Put all characters in freq vector
	        SortFreqCaract(size_pattern, pattern, threshold_freq, map,
	                            size_alphabet, &frequent);

		    ComputeFreq(size_pattern, size_text, size_res, text, pattern, &frequent,
					    map, fft_text, fft_pattern, fft_tmp, tmp_res);
		}


        KeepSmaller(size_res, tmp_res, res);
    }

	// Write in output file
	cout << "Writing results in output file: " << file_out << endl;
	WriteOuput(size_pattern, size_res, res, stream_out);

	stream_text.close();
	stream_out.close();

	delete [] text;
	delete [] pattern;
	delete [] res;
	delete [] tmp_res;
	delete [] map;
	delete [] prime_numbers;

	frequent.clear();
	for (int i=0; i<k_nb_letters; ++i)
		infrequent[i].clear();
	delete [] infrequent;

	delete fft_pattern;
	delete fft_text;
	delete fft_tmp;
	return 0;
}
