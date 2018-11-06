#include<iostream>
#include <fstream>

using namespace std;

int main(int argc, char* argv[]){

	if (argc < 3) {
		cout << "Usage: ./exec k_mism_res exact_res" << endl;
		cout << "If no ouput file is mentionned, result is in verif.out." << endl;
		cout << "/!\\ This algorithm is slow!" << endl;
	} 
	int m, n;

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
	int nb_errors = 0;
	while(fileText >> n){
		filePattern >> m;
		if (m != n){
			if (nb_errors == 0)
				cout << "first error at position " << i << " " << n << " != " << m << endl; 
			nb_errors++;
		}
		i++;
	}
	if (nb_errors == 0)
		cout << "Both files are identicals." << endl;
	else
		cout << "There are " << nb_errors << " errors between the files." << endl;

	return 0;
}