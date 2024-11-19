#include "ilconcert/iloexpression.h"
#include <ilcplex/ilocplex.h>
#include <vector>
#include <iostream>
#include <cstdlib>

using namespace std;

int main() 
{
	const double M = 1e6;
	vector<int> weight = {2, 1, 3, 3, 5};
	int capacity = 7;
	int n = weight.size();

	IloEnv env;
	IloModel master_model(env);

	IloNumVarArray lambda(env, n, 0, IloInfinity);
	

	IloExpr sum_obj(env);
	IloRangeArray partition_constraint(env);

	for (int i = 0; i < n; i++)
	{
		char var_name[50];
		sprintf(var_name, "y%d", i);

		lambda[i].setName(var_name);
		sum_obj += M * lambda[i];

		partition_constraint.add(lambda[i] == 1);
	}

	master_model.add(partition_constraint);

	IloObjective master_objective = IloMinimize(env, sum_obj);
	master_model.add(master_objective);

	IloCplex rmp(master_model);
	rmp.exportModel("model.lp");
	rmp.setOut(env.getNullStream()); // disables CPLEX log

	rmp.solve();

	cout << "Initial lower bound: " << rmp.getObjValue() << endl;

	cout << "Initial solution: ";
	for (size_t j = 0; j < lambda.getSize(); j++)
	{
		cout << rmp.getValue(lambda[j]) << " ";
	}
	cout << endl;

	int lambda_counter = n;
    
	while(true)
	{
		// Get the dual variables
		IloNumArray pi(env, n);

		rmp.getDuals(pi, partition_constraint);

		for (size_t i = 0; i < n; i++)
		{
			cout << "Dual variable of constraint " << i << " = " << pi[i] << endl;
		}
        
        IloModel sub_model(env);
        IloBoolVarArray x(env, n);
        IloExpr reduced_cost(env);
        IloExpr weight_sum(env);

        reduced_cost = 1 - pi[0] *x[0];

        for(int i = 1; i < n; i++){
            reduced_cost -= pi[i] * x[i];
        }
        
        IloObjective sub_objective = IloMinimize(env, reduced_cost);
        sub_model.add(sub_objective);

        for(int i = 0; i < n; i++){
            weight_sum += weight[i] * x[i];
        }

		sub_model.add(weight_sum <= capacity);
        IloCplex pricing_problem(sub_model);

        pricing_problem.setOut(env.getNullStream()); // disables CPLEX log

        pricing_problem.solve();

        for(int i = 0; i < n; i++){
            cout << pricing_problem.getValue(x[i]) << " ";
        }

		reduced_cost.end();
        weight_sum.end();
        
        
		if (pricing_problem.getObjValue() < -1e-5)
		{
			cout << "Reduced cost is equal to " << pricing_problem.getObjValue() << ", which is less than 0..." << endl;

			IloNumArray entering_col(env, n);

			pricing_problem.getValues(x, entering_col);

			cout << endl << "Entering column:" << endl;
			for (size_t i = 0; i < n; i++)
			{
				cout << (entering_col[i] < 0.5 ? 0 : 1) << endl;
			}
			cout << endl;

			// Add the column to the master problem
			// (the cost of the new variable is always 1)
			char var_name[50];
			sprintf(var_name, "y%d", lambda_counter++);
			IloNumVar new_lambda(master_objective(1) + partition_constraint(entering_col), 0, IloInfinity);
			new_lambda.setName(var_name);

			lambda.add(new_lambda);

			cout << "Solving the RMP again..." << endl;

			rmp.solve();

			rmp.exportModel("model.lp");
		}
		else
		{
			cout << "No column with negative reduced costs found. The current basis is optimal" << endl;
			cout << "Final master problem: " << endl;
			IloNumArray variables(env, lambda.getSize());
			rmp.getValues(lambda, variables);
			for(int i = 0; i < lambda.getSize();i++){
				cout << "lambda["<< i <<"] = " << variables[i] << endl;
			}
			rmp.exportModel("model.lp");
			//system("cat model.lp");
			break;
		}
		sub_model.end();
		pricing_problem.end();


	}
	env.end();
    /*
	cout << endl;
	cout << "Forcing items 1 and 2 to be separated in the master (for branch-and-price only): " << endl;
	// 0 1 2 3 4 5 6 7 8 9 10 11
	//                           
	// 1 0 0 0 0 1 1 1 0 1  0  0
	// 0 1 0 0 0 1 1 0 0 0  1  1
	// 0 0 1 0 0 1 0 1 1 0  0  1
	// 0 0 0 1 0 0 1 0 1 0  0  1
	// 0 0 0 0 1 0 0 0 0 1  1  0

	// itens 1 and 2 are together only on columns 5 and 11
	lambda[11].setUB(0.0);
	lambda[5].setUB(0.0);

	// to allow them again:
	// lambda[5].setUB(IloInfinity);
	// lambda[11].setUB(IloInfinity);

	rmp.solve();

	for (size_t j = 0; j < lambda.getSize(); j++)
	{
		cout << rmp.getValue(lambda[j]) << " ";
	}
	cout << endl;
	env.end();
    */

	return 0;
}