//
// parser.h
//   Parser Component Interface
//

#ifndef PARSER_H
#define PARSER_H

#ifdef __cplusplus
# include <iostream>
#endif
#include "redbase.h"
//
// Structure declarations and output functions
//
struct AttrInfo {
	char     *attrName;   /* attribute name       */
	enum AttrType attrType;    /* type of attribute    */
	int      attrLength;  /* length of attribute  */
};

struct RelAttr {
	char     *relName;    // Relation name (may be NULL)
	char     *attrName;   // Attribute name
#ifdef __cplusplus
	// Print function
	friend std::ostream &operator<<(std::ostream &s, const RelAttr &ra);
#endif
};

struct Value {
	enum AttrType type;         /* type of value               */
	void     *data;        /* value                       */
#ifdef __cplusplus
			   /* print function              */
	friend std::ostream &operator<<(std::ostream &s, const Value &v);
#endif
};

struct Condition {
	struct RelAttr  lhsAttr;    /* left-hand side attribute            */
	enum CompOp   op;         /* comparison operator                 */
	int      bRhsIsAttr; /* TRUE if the rhs is an attribute,    */
						 /* in which case rhsAttr below is valid;*/
						 /* otherwise, rhsValue below is valid.  */
	struct RelAttr  rhsAttr;    /* right-hand side attribute            */
	struct Value    rhsValue;   /* right-hand side value                */
#ifdef __cplusplus
			 /* print function                               */
	friend std::ostream &operator<<(std::ostream &s, const Condition &c);
#endif

};

static struct Condition NULLCONDITION;

#ifdef __cplusplus
std::ostream &operator<<(std::ostream &s, const CompOp &op);
std::ostream &operator<<(std::ostream &s, const AttrType &at);
#endif

#ifdef __cplusplus
//
// Parse function
//
class PF_Manager;
class QL_Manager;
class SM_Manager;

void RBparse(PF_Manager &pfm, SM_Manager &smm, QL_Manager &qlm);
#endif

//
// Error printing function; calls component-specific functions
//
void PrintError(RC rc);

// bQueryPlans is allocated by parse.y.  When bQueryPlans is 1 then the
// query plan chosen for the SFW query will be displayed.  When
// bQueryPlans is 0 then no query plan is shown.
extern int bQueryPlans;

#endif
