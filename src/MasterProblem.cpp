#include "MasterProblem.h"
#include "PricingProblem.h"
#include "ilconcert/iloenv.h"
#include "ilconcert/iloexpression.h"
#include "ilconcert/ilolinear.h"
#include "ilconcert/ilosys.h"
#include "ilcplex/ilocplexi.h"
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <limits>
#include <vector>

MasterProblem::MasterProblem(const Data &data, double UB)
    : data(data),
      lambdaItens(data.getNItems(), std::vector<bool>(data.getNItems())) {

  int n = data.getNItems();
  this->env = IloEnv();
  this->masterProblem = IloModel(this->env);
  this->lambda = IloNumVarArray(this->env, n, 0, IloInfinity);

  IloExpr sum_obj(this->env);
  this->masterConstraints = IloRangeArray(this->env);

  for (int i = 0; i < n; i++) {
    char var_name[50];
    sprintf(var_name, "y%d", i);

    lambda[i].setName(var_name);
    sum_obj += M * lambda[i];

    this->masterConstraints.add(lambda[i] == 1);
  }

  this->masterProblem.add(this->masterConstraints);
  this->masterObj = IloObjective(IloMinimize(this->env, sum_obj));
  this->masterProblem.add(this->masterObj);

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      if (i == j)
        lambdaItens[i][j] = 1;
      else
        lambdaItens[i][j] = 0;
    }
  }
}


MasterProblem::~MasterProblem() {
    // Liberar os recursos alocados
    masterObj.end();
    masterConstraints.end();
    lambda.end();
    masterProblem.end();
}


std::pair<int, int> MasterProblem::solve(Node & no, bool root) {
  //std::cout << "NO" << std::endl;


  // for(int i = 0; i < lambdaItens.size(); i++){
  //   for(int j = 0; j < lambdaItens[i].size(); j++){
  //     std::cout << lambdaItens[i][j] << " ";
  //   }
  //     std::cout << std::endl;
  // }
 
  add_revert_pair_constraint(no, 0);
  std::vector<std::vector<bool>> solution;

  int n = this->data.getNItems();
  int lambda_counter = this->lambda.getSize();
  std::pair<int, int> items;

  IloCplex rmp(this->masterProblem);
  rmp.setWarning(this->env.getNullStream());
  rmp.setOut(this->env.getNullStream());
  rmp.solve();
  

  for(int i = 0; i < lambdaItens.size(); i++){
    for(int j = 0; j < lambdaItens[i].size(); j++){
     // std::cout << lambdaItens[i][j] << " ";
    }
    //std::cout << std::endl;
  }
  
  if(rmp.getCplexStatus() == IloCplex::Infeasible){
    no.master_is_feasible = false;
    add_revert_pair_constraint(no, 1);
    return {};
  }else{
    no.master_is_feasible = true;
  }

  while (true) {

    double reduced_cost;
    IloNumArray pi(this->env, n);
    std::vector<double> pi_vector(n);
    std::vector<double> entering_col_vector;

    rmp.getDuals(pi, this->masterConstraints);

    for (int i = 0; i < n; i++) {
      pi_vector[i] = pi[i];
    }
    if(root){
      entering_col_vector = solve_pricing_minknap(n, pi_vector, &reduced_cost, no, this->data);
    }else{
      entering_col_vector = solve_sub_problem(n, pi_vector, &reduced_cost, no, this->data);
    }
    
    IloNumArray entering_col(this->env);
    for (int i = 0; i < entering_col_vector.size(); i++) {
      entering_col.add(entering_col_vector[i]);
    }

    if (reduced_cost < -1e-5) {
      //std::cout << "entrou" << std::endl;
      // std::cout << std::endl << "Entering column:" << std::endl;
      for (size_t i = 0; i < n; i++) {
        this->lambdaItens[i].push_back(entering_col[i] < 0.5 ? 0 : 1);
        // std::cout << (entering_col[i] < 0.5 ? 0 : 1) << std::endl;
      }
      // std::cout << std::endl;

      // std::cout << "matriz: " << std::endl;

      for (int i = 0; i < n; i++) {
        for (int j = 0; j < lambda_counter + 1; j++) {
          // std::cout << lambdaItens[i][j] << " ";
        }
        // std::cout << std::endl;
      }

      // Add the column to the master problem
      // (the cost of the new variable is always 1)
      char var_name[50];
      sprintf(var_name, "y%d", lambda_counter++);
      IloNumVar new_lambda(this->masterObj(1) +
                               this->masterConstraints(entering_col),
                           0, IloInfinity);
      new_lambda.setName(var_name);

      lambda.add(new_lambda);

      // std::cout << "Solving the RMP again..." << std::endl;

      rmp.solve();

    } else {
      std::vector<std::vector<double>> z(n, std::vector<double>(n, 0));

      // std::cout << "No column with negative reduced costs found. The current"
     //               "basis is optimal"
      //           << std::endl;
      // std::cout << "Final master problem: " << std::endl;
      // std::cout << "Bins usados: " << rmp.getObjValue() << std::endl;

      no.bins = rmp.getObjValue();

      IloNumArray variables(this->env, lambda.getSize());
      rmp.getValues(lambda, variables);
      // for (int i = 0; i < lambda.getSize(); i++) {
      //   std::cout << "lambda[" << i << "] = " << variables[i] << std::endl;
      // }

      for (int i = 0; i < variables.getSize(); i++) {
        for (int j = 0; j < n; j++) {
          for (int k = j + 1; k < n; k++) {
            if (lambdaItens[j][i] == 1 && lambdaItens[k][i] == 1) {
              z[j][k] += variables[i];
            }
          }
        }
      }
      bool feasible;
      items = define_pair(n, z, &feasible);
      //std::cout << "feasible: " << feasible << std::endl;
      if(feasible)
        generate_solution(solution, &rmp, variables);
      no.solution = solution;
     
      break;
    }
  }

  add_revert_pair_constraint(no, 1);
  
  
  //generate_solution(solution, &rmp, lambda);
  

  return items;
}

void MasterProblem::prune() {}

void MasterProblem::add_revert_pair_constraint(Node & no, bool ub){

  for(int i = 0; i < this->lambda.getSize(); i++){
    for(int j = 0; j < no.separados.size(); j++){
      int item_1 = no.separados[j].first;
      int item_2 = no.separados[j].second;
      if(this->lambdaItens[item_1][i] == 1 && this->lambdaItens[item_2][i] == 1){
        if(ub)
          this->lambda[i].setUB(IloInfinity); //reverter restrição
        else {
          this->lambda[i].setUB(0); // adicionar restrição
        }
        //std::cout << "sep: lambda " << i << " foi zerado"<< std::endl;
      }
    }
  }

  for(int i = 0; i < this->lambda.getSize(); i++){
    for(int j = 0; j < no.juntos.size(); j++){
      int item_1 = no.juntos[j].first;
      int item_2 = no.juntos[j].second;
      if(this->lambdaItens[item_1][i] == 1 && this->lambdaItens[item_2][i] == 1){
        continue;
      }else if(this->lambdaItens[item_1][i] == 0 && this->lambdaItens[item_2][i] == 0){
        continue;
      }else{
        if(ub)
          this->lambda[i].setUB(IloInfinity);
        else {
          this->lambda[i].setUB(0);
        }
      }
      //std::cout << "jun: lambda " << i << " foi zerado"<< std::endl;
    }
  }
}

std::pair<int, int> MasterProblem::define_pair(int n, std::vector<std::vector<double>>& z, bool * is_feasible) {
  std::pair<int, int> items_pair;
  int best_i, best_j;
  double diff = std::numeric_limits<double>::infinity();
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      if (abs(abs(z[i][j] - 0.5) - diff) < EPSILON) {
        continue;
      }else if(abs(z[i][j] - 0.5) + EPSILON < diff){
        diff = abs(z[i][j] - 0.5);
        best_i = i;
        best_j = j;
      }
      //std::cout << z[i][j] << " ";
    }
    //std::cout << std::endl;
  }
  *is_feasible = z[best_i][best_j] > EPSILON ? false : true; 
  items_pair.first = best_i;
  items_pair.second = best_j;
  return items_pair;
}

void MasterProblem::generate_solution(std::vector<std::vector<bool>>& solution, IloCplex * rmp, IloNumArray& variables){

  for(int i = 0; i < this->lambda.getSize(); i++){
    double var = rmp->getValue(lambda[i]);
    if(abs(variables[i] - 1) < EPSILON){
      std::vector<bool> bin_solution;
      for(int j = 0; j < lambdaItens.size(); j++){
        bin_solution.push_back(lambdaItens[j][i]);
      }
        solution.push_back(bin_solution);
    }
  }
}

bool MasterProblem::is_feasible_solution(IloCplex * rmp, IloNumArray& variables){
  bool is_feasible = true;
  for(int i = 0; i < this->lambda.getSize(); i++){
    double var = rmp->getValue(lambda[i]);
    if(abs(var - 1) > EPSILON || abs(var - 0) > EPSILON){
      is_feasible = false;
      break;
    }
  }
  return is_feasible;
}

