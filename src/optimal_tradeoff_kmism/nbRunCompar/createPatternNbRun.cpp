#include <iostream>
#include <fstream>
#include <sstream>


using namespace std;

bool Usage() {
	cout << endl << "How to run: ./exec size_file period nb_blocs file_out" << endl;
}


int main(int argc, char* argv[]) {
    if (argc < 5) {
        Usage();
        return 0;
    }

    int32_t size_file = atoi(argv[1]);
    int period = atoi(argv[2]);
	int nb_blocs = atoi(argv[3]);
	// string file_out = argv[4];

	stringstream ss;
	ss << "p";

	if (size_file > 1000000)
		ss << (size_file/1000000) << "M_";
	else if (size_file > 1000)
		ss << (size_file/1000) << "K_";
	else
		ss << size_file << "_";
	ss << period << "_" << nb_blocs << ".txt";


	string file_out = ss.str();


    // Open output file
    ofstream stream_out(file_out.c_str(), ios::out | ios::trunc);
    if (!stream_out) {
        cout << "Can't open output file." << endl;
        return 0;
    }

    stream_out << size_file << endl;
    // for (int i =0; i < size_file; ++i) {
    //     stream_out << (i % period) << " ";
    // }

	int nb_blocs_total = size_file / period;
	bool pair = true;
	for (int i = 1; i < nb_blocs_total+1; ++i) {
		// cout << "bloc " << i << " pair : " << pair << endl;
		for (int j = 0; j < period ; ++j) {
			// cout << "write ";
			if (pair) {
				// cout << (i*period + j) << " "<< endl;
				stream_out << ((i*period + j) % period) << " ";
			}
			else {
				// cout << (i*period +j+1) << endl;
				stream_out << ((i*period +j+1) % period) << " ";
			}
		}
		if ((i < nb_blocs*2) &&(i%2 == 0))
			pair = !pair;
	}
}
