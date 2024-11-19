#include "Problema.h"
#include "ilconcert/iloexpression.h"
#include "ilconcert/ilosys.h"
#include "ilcplex/ilocplexi.h"
#include <iostream>
#include <vector>

Problema::Problema(const Data &data, double UB) : data(data) {
  // this->data = data;
  int n = data.getNItems();
  this->env1 = IloEnv();
  this->masterProblem = IloModel(this->env1);
  this->lambda = IloNumVarArray(this->env1, n, 0, IloInfinity);

  IloExpr sum_obj(this->env1);
  this->masterConstraints = IloRangeArray(this->env1);

  for (int i = 0; i < n; i++) {
    char var_name[50];
    sprintf(var_name, "y%d", i);

    lambda[i].setName(var_name);
    sum_obj += M * lambda[i];

    this->masterConstraints.add(lambda[i] == 1);
  }

  this->masterProblem.add(this->masterConstraints);

  this->masterObj = IloObjective(IloMinimize(this->env1, sum_obj));
  this->masterProblem.add(this->masterObj);
}

std::pair<int, int> Problema::solve() {
  
  int n = this->data.getNItems();
  IloCplex rmp(this->masterProblem);
  rmp.setOut(this->env1.getNullStream());

  rmp.solve();

  int lambda_counter = n;

  while (true) {
    // Get the dual variables 

    IloNumArray pi(this->env1, n);

    rmp.getDuals(pi, this->masterConstraints);

    IloModel sub_model(this->env1);
    IloBoolVarArray x(this->env1, n);
    IloExpr reduced_cost(this->env1);
    IloExpr weight_sum(this->env1);

    reduced_cost = 1 - pi[0] * x[0];

    for (int i = 1; i < n; i++) {
      reduced_cost -= pi[i] * x[i];
    }

    IloObjective sub_objective = IloMinimize(this->env1, reduced_cost);
    sub_model.add(sub_objective);

    for (int i = 0; i < n; i++) {
      weight_sum += this->data.getItemWeight(i) * x[i];
    }

    sub_model.add(weight_sum <= this->data.getBinCapacity());
    IloCplex pricing_problem(sub_model);

    pricing_problem.setOut(this->env1.getNullStream()); // disables CPLEX log

    pricing_problem.solve();

    for (int i = 0; i < n; i++) {
      std::cout << pricing_problem.getValue(x[i]) << " ";
    }

    if (pricing_problem.getObjValue() < -1e-5) {
      // std::cout << "Reduced cost is equal to " <<
      // pricing_problem.getObjValue() << ", which is less than 0..." <<
      // std::endl;

      IloNumArray entering_col(this->env1, n);

      pricing_problem.getValues(x, entering_col);

      // std::cout << std::endl << "Entering column:" << std::endl;
      for (size_t i = 0; i < n; i++) {
        // std::cout << (entering_col[i] < 0.5 ? 0 : 1) << std::endl;
      }
      // std::cout << std::endl;

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

        std::cout << "No column with negative reduced costs found. The current"
                    "basis is optimal"
                    << std::endl;
        std::cout << "Final master problem: " << std::endl;
        std::cout << "Bins usados: " << rmp.getObjValue() << std::endl;

        IloNumArray variables(this->env1, lambda.getSize());
        rmp.getValues(lambda, variables);
        for (int i = 0; i < lambda.getSize(); i++) {
            std::cout << "lambda[" << i << "] = " << variables[i] << std::endl;
        }

        for(int i = 0; i < n; i++){
            std::cout << this->masterConstraints[i].getQuadIterator() << std::endl;
        }

        sub_model.end();
        x.end();
        reduced_cost.end();
        weight_sum.end();
        pricing_problem.end();
        break;
    }

    sub_model.end();
    x.end();
    reduced_cost.end();
    weight_sum.end();
    pricing_problem.end();
  }

  return {};
}

void Problema::prune() {}