/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "E:/Src/openmohaa/code/parser/bison_source.txt"

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

#include "scriptcompiler.h"
#include "./yyParser.hpp"
#include "./yyLexer.h"

int yyerror( const char *msg );

extern int prev_yylex;

extern yyparsedata parsedata;

#define YYLLOC node_pos( parsedata.pos - yyleng )


#line 111 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "yyParser.hpp"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* END  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_TOKEN_EOL = 3,                  /* TOKEN_EOL  */
  YYSYMBOL_TOKEN_COMMA = 4,                /* TOKEN_COMMA  */
  YYSYMBOL_TOKEN_ASSIGNMENT = 5,           /* TOKEN_ASSIGNMENT  */
  YYSYMBOL_TOKEN_PLUS_EQUALS = 6,          /* TOKEN_PLUS_EQUALS  */
  YYSYMBOL_TOKEN_MINUS_EQUALS = 7,         /* TOKEN_MINUS_EQUALS  */
  YYSYMBOL_TOKEN_MULTIPLY_EQUALS = 8,      /* TOKEN_MULTIPLY_EQUALS  */
  YYSYMBOL_TOKEN_DIVIDE_EQUALS = 9,        /* TOKEN_DIVIDE_EQUALS  */
  YYSYMBOL_TOKEN_MODULUS_EQUALS = 10,      /* TOKEN_MODULUS_EQUALS  */
  YYSYMBOL_TOKEN_AND_EQUALS = 11,          /* TOKEN_AND_EQUALS  */
  YYSYMBOL_TOKEN_EXCL_OR_EQUALS = 12,      /* TOKEN_EXCL_OR_EQUALS  */
  YYSYMBOL_TOKEN_OR_EQUALS = 13,           /* TOKEN_OR_EQUALS  */
  YYSYMBOL_TOKEN_SHIFT_LEFT_EQUALS = 14,   /* TOKEN_SHIFT_LEFT_EQUALS  */
  YYSYMBOL_TOKEN_SHIFT_RIGHT_EQUALS = 15,  /* TOKEN_SHIFT_RIGHT_EQUALS  */
  YYSYMBOL_TOKEN_TERNARY = 16,             /* TOKEN_TERNARY  */
  YYSYMBOL_TOKEN_LOGICAL_OR = 17,          /* TOKEN_LOGICAL_OR  */
  YYSYMBOL_TOKEN_LOGICAL_AND = 18,         /* TOKEN_LOGICAL_AND  */
  YYSYMBOL_TOKEN_BITWISE_OR = 19,          /* TOKEN_BITWISE_OR  */
  YYSYMBOL_TOKEN_BITWISE_EXCL_OR = 20,     /* TOKEN_BITWISE_EXCL_OR  */
  YYSYMBOL_TOKEN_BITWISE_AND = 21,         /* TOKEN_BITWISE_AND  */
  YYSYMBOL_TOKEN_EQUALITY = 22,            /* TOKEN_EQUALITY  */
  YYSYMBOL_TOKEN_INEQUALITY = 23,          /* TOKEN_INEQUALITY  */
  YYSYMBOL_TOKEN_LESS_THAN = 24,           /* TOKEN_LESS_THAN  */
  YYSYMBOL_TOKEN_LESS_THAN_OR_EQUAL = 25,  /* TOKEN_LESS_THAN_OR_EQUAL  */
  YYSYMBOL_TOKEN_GREATER_THAN = 26,        /* TOKEN_GREATER_THAN  */
  YYSYMBOL_TOKEN_GREATER_THAN_OR_EQUAL = 27, /* TOKEN_GREATER_THAN_OR_EQUAL  */
  YYSYMBOL_TOKEN_SHIFT_LEFT = 28,          /* TOKEN_SHIFT_LEFT  */
  YYSYMBOL_TOKEN_SHIFT_RIGHT = 29,         /* TOKEN_SHIFT_RIGHT  */
  YYSYMBOL_TOKEN_PLUS = 30,                /* TOKEN_PLUS  */
  YYSYMBOL_TOKEN_MINUS = 31,               /* TOKEN_MINUS  */
  YYSYMBOL_TOKEN_MULTIPLY = 32,            /* TOKEN_MULTIPLY  */
  YYSYMBOL_TOKEN_DIVIDE = 33,              /* TOKEN_DIVIDE  */
  YYSYMBOL_TOKEN_MODULUS = 34,             /* TOKEN_MODULUS  */
  YYSYMBOL_TOKEN_LISTENER = 35,            /* TOKEN_LISTENER  */
  YYSYMBOL_TOKEN_NEG = 36,                 /* TOKEN_NEG  */
  YYSYMBOL_TOKEN_NOT = 37,                 /* TOKEN_NOT  */
  YYSYMBOL_TOKEN_COMPLEMENT = 38,          /* TOKEN_COMPLEMENT  */
  YYSYMBOL_TOKEN_FLOAT = 39,               /* TOKEN_FLOAT  */
  YYSYMBOL_TOKEN_INTEGER = 40,             /* TOKEN_INTEGER  */
  YYSYMBOL_TOKEN_IDENTIFIER = 41,          /* TOKEN_IDENTIFIER  */
  YYSYMBOL_TOKEN_STRING = 42,              /* TOKEN_STRING  */
  YYSYMBOL_TOKEN_NIL = 43,                 /* TOKEN_NIL  */
  YYSYMBOL_TOKEN_NULL = 44,                /* TOKEN_NULL  */
  YYSYMBOL_TOKEN_LBRACKET = 45,            /* TOKEN_LBRACKET  */
  YYSYMBOL_TOKEN_RBRACKET = 46,            /* TOKEN_RBRACKET  */
  YYSYMBOL_TOKEN_COLON = 47,               /* TOKEN_COLON  */
  YYSYMBOL_TOKEN_SEMICOLON = 48,           /* TOKEN_SEMICOLON  */
  YYSYMBOL_TOKEN_DOLLAR = 49,              /* TOKEN_DOLLAR  */
  YYSYMBOL_TOKEN_DOUBLE_COLON = 50,        /* TOKEN_DOUBLE_COLON  */
  YYSYMBOL_TOKEN_NUMBER = 51,              /* TOKEN_NUMBER  */
  YYSYMBOL_TOKEN_LSQUARE = 52,             /* TOKEN_LSQUARE  */
  YYSYMBOL_TOKEN_RSQUARE = 53,             /* TOKEN_RSQUARE  */
  YYSYMBOL_TOKEN_LPAREN = 54,              /* TOKEN_LPAREN  */
  YYSYMBOL_TOKEN_RPAREN = 55,              /* TOKEN_RPAREN  */
  YYSYMBOL_TOKEN_INCREMENT = 56,           /* TOKEN_INCREMENT  */
  YYSYMBOL_TOKEN_DECREMENT = 57,           /* TOKEN_DECREMENT  */
  YYSYMBOL_TOKEN_PERIOD = 58,              /* TOKEN_PERIOD  */
  YYSYMBOL_TOKEN_MAKEARRAY = 59,           /* TOKEN_MAKEARRAY  */
  YYSYMBOL_TOKEN_ENDARRAY = 60,            /* TOKEN_ENDARRAY  */
  YYSYMBOL_TOKEN_CATCH = 61,               /* TOKEN_CATCH  */
  YYSYMBOL_TOKEN_TRY = 62,                 /* TOKEN_TRY  */
  YYSYMBOL_TOKEN_DO = 63,                  /* TOKEN_DO  */
  YYSYMBOL_TOKEN_FOR = 64,                 /* TOKEN_FOR  */
  YYSYMBOL_TOKEN_IF = 65,                  /* TOKEN_IF  */
  YYSYMBOL_TOKEN_ELSE = 66,                /* TOKEN_ELSE  */
  YYSYMBOL_TOKEN_SWITCH = 67,              /* TOKEN_SWITCH  */
  YYSYMBOL_TOKEN_WHILE = 68,               /* TOKEN_WHILE  */
  YYSYMBOL_TOKEN_BREAK = 69,               /* TOKEN_BREAK  */
  YYSYMBOL_TOKEN_CASE = 70,                /* TOKEN_CASE  */
  YYSYMBOL_TOKEN_CONTINUE = 71,            /* TOKEN_CONTINUE  */
  YYSYMBOL_TOKEN_SIZE = 72,                /* TOKEN_SIZE  */
  YYSYMBOL_TOKEN_END = 73,                 /* TOKEN_END  */
  YYSYMBOL_TOKEN_RETURN = 74,              /* TOKEN_RETURN  */
  YYSYMBOL_YYACCEPT = 75,                  /* $accept  */
  YYSYMBOL_program = 76,                   /* program  */
  YYSYMBOL_statement_list = 77,            /* statement_list  */
  YYSYMBOL_statement = 78,                 /* statement  */
  YYSYMBOL_compound_statement = 79,        /* compound_statement  */
  YYSYMBOL_expr = 80,                      /* expr  */
  YYSYMBOL_func_prim_expr = 81,            /* func_prim_expr  */
  YYSYMBOL_event_parameter_list = 82,      /* event_parameter_list  */
  YYSYMBOL_event_parameter_list_need = 83, /* event_parameter_list_need  */
  YYSYMBOL_prim_expr = 84,                 /* prim_expr  */
  YYSYMBOL_nonident_prim_expr = 85,        /* nonident_prim_expr  */
  YYSYMBOL_number = 86,                    /* number  */
  YYSYMBOL_identifier_prim = 87,           /* identifier_prim  */
  YYSYMBOL_makearray_statement_list = 88,  /* makearray_statement_list  */
  YYSYMBOL_makearray_statement = 89,       /* makearray_statement  */
  YYSYMBOL_newline = 90                    /* newline  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

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


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
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
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1169

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  75
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  16
/* YYNRULES -- Number of rules.  */
#define YYNRULES  103
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  240

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   329


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
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
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   108,   108,   112,   114,   115,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   132,   136,   137,   138,   139,
     140,   141,   143,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   162,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     191,   193,   195,   196,   197,   198,   199,   200,   204,   205,
     206,   210,   211,   215,   216,   217,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   242,   243,   247,   251,   252,   253,
     257,   258,   262,   263
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "END", "error", "\"invalid token\"", "TOKEN_EOL", "TOKEN_COMMA",
  "TOKEN_ASSIGNMENT", "TOKEN_PLUS_EQUALS", "TOKEN_MINUS_EQUALS",
  "TOKEN_MULTIPLY_EQUALS", "TOKEN_DIVIDE_EQUALS", "TOKEN_MODULUS_EQUALS",
  "TOKEN_AND_EQUALS", "TOKEN_EXCL_OR_EQUALS", "TOKEN_OR_EQUALS",
  "TOKEN_SHIFT_LEFT_EQUALS", "TOKEN_SHIFT_RIGHT_EQUALS", "TOKEN_TERNARY",
  "TOKEN_LOGICAL_OR", "TOKEN_LOGICAL_AND", "TOKEN_BITWISE_OR",
  "TOKEN_BITWISE_EXCL_OR", "TOKEN_BITWISE_AND", "TOKEN_EQUALITY",
  "TOKEN_INEQUALITY", "TOKEN_LESS_THAN", "TOKEN_LESS_THAN_OR_EQUAL",
  "TOKEN_GREATER_THAN", "TOKEN_GREATER_THAN_OR_EQUAL", "TOKEN_SHIFT_LEFT",
  "TOKEN_SHIFT_RIGHT", "TOKEN_PLUS", "TOKEN_MINUS", "TOKEN_MULTIPLY",
  "TOKEN_DIVIDE", "TOKEN_MODULUS", "TOKEN_LISTENER", "TOKEN_NEG",
  "TOKEN_NOT", "TOKEN_COMPLEMENT", "TOKEN_FLOAT", "TOKEN_INTEGER",
  "TOKEN_IDENTIFIER", "TOKEN_STRING", "TOKEN_NIL", "TOKEN_NULL",
  "TOKEN_LBRACKET", "TOKEN_RBRACKET", "TOKEN_COLON", "TOKEN_SEMICOLON",
  "TOKEN_DOLLAR", "TOKEN_DOUBLE_COLON", "TOKEN_NUMBER", "TOKEN_LSQUARE",
  "TOKEN_RSQUARE", "TOKEN_LPAREN", "TOKEN_RPAREN", "TOKEN_INCREMENT",
  "TOKEN_DECREMENT", "TOKEN_PERIOD", "TOKEN_MAKEARRAY", "TOKEN_ENDARRAY",
  "TOKEN_CATCH", "TOKEN_TRY", "TOKEN_DO", "TOKEN_FOR", "TOKEN_IF",
  "TOKEN_ELSE", "TOKEN_SWITCH", "TOKEN_WHILE", "TOKEN_BREAK", "TOKEN_CASE",
  "TOKEN_CONTINUE", "TOKEN_SIZE", "TOKEN_END", "TOKEN_RETURN", "$accept",
  "program", "statement_list", "statement", "compound_statement", "expr",
  "func_prim_expr", "event_parameter_list", "event_parameter_list_need",
  "prim_expr", "nonident_prim_expr", "number", "identifier_prim",
  "makearray_statement_list", "makearray_statement", "newline", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-97)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      -2,   -97,     9,   482,   -97,   -23,   -18,   -97,  1081,  1081,
    1081,   -97,   -97,  1061,   -97,   -97,   -97,    -2,   -25,   911,
      -2,    -2,   -29,  1061,  1061,  1061,   -97,  1061,   -97,   691,
       5,   -97,   313,   -97,   -97,  1061,  1061,   -21,   -21,   -21,
     -97,   360,   -11,   -21,   -97,   482,   -97,   -97,   457,   936,
     936,   936,   961,   -97,    -2,   538,   -97,   -22,   -97,    -5,
     691,   381,     1,     1,     1,   981,     5,   -97,   -97,   457,
     457,   457,   457,   457,   457,   457,   457,   457,   457,   457,
    1061,   457,   -97,   -97,   -27,  1001,  1021,   -97,   -11,  1061,
     482,   747,   961,   -97,   -22,   -97,   -22,   -97,   -22,  1061,
    1061,   -11,  1061,   457,    -2,    -2,    -2,    -2,    -2,    -2,
      -2,    -2,    -2,    -2,    -2,    -2,    -2,    -2,    -2,    -2,
      -2,    -2,   -97,   582,  1061,  1061,    -2,     5,   457,    -4,
     691,    -5,   691,  1041,   -97,  1120,  1120,  1120,  1120,  1120,
    1120,  1120,  1120,  1120,  1120,  1120,  1061,   806,   -97,   -97,
     -97,   -97,   -97,   -11,   -19,   -97,   -11,   -11,   -11,    98,
      98,   910,   457,   457,   457,   457,   457,   457,   457,   457,
     457,   457,   457,   457,   457,   457,   457,   457,   457,   457,
     766,  1061,   -11,    -9,   -13,   844,   457,     5,   -97,     2,
     -97,   -97,   -97,    98,   -17,   -11,   -97,   457,   785,   822,
    1135,   432,   857,   888,   888,    81,    81,    81,    81,    35,
      35,    27,    27,   -97,   -97,   -97,   -97,    -2,  1061,   482,
     877,     4,   -97,   -97,  1120,    -5,   -11,   607,   482,    -2,
     -97,    -2,   649,   691,   691,    -2,     2,     2,   691,     2
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
     102,   103,     0,   102,     1,     0,     0,    86,     0,     0,
       0,    94,    95,    68,    83,    93,    92,   102,     0,     0,
     102,   102,     0,     0,     0,     0,    19,     0,    20,     2,
     102,    10,     0,    84,     5,    68,    68,    89,    91,    90,
      96,    21,    70,    73,    74,   102,    77,    78,     0,     0,
       0,     0,    96,    88,   102,     0,    58,    57,    59,     0,
       0,     0,   102,   102,   102,    68,   102,    36,     4,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      68,     0,    34,    35,     0,     0,     0,     6,    69,     0,
     102,     0,     0,    62,    89,    64,    91,    63,    90,     0,
      60,    72,    97,     0,   102,   102,   102,   102,   102,   102,
     102,   102,   102,   102,   102,   102,   102,   102,   102,   102,
     102,   102,    87,     0,    68,     0,   102,   102,     0,     0,
       0,     0,     0,     0,     3,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    22,     0,    79,    80,
      81,     7,     8,    75,     0,    76,    65,    71,   100,   102,
     102,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    61,    66,     0,     0,     0,    36,   102,    18,    13,
       9,    82,    37,   102,     0,   101,    99,     0,    39,    38,
      42,    41,    40,    43,    44,    45,    47,    46,    48,    54,
      55,    49,    50,    51,    52,    53,    85,   102,     0,   102,
       0,    11,    98,    67,    56,     0,    16,     0,   102,   102,
      17,   102,     0,     0,     0,   102,    12,    15,     0,    14
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -97,   -97,   -43,   -26,   -46,   305,    29,   -24,   -97,   170,
      -3,   -97,   213,   -97,   -96,    54
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     2,    29,    30,    31,    55,    56,    41,   100,    42,
      57,    33,    58,   159,   160,    34
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      32,     1,    90,    66,     1,    37,    38,    39,     1,     4,
      43,    85,    86,   126,   148,   149,    46,    47,    35,   124,
      43,    43,    43,    36,    43,    61,    32,   192,   125,    48,
      81,    81,    43,    43,   127,   129,    84,    84,    43,    89,
      17,   133,    32,   223,   186,   150,    94,    96,    98,    43,
      67,    89,   217,    67,     3,   218,   146,    32,    32,   119,
     120,   121,    43,   193,    66,   117,   118,   119,   120,   121,
     229,    45,     0,     0,    59,    60,     0,    43,    93,    95,
      97,     0,    43,    43,    68,   188,    43,    32,     0,    43,
       0,     0,     0,     0,     0,     0,    43,    43,     0,    43,
     181,     1,     0,     0,   187,     0,   189,     0,   102,   115,
     116,   117,   118,   119,   120,   121,   130,   131,   132,     0,
     134,    43,    43,     0,     0,     0,     0,    32,     0,    32,
      43,     0,     0,     7,     8,     9,    10,    11,    12,    40,
      14,    15,    16,    43,   154,     0,     0,    18,     0,     0,
       0,     0,    19,     0,     0,     0,    43,    43,   162,   163,
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
       0,     0,    44,    44,     0,    44,    88,     0,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,   158,
     195,     0,     0,     0,     0,     0,     0,    44,    44,     0,
       0,     0,     0,     0,     0,     0,    44,     0,     0,     0,
       0,    88,     0,    91,    80,     0,     0,     0,     0,    44,
     123,     0,     0,   195,     0,    81,     0,     0,     0,    82,
      83,    84,    44,    44,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,     0,   147,     0,   226,     0,
       0,     0,     0,     0,    44,     7,     8,     9,    10,    11,
      12,    40,    14,    15,    16,     0,    44,    87,   161,    18,
       0,     5,     6,     0,    19,     0,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,     0,   180,   128,
      18,    44,     0,   185,     0,    19,     0,     0,     0,     0,
       0,     0,     0,    20,    21,    22,    23,     0,    24,    25,
      26,    27,    28,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,     1,     0,     0,     0,     0,
       0,   220,     7,    49,    50,    51,    11,    12,    52,    14,
      15,    16,   224,     0,     0,     0,    18,     0,     0,     0,
       0,    19,     5,     6,     0,     0,    54,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,     0,     0,
       0,    18,     0,     0,     0,     0,    19,     0,     0,     0,
       0,     0,     0,     0,    20,    21,    22,    23,     0,    24,
      25,    26,    27,    28,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,     7,    49,    50,    51,    11,    12,    52,
      14,    15,    16,     0,     0,     0,     0,    18,     0,     0,
       0,     0,    19,   122,     0,     0,     0,    54,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,     7,    49,    50,
      51,    11,    12,    52,    14,    15,    16,     0,     0,     0,
       0,    18,     0,     0,     0,     0,    19,     5,     6,     0,
       0,    54,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,     0,     0,     0,    18,     0,     0,     0,
       0,    19,   231,     0,     0,     0,     0,     0,     0,    20,
      21,    22,    23,     0,    24,    25,    26,    27,    28,     5,
       6,     0,     0,     0,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,     0,     0,     0,    18,     0,
       0,     0,     0,    19,   235,     0,     0,     0,     0,     0,
       0,    20,    21,    22,    23,     0,    24,    25,    26,    27,
      28,     5,     6,     0,     0,     0,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,     0,     0,     0,
      18,     0,     0,     0,     0,    19,     0,     0,     0,     0,
       0,     0,     0,    20,    21,    22,    23,     0,    24,    25,
      26,    27,    28,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,     0,   155,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
       0,   216,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,     0,     0,   191,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   219,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,     0,     0,   228,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,     0,     7,    49,    50,    51,
      11,    12,    52,    14,    15,    16,     0,   197,     0,     0,
      18,     0,     0,     0,     0,    19,    53,     0,     0,     0,
      54,     7,    49,    50,    51,    11,    12,    92,    14,    15,
      16,     0,     0,     0,     0,    18,     0,     0,     0,     0,
      19,     0,     0,     0,     0,    54,     7,     8,     9,    10,
      11,    12,    40,    14,    15,    16,     0,     0,     0,     0,
      18,    99,     0,     0,     0,    19,     7,     8,     9,    10,
      11,    12,    40,    14,    15,    16,     0,     0,     0,     0,
      18,    89,     0,     0,     0,    19,     7,     8,     9,    10,
      11,    12,    40,    14,    15,    16,     0,     0,   151,     0,
      18,     0,     0,     0,     0,    19,     7,     8,     9,    10,
      11,    12,    40,    14,    15,    16,     0,     0,   152,     0,
      18,     0,     0,     0,     0,    19,     7,     8,     9,    10,
      11,    12,    40,    14,    15,    16,     0,     0,   190,     0,
      18,     0,     0,     0,     0,    19,     7,     8,     9,    10,
      11,    12,    40,    14,    15,    16,     0,     0,     0,     0,
      18,     0,     0,     0,     0,    19,     7,     8,     9,    10,
      11,    12,     0,    14,    15,    16,     0,     0,     0,     0,
      18,     0,     0,     0,     0,    19,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121
};

static const yytype_int16 yycheck[] =
{
       3,     3,    45,    29,     3,     8,     9,    10,     3,     0,
      13,    35,    36,    59,    41,    42,    41,    42,    41,    41,
      23,    24,    25,    41,    27,    54,    29,    46,    50,    54,
      52,    52,    35,    36,    60,    61,    58,    58,    41,    50,
      45,    65,    45,    60,    48,    72,    49,    50,    51,    52,
      48,    50,    61,    48,     0,    68,    80,    60,    61,    32,
      33,    34,    65,   159,    90,    30,    31,    32,    33,    34,
      66,    17,    -1,    -1,    20,    21,    -1,    80,    49,    50,
      51,    -1,    85,    86,    30,   131,    89,    90,    -1,    92,
      -1,    -1,    -1,    -1,    -1,    -1,    99,   100,    -1,   102,
     124,     3,    -1,    -1,   130,    -1,   132,    -1,    54,    28,
      29,    30,    31,    32,    33,    34,    62,    63,    64,    -1,
      66,   124,   125,    -1,    -1,    -1,    -1,   130,    -1,   132,
     133,    -1,    -1,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,   146,    90,    -1,    -1,    49,    -1,    -1,
      -1,    -1,    54,    -1,    -1,    -1,   159,   160,   104,   105,
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
      -1,    -1,    99,   100,    -1,   102,   146,    -1,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,   159,
     160,    -1,    -1,    -1,    -1,    -1,    -1,   124,   125,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   133,    -1,    -1,    -1,
      -1,   181,    -1,    48,    41,    -1,    -1,    -1,    -1,   146,
      55,    -1,    -1,   193,    -1,    52,    -1,    -1,    -1,    56,
      57,    58,   159,   160,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    -1,    81,    -1,   218,    -1,
      -1,    -1,    -1,    -1,   181,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    -1,   193,    47,   103,    49,
      -1,    30,    31,    -1,    54,    -1,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    -1,   123,    48,
      49,   218,    -1,   128,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    62,    63,    64,    65,    -1,    67,    68,
      69,    70,    71,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,     3,    -1,    -1,    -1,    -1,
      -1,   186,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,   197,    -1,    -1,    -1,    49,    -1,    -1,    -1,
      -1,    54,    30,    31,    -1,    -1,    59,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    -1,    -1,
      -1,    49,    -1,    -1,    -1,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    62,    63,    64,    65,    -1,    67,
      68,    69,    70,    71,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,    -1,
      -1,    -1,    54,    55,    -1,    -1,    -1,    59,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    -1,    -1,    -1,
      -1,    49,    -1,    -1,    -1,    -1,    54,    30,    31,    -1,
      -1,    59,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    -1,    -1,    -1,    49,    -1,    -1,    -1,
      -1,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    62,
      63,    64,    65,    -1,    67,    68,    69,    70,    71,    30,
      31,    -1,    -1,    -1,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    -1,    -1,    -1,    49,    -1,
      -1,    -1,    -1,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    62,    63,    64,    65,    -1,    67,    68,    69,    70,
      71,    30,    31,    -1,    -1,    -1,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    -1,    -1,    -1,
      49,    -1,    -1,    -1,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    62,    63,    64,    65,    -1,    67,    68,
      69,    70,    71,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    -1,    55,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      -1,    55,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    -1,    53,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    48,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    -1,    -1,    48,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    -1,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    -1,    47,    -1,    -1,
      49,    -1,    -1,    -1,    -1,    54,    55,    -1,    -1,    -1,
      59,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    -1,    -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,
      54,    -1,    -1,    -1,    -1,    59,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,
      49,    50,    -1,    -1,    -1,    54,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,
      49,    50,    -1,    -1,    -1,    54,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    -1,    -1,    47,    -1,
      49,    -1,    -1,    -1,    -1,    54,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    -1,    -1,    47,    -1,
      49,    -1,    -1,    -1,    -1,    54,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    -1,    -1,    47,    -1,
      49,    -1,    -1,    -1,    -1,    54,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    -1,    -1,    -1,    54,    35,    36,    37,    38,
      39,    40,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    -1,    -1,    -1,    54,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,    76,    90,     0,    30,    31,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    49,    54,
      62,    63,    64,    65,    67,    68,    69,    70,    71,    77,
      78,    79,    85,    86,    90,    41,    41,    85,    85,    85,
      41,    82,    84,    85,    87,    90,    41,    42,    54,    36,
      37,    38,    41,    55,    59,    80,    81,    85,    87,    90,
      90,    54,    84,    84,    84,    84,    78,    48,    90,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      41,    52,    56,    57,    58,    82,    82,    47,    84,    50,
      77,    80,    41,    81,    85,    81,    85,    81,    85,    50,
      83,    84,    90,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    55,    80,    41,    50,    79,    78,    48,    78,
      90,    90,    90,    82,    90,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    82,    80,    41,    42,
      72,    47,    47,    84,    90,    55,    84,    84,    84,    88,
      89,    80,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      80,    82,    84,    90,    90,    80,    48,    78,    79,    78,
      47,    53,    46,    89,    90,    84,    90,    47,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    55,    61,    68,    48,
      80,    90,    90,    60,    80,    90,    84,    77,    48,    66,
      79,    55,    77,    90,    90,    55,    78,    78,    90,    78
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
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

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
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


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
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
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
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


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
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
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
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
            else
              goto append;

          append:
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

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
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
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
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
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= END)
    {
      yychar = END;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* program: newline statement_list  */
#line 108 "E:/Src/openmohaa/code/parser/bison_source.txt"
                               { parsedata.val = node1( sval_statement_list, (yyvsp[0].s.val) ); }
#line 1806 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 3: /* statement_list: statement_list statement newline  */
#line 112 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                         { (yyval.s.val) = append_node( (yyvsp[-2].s.val), (yyvsp[-1].s.val) ); }
#line 1812 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 4: /* statement_list: statement newline  */
#line 114 "E:/Src/openmohaa/code/parser/bison_source.txt"
                            { (yyval.s.val) = linked_list_end( (yyvsp[-1].s.val) ); }
#line 1818 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 5: /* statement_list: newline  */
#line 115 "E:/Src/openmohaa/code/parser/bison_source.txt"
                  { (yyval.s.val) = node0( sval_none ); }
#line 1824 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 6: /* statement: TOKEN_IDENTIFIER event_parameter_list TOKEN_COLON  */
#line 119 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                          { (yyval.s.val) = node3( sval_label, (yyvsp[-2].s).val, (yyvsp[-1].s.val), YYLLOC ); }
#line 1830 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 7: /* statement: TOKEN_PLUS TOKEN_IDENTIFIER event_parameter_list TOKEN_COLON  */
#line 120 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                                       { (yyval.s.val) = node3( sval_label, (yyvsp[-2].s).val, (yyvsp[-1].s.val), YYLLOC ); }
#line 1836 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 8: /* statement: TOKEN_MINUS TOKEN_IDENTIFIER event_parameter_list TOKEN_COLON  */
#line 121 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                                        { (yyval.s.val) = node3( sval_privatelabel, (yyvsp[-2].s).val, (yyvsp[-1].s.val), YYLLOC ); }
#line 1842 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 9: /* statement: TOKEN_CASE prim_expr event_parameter_list TOKEN_COLON  */
#line 122 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                                { (yyval.s.val) = node3( sval_case, (yyvsp[-2].s.val), (yyvsp[-1].s.val), YYLLOC ); }
#line 1848 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 11: /* statement: TOKEN_IF prim_expr newline statement newline  */
#line 124 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                          { (yyval.s.val) = node3( sval_if, (yyvsp[-3].s.val), (yyvsp[-1].s.val), YYLLOC ); }
#line 1854 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 12: /* statement: TOKEN_IF prim_expr newline statement newline TOKEN_ELSE newline statement  */
#line 125 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                                                            { (yyval.s.val) = node4( sval_ifelse, (yyvsp[-6].s.val), (yyvsp[-4].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 1860 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 13: /* statement: TOKEN_WHILE prim_expr newline statement  */
#line 126 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                  { (yyval.s.val) = node4( sval_while, (yyvsp[-2].s.val), (yyvsp[0].s.val), node0( sval_none ), YYLLOC ); }
#line 1866 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 14: /* statement: TOKEN_FOR TOKEN_LPAREN statement TOKEN_SEMICOLON expr TOKEN_SEMICOLON statement_list TOKEN_RPAREN newline statement  */
#line 128 "E:/Src/openmohaa/code/parser/bison_source.txt"
        {
		sval_u while_stmt = node4( sval_while, (yyvsp[-5].s.val), (yyvsp[0].s.val), node1( sval_statement_list, (yyvsp[-3].s.val) ), YYLLOC );
		(yyval.s.val) = node1( sval_statement_list, append_node( linked_list_end( (yyvsp[-7].s.val) ), while_stmt ) );
	}
#line 1875 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 15: /* statement: TOKEN_FOR TOKEN_LPAREN TOKEN_SEMICOLON expr TOKEN_SEMICOLON statement_list TOKEN_RPAREN newline statement  */
#line 133 "E:/Src/openmohaa/code/parser/bison_source.txt"
        {
		(yyval.s.val) = node4( sval_while, (yyvsp[-5].s.val), (yyvsp[0].s.val), node1( sval_statement_list, (yyvsp[-3].s.val) ), YYLLOC );
	}
#line 1883 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 16: /* statement: TOKEN_DO newline statement newline TOKEN_WHILE prim_expr  */
#line 136 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                                         { (yyval.s.val) = node3( sval_do, (yyvsp[-3].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 1889 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 17: /* statement: TOKEN_TRY newline compound_statement newline TOKEN_CATCH newline compound_statement  */
#line 137 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                                                              { (yyval.s.val) = node3( sval_catch, (yyvsp[-4].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 1895 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 18: /* statement: TOKEN_SWITCH prim_expr newline compound_statement  */
#line 138 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                            { (yyval.s.val) = node3( sval_switch, (yyvsp[-2].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 1901 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 19: /* statement: TOKEN_BREAK  */
#line 139 "E:/Src/openmohaa/code/parser/bison_source.txt"
                      { (yyval.s.val) = node1( sval_break, YYLLOC ); }
#line 1907 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 20: /* statement: TOKEN_CONTINUE  */
#line 140 "E:/Src/openmohaa/code/parser/bison_source.txt"
                         { (yyval.s.val) = node1( sval_continue, YYLLOC ); }
#line 1913 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 21: /* statement: TOKEN_IDENTIFIER event_parameter_list  */
#line 141 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                { (yyval.s.val) = node3( sval_cmd, (yyvsp[-1].s).val, node1( sval_none, (yyvsp[0].s.val) ), node_pos( (yyvsp[-1].s).sourcePos ) ); }
#line 1919 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 22: /* statement: nonident_prim_expr TOKEN_IDENTIFIER event_parameter_list  */
#line 143 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                                   { (yyval.s.val) = node4( sval_cmd_method, (yyvsp[-2].s.val), (yyvsp[-1].s).val, node1( sval_none, (yyvsp[0].s.val) ), node_pos( (yyvsp[-1].s).sourcePos ) ); }
#line 1925 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 23: /* statement: nonident_prim_expr TOKEN_ASSIGNMENT expr  */
#line 145 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                   { (yyval.s.val) = node3( sval_assignment, (yyvsp[-2].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 1931 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 24: /* statement: nonident_prim_expr TOKEN_PLUS_EQUALS expr  */
#line 146 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                    { (yyval.s.val) = node3( sval_assignment, (yyvsp[-2].s.val), node4( sval_operation, node1b( OP_BIN_PLUS ), (yyvsp[-2].s.val), (yyvsp[0].s.val), YYLLOC ), YYLLOC ); }
#line 1937 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 25: /* statement: nonident_prim_expr TOKEN_MINUS_EQUALS expr  */
#line 147 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                     { (yyval.s.val) = node3( sval_assignment, (yyvsp[-2].s.val), node4( sval_operation, node1b( OP_BIN_MINUS ), (yyvsp[-2].s.val), (yyvsp[0].s.val), YYLLOC ), YYLLOC ); }
#line 1943 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 26: /* statement: nonident_prim_expr TOKEN_MULTIPLY_EQUALS expr  */
#line 148 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                        { (yyval.s.val) = node3( sval_assignment, (yyvsp[-2].s.val), node4( sval_operation, node1b( OP_BIN_MULTIPLY ), (yyvsp[-2].s.val), (yyvsp[0].s.val), YYLLOC ), YYLLOC ); }
#line 1949 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 27: /* statement: nonident_prim_expr TOKEN_DIVIDE_EQUALS expr  */
#line 149 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                      { (yyval.s.val) = node3( sval_assignment, (yyvsp[-2].s.val), node4( sval_operation, node1b( OP_BIN_DIVIDE ), (yyvsp[-2].s.val), (yyvsp[0].s.val), YYLLOC ), YYLLOC ); }
#line 1955 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 28: /* statement: nonident_prim_expr TOKEN_MODULUS_EQUALS expr  */
#line 150 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                       { (yyval.s.val) = node3( sval_assignment, (yyvsp[-2].s.val), node4( sval_operation, node1b( OP_BIN_PERCENTAGE ), (yyvsp[-2].s.val), (yyvsp[0].s.val), YYLLOC ), YYLLOC ); }
#line 1961 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 29: /* statement: nonident_prim_expr TOKEN_AND_EQUALS expr  */
#line 151 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                   { (yyval.s.val) = node3( sval_assignment, (yyvsp[-2].s.val), node4( sval_operation, node1b( OP_BIN_BITWISE_AND ), (yyvsp[-2].s.val), (yyvsp[0].s.val), YYLLOC ), YYLLOC ); }
#line 1967 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 30: /* statement: nonident_prim_expr TOKEN_EXCL_OR_EQUALS expr  */
#line 152 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                       { (yyval.s.val) = node3( sval_assignment, (yyvsp[-2].s.val), node4( sval_operation, node1b( OP_BIN_BITWISE_EXCL_OR ), (yyvsp[-2].s.val), (yyvsp[0].s.val), YYLLOC ), YYLLOC ); }
#line 1973 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 31: /* statement: nonident_prim_expr TOKEN_OR_EQUALS expr  */
#line 153 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                  { (yyval.s.val) = node3( sval_assignment, (yyvsp[-2].s.val), node4( sval_operation, node1b( OP_BIN_BITWISE_OR ), (yyvsp[-2].s.val), (yyvsp[0].s.val), YYLLOC ), YYLLOC ); }
#line 1979 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 32: /* statement: nonident_prim_expr TOKEN_SHIFT_LEFT_EQUALS expr  */
#line 154 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                          { (yyval.s.val) = node3( sval_assignment, (yyvsp[-2].s.val), node4( sval_operation, node1b( OP_BIN_SHIFT_LEFT ), (yyvsp[-2].s.val), (yyvsp[0].s.val), YYLLOC ), YYLLOC ); }
#line 1985 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 33: /* statement: nonident_prim_expr TOKEN_SHIFT_RIGHT_EQUALS expr  */
#line 155 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                           { (yyval.s.val) = node3( sval_assignment, (yyvsp[-2].s.val), node4( sval_operation, node1b( OP_BIN_SHIFT_RIGHT ), (yyvsp[-2].s.val), (yyvsp[0].s.val), YYLLOC ), YYLLOC ); }
#line 1991 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 34: /* statement: nonident_prim_expr TOKEN_INCREMENT  */
#line 156 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                             { (yyval.s.val) = node3( sval_assignment, (yyvsp[-1].s.val), node3( sval_func1, node1b( OP_UN_INC ), (yyvsp[-1].s.val), YYLLOC ), YYLLOC ); }
#line 1997 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 35: /* statement: nonident_prim_expr TOKEN_DECREMENT  */
#line 157 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                             { (yyval.s.val) = node3( sval_assignment, (yyvsp[-1].s.val), node3( sval_func1, node1b( OP_UN_DEC ), (yyvsp[-1].s.val), YYLLOC ), YYLLOC ); }
#line 2003 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 37: /* compound_statement: TOKEN_LBRACKET newline statement_list newline TOKEN_RBRACKET  */
#line 162 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                                     { (yyval.s.val) = node1( sval_statement_list, (yyvsp[-2].s.val) ); }
#line 2009 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 38: /* expr: expr TOKEN_LOGICAL_AND newline expr  */
#line 166 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                            { (yyval.s.val) = node3( sval_and, (yyvsp[-3].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 2015 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 39: /* expr: expr TOKEN_LOGICAL_OR newline expr  */
#line 167 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                             { (yyval.s.val) = node3( sval_or, (yyvsp[-3].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 2021 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 40: /* expr: expr TOKEN_BITWISE_AND newline expr  */
#line 168 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                              { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_BITWISE_AND ), (yyvsp[-3].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 2027 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 41: /* expr: expr TOKEN_BITWISE_EXCL_OR newline expr  */
#line 169 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                  { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_BITWISE_EXCL_OR ), (yyvsp[-3].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 2033 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 42: /* expr: expr TOKEN_BITWISE_OR newline expr  */
#line 170 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                             { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_BITWISE_OR ), (yyvsp[-3].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 2039 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 43: /* expr: expr TOKEN_EQUALITY newline expr  */
#line 171 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                           { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_EQUALITY ), (yyvsp[-3].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 2045 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 44: /* expr: expr TOKEN_INEQUALITY newline expr  */
#line 172 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                             { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_INEQUALITY ), (yyvsp[-3].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 2051 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 45: /* expr: expr TOKEN_LESS_THAN newline expr  */
#line 173 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                            { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_LESS_THAN ), (yyvsp[-3].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 2057 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 46: /* expr: expr TOKEN_GREATER_THAN newline expr  */
#line 174 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                               { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_GREATER_THAN ), (yyvsp[-3].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 2063 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 47: /* expr: expr TOKEN_LESS_THAN_OR_EQUAL newline expr  */
#line 175 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                     { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_LESS_THAN_OR_EQUAL ), (yyvsp[-3].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 2069 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 48: /* expr: expr TOKEN_GREATER_THAN_OR_EQUAL newline expr  */
#line 176 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                        { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_GREATER_THAN_OR_EQUAL ), (yyvsp[-3].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 2075 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 49: /* expr: expr TOKEN_PLUS newline expr  */
#line 177 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                       { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_PLUS ), (yyvsp[-3].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 2081 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 50: /* expr: expr TOKEN_MINUS newline expr  */
#line 178 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                        { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_MINUS ), (yyvsp[-3].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 2087 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 51: /* expr: expr TOKEN_MULTIPLY newline expr  */
#line 179 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                           { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_MULTIPLY ), (yyvsp[-3].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 2093 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 52: /* expr: expr TOKEN_DIVIDE newline expr  */
#line 180 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                         { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_DIVIDE ), (yyvsp[-3].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 2099 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 53: /* expr: expr TOKEN_MODULUS newline expr  */
#line 181 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                          { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_PERCENTAGE ), (yyvsp[-3].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 2105 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 54: /* expr: expr TOKEN_SHIFT_LEFT newline expr  */
#line 182 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                             { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_SHIFT_LEFT ), (yyvsp[-3].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 2111 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 55: /* expr: expr TOKEN_SHIFT_RIGHT newline expr  */
#line 183 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                              { (yyval.s.val) = node4( sval_operation, node1b( OP_BIN_SHIFT_RIGHT ), (yyvsp[-3].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 2117 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 56: /* expr: expr TOKEN_TERNARY expr TOKEN_COLON expr  */
#line 184 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                   { (yyval.s.val) = node4( sval_ifelse, (yyvsp[-4].s.val), (yyvsp[-2].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 2123 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 59: /* expr: identifier_prim  */
#line 187 "E:/Src/openmohaa/code/parser/bison_source.txt"
                          { (yyval.s.val) = node1( sval_store_string, (yyvsp[0].s).val ); }
#line 2129 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 60: /* func_prim_expr: TOKEN_IDENTIFIER event_parameter_list_need  */
#line 191 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                   { (yyval.s.val) = node3( sval_cmd_default_ret, (yyvsp[-1].s).val, node1( sval_none, (yyvsp[0].s.val) ), node_pos( (yyvsp[-1].s).sourcePos ) ); }
#line 2135 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 61: /* func_prim_expr: nonident_prim_expr TOKEN_IDENTIFIER event_parameter_list  */
#line 193 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                                   { (yyval.s.val) = node4( sval_cmd_method_ret, (yyvsp[-2].s.val), (yyvsp[-1].s).val, node1( sval_none, (yyvsp[0].s.val) ), node_pos( (yyvsp[-1].s).sourcePos ) ); }
#line 2141 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 62: /* func_prim_expr: TOKEN_NEG func_prim_expr  */
#line 195 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                   { (yyval.s.val) = node3( sval_func1, node1b( OP_UN_MINUS ), (yyvsp[0].s.val), YYLLOC ); }
#line 2147 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 63: /* func_prim_expr: TOKEN_COMPLEMENT func_prim_expr  */
#line 196 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                          { (yyval.s.val) = node3( sval_func1, node1b( OP_UN_COMPLEMENT ), (yyvsp[0].s.val), YYLLOC ); }
#line 2153 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 64: /* func_prim_expr: TOKEN_NOT func_prim_expr  */
#line 197 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                   { (yyval.s.val) = node2( sval_not, (yyvsp[0].s.val), YYLLOC ); }
#line 2159 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 65: /* func_prim_expr: TOKEN_IDENTIFIER TOKEN_DOUBLE_COLON prim_expr  */
#line 198 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                        { (yyval.s.val) = node3( sval_constarray, node2( sval_store_string, (yyvsp[-2].s).val, node_pos( (yyvsp[-2].s).sourcePos ) ), (yyvsp[0].s.val), YYLLOC ); }
#line 2165 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 66: /* func_prim_expr: nonident_prim_expr TOKEN_DOUBLE_COLON prim_expr  */
#line 199 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                          { (yyval.s.val) = node3( sval_constarray, (yyvsp[-2].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 2171 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 67: /* func_prim_expr: TOKEN_MAKEARRAY newline makearray_statement_list newline TOKEN_ENDARRAY  */
#line 200 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                                                  { (yyval.s.val) = node1( sval_makearray, (yyvsp[-2].s.val) ); }
#line 2177 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 68: /* event_parameter_list: %empty  */
#line 204 "E:/Src/openmohaa/code/parser/bison_source.txt"
        { (yyval.s.val) = node0( sval_none ); }
#line 2183 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 69: /* event_parameter_list: event_parameter_list prim_expr  */
#line 205 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                         { (yyval.s.val) = append_node( (yyvsp[-1].s.val), (yyvsp[0].s.val) ); }
#line 2189 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 70: /* event_parameter_list: prim_expr  */
#line 206 "E:/Src/openmohaa/code/parser/bison_source.txt"
                    { (yyval.s.val) = linked_list_end( (yyvsp[0].s.val) ); }
#line 2195 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 71: /* event_parameter_list_need: event_parameter_list_need prim_expr  */
#line 210 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                            { (yyval.s.val) = append_node( (yyvsp[-1].s.val), (yyvsp[0].s.val) ); }
#line 2201 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 72: /* event_parameter_list_need: prim_expr  */
#line 211 "E:/Src/openmohaa/code/parser/bison_source.txt"
                    { (yyval.s.val) = linked_list_end( (yyvsp[0].s.val) ); }
#line 2207 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 74: /* prim_expr: identifier_prim  */
#line 216 "E:/Src/openmohaa/code/parser/bison_source.txt"
                          { (yyval.s.val) = node1( sval_store_string, (yyvsp[0].s).val ); }
#line 2213 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 75: /* prim_expr: prim_expr TOKEN_DOUBLE_COLON prim_expr  */
#line 217 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                 { (yyval.s.val) = node3( sval_constarray, (yyvsp[-2].s.val), (yyvsp[0].s.val), YYLLOC ); }
#line 2219 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 76: /* nonident_prim_expr: TOKEN_DOLLAR TOKEN_LPAREN expr TOKEN_RPAREN  */
#line 221 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                    { (yyval.s.val) = node3( sval_func1, node1b( OP_UN_TARGETNAME ), (yyvsp[-1].s.val), YYLLOC ); }
#line 2225 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 77: /* nonident_prim_expr: TOKEN_DOLLAR TOKEN_IDENTIFIER  */
#line 222 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                        { (yyval.s.val) = node3( sval_func1, node1b( OP_UN_TARGETNAME ), node1( sval_store_string, (yyvsp[0].s).val ), YYLLOC ); }
#line 2231 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 78: /* nonident_prim_expr: TOKEN_DOLLAR TOKEN_STRING  */
#line 223 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                    { (yyval.s.val) = node3( sval_func1, node1b( OP_UN_TARGETNAME ), node1( sval_store_string, (yyvsp[0].s).val ), YYLLOC ); }
#line 2237 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 79: /* nonident_prim_expr: nonident_prim_expr TOKEN_PERIOD TOKEN_IDENTIFIER  */
#line 224 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                           { (yyval.s.val) = node3( sval_field, (yyvsp[-2].s.val), (yyvsp[0].s).val, node_pos( (yyvsp[0].s).sourcePos ) ); }
#line 2243 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 80: /* nonident_prim_expr: nonident_prim_expr TOKEN_PERIOD TOKEN_STRING  */
#line 225 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                       { (yyval.s.val) = node3( sval_field, (yyvsp[-2].s.val), (yyvsp[0].s).val, node_pos( (yyvsp[0].s).sourcePos ) ); }
#line 2249 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 81: /* nonident_prim_expr: nonident_prim_expr TOKEN_PERIOD TOKEN_SIZE  */
#line 226 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                     { (yyval.s.val) = node3( sval_func1, node1b( OP_UN_SIZE ), (yyvsp[-2].s.val), YYLLOC ); }
#line 2255 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 82: /* nonident_prim_expr: nonident_prim_expr TOKEN_LSQUARE expr TOKEN_RSQUARE  */
#line 227 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                              { (yyval.s.val) = node3( sval_array, (yyvsp[-3].s.val), (yyvsp[-1].s.val), (yyvsp[-3].s.val) ); }
#line 2261 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 83: /* nonident_prim_expr: TOKEN_STRING  */
#line 228 "E:/Src/openmohaa/code/parser/bison_source.txt"
                       { (yyval.s.val) = node1( sval_store_string, (yyvsp[0].s).val ); }
#line 2267 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 85: /* nonident_prim_expr: TOKEN_LPAREN expr expr expr TOKEN_RPAREN  */
#line 230 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                   { (yyval.s.val) = node4( sval_calc_vector, (yyvsp[-3].s.val), (yyvsp[-2].s.val), (yyvsp[-1].s.val), YYLLOC ); }
#line 2273 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 86: /* nonident_prim_expr: TOKEN_LISTENER  */
#line 231 "E:/Src/openmohaa/code/parser/bison_source.txt"
                         { (yyval.s.val) = node2( sval_store_method, (yyvsp[0].s).val, YYLLOC ); }
#line 2279 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 87: /* nonident_prim_expr: TOKEN_LPAREN expr TOKEN_RPAREN  */
#line 232 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                         { (yyval.s.val) = (yyvsp[-1].s.val); }
#line 2285 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 88: /* nonident_prim_expr: TOKEN_LPAREN TOKEN_RPAREN  */
#line 233 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                    { (yyval.s.val) = node0( sval_none ); }
#line 2291 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 89: /* nonident_prim_expr: TOKEN_NEG nonident_prim_expr  */
#line 234 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                       { (yyval.s.val) = node3( sval_func1, node1b( OP_UN_MINUS ), (yyvsp[0].s.val), YYLLOC ); }
#line 2297 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 90: /* nonident_prim_expr: TOKEN_COMPLEMENT nonident_prim_expr  */
#line 235 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                              { (yyval.s.val) = node3( sval_func1, node1b( OP_UN_COMPLEMENT ), (yyvsp[0].s.val), YYLLOC ); }
#line 2303 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 91: /* nonident_prim_expr: TOKEN_NOT nonident_prim_expr  */
#line 236 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                       { (yyval.s.val) = node2( sval_not, (yyvsp[0].s.val), YYLLOC ); }
#line 2309 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 92: /* nonident_prim_expr: TOKEN_NULL  */
#line 237 "E:/Src/openmohaa/code/parser/bison_source.txt"
                     { (yyval.s.val) = node1( sval_store_null, YYLLOC ); }
#line 2315 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 93: /* nonident_prim_expr: TOKEN_NIL  */
#line 238 "E:/Src/openmohaa/code/parser/bison_source.txt"
                    { (yyval.s.val) = node1( sval_store_nil, YYLLOC ); }
#line 2321 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 94: /* number: TOKEN_FLOAT  */
#line 242 "E:/Src/openmohaa/code/parser/bison_source.txt"
                    { (yyval.s.val) = node1( sval_store_float, (yyvsp[0].s).val ); }
#line 2327 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 95: /* number: TOKEN_INTEGER  */
#line 243 "E:/Src/openmohaa/code/parser/bison_source.txt"
                        { (yyval.s.val) = node1( sval_store_integer, (yyvsp[0].s).val ); }
#line 2333 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 97: /* makearray_statement_list: %empty  */
#line 251 "E:/Src/openmohaa/code/parser/bison_source.txt"
        { (yyval.s.val) = node0( sval_none ); }
#line 2339 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 98: /* makearray_statement_list: makearray_statement_list makearray_statement newline  */
#line 252 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                                               { (yyval.s.val) = append_node( (yyvsp[-2].s.val), node1( sval_makearray, (yyvsp[-1].s.val) ) ); }
#line 2345 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 99: /* makearray_statement_list: makearray_statement newline  */
#line 253 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                      { (yyval.s.val) = linked_list_end( node1( sval_makearray, (yyvsp[-1].s.val) ) ); }
#line 2351 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 100: /* makearray_statement: prim_expr  */
#line 257 "E:/Src/openmohaa/code/parser/bison_source.txt"
                  { (yyval.s.val) = linked_list_end( (yyvsp[0].s.val) ); }
#line 2357 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 101: /* makearray_statement: makearray_statement prim_expr  */
#line 258 "E:/Src/openmohaa/code/parser/bison_source.txt"
                                        { (yyval.s.val) = append_node( (yyvsp[-1].s.val), (yyvsp[0].s.val) ); }
#line 2363 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 102: /* newline: %empty  */
#line 262 "E:/Src/openmohaa/code/parser/bison_source.txt"
        {}
#line 2369 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"
    break;


#line 2373 "E:/Src/openmohaa/code/parser/generated/yyParser.cpp"

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
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= END)
        {
          /* Return failure if at end of input.  */
          if (yychar == END)
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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 266 "E:/Src/openmohaa/code/parser/bison_source.txt"

