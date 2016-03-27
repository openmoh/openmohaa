/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
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

#ifndef YY_YY_CODE_GLOBALCPP_PARSER_YYPARSER_H_INCLUDED
# define YY_YY_CODE_GLOBALCPP_PARSER_YYPARSER_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     END = 0,
     TOKEN_EOL = 258,
     TOKEN_COMMA = 259,
     TOKEN_TERNARY = 260,
     TOKEN_SHIFT_RIGHT_EQUALS = 261,
     TOKEN_SHIFT_LEFT_EQUALS = 262,
     TOKEN_OR_EQUALS = 263,
     TOKEN_EXCL_OR_EQUALS = 264,
     TOKEN_AND_EQUALS = 265,
     TOKEN_MODULUS_EQUALS = 266,
     TOKEN_DIVIDE_EQUALS = 267,
     TOKEN_MULTIPLY_EQUALS = 268,
     TOKEN_MINUS_EQUALS = 269,
     TOKEN_PLUS_EQUALS = 270,
     TOKEN_ASSIGNMENT = 271,
     TOKEN_LOGICAL_OR = 272,
     TOKEN_LOGICAL_AND = 273,
     TOKEN_BITWISE_OR = 274,
     TOKEN_BITWISE_EXCL_OR = 275,
     TOKEN_BITWISE_AND = 276,
     TOKEN_INEQUALITY = 277,
     TOKEN_EQUALITY = 278,
     TOKEN_GREATER_THAN_OR_EQUAL = 279,
     TOKEN_GREATER_THAN = 280,
     TOKEN_LESS_THAN_OR_EQUAL = 281,
     TOKEN_LESS_THAN = 282,
     TOKEN_SHIFT_RIGHT = 283,
     TOKEN_SHIFT_LEFT = 284,
     TOKEN_MINUS = 285,
     TOKEN_PLUS = 286,
     TOKEN_MODULUS = 287,
     TOKEN_DIVIDE = 288,
     TOKEN_MULTIPLY = 289,
     TOKEN_LISTENER = 290,
     TOKEN_COMPLEMENT = 291,
     TOKEN_NOT = 292,
     TOKEN_NEG = 293,
     TOKEN_FLOAT = 294,
     TOKEN_INTEGER = 295,
     TOKEN_IDENTIFIER = 296,
     TOKEN_STRING = 297,
     TOKEN_NIL = 298,
     TOKEN_NULL = 299,
     TOKEN_LBRACKET = 300,
     TOKEN_RBRACKET = 301,
     TOKEN_COLON = 302,
     TOKEN_SEMICOLON = 303,
     TOKEN_DOLLAR = 304,
     TOKEN_DOUBLE_COLON = 305,
     TOKEN_NUMBER = 306,
     TOKEN_PERIOD = 307,
     TOKEN_DECREMENT = 308,
     TOKEN_INCREMENT = 309,
     TOKEN_RPAREN = 310,
     TOKEN_LPAREN = 311,
     TOKEN_RSQUARE = 312,
     TOKEN_LSQUARE = 313,
     TOKEN_MAKEARRAY = 314,
     TOKEN_ENDARRAY = 315,
     TOKEN_CATCH = 316,
     TOKEN_TRY = 317,
     TOKEN_DO = 318,
     TOKEN_FOR = 319,
     TOKEN_IF = 320,
     TOKEN_ELSE = 321,
     TOKEN_SWITCH = 322,
     TOKEN_WHILE = 323,
     TOKEN_BREAK = 324,
     TOKEN_CASE = 325,
     TOKEN_CONTINUE = 326,
     TOKEN_SIZE = 327,
     TOKEN_END = 328,
     TOKEN_RETURN = 329
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 2058 of yacc.c  */
#line 53 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"

	stype_t s;


/* Line 2058 of yacc.c  */
#line 137 "../../../code/globalcpp/parser/yyParser.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_YY_CODE_GLOBALCPP_PARSER_YYPARSER_H_INCLUDED  */
