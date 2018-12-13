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
#include "LCP.hpp"
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

// Intialise the pattern and size_pattern from a file
void ReadFile(string file, int32_t *size_pattern, int **pattern) {
	ifstream stream_file(file.c_str(), ios::in);
	char character;

	if (stream_file) {
		int size_tmp;
		stream_file >> size_tmp;
		(*size_pattern) = size_tmp;

		stream_file.get(character);  // eliminate the \n character
		(*pattern) = new int[size_tmp]();
		for (int32_t i = 0; i < size_tmp; ++i) {
			// stream_file.get(character);
			// (*pattern)[i] = character;
			stream_file >> (*pattern)[i];
		}

		stream_file.close();
	}
	else
		cout << "Can't open pattern file." << endl;
}


void WriteOuput(int32_t size_res, int *res, int error_k,
				ofstream &stream_out) {
	string buffer;
	buffer.reserve(LIMIT);
	string res_i_str;
	int cpt = 0;
	for (int32_t i = 0; i < size_res; ++i) {
		if (res[i] > error_k)
			res_i_str = to_string(-1);
		else
			res_i_str = to_string(res[i]);
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

	chrono::time_point<chrono::system_clock> start, mid, end;
    chrono::duration<double> texec;
    start = chrono::system_clock::now();

    int32_t size_pattern, size_res, size_text;
	int *pattern, *text;
	int *res;

	// Open and read the files containing the pattern
	ReadFile(file_pattern, &size_pattern, &pattern);

	// Open output file
	ofstream stream_out(file_out.c_str(), ios::out | ios::trunc);
	if (!stream_out) {
		cout << "Can't open output file." << endl;
		return 0;
	}

	// Search for an approximate period
	int approx_period = findApproximatePeriod(size_pattern, pattern,
		k_nb_letters, 1, error_k, 500*error_k); // TODO: remettre 8*errok_k
	cout << "period: " << approx_period << endl;

		mid = chrono::system_clock::now();
		texec = mid-start;
		cout << endl << "Approx time : " << texec.count() << "s" << endl;
		end = mid;

	int nb_updates = 0;
	//  Case 1 in the papermid
	if (approx_period == 0) {
		cout << "There is no 4k-period" << endl;
		ReadFile(file_text, &size_text, &text);
		assert(size_text >= size_pattern &&
				"The text's length must be longer or equal to the pattern's. Did you invert the text and pattern calls?");
		size_res = size_text - size_pattern +1;
		res = new int[size_res];
		InitTabZeros(size_res, res);

		NoSmall4kPeriod(size_text, text, size_pattern, pattern, k_nb_letters,
			error_k, size_res, res);

	    WriteOuput(size_res, res, error_k, stream_out);
		}
	else {  // There is a 8k-period <= k, case 2 in the paper
		cout << "There is a 8k-period" << endl;
		// Read text
		ifstream stream_text(file_text.c_str(), ios::in);
		if (!stream_text) {
			cout << "Can't open text file." << endl;
			return 0;
		}

		stream_text >> size_text;
		assert(size_text >= size_pattern &&
						"The text's length must be longer or equal to the pattern's. Did you invert the text and pattern calls?");

		int32_t rest = size_text % size_pattern;
		int32_t nb_iter = ceil(size_text/(size_pattern+1));
		cout << "nb_iter " << nb_iter << endl;
		size_text = 2*size_pattern;
		// cout << "size_text " << size_text << endl;

		char character;
		stream_text.get(character);  // eliminate the \n character
		text = new int[2*size_pattern]();
		for (int32_t i = 0; i < size_pattern-1; ++i) {
			// stream_text.get(character);
			// text[i] = character;
			stream_text >> text[i];
		}



		size_res = size_pattern +1;
		res = new int[size_res];
		InitTabZeros(size_res, res);

		for (int i = 0; i < nb_iter; ++i) {
			// cout << "num iter = " << i << endl;
			//  add size_pattern letters
			if (i < nb_iter -1) {
				for (unsigned int i = 0; i < size_pattern+1; ++i){
					// stream_text.get(character);
					// text[size_pattern+i-1] = character;
					stream_text >> text[size_pattern+i-1];
				}
			}
			else {

				for (unsigned int i = 0; i < rest; ++i){
					// stream_text.get(character);
					// text[size_pattern+i-1] = character;
					stream_text >> text[size_pattern+i-1];
				}
				for (unsigned int i = rest; i < size_pattern+1; ++i){
					text[size_pattern+i-1] = 98;
				}
			}

			Small8kPeriod(size_text, text, size_pattern, pattern, k_nb_letters, error_k,
							approx_period, size_res, res, &nb_updates);

		    WriteOuput(size_pattern+1, res, error_k, stream_out);

			//  slide letters
			for (unsigned int i = 0; i < size_pattern-1; ++i){
				text[i] = text[i+size_pattern+1];
			}
		}
		stream_text.close();
	}
	cout << "nb updates " << nb_updates << endl;


    end = chrono::system_clock::now();
	texec = end-mid;
    cout << endl << "LCP/RLE time : " << texec.count() << "s" << endl;
    texec = end-start;
    cout << endl << "Total time : " << texec.count() << "s" << endl;

    stream_out.close();

    delete [] text;
    delete [] pattern;
    delete [] res;

    return 0;
}
