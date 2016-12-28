//
// ql.h
//   Query Language Component Interface
//

// This file only gives the stub for the QL component

#ifndef QL_H
#define QL_H

#include <stdlib.h>
#include <string.h>
#include <ostream>
#include "redbase.h"
#include "parser.h"
#include "rm.h"
#include "ix.h"
#include "sm.h"
#include "ql_queryplan.h"

typedef std::pair<std::string, std::string> AttrTag;

template <typename T>
using AttrMap = std::map<AttrTag, T>;

//
// QL_Manager: query language (DML)
//
class QL_Manager {
public:
    QL_Manager (SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm);
    ~QL_Manager();                               // Destructor

    RC Select  (int nSelAttrs,                   // # attrs in select clause
                const RelAttr selAttrs[],        // attrs in select clause
                int   nRelations,                // # relations in from clause
                const char * const relations[],  // relations in from clause
                int   nConditions,               // # conditions in where clause
                const Condition conditions[]);   // conditions in where clause

    RC Insert  (const char *relName,             // relation to insert into
                int   nValues,                   // # values
                const Value values[]);           // values to insert

    RC Delete  (const char *relName,             // relation to delete from
                int   nConditions,               // # conditions in where clause
                const Condition conditions[]);   // conditions in where clause

    RC Update  (const char *relName,             // relation to update
                const RelAttr &updAttr,          // attribute to update
                const int bIsValue,              // 1 if RHS is a value, 0 if attr
                const RelAttr &rhsRelAttr,       // attr on RHS to set LHS eq to
                const Value &rhsValue,           // or value to set attr eq to
                int   nConditions,               // # conditions in where clause
                const Condition conditions[]);   // conditions in where clause

private:
    SM_Manager *pSmm;
    IX_Manager *pIxm;
    RM_Manager *pRmm;
    
    RC PrintQueryPlan(const QL_QueryPlan &queryPlan, int indent = 0);

    RC ExecuteQueryPlan(const QL_QueryPlan &queryPlan,
                        const AttrMap<DataAttrInfo &> &attrMap,
                        const AttrMap<Value> &valMap);
};

//
// Print-error function
//
void QL_PrintError(RC rc);

#define QL_ATTR_COUNT_MISMATCH      (START_QL_WARN + 0)
#define QL_VALUE_TYPES_MISMATCH     (START_QL_WARN + 1)
#define QL_STRING_VAL_TOO_LONG      (START_QL_WARN + 2)
#define QL_ATTR_NOTEXIST            (START_QL_WARN + 3)
#define QL_ATTR_TYPES_MISMATCH      (START_QL_WARN + 4)
#define QL_LASTWARN QL_ATTR_NOTEXIST

#define QL_SOMEERROR                (START_QL_ERR - 0)
#define QL_LASTERROR QL_SOMEERROR

#endif // QL_H
