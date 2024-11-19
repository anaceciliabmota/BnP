#include "Data.h"
#include "Problema.h"
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

    Problema p(data, numeric_limits<double>::infinity());

    p.solve();

    return 0;
}
