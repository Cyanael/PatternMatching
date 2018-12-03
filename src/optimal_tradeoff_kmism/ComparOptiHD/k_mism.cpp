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


#include "HammingDistance1.hpp"
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

void LoadPattern(int32_t size_text, char *text_pattern, int32_t size_pattern,
                char **pattern) {

    (*pattern) = new char[size_pattern]();
    struct timespec ts;
    if (timespec_get(&ts, TIME_UTC) == 0) {
        cout << "error seed." << endl;
    }
    srandom(ts.tv_nsec ^ ts.tv_sec); // compute seed

    int32_t begin = random()%(size_text - size_pattern);
    for (int32_t i = 0; i < size_pattern; ++i){
        (*pattern)[i] = text_pattern[begin + i];
        text_pattern[size_text + i] = text_pattern[begin + i];
    }
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
	int32_t size_pattern = atoi(argv[2]);
	int error_k = atoi(argv[3]);

	char c;
	while((c = getopt(argc, argv, "p:")) !=EOF) {
		switch (c) {
			case 'p':
				LoadSavedPlan(optarg);
				break;
			default:
				Usage();
				break;
		}
	}

	chrono::time_point<chrono::system_clock> start, mid, end;
    chrono::duration<double> texec;


    int32_t size_res, size_text;
	char *text;

	// Open and read the files containing the pattern
	ReadFile(file_text, &size_text, &text);

	size_res = size_text - size_pattern +1;
	int *res = new int[size_res];
	int *res_hd = new int[size_res]();
	int *res_naif = new int[size_res]();
	InitTabZeros(size_res, res);

	float ave_rle = 0, ave_hd = 0, ave_naif;


	int nb_loop = 20;
	for (int i = 0; i < nb_loop; ++i) {
	    start = chrono::system_clock::now();
		char *pattern;

		LoadPattern(size_text, text, size_pattern, &pattern);

		// Search for an approximate period
		int approx_period = findApproximatePeriod(size_pattern, pattern,
			k_nb_letters, 1, error_k, 8*error_k);

		mid = chrono::system_clock::now();
		texec = mid-start;
		// cout << endl << "Approx time : " << texec.count() << "s" << endl;
		end = mid;

		assert(size_text >= size_pattern &&
							"The text's length must be longer or equal to the pattern's. Did you invert the text and pattern calls?");

		//  Case 1 in the papermid
		if (approx_period == 0) {
			cout << "There is no 4k-period" << endl;

			NoSmall4kPeriod(size_text, text, size_pattern, pattern, k_nb_letters,
				error_k, size_res, res);

		    // WriteOuput(size_res, res, error_k, stream_out);
			}
		else {  // There is a 8k-period <= k, case 2 in the paper
			cout << "There is a 8k-period" << endl;
			Small8kPeriod(size_text, text, size_pattern, pattern, k_nb_letters, error_k,
								approx_period, size_res, res);
		}

	    end = chrono::system_clock::now();
	    texec = end-start;
	    // cout << endl << "Total time : " << texec.count() << "s" << endl;
	    ave_rle += texec.count();
	    mid = chrono::system_clock::now();



		HD2(size_text, text, size_pattern, pattern, k_nb_letters, size_res, res_hd);

		mid = chrono::system_clock::now();
		texec = mid-start;
		cout << endl << "HD : " << texec.count() << "s ";
		end = mid;
		ave_hd += texec.count();

		bool verif = true;
        int pos = 0;
        while (verif && pos < size_res) {
            if (res[pos] != (size_pattern - res_hd[pos]))
                verif = false;
            pos++;
        }
        if (verif)
            cout << "== " << endl;
        else
            cout << "!= " << endl;      

   		mid = chrono::system_clock::now();

		// naive algo
        int curr_error, pos_p;
        for (int j = 0; j < size_res; ++j) {
            curr_error = 0;
            pos_p = 0;
            while (curr_error < error_k && pos_p < size_pattern) {
    			if (pattern[pos_p]!=text[j + pos_p]){
    				curr_error++;
    			}
    			pos_p++;
    		}

            if (curr_error >= error_k)
                res_naif[j] = -1;
            else
                res_naif[j] = curr_error;
        }

        end = chrono::system_clock::now();
        texec = end-mid;
        cout << "Naif : " << texec.count() << "s ";
        ave_naif += texec.count();

        verif = true;
        pos = 0;
        while (verif && pos < size_res) {
        	// cout << res[pos] << " " << res_naif[pos] << endl;
            if (res[pos] != res_naif[pos] && (res[pos] < error_k))
                verif = false;
            pos++;
        }
        if (verif)
            cout << "== " << endl;
        else
            cout << "!= " << endl;


	    delete [] pattern;
	}

	ave_hd /= nb_loop;
	ave_rle /= nb_loop;
	ave_naif /=nb_loop;
	cout << endl << "Temps moyen RLE : " << ave_rle +0.83<< endl;
	cout << "Temps moyen HD : " << ave_hd + 0.83 << endl;
	cout << "Temps moyen naif : " << ave_naif + 0.83 << endl;

    delete [] text;
    delete [] res;
    delete [] res_hd;

    return 0;
}
