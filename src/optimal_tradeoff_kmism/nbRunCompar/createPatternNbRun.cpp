#include <iostream>
#include <fstream>


using namespace std;

bool Usage() {
	cout << endl << "How to run: ./exec size_file period file_out" << endl;
}


int main(int argc, char* argv[]) {
    if (argc < 4) {
        Usage();
        return 0;
    }

    int32_t size_file = atoi(argv[1]);
    int period = atoi(argv[2]);
	string file_out = argv[3];

    // Open output file
    ofstream stream_out(file_out.c_str(), ios::out | ios::trunc);
    if (!stream_out) {
        cout << "Can't open output file." << endl;
        return 0;
    }

    stream_out << size_file << endl;
    for (int i =0; i < size_file; ++i) {
        stream_out << (i % period) << " ";
    }
}
