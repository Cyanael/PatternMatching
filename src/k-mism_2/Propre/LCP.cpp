/* Copyright : GNU GPL V3
Author : Tatiana Rocher, tatiana.rocher@gmail.com (kangaroo algorithm)
& geeksforgeeks.org (SA, LCP, LU building et query)
SA &  LCP : https://www.geeksforgeeks.org/%C2%AD%C2%ADkasais-algorithm-for-construction-of-lcp-array-from-suffix-array/


This file contains the kangaroo algorithm
used when there is more than 2*sqrt(k) frequent symbols.
*/


#include <bits/stdc++.h>
#include <unistd.h>

#include "LCP.hpp"
#include "Tools.hpp"

// Structure to store information of a suffix
struct suffix {
    int index;  // To store original index
    int rank[2];  // To store ranks and next rank pair
};

/**********Construction functions ************/

// Intialise the pattern, size_pattern and size_text  from a file
void ConcatTextPattern(int32_t size_text, char *text, int32_t size_pattern,
                        char *pattern, char **text_pattern) {
    (*text_pattern) = new char[size_text + size_pattern]();
    for (int32_t i = 0; i < size_text; ++i)
        (*text_pattern)[i] = text[i];
    for (int32_t i = 0; i < size_pattern; ++i)
        (*text_pattern)[size_text + i] = pattern[i];
}


// A comparison function used by sort() to compare two suffixes
// Compares two pairs, returns 1 if first pair is smaller
int cmp(struct suffix a, struct suffix b) {
    return (a.rank[0] == b.rank[0])? (a.rank[1] < b.rank[1] ?1: 0):
           (a.rank[0] < b.rank[0] ?1: 0);
}

// This is the main function that takes a string 'txt' of size n as an
// argument, builds and return the suffix array for the given string
void BuildSuffixArray(char *txt, int size_suff_array, int** suff_array) {
    // A structure to store suffixes and their indexes
    // struct suffix suffixes[size_suff_array];
    suffix *suffixes = new suffix[size_suff_array]();

    // Store suffixes and their indexes in an array of structures.
    // The structure is needed to sort the suffixes alphabatically
    // and maintain their old indexes while sorting
    for (int i = 0; i < size_suff_array; ++i) {
        suffixes[i].index = i;
        suffixes[i].rank[0] = txt[i] - 'a';
        suffixes[i].rank[1] = ((i+1) < size_suff_array)? (txt[i + 1] - 'a'): -1;
    }

    // Sort the suffixes using the comparison function
    // defined above.
    sort(suffixes, suffixes+size_suff_array, cmp);

    // At his point, all suffixes are sorted according to first
    // 2 characters.  Let us sort suffixes according to first 4
    // characters, then first 8 and so on
    // int ind[size_suff_array];
    // This array is needed to get the index in suffixes[]
    int *ind = new int[size_suff_array]();
    // from original index.  This mapping is needed to get
    // next suffix.
    for (int k = 4; k < 2*size_suff_array; k = k*2) {
        // Assigning rank and index values to first suffix
        int rank = 0;
        int prev_rank = suffixes[0].rank[0];
        suffixes[0].rank[0] = rank;
        ind[suffixes[0].index] = 0;

        // Assigning rank to suffixes
        for (int i = 1; i < size_suff_array; ++i) {
            // If first rank and next ranks are same as that of previous
            // suffix in array, assign the same new rank to this suffix
            if (suffixes[i].rank[0] == prev_rank &&
                    suffixes[i].rank[1] == suffixes[i-1].rank[1]) {
                prev_rank = suffixes[i].rank[0];
                suffixes[i].rank[0] = rank;
            } else {  // Otherwise increment rank and assign
                prev_rank = suffixes[i].rank[0];
                suffixes[i].rank[0] = ++rank;
            }
            ind[suffixes[i].index] = i;
        }

        // Assign next rank to every suffix
        for (int i = 0; i < size_suff_array; ++i) {
            int nextindex = suffixes[i].index + k/2;
            suffixes[i].rank[1] = (nextindex < size_suff_array)?
                                  suffixes[ind[nextindex]].rank[0]: -1;
        }

        // Sort the suffixes according to first k characters
        sort(suffixes, suffixes+size_suff_array, cmp);
    }

    // Store indexes of all sorted suffixes in the suffix array
    for (int i = 0; i < size_suff_array; i++)
        (*suff_array)[i] = suffixes[i].index;
}

void BuildInvSuffArray(int32_t size_suff_array, int* suff_array,
                        int** inv_suff_array) {
    // Fill values in invSuff[]
    for (int i=0; i < size_suff_array; i++)
        (*inv_suff_array)[suff_array[i]] = i;
}

/* To construct and return LCP */
void Kasai(char* text_pattern, int32_t size_suff_array, int* suffixArr,
            int** lcp, int* inv_suff_array) {
    // Initialize length of previous LCP
    int k = 0;

    // Process all suffixes one by one starting from
    // first suffix in txt[]
    for (int i = 0; i < size_suff_array; ++i) {
        /* If the current suffix is at n-1, then we don’t
           have next substring to consider. So lcp is not
           defined for this substring, we put zero. */
        if (inv_suff_array[i] == size_suff_array-1) {
            k = 0;
            continue;
        }

        /* j contains index of the next substring to
           be considered  to compare with the present
           substring, i.e., next string in suffix array */
        int j = suffixArr[inv_suff_array[i]+1];

        // Directly start matching from k'th index as
        // at-least k-1 characters will match
        while (i+k < size_suff_array && j+k < size_suff_array
                && text_pattern[i+k] == text_pattern[j+k])
            k++;

        (*lcp)[inv_suff_array[i]] = k;  // lcp for the present suffix.

        // Deleting the starting character from the string.
        if (k > 0)
            k--;
    }
}

// Fills lookup array lookup[][] in bottom up manner.
void BuildLU(int* lcp, int32_t size_suff_array, vector<int> *lu) {
    // Initialize M for the intervals with length 1
    for (int i = 0; i < size_suff_array; i++) {
        vector<int> v;
        lu[i] = v;
        lu[i].push_back(i);
    }

    // Compute values from smaller to bigger intervals
    for (int j = 1; (1 << j) <= size_suff_array; j++) {
        // Compute minimum value for all intervals with size 2^j
        for (int i = 0; (i + (1 << j)-1) < size_suff_array; i++) {
            // For arr[2][10], we compare arr[lookup[0][3]] and
            // arr[lookup[3][3]]
            if (lcp[lu[i][j-1]] < lcp[lu[i + (1 << (j-1))][j-1]])
                lu[i].push_back(lu[i][j-1]);
            else
                lu[i].push_back(lu[i + (1 << (j-1))][j-1]);
        }
    }
}


/********** Queries *************/

// Returns minimum of arr[L..R]
int Query(int* lcp, vector<int>* lu, int pos_start, int pos_end) {
    int j = floor(log2(pos_end-pos_start+1));

    if (lcp[lu[pos_start][j]] <= lcp[lu[pos_end - (1 << j) + 1][j]])
        return lcp[lu[pos_start][j]];
    else
        return lcp[lu[pos_end - (1 << j) + 1][j]];
}

void Kangaroo(int32_t size_text, int32_t size_pattern, int32_t size_suff_array,
                int* inv_suff_array, int* lcp, vector<int>* lu,
                int32_t size_res, int nb_error_max, int *dk, int* res) {
    int32_t current_pos_p;
    int32_t current_pos_t;
    int current_nb_error;
    int pas;
    int start, end;
    for (int32_t i = 0; i < size_res; ++i) {
        if (dk[i] < nb_error_max)
            res[i] = -1;
        else {
            current_pos_p = 0;
            current_pos_t = i;
            current_nb_error = 0;
            while (current_nb_error < nb_error_max) {
                start = inv_suff_array[current_pos_t];
                end = inv_suff_array[current_pos_p+size_text];
                pas = Query(lcp, lu, min(start, end), max(start, end)-1);
                current_pos_t += pas+1;
                current_pos_p += pas+1;
                if (current_pos_p >= size_pattern)
                    break;
                current_nb_error++;
            }

            if (current_pos_p < size_pattern)
                res[i] = -1;
            else if (current_pos_p == size_pattern)
                res[i] = current_nb_error+1;
            else
                res[i] = current_nb_error;
        }
    }
}


bool IsFreq(char letter, vector<int32_t> *freqChar) {
    if (freqChar[CharToInt(letter)].size() > 0)
        return true;
    return false;
}


void SortfreqInfreqCaract(int32_t size_pattern, char *pattern,
                        float threshold_freq, vector<int32_t> *infreq,
                        int size_alphabet) {
    int nb_char_selected = 0, i = 0;

    for (int i= 0; i< size_alphabet; ++i) {
        if (infreq[i].size() >= threshold_freq &&
            nb_char_selected < 2*threshold_freq) {
            infreq[i].erase(infreq[i].begin() + ceil(threshold_freq),
                            infreq[i].end());
            nb_char_selected++;
        } else
            infreq[i].clear();
    }
}


void InterestingPosition(int32_t size_text, char *text,
                        vector<int32_t> *freqChar, float threshold_freq,
                        int *dk) {
int pos_in_pat;
    for (int i = 0; i < size_text; ++i) {
        if (IsFreq(text[i], freqChar))
            for (int j = 0; j < freqChar[CharToInt(text[i])].size() ; ++j) {
                pos_in_pat = freqChar[CharToInt(text[i])][j];
                if (i - pos_in_pat >=0) {
                    dk[i - pos_in_pat]++;
                }
            }
    }
}


void ComputeLCP(int32_t size_text, char *text, int32_t size_pattern,
                        char *pattern, vector<int32_t> *freqChar,
                        float threshold_freq, int size_alphabet,
                        int nb_error_max, int32_t size_res, int *res) {
    SortfreqInfreqCaract(size_pattern, pattern, threshold_freq,
                        freqChar, size_alphabet);

    char *text_pattern;
    ConcatTextPattern(size_text, text, size_pattern, pattern, &text_pattern);

    int32_t size_suff_array = size_text+size_pattern;
    int *suff_array = new int[size_suff_array]();
    int *inv_suff_array = new int[size_suff_array]();

    BuildSuffixArray(text_pattern, size_suff_array, &suff_array);
    BuildInvSuffArray(size_suff_array, suff_array, &inv_suff_array);

    int *lcp = new int[size_suff_array]();
    Kasai(text_pattern, size_suff_array, suff_array, &lcp, inv_suff_array);

    vector<int> *lu = new vector<int>[size_suff_array]();
    BuildLU(lcp, size_suff_array, lu);

    int *dk = new int[size_res]();
    InterestingPosition(size_text, text, freqChar, threshold_freq, dk);

    Kangaroo(size_text, size_pattern, size_suff_array, inv_suff_array, lcp,
                lu, size_res, nb_error_max, dk, res);
}
