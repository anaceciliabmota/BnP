#include "Data.h"
#include "MasterProblem.h"
#include "Node.h"
#include <limits>
#include <stdio.h>
#include <iostream>
#include <list>
#include <ilcplex/ilocplex.h>
#include <chrono>

using namespace std;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage:\n./bin instance\n");
        return 0;
    }

    Data data;
    data.readData(argv[1]);

    cout << "bin capacity: " << data.getBinCapacity() << endl;

    cout << "items: " << data.getNItems() << endl;

    MasterProblem mp(data, numeric_limits<double>::infinity());

    Node no;
    pair<int, int> items = mp.solve(no);

    cout << items.first << " " << items.second << endl;


    no.separados = {make_pair(0, 1)};
    items = mp.solve(no);

    cout << items.first << " " << items.second << endl;

    no.separados = {make_pair(3, 4)};
    items = mp.solve(no);

    cout << items.first << " " << items.second << endl;

    return 0;
}

void Branch_and_Price(MasterProblem& mp, double upper_bound){

    list<Node> tree;
    list<Node>::iterator it;
    Node root;
    root.LB = numeric_limits<double>::infinity();

    Node best


}
