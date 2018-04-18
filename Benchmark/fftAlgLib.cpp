// #include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "./alglib/src/fasttransforms.h"

// compilation :
    // g++ fftAlgLib.cpp alglib/src/fasttransforms.cpp alglib/src/ap.cpp alglib/src/alglibinternal.cpp -o al


using namespace std;	
using namespace alglib;


int main(int argc, char **argv)
{
    //
    // first we demonstrate forward FFT:
    // [1i,1i,1i,1i] is converted to [4i, 0, 0, 0]
    //
    // complex_1d_array z = "[0,0,1,-1]";
    // fftc1d(z);

    real_1d_array x = "[0,0,1,-1]";
    complex_1d_array z;
    fftr1d(x, z);

    cout << z.tostring(3).c_str() << endl; // EXPECTED: [4i,0,0,0]

    //
    // now we convert [4i, 0, 0, 0] back to [1i,1i,1i,1i]
    // with backward FFT
    //
    // fftc1dinv(z);
    // printf("%s\n", z.tostring(3).c_str()); // EXPECTED: [1i,1i,1i,1i]
    return 0;
}
