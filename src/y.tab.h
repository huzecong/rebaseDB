/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    RW_CREATE = 258,
    RW_DROP = 259,
    RW_TABLE = 260,
    RW_INDEX = 261,
    RW_LOAD = 262,
    RW_SET = 263,
    RW_HELP = 264,
    RW_PRINT = 265,
    RW_EXIT = 266,
    RW_SELECT = 267,
    RW_FROM = 268,
    RW_WHERE = 269,
    RW_INSERT = 270,
    RW_DELETE = 271,
    RW_UPDATE = 272,
    RW_AND = 273,
    RW_INTO = 274,
    RW_VALUES = 275,
    RW_DATABASE = 276,
    RW_DATABASES = 277,
    RW_TABLES = 278,
    RW_SHOW = 279,
    RW_USE = 280,
    RW_PRIMARY = 281,
    RW_KEY = 282,
    RW_NOT = 283,
    RW_NULL = 284,
    RW_IS = 285,
    RW_INT = 286,
    RW_DESC = 287,
    T_EQ = 288,
    T_LT = 289,
    T_LE = 290,
    T_GT = 291,
    T_GE = 292,
    T_NE = 293,
    T_EOF = 294,
    NOTOKEN = 295,
    RW_RESET = 296,
    RW_IO = 297,
    RW_BUFFER = 298,
    RW_RESIZE = 299,
    RW_QUERY_PLAN = 300,
    RW_ON = 301,
    RW_OFF = 302,
    T_INT = 303,
    T_REAL = 304,
    T_STRING = 305,
    T_QSTRING = 306,
    T_SHELL_CMD = 307
  };
#endif
/* Tokens.  */
#define RW_CREATE 258
#define RW_DROP 259
#define RW_TABLE 260
#define RW_INDEX 261
#define RW_LOAD 262
#define RW_SET 263
#define RW_HELP 264
#define RW_PRINT 265
#define RW_EXIT 266
#define RW_SELECT 267
#define RW_FROM 268
#define RW_WHERE 269
#define RW_INSERT 270
#define RW_DELETE 271
#define RW_UPDATE 272
#define RW_AND 273
#define RW_INTO 274
#define RW_VALUES 275
#define RW_DATABASE 276
#define RW_DATABASES 277
#define RW_TABLES 278
#define RW_SHOW 279
#define RW_USE 280
#define RW_PRIMARY 281
#define RW_KEY 282
#define RW_NOT 283
#define RW_NULL 284
#define RW_IS 285
#define RW_INT 286
#define RW_DESC 287
#define T_EQ 288
#define T_LT 289
#define T_LE 290
#define T_GT 291
#define T_GE 292
#define T_NE 293
#define T_EOF 294
#define NOTOKEN 295
#define RW_RESET 296
#define RW_IO 297
#define RW_BUFFER 298
#define RW_RESIZE 299
#define RW_QUERY_PLAN 300
#define RW_ON 301
#define RW_OFF 302
#define T_INT 303
#define T_REAL 304
#define T_STRING 305
#define T_QSTRING 306
#define T_SHELL_CMD 307

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 71 "parse.y" /* yacc.c:1909  */

    int ival;
    enum CompOp cval;
    float rval;
    char *sval;
    NODE *n;

#line 166 "y.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
