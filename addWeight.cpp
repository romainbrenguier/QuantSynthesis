#include <iostream>
#include <sstream>
#include "aiger.h"

using namespace std;

int main(int argc, char ** argv)
{
  if(argc < 2) {
    std::cout << "usage: "<<argv[0] <<" file_name.aag" << endl;
    std::cout << "Add outputs that monitor a change in the value of latches." << endl;
    std::cout << "This outputs have names starting with o_weight and the" << endl;
    std::cout << "number of these outputs that are true corresponds to the" << endl;
    std::cout << "number of latches whose value have changed." << endl;
    return 1;
  }

  string file_name(argv[1]);

  aiger * a = aiger_init();
  FILE* file = fopen(file_name.c_str(), "r");
  
  const char * message = aiger_read_from_file(a,file);
  if(message != NULL)
    cerr << "aiger_read_from_file: "<<message<< endl;
  fclose(file);

  if(a->num_latches == 0)
    cerr << "no latches in the given aiger file" << endl;

  for(unsigned i = 0; i < a->num_latches; i++) {
    unsigned gate1 = 2 * (++a->maxvar);
    unsigned gate2 = 2 * (++a->maxvar);
    unsigned gate3 = 2 * (++a->maxvar);
    
    aiger_add_and(a,gate1,a->latches[i].lit,aiger_not(a->latches[i].next));
    aiger_add_and(a,gate2,a->latches[i].next,aiger_not(a->latches[i].lit));
    aiger_add_and(a,gate3,aiger_not(gate1),aiger_not(gate2));
    string name("o_weight<"+std::to_string(i)+">");
    aiger_add_output(a,aiger_not(gate3),name.c_str());
  }

  stringstream s;
  s << file_name << "_weighted" << ".aag";
  string new_file_name = s.str();
  cout << "writing "<<new_file_name << endl;
  FILE* file1 = fopen(new_file_name.c_str(), "w");
  aiger_write_to_file(a,aiger_ascii_mode,file1);
  fclose(file1);
}
