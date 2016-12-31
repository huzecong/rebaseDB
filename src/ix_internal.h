#pragma once

#include "redbase.h"

static const int kLastFreePage = -1;
static const int kNullNode = -1;

struct IX_FileHeader {
    AttrType attrType;
    int attrLength;
    int firstFreePage;
};

enum IX_NodeType {
    kInternalNode,
    kLeafNode,
};

struct IX_PageHeader {
    IX_NodeType type;
    short childrenNum;
};

struct Entry {
    // internal nodes: pageNum = the pageNum of child
    // leaf nodes: RID(pageNum, slotNum) is the data, with the exception that
    //     the last entry (entries[header->childrenNum])
    //     contains the pageNum to next leaf node
    int pageNum;
    int slotNum;
    char key[4];
};

template <int N, class T>
inline T upper_align(T x) {
    return (x + (N - 1)) & ~((unsigned)(N - 1));
}

