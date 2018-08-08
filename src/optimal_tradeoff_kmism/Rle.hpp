#ifndef RLE_HPP
#define RLE_HPP


#include <list>
#include <cstdint>

using namespace std;

class RunRle {
	private:
		int position_;
		int size_;
		char letter_;

	public:
		RunRle(int p, char l);
		// ~Run_rle();

		void ExtendRunBegin(int pos);
		void ExtendRunEnd();

		char GetChar();
		int GetSize();

		void PrintRun();

};



class Rle {
	private:
		list<RunRle*> *rle_;
		int period_; //  = l, number of 
		int begin_;  // period where T* begins
		int nb_runs_;	//  total number of runs
		int *cpt_letter_run_;  // count the number of runs containing each letter
		int32_t size_;	// total number of letters

	public:
		Rle(int size, int nb_letters);
		~Rle();
		void AddLetterBegin(int pos, int period, char letter);
		void AddLetterEnd(int pos, int period, char letter);

		// construct T' and initialise i_l and i_r : 
		// search for the longest suffix T_l of T[i_l, m-1] where run_l(T_l) <= 6k
		// then the longest prefixe T_r of T[m, i_r] where run_l(T_r) <=6k
		// pad with letters at the right of T' or '#' to have a lenght divisible by period_
		void RleText(int32_t size_pattern, int32_t size_text, char *text,
						int size_max, int32_t *i_l, int32_t *i_r);

		// construct the RLE()
		void RlePattern(int32_t size_pattern, char *pattern, int32_t size_padding);

		int NbRuns();
		int NbLettersRuns(char letter);
		int32_t Size();
		void PrintRle();
		void ToString(int32_t *size, char **str);

};


#endif  // RLE_HPP
