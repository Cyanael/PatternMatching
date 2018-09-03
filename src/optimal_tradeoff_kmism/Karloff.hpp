/* Copyright : ???
Author : Tatiana Rocher, tatiana.rocher@gmail.com

This file contains the Karloff algorithm.
*/

#ifndef KARLOFF_H
#define KARLOFF_H

//  Look for the approximate period in the pattern
//  Run until k or found
int findApproximatePeriod(int32_t size_pattern, char* pattern, int k);

//  Used when there is no small period (2k-period > k)
//  Put the position when HD(i) <= k in pos_to_search for a futur LCP search
void Karloff(int size_pattern, char *pattern, int32_t size_text, char *text,
            int nb_error_max, vecot<int32_t> pos_to_search);

#endif  // KARLOFF_K
