/* Copyright : GNU GPL V3
Author : Tatiana Rocher, tatiana.rocher@gmail.com
*/

#ifndef LCP_HPP
#define LCP_HPP

#include <vector>
#include <algorithm>

using namespace std;


void ConcatTextPattern(string file_text, string file_pattern,
                       int32_t *size_text, int32_t *size_pattern, char **text);

int cmp(struct suffix a, struct suffix b);

void BuildSuffixArray(char *txt, int size_suff_array, int** suff_array);

void BuildInvSuffArray(int32_t size_suff_array, int* suff_array,
                        int** inv_suff_array);

void Kasai(char* text_pattern, int32_t size_suff_array, int* suffixArr,
            int** lcp, int* inv_suff_array);

void BuildLU(int* lcp, int32_t size_suff_array, vector<int> *lu);

int Query(int* lcp, vector<int>* lu, int pos_start, int pos_end);

void Kangaroo(int32_t size_text, int32_t size_pattern, int32_t size_suff_array,
                int* inv_suff_array, int* lcp, vector<int>* lu,
                int32_t size_res, int nb_error_max, vector<int32_t> pos_to_search, int* res);

void LCP(int32_t size_text, char *text, int32_t size_pattern,
                        char *pattern, int size_alphabet,
                        int nb_error_max, vector<int32_t> pos_to_search,
                        int32_t size_res, int *res);


#endif  // LCP_HPP
