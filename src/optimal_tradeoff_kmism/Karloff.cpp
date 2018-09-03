/* Copyright : ???
Author : Tatiana Rocher, tatiana.rocher@gmail.com

This file contains the Karloff algorithm.
*/

#include <iostream>
#include <vector>
#include <cstdint>
#include <chrono>


using namespace std;

int findApproximatePeriod(int32_t size_pattern, char* pattern, int k) {
  int mism_so_far;
  for (int i = 1; i <= k; i++) {
    mism_so_far = 0;
    for (int j = 0; j <= size_pattern-1-i && mism_so_far <= 4*k; j++) {
      if (pattern[j] != pattern[i+j])
        mism_so_far++;
    }
    if (mism_so_far <= 4*k)
      return i;
  }
  return 0;
}

//  TODO : to change for the real Karloff's algorithm
void Karloff(int size_pattern, char *pattern, int32_t size_text, char *text,
            int nb_error_max, vecot<int32_t> pos_to_search) {

}
