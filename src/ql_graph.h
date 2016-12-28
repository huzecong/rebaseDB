//
// Created by Kanari on 2016/12/28.
//

#ifndef REBASE_QL_GRAPH_H
#define REBASE_QL_GRAPH_H

#include <vector>

class QL_Graph {
    int n;
    std::vector<int> *edge;
    
    void addHalfEdge(int a, int b);
    
    std::vector<std::vector<int>> blocks;
    
    bool foundBlocks;
    
    void findBlocks();
    
public:
    QL_Graph(int nodeCount);
    virtual ~QL_Graph();
    
    void insertEdge(int a, int b);
    
    typedef decltype(blocks)::iterator iterator;
    iterator begin();
    iterator end();
};


#endif //REBASE_QL_GRAPH_H
