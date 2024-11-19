#ifndef CG_H
#define CG_H

#include "Data.h"
#include "Node.h"
#include <ilcplex/ilocplex.h>
#include <cmath>
#include <limits>
#include <algorithm>
#include <vector>

#define EPSILON 1e-6
#define M 1e6

class Problema
{
    const Data& data;
    IloModel masterProblem;
    IloNumVarArray lambda;
    IloRangeArray masterConstraints;
    IloObjective masterObj;

    std::vector<std::vector<bool>> lambdaItens;
    std::vector<std::vector<double>> z;

  public:
    Problema(const Data &data, double UB);
    std::pair<int, int> solve();
    void prune();
    IloEnv env1;
    double bestInteger, UB, LB;
};

#endif