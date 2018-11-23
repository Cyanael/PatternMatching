/* Copyright : GNU GPL V3
Authors : Tatiana Rocher, tatiana.rocher@gmail.com (kangaroo algorithm)
& geeksforgeeks.org (SA, LCP, LU building et query)
SA &  LCP : https://www.geeksforgeeks.org/%C2%AD%C2%ADkasais-algorithm-for-construction-of-lcp-array-from-suffix-array/
*/

#ifndef LCP_HPP
#define LCP_HPP

#include <vector>
#include <algorithm>

using namespace std;


/*** Construction ***/

void ConcatTextPattern(string file_text, string file_pattern,
                       int32_t *size_text, int32_t *size_pattern, char **text);

int cmp(struct suffix a, struct suffix b);

void BuildSuffixArray(char *txt, int size_suff_array, int** suff_array);

void BuildInvSuffArray(int32_t size_suff_array, int* suff_array,
                        int** inv_suff_array);

// Build the structur who computes the LCP queries
void Kasai(char* text_pattern, int32_t size_suff_array, int* suffixArr,
            int** lcp, int* inv_suff_array);

void BuildLU(int* lcp, int32_t size_suff_array, vector<int> *lu);

bool IsFreq(char letter, vector<int32_t> *freqChar);

// input : infreq contains the occurences of all pattern letters
// Select sqrt(k) occurrence of 2*sqrt(k) frequent letters (in infreq)
void SortfreqInfreqCaract(int32_t size_pattern, char *pattern, float limit,
                            vector<int32_t> *infreq, int size_alphabet);


/*** Queries ***/

// Count the number of frequent letters who match at every position
void InterestingPosition(int32_t size_text, char *text, vector<int32_t> *freqChar,
                        float threshold_freq, int *dk);

int Query(int* lcp, vector<int>* lu, int pos_start, int pos_end);

// Do the computation on the position where more than k frequent letters match
void Kangaroo(int32_t size_text, int32_t size_pattern, int32_t size_suff_array,
                int* inv_suff_array, int* lcp, vector<int>* lu,
                int32_t size_res, int nb_error_max, int* res);

void ComputeLCP(int32_t size_text, char *text, int32_t size_pattern,
                        char *pattern, vector<int32_t> *infrequent, float threshold,
                        int size_alphabet, int nb_errors, int32_t size_res, int *res);


#endif  // LCP_HPP
