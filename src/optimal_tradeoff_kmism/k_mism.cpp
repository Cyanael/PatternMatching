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

int findApproximatePeriod(int32_t size_pattern, char* pattern, int k) {
  int mism_so_far;
  bool found_approximate_period = false;
  for (int i = 1; i <= k && found_approximate_period == false; i++) {
    mism_so_far = 0;
    for (int j = 0; j <= size_pattern-1-i && mism_so_far <= 4*k; j++) {
      if (pattern[j] != pattern[i+j])
        mism_so_far++;
    }
    cout << i << " : " << mism_so_far << endl;
    if (mism_so_far <= 4*k)
      return i;
  }
  return 0;
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
	int *res;

	// Open and read the files containing the text and pattern
	ReadFile(file_pattern, &size_pattern, &pattern);
	ReadFile(file_text, &size_text, &text);
	assert(size_text >= size_pattern &&
			"The text's length must be longer or equal to the pattern's. Did you invert the text and pattern calls?");
	size_res = size_text - size_pattern +1;
    res = new int[size_text - size_pattern];

    // Recherche periode
	int approx_period = findApproximatePeriod(size_pattern, pattern, error_k);

    // Cas 1 
    // LCP

    // Cas 2
    // Creation T* et P*
    cout << "period : " << approx_period << endl;
    Rle *t_prime = new Rle(approx_period, k_nb_letters);
    int32_t i_l, i_r;
    t_prime->RleText(size_pattern, size_text, text, error_k, &i_l, &i_r);
    cout << "construction done" << endl;
    t_prime->PrintRle();
    cout << "i_l " << i_l << " i_r " << i_r << endl;

    int32_t size_padding = (i_r - i_l + 1) - size_pattern;
    Rle *p_rle = new Rle(approx_period, k_nb_letters);
    p_rle->RlePattern(size_pattern, pattern, size_padding);
    cout << "constr P* done" << endl;
    p_rle->PrintRle();
    int32_t size_res_star = t_prime->Size() * 2 - p_rle->Size();
    int *res_star = new int[size_res_star];


    char *t_star, *p_star;
    int32_t size_t_star, size_p_star;
    t_prime->ToString(&size_t_star, &t_star);
    cout << "T* ";
    for (int i = 0; i < size_t_star; ++i)
		cout << t_star[i];
	cout << endl;
    p_rle->ToString(&size_p_star, &p_star);
	cout << "P* " ;
	for (int i = 0; i < size_p_star; ++i)
		cout << p_star[i];
	cout << endl;


    // cout << "nb a " << t_prime->NbLettersRuns('a') << endl;
    // cout << "nb b " << t_prime->NbLettersRuns('b');
    // cout << "nb c " << t_prime->NbLettersRuns('c');

 //    int threshold = 3;

 //    int32_t size_fft = UpperPowOfTwo(size_text);
 //    FFT_wak *fft_text = new FFT_wak(size_fft);
	// FFT_wak *fft_pattern = new FFT_wak(size_fft);
	// FFT_wak *fft_res = new FFT_wak(size_fft, false);


 //    for (int i = 0; i < k_nb_letters; ++i) {
 //    	if (p_star->NbLettersRuns(IntToChar(i)) > threshold ) { //  heavy letter 
 //    		fft_text->MatchLetters(IntToChar(i), t_prime);
 //    		fft_pattern->MatchLetters(IntToChar(i),p_star);
 //    		fft_pattern->ReversePolynome(p_star->Size());
 //    		fft_text->ExecFFT();
	// 		fft_pattern->ExecFFT();
 //    		fft_res->FFTMultiplication(fft_text, fft_pattern);
 //    		fft_res->ExecFFT();

 //    		for (int i = 0; i < size_res_star ; ++i)
 //    			res_star[i] += (fft_res->getVal(i+p_star->Size()-1)+0.5);
 //    	}
 //    	else {  // light letter

 //    	}
 //    }
    // Tri character freq / infreq

    // Freq : HD

    // Infreq : pour faire lettre infreq, faire la liste des run qui la contiennent dans P*
    // scanner T*
    // 		pour chaque lettre alpha
    //			pour chaque run contenant alpha de P*
    // 				???



    end = chrono::system_clock::now();
    texec = end-start;
    cout << endl << "Total time : " << texec.count() << "s" << endl;


    return 0;
}
