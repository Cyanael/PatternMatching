#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

using namespace std;

int main(int argc, char* argv[]) {
	if (argc < 4)
		cout << "Usage: ./exec nb_of_elem nb_min nb_max name_file" << endl;

	int length = atoi(argv[1]);
	int min = atoi(argv[2]);
	int max = atoi(argv[3]);
	string name_file = argv[4];
	// string nameOut = "text" + length + ".txt";

    ofstream fichier(name_file, ios::out | ios::trunc);  // ouverture en écriture avec effacement du fichier ouvert
 	fichier << length << endl;


    if(fichier) {
    	for (int i=0; i<length; i++)
	        fichier << rand()%max+min << " ";

        fichier.close();
    }
    else
        cerr << "Can't open file !" << endl;

    return 0;
}
