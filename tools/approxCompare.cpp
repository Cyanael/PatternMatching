#include<iostream>
#include <fstream>
#include <cmath>

using namespace std;

int main(int argc, char* argv[]){

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

	double dist_error = argv[3];


	int i=0;
	int error = 0;
	while(fileText >> n){
		filePattern >> m;
		if (abs(m - n) > dist_error){
			if (error == 0) {
				cout << "erreur pos " << i << " " << n << " - " << m;
				cout << " < " << dist_error << endl;
			}
			error++;
		}
		i++;
	}
	if (error == 0)
		cout << "Both files are identicals." << endl;
	else
		cout << "Il y a " << error << " erreurs" << endl;

	return 0;
}
