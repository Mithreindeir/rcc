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

#ifndef YY_YY_INCLUDE_PARSER_H_INCLUDED
# define YY_YY_INCLUDE_PARSER_H_INCLUDED
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
    T_IDENT = 258,
    T_CINT = 259,
    T_CDOUBLE = 260,
    T_ASN = 261,
    T_EQ = 262,
    T_NEQ = 263,
    T_LT = 264,
    T_LTE = 265,
    T_LPAREN = 266,
    T_RPAREN = 267,
    T_LCBRK = 268,
    T_RCBRK = 269,
    T_DOT = 270,
    T_COMMA = 271,
    T_MUL = 272,
    T_DIV = 273,
    T_PLUS = 274,
    T_SUB = 275,
    T_PTRMEM = 276,
    T_INC = 277,
    T_DEC = 278,
    T_LBRCK = 279,
    T_RBRCK = 280,
    T_GTE = 281,
    T_GT = 282,
    T_SEMIC = 283,
    T_VOID = 284,
    T_CHAR = 285,
    T_SHORT = 286,
    T_INT = 287,
    T_LONG = 288,
    T_SIGNED = 289,
    T_UNSIGNED = 290,
    T_FLOAT = 291,
    T_DOUBLE = 292,
    T_IF = 293,
    T_ELSE = 294,
    T_FOR = 295,
    T_WHILE = 296,
    T_DO = 297,
    T_BAND = 298,
    T_AND = 299,
    T_BOR = 300,
    T_OR = 301,
    T_XOR = 302
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 15 "src/parser.y" /* yacc.c:1909  */

	t_expr *expr;
	t_ident *ident;
	t_numeric *cnumeric;
	t_declr *declr;
	t_dir_declr *ddeclr;
	t_decl_spec *decl_spec;
	t_conditional_stmt * cstmt;
	t_iterative_stmt *itstmt;
	t_block *block;
	t_stmt *statement;
	char *string;
	int token;
	int type;
	int oper;
	int ptr;

#line 120 "include/parser.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_INCLUDE_PARSER_H_INCLUDED  */
