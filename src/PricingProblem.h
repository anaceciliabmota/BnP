#ifndef PRICING_PROBLEM_H
#define PRICING_PROBLEM_H

#include "Data.h"
#include "Node.h"
#include <algorithm>
#include <cmath>
#include <ilcplex/ilocplex.h>
#include <limits>


std::vector<double> solve_sub_problem(int n, std::vector<double> &pi,
                                      double *objective_value, Node &no,
                                      const Data& data);

void separate_pairs_subproblem(Node &no, IloBoolVarArray &x, IloEnv &env,
                               IloModel &model);

                               std::vector<double> solve_pricing_minknap(int n, std::vector<double> &pi, double *objective_value, Node &no, const Data& data);
#endif