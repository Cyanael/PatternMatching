#include <iostream>
#include <vector>
#include <chrono>
#include <cassert>
#include <unistd.h>

#include "Rle.hpp"
#include "Tools.hpp"
#include "Fft_wak.hpp"
#include "HammingDistance.hpp"

using namespace std;


RunRle::RunRle(char l) {
    position_ = -1;
    size_ = 1;
    letter_ = l;
}

RunRle::RunRle(RunRle *run) {
    position_ = run->GetPosition();
    size_ = run->GetSize();
    letter_ = run->GetChar();
}

void RunRle::DeleteRunBegin(int period) {
    position_ += period;
    size_--;
}

void RunRle::ReduceRun(int val) {
    if (val >= size_) {
        cout << "Reduce Run : taille du run trop petit par rapport à la réduction demandée" << endl;
        return;
    }
    size_ -= val;
}

void RunRle::ExtendRunBegin(int pos) {
    position_ = pos;
    size_ ++;
}

void RunRle::ExtendRunEnd() {
    size_++;
}


void RunRle::ExtendRunEnd(int32_t size) {
    size_+= size;
}

bool RunRle::ConcatRuns(RunRle *r) {
    if (letter_ != r->GetChar())
        return false;
    size_ += r->GetSize();
}

void RunRle::PrintRun() const {
    std::cout << "(" << size_ << ", " << letter_ << ")";
}

/******** Getter***********/

int RunRle::GetPosition() const {
    return position_;
}

void RunRle::SetPosition(int32_t p) {
    position_ = p;
}


char RunRle::GetChar() const {
    return letter_;
}

int RunRle::GetSize() const {
    return size_;
}



/************ Rle Class ************/

Rle::Rle() {
    size_ = 0;
}

Rle::Rle(int p) {
    period_ = p;
    size_ = 0;
    list<RunRle*> *rle = new list<RunRle*>[p]();
    for (int i = 0; i < period_; ++i) {
        list<RunRle*> *list_run = new list<RunRle*>();
        rle[i] = (*list_run);
    }
    rle_ = rle;
}

RleText::RleText(int p) : Rle(p) {
    list<RunRle*> *t_sec = new list<RunRle*>[p]();
    for (int i = 0; i < period_; ++i) {
        list<RunRle*> *liste = new list<RunRle*>();
        t_sec[i] = (*liste);
    }
    t_sec_ = t_sec;
}

Rle::~Rle() {
    for (int i = 0; i < period_; ++i)
        rle_[i].clear();
    delete [] rle_;
}

int RleText::AddLetterBegin(int pos, int period, char letter) {
    size_ ++;
    if (rle_[period].size() == 0) {
        RunRle *run = new RunRle(letter);
        rle_[period].push_front(run);
        return 1;
    }
    else {
        char c = rle_[period].front()->GetChar();
        if (c == letter) {
            rle_[period].front()->ExtendRunBegin(pos);
            return 0;
        }
        else {
            RunRle *run = new RunRle(letter);
            rle_[period].push_front(run);
            return 1;
        }
    }
}

int Rle::AddLetterEnd(int pos, int period, char letter) {
    size_ ++;
    if (rle_[period].size() == 0) {
        RunRle *run = new RunRle(letter);
        rle_[period].push_back(run);
        return 1;
    }
    else {
        char c = rle_[period].back()->GetChar();
        if (c == letter) {
            rle_[period].back()->ExtendRunEnd();
            return 0;
        }
        else {
            RunRle *run = new RunRle(letter);
            rle_[period].push_back(run);
            return 1;
        }
    }
}

int RleText::AddLetterEndTSec(int pos, int period, char letter) {
    if (t_sec_[period].size() == 0) {
        RunRle *run = new RunRle(letter);
        t_sec_[period].push_back(run);
        return 1;
    }
    else {
        char c = rle_[period].back()->GetChar();
        if (c == letter) {
            t_sec_[period].back()->ExtendRunEnd();
            return 0;
        }
        else {
            RunRle *run = new RunRle(letter);
            t_sec_[period].push_back(run);
            return 1;
        }
    }
}

void Rle::PrintRle() const {
    cout << "period = " << period_;
    cout << " nb of letters = " << size_ << endl;
    for (int i = 0; i < period_; ++i) {
        cout << i << " " ;
        for (list<RunRle*>::iterator it = rle_[i].begin(); it != rle_[i].end(); it++) {
            (*it)->PrintRun();
            cout << " ";
        }
        cout << endl;
    }
}

void RleText::PrintRle() const {
    Rle::PrintRle();
    for (int i = 0; i < period_; ++i) {
        cout << i << " " ;
        for (list<RunRle*>::iterator it = t_sec_[i].begin(); it != t_sec_[i].end(); it++) {
            (*it)->PrintRun();
            cout << " ";
        }
        cout << endl;
    }
}


list<RunRle*>* Rle::GetList(int period) const {
    return &rle_[period];
}


list<RunRle*>* RleText::GetListTSec(int period) const {
    return &t_sec_[period];
}



bool Usage() {
    cout << endl << "How to run: ./exec size_text size_pattern nb_runs";
    cout << " max_run pas_run -p optimalPlan" << endl;
}

void LoadSavedPlan(char* file) {
	int res = 0;
	res = fftw_import_wisdom_from_filename(file);
	if (res != 0)
		cout << "Loading plans from " << file << " succeed."<< endl;
	else
		cout << "Error while loading plans from " << file << endl;
}

void Rle::UpdateRle(int pas_run) {
    for (int i = 0; i < pas_run; ++i) {
        RunRle *run3 = new RunRle('b');
        rle_[0].push_back(run3);
        RunRle *run4 = new RunRle('a');
        rle_[0].push_back(run4);
    }
    rle_[1].front()->ReduceRun(pas_run);
}


void RleText::ConcatTSec() {
    for (int i = 0; i < period_; ++i) {
        RunRle *run = new RunRle(*(rle_[i].begin()));
        if (run->GetSize() > 1) {
            run->DeleteRunBegin(period_);
            t_sec_[i].push_back(run);
        }
        for (std::list<RunRle*>::iterator it = next(rle_[i].begin()); it != rle_[i].end(); ++it) {
            RunRle *r = new RunRle(*it);
            t_sec_[i].push_back(r);
        }
    }
    // add the last letter '#' for T"
    for (int i = 0; i < period_; ++i) {
        AddLetterEndTSec(i, i, 'b');
    }
    size_ *= 2;
}

void RleText::SetPositionRuns() {
    int32_t pos = 0;
    for (int i = 0; i < period_; ++i) {
        for (std::list<RunRle*>::iterator it = rle_[i].begin(); it != rle_[i].end(); ++it) {
            (*it)->SetPosition(pos);
            pos += (*it)->GetSize();
        }
    }
    for (int i = 0; i < period_; ++i) {
        for (std::list<RunRle*>::iterator it = t_sec_[i].begin(); it != t_sec_[i].end(); ++it) {
            (*it)->SetPosition(pos);
            pos += (*it)->GetSize();
        }
    }
}

void RleText::MakeText(int32_t size_text, int nb_runs) {
    size_ = size_text/2;
    int32_t size_to_do = min(size_text/2, nb_runs);

    RunRle *run1 = new RunRle('a');
    rle_[0].push_back(run1);
    for (int i = 0; i < size_to_do/2 -1; ++i) {
        RunRle *run3 = new RunRle('b');
        rle_[0].push_back(run3);
        RunRle *run4 = new RunRle('a');
        rle_[0].push_back(run4);
    }

    RunRle *run2 = new RunRle('b');
    if (size_to_do < size_text/2)
        run2->ExtendRunEnd(size_text/2 - size_to_do);
    rle_[1].push_back(run2);

    // concatenate T"
    ConcatTSec();
    //  puts the position where the run starts in T*
    SetPositionRuns();
    // cout << "nb runs T = " << 8*error*2 << endl;
}

void Rle::MakePattern(int32_t size_pattern, int nb_runs) {
    size_ = size_pattern;
    RunRle *run1 = new RunRle('a');
    rle_[0].push_back(run1);
    for (int i = 0; i < size_pattern/2 -1; ++i) {
        RunRle *run3 = new RunRle('b');
        rle_[0].push_back(run3);
        RunRle *run4 = new RunRle('a');
        rle_[0].push_back(run4);
    }

    RunRle *run2 = new RunRle('b');
    rle_[1].push_back(run2);
}

void Rle::DoString(int32_t *size, char **str) const {
    (*size) = size_;
    (*str) = new char[size_];
    int pos = 0;
    char letter;

    for (int i = 0; i < period_; ++i) {
        for (list<RunRle*>::iterator it=rle_[i].begin(); it != rle_[i].end(); ++it) {
            letter = (*it)->GetChar();
            for (int j = 0; j < (*it)->GetSize(); ++j) {
                (*str)[pos] = letter;
                pos++;
            }
        }
    }
}

int32_t Rle::GetSize() const {
    return size_;
}

void RleText::DoString(int32_t *size, char **str) const {
    (*size) = size_;
    (*str) = new char[size_];
    int pos = 0;
    char letter;

    for (int i = 0; i < period_; ++i) {
        for (list<RunRle*>::iterator it=rle_[i].begin(); it != rle_[i].end(); ++it) {
            letter = (*it)->GetChar();
            for (int j = 0; j < (*it)->GetSize(); ++j) {
                (*str)[pos] = letter;
                pos++;
            }
        }
    }
    for (int i = 0; i < period_; ++i) {
        for (list<RunRle*>::iterator it=t_sec_[i].begin(); it != t_sec_[i].end(); ++it) {
            letter = (*it)->GetChar();
            for (int j = 0; j < (*it)->GetSize(); ++j) {
                (*str)[pos] = letter;
                pos++;
            }
        }
    }
}

void UpdateDeriv(RunRle *runT, RunRle *runP, int32_t size_deriv, int *deriv) {
    int u, v, y, z;
    u = runT->GetPosition();
    v = u + runT->GetSize()-1;
    y = runP->GetPosition();
    z = y + runP->GetSize()-1;

    if ((u - z >= 0) && ((u - z) < size_deriv)) deriv[u - z]++;
    if ((v - z + 1 >= 0) && ((v - z + 1) < size_deriv)) deriv[v - z + 1]--;
    if ((u - y + 1 >= 0) && ((u - y + 1) < size_deriv)) deriv[u - y + 1]--;
    if ((v - y + 2 >= 0) && ((v - y + 2) < size_deriv)) deriv[v -y + 2]++;
}

int main(int argc, char* argv[]) {

    if (argc < 4) {
        Usage();
        return 0;
    }

    chrono::time_point<chrono::system_clock> start, mid, end;
    chrono::duration<double> texec;
    start = chrono::system_clock::now();
    mid = start;

    int32_t size_text = atoi(argv[1]);
    int32_t size_pattern = atoi(argv[2]);
    int nb_runs = atoi(argv[3]);
    assert(size_pattern <= nb_runs && "size_pattern doit etre plus grand ou egal au nb de runs");

    char c;
	while((c = getopt(argc, argv, "p:o:")) !=EOF) {
		switch (c) {
			case 'p':
				LoadSavedPlan(optarg);
				break;
			default:
				Usage();
				break;
		}
	}

    RleText *t_rle = new RleText(2);  // T* stored as RLE blocks
    t_rle->MakeText(size_text, nb_runs);
    // cout << "T : " << endl;
    // t_rle->PrintRle();

    Rle *p_rle = new Rle(2);  // P* stored as RLE blocks
    p_rle->MakePattern(size_pattern, nb_runs);
    // cout << "P : " << endl;
    // p_rle->PrintRle();

	vector<RunRle*> *infreq;
	infreq = new vector<RunRle*>[2];
	for (int i = 0; i < 2; ++i) {
		vector<RunRle*> l;
		infreq[i] = l;
	}

	// sort the pattern runs by letter, put them all in infreq
    for (int i = 0; i < 2; ++i) {
    	for (std::list<RunRle*>::iterator it = p_rle->GetList(i)->begin(); it != p_rle->GetList(i)->end(); ++it) {
    		c = (*it)->GetChar();
    		infreq[CharToInt(c)].push_back(*it);
    	}
    }

	//  TODO : find threshold
 //    int threshold = 1;
	vector<char> freq;
    freq.push_back('a');
    // freq.push_back('b');

    int32_t size_t_star, size_p_star;
    int32_t size_res_star = t_rle->GetSize() - p_rle->GetSize() + 1;
    int *res_hd = new int[size_res_star];

    float ave_time_hd = 0;
    int nb_loops = 10;
    for (int i = 0; i < nb_loops; ++i) {

        mid = chrono::system_clock::now();

        // //init T* and P*
        char *t_star, *p_star;
        t_rle->DoString(&size_t_star, &t_star);
        p_rle->DoString(&size_p_star, &p_star);



        // Frequent letters
        HD(size_t_star, t_star, size_p_star,  p_star, &freq,
    		size_res_star, res_hd);

        end = chrono::system_clock::now();
        texec = end-mid;
        // cout << "HD : " << texec.count() << "s" << endl;
        mid= end;
        ave_time_hd += texec.count();

    }
    ave_time_hd /= nb_loops;
    cout << "HD moyen  = " << ave_time_hd << endl;


    float ave_time_rle = 0;
    int32_t size_deriv = size_res_star;
    int *deriv = new int[size_deriv];
    int *res_star = new int[size_res_star];

    for (int i = 0; i < nb_loops; ++i) {

        mid = chrono::system_clock::now();
        // Infrequent letters
        RunRle *runP;
        InitTabZeros(size_deriv, deriv);
        InitTabZeros(size_res_star, res_star);
        int nb_pair = 0;

        // compute pair of runs in T' and P*
        for (int i = 0; i < 2; ++i) {
        	for (std::list<RunRle*>::iterator it = t_rle->GetList(i)->begin(); it != t_rle->GetList(i)->end(); ++it) {
        		c = (*it)->GetChar();
        		for (int k = 0; k < infreq[CharToInt(c)].size(); ++k) {
        			runP = infreq[CharToInt(c)][k];
        			UpdateDeriv((*it), runP, size_deriv, deriv);
                    nb_pair++;
        		}
        	}
        }
        //  do the same with T" and P*
        for (int i = 0; i < 2; ++i) {
        	for (std::list<RunRle*>::iterator it = t_rle->GetListTSec(i)->begin(); it != t_rle->GetListTSec(i)->end(); ++it) {
        		c = (*it)->GetChar();
        		for (int k = 0; k < infreq[CharToInt(c)].size(); ++k) {
        			runP = infreq[CharToInt(c)][k];
        			UpdateDeriv((*it), runP, size_deriv, deriv);
                    nb_pair++;
        		}
        	}
        }
        cout << "nb de pairs " << nb_pair << endl;

        end = chrono::system_clock::now();
        texec = end-mid;
        // cout << "Rle : " << texec.count() << "s" << endl;
        mid= end;


        // p_rle->UpdateRle(pas_run);
        ave_time_rle += texec.count();

    }


    ave_time_rle /= nb_loops*2;
    cout << "RLE moyen  = " << ave_time_rle << endl;


 //    delete [] t_star;
 //    delete [] p_star;
 //    delete [] res_star;
 //    delete [] deriv;
 //    delete [] res_hd;
 //    freq.clear();
    return 0;
}
