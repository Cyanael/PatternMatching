/* Copyright : ???
Author : Tatiana Rocher, tatiana.rocher@gmail.com

Compilation :
g++ -std=c++11 -O3 test_r.cpp HammingDistance.cpp Tools.cpp Fft_wak.cpp -o test_r -lfftw3 -lm

Execution :
./hd text.in pattern.in error_max -o optionalOutput.out -p optionalPlan
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
#include "Tools.hpp"

using namespace std;

int LIMIT = 1048576;  // size of the output buffer
int k_nb_letters = 128;


bool Usage() {
	cout << endl << "How to run: ./exec text size_pattern epsilon r_min r_max optionalOutput ";
	cout << "-p optinalPlan" << endl;
	cout << "/!\\ The text (or pattern) input file must ";
	cout << "contain its lenght first, then the text (or pattern)." << endl;
	cout << "The size of the text is limited to 2^32/2 bits. ";
	cout << "If your text is longer, consider spliting it."<< endl;
	cout << "Be carefull to have only ascii characters ";
	cout << "in the text and pattern" << endl;
	cout << endl;
}



/*************** Input / Ouput *******************/

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

void ReadPattern(int32_t size_text, char *text, int32_t size_pattern, char *pattern) {
    struct timespec ts;
    if (timespec_get(&ts, TIME_UTC) == 0) {
        cout << "error seed." << endl;
    }
    srandom(ts.tv_nsec ^ ts.tv_sec); // compute seed

    int32_t begin = random()%(size_text - size_pattern);
    for (int32_t i = 0; i < size_pattern; ++i){
        pattern[i] = text[begin + i];
    }
       
}

void WriteOuput(int32_t size_pattern, int32_t size_res, int *res,
				int error_k, ofstream &stream_out) {
	string buffer;
	buffer.reserve(LIMIT);
	string res_i_str;
	int cpt = 0;
	for (int32_t i = 0; i < size_res; ++i) {
		if (size_pattern - res[i] > error_k)
			res_i_str = to_string(-1);
		else
			res_i_str = to_string(size_pattern - res[i]);
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


/************** Constructions *****************/

void makeMatrix(int r, int m, bool **matrix[]) {
	(*matrix) = new bool*[r];
	struct timespec ts;

	if (timespec_get(&ts, TIME_UTC) == 0) {
		cout << "error seed." << endl;
	}
	srandom(ts.tv_nsec ^ ts.tv_sec); // compute seed
		
	int rand;
	for (int i = 0; i < r; ++i) {
		(*matrix)[i] = new bool[m];
		for (int j = 0; j < m; ++j){
			rand = random()%2;
			if (rand == 0)
				(*matrix)[i][j] = true;
			else
				(*matrix)[i][j] = false; 
		}
	}
}

void PrintMatrix(int r, int m, bool *matrix[]) {
	for (int i = 0; i < r; ++i) {
		// for (int j = 0; j < m; ++j)
			cout << matrix[i][97] << " " << matrix[i][98] << " " << matrix[i][99] << " ";
	cout << endl;
	}
}


/************** Computation ******************/


void Karloff(int32_t size_pattern, char *pattern, int32_t size_text,
			char  *text, float epsilon, int r, int *tmp, int32_t size_res, int *res) {
	// int *tmp = new int[size_res];
	// construction matrice
	int m = k_nb_letters;
	bool **matrix;
	makeMatrix(r, m, &matrix);
	PrintMatrix(r, m, matrix);

	int32_t size_fft = UpperPowOfTwo(size_text);
    FFT_wak *fft_text = new FFT_wak(size_fft);
	FFT_wak *fft_pattern = new FFT_wak(size_fft);
	FFT_wak *fft_res = new FFT_wak(size_fft, false);



	for (int i = 0; i < r; ++i) {
		HD(size_pattern, size_text, size_res, text, pattern, fft_text,
			fft_pattern, fft_res, matrix, i, tmp);
	
		// cout << "Res fft : " ;
		for (int j = 0; j < size_res; ++j) {
			res[j] += tmp[j];
		// 	cout << tmp[j] << " ";
		}
		// cout << endl;
	}
	// PrintTable(size_res, res);

	for (int i = 0; i < size_res; ++i)
		res[i] /= (((float)r/2) - (epsilon/6) * r);

	// PrintTable(size_res, res);

	// delete [] tmp;
	for (int i = 0; i < r; ++i)
		delete [] matrix[i];
	delete [] matrix;
}


int main(int argc, char* argv[]) {
	if (argc < 4) {
		Usage();
		return 0;
	}

	string file_text = argv[1];
	int32_t size_pattern = atoi(argv[2]);
	float epsilon = atof(argv[3]);
	int min_r = atoi(argv[4]);
	int max_r = atoi(argv[5]);
	string file_out = "out.out";

	char c;
	while((c = getopt(argc, argv, "o:")) !=EOF) {
		switch (c) {
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

    int32_t size_text, size_res;
	char *pattern, *text;
	int *res;

	// Open and read the files containing the text and pattern
	ReadFile(file_text, &size_text, &text);
	assert(size_text >= size_pattern &&
			"The text's length must be longer or equal to the pattern's. Did you invert the text and pattern calls?");

	size_res = size_text - size_pattern +1;
    res = new int[size_res];

    pattern = new char[size_pattern];
    int *res_2 = new int[size_res];
    int *tmp = new int[size_res];

    for (int r = min_r; r <= max_r; ++r) {
    	cout << endl << "R = " << r << endl;
    	for (int nb_run = 0; nb_run < 5; ++nb_run) {
			ReadPattern(size_text, text, size_pattern, pattern);
		    InitTabZeros(size_res, res);
		    PrintTable(size_res, res);

		    Karloff(size_pattern, pattern, size_text, text, epsilon, r, tmp, size_res, res);

		    //res exact
		    int nb_err;
		    for (int i = 0; i < size_text - size_pattern + 1; ++i) {
		    	nb_err = 0;
		    	for (int j = 0; j < size_pattern; ++j){
		    		if (text[i] != pattern[i+j])
		    			nb_err++;
		    	}
		    	res_2[i] = nb_err;
		    }

			// check res
			int n, m;
			int error = 0;
			for (int i=0; i < size_res; ++i) {
				n = res[i];
				m = res_2[i];
				if ((1 - epsilon)*n > m) {
					// if (error == 0) {
						cout << "erreur pos " << i << " : (1-" << epsilon << ")*" << n << " : " << (1 - epsilon)*n << " > " << m << endl;
					// }
					error++;
				}
				else if ((1+ epsilon)*n < m ) {
					// if (error == 0) {
						cout << "erreur pos " << i << " : (1+" << epsilon << ")*" << n << " : " << (1+ epsilon)*n << " < " << m << endl;
					// }
					error++;
				}
			}
			if (error == 0)
				cout << "Both files are identicals." << endl;
			else
				cout << "Il y a " << error << " erreurs" << endl;
	    }
	}

    delete [] text;
    delete [] pattern;
    delete [] res;
    delete [] res_2;
    delete [] tmp;

    return 0;
}