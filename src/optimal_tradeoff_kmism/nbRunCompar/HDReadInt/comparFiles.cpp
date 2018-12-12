/* Copyright : GNU GPL V3
Author : Tatiana Rocher, tatiana.rocher@gmail.com
*/

#include<iostream>
#include <fstream>

using namespace std;

int main(int argc, char* argv[]){

double m, n;

	string inText= argv[1];
	ifstream fileText(inText.c_str(), ios::in);

	if (!fileText){
		cout << "Can't open first file." << endl;
		return 0;
	}
	string inPat= argv[2];
	ifstream filePattern(inPat.c_str(), ios::in);

	if (!filePattern){
		cout << "Can't open second file." << endl;
		return 0;
	}


	int i=0;
	int error = 0;
	while(fileText >> n){
		filePattern >> m;
		if (m != n){
			if (error == 0)
				cout << "First error at position " << i << " " << n << " != " << m << endl;
			error++;
		}
		i++;
	}
	if (error == 0)
		cout << "Both files are identicals." << endl;
	else
		cout << "There are " << error << " errors between the two files." << endl;

	return 0;
}
