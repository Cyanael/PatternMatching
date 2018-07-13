#include<iostream>
#include <fstream>
#include <ctime>
#include <chrono>
#include <vector>

using namespace std;

int main(int argc, char* argv[]){

	if (argc < 4) {
		cout << "Usage: ./exec text_file pattern_file error_max" << endl;
		cout << "If no ouput file is mentionned, result is in verif.out." << endl;
		cout << "/!\\ This algorithm is slow!" << endl;
	}

	chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> texec;
    start = chrono::system_clock::now();

	int m, n;

	string inPat= argv[2];
	ifstream filePattern(inPat.c_str(), ios::in);
	filePattern >> m;

	vector<char> pattern;

	for(int i = 0; i < m; i++){
		char d;
		filePattern >> d;
		pattern.push_back(d);
	}
	filePattern.close();

	string inText= argv[1];
	ifstream fileText(inText.c_str(), ios::in);
	fileText >> n;

	int error_max = atoi(argv[3]);

	string out = "verif.out";
	ofstream fileOut(out.c_str(), ios::out | ios::trunc);

	vector<char> text;
	for (int i=0; i<m; i++){
		char d;
		fileText >> d;
		text.push_back(d);
	}

	end = chrono::system_clock::now();
    texec = end-start;
    cout << "Init time : " << texec.count() << "s" << endl;

	n = n-m;
	char p, t, d;
	int res;
	int i;

	while(n>=0){
		res = 0;
		i = 0;
		while (res < error_max && i < m) {
			if (pattern[i]!=text[i]){
				res++;
			}
			i++;
		}
		if (res >= error_max)
			fileOut << "-1" << endl;
		else
			fileOut << res << endl;

		text.erase(text.begin(), text.begin()+1);
		fileText >> d;
		text.push_back(d);
		n--;
	}

	end = chrono::system_clock::now();
    texec = end-start;
    cout << "Total time : " << texec.count() << "s" << endl;

	return 0;
}
