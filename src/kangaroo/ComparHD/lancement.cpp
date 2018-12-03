/*
Authors : Tatiana Rocher (kangaroo algorithm) & geeksforgeeks.org (SA, LCP, LU building et query)
SA &  LCP : https://www.geeksforgeeks.org/%C2%AD%C2%ADkasais-algorithm-for-construction-of-lcp-array-from-suffix-array/


Compilation : g++ -std=c++11 kangaroo.cpp -o kangaroo
Execution : ./exec text pattern nb_errors -o optionalOutput

*/


// C++ program for building LCP array for given text
#include <bits/stdc++.h>
#include <unistd.h>

#include "HamDist.hpp"


using namespace std;

int LIMIT = 524288;  // size of the output buffer

// Structure to store information of a suffix
struct suffix {
    int index;  // To store original index
    int rank[2];  // To store ranks and next rank pair
};


void LoadSavedPlan(char* file) {
    int res=0;
    res = fftw_import_wisdom_from_filename(file);
    if (res != 0)
        cout << "Loading plans from " << file << " succeed."<< endl;
    else
        cout << "Error while loading plans from " << file << endl;
}


bool Usage() {
    cout << endl << "How to run: ./exec text size_pattern nb_errors ";
    cout << "-o optionalOutput" << endl;
    cout << "/!\\ The text (or pattern) input file must ";
    cout << "contain its lenght first, then the text (or pattern)." << endl;
    cout << "The size of the text is limited to 2^32/2 bits. ";
    cout << "If your text is longer, consider spliting it."<< endl;
    cout << "Be carefull to have only ascii characters ";
    cout << "in the text and pattern" << endl;
    cout << endl;
}

void InitTable(int32_t size, int *table) {
    for (int i = 0; i < size; ++i)
        table[i] = 0;
}

// Intialise the pattern, size_pattern and size_text  from a file
void ConcatTextPattern(string file_text, int32_t *size_text,
                    int32_t size_pattern, char **text_pattern, char **text) {
    ifstream stream_text(file_text.c_str(), ios::in);
    char character;

    if (stream_text) {
        int size_tmp, size_tmp2;
        stream_text >> size_tmp;
        (*size_text) = size_tmp;

        stream_text.get(character);  // eliminate the \n character
        (*text) = new char[size_tmp];
        (*text_pattern) = new char[size_tmp + size_tmp2]();
        for (int32_t i = 0; i < size_tmp; ++i) {
            stream_text.get(character);
            (*text)[i] = character;
            (*text_pattern)[i] = character;
        }
        stream_text.close();

        for (int32_t i = 0; i < size_pattern; ++i) {
            (*text_pattern)[(*size_text) + i] = 'a';
        }
    }
    else
        cout << "Can't open pattern file." << endl;
}

void LoadPattern(int32_t size_text, char *text_pattern, int32_t size_pattern,
                char **pattern) {

    (*pattern) = new char[size_pattern]();
    struct timespec ts;
    if (timespec_get(&ts, TIME_UTC) == 0) {
        cout << "error seed." << endl;
    }
    srandom(ts.tv_nsec ^ ts.tv_sec); // compute seed

    int32_t begin = random()%(size_text - size_pattern);
    for (int32_t i = 0; i < size_pattern; ++i){
        (*pattern)[i] = text_pattern[begin + i];
        text_pattern[size_text + i] = text_pattern[begin + i];
    }
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

    delete [] ind;
    delete [] suffixes;
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

// Returns minimum of arr[L..R]
int Query(int* lcp, vector<int>* lu, int pos_start, int pos_end) {
    int j = (int)log2(pos_end-pos_start+1);

    if (lcp[lu[pos_start][j]] <= lcp[lu[pos_end - (1 << j) + 1][j]])
        return lcp[lu[pos_start][j]];
    else
        return lcp[lu[pos_end - (1 << j) + 1][j]];
}

int Kangaroo(int32_t size_text, int32_t size_pattern, int32_t size_suff_array,
                int* inv_suff_array, int* lcp, vector<int>* lu,
                int32_t size_res, int nb_error_max, int* res) {
    int32_t current_pos_p;
    int32_t current_pos_t;
    int current_nb_error;
    int pas;
    int start, end;
    int nb_matchs = 0;
    for (int32_t i = 0; i < size_res; ++i) {
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
        else {
            nb_matchs++;
            if (current_pos_p == size_pattern)
                res[i] = current_nb_error+1;
            else
                res[i] = current_nb_error;
            }
    }
    return nb_matchs;
}



int main(int argc, char* argv[]) {
    if (argc < 4) {
        Usage();
        return 0;
    }

    chrono::time_point<chrono::system_clock> start, mid, end;
    chrono::duration<double> texec, tinit;
    start = chrono::system_clock::now();

    string file_text = argv[1];
    int32_t size_pattern = atoi(argv[2]);
    int nb_error_max = atoi(argv[3]);
    string file_out = "out.out";

    char c;
    while ((c = getopt(argc, argv, "o:p:")) !=EOF) {
        switch (c) {
            case 'o':
                file_out = optarg;
                break;
            case 'p':
                LoadSavedPlan(optarg);
                break;
            default:
                Usage();
                break;
        }
    }


    ofstream stream_out(file_out.c_str(), ios::out | ios::trunc);
    if (!stream_out) {
        cout << "Can't open output file." << endl;
        return 0;
    }

    int32_t size_text;
    char *text_pattern, *text, *pattern;

    float time_SA = 0, time_ISA  = 0, time_LCP = 0, time_naif = 0, time_HD = 0;
    float time_LU = 0, time_init = 0, time_comput = 0, time_tot = 0;

    ConcatTextPattern(file_text, &size_text, size_pattern, &text_pattern, &text);

    int32_t size_suff_array = size_text+size_pattern;
    int32_t size_res = size_text - size_pattern + 1;

    int *res = new int[size_res]();
    int *res_naif = new int[size_res]();
    int *suff_array = new int[size_suff_array]();
    int *inv_suff_array = new int[size_suff_array]();
    int *lcp = new int[size_suff_array]();
    vector<int> *lu = new vector<int>[size_suff_array]();

    end = chrono::system_clock::now();
    tinit = end-start;
    cout << "Init : " << tinit.count() << endl;

    int nb_loops = 20;
    for (int i=0; i<nb_loops; ++i) {

        LoadPattern(size_text, text_pattern, size_pattern, &pattern);

        // InitTable(size_suff_array, suff_array);
        // InitTable(size_suff_array, inv_suff_array);
        // InitTable(size_suff_array, lcp);
        // InitTable(size_res, res);
        // for (int j = 0; j < size_suff_array; ++j)
        //     lu[j].clear();
        //
        //
        // // cout << "textpat : " << size_suff_array << endl ;
        // // for (int j = 0; j < size_suff_array; ++j)
        // //     cout << text_pattern[j];
        // // cout << endl;
        //
        // start = chrono::system_clock::now();
        //
        // BuildSuffixArray(text_pattern, size_suff_array, &suff_array);
        //
        // end = chrono::system_clock::now();
        // texec = end-start;
        // // cout << "   SA : " << texec.count() << endl;
        // mid = end;
        // time_SA += texec.count();
        //
        // BuildInvSuffArray(size_suff_array, suff_array, &inv_suff_array);
        //
        // end = chrono::system_clock::now();
        // texec = end-mid;
        // // cout << "   ISA : " << texec.count() << endl;
        // mid = end;
        // time_ISA += texec.count();
        //
        // Kasai(text_pattern, size_suff_array, suff_array, &lcp, inv_suff_array);
        //
        // end = chrono::system_clock::now();
        // texec = end-mid;
        // // cout << "   LCP : " << texec.count() << endl;
        // mid = end;
        // time_LCP += texec.count();
        //
        // BuildLU(lcp, size_suff_array, lu);
        //
        // end = chrono::system_clock::now();
        // texec = end-mid;
        // // cout << "   LU : " << texec.count() << endl;
        // mid = end;
        // time_LU += texec.count();
        // end = chrono::system_clock::now();
        // texec = end-start;
        // // cout << "Init : " << texec.count() << endl;
        // mid = end;
        // time_init += texec.count();
        //
        // int nb_matches = Kangaroo(size_text, size_pattern, size_suff_array,
        //                 inv_suff_array, lcp, lu, size_res, nb_error_max, res);
        //
        end = chrono::system_clock::now();
        // texec = end-mid;
        // // cout << "Computation : " << texec.count() << endl;
        // // cout << "Nb matches = " << nb_matches << endl;
        mid = end;
        // time_comput += texec.count();
        //
        // texec = end-start;
        // cout << "Total : " << texec.count() + tinit.count() << endl;
        // time_tot += texec.count() + tinit.count();


        // naive algo
        int curr_error, pos_p;
        for (int j = 0; j < size_res; ++j) {
            curr_error = 0;
            pos_p = 0;
            while (curr_error < nb_error_max && pos_p < size_pattern) {
    			if (pattern[pos_p]!=text[j + pos_p]){
    				curr_error++;
    			}
    			pos_p++;
    		}

            if (curr_error >= nb_error_max)
                res_naif[j] = -1;
            else
                res_naif[j] = curr_error;
        }

        end = chrono::system_clock::now();
        texec = end-mid;
        cout << "Naif : " << texec.count() + tinit.count() << endl;
        time_naif += texec.count() + tinit.count();

        bool verif = true;
        int pos = 0;
        while (verif && pos < size_res) {
            if (res[pos] != res_naif[pos])
                verif = false;
            pos++;
        }
        if (verif)
            cout << "== ";
        else
            cout << "!= ";


        mid = chrono::system_clock::now();

        HD(size_text, text, size_pattern, pattern, size_res, res_naif);

        end = chrono::system_clock::now();
        texec = end-mid;
        cout << "HD : " << texec.count() + tinit.count() << endl;
        time_HD += texec.count() + tinit.count();

        verif = true;
        pos = 0;
        while (verif && pos < size_res) {
            if (res[pos] != (size_pattern - res_naif[pos]))
                verif = false;
            pos++;
        }
        if (verif)
            cout << "== ";
        else
            cout << "!= ";



    }
    delete [] lcp;
    delete [] res;
    for (int i=0; i<size_suff_array; ++i)
    lu[i].clear();
    delete [] lu;
    delete [] text_pattern;
    delete [] suff_array;
    delete [] inv_suff_array;

    time_SA /= nb_loops;
    time_ISA  /= nb_loops;
    time_LCP /= nb_loops;
    time_LU /= nb_loops;
    time_init /= nb_loops;
    time_comput /= nb_loops;
    time_tot /= nb_loops;
    time_naif /= nb_loops;
    time_HD /= nb_loops;

    cout << "Av SA : " << time_SA << endl;
    cout << "Av ISA : " << time_ISA << endl;
    cout << "Av LCP : " << time_LCP << endl;
    cout << "Av LU : " << time_LU << endl;
    cout << "Av init : " << time_init << endl;
    cout << "Av comput : " << time_comput << endl;
    cout << "Av tot : " << time_tot << endl;
    cout << "Av naif : " << time_naif << endl;
    cout << "Av HD : " << time_HD << endl;

    return 0;
}
