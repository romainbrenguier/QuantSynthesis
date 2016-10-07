#include <iostream>
#include <cassert>
#include <sstream>
#include "energyGames.h"
#include "cudd.h"
using namespace std;

#define ENERGY_GAME_DEBUG 0

 
ostream & dbgMsg(string s)
{ cerr << s << endl; return cerr; }


ADD EnergyGame::addMin(ADD a,vector<lit> v){
  vector<ADD> tab;
  for(unsigned i = 0; i< v.size(); i++)
    tab.push_back(addOfLit(v[i]));
  ADD z = mgr->constant(1.0);
  setRing(OP_MINIMUM,OP_TIMES,10000);
  ADD result = a.MatrixMultiply(z,tab);
  return result;
}

ADD EnergyGame::addMax(ADD a,vector<lit> v){
  vector<ADD> tab;
  for(unsigned i = 0; i< v.size(); i++)
    tab.push_back(addOfLit(v[i]));
  ADD z = mgr->constant(1.0);
  setRing(OP_MAXIMUM,OP_TIMES,-1.0);
  ADD result = a.MatrixMultiply(z,tab);
  return result;
}
  

void EnergyGame::initialize_input_cubes(){
  cinput_cube = mgr -> bddOne();
  uinput_cube = mgr -> bddOne();
  for (unsigned i = 0; i < spec->num_inputs; i++) {
    aiger_symbol symbol = spec->inputs[i];
    std::string name(symbol.name);
    if (name.find("controllable") == 0) { // starts with "controllable"
      cinput_cube &= ofLit(symbol.lit);
      cinput.push_back(symbol.lit);
      cout << "lit "<< symbol.lit<<" is controllable" << endl;
    }
    else {
      uinput_cube &= ofLit(symbol.lit);
      uinput.push_back(symbol.lit);
      cout << "lit "<< symbol.lit<<" is uncontrollable" << endl;
    }
  }
}

void EnergyGame::introduceErrorLatch() {
  if (error_fake_latch != NULL)
    return;
  error_fake_latch = (aiger_symbol*) malloc(sizeof(aiger_symbol));
  error_fake_latch->name = new char[6];
  string("error").copy(error_fake_latch->name, 6);
  error_fake_latch->lit = (spec->maxvar + 1) * 2;
  spec->maxvar++;
  /*
  unsigned i = 0;
  string o_err("o_err");
  bool found = false;
  while(!found) {
    if(i == spec->num_outputs) {
      dbgMsg("No output named o_err");
      found = true;
    } else if(o_err.compare(spec->outputs[i]) == 0) {
      error_fake_latch->next = spec->outputs[i].lit;
      found = true;
    } else i++;
    }
  dbgMsg(std::string("Error fake latch = ") + 
  std::to_string(error_fake_latch->lit));*/
}

void EnergyGame::initializeOutputs(){
  introduceErrorLatch();
  dbgMsg(std::string("Error fake latch = ") + 
	 std::to_string(error_fake_latch->lit));
  string o_err("o_err");
  string err("err");
  bool found = false;

  for(unsigned i=0;i < spec->num_outputs;i++) {
    if(o_err.compare(spec->outputs[i].name) == 0  || err.compare(spec->outputs[i].name) == 0) {
      error_fake_latch->next = spec->outputs[i].lit;
      found = true;
    }
    //else if(o_weight.compare(spec->outputs[i].name) == 0)
    //weight_output = new unsigned (spec->outputs[i].lit);
    //else dbgMsg("unknown output ") << spec->outputs[i].name;
  }
  if(! found) dbgMsg("Warning: no error output");
  assert(found);
  
  if(latch.size() > 0) latch.push_back(error_fake_latch->lit);
  //  if(weight_output == NULL) dbgMsg("Warning: no weight output");
  //dbgMsg("Weight output = ") << (*weight_output) << endl;;

}

EnergyGame::EnergyGame(aiger &base, Cudd * local_mgr) {
  spec = & base;
  mgr = local_mgr;
  lit2bdd_map.resize(spec->maxvar * 2);
  lit2bdd_map_filled.resize(spec->maxvar * 2,false);  
  lit2add_map.resize(spec->maxvar * 2);
  lit2add_map_filled.resize(spec->maxvar * 2,false);  
  error_fake_latch = NULL;
  weight_output = NULL;
  initialize_input_cubes();
  initializeOutputs();
}


EnergyGame::EnergyGame(std::string file_name) {
  aiger * a = aiger_init();
  FILE* file = fopen(file_name.c_str(), "r");
  mgr = new Cudd();
  const char * message = aiger_read_from_file(a,file);
  if(message != NULL)
    cerr << "aiger_read_from_file: "<<message<< endl;
  spec = a;
  lit2bdd_map.resize(spec->maxvar * 2 + 2);
  lit2bdd_map_filled.resize(spec->maxvar * 2 + 2,false);
  lit2add_map.resize(spec->maxvar * 2 + 2);
  lit2add_map_filled.resize(spec->maxvar * 2 + 2,false);
  dbgMsg("size :") << (spec->maxvar * 2 + 2) << endl;
  //initialize_input_cubes();
  primed_latch_cube = NULL;
  next_fun_compose_vec = NULL;
  //  trans_rel = NULL;
  short_error = NULL;
  error_fake_latch = NULL;
  weight_output = NULL;
  initializeOutputs();

}

std::vector<lit> EnergyGame::latches() {
  if(latch.size() != 0) return latch;
  for(unsigned i=0; i < spec->num_latches; i++)
    latch.push_back(spec->latches[i].lit);
  cout << "error fake  latch:" << error_fake_latch->lit << endl;
  latch.push_back(error_fake_latch->lit);
  return latch;
}

std::vector<lit> EnergyGame::cinputs() {
  if(cinput.size() != 0) return cinput;
  initialize_input_cubes();
  return cinput;
}

std::vector<lit> EnergyGame::uinputs() {
  if(uinput.size() != 0) return uinput;
  initialize_input_cubes();
  return uinput;
}


void EnergyGame::dump2dot(BDD b, std::string file_name) {
  std::vector<BDD> v;
  const char * fn = file_name.c_str();
  v.push_back(b);
  FILE* file = fopen(fn, "w");
  this->mgr->DumpDot(v, 0, 0, file);
  fclose(file);
}

void EnergyGame::dump2dot(ADD b, std::string file_name) {
  std::vector<ADD> v;
  const char * fn = file_name.c_str();
  v.push_back(b);
  FILE* file = fopen(fn, "w");
  this->mgr->DumpDot(v, 0, 0, file);
  fclose(file);
}

EnergyGame::~EnergyGame() {
    if (this->primed_latch_cube != NULL)
        delete this->primed_latch_cube;
    /*if (this->cinput_cube != NULL)
        delete this->cinput_cube;
    if (this->uinput_cube != NULL)
    delete this->uinput_cube;*/
    if (this->next_fun_compose_vec != NULL)
        delete this->next_fun_compose_vec;
    //if (this->trans_rel != NULL)
    //  delete this->trans_rel;
    if (this->short_error != NULL)
        delete this->short_error;
}

ADD EnergyGame::initState() {
  ADD result = mgr->constant(1.0);
  /*
  for (unsigned i = 0; i < spec->num_latches; i++)
  result &= ~ mgr->addVar(spec->latches[i].lit);*/
  vector<lit> l = latches();
  for (unsigned i = 0; i < l.size(); i++)
    result &= ~ mgr->addVar(l[i]);
  return result;
}

BDD EnergyGame::errorStates() {
  dbgMsg("error states: ") << error_fake_latch << endl;
  assert(error_fake_latch != NULL);
  return mgr->bddVar(error_fake_latch->lit);
}


BDD EnergyGame::primeLatchesInBdd(BDD original) {
  std::vector<BDD> latch_bdds, primed_latch_bdds;
  for (unsigned i = 0; i < spec->num_latches; i++) {
    latch_bdds.push_back(mgr->bddVar(spec->latches[i].lit));
    primed_latch_bdds.push_back(mgr->bddVar(primeVar(spec->latches[i].lit)));
  }
  BDD result = original.SwapVariables(latch_bdds, primed_latch_bdds);
  return result;
}

ADD EnergyGame::primeLatchesInAdd(ADD original) {
  std::vector<ADD> latch, primed_latch;
  for (unsigned i = 0; i < spec->num_latches; i++) {
    latch.push_back(mgr->addVar(spec->latches[i].lit));
    primed_latch.push_back(mgr->addVar(primeVar(spec->latches[i].lit)));
  }
  latch.push_back(mgr->addVar(error_fake_latch->lit));
  primed_latch.push_back(mgr->addVar(primeVar(error_fake_latch->lit)));
  ADD result = original.SwapVariables(latch, primed_latch);
  return result;
}


BDD EnergyGame::unprimeLatchesInBdd(const BDD & original) {
  std::vector<BDD> latch_bdds, primed_latch_bdds;
  for (unsigned i = 0; i< spec->num_latches; i++) {
    latch_bdds.push_back(mgr->bddVar(spec->latches[i].lit));
    primed_latch_bdds.push_back(mgr->bddVar(primeVar(spec->latches[i].lit)));
  }
  return original.SwapVariables(primed_latch_bdds,latch_bdds);
}

BDD EnergyGame::primedLatchCube() {
  if (primed_latch_cube == NULL) {
    BDD result = mgr->bddOne();
    for (unsigned i = 0; i < spec->num_latches; i++)
      result &= ofPrimeLit(spec->latches[i].lit);
    primed_latch_cube = new BDD(result);
  }
  return BDD(*this->primed_latch_cube);
}

BDD EnergyGame::cinputCube() {
  /* if (cinput_cube == NULL) {
    BDD result = mgr->bddOne();
    for (int_
	 i != this->c_inputs.end(); i++)
      result &= this->mgr->bddVar((*i)->lit);
    this->cinput_cube = new BDD(result);
    }
  return BDD(*this->cinput_cube);*/
  return cinput_cube;
}

BDD EnergyGame::uinputCube() {
  /*if (this->uinput_cube == NULL) {
        BDD result = this->mgr->bddOne();
        for (std::vector<aiger_symbol*>::iterator i = this->u_inputs.begin();
             i != this->u_inputs.end(); i++)
            result &= this->mgr->bddVar((*i)->lit);
        this->uinput_cube = new BDD(result);
    }
    return BDD(*this->uinput_cube);*/
  return uinput_cube;
}

BDD EnergyGame::toCube(std::vector<lit> &vars) {
  BDD result = this->mgr->bddOne();
  for (std::vector<lit>::iterator i = vars.begin(); i != vars.end(); i++)
    result &= this->mgr->bddVar((*i));
  return result;
}

BDD EnergyGame::ofLit(lit var) {
  return mgr->bddVar(var);
}

BDD EnergyGame::ofPrimeLit(lit var) {
  return mgr->bddVar(primeVar(var));
}

BDD EnergyGame::lit2bdd(unsigned lit) {
  BDD result;
  // we first check the cache
  if (lit2bdd_map_filled[lit]){
    return lit2bdd_map[lit];
  }
  unsigned stripped_lit = aiger_strip(lit);
  if (stripped_lit == 0) { // return the true/false BDD
    result = ~this->mgr->bddOne();
  } else {
    aiger_and* and_gate = aiger_is_and(this->spec, stripped_lit);
    if (and_gate) 
      result = (lit2bdd(and_gate->rhs0) & lit2bdd(and_gate->rhs1));
    else if (stripped_lit == this->error_fake_latch->lit) 
      result = this->mgr->bddVar(stripped_lit);
    else {
      // is it an input or latch? these are base cases
      aiger_symbol* symbol = aiger_is_input(this->spec, stripped_lit);
      if (!symbol)
	symbol = aiger_is_latch(this->spec, stripped_lit);
      assert(symbol);
      result = this->mgr->bddVar(stripped_lit);
    }
  }
  // let us deal with the negation now
  if (aiger_sign(lit))
    result = ~result;
  // cache result
  lit2bdd_map[lit] = result;
  lit2bdd_map[aiger_not(lit)] = ~result;
  lit2bdd_map_filled[lit] = true;
  lit2bdd_map_filled[aiger_not(lit)] = true;
  return result;
}

ADD EnergyGame::addOfLit(lit l) {
  return mgr->addVar(l);
}

ADD EnergyGame::lit2add(unsigned lit) {
  ADD result;

  if (lit2add_map_filled[lit]) 
    return lit2add_map[lit];

  unsigned stripped_lit = aiger_strip(lit);
  if (stripped_lit == 0)
    result = mgr->constant(0.0);
  else {
    aiger_and* and_gate = aiger_is_and(spec, stripped_lit);
    if (and_gate) 
      result = lit2add(and_gate->rhs0) & lit2add(and_gate->rhs1);
    else {
      aiger_symbol* symbol = aiger_is_input(spec, stripped_lit);
      if (!symbol)
	symbol = aiger_is_latch(spec, stripped_lit);
      assert(symbol);
      result = addOfLit(stripped_lit);
    }
  }

  if (aiger_sign(lit)) result = ~result;

  lit2add_map[lit] = result;
  lit2add_map[aiger_not(lit)] = ~result;
  lit2add_map_filled[lit] = true;
  lit2add_map_filled[aiger_not(lit)] = true;
  return result;
}

Cudd * EnergyGame::manager() { return mgr; }

ADD safeRestrict(ADD original, ADD rest_region) {
  ADD approx = original.Restrict(rest_region);
  assert((approx & rest_region) == (original & rest_region));
  if (approx.nodeCount() < original.nodeCount())
    return approx;
  else
    return original;
}

inline int max(int a,int b){ return ((a<b)?b:a); }
std::vector<ADD> EnergyGame::nextFunComposeVec(ADD* care_region) {
  if (this->next_fun_compose_vec == NULL) {
    next_fun_compose_vec = new std::vector<ADD>(max(mgr->ReadSize(),2 * spec->maxvar+2));
    for (unsigned i = 0; i < next_fun_compose_vec->size(); i++) 
      (*next_fun_compose_vec)[i]= addOfLit(i);
    
    for (unsigned i = 0; i < spec->num_latches; i++) {
      ADD next_fun;
      if (this->short_error != NULL) { 
	next_fun = lit2add(spec->latches[i].next);
	next_fun = safeRestrict(next_fun, ~(*short_error));
      } else {
	next_fun = lit2add(spec->latches[i].next);
      }
      (*next_fun_compose_vec)[primeVar(spec->latches[i].lit)] = next_fun;
    }

    ADD next_fun;    
    if (short_error != NULL) {
      next_fun = *short_error;
    }
    else {
      next_fun = lit2add(error_fake_latch->next);
    }
    (*next_fun_compose_vec)[primeVar(error_fake_latch->lit)] = next_fun;
    
  }
  
  std::vector<ADD> result = *next_fun_compose_vec;
  
  if (care_region != NULL) 
    for (unsigned i = 0; i < spec->num_latches; i++)
      result[i] = safeRestrict(result[spec->latches[i].lit], *care_region);
  
  return result;
}

ADD EnergyGame::weight_function() {
  if (weight_output != NULL) return *weight_output;

  ADD weight = mgr->constant(0.0);
  string o_weight("o_weight");

  for(unsigned i=0;i < spec->num_outputs;i++)
    if(string(spec->outputs[i].name).find(o_weight) == 0) {
      dbgMsg("Adding weight of output ") << i << endl;
      weight += lit2add(spec->outputs[i].lit);
    }
  
  weight_output = new ADD(weight);
  return *weight_output;
}


ADD EnergyGame::solve(double threshold,int max_iterations,double shift, double scale,bool minimizer) {
  ADD value = mgr->addVar(error_fake_latch->lit);
  value *= mgr->plusInfinity();

  ADD weight = weight_function() * mgr->constant(scale) + mgr->constant(shift);

  // cout << "writing weight.dot" << endl;
  // dump2dot(weight,"weight.dot");

  vector<ADD> next_funs = nextFunComposeVec();

  bool fixpoint = false;
  int it = 0;
  
  while(!fixpoint && it != max_iterations) {
    cout << "iteration : " << ++it << endl;
    // cout << "writing value"<< it << ".dot" << endl;
    // stringstream name;
    // name << "value" << it << ".dot";
    // dump2dot(value,name.str().c_str());

    //dbgMsg("renaming");
    ADD primed = primeLatchesInAdd(value);
    // cout << "writing primed.dot" << endl;
    // dump2dot(primed,"primed.dot");

    //dbgMsg("composing");
    ADD trans_bdd = primed.VectorCompose(next_funs);
    // cout << "writing trans_bdd.dot" << endl;
    // dump2dot(trans_bdd,"trans_bdd.dot");

    //dbgMsg("adding weights");
    ADD weighted = trans_bdd+weight;
    cout << "writing weighted.dot" << endl;
    // dump2dot(weighted,"weighted.dot");

    //dbgMsg("choosing actions");
    ADD a;
    
    if(minimizer) {
      ADD min = addMin(weighted,cinputs());
      if(ENERGY_GAME_DEBUG) { 
	cout << "writing min.dot" << endl;
	dump2dot(min,"min.dot");
      }

      ADD max = addMax(min,uinputs());
      if(ENERGY_GAME_DEBUG) { 
	cout << "writing max.dot" << endl;
	dump2dot(max,"max.dot");
      }
      a = max;
    }
    else
      a = addMin(addMax(weighted,cinputs()),uinputs());

    if(ENERGY_GAME_DEBUG) { 
      cout << "writing a.dot" << endl;
      dump2dot(a,"a.dot");
    }

    /*
    ADD amin = addMin(addMin(weighted,cinputs()),uinputs());
    cout << "writing a-min.dot" << endl;
    dump2dot(amin,"a-min.dot");
    ADD diff = addMax(weighted,uinputs()) - weighted; //addMin(weighted,uinputs());
    cout << "writing diff.dot" << endl;
    dump2dot(diff,"diff.dot");
    */

    //dbgMsg("removing negatives");
    ADD b = a.Maximum(mgr->constant(0.0));
    if(ENERGY_GAME_DEBUG) { 
      cout << "writing b.dot" << endl;
      dump2dot(b,"b.dot");
    }

    //dbgMsg("puting values above the threshold to infinity");
     // Threshold puts the value that are above to infinity and below to 0
    ADD c = b.Threshold(mgr->constant(threshold));
    if(ENERGY_GAME_DEBUG) { 
      cout << "writing c.dot | threshold is : " << threshold << endl;
      dump2dot(c,"c.dot");
    }

    ADD d = b + c ;
    if(ENERGY_GAME_DEBUG) { 
      cout << "writing d.dot" << endl;
      dump2dot(d,"d.dot");
    }

    //dbgMsg("testing for fixpoint");
    if(d == value) fixpoint = true;
    else { 
      value = d;
      ADD val = addMax(initState() & d, latches());
      if(val == mgr->plusInfinity())
	fixpoint = true;
    }
  }

  if(it == max_iterations) 
    return (mgr->plusInfinity());
  else
    return value;
}
