#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <chrono>

#include "../Lib/flint-2.5.2/fmpq_poly.h"

using namespace std;


int main(int argc, char* argv[]) {
    // char *str, *strf, *strg;
    if (argc < 3) {
        cout << "Execution : ./exec size_polynome nb_of_iterations." << endl;
        return 0;
    }

    fmpq_poly_t f, g;
    int size_text = atoi(argv[1]);
    int nb_iter = atoi(argv[2]);

    chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> texec;

    FLINT_TEST_INIT(state);

    int r;
    float ave = 0;
    for (int j = 0; j < nb_iter; ++j) {
    fmpq_poly_init2(f, size_text);
    fmpq_poly_init2(g, size_text); 
    _fmpq_poly_set_length(f, size_text);
    _fmpq_poly_set_length(g, size_text);

        for (int i = 0; i< size_text; ++i) {
            r = random()%10;
            fmpq_poly_set_coeff_si(f, i, r);
            r = random()%10;
            fmpq_poly_set_coeff_si(g, i, r);
        }

        start = chrono::system_clock::now();
        fmpq_poly_mul(f, f, g);
        end = chrono::system_clock::now();
        texec = end - start;
        cout << "Exec : " << texec.count() << "s" << endl;
        ave += texec.count();
        fmpq_poly_clear(f);
        fmpq_poly_clear(g);
    }
    ave /= nb_iter;
    cout << "Ave axec = " << ave << endl;


    return EXIT_SUCCESS;
}

