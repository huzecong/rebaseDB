//
// redbase.h
//   global declarations
//
#ifndef REDBASE_H
#define REDBASE_H

// Please DO NOT include any other files in this file.

//
// Globally-useful defines
//
#define MAXNAME       24                // maximum length of a relation
                                        // or attribute name
#define MAXSTRINGLEN  255               // maximum length of a
                                        // string-type attribute
#define MAXATTRS      40                // maximum number of attributes
                                        // in a relation
#define MAXINSERTATTRS  1024            // maximum number of attributes
                                        // in a single INSERT command

//#define yywrap() 1
inline static int yywrap() {
    return 1;
}
void yyerror(const char *);

//
// Return codes
//
typedef int RC;

#define OK_RC         0    // OK_RC return code is guaranteed to always be 0

#define START_PF_ERR  (-1)
#define END_PF_ERR    (-100)
#define START_RM_ERR  (-101)
#define END_RM_ERR    (-200)
#define START_IX_ERR  (-201)
#define END_IX_ERR    (-300)
#define START_SM_ERR  (-301)
#define END_SM_ERR    (-400)
#define START_QL_ERR  (-401)
#define END_QL_ERR    (-500)

#define START_PF_WARN  1
#define END_PF_WARN    100
#define START_RM_WARN  101
#define END_RM_WARN    200
#define START_IX_WARN  201
#define END_IX_WARN    300
#define START_SM_WARN  301
#define END_SM_WARN    400
#define START_QL_WARN  401
#define END_QL_WARN    500

// ALL_PAGES is defined and used by the ForcePages method defined in RM
// and PF layers
const int ALL_PAGES = -1;

//
// Attribute types
//
enum AttrType {
    INT,
    FLOAT,
    STRING
};

enum ValueType {
    VT_NULL,
    VT_INT,
    VT_FLOAT,
    VT_STRING,
};

// Attribute specifications
enum AttrSpec {
    ATTR_SPEC_NONE = 0x0,
    ATTR_SPEC_NOTNULL = 0x1,
    ATTR_SPEC_PRIMARYKEY = 0x2,
};

//
// Comparison operators
//
enum CompOp {
    NO_OP,                                      // no comparison
    ISNULL_OP, NOTNULL_OP,                      // unary operators
    EQ_OP, NE_OP, LT_OP, GT_OP, LE_OP, GE_OP    // binary atomic operators
};

//
// Pin Strategy Hint
//
enum ClientHint {
    NO_HINT                                     // default value
};

//
// TRUE, FALSE and BOOLEAN
//
#ifndef BOOLEAN
typedef char Boolean;
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef NULL
#define NULL 0
#endif


#define CVOID(_x) (*(reinterpret_cast<char**>(&(_x))))

#define ARR_PTR(_name, _type, _size) \
    auto __##_name##__ = std::make_unique<_type[]>((size_t)_size); \
    _type * _name = __##_name##__.get()
//#define ARR_PTR(_name, _type, _size) _type * _name = new _type[(size_t)_size]

#ifdef __cplusplus
template <int N, class T>
inline T upper_align(T x) {
    return (x + (N - 1)) & ~((unsigned)(N - 1));
}

#include <glog/logging.h>

#define TRY(_x) \
    if (int __rc = (_x)) { \
        VLOG(1) << "non-zero return code " << __rc; \
        return __rc; \
    }

#else

#define TRY(_x) \
    if (int __rc = (-x)) { \
        return __rc; \
    }

#endif // __cpluslus

#endif
