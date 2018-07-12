#ifndef LCP_HPP
#define LCP_HPP

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
                int32_t size_res, int nb_error_max, int* res);

bool IsFreq(char letter, vector<int32_t> *freqChar);

void SortfreqInfreqCaract(int32_t size_pattern, char *pattern, float limit,
                            vector<int32_t> *infreq, int size_alphabet);

void InterestingPosition(int32_t size_text, char *text, vector<int32_t> *freqChar,
                        float threshold_freq, int *dk);

void ComputeLCP(int32_t size_text, char *text, int32_t size_pattern, 
                        char *pattern, vector<int32_t> *infrequent, float threshold,
                        int size_alphabet, int nb_errors, int32_t size_res, int *res);


#endif  // LCP_HPP