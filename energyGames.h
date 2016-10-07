#ifndef ENERGY_GAMES_H
#define ENERGY_GAMES_H
#include "cuddObj.hh"

#include "aiger.h"
typedef unsigned lit;



class EnergyGame {
 public:
  // Initialization from an AIG and a manager
  EnergyGame(aiger &, Cudd*);
  // Initialization by copying an existing EnergyGame and ???
  EnergyGame(const EnergyGame&, BDD);
  // Initialization from the name of an AIG file.
  // The aiger file should have one output named "err" and one named "o_weight"
  // weights represents the energy consumed at each step
  EnergyGame(std::string);

  // Destructor.
  ~EnergyGame();

  // Returns a literal used to represent its next valuation
  static unsigned primeVar(lit lit) { return aiger_strip(lit) + 1; }
  // Write a BDD in a file
  void dump2dot(BDD, std::string);
  void dump2dot(ADD, std::string);
  // Returns a BDD representing the initial state.
  ADD initState();
  // Returns a BDD representing the error states.
  BDD errorStates();
  // Weight function. The circuit should have one output named weight
  ADD weight_function();
  // Returns a BDD where all latches have been replaced by the corresponding next literal.
  BDD primeLatchesInBdd(BDD);
  ADD primeLatchesInAdd(ADD);
  // Returns a BDD where all primed latches have been replaced by the original (unprimed) literal.
  BDD unprimeLatchesInBdd(const BDD &);
  // Cube containing the latch literals 
  BDD primedLatchCube();
  // Cube containing the controllable inputs literals 
  BDD cinputCube();
  // Cube containing the uncontrollable inputs literals 
  BDD uinputCube();
  // Takes a set of literals and return the corresponding cube
  BDD toCube(std::vector<lit>&);
  // BDD representing the valuation of a litteral
  BDD ofLit(lit);
  BDD ofPrimeLit(lit);
  ADD addOfLit(lit);
  // Minimum over a set of variable (similar to a universal quantification)
  ADD addMin(ADD,std::vector<lit>);
  // Maximum over a set of variable (similar to a existential quantification)
  ADD addMax(ADD,std::vector<lit>);

  /*
   * Returns for each latch the BDD that represents its update function.
   * The argument is represents a care region, it is set to NULL by default. */
  std::vector<ADD> nextFunComposeVec(ADD* care_region=NULL);

  Cudd* manager();

 
  // threshold is the number above which we set the value to infinity
  // max_iterations is a number of iterations after which we stop even if we did not reach a fixpoint. If it is set to a negative number then we wait for a fixpoint.
  // shift is added to the weights of the game.
  // scale is use to scale the weights. It can be used for instance to revert the weights by puting it to -1.
  // minimizer is set to true to mean that the controller tries to minimize the energy used.
  // Will stop when if the initial state requieres infinite energy.
  ADD solve(double threshold,int max_iterations=-1,double shift=0,double scale=1,bool minimizer=true);

  std::vector<lit> cinputs();
  std::vector<lit> uinputs();
  std::vector<lit> latches();
  
 private:
  aiger* spec;
  Cudd* mgr;
  BDD* primed_latch_cube;
  BDD cinput_cube;
  BDD uinput_cube;
  std::vector<lit> cinput;
  std::vector<lit> uinput;
  std::vector<lit> latch;
  ADD* short_error;
  std::vector<BDD> lit2bdd_map;
  std::vector<bool> lit2bdd_map_filled;
  BDD lit2bdd(lit);
  std::vector<ADD> lit2add_map;
  std::vector<bool> lit2add_map_filled;
  ADD lit2add(lit);
  std::vector<ADD> * next_fun_compose_vec;
  aiger_symbol* error_fake_latch;
  ADD * weight_output;

  void initialize_input_cubes();
  void introduceErrorLatch();  
  void initializeOutputs();
};

#endif
         
