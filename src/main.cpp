#include "Data.h"
#include "MasterProblem.h"
#include "Node.h"
#include <ctime>
#include <limits>
#include <stdio.h>
#include <iostream>
#include <list>
#include <ilcplex/ilocplex.h>
#include <chrono>
#include <time.h>

using namespace std;

Node Branch_and_Price(MasterProblem&, double);

int main(int argc, char **argv)
{
    clock_t start, end;
    start = clock();
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

    // Node no;
    // pair<int, int> items = mp.solve(no);

    // cout << items.first << " " << items.second << endl;


    // no.separados = {make_pair(0, 1)};
    // items = mp.solve(no);

    // cout << items.first << " " << items.second << endl;

    // no.separados = {make_pair(3, 4)};
    // items = mp.solve(no);
    Node solution = Branch_and_Price(mp, -numeric_limits<double>::infinity());
    end = clock();
    double time_used = double(end - start) / double(CLOCKS_PER_SEC);
    cout << "Solucao: " << solution.bins << endl;
    cout << "Tempo gasto: " << time_used << endl;
    return 0;
}

Node Branch_and_Price(MasterProblem& mp, double dual){

    list<Node> tree;
    list<Node>::iterator it;
    Node root;
    tree.push_back(root);
    Node best;
    best.bins = numeric_limits<double>::infinity();
    int cont = 1;
    while(!tree.empty()){
        //selecionando o ultimo (dfs)
        it = --tree.end();
        cout << "Nó de número " << cont << endl;
        cont++;
        Node node = tree.back();
        pair<int, int> selected_items = mp.solve(node);
        bool is_integer_solution = node.solution.size() > 0 ? true : false;
        cout << "Master é viável?" << (node.master_is_feasible ? "Sim" : "Não") << endl;
        cout << "É inteiro? " << (is_integer_solution ? "Sim" : "Não") << endl;
        std::cout << "Nós selecionados: " << selected_items.first << " " << selected_items.second << endl;
        //std::cout << node.master_is_feasible << endl;
        cout << "Decisão" << endl;
        if(!node.master_is_feasible || ceil(node.bins) > best.bins){
            std::cout << "entrou na primeira poda" << endl;
            tree.erase(it);
            continue;
        }else if(node.master_is_feasible && is_integer_solution){
            //std::cout << "entrou2" << endl;
            if(node.bins + EPSILON < best.bins){
                best = node;
            }
            std::cout << "entrou na segunda poda" << endl;

        }else{
            std::cout << "criou filhos" << endl << endl;
            if(node.bins > dual){
                dual = node.bins;
            }
            //std::cout << "entrou3" << endl;
            Node child1;
            child1.juntos = node.juntos; 
            child1.separados = node.separados; 
            child1.separados.push_back(selected_items);

            Node child2;
            child2.juntos = node.juntos;
            child2.separados = node.separados;
            child2.juntos.push_back(selected_items);

            tree.push_back(child1);
            tree.push_back(child2);
        }
        tree.erase(it);
        
    }
    return best;
}
