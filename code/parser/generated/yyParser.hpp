/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_Z_OPENMOHAA_CODE_PARSER_GENERATED_YYPARSER_HPP_INCLUDED
# define YY_YY_Z_OPENMOHAA_CODE_PARSER_GENERATED_YYPARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    END = 0,                       /* END  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    TOKEN_EOL = 258,               /* TOKEN_EOL  */
    TOKEN_COMMA = 259,             /* TOKEN_COMMA  */
    TOKEN_ASSIGNMENT = 260,        /* TOKEN_ASSIGNMENT  */
    TOKEN_PLUS_EQUALS = 261,       /* TOKEN_PLUS_EQUALS  */
    TOKEN_MINUS_EQUALS = 262,      /* TOKEN_MINUS_EQUALS  */
    TOKEN_MULTIPLY_EQUALS = 263,   /* TOKEN_MULTIPLY_EQUALS  */
    TOKEN_DIVIDE_EQUALS = 264,     /* TOKEN_DIVIDE_EQUALS  */
    TOKEN_MODULUS_EQUALS = 265,    /* TOKEN_MODULUS_EQUALS  */
    TOKEN_AND_EQUALS = 266,        /* TOKEN_AND_EQUALS  */
    TOKEN_EXCL_OR_EQUALS = 267,    /* TOKEN_EXCL_OR_EQUALS  */
    TOKEN_OR_EQUALS = 268,         /* TOKEN_OR_EQUALS  */
    TOKEN_SHIFT_LEFT_EQUALS = 269, /* TOKEN_SHIFT_LEFT_EQUALS  */
    TOKEN_SHIFT_RIGHT_EQUALS = 270, /* TOKEN_SHIFT_RIGHT_EQUALS  */
    TOKEN_TERNARY = 271,           /* TOKEN_TERNARY  */
    TOKEN_LOGICAL_OR = 272,        /* TOKEN_LOGICAL_OR  */
    TOKEN_LOGICAL_AND = 273,       /* TOKEN_LOGICAL_AND  */
    TOKEN_BITWISE_OR = 274,        /* TOKEN_BITWISE_OR  */
    TOKEN_BITWISE_EXCL_OR = 275,   /* TOKEN_BITWISE_EXCL_OR  */
    TOKEN_BITWISE_AND = 276,       /* TOKEN_BITWISE_AND  */
    TOKEN_EQUALITY = 277,          /* TOKEN_EQUALITY  */
    TOKEN_INEQUALITY = 278,        /* TOKEN_INEQUALITY  */
    TOKEN_LESS_THAN = 279,         /* TOKEN_LESS_THAN  */
    TOKEN_LESS_THAN_OR_EQUAL = 280, /* TOKEN_LESS_THAN_OR_EQUAL  */
    TOKEN_GREATER_THAN = 281,      /* TOKEN_GREATER_THAN  */
    TOKEN_GREATER_THAN_OR_EQUAL = 282, /* TOKEN_GREATER_THAN_OR_EQUAL  */
    TOKEN_SHIFT_LEFT = 283,        /* TOKEN_SHIFT_LEFT  */
    TOKEN_SHIFT_RIGHT = 284,       /* TOKEN_SHIFT_RIGHT  */
    TOKEN_PLUS = 285,              /* TOKEN_PLUS  */
    TOKEN_MINUS = 286,             /* TOKEN_MINUS  */
    TOKEN_MULTIPLY = 287,          /* TOKEN_MULTIPLY  */
    TOKEN_DIVIDE = 288,            /* TOKEN_DIVIDE  */
    TOKEN_MODULUS = 289,           /* TOKEN_MODULUS  */
    TOKEN_LISTENER = 290,          /* TOKEN_LISTENER  */
    TOKEN_NEG = 291,               /* TOKEN_NEG  */
    TOKEN_NOT = 292,               /* TOKEN_NOT  */
    TOKEN_COMPLEMENT = 293,        /* TOKEN_COMPLEMENT  */
    TOKEN_FLOAT = 294,             /* TOKEN_FLOAT  */
    TOKEN_INTEGER = 295,           /* TOKEN_INTEGER  */
    TOKEN_IDENTIFIER = 296,        /* TOKEN_IDENTIFIER  */
    TOKEN_STRING = 297,            /* TOKEN_STRING  */
    TOKEN_NIL = 298,               /* TOKEN_NIL  */
    TOKEN_NULL = 299,              /* TOKEN_NULL  */
    TOKEN_LBRACKET = 300,          /* TOKEN_LBRACKET  */
    TOKEN_RBRACKET = 301,          /* TOKEN_RBRACKET  */
    TOKEN_COLON = 302,             /* TOKEN_COLON  */
    TOKEN_SEMICOLON = 303,         /* TOKEN_SEMICOLON  */
    TOKEN_DOLLAR = 304,            /* TOKEN_DOLLAR  */
    TOKEN_DOUBLE_COLON = 305,      /* TOKEN_DOUBLE_COLON  */
    TOKEN_NUMBER = 306,            /* TOKEN_NUMBER  */
    TOKEN_LSQUARE = 307,           /* TOKEN_LSQUARE  */
    TOKEN_RSQUARE = 308,           /* TOKEN_RSQUARE  */
    TOKEN_LPAREN = 309,            /* TOKEN_LPAREN  */
    TOKEN_RPAREN = 310,            /* TOKEN_RPAREN  */
    TOKEN_INCREMENT = 311,         /* TOKEN_INCREMENT  */
    TOKEN_DECREMENT = 312,         /* TOKEN_DECREMENT  */
    TOKEN_PERIOD = 313,            /* TOKEN_PERIOD  */
    TOKEN_MAKEARRAY = 314,         /* TOKEN_MAKEARRAY  */
    TOKEN_ENDARRAY = 315,          /* TOKEN_ENDARRAY  */
    TOKEN_CATCH = 316,             /* TOKEN_CATCH  */
    TOKEN_TRY = 317,               /* TOKEN_TRY  */
    TOKEN_DO = 318,                /* TOKEN_DO  */
    TOKEN_FOR = 319,               /* TOKEN_FOR  */
    TOKEN_IF = 320,                /* TOKEN_IF  */
    TOKEN_ELSE = 321,              /* TOKEN_ELSE  */
    TOKEN_SWITCH = 322,            /* TOKEN_SWITCH  */
    TOKEN_WHILE = 323,             /* TOKEN_WHILE  */
    TOKEN_BREAK = 324,             /* TOKEN_BREAK  */
    TOKEN_CASE = 325,              /* TOKEN_CASE  */
    TOKEN_CONTINUE = 326,          /* TOKEN_CONTINUE  */
    TOKEN_SIZE = 327,              /* TOKEN_SIZE  */
    TOKEN_END = 328,               /* TOKEN_END  */
    TOKEN_RETURN = 329             /* TOKEN_RETURN  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 50 "Z:/openmohaa/code/parser/bison_source.txt"

	stype_t s;

#line 142 "Z:/openmohaa/code/parser/generated/yyParser.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_Z_OPENMOHAA_CODE_PARSER_GENERATED_YYPARSER_HPP_INCLUDED  */
