#ifndef RLE_HPP
#define RLE_HPP


#include <list>
#include <cstdint>

using namespace std;

/***** Run RLE *******/
//  ex : aaaaaa = RunRle(0, 6, a)


class RunRle {
	private:
		int32_t position_; // where the run starts in the text/pattern 
		int size_;
		char letter_;

	public:
		RunRle(char l);
		RunRle(RunRle *run);
		~RunRle();

		void DeleteRunBegin(int period);
		void ExtendRunBegin(int pos);
		void ExtendRunEnd();
		bool ConcatRuns(RunRle *r);


		int GetPosition() const;
		void SetPosition(int32_t p);
		char GetChar() const;
		int GetSize() const;

		void PrintRun() const;

};



class Rle {
	protected:
		list<RunRle*> *rle_;
		int period_; //  = l
		int32_t size_;	// total number of letters

	public:
		Rle();
		Rle(int period);
		~Rle();
		
		// return 1 if the function add a run, 0 else
		int AddLetterEnd(int pos, int period, char letter);

		// Put the starting position of every run in the reading order 
		void SetPositionRuns();

		// construct the RLE()
		void MakeRle(int32_t size_pattern, char *pattern, int32_t size_total);

		list<RunRle*>* GetList(int period) const;
		
		// total number of letter in the class
		int32_t GetSize() const;
		int GetPeriod() const;

		//  nb of runs in the pos_periodth list 
		int GetSizeList(int pos_period) const; 
		void PrintRle() const;
		void DoString(int32_t *size, char **str) const;
};



class RleText : public Rle {
	private:
			list<RunRle*> *t_sec_;

	public:
		RleText(int size);
		~RleText();

		//  return 1 if the function add a run, 0 else
		int AddLetterBegin(int pos, int period, char letter);
		int AddLetterEndTSec(int pos, int period, char letter);

		// reorganise the lists of run so the first letter is in rle_[0]
		void ReorganiseListRun(int32_t step);

		// add T" after T' to construct T*
		void ConcatTSec();

		// construct T' and initialise i_l and i_r : 
		// search for the longest suffix T_l of T[i_l, m-1] where run_l(T_l) <= 6k
		// then the longest prefixe T_r of T[m, i_r] where run_l(T_r) <=6k
		// pad with '#' to have a lenght divisible by period_
		void MakeRle(int32_t size_pattern, int32_t size_text, char *text,
						int size_max, int32_t *i_l, int32_t *i_r, int32_t *m1);

		void SetPositionRuns();
		list<RunRle*>* GetListTSec(int period) const;

		void PrintRle() const;
		void DoString(int32_t *size, char **str) const;

};


void Small8kPeriod(int32_t size_text, char *text, int32_t size_pattern,
					char *pattern, int k_nb_letters, int error_k, int approx_period,
					int32_t size_res, int *res);

void ComputeResStar(char* text, int32_t size_pattern, char* pattern,
					int nb_freq_letters, int nb_$,
					int32_t size_deriv, int* deriv, int* res_hd,
					int32_t size_res, int* res);

void ReorganiseRes(int32_t i_l, int32_t i_r, int period,
					int32_t size_res_star, int* res_star, int32_t size_res, int* res);

void UpdateDeriv(RunRle *runT, RunRle *runP, int *deriv);


#endif  // RLE_HPP
