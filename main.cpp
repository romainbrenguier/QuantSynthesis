#include <iostream>
#include "energyGames.h"
#include "cuddInt.h"

using namespace std;

int main(int argc, char ** argv)
{
  if(argc < 2) {
    std::cout << "usage: "<<argv[0] <<" file_name.aag" << endl;
    return 1;
  }

  string file_name(argv[1]);
  EnergyGame g(file_name);
  activate_cache();

  //ADD sol = g.weight_function();
  cout << "solving ..." << endl;
  
  bool finite_value = false;
  double threshold = 100.0;
  double max_iteration = 100;
  double max_value = 15;

  for(int i = 0; ! finite_value && i < max_value ; i++) {
    ADD sol = g.solve(threshold,max_iteration, -i);
    //g.dump2dot(sol,"solution.dot");
    //cout << "wrote solution.dot" << std::endl;

    ADD val = g.addMax(g.initState() & sol, g.latches());
    g.dump2dot(val,"value.dot");
    std::cout << "wrote value.dot" << std::endl;
    DdNode * n = val.getNode();
    double value = cuddV(n);
    cout << "necessary energy for average " << i << " : " << value << endl;
    if(value < threshold) finite_value = true;
  }
}
