// Author : Bogdan Manghiuc
// Modified by Tatiana Rocher


// Algorithm that solves an instance of the k-mismatch problem when the pattern (P)
// has approximate period l <= k
// We assume that |P| = m
// We assume that |T| = 2m
// INPUT Text: text
// INPUT Pattern: pattern k with |text| = 2 * |pattern|

//RUN : ./exec fileText filePattern fileOutput

#include <bits/stdc++.h>
#include <fstream>
using namespace std;

int i_left;
int i_right;
int pattern_size;
int approximate_period;

struct RLE_instance{
  int position;
  char label;
  int dimension;
};
struct Corner{
  int x;
  int y;
  bool influence;
};
struct DELTA{
  int previous;
  int current;
};
struct HAMMING{
  int previous;
  int current;
};

struct i_star_instance{
  int value;
  pair<int, int> instance;
};
struct Compare_corners{
  int diag1, diag2;
  bool operator()(const Corner& qa,const Corner& qb){
    diag1 = pattern_size - 1 - qa.y + qa.x;
    diag2 = pattern_size - 1 - qb.y + qb.x;
    return diag1 > diag2;
  }
};
struct A_RLE{
  priority_queue<Corner, vector<Corner>, Compare_corners> corners;
  queue<RLE_instance> new_runs;
  DELTA delta;
  HAMMING ham;
  int correction;
  int current_diagonal;
};

struct Compare_i_star{
  bool operator()(const i_star_instance& i1, const i_star_instance& i2){
    return i1.value > i2.value;
  }
};

//RLE encoding of the text
vector<deque<RLE_instance> > text_RLE;
//RLE encoding of the pattern
vector<vector<RLE_instance> > pattern_rle;
//The number of runs at a current position in the RLE encoding of the text
int runs;
//The length of each list in the RLE encoding of the text
vector<int> text_runs_length;
//Same as above but for the pattern
vector<int> pattern_runs_length;
//The position in T where T_L starts
vector<priority_queue<i_star_instance,
                      vector<i_star_instance>,
                      Compare_i_star> >  i_stars;
deque<pair<int, int> > last_l_HAM;
vector<vector<A_RLE*> > instance_A_RLE;












//******************************INITIAlISATION**********************************
//******************************************************************************
void add_character(int i, char c){
  int r = i % approximate_period;
  RLE_instance new_pair;
  int pos;
  if(text_runs_length[r] == 0){
    new_pair.position = i;
    new_pair.label    = c;
    new_pair.dimension = 1;
    text_RLE[r].push_back(new_pair);
    text_runs_length[r] ++ ;
    runs++;
  } else {
    pos = text_runs_length[r] - 1;
    if(text_RLE[r][pos].label != c){

      new_pair.position = i;
      new_pair.label    = c;
      new_pair.dimension = 1;
      text_RLE[r].push_back(new_pair);
      text_runs_length[r]++;
      runs++;
    } else {
      text_RLE[r][pos].dimension ++;
    }
  }
}
bool compute_i_right(int i, char c, int k){
  add_character(i, c);
  if(runs > 10 * k){
    i_right = i-1;
    return true;
  }
  return false;
}
void readText(string in1, string &text){
  ifstream fileText(in1, ios::in);
  getline(fileText, text);
  fileText.close();
}

void readPattern(string in2, string &pattern, int &k){
  ifstream filePattern(in2, ios::in);
  getline(filePattern, pattern);

  string k_str;
  getline(filePattern, k_str);
  k = atoi(k_str.c_str());

  pattern_size = (int) pattern.size();
  
  filePattern.close();
}

void find_approximate_period(string pattern, int k){
  int l=0;
  int mism_so_far;
  bool found_approximate_period = false;
  for(int i = 1; i <= k && found_approximate_period == false; i++){
    mism_so_far = 0;
    for(int j = 0; j <= pattern_size-1-i && mism_so_far <= 3*k; j++){
      if(pattern[j] != pattern[i+j])
        mism_so_far ++;
    }
    if(mism_so_far <= 3*k){
      l = i;
      found_approximate_period = true;
    }
  }
  approximate_period = l;
}
void initialise_lengths(){
  deque<RLE_instance> init;
  init.clear();
  for(int i = 0; i < approximate_period; i++){
    text_runs_length.push_back(0);
  }
  for(int i = 0; i < approximate_period; i++){
    text_RLE.push_back(init);
  }
}
void initialise(){
  text_RLE.clear();
  text_runs_length.clear();
  runs = 0;
  i_left = 0;
  initialise_lengths();
}
void encode_pattern(string pattern){
  RLE_instance new_run;
  vector<RLE_instance> pat;
  int no_of_runs;
  for(int i=0; i<approximate_period; i++){
    pat.clear();
    no_of_runs = 0;
    for(int j = i; j < pattern_size; j+=approximate_period){
      if(j < approximate_period){
        new_run.position = j;
        new_run.label = pattern[j];
        new_run.dimension = 1;
        pat.push_back(new_run);
        no_of_runs++;
      } else {
        if(pat[no_of_runs - 1].label == pattern[j]){
          pat[no_of_runs - 1].dimension ++;
        } else {
          new_run.position = j;
          new_run.label = pattern[j];
          new_run.dimension = 1;
          pat.push_back(new_run);
          no_of_runs++;
        }
      }
    }
    pattern_rle.push_back(pat);
    pattern_runs_length.push_back(no_of_runs);
  }
}
//******************************************************************************
//******************************************************************************








//******************************SETUP PHASE*************************************
//******************************************************************************
void check_runs(int k){
  if(runs > 5*k){
    int r = i_left % approximate_period;
    if(text_runs_length[r] == 0){
      cout<<"Error check runs";
      exit(EXIT_FAILURE);
    } else if(text_runs_length[r] == 1){
      text_RLE[r].pop_front();
      text_runs_length[r]--;
      runs--;
      i_left++;
      check_runs(k);
    } else if(text_runs_length[r] >= 2){
      int diff = (text_RLE[r][1].position - text_RLE[r][0].position)/approximate_period;
      if(diff == 1){
        text_RLE[r].pop_front();
        text_runs_length[r]--;
        runs--;
        i_left++;
        check_runs(k);
      } else {
        text_RLE[r][0].position += approximate_period;
        text_RLE[r][0].dimension --;
        i_left++;
        check_runs(k);
      }
    }
  }
  //the first l characters will go in here
  //we initialize each list in the vector text_RLE
}
void setup_phase_per_character(int i, char c, int k){
  add_character(i, c);
  check_runs(k);
}
void setup(int k, string &text){
  char text_char;
  for(int i=0; i<pattern_size; i++){
    text_char = text[i];
    setup_phase_per_character(i, text_char, k);
  }
}
//******************************************************************************
//******************************************************************************











//******************************HANDOVER****************************************
//******************************************************************************
void pre_handover(int k, const int text_size, string &text){
  bool found_i_right;
  found_i_right = false;
  char text_char;
  for(int i=pattern_size; i<text_size && found_i_right == false; i++){
    text_char = text[i];
    found_i_right = compute_i_right(i, text_char, k);
  }
  if(found_i_right == false){
    i_right = text_size - 1;
  }
}
void start_A_RLE(int r, int s){
  A_RLE *new_instance;
  new_instance = new A_RLE;
  int diff = (text_RLE[s][0].position - s)/approximate_period;
  new_instance->correction = diff;
  for(int i = 0; i < text_runs_length[s]; i++){
    new_instance->new_runs.push(text_RLE[s][i]);
  }
  new_instance->ham = {0,0};
  new_instance->delta = {0,0};
  new_instance->current_diagonal = new_instance->correction;
  if(s == 0){
    vector<A_RLE*> new_line;
    new_line.push_back(new_instance);
    instance_A_RLE.push_back(new_line);
  } else {
    instance_A_RLE[r].push_back(new_instance);
  }
}
int get_diagonal(Corner c, int r){
  int pattern_encoded_length;
  pattern_encoded_length = pattern_size / approximate_period +
                           max(0, pattern_size % approximate_period - r);
  return (pattern_encoded_length - 1 + c.x - c.y);
}
void create_corners(vector<Corner> &corners, RLE_instance text_instance,
                              RLE_instance pattern_instance){
  corners.clear();
  int height = pattern_instance.dimension;
  int width  = text_instance.dimension;
  Corner c;
  if(pattern_instance.label != text_instance.label){
//  Obtain the top-left corner of the block
    c.y = pattern_instance.position / approximate_period;
    c.x = text_instance.position / approximate_period;

//  add the top corners that influence delta
    c.x ++;
    c.influence = false;
    corners.push_back(c);

    c.x += width;
    c.influence = true;
    corners.push_back(c);
//  go back to the top left corner
    c.x -= width;
    c.x --;
//  add the bottom corners that influence delta
    c.y += height -1;
    c.influence = true;
    corners.push_back(c);

    c.x += width;
    c.influence = false;
    corners.push_back(c);
    //NOTE: this also works for MR being a 1 x d or d x 1 line and a 1 x 1 square
  }
}
void add_corners(int r, int s){
  RLE_instance current_text_run;
  RLE_instance current_pattern_run;
  vector<Corner> corners;
  A_RLE *instance = instance_A_RLE[r][s];
//create a dummy corner at the end of the text run to indicate the end of the corners
  Corner dummy_c;
  dummy_c.y = 0;
  dummy_c.x = 2* pattern_size;
  dummy_c.influence = 0;
  instance->corners.push(dummy_c);
  while(! instance->new_runs.empty() ){
    for(int i = 0; i < pattern_runs_length[r]; i++){
      current_text_run = instance->new_runs.front();
      current_pattern_run = pattern_rle[r][i];
      create_corners(corners, current_text_run, current_pattern_run);
      for(int i=0; i< (int) corners.size(); i++){
        instance->corners.push(corners[i]);
      }
    }
    instance->new_runs.pop();
  }
}
void create_i_star_list(int r){
  priority_queue<i_star_instance, vector<i_star_instance>, Compare_i_star> tempq;
  i_star_instance temp_i_star;
  Corner tempc;
  for(int s = 0; s < approximate_period; s++){
    tempc = instance_A_RLE[r][s]->corners.top();
    temp_i_star.value = get_diagonal(tempc, r);
    temp_i_star.instance.first = r;
    temp_i_star.instance.second = s;
    tempq.push(temp_i_star);
  }
  i_stars.push_back(tempq);
}
void initialise_i_stars(){

  for(int r = 0; r < approximate_period; r++){
    create_i_star_list(r);
  }
}
int get_influence(bool influence){
  if(influence == true)
    return 1;
  else
    return -1;
}
void reach_diagonal(int r, int diagonal){
  int s;
  int corner_diagonal;
  int influence;
  HAMMING ham;
  DELTA delta;
  int pos;
  Corner c;
  i_star_instance temp_i_star;
  temp_i_star = i_stars[r].top();
//continue as long as all i_stars in the list
//are greater than the needed diagonal
  while(temp_i_star.value <= diagonal){
    s = temp_i_star.instance.second;
    c = instance_A_RLE[r][s]->corners.top();
    influence = get_influence(c.influence);
    instance_A_RLE[r][s]->corners.pop();
    corner_diagonal = get_diagonal(c, r);
    //check if it is the first corner
    if(corner_diagonal == instance_A_RLE[r][s]->correction){
      instance_A_RLE[r][s]->delta.current = influence;
      instance_A_RLE[r][s]->ham.current += instance_A_RLE[r][s]->delta.current;
    } else {
      //check if it is on the current diagonal
      if(corner_diagonal == instance_A_RLE[r][s]->current_diagonal){
        instance_A_RLE[r][s]->delta.current += influence;
        instance_A_RLE[r][s]->ham.current += influence;
      } else {
//      use temporary variables for legibility
        ham = instance_A_RLE[r][s]->ham;
        delta = instance_A_RLE[r][s]->delta;
        pos = instance_A_RLE[r][s]->current_diagonal;
//      compute the new values
        ham.previous = (corner_diagonal - pos - 1) * delta.current + ham.current;
        ham.current = ham.previous + delta.current + influence;
        delta.previous = delta.current;
        delta.current = delta.previous + influence;
        pos = corner_diagonal;
//      update the instance (r,s) with the new values
        instance_A_RLE[r][s]->ham = ham;
        instance_A_RLE[r][s]->delta = delta;
        instance_A_RLE[r][s]->current_diagonal = pos;
      }
    }
    c = instance_A_RLE[r][s]->corners.top();
    temp_i_star.value = get_diagonal(c, r);
    i_stars[r].pop();
    i_stars[r].push(temp_i_star);
    temp_i_star = i_stars[r].top();
  }
}
void handover(const int k){
  for(int r = 0; r < approximate_period; r++){
    for(int s = 0; s < approximate_period; s++){
      start_A_RLE(r, s);
    }
  }
//  Transform new_runs into corners (can do this as we are in the offline setting)
  for(int r=0; r < approximate_period; r++){
    for(int s = 0; s < approximate_period; s++){
      add_corners(r, s);
    }
  }
  initialise_i_stars();
  for(int r = 0; r < approximate_period; r++){
    reach_diagonal(r, (pattern_size - 1)/approximate_period);
  }
}
//******************************************************************************
//******************************************************************************











//******************************OUTPUT******************************************
//******************************************************************************
int get_delta(int r, int s, int diagonal){
  if(diagonal < instance_A_RLE[r][s]->current_diagonal){
    return instance_A_RLE[r][s]->delta.previous;
  }
  reach_diagonal(r, diagonal);
  return instance_A_RLE[r][s]->delta.current;
}
int get_ham_dist(int r, int s, int diagonal){
  int result;
  if(diagonal < instance_A_RLE[r][s]->current_diagonal){
    result = instance_A_RLE[r][s]->ham.previous;
    return result;
  }
  reach_diagonal(r, diagonal);
  result = (instance_A_RLE[r][s]->ham.current) +
           (instance_A_RLE[r][s]->delta.current) *
           (diagonal - instance_A_RLE[r][s]->current_diagonal);
  return result;
}
//Function R(r,i) as describer in Lemma 6.6 of the paper
int R(int r, int i){
  return (r + i - pattern_size + 1) % approximate_period;
}
//Function Q(r,i) as describer in Lemma 6.6 of the paper
int Q(int r, int i){
  int pattern_encoded_length;
  pattern_encoded_length = pattern_size / approximate_period +
                           max(0, pattern_size % approximate_period - r);
  return ( (r + i - pattern_size + 1) / approximate_period +
            pattern_encoded_length -1);
}
void output_result(int k, int position){
//all the potential matching position start from i_left onwards
  if(position < i_left + pattern_size - 1){
    cout<<"M ";
  } else {
    pair<int, int> ham_position;
    int sum = 0;
//  compute the hamming distance at the current position using lemma 6.6
    if((int) last_l_HAM.size() < approximate_period){
      ham_position.first = position;
      for(int r = 0; r < approximate_period; r++){
        sum += get_ham_dist(r, R(r, position), Q(r, position));
      }
      ham_position.second = sum;
      if(sum <= k && position >= i_left + pattern_size - 1){
        cout<<sum<<" ";
      } else {
        cout<<"M ";
      }
      last_l_HAM.push_back(ham_position);
//  compute the hamming distance at the current position using lemma 6.7
    } else {
      ham_position = last_l_HAM.front();
      last_l_HAM.pop_front();
      for(int r = 0; r < approximate_period; r++){
        sum += get_delta(r, R(r, position), Q(r, position));
      }
      ham_position.first = position;
      ham_position.second += sum ;
      last_l_HAM.push_back(ham_position);
      if(ham_position.second <= k){
        cout<<ham_position.second<<" ";
      } else {
        cout<<"M ";
      }
    }
  }
}
void output(int k, const int text_size){
  for(int i = pattern_size - 1; i <= i_right; i++){
    output_result(k, i);
  }
  for(int i = i_right + 1; i < text_size; i++){
    cout<<"M ";
  }
  cout<<'\n';
}
//******************************************************************************
//******************************************************************************









//***************************PRINT FUNCTIONS************************************
//******************************************************************************
void print_i(){
  cout<<"I_LEFT : "<<i_left<<'\n';
  cout<<"I_RIGHT : "<<i_right<<'\n';
}
void print_approximate_period(bool found){
  if(found == true){
    cout<<"SMALL APPROXIMATE PERIOD l = "<<approximate_period;
  } else {
    cout<<"LARGE APPROXIMATE PERIOD ";
  }
  cout<<'\n';
}
void print_setup_phase(){
  cout<<"TEXT ENCODING AFTER SETUP PHASE: "<<'\n';
  for(int i = 0; i < approximate_period; i++){
    for(int j =0; j<text_runs_length[i]; j++){
      cout<<"("<<text_RLE[i][j].position<<","<<text_RLE[i][j].label;
      cout<<") -> ";
    }
    cout<<'\n';
  }
}
void print_text_RLE(){
  cout<<"TEXT ENCODING AFTER BOTH ENCODINGS: "<<'\n';
  for(int i = 0; i < approximate_period; i++){
    for(int j =0; j<text_runs_length[i]; j++){
      cout<<"("<<text_RLE[i][j].position<<","<<text_RLE[i][j].label;
      cout<<","<<text_RLE[i][j].dimension<<") -> ";
    }
    cout<<'\n';
  }
}
void print_pattern_rle(){
  cout<<"PATTERN ENCODING : "<<'\n';
  for(int i = 0; i < approximate_period; i++){
    for(int j =0; j<pattern_runs_length[i]; j++){
      cout<<"("<<pattern_rle[i][j].position<<","<<pattern_rle[i][j].label;
      cout<<","<<pattern_rle[i][j].dimension<<") -> ";
    }
    cout<<'\n';
  }
}
void print_text_runs_length(){
  for(int i=0; i<approximate_period; i++){
    cout<<text_runs_length[i]<<" ";
  }
  cout<<'\n';
}
void check_corners_added_correctly(){
  for(int r = 0; r < approximate_period; r++){
    for(int s = 0; s < approximate_period; s++){
      cout<<"A_RLE "<<r<<" "<<s<<" : "<<'\n';
      while(!instance_A_RLE[r][s]->corners.empty()){
        cout<<"(";
        cout<<instance_A_RLE[r][s]->corners.top().x;
        cout<<","<<instance_A_RLE[r][s]->corners.top().y;
        cout<<","<<instance_A_RLE[r][s]->corners.top().influence;
        cout<<")-";
        instance_A_RLE[r][s]->corners.pop();
      }
      cout<<'\n';
    }
  }
}
void print_i_stars(){
  for(int r = 0; r < approximate_period; r++){
    cout<<"r = "<<r<<" I_STARS ARE : ";
    i_star_instance temp_i_star;
    while(! i_stars[r].empty()){
      temp_i_star = i_stars[r].top();
      cout<<"(s = "<<temp_i_star.instance.second<<", value = ";
      cout<<temp_i_star.value<<")-";
      i_stars[r].pop();
    }
    cout<<'\n';
  }
}
void print_position_delta_hamming(){
  for(int r = 0; r < approximate_period; r++){
    cout<<"r = "<<r<<" : "<<"\n";
    for(int s = 0; s < approximate_period; s++){
      cout<<"  "<<" s = "<<s<<" : ";
      cout<<" diag = "<<instance_A_RLE[r][s]->current_diagonal;
      cout<<", HAM = ("<<instance_A_RLE[r][s]->ham.previous<<",";
      cout<<instance_A_RLE[r][s]->ham.current<<"), DELTA = (";
      cout<<instance_A_RLE[r][s]->delta.previous<<",";
      cout<<instance_A_RLE[r][s]->delta.current<<")\n";
    }
  }
}
//******************************************************************************
//******************************************************************************
int main(int argc, char* argv[])
{

  if (argc<3){
    cout << "-----Help -----" << endl;
    cout << "To call the programm : ./main fileText filePattern optionalOutputfile" << endl;
    cout << "If no output file is specified, the result will be in k-mismatch.out" << endl;
    return 0;
  }

  string fileTexte = argv[1];
  string filePattern = argv[2];
  string pattern;
  string text;
  int k;
  readText(fileTexte, text);
  readPattern(filePattern, pattern, k);

cout << argc << endl;
  if (argc>3)
    freopen(argv[3], "w", stdout);
  else
    freopen("k-mismatch.out", "w", stdout);
  
// approximate period is 0 after this step if it is > k;
  find_approximate_period(pattern, k);

// pattern length check
  if(approximate_period == 0){
    cout<<"LARGE APPROXIMATE PERIOD ";
    return 0;
  }
//  print_approximate_period(found_approximate_period);

  const int text_size = 2 * pattern_size;
  initialise();
  encode_pattern(pattern);

//SETUP PHASE
  setup(k, text);

//Here we compute T_R taking advantage the input is offline
  pre_handover(k, text_size, text);

//HANDOVER PHASE
  handover(k);

//OUTPUT PHASE
  output(k, text_size);

//  print_pattern_rle();
//  print_setup_phase();
//  print_text_RLE();
//  print_text_runs_length();
//  check_corners_added_correctly();
//  print_i_stars();
//  print_position_delta_hamming();
//  print_i();

  close (stdout);

  return 0;
}
