#include<iostream>
#include <fstream>
#include <ctime>
#include <chrono>
#include <vector>

using namespace std;
int LIMIT = 524288;  // size of the output buffer

void ReadFile(string file, int32_t *size_pattern, char **pattern) {
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

void WriteOuput(int32_t size_res, int *res, ofstream &file_out) {
    string buffer;
    buffer.reserve(LIMIT);
    string res_i_str;
    for (int32_t i = 0; i < size_res; ++i) {
            res_i_str = to_string(res[i]);
        if (buffer.length() + res_i_str.length() + 1 >= LIMIT) {
            file_out << buffer;
            buffer.resize(0);
        }
        buffer.append(res_i_str);
        buffer.append("\n");
    }
    file_out << buffer;
}

int main(int argc, char* argv[]){

	if (argc < 3) {
		cout << "Usage: ./exec text_file pattern_file" << endl;
		cout << "If no ouput file is mentionned, result is in verif.out." << endl;
		cout << "/!\\ This algorithm is slow!" << endl;
	}

	chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> texec;
    start = chrono::system_clock::now();

	int32_t m, n;

	char *text, *pattern;
	string inText= argv[1];
	string inPat= argv[2];

	ReadFile(inText, &n, &text);
	ReadFile(inPat, &m, &pattern);

	string out = "naive.out";
	ofstream fileOut(out.c_str(), ios::out | ios::trunc);

	int32_t size_res = n - m +1;
	int *res = new int[size_res]();

	char d;
	int current_err;

	for (int i = 0; i < size_res; ++i){
		current_err = 0;
		for (int j = 0; j < m; ++j) {
			if (pattern[j]!=text[j + i]){
				current_err++;
			}
		}
		res[i] = current_err;
	}

	cout << "Write results in " << out << endl;
	WriteOuput(size_res, res, fileOut);

	end = chrono::system_clock::now();
    texec = end-start;
    cout << "Total time : " << texec.count() << "s" << endl;

	return 0;
}
