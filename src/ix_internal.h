#pragma once

#include "redbase.h"
#include "rm_rid.h"

static const int kLastFreePage = -1;
static const int kNullNode = -1;
static const int kInvalidBucket = -1;

struct IX_FileHeader {
    AttrType attrType;
    int attrLength;
    int root;
    int firstFreePage;
};

enum IX_NodeType {
    kInternalNode,
    kLeafNode,
};

struct IX_PageHeader {
    short type;
    short childrenNum;
    char entries[4];
};

struct Entry {
    // internal nodes: pageNum = the pageNum of child
    // leaf nodes: pageNum is the page no. of corresponding bucket, with the exception that
    //     the last entry (entries[header->childrenNum])
    //     contains the pageNum to next leaf node
    int pageNum;
    char key[4];
};

struct IX_BucketHeader {
    int ridNum;
    RID rids[1];
};

