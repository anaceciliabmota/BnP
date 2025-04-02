#ifndef CG_H
#define CG_H

#include "Data.h"
#include "Node.h"
#include "PricingProblem.h"
#include <ilcplex/ilocplex.h>
#include <cmath>
#include <limits>
#include <algorithm>


#define EPSILON 1e-6
#define M 1e6

class MasterProblem
{
    const Data& data;
    IloModel masterProblem;
    IloNumVarArray lambda;
    IloRangeArray masterConstraints;
    IloObjective masterObj;

    std::vector<std::vector<bool>> lambdaItens;
    std::pair<int, int> define_pair(int n, std::vector<std::vector<double>>& z, bool * is_feasible); // Explicar
    
    void generate_solution(std::vector<std::vector<bool>>& solution, IloCplex *  rmp,IloNumArray& variables);
    bool is_feasible_solution(IloCplex * rmp, IloNumArray& variables);
    //void separate_pairs_subproblem(Node& no, IloBoolVarArray& x, IloEnv& env, IloModel& model);

  public:
    MasterProblem(const Data &data, double UB);
    ~MasterProblem();
    void add_revert_pair_constraint(Node & no, bool ub); // Explicar
    //std::vector<double> solve_sub_problem(int n, std::vector<double>& pi, double * objective_value, Node& no);
    std::pair<int, int> solve(Node &no);
    void prune();
    IloEnv env;
    double bestInteger, UB, LB;
};

#endif