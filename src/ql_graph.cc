//
// Created by Kanari on 2016/12/28.
//

#include <glog/logging.h>
#include <cassert>
#include "ql_graph.h"

QL_Graph::QL_Graph(int nodeCount) {
    this->n = nodeCount;
    this->edge = new std::vector<int>[nodeCount];
    this->foundBlocks = false;
}

void QL_Graph::addHalfEdge(int a, int b) {
    this->edge[a].push_back(b);
    this->edge[b].push_back(a);
}

void QL_Graph::insertEdge(int a, int b) {
    assert(0 <= a && a < n && 0 <= b && b < n);
    addHalfEdge(a, b);
    addHalfEdge(b, a);
}

QL_Graph::~QL_Graph() {
    delete[] this->edge;
}

void QL_Graph::findBlocks() {
    bool *v = new bool[n];
    memset(v, 0, sizeof(bool) * n);

    for (int i = 0; i < n; ++i) {
        if (v[i]) continue;
        std::vector<int> current;
        current.push_back(i);
        v[i] = true;
        for (int j = 0; j < current.size(); ++j) {
            int x = current[j];
            for (int y : edge[x])
                if (!v[y]) {
                    v[y] = true;
                    current.push_back(y);
                }
        }
        blocks.push_back(current);
    }

    delete[] v;
    foundBlocks = true;
}

QL_Graph::iterator QL_Graph::begin() {
    if (!foundBlocks) findBlocks();
    return blocks.begin();
}

QL_Graph::iterator QL_Graph::end() {
    if (!foundBlocks) findBlocks();
    return blocks.end();
}
