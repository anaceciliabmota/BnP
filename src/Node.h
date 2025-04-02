#ifndef NODE_H
#define NODE_H

#include "Data.h"
#include <ilcplex/ilocplex.h>
#include <vector>
#include <map>

#define EPSILON 0.000001

class Node
{
public:
    bool master_is_feasible;
    bool tipo_branch; // 1 se estão juntos, 0 se estão separados    
    double bins;
    double LB;
    std::vector<std::vector<bool>> solution;
    std::vector<std::pair<int, int>> separados, juntos;    
};

#endif