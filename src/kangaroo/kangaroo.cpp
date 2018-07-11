/*
Authors : Tatiana Rocher (kangaroo algorithm) & geeksforgeeks.org (SA, LCP, LU building et query)
SA &  LCP : https://www.geeksforgeeks.org/%C2%AD%C2%ADkasais-algorithm-for-construction-of-lcp-array-from-suffix-array/


Compilation : g++ -std=c++11 kangaroo.cpp -o kangaroo
Execution : ./exec text pattern nb_errors -o optionalOutput

*/


// C++ program for building LCP array for given text
#include <bits/stdc++.h>
#include <unistd.h>


using namespace std;

int LIMIT = 524288;  // size of the output buffer

// Structure to store information of a suffix
struct suffix {
    int index;  // To store original index
    int rank[2];  // To store ranks and next rank pair
};


bool Usage() {
    cout << endl << "How to run: ./exec text pattern nb_errors ";
    cout << "-o optionalOutput" << endl;
    cout << "/!\\ The text (or pattern) input file must ";
    cout << "contain its lenght first, then the text (or pattern)." << endl;
    cout << "The size of the text is limited to 2^32/2 bits. ";
    cout << "If your text is longer, consider spliting it."<< endl;
    cout << "Be carefull to have only ascii characters ";
    cout << "in the text and pattern" << endl;
    cout << endl;
}

// Intialise the pattern, size_pattern and size_text  from a file
void ConcatTextPattern(string file_text, string file_pattern,
                        int32_t *size_text, int32_t *size_pattern,
                        char **text) {
    ifstream stream_text(file_text.c_str(), ios::in);
    ifstream stream_pattern(file_pattern.c_str(), ios::in);
    char character;

    if (stream_pattern && stream_text) {
        int size_tmp, size_tmp2;
        stream_text >> size_tmp;
        (*size_text) = size_tmp;
        stream_pattern >> size_tmp2;
        (*size_pattern) = size_tmp2;

        stream_text.get(character);  // eliminate the \n character
        stream_pattern.get(character);  // same
        (*text) = new char[size_tmp + size_tmp2]();
        for (int32_t i = 0; i < size_tmp; ++i) {
            stream_text.get(character);
            (*text)[i] = character;
        }
        for (int32_t i = 0; i < size_tmp2; ++i) {
            stream_pattern.get(character);
            (*text)[size_tmp + i] = character;
        }

        stream_text.close();
        stream_pattern.close();
    }
    else
        cout << "Can't open pattern file." << endl;
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

// Returns minimum of arr[L..R]
int Query(int* lcp, vector<int>* lu, int pos_start, int pos_end) {
    int j = (int)log2(pos_end-pos_start+1);

    if (lcp[lu[pos_start][j]] <= lcp[lu[pos_end - (1 << j) + 1][j]])
        return lcp[lu[pos_start][j]];
    else
        return lcp[lu[pos_end - (1 << j) + 1][j]];
}

void Kangaroo(int32_t size_text, int32_t size_pattern, int32_t size_suff_array,
                int* inv_suff_array, int* lcp, vector<int>* lu,
                int32_t size_res, int nb_error_max, int* res) {
    int32_t current_pos_p;
    int32_t current_pos_t;
    int current_nb_error;
    int pas;
    int start, end;
    for (int32_t i = 0; i < size_res; ++i) {
        current_pos_p = 0;
        current_pos_t = i;
        current_nb_error = 0;
        while (current_nb_error < nb_error_max) {
            // cout << "err : " << current_nb_error << endl;
            start = inv_suff_array[current_pos_t];
            end = inv_suff_array[current_pos_p+size_text];
            // cout << start << " " << end ;
            pas = Query(lcp, lu, min(start, end), max(start, end)-1);
            // cout << "   pas " << pas << endl;
            current_pos_t += pas+1;
            current_pos_p += pas+1;
            // cout << "current " << current_pos_t << " " << current_pos_p << endl;
            // cout << "size P " << size_pattern << endl;
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
        // cout << "error : " << res[i] << endl << endl;
    }
}

void WriteOuput(int32_t size_pattern, int32_t size_res, int *res,
                ofstream &file_out) {
    string buffer;
    buffer.reserve(LIMIT);
    string res_i_str;
    for (int32_t i = 0; i < size_res; ++i) {
        if (res[i] == -1)
            res_i_str = "N";
        else
            res_i_str = to_string(res[i]);
        if (buffer.length() + res_i_str.length() + 1 >= LIMIT) {
            file_out << buffer;
            buffer.resize(0);
        }
        buffer.append(res_i_str);
        buffer.append("\n");
    }
    file_out << buffer;
}

void printArr(int* arr, int n) {
    for (int i = 0; i < n; ++i)
        cout << arr[i] << " ";
    cout << endl;
}


int main(int argc, char* argv[]) {
    if (argc < 4) {
        Usage();
        return 0;
    }

    string file_text = argv[1];
    string file_pattern = argv[2];
    int nb_error_max = atoi(argv[3]);
    string file_out = "out.out";

    char c;
    while ((c = getopt(argc, argv, "o:")) !=EOF) {
        switch (c) {
            case 'o':
                file_out = optarg;
                break;
            default:
                Usage();
                break;
        }
    }

    chrono::time_point<chrono::system_clock> start, mid, end;
    chrono::duration<double> texec, tinit;
    start = chrono::system_clock::now();

    ofstream stream_out(file_out.c_str(), ios::out | ios::trunc);
    if (!stream_out) {
        cout << "Can't open output file." << endl;
        return 0;
    }

    int32_t size_text, size_pattern;
    char *text_pattern;
    ConcatTextPattern(file_text, file_pattern, &size_text,
                        &size_pattern, &text_pattern);

    int32_t size_suff_array = size_text+size_pattern;
    int *suff_array = new int[size_suff_array]();
    int *inv_suff_array = new int[size_suff_array]();
    end = chrono::system_clock::now();
    texec = end-start;
    cout << "   Text-Pattern : " << texec.count() << endl;
    mid = end;
    BuildSuffixArray(text_pattern, size_suff_array, &suff_array);
    end = chrono::system_clock::now();
    texec = end-mid;
    cout << "   SA : " << texec.count() << endl;
    mid = end;
    BuildInvSuffArray(size_suff_array, suff_array, &inv_suff_array);
    end = chrono::system_clock::now();
    texec = end-mid;
    cout << "   ISA : " << texec.count() << endl;
    mid = end;
    int *lcp = new int[size_suff_array]();
    Kasai(text_pattern, size_suff_array, suff_array, &lcp, inv_suff_array);
    end = chrono::system_clock::now();
    texec = end-mid;
    cout << "   LCP : " << texec.count() << endl;
    mid = end;
    // cout << "Suffix Array : \n";
    // printArr(suff_array, size_suff_array);
    // cout << "LCP Array : \n";
    // printArr(lcp, size_suff_array);

    vector<int> *lu = new vector<int>[size_suff_array]();
    BuildLU(lcp, size_suff_array, lu);
    cout << "LU Array : " << endl;
    // for (int i=0; i<size_suff_array; ++i) {
    //     for (int j=0; j<lu[i].size(); ++j)
    //         cout << lu[i][j] << " ";
    //     cout << endl;
    // }
    end = chrono::system_clock::now();
    texec = end-mid;
    cout << "   LU : " << texec.count() << endl;
    mid = end;
    end = chrono::system_clock::now();
    texec = end-start;
    cout << "Init : " << texec.count() << endl;
    mid = end;

    int32_t size_res = size_text - size_pattern + 1;
    int *res = new int[size_res]();
    Kangaroo(size_text, size_pattern, size_suff_array, inv_suff_array,
            lcp, lu, size_res, nb_error_max, res);

    end = chrono::system_clock::now();
    texec = end-mid;
    cout << "Computation : " << texec.count() << endl;
    mid = end;

    // Write in output file
    cout << "Writing results in output file: " << file_out << endl;
    WriteOuput(size_pattern, size_res, res, stream_out);

    end = chrono::system_clock::now();
    texec = end-mid;
    cout << "Writing time : " << texec.count() << endl;
    mid = end;
    texec = end-start;
    cout << "Total : " << texec.count() << endl;
    mid = end;

    return 0;
}
