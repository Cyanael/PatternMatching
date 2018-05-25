#include<iostream>
#include <fstream>
#include <ctime> 
#include <chrono>
#include <vector>

using namespace std;

int main(int argc, char* argv[]){

	chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> texec;
    start = chrono::system_clock::now();

	int m, n;

	string inPat= argv[2];
	ifstream filePattern(inPat.c_str(), ios::in);
	filePattern >> m;

	vector<char> pattern;

	// cout << "Input : " << endl;
	for(int i = 0; i < m; i++){
		char d;
		filePattern >> d;
		pattern.push_back(d);
	}
	filePattern.close();

	string inText= argv[1];
	ifstream fileText(inText.c_str(), ios::in);
	fileText >> n;

	string out = "verif.out";
	ofstream fileOut(out.c_str(), ios::out | ios::trunc);

	vector<char> text;
	for (int i=0; i<m; i++){
		char d;
		fileText >> d;
		text.push_back(d);
	}

	n = n-m;
	char p, t;
	int res;



	while(n>=0){
	int j=0;
	// while(j<1){
	// cout << "pattern :	";
	// for (int i = 0; i<m; i++)
	// 	cout << pattern[i] << " ";
	// cout << endl;

	// cout << "text :		";
	// for (int i = 0; i<m; i++)
	// 	cout << text[i] << " ";
	// cout << endl;
		res = 0;
		for (int i=0; i<m; i++){
			p = pattern[i];
			t = text[i];
			if (p!=t){
				res++;
			}
			// cout << i << " " << t << " != " << p << " -> " << res << endl;
		}
		fileOut << res << endl;

		text.erase(text.begin(), text.begin()+1);
		char d;
		fileText >> d;
		text.push_back(d);
		n--;
		j++;
	}

	
	end = chrono::system_clock::now();
    texec = end-start;
    cout << "Total time : " << texec.count() << "s" << endl;

	return 0;
}