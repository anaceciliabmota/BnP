
#include "PricingProblem.h"
#include "minknap.c"

#define M 1e6

std::vector<double> solve_sub_problem(int n, std::vector<double> &pi,
                                      double *objective_value, Node &no,
                                      const Data& data) {

  IloEnv env;
  IloModel model(env);
  IloBoolVarArray x(env, n);
  IloExpr reduced_cost(env);
  IloExpr weight_sum(env);

  reduced_cost = 1 - pi[0] * x[0];
  for (int i = 1; i < n; i++) {
    reduced_cost -= pi[i] * x[i];
  }

  IloObjective obj = IloMinimize(env, reduced_cost);
  model.add(obj);

  for (int i = 0; i < n; i++) {
    weight_sum += data.getItemWeight(i) * x[i];
  }

  model.add(weight_sum <= data.getBinCapacity());
  separate_pairs_subproblem(no, x, env, model);
  IloCplex pricing_problem(model);
  pricing_problem.setParam(IloCplex::Param::Threads, 1);
  pricing_problem.setOut(env.getNullStream()); // disables CPLEX log
  pricing_problem.solve();
  IloNumArray entering_col(env, n);
  pricing_problem.getValues(x, entering_col);

  *objective_value = pricing_problem.getObjValue();

  std::vector<double> entering_col_vector(n);
  for (IloInt i = 0; i < entering_col.getSize(); ++i) {
    entering_col_vector[i] = entering_col[i];
  }

  model.end();
  x.end();
  reduced_cost.end();
  weight_sum.end();
  obj.end();
  entering_col.end();
  pricing_problem.end();
  env.end();

  return entering_col_vector;
}

void separate_pairs_subproblem(Node &no, IloBoolVarArray &x, IloEnv &env,
                               IloModel &model) {

  for (int i = 0; i < no.separados.size(); i++) {
    int item_1 = no.separados[i].first;
    int item_2 = no.separados[i].second;
    IloExpr separation_constr(env);
    separation_constr = x[item_1] + x[item_2];
    model.add(separation_constr <= 1);
  }

  for (int i = 0; i < no.juntos.size(); i++) {
    int item_1 = no.juntos[i].first;
    int item_2 = no.juntos[i].second;
    model.add(x[item_1] == x[item_2]);
  }
}

std::vector<double> solve_pricing_minknap(int n, std::vector<double> &pi, double *objective_value, Node &no, const Data& data){
  int c = data.getBinCapacity();
  long *p = (long*)malloc(sizeof(long)*n);
  int *w = (int*)malloc(sizeof(int)*n);
  int *x = (int*)malloc(sizeof(int)*n);

  for(int i = 0; i < pi.size(); i++){
    p[i] = M*pi[i];
    w[i] = data.getItemWeight(i);
  }

  double z = minknap(n, p, w, x, c);

  *objective_value = 1-(z/M);
  

  std::vector<double> entering_column(n);

  for(int i = 0; i < n; i++){
    entering_column[i] = x[i];
  }

  free(x);
  free(p);
  free(w);

  return entering_column;

}

