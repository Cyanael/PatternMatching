/* Copyright : ???
Author : Tatiana Rocher, tatiana.rocher@gmail.com 

Compilation :
install the fftw3 library
g++ -std=c++11 hamDist.cpp Fft_wak.cpp -o hd -lfftw3 -lm

Execution :
./hd text.in pattern.in optional.out
The pattern/text input file must contain its lenght then the pattern/text
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cassert>
#include <cstdint>
#include <chrono>

extern "C" {
	#include "../fftw-3.3.7/api/fftw3.h"
}

#include "Fft_wak.hpp"

using namespace std;

// int NBLETTER = 26;
int k_nb_letters = 128;
int LIMIT = 1000;  // size of the output buffer

bool usage(int argc) {
	if (argc < 3) {
		cout << endl << "How to run: ./exec text pattern optionalOutput" << endl;
		cout << "/!\\ The text (or pattern) input file must ";
		cout << "contain its lenght first, then the text (or pattern)." << endl;
		cout << "The size of the text is limited to 2^32/2 bits. ";
		cout << "If your text is longer, consider spliting it."<< endl;
		cout << "Be carefull to have only ascii characters ";
		cout << "in the text and pattern" << endl;
		cout << endl;
		return false;
	}
	return true;
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
	if (val >k_nb_letters)
		cout << "lettre trop grande " << val << endl;
	return (char) val;
}

// Intialise the pattern, size_pattern and size_text  from a file
void ReadPattern(string file, int32_t *size_pattern, char **pattern) {
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

bool SortVectofPair(pair<char, int> a, pair<char, int> b) {
	return a.first < b.first;
}

void SortfreqInfreqCaract(int32_t size_pattern, char *pattern, float limit,
						vector<char> *freq, vector<int32_t> *infreq) {
	for (int32_t i = 0; i < size_pattern; ++i) {
		if (infreq[CharToInt(pattern[i])].size() == 0) {
			vector<int32_t> v = {i};
			infreq[CharToInt(pattern[i])] = v;
		}
		else
			infreq[CharToInt(pattern[i])].push_back(i);
	}
	for (int32_t i = 0; i < k_nb_letters; ++i) {
		if (infreq[i].size() >= limit) {
			freq->push_back(IntToChar(i));
			infreq[i].clear();
		}
	}
}


bool IsInfreq(char letter, vector<int32_t> *infreq) {
	if (infreq[CharToInt(letter)].size() > 0)
		return true;
	return false;
}

// Computes the boolean vector of the text
// when there is a 1 where the matching letter is "letter"
void MatchLetterText(int32_t size, char *text, char letter,
					FFT_wak *fft_text) {
	for (int32_t i = 0; i < size; ++i) {
		if(text[i] == letter)
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
				char *text, char *pattern, vector<char> *frequent,
				FFT_wak *fft_text, FFT_wak *fft_pattern, FFT_wak *fft_res,
				int *res) {
	InitTabZeros(size_res, res);

	char current_char;
	for (auto j = frequent->begin(); j != frequent->end(); ++j) {
		current_char = *j;
		MatchLetterText(size_text, text, current_char, fft_text);
		MatchLetterText(size_pattern, pattern, current_char, fft_pattern);

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
				vector<int32_t> *infreq, int *res) {
	char current_char;
	for (int32_t i = 0; i < size_text; ++i) {
		if (IsInfreq(text[i], infreq)) {
			current_char = CharToInt(text[i]);
			for (int32_t j = 0; j < infreq[current_char].size(); ++j) { 
				if (i >= infreq[current_char][j]) {
					res[i-infreq[current_char][j]]++;
				}
			}
		}
	}
}

void WriteOuput(int32_t size_res, int *res, ofstream &file_out) {
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
	if (!usage(argc)) return 0;

	chrono::time_point<chrono::system_clock> start, mid, end;
    chrono::duration<double> texec;
    start = chrono::system_clock::now();

	int32_t size_pattern, size_text;
	char *pattern;

	// Open and read the file containing the pattern
	string file_pattern = argv[2];
	ReadPattern(file_pattern, &size_pattern, &pattern);

	// Open file containing the text
	string file_text = argv[1];
	ifstream stream_text(file_text.c_str(), ios::in);

	if (!stream_text) {
		cout << "Can't open text file." << endl;
		return 0;
	}

	stream_text >> size_text;
	assert(size_text >= size_pattern &&
			"The text's length must be longer or equal to the pattern's. Did you invert the text and pattern calls?");

	// Open output file
	string out;
	if (argc < 4)
		out = "out.out";
	else
		out = argv[3];

	ofstream file_out(out.c_str(), ios::out | ios::trunc);
	if (!file_out) {
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

	// Begin of computation
	// Sort P's caracteres in frequent/infrequent caractere
	int32_t threshold_freq = sqrt(size_pattern *log2(size_text));
	vector<char> frequent;
	vector<int32_t> *infrequent = new vector<int32_t>[k_nb_letters];
	SortfreqInfreqCaract(size_pattern, pattern, threshold_freq,
						&frequent, infrequent);

    // cout << "freq : ";
    // for (int i = 0; i < frequent.size(); i++)
    //  	cout << frequent[i] << " ";
    //  cout << endl;

    // cout << "infreq : ";
    // for (int i = 0; i < k_nb_letters; i++)
    //  	// if (infrequent[i].size() > 0)
	   //   	cout << i << " " << IntToChar(i) << " ";
    //  cout << endl;

	// Init size_res : length of the res table which indicates the hamm dist
	int32_t size_res = size_text - size_pattern + 1;
	int *res = new int[size_res]();

	// Read the text
	char character;
    stream_text.get(character);  // eliminate the \n character
	for (int32_t i = 0; i < size_text; ++i) {
		stream_text.get(character);
		text[i] = character;
	}

	ComputeFreq(size_pattern, size_text, size_res, text, pattern, &frequent,
				fft_text, fft_pattern, fft_tmp, res);
	
	ComputeInfreq(size_text, text, size_res, infrequent, res);

	// Write in output file
	cout << "Writing results in output file: " << out << endl;
	WriteOuput(size_res, res, file_out);

	stream_text.close();
	file_out.close();

	delete [] text;
	delete [] pattern;
	delete [] res;

	delete fft_pattern;
	delete fft_text;
	delete fft_tmp;

	end = chrono::system_clock::now();
    texec = end-start;
    cout << "Total time : " << texec.count() << "s" << endl;

	return 0;
}