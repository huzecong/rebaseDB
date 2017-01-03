//
// Created by Kanari on 2017/1/3.
//

#include "ql_disjoint.h"

QL_DisjointSet::QL_DisjointSet(int n) {
    this->n = n;
    this->f = new int[n];
    for (int i = 0; i < n; ++i)
        f[i] = i;
}

QL_DisjointSet::~QL_DisjointSet() {
    delete[] this->f;
}

int QL_DisjointSet::find(int x) {
    return x == f[x] ? x : f[x] = find(f[x]);
}

int QL_DisjointSet::join(int a, int b) {
    return f[find(a)] = find(b);
}

bool QL_DisjointSet::connected(int a, int b) {
    return find(a) == find(b);
}
