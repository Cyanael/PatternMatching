#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <chrono>

#include "../Lib/flint-2.5.2/fmpz_poly.h"
#include <gmp.h>

#include "../Lib/flint-2.5.2/flint.h"
#include "../Lib/flint-2.5.2/fmpz.h"
#include "../Lib/flint-2.5.2/fmpz_poly.h"
#include "../Lib/flint-2.5.2/fmpz_poly_q.h"

using namespace std;

int main(int argc, char* argv[]) {
   if (argc < 3) {
        cout << "Execution : ./exec size_polynome nb_of_iterations." << endl;
        return 0;
    }

    int size_text = atoi(argv[1]);
    int nb_iter = atoi(argv[2]);
    fmpz_poly_t f, g;

    chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> texec;

    fmpz_poly_init(f);
    fmpz_poly_init(g);

    int r;
    float ave = 0;
    for (int j = 0; j < nb_iter; ++j) {
    fmpz_poly_init2(f, size_text);
    fmpz_poly_init2(g, size_text); 
    _fmpz_poly_set_length(f, size_text);
    _fmpz_poly_set_length(g, size_text);

        for (int i = 0; i< size_text; ++i) {
            r = random()%10;
            fmpz_poly_set_coeff_si(f, i, r);
            r = random()%10;
            fmpz_poly_set_coeff_si(g, i, r);
        }

        // fmpz_poly_q_set_str(f, "2  1 3/1  2");
        // fmpz_poly_q_set_str(g, "1  3/2  2 7");

        start = chrono::system_clock::now();
        fmpz_poly_mul(f, f, g);
        end = chrono::system_clock::now();
        texec = end - start;
        cout << "Exec : " << texec.count() << "s" << endl;
        ave += texec.count();

        fmpz_poly_clear(f);
        fmpz_poly_clear(g);
    }
    ave /= nb_iter;
    cout << "Ave axec = " << ave << endl;


    return EXIT_SUCCESS;
}

