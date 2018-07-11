#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <chrono>

#include "../Lib/flint-2.5.2/fq_poly.h"

using namespace std;


int main(int argc, char* argv[]) {
    // char *str, *strf, *strg;
    if (argc < 3) {
        cout << "Execution : ./exec size_polynome nb_of_iterations." << endl;
        return 0;
    }

    fq_poly_t f, g;
    fq_ctx_t ctx;
    long d;
    fmpz_t p;
    int size_text = atoi(argv[1]);
    int nb_iter = atoi(argv[2]);
    FLINT_TEST_INIT(state);

    chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> texec;

    int r;
    float ave = 0;
    fmpz_init_set_ui(p, 3);
    d = 2;
    fq_ctx_init_conway(ctx, p, d, "X");

    for (int j = 0; j < nb_iter; ++j) {
    fq_poly_init2(f, size_text, ctx);
    fq_poly_init2(g, size_text, ctx); 
    fq_poly_randtest(f, state, size_text, ctx);
    fq_poly_randtest(g, state, size_text, ctx);
    // for (int i = 0; i< size_text; ++i) {
    //         r = random()%2;
    //         fq_poly_set_coeff(f, i, r, ctx);
    //         r = random()%2;
    //         fq_poly_set_coeff(g, i, r, ctx);
    //     }


    start = chrono::system_clock::now();
    // fq_poly_mul_classical(f, f, g, ctx);
    // fq_poly_mul_reorder(f, g, f, ctx);
    fq_poly_mul_KS(f, g, f, ctx);
    end = chrono::system_clock::now();
    texec = end - start;
    cout << "Exec : " << texec.count() << "s" << endl;
    ave += texec.count();
    fq_poly_clear(f, ctx);
    fq_poly_clear(g, ctx);
    FLINT_TEST_CLEANUP(state);
    }

    ave /= nb_iter;
    cout << "Ave axec = " << ave << endl;


    return EXIT_SUCCESS;
}

