#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
 
using namespace std;
 
int main() {

	int length = 100000;
	// string nameOut = "text" + length + ".txt";
    ofstream fichier("test100000.txt", ios::out | ios::trunc);  // ouverture en écriture avec effacement du fichier ouvert
 	fichier << length << endl;


    if(fichier) {
    	for (int i=0; i<length; i++)
	        fichier << rand()%100 << " ";
 
        fichier.close();
    }
    else
        cerr << "Impossible d'ouvrir le fichier !" << endl;
 
    return 0;
}
