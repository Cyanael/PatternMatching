/* Copyright : ???
Author : Tatiana Rocher, tatiana.rocher@gmail.com

Compilation :
install the fftw3 library
g++ -std=c++11 -O3 hamDist.cpp Fft_wak.cpp -o hd -lfftw3 -lm

Execution :
./hd text.in pattern.in optional.out
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
#include "LCP.hpp"
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
		cout << "Can't open pattern file." << endl;
}

void WriteOuput(int32_t size_pattern, int32_t size_res, int *res, bool isHD,
				int error_k, ofstream &stream_out) {
	string buffer;
	buffer.reserve(LIMIT);
	string res_i_str;
	int cpt = 0;
	for (int32_t i = 0; i < size_res; ++i) {
		if (isHD) {
			if (size_pattern - res[i] > error_k)
				res_i_str = to_string(-1);
			else
				res_i_str = to_string(size_pattern - res[i]);
		}
		else {
		    if (res[i] == -1)
		    	res_i_str = to_string(-1);
		    else
		    	res_i_str = to_string(res[i]);
		}
    	if (buffer.length() + res_i_str.length() + 1 >= LIMIT) {
        	stream_out << buffer;
			cpt++;
        	buffer.resize(0);
    	}
    	buffer.append(res_i_str);
    	buffer.append("\n");
		}
	stream_out << buffer;
}

int SortCaract(int32_t size_pattern, char *pattern, int32_t threshold_freq,
				vector<int32_t> *sortedChar) {
	for (int32_t i = 0; i < size_pattern; ++i) {
		if (sortedChar[CharToInt(pattern[i])].size() == 0) {
			vector<int32_t> v = {i};
			sortedChar[CharToInt(pattern[i])] = v;
		}
		else
			sortedChar[CharToInt(pattern[i])].push_back(i);
	}
	int freqChar = 0;
	for (int i=0; i<k_nb_letters; ++i)
    	if (sortedChar[i].size()> threshold_freq)
    		freqChar++;
    return freqChar;
}

int main(int argc, char* argv[]) {
	if (argc < 3) {
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

	chrono::time_point<chrono::system_clock> start, mid, end;
    chrono::duration<double> texec;
    start = chrono::system_clock::now();


	int32_t size_pattern, size_text, size_res;
	char *pattern, *text;

	// Open and read the files containing the text and pattern
	ReadFile(file_pattern, &size_pattern, &pattern);
	ReadFile(file_text, &size_text, &text);
	assert(size_text >= size_pattern &&
			"The text's length must be longer or equal to the pattern's. Did you invert the text and pattern calls?");

	// Init size_res : length of the res table which indicates the hamm dist
	size_res = size_text - size_pattern + 1;
	int *res = new int[size_res]();

	// Open output file
	ofstream stream_out(file_out.c_str(), ios::out | ios::trunc);
	if (!stream_out) {
		cout << "Can't open output file." << endl;
		return 0;
	}

	end = chrono::system_clock::now();
	texec = end-start;
	cout << "Init vectors : " << texec.count() << "s" << endl;
	mid = end;



	// Begin of computation
	// Sort P's caracteres in frequent/infrequent caractere
	float threshold_freq = sqrt(error_k);
	vector<int32_t> *sortedChar = new vector<int32_t>[k_nb_letters];

	end = chrono::system_clock::now();
	texec = end-start;
	cout << "Init ffts : " << texec.count() << "s" << endl;
	mid = end;

	int freqChar = SortCaract(size_pattern, pattern, threshold_freq, sortedChar);


	 end = chrono::system_clock::now();
	 texec = end-mid;
	 cout << "sort freq infreq : " << texec.count() << "s" << endl;
	 mid = end;

	// Chose between the Hamming Distance algorithm or kangaroo algorithm
	bool isHD = true;
	if (freqChar < 2*threshold_freq) {
		ComputeHD(size_text, text, size_pattern, pattern,
						sortedChar, threshold_freq, k_nb_letters, size_res, res);
	} else {
		ComputeLCP(size_text, text, size_pattern, pattern,
						sortedChar, threshold_freq, k_nb_letters, error_k, size_res, res);
		isHD = false;
	}

	end = chrono::system_clock::now();
	texec = end-start;
	cout << "Computation : " << texec.count() << "s" << endl;
	mid = end;

	// Write in output file
	cout << "Writing results in output file: " << file_out << endl;
	WriteOuput(size_pattern, size_res, res, isHD, error_k, stream_out);

	end = chrono::system_clock::now();
	texec = end-mid;
	cout << "write out : " << texec.count() << "s" << endl;
	mid = end;

	// stream_text.close();
    stream_out.close();

    delete [] text;
    delete [] pattern;
    delete [] res;
	for (int i=0; i<k_nb_letters; ++i)
		sortedChar[i].clear();
	delete [] sortedChar;

	end = chrono::system_clock::now();
    texec = end-mid;
    cout << "Free : " << texec.count() << "s" << endl;
    texec = end-start;
    cout << endl << "Total time : " << texec.count() << "s" << endl;

    return 0;
}
