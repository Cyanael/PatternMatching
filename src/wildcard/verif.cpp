#include<iostream>
#include <fstream>
#include <ctime> 
#include <chrono>

using namespace std;

int main(int argc, char* argv[]){

	chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> texec;
    start = chrono::system_clock::now();

	int m, n;

	string inPat= argv[2];
	ifstream filePattern(inPat.c_str(), ios::in);
	filePattern >> m;

	double *text, *pattern;
	text = (double *) malloc(m*sizeof(double));
	pattern = (double *) malloc(m*sizeof(double));

	// cout << "Input : " << endl;
	for(int i = 0; i < m; i++)
		filePattern >> pattern[i];
	filePattern.close();
	
	string inText= argv[1];
	ifstream fileText(inText.c_str(), ios::in);
	fileText >> n;

	string out = "verif.out";
	ofstream fileOut(out.c_str(), ios::out | ios::trunc);

	for (int i=0; i<m; i++)
		fileText >> text[i];

	n = n-m;
	int p, t, res;

	while(n>=0){
		res = 0;
		for (int i=0; i<m; i++){
			p = pattern[i];
			t = text[i];
			res += p * t * (t-p) * (t-p);
		}
		fileOut << res << endl;

		for (int i=0; i<m-1; i++)
			text[i] = text[i+1];
		fileText >> text[m-1];
		n--;
	}

	
	end = chrono::system_clock::now();
    texec = end-start;
    cout << "Total time : " << texec.count() << "s" << endl;

	return 0;
}