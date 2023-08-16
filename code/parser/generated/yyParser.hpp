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

#ifndef YY_YY_USR_SRC_OPENMOHAA_CODE_PARSER_GENERATED_YYPARSER_HPP_INCLUDED
# define YY_YY_USR_SRC_OPENMOHAA_CODE_PARSER_GENERATED_YYPARSER_HPP_INCLUDED
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
    TOKEN_EOF = 0,                 /* TOKEN_EOF  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    TOKEN_EOL = 259,               /* TOKEN_EOL  */
    TOKEN_COMMA = 260,             /* TOKEN_COMMA  */
    TOKEN_IF = 261,                /* TOKEN_IF  */
    THEN = 262,                    /* THEN  */
    TOKEN_ELSE = 263,              /* TOKEN_ELSE  */
    TOKEN_WHILE = 264,             /* TOKEN_WHILE  */
    TOKEN_FOR = 265,               /* TOKEN_FOR  */
    TOKEN_DO = 266,                /* TOKEN_DO  */
    TOKEN_IDENTIFIER = 267,        /* TOKEN_IDENTIFIER  */
    TOKEN_LEFT_BRACES = 268,       /* TOKEN_LEFT_BRACES  */
    TOKEN_RIGHT_BRACES = 269,      /* TOKEN_RIGHT_BRACES  */
    TOKEN_LEFT_BRACKET = 270,      /* TOKEN_LEFT_BRACKET  */
    TOKEN_RIGHT_BRACKET = 271,     /* TOKEN_RIGHT_BRACKET  */
    TOKEN_LEFT_SQUARE_BRACKET = 272, /* TOKEN_LEFT_SQUARE_BRACKET  */
    TOKEN_RIGHT_SQUARE_BRACKET = 273, /* TOKEN_RIGHT_SQUARE_BRACKET  */
    TOKEN_ASSIGNMENT = 274,        /* TOKEN_ASSIGNMENT  */
    TOKEN_PLUS_EQUALS = 275,       /* TOKEN_PLUS_EQUALS  */
    TOKEN_MINUS_EQUALS = 276,      /* TOKEN_MINUS_EQUALS  */
    TOKEN_MULTIPLY_EQUALS = 277,   /* TOKEN_MULTIPLY_EQUALS  */
    TOKEN_DIVIDE_EQUALS = 278,     /* TOKEN_DIVIDE_EQUALS  */
    TOKEN_MODULUS_EQUALS = 279,    /* TOKEN_MODULUS_EQUALS  */
    TOKEN_AND_EQUALS = 280,        /* TOKEN_AND_EQUALS  */
    TOKEN_EXCL_OR_EQUALS = 281,    /* TOKEN_EXCL_OR_EQUALS  */
    TOKEN_OR_EQUALS = 282,         /* TOKEN_OR_EQUALS  */
    TOKEN_SHIFT_LEFT_EQUALS = 283, /* TOKEN_SHIFT_LEFT_EQUALS  */
    TOKEN_SHIFT_RIGHT_EQUALS = 284, /* TOKEN_SHIFT_RIGHT_EQUALS  */
    TOKEN_TERNARY = 285,           /* TOKEN_TERNARY  */
    TOKEN_COLON = 286,             /* TOKEN_COLON  */
    TOKEN_LOGICAL_OR = 287,        /* TOKEN_LOGICAL_OR  */
    TOKEN_LOGICAL_AND = 288,       /* TOKEN_LOGICAL_AND  */
    TOKEN_BITWISE_OR = 289,        /* TOKEN_BITWISE_OR  */
    TOKEN_BITWISE_EXCL_OR = 290,   /* TOKEN_BITWISE_EXCL_OR  */
    TOKEN_BITWISE_AND = 291,       /* TOKEN_BITWISE_AND  */
    TOKEN_EQUALITY = 292,          /* TOKEN_EQUALITY  */
    TOKEN_INEQUALITY = 293,        /* TOKEN_INEQUALITY  */
    TOKEN_LESS_THAN = 294,         /* TOKEN_LESS_THAN  */
    TOKEN_LESS_THAN_OR_EQUAL = 295, /* TOKEN_LESS_THAN_OR_EQUAL  */
    TOKEN_GREATER_THAN = 296,      /* TOKEN_GREATER_THAN  */
    TOKEN_GREATER_THAN_OR_EQUAL = 297, /* TOKEN_GREATER_THAN_OR_EQUAL  */
    TOKEN_SHIFT_LEFT = 298,        /* TOKEN_SHIFT_LEFT  */
    TOKEN_SHIFT_RIGHT = 299,       /* TOKEN_SHIFT_RIGHT  */
    TOKEN_PLUS = 300,              /* TOKEN_PLUS  */
    TOKEN_MINUS = 301,             /* TOKEN_MINUS  */
    TOKEN_MULTIPLY = 302,          /* TOKEN_MULTIPLY  */
    TOKEN_DIVIDE = 303,            /* TOKEN_DIVIDE  */
    TOKEN_MODULUS = 304,           /* TOKEN_MODULUS  */
    TOKEN_NEG = 305,               /* TOKEN_NEG  */
    TOKEN_NOT = 306,               /* TOKEN_NOT  */
    TOKEN_COMPLEMENT = 307,        /* TOKEN_COMPLEMENT  */
    TOKEN_STRING = 308,            /* TOKEN_STRING  */
    TOKEN_INTEGER = 309,           /* TOKEN_INTEGER  */
    TOKEN_FLOAT = 310,             /* TOKEN_FLOAT  */
    TOKEN_LISTENER = 311,          /* TOKEN_LISTENER  */
    TOKEN_NIL = 312,               /* TOKEN_NIL  */
    TOKEN_NULL = 313,              /* TOKEN_NULL  */
    TOKEN_DOUBLE_COLON = 314,      /* TOKEN_DOUBLE_COLON  */
    TOKEN_SEMICOLON = 315,         /* TOKEN_SEMICOLON  */
    TOKEN_DOLLAR = 316,            /* TOKEN_DOLLAR  */
    TOKEN_NUMBER = 317,            /* TOKEN_NUMBER  */
    TOKEN_INCREMENT = 318,         /* TOKEN_INCREMENT  */
    TOKEN_DECREMENT = 319,         /* TOKEN_DECREMENT  */
    TOKEN_PERIOD = 320,            /* TOKEN_PERIOD  */
    TOKEN_CATCH = 321,             /* TOKEN_CATCH  */
    TOKEN_TRY = 322,               /* TOKEN_TRY  */
    TOKEN_SWITCH = 323,            /* TOKEN_SWITCH  */
    TOKEN_CASE = 324,              /* TOKEN_CASE  */
    TOKEN_BREAK = 325,             /* TOKEN_BREAK  */
    TOKEN_CONTINUE = 326,          /* TOKEN_CONTINUE  */
    TOKEN_SIZE = 327,              /* TOKEN_SIZE  */
    TOKEN_END = 328,               /* TOKEN_END  */
    TOKEN_RETURN = 329,            /* TOKEN_RETURN  */
    TOKEN_MAKEARRAY = 330,         /* TOKEN_MAKEARRAY  */
    TOKEN_ENDARRAY = 331           /* TOKEN_ENDARRAY  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 59 "/usr/src/openmohaa/code/parser/bison_source.txt"

	stype_t s;

#line 143 "/usr/src/openmohaa/code/parser/generated/yyParser.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
typedef  parse_pos_t  YYLTYPE;


extern YYSTYPE yylval;
extern YYLTYPE yylloc;

int yyparse (void);


#endif /* !YY_YY_USR_SRC_OPENMOHAA_CODE_PARSER_GENERATED_YYPARSER_HPP_INCLUDED  */
