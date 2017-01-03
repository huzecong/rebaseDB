//
// Created by Kanari on 2017/1/3.
//

#ifndef REBASE_QL_DISJOINT_H
#define REBASE_QL_DISJOINT_H


class QL_DisjointSet {
    int *f, n;

public:
    QL_DisjointSet(int n);
    ~QL_DisjointSet();
    int find(int x);
    int join(int a, int b);
    bool connected(int a, int b);
};


#endif //REBASE_QL_DISJOINT_H
