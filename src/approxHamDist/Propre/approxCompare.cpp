#include<iostream>
#include <fstream>
#include <cmath>

using namespace std;

int main(int argc, char* argv[]){

	if (argc < 4) {
		cout << "Run with ./exec file1 file2 error" << endl;
		return 0;
	}
	double m, n;

	string inText= argv[1];
	ifstream fileText(inText.c_str(), ios::in);

	if (!fileText){
		cout << "Can't open text file." << endl;
		return 0;
	}
	string inPat= argv[2];
	ifstream filePattern(inPat.c_str(), ios::in);

	if (!filePattern){
		cout << "Can't open pattern file." << endl;
		return 0;
	}

	double dist_error = atof(argv[3]);


	int i=0;
	int error = 0;
	while(fileText >> n){
		filePattern >> m;
		if ((1 - dist_error)*n > m) {
			if (error == 0) {
				cout << "erreur pos " << i << " : (1-" << dist_error << ")*" << n << " : " << (1 - dist_error)*n << " > " << m << endl;
			}
		}
		else if ((1+ dist_error)*n < m ) {
			if (error == 0) {
				cout << "erreur pos " << i << " : (1-" << dist_error << ")*" << n << " : " << (1+ dist_error)*n << " < " << m << endl;
			}
			error++;
		}
		i++;
	}
	if (error == 0)
		cout << "Both files are identicals." << endl;
	else
		cout << "There are " << error << " errors" << endl;

	return 0;
}
