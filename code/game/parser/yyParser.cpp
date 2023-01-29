/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison implementation for Yacc-like parsers in C
   
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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.7"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
/* Line 371 of yacc.c  */
#line 1 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"

/*
* ===========================================================================
* Copyright (C) 2015 the OpenMoHAA team
* 
* This file is part of OpenMoHAA source code.
* 
* OpenMoHAA source code is free software; you can redistribute it
* and/or modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of the License,
* or (at your option) any later version.
* 
* OpenMoHAA source code is distributed in the hope that it will be
* useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with OpenMoHAA source code; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
* ===========================================================================
*
*
* yyParser.*: BISON Parser for MoHScript.
*/

#include "compiler.h"
#include "yyParser.h"
#include "yyLexer.h"

int yyerror( const char *msg );

extern int prev_yylex;

extern yyparsedata parsedata;

#define YYLLOC node_pos( parsedata.pos - yyleng )


/* Line 371 of yacc.c  */
#line 108 "../../../code/globalcpp/parser/yyParser.cpp"

# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "yyParser.h".  */
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
/* Line 387 of yacc.c  */
#line 53 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"

	stype_t s;


/* Line 387 of yacc.c  */
#line 231 "../../../code/globalcpp/parser/yyParser.cpp"
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

/* Copy the second part of user declarations.  */

/* Line 390 of yacc.c  */
#line 259 "../../../code/globalcpp/parser/yyParser.cpp"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(N) (N)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1284

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  75
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  16
/* YYNRULES -- Number of rules.  */
#define YYNRULES  103
/* YYNRULES -- Number of states.  */
#define YYNSTATES  240

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   329

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,    10,    13,    15,    19,    24,    29,
      34,    36,    42,    51,    56,    67,    77,    84,    92,    97,
      99,   101,   104,   108,   112,   116,   120,   124,   128,   132,
     136,   140,   144,   148,   152,   155,   158,   161,   167,   172,
     177,   182,   187,   192,   197,   202,   207,   212,   217,   222,
     227,   232,   237,   242,   247,   252,   257,   263,   265,   267,
     269,   272,   276,   279,   282,   285,   289,   293,   299,   300,
     303,   305,   308,   310,   312,   314,   318,   323,   326,   329,
     333,   337,   341,   346,   348,   350,   356,   358,   362,   365,
     368,   371,   374,   376,   378,   380,   382,   384,   385,   389,
     392,   394,   397,   398
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      76,     0,    -1,    90,    77,    -1,    77,    78,    90,    -1,
      78,    90,    -1,    90,    -1,    41,    82,    47,    -1,    31,
      41,    82,    47,    -1,    30,    41,    82,    47,    -1,    70,
      84,    82,    47,    -1,    79,    -1,    65,    84,    90,    78,
      90,    -1,    65,    84,    90,    78,    90,    66,    90,    78,
      -1,    68,    84,    90,    78,    -1,    64,    56,    78,    48,
      80,    48,    77,    55,    90,    78,    -1,    64,    56,    48,
      80,    48,    77,    55,    90,    78,    -1,    63,    90,    78,
      90,    68,    84,    -1,    62,    90,    79,    90,    61,    90,
      79,    -1,    67,    84,    90,    79,    -1,    69,    -1,    71,
      -1,    41,    82,    -1,    85,    41,    82,    -1,    85,    16,
      80,    -1,    85,    15,    80,    -1,    85,    14,    80,    -1,
      85,    13,    80,    -1,    85,    12,    80,    -1,    85,    11,
      80,    -1,    85,    10,    80,    -1,    85,     9,    80,    -1,
      85,     8,    80,    -1,    85,     7,    80,    -1,    85,     6,
      80,    -1,    85,    54,    -1,    85,    53,    -1,    78,    48,
      -1,    45,    90,    77,    90,    46,    -1,    80,    18,    90,
      80,    -1,    80,    17,    90,    80,    -1,    80,    21,    90,
      80,    -1,    80,    20,    90,    80,    -1,    80,    19,    90,
      80,    -1,    80,    23,    90,    80,    -1,    80,    22,    90,
      80,    -1,    80,    27,    90,    80,    -1,    80,    25,    90,
      80,    -1,    80,    26,    90,    80,    -1,    80,    24,    90,
      80,    -1,    80,    31,    90,    80,    -1,    80,    30,    90,
      80,    -1,    80,    34,    90,    80,    -1,    80,    33,    90,
      80,    -1,    80,    32,    90,    80,    -1,    80,    29,    90,
      80,    -1,    80,    28,    90,    80,    -1,    80,     5,    80,
      47,    80,    -1,    85,    -1,    81,    -1,    87,    -1,    41,
      83,    -1,    85,    41,    82,    -1,    38,    81,    -1,    36,
      81,    -1,    37,    81,    -1,    41,    50,    84,    -1,    85,
      50,    84,    -1,    59,    90,    88,    90,    60,    -1,    -1,
      82,    84,    -1,    84,    -1,    83,    84,    -1,    84,    -1,
      85,    -1,    87,    -1,    84,    50,    84,    -1,    49,    56,
      80,    55,    -1,    49,    41,    -1,    49,    42,    -1,    85,
      52,    41,    -1,    85,    52,    42,    -1,    85,    52,    72,
      -1,    85,    58,    80,    57,    -1,    42,    -1,    86,    -1,
      56,    80,    80,    80,    55,    -1,    35,    -1,    56,    80,
      55,    -1,    56,    55,    -1,    38,    85,    -1,    36,    85,
      -1,    37,    85,    -1,    44,    -1,    43,    -1,    39,    -1,
      40,    -1,    41,    -1,    -1,    88,    89,    90,    -1,    89,
      90,    -1,    84,    -1,    89,    84,    -1,    -1,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   111,   111,   115,   117,   118,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   135,   139,   140,   141,   142,
     143,   144,   146,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   165,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     194,   196,   198,   199,   200,   201,   202,   203,   207,   208,
     209,   213,   214,   218,   219,   220,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   245,   246,   250,   254,   255,   256,
     260,   261,   265,   266
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "END", "error", "$undefined", "TOKEN_EOL", "TOKEN_COMMA",
  "TOKEN_TERNARY", "TOKEN_SHIFT_RIGHT_EQUALS", "TOKEN_SHIFT_LEFT_EQUALS",
  "TOKEN_OR_EQUALS", "TOKEN_EXCL_OR_EQUALS", "TOKEN_AND_EQUALS",
  "TOKEN_MODULUS_EQUALS", "TOKEN_DIVIDE_EQUALS", "TOKEN_MULTIPLY_EQUALS",
  "TOKEN_MINUS_EQUALS", "TOKEN_PLUS_EQUALS", "TOKEN_ASSIGNMENT",
  "TOKEN_LOGICAL_OR", "TOKEN_LOGICAL_AND", "TOKEN_BITWISE_OR",
  "TOKEN_BITWISE_EXCL_OR", "TOKEN_BITWISE_AND", "TOKEN_INEQUALITY",
  "TOKEN_EQUALITY", "TOKEN_GREATER_THAN_OR_EQUAL", "TOKEN_GREATER_THAN",
  "TOKEN_LESS_THAN_OR_EQUAL", "TOKEN_LESS_THAN", "TOKEN_SHIFT_RIGHT",
  "TOKEN_SHIFT_LEFT", "TOKEN_MINUS", "TOKEN_PLUS", "TOKEN_MODULUS",
  "TOKEN_DIVIDE", "TOKEN_MULTIPLY", "TOKEN_LISTENER", "TOKEN_COMPLEMENT",
  "TOKEN_NOT", "TOKEN_NEG", "TOKEN_FLOAT", "TOKEN_INTEGER",
  "TOKEN_IDENTIFIER", "TOKEN_STRING", "TOKEN_NIL", "TOKEN_NULL",
  "TOKEN_LBRACKET", "TOKEN_RBRACKET", "TOKEN_COLON", "TOKEN_SEMICOLON",
  "TOKEN_DOLLAR", "TOKEN_DOUBLE_COLON", "TOKEN_NUMBER", "TOKEN_PERIOD",
  "TOKEN_DECREMENT", "TOKEN_INCREMENT", "TOKEN_RPAREN", "TOKEN_LPAREN",
  "TOKEN_RSQUARE", "TOKEN_LSQUARE", "TOKEN_MAKEARRAY", "TOKEN_ENDARRAY",
  "TOKEN_CATCH", "TOKEN_TRY", "TOKEN_DO", "TOKEN_FOR", "TOKEN_IF",
  "TOKEN_ELSE", "TOKEN_SWITCH", "TOKEN_WHILE", "TOKEN_BREAK", "TOKEN_CASE",
  "TOKEN_CONTINUE", "TOKEN_SIZE", "TOKEN_END", "TOKEN_RETURN", "$accept",
  "program", "statement_list", "statement", "compound_statement", "expr",
  "func_prim_expr", "event_parameter_list", "event_parameter_list_need",
  "prim_expr", "nonident_prim_expr", "number", "identifier_prim",
  "makearray_statement_list", "makearray_statement", "newline", YY_NULL
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    75,    76,    77,    77,    77,    78,    78,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    78,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    78,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    79,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      81,    81,    81,    81,    81,    81,    81,    81,    82,    82,
      82,    83,    83,    84,    84,    84,    85,    85,    85,    85,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    85,
      85,    85,    85,    85,    86,    86,    87,    88,    88,    88,
      89,    89,    90,    90
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     3,     2,     1,     3,     4,     4,     4,
       1,     5,     8,     4,    10,     9,     6,     7,     4,     1,
       1,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     2,     2,     5,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     5,     1,     1,     1,
       2,     3,     2,     2,     2,     3,     3,     5,     0,     2,
       1,     2,     1,     1,     1,     3,     4,     2,     2,     3,
       3,     3,     4,     1,     1,     5,     1,     3,     2,     2,
       2,     2,     1,     1,     1,     1,     1,     0,     3,     2,
       1,     2,     0,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
     102,   103,     0,   102,     1,     0,     0,    86,     0,     0,
       0,    94,    95,    68,    83,    93,    92,   102,     0,     0,
     102,   102,     0,     0,     0,     0,    19,     0,    20,     2,
     102,    10,     0,    84,     5,    68,    68,    90,    91,    89,
      96,    21,    70,    73,    74,   102,    77,    78,     0,     0,
       0,     0,    96,    88,   102,     0,    58,    57,    59,     0,
       0,     0,   102,   102,   102,    68,   102,    36,     4,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      68,     0,    35,    34,     0,     0,     0,     6,    69,     0,
     102,     0,     0,    63,    90,    64,    91,    62,    89,     0,
      60,    72,    97,     0,   102,   102,   102,   102,   102,   102,
     102,   102,   102,   102,   102,   102,   102,   102,   102,   102,
     102,   102,    87,     0,    68,     0,   102,   102,     0,     0,
       0,     0,     0,     0,     3,    33,    32,    31,    30,    29,
      28,    27,    26,    25,    24,    23,    22,    79,    80,    81,
       0,     8,     7,    75,     0,    76,    65,    71,   100,   102,
     102,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    61,    66,     0,     0,     0,    36,   102,    18,    13,
       9,    82,    37,   102,     0,   101,    99,     0,    39,    38,
      42,    41,    40,    44,    43,    48,    46,    47,    45,    55,
      54,    50,    49,    53,    52,    51,    85,   102,     0,   102,
       0,    11,    98,    67,    56,     0,    16,     0,   102,   102,
      17,   102,     0,     0,     0,   102,    12,    15,     0,    14
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,    29,    30,    31,    55,    56,    41,   100,    42,
      57,    33,    58,   159,   160,    34
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -90
static const yytype_int16 yypact[] =
{
      -2,   -90,     9,   482,   -90,   -23,   -16,   -90,  1157,  1157,
    1157,   -90,   -90,  1142,   -90,   -90,   -90,    -2,   -25,   284,
      -2,    -2,   -19,  1142,  1142,  1142,   -90,  1142,   -90,   904,
       5,   -90,   445,   -90,   -90,  1142,  1142,   -29,   -29,   -29,
     -90,  1010,   -11,   -29,   -90,   482,   -90,   -90,   960,   985,
     985,   985,  1032,   -90,    -2,   537,   -90,   -22,   -90,   -18,
     904,   778,     1,     1,     1,  1054,     5,   -90,   -90,   960,
     960,   960,   960,   960,   960,   960,   960,   960,   960,   960,
    1142,   -27,   -90,   -90,   960,  1076,  1098,   -90,   -11,  1142,
     482,   623,  1032,   -90,   -22,   -90,   -22,   -90,   -22,  1142,
    1142,   -11,  1142,   960,    -2,    -2,    -2,    -2,    -2,    -2,
      -2,    -2,    -2,    -2,    -2,    -2,    -2,    -2,    -2,    -2,
      -2,    -2,   -90,   580,  1142,  1142,    -2,     5,   960,    -8,
     904,   -18,   904,  1120,   -90,   959,   959,   959,   959,   959,
     959,   959,   959,   959,   959,   959,  1142,   -90,   -90,   -90,
     392,   -90,   -90,   -11,     4,   -90,   -11,   -11,   -11,    98,
      98,   765,   960,   960,   960,   960,   960,   960,   960,   960,
     960,   960,   960,   960,   960,   960,   960,   960,   960,   960,
     662,  1142,   -11,   -17,   -13,   701,   960,     5,   -90,    -5,
     -90,   -90,   -90,    98,     3,   -11,   -90,   960,  1196,  1212,
     414,  1226,  1239,  1250,  1250,    81,    81,    81,    81,    35,
      35,    27,    27,   -90,   -90,   -90,   -90,    -2,  1142,   482,
     733,   -14,   -90,   -90,   959,   -18,   -11,   820,   482,    -2,
     -90,    -2,   862,   904,   904,    -2,    -5,    -5,   904,    -5
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -90,   -90,   -43,   -26,   -46,   305,    29,   -24,   -90,   170,
      -3,   -90,   213,   -90,   -89,    54
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      32,     1,    90,    66,     1,    37,    38,    39,     1,     4,
      43,    85,    86,   126,   147,   148,    46,    47,    35,   124,
      43,    43,    43,    81,    43,    36,    32,    17,   125,    84,
      81,    48,    43,    43,   127,   129,    84,    61,    43,    89,
     186,   133,    32,    67,   217,   149,    94,    96,    98,    43,
     192,    89,   229,    67,     3,   218,   146,    32,    32,   119,
     120,   121,    43,   223,    66,   117,   118,   119,   120,   121,
     193,    45,     0,     0,    59,    60,     0,    43,    93,    95,
      97,     0,    43,    43,    68,   188,    43,    32,     0,    43,
       0,     0,     0,     0,     0,     0,    43,    43,     0,    43,
     181,     1,     0,     0,   187,     0,   189,     0,   102,   115,
     116,   117,   118,   119,   120,   121,   130,   131,   132,     0,
     134,    43,    43,     0,     0,     0,     0,    32,     0,    32,
      43,     0,     0,     7,     8,     9,    10,    11,    12,    40,
      14,    15,    16,    43,   154,     0,     0,    18,     0,     0,
       0,     0,     0,     0,    19,     0,    43,    43,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   227,     0,    43,   230,
     183,   184,     0,     0,     0,   232,     0,     0,     0,     0,
      43,     0,     0,    62,    63,    64,     0,    65,     0,     0,
       0,    66,     0,     0,     0,     0,    66,   236,   237,     0,
       0,    88,   239,   194,   196,    43,    32,     0,     0,     0,
       0,     0,   101,     0,    32,    32,    44,     0,     0,    32,
      32,    32,     0,     0,     0,    32,    44,    44,    44,     0,
      44,   221,     0,     0,     0,     0,     0,   222,    44,    44,
       0,     0,     0,     0,    44,    88,    88,     0,     0,   153,
       0,     0,   101,     0,     0,    44,     0,     0,     0,   156,
     157,   225,   158,     0,     0,     0,     0,     0,    44,     0,
       0,     0,     0,   233,     0,   234,     0,     0,     0,   238,
       0,     0,     0,    44,     0,   182,     0,     0,    44,    44,
       0,     0,    44,    88,     0,    44,     0,     0,     0,     0,
       0,     0,    44,    44,     0,    44,    88,     0,     0,     7,
      49,    50,    51,    11,    12,    52,    14,    15,    16,   158,
     195,     0,     0,    18,     0,     0,     0,    44,    44,    53,
      19,     0,     0,    54,     0,     0,    44,     0,     0,     0,
       0,    88,     0,    91,     0,     0,     0,     0,     0,    44,
     123,     0,     0,   195,     0,     0,     0,     0,     0,     0,
       0,     0,    44,    44,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,     0,     0,     0,   226,   150,
       0,     0,     0,     0,    44,     0,     0,   103,     0,     0,
       0,     0,     0,     0,     0,     0,    44,     0,   161,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,     0,   180,     0,
       0,    44,     0,   185,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   191,
       0,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,     0,     0,     0,     0,     0,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,     1,    80,     0,     0,     0,
       0,   220,     0,     0,     0,     0,     0,    81,    82,    83,
       0,     0,   224,    84,     0,     0,     0,     0,     0,     0,
       0,     0,     5,     6,     0,     0,     0,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,     0,     0,
       0,    18,     0,     0,     0,     0,     0,     0,    19,     0,
       0,     0,   103,     0,    20,    21,    22,    23,     0,    24,
      25,    26,    27,    28,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,     7,    49,    50,    51,    11,    12,    52,    14,
      15,    16,     0,     0,     0,   103,    18,     0,     0,     0,
       0,     0,   122,    19,     0,     0,    54,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,     7,    49,    50,    51,    11,
      12,    52,    14,    15,    16,     0,     0,     0,   103,    18,
       0,     0,     0,     0,     0,     0,    19,     0,     0,    54,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   103,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   155,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   103,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   216,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,     0,     0,   103,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   219,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,     0,     0,
     103,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   228,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
       0,     0,     0,     0,     0,     0,     0,     0,     5,     6,
       0,     0,   197,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,     0,     0,   128,    18,     0,     0,
       0,     0,     0,     0,    19,     0,     0,     0,     0,     0,
      20,    21,    22,    23,     0,    24,    25,    26,    27,    28,
       5,     6,     0,     0,     0,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,     0,     0,     0,    18,
       0,     0,     0,     0,     0,   231,    19,     0,     0,     0,
       0,     0,    20,    21,    22,    23,     0,    24,    25,    26,
      27,    28,     5,     6,     0,     0,     0,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,     0,     0,
       0,    18,     0,     0,     0,     0,     0,   235,    19,     0,
       0,     0,     0,     0,    20,    21,    22,    23,     0,    24,
      25,    26,    27,    28,     5,     6,     0,     0,     0,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
       0,     0,     0,    18,     0,     0,     0,     0,     0,     0,
      19,     0,     0,     0,   103,     0,    20,    21,    22,    23,
       0,    24,    25,    26,    27,    28,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,     0,     7,    49,    50,    51,    11,
      12,    52,    14,    15,    16,     0,     0,     0,     0,    18,
       0,     0,     0,     0,     0,     0,    19,     0,     0,    54,
       7,    49,    50,    51,    11,    12,    92,    14,    15,    16,
       0,     0,     0,     0,    18,     0,     0,     0,     0,     0,
       0,    19,     0,     0,    54,     7,     8,     9,    10,    11,
      12,    40,    14,    15,    16,     0,     0,    87,     0,    18,
       0,     0,     0,     0,     0,     0,    19,     7,     8,     9,
      10,    11,    12,    40,    14,    15,    16,     0,     0,     0,
       0,    18,    99,     0,     0,     0,     0,     0,    19,     7,
       8,     9,    10,    11,    12,    40,    14,    15,    16,     0,
       0,     0,     0,    18,    89,     0,     0,     0,     0,     0,
      19,     7,     8,     9,    10,    11,    12,    40,    14,    15,
      16,     0,     0,   151,     0,    18,     0,     0,     0,     0,
       0,     0,    19,     7,     8,     9,    10,    11,    12,    40,
      14,    15,    16,     0,     0,   152,     0,    18,     0,     0,
       0,     0,     0,     0,    19,     7,     8,     9,    10,    11,
      12,    40,    14,    15,    16,     0,     0,   190,     0,    18,
       0,     0,     0,     0,     0,     0,    19,     7,     8,     9,
      10,    11,    12,    40,    14,    15,    16,     0,     0,     0,
       0,    18,     7,     8,     9,    10,    11,    12,    19,    14,
      15,    16,     0,     0,     0,     0,    18,     0,     0,     0,
       0,     0,     0,    19,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-90)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       3,     3,    45,    29,     3,     8,     9,    10,     3,     0,
      13,    35,    36,    59,    41,    42,    41,    42,    41,    41,
      23,    24,    25,    52,    27,    41,    29,    45,    50,    58,
      52,    56,    35,    36,    60,    61,    58,    56,    41,    50,
      48,    65,    45,    48,    61,    72,    49,    50,    51,    52,
      46,    50,    66,    48,     0,    68,    80,    60,    61,    32,
      33,    34,    65,    60,    90,    30,    31,    32,    33,    34,
     159,    17,    -1,    -1,    20,    21,    -1,    80,    49,    50,
      51,    -1,    85,    86,    30,   131,    89,    90,    -1,    92,
      -1,    -1,    -1,    -1,    -1,    -1,    99,   100,    -1,   102,
     124,     3,    -1,    -1,   130,    -1,   132,    -1,    54,    28,
      29,    30,    31,    32,    33,    34,    62,    63,    64,    -1,
      66,   124,   125,    -1,    -1,    -1,    -1,   130,    -1,   132,
     133,    -1,    -1,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,   146,    90,    -1,    -1,    49,    -1,    -1,
      -1,    -1,    -1,    -1,    56,    -1,   159,   160,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   219,    -1,   181,   225,
     126,   127,    -1,    -1,    -1,   228,    -1,    -1,    -1,    -1,
     193,    -1,    -1,    23,    24,    25,    -1,    27,    -1,    -1,
      -1,   227,    -1,    -1,    -1,    -1,   232,   233,   234,    -1,
      -1,    41,   238,   159,   160,   218,   219,    -1,    -1,    -1,
      -1,    -1,    52,    -1,   227,   228,    13,    -1,    -1,   232,
     233,   234,    -1,    -1,    -1,   238,    23,    24,    25,    -1,
      27,   187,    -1,    -1,    -1,    -1,    -1,   193,    35,    36,
      -1,    -1,    -1,    -1,    41,    85,    86,    -1,    -1,    89,
      -1,    -1,    92,    -1,    -1,    52,    -1,    -1,    -1,    99,
     100,   217,   102,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,   229,    -1,   231,    -1,    -1,    -1,   235,
      -1,    -1,    -1,    80,    -1,   125,    -1,    -1,    85,    86,
      -1,    -1,    89,   133,    -1,    92,    -1,    -1,    -1,    -1,
      -1,    -1,    99,   100,    -1,   102,   146,    -1,    -1,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,   159,
     160,    -1,    -1,    49,    -1,    -1,    -1,   124,   125,    55,
      56,    -1,    -1,    59,    -1,    -1,   133,    -1,    -1,    -1,
      -1,   181,    -1,    48,    -1,    -1,    -1,    -1,    -1,   146,
      55,    -1,    -1,   193,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   159,   160,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    -1,    -1,    -1,   218,    84,
      -1,    -1,    -1,    -1,   181,    -1,    -1,     5,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   193,    -1,   103,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,   123,    -1,
      -1,   218,    -1,   128,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    57,
      -1,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,     3,    41,    -1,    -1,    -1,
      -1,   186,    -1,    -1,    -1,    -1,    -1,    52,    53,    54,
      -1,    -1,   197,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    -1,    -1,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    -1,    -1,
      -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,
      -1,    -1,     5,    -1,    62,    63,    64,    65,    -1,    67,
      68,    69,    70,    71,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    -1,    -1,    -1,     5,    49,    -1,    -1,    -1,
      -1,    -1,    55,    56,    -1,    -1,    59,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    -1,    -1,    -1,     5,    49,
      -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,    -1,    59,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     5,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     5,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    -1,    -1,     5,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    -1,    -1,
       5,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    -1,    47,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    -1,    -1,    48,    49,    -1,    -1,
      -1,    -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,
      62,    63,    64,    65,    -1,    67,    68,    69,    70,    71,
      30,    31,    -1,    -1,    -1,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    -1,    -1,    -1,    49,
      -1,    -1,    -1,    -1,    -1,    55,    56,    -1,    -1,    -1,
      -1,    -1,    62,    63,    64,    65,    -1,    67,    68,    69,
      70,    71,    30,    31,    -1,    -1,    -1,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    -1,    -1,
      -1,    49,    -1,    -1,    -1,    -1,    -1,    55,    56,    -1,
      -1,    -1,    -1,    -1,    62,    63,    64,    65,    -1,    67,
      68,    69,    70,    71,    30,    31,    -1,    -1,    -1,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,
      56,    -1,    -1,    -1,     5,    -1,    62,    63,    64,    65,
      -1,    67,    68,    69,    70,    71,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    -1,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,    -1,    59,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      -1,    -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,    -1,
      -1,    56,    -1,    -1,    59,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    -1,    -1,    47,    -1,    49,
      -1,    -1,    -1,    -1,    -1,    -1,    56,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    -1,    -1,    -1,
      -1,    49,    50,    -1,    -1,    -1,    -1,    -1,    56,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    -1,
      -1,    -1,    -1,    49,    50,    -1,    -1,    -1,    -1,    -1,
      56,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    -1,    -1,    47,    -1,    49,    -1,    -1,    -1,    -1,
      -1,    -1,    56,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    -1,    -1,    47,    -1,    49,    -1,    -1,
      -1,    -1,    -1,    -1,    56,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    -1,    -1,    47,    -1,    49,
      -1,    -1,    -1,    -1,    -1,    -1,    56,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    -1,    -1,    -1,
      -1,    49,    35,    36,    37,    38,    39,    40,    56,    42,
      43,    44,    -1,    -1,    -1,    -1,    49,    -1,    -1,    -1,
      -1,    -1,    -1,    56,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,    76,    90,     0,    30,    31,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    49,    56,
      62,    63,    64,    65,    67,    68,    69,    70,    71,    77,
      78,    79,    85,    86,    90,    41,    41,    85,    85,    85,
      41,    82,    84,    85,    87,    90,    41,    42,    56,    36,
      37,    38,    41,    55,    59,    80,    81,    85,    87,    90,
      90,    56,    84,    84,    84,    84,    78,    48,    90,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      41,    52,    53,    54,    58,    82,    82,    47,    84,    50,
      77,    80,    41,    81,    85,    81,    85,    81,    85,    50,
      83,    84,    90,     5,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    55,    80,    41,    50,    79,    78,    48,    78,
      90,    90,    90,    82,    90,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    82,    41,    42,    72,
      80,    47,    47,    84,    90,    55,    84,    84,    84,    88,
      89,    80,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      80,    82,    84,    90,    90,    80,    48,    78,    79,    78,
      47,    57,    46,    89,    90,    84,    90,    47,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    55,    61,    68,    48,
      80,    90,    90,    60,    80,    90,    84,    77,    48,    66,
      79,    55,    77,    90,    90,    55,    78,    78,    90,    78
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))

/* Error token number */
#define YYTERROR	1
#define YYERRCODE	256


/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */
#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
        break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULL, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULL;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULL, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
        break;
    }
}




/* The lookahead symbol.  */
int yychar;


#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval YY_INITIAL_VALUE(yyval_default);

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
/* Line 1792 of yacc.c  */
#line 111 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { parsedata.val = node1( sval_statement_list, (yyvsp[(2) - (2)].s.val) ); }
    break;

  case 3:
/* Line 1792 of yacc.c  */
#line 115 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = append_node( (yyvsp[(1) - (3)].s.val), (yyvsp[(2) - (3)].s.val) ); }
    break;

  case 4:
/* Line 1792 of yacc.c  */
#line 117 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = linked_list_end( (yyvsp[(1) - (2)].s.val) ); }
    break;

  case 5:
/* Line 1792 of yacc.c  */
#line 118 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node0( sval_none ); }
    break;

  case 6:
/* Line 1792 of yacc.c  */
#line 122 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_label, (yyvsp[(1) - (3)].s).val, (yyvsp[(2) - (3)].s.val), YYLLOC ); }
    break;

  case 7:
/* Line 1792 of yacc.c  */
#line 123 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_label, (yyvsp[(2) - (4)].s).val, (yyvsp[(3) - (4)].s.val), YYLLOC ); }
    break;

  case 8:
/* Line 1792 of yacc.c  */
#line 124 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_privatelabel, (yyvsp[(2) - (4)].s).val, (yyvsp[(3) - (4)].s.val), YYLLOC ); }
    break;

  case 9:
/* Line 1792 of yacc.c  */
#line 125 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_case, (yyvsp[(2) - (4)].s.val), (yyvsp[(3) - (4)].s.val), YYLLOC ); }
    break;

  case 11:
/* Line 1792 of yacc.c  */
#line 127 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_if, (yyvsp[(2) - (5)].s.val), (yyvsp[(4) - (5)].s.val), YYLLOC ); }
    break;

  case 12:
/* Line 1792 of yacc.c  */
#line 128 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node4( sval_ifelse, (yyvsp[(2) - (8)].s.val), (yyvsp[(4) - (8)].s.val), (yyvsp[(8) - (8)].s.val), YYLLOC ); }
    break;

  case 13:
/* Line 1792 of yacc.c  */
#line 129 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node4( sval_while, (yyvsp[(2) - (4)].s.val), (yyvsp[(4) - (4)].s.val), node0( sval_none ), YYLLOC ); }
    break;

  case 14:
/* Line 1792 of yacc.c  */
#line 131 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    {
		sval_u while_stmt = node4( sval_while, (yyvsp[(5) - (10)].s.val), (yyvsp[(10) - (10)].s.val), node1( sval_statement_list, (yyvsp[(7) - (10)].s.val) ), YYLLOC );
		(yyval.s.val) = node1( sval_statement_list, append_node( linked_list_end( (yyvsp[(3) - (10)].s.val) ), while_stmt ) );
	}
    break;

  case 15:
/* Line 1792 of yacc.c  */
#line 136 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    {
		(yyval.s.val) = node4( sval_while, (yyvsp[(4) - (9)].s.val), (yyvsp[(9) - (9)].s.val), node1( sval_statement_list, (yyvsp[(6) - (9)].s.val) ), YYLLOC );
	}
    break;

  case 16:
/* Line 1792 of yacc.c  */
#line 139 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_do, (yyvsp[(3) - (6)].s.val), (yyvsp[(6) - (6)].s.val), YYLLOC ); }
    break;

  case 17:
/* Line 1792 of yacc.c  */
#line 140 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_catch, (yyvsp[(3) - (7)].s.val), (yyvsp[(7) - (7)].s.val), YYLLOC ); }
    break;

  case 18:
/* Line 1792 of yacc.c  */
#line 141 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_switch, (yyvsp[(2) - (4)].s.val), (yyvsp[(4) - (4)].s.val), YYLLOC ); }
    break;

  case 19:
/* Line 1792 of yacc.c  */
#line 142 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node1( sval_break, YYLLOC ); }
    break;

  case 20:
/* Line 1792 of yacc.c  */
#line 143 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node1( sval_continue, YYLLOC ); }
    break;

  case 21:
/* Line 1792 of yacc.c  */
#line 144 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_cmd, (yyvsp[(1) - (2)].s).val, node1( sval_none, (yyvsp[(2) - (2)].s.val) ), node_pos( (yyvsp[(1) - (2)].s).sourcePos ) ); }
    break;

  case 22:
/* Line 1792 of yacc.c  */
#line 146 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node4( sval_cmd_method, (yyvsp[(1) - (3)].s.val), (yyvsp[(2) - (3)].s).val, node1( sval_none, (yyvsp[(3) - (3)].s.val) ), node_pos( (yyvsp[(2) - (3)].s).sourcePos ) ); }
    break;

  case 23:
/* Line 1792 of yacc.c  */
#line 148 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_assignment, (yyvsp[(1) - (3)].s.val), (yyvsp[(3) - (3)].s.val), YYLLOC ); }
    break;

  case 24:
/* Line 1792 of yacc.c  */
#line 149 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_assignment, (yyvsp[(1) - (3)].s.val), node4( sval_operation, node1b( OP_BIN_PLUS ), (yyvsp[(1) - (3)].s.val), (yyvsp[(3) - (3)].s.val), YYLLOC ), YYLLOC ); }
    break;

  case 25:
/* Line 1792 of yacc.c  */
#line 150 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_assignment, (yyvsp[(1) - (3)].s.val), node4( sval_operation, node1b( OP_BIN_MINUS ), (yyvsp[(1) - (3)].s.val), (yyvsp[(3) - (3)].s.val), YYLLOC ), YYLLOC ); }
    break;

  case 26:
/* Line 1792 of yacc.c  */
#line 151 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_assignment, (yyvsp[(1) - (3)].s.val), node4( sval_operation, node1b( OP_BIN_MULTIPLY ), (yyvsp[(1) - (3)].s.val), (yyvsp[(3) - (3)].s.val), YYLLOC ), YYLLOC ); }
    break;

  case 27:
/* Line 1792 of yacc.c  */
#line 152 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_assignment, (yyvsp[(1) - (3)].s.val), node4( sval_operation, node1b( OP_BIN_DIVIDE ), (yyvsp[(1) - (3)].s.val), (yyvsp[(3) - (3)].s.val), YYLLOC ), YYLLOC ); }
    break;

  case 28:
/* Line 1792 of yacc.c  */
#line 153 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_assignment, (yyvsp[(1) - (3)].s.val), node4( sval_operation, node1b( OP_BIN_PERCENTAGE ), (yyvsp[(1) - (3)].s.val), (yyvsp[(3) - (3)].s.val), YYLLOC ), YYLLOC ); }
    break;

  case 29:
/* Line 1792 of yacc.c  */
#line 154 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_assignment, (yyvsp[(1) - (3)].s.val), node4( sval_operation, node1b( OP_BIN_BITWISE_AND ), (yyvsp[(1) - (3)].s.val), (yyvsp[(3) - (3)].s.val), YYLLOC ), YYLLOC ); }
    break;

  case 30:
/* Line 1792 of yacc.c  */
#line 155 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_assignment, (yyvsp[(1) - (3)].s.val), node4( sval_operation, node1b( OP_BIN_BITWISE_EXCL_OR ), (yyvsp[(1) - (3)].s.val), (yyvsp[(3) - (3)].s.val), YYLLOC ), YYLLOC ); }
    break;

  case 31:
/* Line 1792 of yacc.c  */
#line 156 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_assignment, (yyvsp[(1) - (3)].s.val), node4( sval_operation, node1b( OP_BIN_BITWISE_OR ), (yyvsp[(1) - (3)].s.val), (yyvsp[(3) - (3)].s.val), YYLLOC ), YYLLOC ); }
    break;

  case 32:
/* Line 1792 of yacc.c  */
#line 157 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_assignment, (yyvsp[(1) - (3)].s.val), node4( sval_operation, node1b( OP_BIN_SHIFT_LEFT ), (yyvsp[(1) - (3)].s.val), (yyvsp[(3) - (3)].s.val), YYLLOC ), YYLLOC ); }
    break;

  case 33:
/* Line 1792 of yacc.c  */
#line 158 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_assignment, (yyvsp[(1) - (3)].s.val), node4( sval_operation, node1b( OP_BIN_SHIFT_RIGHT ), (yyvsp[(1) - (3)].s.val), (yyvsp[(3) - (3)].s.val), YYLLOC ), YYLLOC ); }
    break;

  case 34:
/* Line 1792 of yacc.c  */
#line 159 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_assignment, (yyvsp[(1) - (2)].s.val), node2( sval_func1, node1b( OP_UN_INC ), (yyvsp[(1) - (2)].s.val) ), YYLLOC ); }
    break;

  case 35:
/* Line 1792 of yacc.c  */
#line 160 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_assignment, (yyvsp[(1) - (2)].s.val), node2( sval_func1, node1b( OP_UN_DEC ), (yyvsp[(1) - (2)].s.val) ), YYLLOC ); }
    break;

  case 37:
/* Line 1792 of yacc.c  */
#line 165 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node1( sval_statement_list, (yyvsp[(3) - (5)].s.val) ); }
    break;

  case 38:
/* Line 1792 of yacc.c  */
#line 169 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_and, (yyvsp[(1) - (4)].s.val), (yyvsp[(4) - (4)].s.val), YYLLOC ); }
    break;

  case 39:
/* Line 1792 of yacc.c  */
#line 170 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_or, (yyvsp[(1) - (4)].s.val), (yyvsp[(4) - (4)].s.val), YYLLOC ); }
    break;

  case 40:
/* Line 1792 of yacc.c  */
#line 171 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_BITWISE_AND ), (yyvsp[(1) - (4)].s.val), (yyvsp[(4) - (4)].s.val), YYLLOC ); }
    break;

  case 41:
/* Line 1792 of yacc.c  */
#line 172 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_BITWISE_EXCL_OR ), (yyvsp[(1) - (4)].s.val), (yyvsp[(4) - (4)].s.val), YYLLOC ); }
    break;

  case 42:
/* Line 1792 of yacc.c  */
#line 173 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_BITWISE_OR ), (yyvsp[(1) - (4)].s.val), (yyvsp[(4) - (4)].s.val), YYLLOC ); }
    break;

  case 43:
/* Line 1792 of yacc.c  */
#line 174 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_EQUALITY ), (yyvsp[(1) - (4)].s.val), (yyvsp[(4) - (4)].s.val), YYLLOC ); }
    break;

  case 44:
/* Line 1792 of yacc.c  */
#line 175 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_INEQUALITY ), (yyvsp[(1) - (4)].s.val), (yyvsp[(4) - (4)].s.val), YYLLOC ); }
    break;

  case 45:
/* Line 1792 of yacc.c  */
#line 176 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_LESS_THAN ), (yyvsp[(1) - (4)].s.val), (yyvsp[(4) - (4)].s.val), YYLLOC ); }
    break;

  case 46:
/* Line 1792 of yacc.c  */
#line 177 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_GREATER_THAN ), (yyvsp[(1) - (4)].s.val), (yyvsp[(4) - (4)].s.val), YYLLOC ); }
    break;

  case 47:
/* Line 1792 of yacc.c  */
#line 178 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_LESS_THAN_OR_EQUAL ), (yyvsp[(1) - (4)].s.val), (yyvsp[(4) - (4)].s.val), YYLLOC ); }
    break;

  case 48:
/* Line 1792 of yacc.c  */
#line 179 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_GREATER_THAN_OR_EQUAL ), (yyvsp[(1) - (4)].s.val), (yyvsp[(4) - (4)].s.val), YYLLOC ); }
    break;

  case 49:
/* Line 1792 of yacc.c  */
#line 180 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_PLUS ), (yyvsp[(1) - (4)].s.val), (yyvsp[(4) - (4)].s.val), YYLLOC ); }
    break;

  case 50:
/* Line 1792 of yacc.c  */
#line 181 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_MINUS ), (yyvsp[(1) - (4)].s.val), (yyvsp[(4) - (4)].s.val), YYLLOC ); }
    break;

  case 51:
/* Line 1792 of yacc.c  */
#line 182 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_MULTIPLY ), (yyvsp[(1) - (4)].s.val), (yyvsp[(4) - (4)].s.val), YYLLOC ); }
    break;

  case 52:
/* Line 1792 of yacc.c  */
#line 183 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_DIVIDE ), (yyvsp[(1) - (4)].s.val), (yyvsp[(4) - (4)].s.val), YYLLOC ); }
    break;

  case 53:
/* Line 1792 of yacc.c  */
#line 184 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_PERCENTAGE ), (yyvsp[(1) - (4)].s.val), (yyvsp[(4) - (4)].s.val), YYLLOC ); }
    break;

  case 54:
/* Line 1792 of yacc.c  */
#line 185 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_SHIFT_LEFT ), (yyvsp[(1) - (4)].s.val), (yyvsp[(4) - (4)].s.val), YYLLOC ); }
    break;

  case 55:
/* Line 1792 of yacc.c  */
#line 186 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_SHIFT_RIGHT ), (yyvsp[(1) - (4)].s.val), (yyvsp[(4) - (4)].s.val), YYLLOC ); }
    break;

  case 56:
/* Line 1792 of yacc.c  */
#line 187 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node4( sval_ifelse, (yyvsp[(1) - (5)].s.val), (yyvsp[(3) - (5)].s.val), (yyvsp[(5) - (5)].s.val), YYLLOC ); }
    break;

  case 59:
/* Line 1792 of yacc.c  */
#line 190 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node1( sval_store_string, (yyvsp[(1) - (1)].s).val ); }
    break;

  case 60:
/* Line 1792 of yacc.c  */
#line 194 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_cmd_default_ret, (yyvsp[(1) - (2)].s).val, node1( sval_none, (yyvsp[(2) - (2)].s.val) ), node_pos( (yyvsp[(1) - (2)].s).sourcePos ) ); }
    break;

  case 61:
/* Line 1792 of yacc.c  */
#line 196 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node4( sval_cmd_method_ret, (yyvsp[(1) - (3)].s.val), (yyvsp[(2) - (3)].s).val, node1( sval_none, (yyvsp[(3) - (3)].s.val) ), node_pos( (yyvsp[(2) - (3)].s).sourcePos ) ); }
    break;

  case 62:
/* Line 1792 of yacc.c  */
#line 198 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_func1, node1b( OP_UN_MINUS ), (yyvsp[(2) - (2)].s.val), YYLLOC ); }
    break;

  case 63:
/* Line 1792 of yacc.c  */
#line 199 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_func1, node1b( OP_UN_COMPLEMENT ), (yyvsp[(2) - (2)].s.val), YYLLOC ); }
    break;

  case 64:
/* Line 1792 of yacc.c  */
#line 200 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node2( sval_not, (yyvsp[(2) - (2)].s.val), YYLLOC ); }
    break;

  case 65:
/* Line 1792 of yacc.c  */
#line 201 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_constarray, node2( sval_store_string, (yyvsp[(1) - (3)].s).val, node_pos( (yyvsp[(1) - (3)].s).sourcePos ) ), (yyvsp[(3) - (3)].s.val), YYLLOC ); }
    break;

  case 66:
/* Line 1792 of yacc.c  */
#line 202 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_constarray, (yyvsp[(1) - (3)].s.val), (yyvsp[(3) - (3)].s.val), YYLLOC ); }
    break;

  case 67:
/* Line 1792 of yacc.c  */
#line 203 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node1( sval_makearray, (yyvsp[(3) - (5)].s.val) ); }
    break;

  case 68:
/* Line 1792 of yacc.c  */
#line 207 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node0( sval_none ); }
    break;

  case 69:
/* Line 1792 of yacc.c  */
#line 208 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = append_node( (yyvsp[(1) - (2)].s.val), (yyvsp[(2) - (2)].s.val) ); }
    break;

  case 70:
/* Line 1792 of yacc.c  */
#line 209 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = linked_list_end( (yyvsp[(1) - (1)].s.val) ); }
    break;

  case 71:
/* Line 1792 of yacc.c  */
#line 213 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = append_node( (yyvsp[(1) - (2)].s.val), (yyvsp[(2) - (2)].s.val) ); }
    break;

  case 72:
/* Line 1792 of yacc.c  */
#line 214 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = linked_list_end( (yyvsp[(1) - (1)].s.val) ); }
    break;

  case 74:
/* Line 1792 of yacc.c  */
#line 219 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node1( sval_store_string, (yyvsp[(1) - (1)].s).val ); }
    break;

  case 75:
/* Line 1792 of yacc.c  */
#line 220 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_constarray, (yyvsp[(1) - (3)].s.val), (yyvsp[(3) - (3)].s.val), YYLLOC ); }
    break;

  case 76:
/* Line 1792 of yacc.c  */
#line 224 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_func1, node1b( OP_UN_TARGETNAME ), (yyvsp[(3) - (4)].s.val), YYLLOC ); }
    break;

  case 77:
/* Line 1792 of yacc.c  */
#line 225 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_func1, node1b( OP_UN_TARGETNAME ), node1( sval_store_string, (yyvsp[(2) - (2)].s).val ), YYLLOC ); }
    break;

  case 78:
/* Line 1792 of yacc.c  */
#line 226 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_func1, node1b( OP_UN_TARGETNAME ), node1( sval_store_string, (yyvsp[(2) - (2)].s).val ), YYLLOC ); }
    break;

  case 79:
/* Line 1792 of yacc.c  */
#line 227 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_field, (yyvsp[(1) - (3)].s.val), (yyvsp[(3) - (3)].s).val, node_pos( (yyvsp[(3) - (3)].s).sourcePos ) ); }
    break;

  case 80:
/* Line 1792 of yacc.c  */
#line 228 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_field, (yyvsp[(1) - (3)].s.val), (yyvsp[(3) - (3)].s).val, node_pos( (yyvsp[(3) - (3)].s).sourcePos ) ); }
    break;

  case 81:
/* Line 1792 of yacc.c  */
#line 229 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_func1, node1b( OP_UN_SIZE ), (yyvsp[(1) - (3)].s.val), YYLLOC ); }
    break;

  case 82:
/* Line 1792 of yacc.c  */
#line 230 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_array, (yyvsp[(1) - (4)].s.val), (yyvsp[(3) - (4)].s.val), (yyvsp[(1) - (4)].s.val) ); }
    break;

  case 83:
/* Line 1792 of yacc.c  */
#line 231 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node1( sval_store_string, (yyvsp[(1) - (1)].s).val ); }
    break;

  case 85:
/* Line 1792 of yacc.c  */
#line 233 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node4( sval_calc_vector, (yyvsp[(2) - (5)].s.val), (yyvsp[(3) - (5)].s.val), (yyvsp[(4) - (5)].s.val), YYLLOC ); }
    break;

  case 86:
/* Line 1792 of yacc.c  */
#line 234 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node2( sval_store_method, (yyvsp[(1) - (1)].s).val, YYLLOC ); }
    break;

  case 87:
/* Line 1792 of yacc.c  */
#line 235 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = (yyvsp[(2) - (3)].s.val); }
    break;

  case 88:
/* Line 1792 of yacc.c  */
#line 236 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node0( sval_none ); }
    break;

  case 89:
/* Line 1792 of yacc.c  */
#line 237 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_func1, node1b( OP_UN_MINUS ), (yyvsp[(2) - (2)].s.val), YYLLOC ); }
    break;

  case 90:
/* Line 1792 of yacc.c  */
#line 238 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node3( sval_func1, node1b( OP_UN_COMPLEMENT ), (yyvsp[(2) - (2)].s.val), YYLLOC ); }
    break;

  case 91:
/* Line 1792 of yacc.c  */
#line 239 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node2( sval_not, (yyvsp[(2) - (2)].s.val), YYLLOC ); }
    break;

  case 92:
/* Line 1792 of yacc.c  */
#line 240 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node1( sval_store_null, YYLLOC ); }
    break;

  case 93:
/* Line 1792 of yacc.c  */
#line 241 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node1( sval_store_nil, YYLLOC ); }
    break;

  case 94:
/* Line 1792 of yacc.c  */
#line 245 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node1( sval_store_float, (yyvsp[(1) - (1)].s).val ); }
    break;

  case 95:
/* Line 1792 of yacc.c  */
#line 246 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node1( sval_store_integer, (yyvsp[(1) - (1)].s).val ); }
    break;

  case 97:
/* Line 1792 of yacc.c  */
#line 254 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = node0( sval_none ); }
    break;

  case 98:
/* Line 1792 of yacc.c  */
#line 255 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = append_node( (yyvsp[(1) - (3)].s.val), node1( sval_makearray, (yyvsp[(2) - (3)].s.val) ) ); }
    break;

  case 99:
/* Line 1792 of yacc.c  */
#line 256 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = linked_list_end( node1( sval_makearray, (yyvsp[(1) - (2)].s.val) ) ); }
    break;

  case 100:
/* Line 1792 of yacc.c  */
#line 260 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = linked_list_end( (yyvsp[(1) - (1)].s.val) ); }
    break;

  case 101:
/* Line 1792 of yacc.c  */
#line 261 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    { (yyval.s.val) = append_node( (yyvsp[(1) - (2)].s.val), (yyvsp[(2) - (2)].s.val) ); }
    break;

  case 102:
/* Line 1792 of yacc.c  */
#line 265 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"
    {}
    break;


/* Line 1792 of yacc.c  */
#line 2445 "../../../code/globalcpp/parser/yyParser.cpp"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


/* Line 2055 of yacc.c  */
#line 269 "..\\..\\..\\code\\globalcpp\\parser\\yyParser.yy"

