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
#line 1 "/usr/src/openmohaa/code/parser/bison_source.txt"

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

int prev_yylex;
int out_pos;
int success_pos;

#define YYLLOC node_pos(success_pos - yyleng)
#define TOKPOS(pos) node_pos(pos.sourcePos)


#line 116 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"

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
  YYSYMBOL_YYEOF = 0,                      /* TOKEN_EOF  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_3_end_of_file_ = 3,             /* "end of file"  */
  YYSYMBOL_TOKEN_EOL = 4,                  /* TOKEN_EOL  */
  YYSYMBOL_TOKEN_COMMA = 5,                /* TOKEN_COMMA  */
  YYSYMBOL_TOKEN_IF = 6,                   /* TOKEN_IF  */
  YYSYMBOL_TOKEN_ELSE = 7,                 /* TOKEN_ELSE  */
  YYSYMBOL_TOKEN_WHILE = 8,                /* TOKEN_WHILE  */
  YYSYMBOL_TOKEN_FOR = 9,                  /* TOKEN_FOR  */
  YYSYMBOL_TOKEN_DO = 10,                  /* TOKEN_DO  */
  YYSYMBOL_TOKEN_IDENTIFIER = 11,          /* TOKEN_IDENTIFIER  */
  YYSYMBOL_TOKEN_LEFT_BRACES = 12,         /* TOKEN_LEFT_BRACES  */
  YYSYMBOL_TOKEN_RIGHT_BRACES = 13,        /* TOKEN_RIGHT_BRACES  */
  YYSYMBOL_TOKEN_LEFT_BRACKET = 14,        /* TOKEN_LEFT_BRACKET  */
  YYSYMBOL_TOKEN_RIGHT_BRACKET = 15,       /* TOKEN_RIGHT_BRACKET  */
  YYSYMBOL_TOKEN_LEFT_SQUARE_BRACKET = 16, /* TOKEN_LEFT_SQUARE_BRACKET  */
  YYSYMBOL_TOKEN_RIGHT_SQUARE_BRACKET = 17, /* TOKEN_RIGHT_SQUARE_BRACKET  */
  YYSYMBOL_TOKEN_ASSIGNMENT = 18,          /* TOKEN_ASSIGNMENT  */
  YYSYMBOL_TOKEN_PLUS_EQUALS = 19,         /* TOKEN_PLUS_EQUALS  */
  YYSYMBOL_TOKEN_MINUS_EQUALS = 20,        /* TOKEN_MINUS_EQUALS  */
  YYSYMBOL_TOKEN_MULTIPLY_EQUALS = 21,     /* TOKEN_MULTIPLY_EQUALS  */
  YYSYMBOL_TOKEN_DIVIDE_EQUALS = 22,       /* TOKEN_DIVIDE_EQUALS  */
  YYSYMBOL_TOKEN_MODULUS_EQUALS = 23,      /* TOKEN_MODULUS_EQUALS  */
  YYSYMBOL_TOKEN_AND_EQUALS = 24,          /* TOKEN_AND_EQUALS  */
  YYSYMBOL_TOKEN_EXCL_OR_EQUALS = 25,      /* TOKEN_EXCL_OR_EQUALS  */
  YYSYMBOL_TOKEN_OR_EQUALS = 26,           /* TOKEN_OR_EQUALS  */
  YYSYMBOL_TOKEN_SHIFT_LEFT_EQUALS = 27,   /* TOKEN_SHIFT_LEFT_EQUALS  */
  YYSYMBOL_TOKEN_SHIFT_RIGHT_EQUALS = 28,  /* TOKEN_SHIFT_RIGHT_EQUALS  */
  YYSYMBOL_TOKEN_TERNARY = 29,             /* TOKEN_TERNARY  */
  YYSYMBOL_TOKEN_COLON = 30,               /* TOKEN_COLON  */
  YYSYMBOL_TOKEN_LOGICAL_OR = 31,          /* TOKEN_LOGICAL_OR  */
  YYSYMBOL_TOKEN_LOGICAL_AND = 32,         /* TOKEN_LOGICAL_AND  */
  YYSYMBOL_TOKEN_BITWISE_OR = 33,          /* TOKEN_BITWISE_OR  */
  YYSYMBOL_TOKEN_BITWISE_EXCL_OR = 34,     /* TOKEN_BITWISE_EXCL_OR  */
  YYSYMBOL_TOKEN_BITWISE_AND = 35,         /* TOKEN_BITWISE_AND  */
  YYSYMBOL_TOKEN_EQUALITY = 36,            /* TOKEN_EQUALITY  */
  YYSYMBOL_TOKEN_INEQUALITY = 37,          /* TOKEN_INEQUALITY  */
  YYSYMBOL_TOKEN_LESS_THAN = 38,           /* TOKEN_LESS_THAN  */
  YYSYMBOL_TOKEN_LESS_THAN_OR_EQUAL = 39,  /* TOKEN_LESS_THAN_OR_EQUAL  */
  YYSYMBOL_TOKEN_GREATER_THAN = 40,        /* TOKEN_GREATER_THAN  */
  YYSYMBOL_TOKEN_GREATER_THAN_OR_EQUAL = 41, /* TOKEN_GREATER_THAN_OR_EQUAL  */
  YYSYMBOL_TOKEN_SHIFT_LEFT = 42,          /* TOKEN_SHIFT_LEFT  */
  YYSYMBOL_TOKEN_SHIFT_RIGHT = 43,         /* TOKEN_SHIFT_RIGHT  */
  YYSYMBOL_TOKEN_PLUS = 44,                /* TOKEN_PLUS  */
  YYSYMBOL_TOKEN_MINUS = 45,               /* TOKEN_MINUS  */
  YYSYMBOL_TOKEN_MULTIPLY = 46,            /* TOKEN_MULTIPLY  */
  YYSYMBOL_TOKEN_DIVIDE = 47,              /* TOKEN_DIVIDE  */
  YYSYMBOL_TOKEN_MODULUS = 48,             /* TOKEN_MODULUS  */
  YYSYMBOL_TOKEN_NEG = 49,                 /* TOKEN_NEG  */
  YYSYMBOL_TOKEN_NOT = 50,                 /* TOKEN_NOT  */
  YYSYMBOL_TOKEN_COMPLEMENT = 51,          /* TOKEN_COMPLEMENT  */
  YYSYMBOL_TOKEN_STRING = 52,              /* TOKEN_STRING  */
  YYSYMBOL_TOKEN_INTEGER = 53,             /* TOKEN_INTEGER  */
  YYSYMBOL_TOKEN_FLOAT = 54,               /* TOKEN_FLOAT  */
  YYSYMBOL_TOKEN_LISTENER = 55,            /* TOKEN_LISTENER  */
  YYSYMBOL_TOKEN_NIL = 56,                 /* TOKEN_NIL  */
  YYSYMBOL_TOKEN_NULL = 57,                /* TOKEN_NULL  */
  YYSYMBOL_TOKEN_DOUBLE_COLON = 58,        /* TOKEN_DOUBLE_COLON  */
  YYSYMBOL_TOKEN_SEMICOLON = 59,           /* TOKEN_SEMICOLON  */
  YYSYMBOL_TOKEN_DOLLAR = 60,              /* TOKEN_DOLLAR  */
  YYSYMBOL_TOKEN_NUMBER = 61,              /* TOKEN_NUMBER  */
  YYSYMBOL_TOKEN_INCREMENT = 62,           /* TOKEN_INCREMENT  */
  YYSYMBOL_TOKEN_DECREMENT = 63,           /* TOKEN_DECREMENT  */
  YYSYMBOL_TOKEN_PERIOD = 64,              /* TOKEN_PERIOD  */
  YYSYMBOL_TOKEN_CATCH = 65,               /* TOKEN_CATCH  */
  YYSYMBOL_TOKEN_TRY = 66,                 /* TOKEN_TRY  */
  YYSYMBOL_TOKEN_SWITCH = 67,              /* TOKEN_SWITCH  */
  YYSYMBOL_TOKEN_CASE = 68,                /* TOKEN_CASE  */
  YYSYMBOL_TOKEN_BREAK = 69,               /* TOKEN_BREAK  */
  YYSYMBOL_TOKEN_CONTINUE = 70,            /* TOKEN_CONTINUE  */
  YYSYMBOL_TOKEN_SIZE = 71,                /* TOKEN_SIZE  */
  YYSYMBOL_TOKEN_END = 72,                 /* TOKEN_END  */
  YYSYMBOL_TOKEN_RETURN = 73,              /* TOKEN_RETURN  */
  YYSYMBOL_TOKEN_MAKEARRAY = 74,           /* TOKEN_MAKEARRAY  */
  YYSYMBOL_TOKEN_ENDARRAY = 75,            /* TOKEN_ENDARRAY  */
  YYSYMBOL_YYACCEPT = 76,                  /* $accept  */
  YYSYMBOL_program = 77,                   /* program  */
  YYSYMBOL_statement_list = 78,            /* statement_list  */
  YYSYMBOL_statement = 79,                 /* statement  */
  YYSYMBOL_statement_declaration = 80,     /* statement_declaration  */
  YYSYMBOL_compound_statement = 81,        /* compound_statement  */
  YYSYMBOL_selection_statement = 82,       /* selection_statement  */
  YYSYMBOL_iteration_statement = 83,       /* iteration_statement  */
  YYSYMBOL_expr = 84,                      /* expr  */
  YYSYMBOL_func_prim_expr = 85,            /* func_prim_expr  */
  YYSYMBOL_event_parameter_list = 86,      /* event_parameter_list  */
  YYSYMBOL_event_parameter_list_need = 87, /* event_parameter_list_need  */
  YYSYMBOL_event_parameter = 88,           /* event_parameter  */
  YYSYMBOL_const_array_list = 89,          /* const_array_list  */
  YYSYMBOL_const_array = 90,               /* const_array  */
  YYSYMBOL_prim_expr = 91,                 /* prim_expr  */
  YYSYMBOL_identifier_prim = 92,           /* identifier_prim  */
  YYSYMBOL_identifier = 93,                /* identifier  */
  YYSYMBOL_nonident_prim_expr = 94,        /* nonident_prim_expr  */
  YYSYMBOL_func_expr = 95,                 /* func_expr  */
  YYSYMBOL_makearray_statement_list = 96,  /* makearray_statement_list  */
  YYSYMBOL_makearray_statement = 97,       /* makearray_statement  */
  YYSYMBOL_line_opt = 98                   /* line_opt  */
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
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

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
#define YYFINAL  6
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   864

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  76
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  23
/* YYNRULES -- Number of rules.  */
#define YYNRULES  109
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  205

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   330


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
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   130,   130,   131,   135,   136,   140,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   173,   174,   175,   179,   180,   181,   185,   186,   191,
     195,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   225,   226,   227,   228,   229,   233,
     237,   244,   245,   249,   253,   254,   258,   259,   263,   264,
     268,   269,   270,   274,   278,   279,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     300,   301,   305,   306,   307,   308,   312,   313,   317,   318
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
  "TOKEN_EOF", "error", "\"invalid token\"", "\"end of file\"",
  "TOKEN_EOL", "TOKEN_COMMA", "TOKEN_IF", "TOKEN_ELSE", "TOKEN_WHILE",
  "TOKEN_FOR", "TOKEN_DO", "TOKEN_IDENTIFIER", "TOKEN_LEFT_BRACES",
  "TOKEN_RIGHT_BRACES", "TOKEN_LEFT_BRACKET", "TOKEN_RIGHT_BRACKET",
  "TOKEN_LEFT_SQUARE_BRACKET", "TOKEN_RIGHT_SQUARE_BRACKET",
  "TOKEN_ASSIGNMENT", "TOKEN_PLUS_EQUALS", "TOKEN_MINUS_EQUALS",
  "TOKEN_MULTIPLY_EQUALS", "TOKEN_DIVIDE_EQUALS", "TOKEN_MODULUS_EQUALS",
  "TOKEN_AND_EQUALS", "TOKEN_EXCL_OR_EQUALS", "TOKEN_OR_EQUALS",
  "TOKEN_SHIFT_LEFT_EQUALS", "TOKEN_SHIFT_RIGHT_EQUALS", "TOKEN_TERNARY",
  "TOKEN_COLON", "TOKEN_LOGICAL_OR", "TOKEN_LOGICAL_AND",
  "TOKEN_BITWISE_OR", "TOKEN_BITWISE_EXCL_OR", "TOKEN_BITWISE_AND",
  "TOKEN_EQUALITY", "TOKEN_INEQUALITY", "TOKEN_LESS_THAN",
  "TOKEN_LESS_THAN_OR_EQUAL", "TOKEN_GREATER_THAN",
  "TOKEN_GREATER_THAN_OR_EQUAL", "TOKEN_SHIFT_LEFT", "TOKEN_SHIFT_RIGHT",
  "TOKEN_PLUS", "TOKEN_MINUS", "TOKEN_MULTIPLY", "TOKEN_DIVIDE",
  "TOKEN_MODULUS", "TOKEN_NEG", "TOKEN_NOT", "TOKEN_COMPLEMENT",
  "TOKEN_STRING", "TOKEN_INTEGER", "TOKEN_FLOAT", "TOKEN_LISTENER",
  "TOKEN_NIL", "TOKEN_NULL", "TOKEN_DOUBLE_COLON", "TOKEN_SEMICOLON",
  "TOKEN_DOLLAR", "TOKEN_NUMBER", "TOKEN_INCREMENT", "TOKEN_DECREMENT",
  "TOKEN_PERIOD", "TOKEN_CATCH", "TOKEN_TRY", "TOKEN_SWITCH", "TOKEN_CASE",
  "TOKEN_BREAK", "TOKEN_CONTINUE", "TOKEN_SIZE", "TOKEN_END",
  "TOKEN_RETURN", "TOKEN_MAKEARRAY", "TOKEN_ENDARRAY", "$accept",
  "program", "statement_list", "statement", "statement_declaration",
  "compound_statement", "selection_statement", "iteration_statement",
  "expr", "func_prim_expr", "event_parameter_list",
  "event_parameter_list_need", "event_parameter", "const_array_list",
  "const_array", "prim_expr", "identifier_prim", "identifier",
  "nonident_prim_expr", "func_expr", "makearray_statement_list",
  "makearray_statement", "line_opt", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-98)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-3)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      15,   -98,    38,    39,   -98,   449,   -98,   -98,   449,   608,
     608,     4,    15,   608,    15,   158,   -98,   170,   -98,   -98,
     -98,   -98,   -98,   -98,   -98,   -98,   608,    53,   608,   608,
     -98,   -98,    15,   -98,   -98,   -98,   580,   170,    53,   -98,
      12,   -98,    14,    12,    10,    50,    36,   608,     1,    29,
     382,   158,   560,   479,   231,   315,   -98,    18,   479,    20,
       1,    26,    16,   581,   -98,   608,   158,   158,   158,   158,
     158,   158,   158,   158,   158,   158,   158,   158,   -98,   -98,
       2,    14,    15,   608,    89,   -98,   158,    28,   608,   -98,
       1,   -98,   -98,   770,   608,   -98,   608,   560,   -98,    46,
     231,     1,   432,   509,   -98,   158,   158,   158,   158,   158,
     158,   158,   158,   158,   158,   158,   158,   158,   158,   158,
     158,   158,   158,   158,   370,   608,   608,   -98,    18,    53,
     -98,    75,   -98,   670,   770,   770,   770,   770,   770,   770,
     770,   770,   770,   770,   770,   -98,   -98,   -98,   -98,   -98,
      49,   -98,   -98,    14,    15,   690,   158,     1,    49,   608,
     -98,   533,   -98,     1,   750,   787,   431,   802,   816,   295,
     650,   650,   603,   603,   603,   603,    91,    91,    65,    65,
     -98,   -98,   -98,   638,   -98,    49,   -98,   -98,   -98,   608,
     -98,    15,   719,   -98,   158,   -98,   -98,    56,    15,   770,
      15,    68,   -98,    15,   -98
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
     108,   109,     0,   108,     4,     3,     1,     5,     0,     0,
       0,     0,   108,    71,   108,     0,   100,     0,   101,    90,
      91,    92,    94,    99,    98,    30,     0,     0,     0,     0,
      13,    14,   108,     9,    10,    11,     0,     0,     0,    83,
     108,    81,    80,   108,   108,     0,    15,    72,    74,   108,
       0,     0,    63,     0,   102,     0,    62,    61,     0,    97,
      86,     0,     0,    71,     6,    71,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    28,    29,
       0,    96,   108,     0,    34,    37,     0,     0,     0,     7,
      75,    31,    32,    60,     0,    64,    73,     0,    67,    97,
     102,   106,     0,     0,    95,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    71,     0,    66,    96,     0,
      36,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    84,    85,    88,    87,    33,
      82,    76,    79,    78,   108,     0,     0,    40,    68,   105,
      70,     0,   104,   107,     0,    42,    41,    44,    45,    43,
      46,    47,    48,    50,    49,    51,    57,    58,    52,    53,
      54,    55,    56,     0,    65,    69,    12,     8,    89,     0,
      35,   108,     0,   103,     0,    93,    77,   108,   108,    59,
     108,   108,    39,   108,    38
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -98,   -98,   -13,    -3,   -98,   -15,   -98,   -98,   189,    22,
     -61,   -98,   -20,   -58,   -74,    93,   -73,   -98,    -2,    80,
      25,   -97,    17
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     2,     3,     4,    32,    33,    34,    35,    55,    56,
      46,    95,    47,   150,   151,    48,    41,   148,    57,    58,
     102,   103,     8
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
       7,    49,   131,    36,   132,   161,    36,    42,    42,    45,
     152,    42,    61,   145,     1,    59,     1,     5,    44,     1,
       1,   152,    38,    82,    42,    38,    42,    42,    14,   125,
      66,    50,    96,     1,    66,    81,   158,    84,     6,    -2,
      85,    87,    91,     1,    38,    42,     7,   130,    36,    64,
      42,    99,    42,   152,   146,    38,   128,     1,    88,    83,
       1,    42,   161,    42,   184,    14,    89,    38,   185,    86,
      83,   200,     1,   147,    83,    98,   126,    96,    80,    38,
     127,   153,    80,   203,    80,    37,    42,   156,    37,    37,
      37,   129,   153,    37,    42,    42,   154,    37,    42,   149,
      42,    42,    40,    43,   126,   187,    37,   189,    37,    37,
      80,   121,   122,   123,   186,   196,   152,    37,     0,    60,
       0,    62,    63,    42,   153,   159,     0,    37,     0,     0,
      37,     0,    37,     0,    37,   119,   120,   121,   122,   123,
      90,     0,     0,    37,     0,    37,    38,   101,     0,     0,
       0,   190,     0,     0,     0,     0,     0,    42,     0,    42,
       0,     0,    51,    37,     0,     0,     0,     0,    37,    52,
       0,     0,    15,     0,    37,     0,    37,    37,   197,     0,
      37,   157,    37,    37,    15,   201,     0,   153,     0,    90,
       0,     0,     0,   101,     7,   101,   163,   202,     7,     0,
     204,     0,     0,     0,     0,    37,    37,    16,    53,    18,
      19,    20,    21,    22,    23,    24,     0,     0,    26,    16,
      17,    18,    19,    20,    21,    22,    23,    24,     0,     0,
      26,     0,    54,     0,     0,   100,     0,     0,     0,    37,
      93,    37,    39,     0,   124,    15,     0,     0,     0,     0,
       0,     0,   101,     0,   163,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,     0,     0,    37,
       0,     0,     0,     0,     0,   155,     0,     0,     0,     0,
      16,    17,    18,    19,    20,    21,    22,    23,    24,     0,
       0,    26,     0,     0,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,     0,     0,     0,     0,     0,    51,
       0,     0,     0,     0,     0,     0,    52,     0,     0,    15,
     104,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   105,   192,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,    16,    53,    18,    19,    20,    21,
      22,    23,    24,     0,    51,    26,     0,     0,     0,     0,
       0,    52,     0,   199,    15,     0,     1,     0,     9,    54,
      10,    11,    12,    13,    14,    92,    15,     0,     0,   105,
       0,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,    16,
      53,    18,    19,    20,    21,    22,    23,    24,     0,     0,
      26,    16,    17,    18,    19,    20,    21,    22,    23,    24,
       0,    25,    26,    39,    54,     0,    15,     0,    27,    28,
      29,    30,    31,     1,     0,     9,     0,    10,    11,    12,
      13,    14,     0,    15,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
       0,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      97,     0,    26,    15,     0,     0,     0,     0,    16,    17,
      18,    19,    20,    21,    22,    23,    24,   160,    25,    26,
       0,     0,     0,   162,     0,    27,    28,    29,    30,    31,
      39,     0,     0,    15,     0,     0,     0,     0,    16,    53,
      18,    19,    20,    21,    22,    23,    24,   193,     0,    26,
       0,     0,     0,     0,    39,     0,     0,    15,     0,     0,
       0,     0,     0,    54,     0,     0,     0,     0,    16,    17,
      18,    19,    20,    21,    22,    23,    24,     0,     0,    26,
       0,    39,     0,     0,    15,     0,     0,     0,     0,     0,
       0,     0,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    65,    39,    26,     0,    15,    66,     0,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    94,    39,
      26,     0,    15,     0,     0,     0,     0,     0,     0,     0,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    83,
       0,    26,    78,    79,    80,   117,   118,   119,   120,   121,
     122,   123,     0,   195,     0,     0,     0,    16,    17,    18,
      19,    20,    21,    22,    23,    24,     0,   105,    26,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   188,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   105,
       0,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   105,
       0,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   105,   191,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   198,   105,
     194,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   105,
       0,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123
};

static const yytype_int16 yycheck[] =
{
       3,    14,    63,     5,    65,   102,     8,     9,    10,    12,
      83,    13,    27,    11,     4,    17,     4,     0,    14,     4,
       4,    94,     5,    38,    26,     8,    28,    29,    12,    11,
      16,    14,    52,     4,    16,    37,    94,    40,     0,     0,
      43,    44,    13,     4,    27,    47,    49,    62,    50,    32,
      52,    53,    54,   126,    52,    38,    58,     4,     8,    58,
       4,    63,   159,    65,   125,    12,    30,    50,   126,    59,
      58,    15,     4,    71,    58,    53,    58,    97,    64,    62,
      58,    83,    64,    15,    64,     5,    88,    59,     8,     9,
      10,    65,    94,    13,    96,    97,     7,    17,   100,    82,
     102,   103,     9,    10,    58,    30,    26,    58,    28,    29,
      64,    46,    47,    48,   129,   189,   189,    37,    -1,    26,
      -1,    28,    29,   125,   126,   100,    -1,    47,    -1,    -1,
      50,    -1,    52,    -1,    54,    44,    45,    46,    47,    48,
      47,    -1,    -1,    63,    -1,    65,   129,    54,    -1,    -1,
      -1,   154,    -1,    -1,    -1,    -1,    -1,   159,    -1,   161,
      -1,    -1,     4,    83,    -1,    -1,    -1,    -1,    88,    11,
      -1,    -1,    14,    -1,    94,    -1,    96,    97,   191,    -1,
     100,    88,   102,   103,    14,   198,    -1,   189,    -1,    96,
      -1,    -1,    -1,   100,   197,   102,   103,   200,   201,    -1,
     203,    -1,    -1,    -1,    -1,   125,   126,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    -1,    -1,    60,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    -1,    -1,
      60,    -1,    74,    -1,    -1,     4,    -1,    -1,    -1,   159,
      51,   161,    11,    -1,    55,    14,    -1,    -1,    -1,    -1,
      -1,    -1,   159,    -1,   161,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,   189,
      -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    -1,
      -1,    60,    -1,    -1,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,    -1,    -1,    -1,    -1,    -1,     4,
      -1,    -1,    -1,    -1,    -1,    -1,    11,    -1,    -1,    14,
      15,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    29,   156,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    -1,     4,    60,    -1,    -1,    -1,    -1,
      -1,    11,    -1,   194,    14,    -1,     4,    -1,     6,    74,
       8,     9,    10,    11,    12,    13,    14,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    -1,    -1,
      60,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      -1,    59,    60,    11,    74,    -1,    14,    -1,    66,    67,
      68,    69,    70,     4,    -1,     6,    -1,     8,     9,    10,
      11,    12,    -1,    14,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      11,    -1,    60,    14,    -1,    -1,    -1,    -1,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    75,    59,    60,
      -1,    -1,    -1,     4,    -1,    66,    67,    68,    69,    70,
      11,    -1,    -1,    14,    -1,    -1,    -1,    -1,    49,    50,
      51,    52,    53,    54,    55,    56,    57,     4,    -1,    60,
      -1,    -1,    -1,    -1,    11,    -1,    -1,    14,    -1,    -1,
      -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    -1,    -1,    60,
      -1,    11,    -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    11,    11,    60,    -1,    14,    16,    -1,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    11,
      60,    -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      -1,    60,    62,    63,    64,    42,    43,    44,    45,    46,
      47,    48,    -1,    15,    -1,    -1,    -1,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    -1,    29,    60,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    17,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    59,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     4,    77,    78,    79,    98,     0,    79,    98,     6,
       8,     9,    10,    11,    12,    14,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    59,    60,    66,    67,    68,
      69,    70,    80,    81,    82,    83,    94,    95,    98,    11,
      91,    92,    94,    91,    14,    79,    86,    88,    91,    78,
      98,     4,    11,    50,    74,    84,    85,    94,    95,    94,
      91,    81,    91,    91,    98,    11,    16,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    62,    63,
      64,    94,    81,    58,    79,    79,    59,    79,     8,    30,
      91,    13,    13,    84,    58,    87,    88,    11,    85,    94,
       4,    91,    96,    97,    15,    29,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    84,    11,    58,    85,    94,    65,
      81,    86,    86,    84,    84,    84,    84,    84,    84,    84,
      84,    84,    84,    84,    84,    11,    52,    71,    93,    98,
      89,    90,    92,    94,     7,    84,    59,    91,    89,    96,
      75,    97,     4,    91,    84,    84,    84,    84,    84,    84,
      84,    84,    84,    84,    84,    84,    84,    84,    84,    84,
      84,    84,    84,    84,    86,    89,    81,    30,    17,    58,
      79,    59,    84,     4,    30,    15,    90,    78,    59,    84,
      15,    78,    79,    15,    79
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    76,    77,    77,    78,    78,    79,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    81,    81,    81,    82,    82,    82,    83,    83,    83,
      83,    84,    84,    84,    84,    84,    84,    84,    84,    84,
      84,    84,    84,    84,    84,    84,    84,    84,    84,    84,
      84,    84,    84,    84,    85,    85,    85,    85,    85,    85,
      85,    86,    86,    87,    88,    88,    89,    89,    90,    90,
      91,    91,    91,    92,    93,    93,    94,    94,    94,    94,
      94,    94,    94,    94,    94,    94,    94,    94,    94,    94,
      95,    95,    96,    96,    96,    96,    97,    97,    98,    98
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     1,     2,     3,     3,     4,     1,
       1,     1,     4,     1,     1,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       1,     3,     3,     3,     3,     5,     3,     3,     9,     8,
       4,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     5,
       2,     1,     1,     1,     2,     3,     2,     2,     3,     3,
       3,     0,     1,     1,     1,     2,     1,     3,     1,     1,
       1,     1,     3,     1,     1,     1,     2,     3,     3,     4,
       1,     1,     1,     5,     1,     3,     2,     2,     1,     1,
       1,     1,     0,     3,     2,     2,     1,     2,     0,     1
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

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


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


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
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
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp);
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
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
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]));
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
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
  YYLTYPE *yylloc;
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
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
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
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
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

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  yylsp[0] = yylloc;
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
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
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
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

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

  if (yychar <= TOKEN_EOF)
    {
      yychar = TOKEN_EOF;
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
      yyerror_range[1] = yylloc;
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
  *++yylsp = yylloc;

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

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* program: statement_list  */
#line 130 "/usr/src/openmohaa/code/parser/bison_source.txt"
                               { parsedata.val = node1(ENUM_statement_list, (yyvsp[0].s.val)); }
#line 1875 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 3: /* program: line_opt  */
#line 131 "/usr/src/openmohaa/code/parser/bison_source.txt"
                   { parsedata.val = node0(ENUM_NOP); }
#line 1881 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 4: /* statement_list: statement  */
#line 135 "/usr/src/openmohaa/code/parser/bison_source.txt"
                    { (yyval.s.val) = linked_list_end((yyvsp[0].s.val)); }
#line 1887 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 5: /* statement_list: statement_list statement  */
#line 136 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                         { (yyval.s.val) = append_node((yyvsp[-1].s.val), (yyvsp[0].s.val)); }
#line 1893 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 6: /* statement: line_opt statement_declaration line_opt  */
#line 140 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                             { (yyval.s.val) = (yyvsp[-1].s.val); }
#line 1899 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 7: /* statement_declaration: TOKEN_IDENTIFIER event_parameter_list TOKEN_COLON  */
#line 144 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                            { (yyval.s.val) = node3(ENUM_labeled_statement, (yyvsp[-2].s.val), (yyvsp[-1].s.val), TOKPOS((yylsp[-2]))); }
#line 1905 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 8: /* statement_declaration: TOKEN_CASE prim_expr event_parameter_list TOKEN_COLON  */
#line 145 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                                { (yyval.s.val) = node3(ENUM_int_labeled_statement, (yyvsp[-2].s.val), (yyvsp[-1].s.val), TOKPOS((yylsp[-3]))); }
#line 1911 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 12: /* statement_declaration: TOKEN_TRY compound_statement TOKEN_CATCH compound_statement  */
#line 149 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                                              { (yyval.s.val) = node3(ENUM_try, (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-3]))); }
#line 1917 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 13: /* statement_declaration: TOKEN_BREAK  */
#line 150 "/usr/src/openmohaa/code/parser/bison_source.txt"
                      { (yyval.s.val) = node1(ENUM_break, TOKPOS((yylsp[0]))); }
#line 1923 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 14: /* statement_declaration: TOKEN_CONTINUE  */
#line 151 "/usr/src/openmohaa/code/parser/bison_source.txt"
                         { (yyval.s.val) = node1(ENUM_continue, TOKPOS((yylsp[0]))); }
#line 1929 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 15: /* statement_declaration: TOKEN_IDENTIFIER event_parameter_list  */
#line 152 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                { (yyval.s.val) = node3(ENUM_cmd_event_statement, (yyvsp[-1].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1]))); }
#line 1935 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 16: /* statement_declaration: nonident_prim_expr TOKEN_IDENTIFIER event_parameter_list  */
#line 153 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                                   { (yyval.s.val) = node4(ENUM_method_event_statement, (yyvsp[-2].s.val), (yyvsp[-1].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1]))); }
#line 1941 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 17: /* statement_declaration: nonident_prim_expr TOKEN_ASSIGNMENT expr  */
#line 154 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                   { (yyval.s.val) = node3(ENUM_assignment_statement, (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1]))); }
#line 1947 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 18: /* statement_declaration: nonident_prim_expr TOKEN_PLUS_EQUALS expr  */
#line 155 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                    { (yyval.s.val) = node3(ENUM_assignment_statement, (yyvsp[-2].s.val), node4(ENUM_func2_expr, node1b(OP_BIN_PLUS), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1]))), TOKPOS((yylsp[-1]))); }
#line 1953 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 19: /* statement_declaration: nonident_prim_expr TOKEN_MINUS_EQUALS expr  */
#line 156 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                     { (yyval.s.val) = node3(ENUM_assignment_statement, (yyvsp[-2].s.val), node4(ENUM_func2_expr, node1b(OP_BIN_MINUS), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1]))), TOKPOS((yylsp[-1]))); }
#line 1959 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 20: /* statement_declaration: nonident_prim_expr TOKEN_MULTIPLY_EQUALS expr  */
#line 157 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                        { (yyval.s.val) = node3(ENUM_assignment_statement, (yyvsp[-2].s.val), node4(ENUM_func2_expr, node1b(OP_BIN_MULTIPLY), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1]))), TOKPOS((yylsp[-1]))); }
#line 1965 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 21: /* statement_declaration: nonident_prim_expr TOKEN_DIVIDE_EQUALS expr  */
#line 158 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                      { (yyval.s.val) = node3(ENUM_assignment_statement, (yyvsp[-2].s.val), node4(ENUM_func2_expr, node1b(OP_BIN_DIVIDE), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1]))), TOKPOS((yylsp[-1]))); }
#line 1971 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 22: /* statement_declaration: nonident_prim_expr TOKEN_MODULUS_EQUALS expr  */
#line 159 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                       { (yyval.s.val) = node3(ENUM_assignment_statement, (yyvsp[-2].s.val), node4(ENUM_func2_expr, node1b(OP_BIN_PERCENTAGE), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1]))), TOKPOS((yylsp[-1]))); }
#line 1977 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 23: /* statement_declaration: nonident_prim_expr TOKEN_AND_EQUALS expr  */
#line 160 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                   { (yyval.s.val) = node3(ENUM_assignment_statement, (yyvsp[-2].s.val), node4(ENUM_func2_expr, node1b(OP_BIN_BITWISE_AND), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1]))), TOKPOS((yylsp[-1]))); }
#line 1983 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 24: /* statement_declaration: nonident_prim_expr TOKEN_EXCL_OR_EQUALS expr  */
#line 161 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                       { (yyval.s.val) = node3(ENUM_assignment_statement, (yyvsp[-2].s.val), node4(ENUM_func2_expr, node1b(OP_BIN_BITWISE_EXCL_OR), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1]))), TOKPOS((yylsp[-1]))); }
#line 1989 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 25: /* statement_declaration: nonident_prim_expr TOKEN_OR_EQUALS expr  */
#line 162 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                  { (yyval.s.val) = node3(ENUM_assignment_statement, (yyvsp[-2].s.val), node4(ENUM_func2_expr, node1b(OP_BIN_BITWISE_OR), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1]))), TOKPOS((yylsp[-1]))); }
#line 1995 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 26: /* statement_declaration: nonident_prim_expr TOKEN_SHIFT_LEFT_EQUALS expr  */
#line 163 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                          { (yyval.s.val) = node3(ENUM_assignment_statement, (yyvsp[-2].s.val), node4(ENUM_func2_expr, node1b(OP_BIN_SHIFT_LEFT), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1]))), TOKPOS((yylsp[-1]))); }
#line 2001 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 27: /* statement_declaration: nonident_prim_expr TOKEN_SHIFT_RIGHT_EQUALS expr  */
#line 164 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                           { (yyval.s.val) = node3(ENUM_assignment_statement, (yyvsp[-2].s.val), node4(ENUM_func2_expr, node1b(OP_BIN_SHIFT_RIGHT), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1]))), TOKPOS((yylsp[-1]))); }
#line 2007 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 28: /* statement_declaration: nonident_prim_expr TOKEN_INCREMENT  */
#line 165 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                             { (yyval.s.val) = node3(ENUM_assignment_statement, (yyvsp[-1].s.val), node3(ENUM_func1_expr, node1b(OP_UN_INC), (yyvsp[-1].s.val), TOKPOS((yylsp[0]))), TOKPOS((yylsp[0]))); }
#line 2013 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 29: /* statement_declaration: nonident_prim_expr TOKEN_DECREMENT  */
#line 166 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                             { (yyval.s.val) = node3(ENUM_assignment_statement, (yyvsp[-1].s.val), node3(ENUM_func1_expr, node1b(OP_UN_DEC), (yyvsp[-1].s.val), TOKPOS((yylsp[0]))), TOKPOS((yylsp[0]))); }
#line 2019 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 30: /* statement_declaration: TOKEN_SEMICOLON  */
#line 167 "/usr/src/openmohaa/code/parser/bison_source.txt"
                          { (yyval.s.val) = node0(ENUM_NOP); }
#line 2025 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 31: /* compound_statement: TOKEN_LEFT_BRACES statement_list TOKEN_RIGHT_BRACES  */
#line 173 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                              { (yyval.s.val) = node1(ENUM_statement_list, (yyvsp[-1].s.val)); }
#line 2031 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 32: /* compound_statement: TOKEN_LEFT_BRACES line_opt TOKEN_RIGHT_BRACES  */
#line 174 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                        { (yyval.s.val) = node0(ENUM_NOP); }
#line 2037 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 33: /* compound_statement: line_opt compound_statement line_opt  */
#line 175 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                          { (yyval.s.val) = (yyvsp[-1].s.val); }
#line 2043 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 34: /* selection_statement: TOKEN_IF prim_expr statement  */
#line 179 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                  { (yyval.s.val) = node3(ENUM_if_statement, (yyvsp[-1].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-2]))); }
#line 2049 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 35: /* selection_statement: TOKEN_IF prim_expr statement TOKEN_ELSE statement  */
#line 180 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                                                     { (yyval.s.val) = node4(ENUM_if_else_statement, (yyvsp[-3].s.val), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-4]))); }
#line 2055 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 36: /* selection_statement: TOKEN_SWITCH prim_expr compound_statement  */
#line 181 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                                    { (yyval.s.val) = node3(ENUM_switch, (yyvsp[-1].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-2]))); }
#line 2061 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 37: /* iteration_statement: TOKEN_WHILE prim_expr statement  */
#line 185 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                    { (yyval.s.val) = node4(ENUM_while_statement, (yyvsp[-1].s.val), (yyvsp[0].s.val), node0(ENUM_NOP), TOKPOS((yylsp[-2]))); }
#line 2067 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 38: /* iteration_statement: TOKEN_FOR TOKEN_LEFT_BRACKET statement TOKEN_SEMICOLON expr TOKEN_SEMICOLON statement_list TOKEN_RIGHT_BRACKET statement  */
#line 187 "/usr/src/openmohaa/code/parser/bison_source.txt"
        {
		sval_t while_stmt = node4(ENUM_while_statement, (yyvsp[-4].s.val), (yyvsp[0].s.val), node1(ENUM_statement_list, (yyvsp[-2].s.val)), TOKPOS((yylsp[-8])));
		(yyval.s.val) = node1(ENUM_statement_list, append_node(linked_list_end((yyvsp[-6].s.val)), while_stmt));
	}
#line 2076 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 39: /* iteration_statement: TOKEN_FOR TOKEN_LEFT_BRACKET TOKEN_SEMICOLON expr TOKEN_SEMICOLON statement_list TOKEN_RIGHT_BRACKET statement  */
#line 192 "/usr/src/openmohaa/code/parser/bison_source.txt"
        {
		(yyval.s.val) = node4(ENUM_while_statement, (yyvsp[-4].s.val), (yyvsp[0].s.val), node1(ENUM_statement_list, (yyvsp[-2].s.val)), TOKPOS((yylsp[-7])));
	}
#line 2084 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 40: /* iteration_statement: TOKEN_DO statement TOKEN_WHILE prim_expr  */
#line 195 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                             { (yyval.s.val) = node3(ENUM_do, (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-3]))); }
#line 2090 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 41: /* expr: expr TOKEN_LOGICAL_AND expr  */
#line 199 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                    { (yyval.s.val) = node3( ENUM_logical_and, (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1])) ); }
#line 2096 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 42: /* expr: expr TOKEN_LOGICAL_OR expr  */
#line 200 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                     { (yyval.s.val) = node3( ENUM_logical_or, (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1])) ); }
#line 2102 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 43: /* expr: expr TOKEN_BITWISE_AND expr  */
#line 201 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                      { (yyval.s.val) = node4( ENUM_func2_expr, node1b( OP_BIN_BITWISE_AND ), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1])) ); }
#line 2108 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 44: /* expr: expr TOKEN_BITWISE_OR expr  */
#line 202 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                     { (yyval.s.val) = node4( ENUM_func2_expr, node1b( OP_BIN_BITWISE_OR ), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1])) ); }
#line 2114 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 45: /* expr: expr TOKEN_BITWISE_EXCL_OR expr  */
#line 203 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                          { (yyval.s.val) = node4( ENUM_func2_expr, node1b( OP_BIN_BITWISE_EXCL_OR ), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1])) ); }
#line 2120 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 46: /* expr: expr TOKEN_EQUALITY expr  */
#line 204 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                   { (yyval.s.val) = node4( ENUM_func2_expr, node1b( OP_BIN_EQUALITY ), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1])) ); }
#line 2126 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 47: /* expr: expr TOKEN_INEQUALITY expr  */
#line 205 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                     { (yyval.s.val) = node4( ENUM_func2_expr, node1b( OP_BIN_INEQUALITY ), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1])) ); }
#line 2132 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 48: /* expr: expr TOKEN_LESS_THAN expr  */
#line 206 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                    { (yyval.s.val) = node4( ENUM_func2_expr, node1b( OP_BIN_LESS_THAN ), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1])) ); }
#line 2138 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 49: /* expr: expr TOKEN_GREATER_THAN expr  */
#line 207 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                       { (yyval.s.val) = node4( ENUM_func2_expr, node1b( OP_BIN_GREATER_THAN ), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1])) ); }
#line 2144 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 50: /* expr: expr TOKEN_LESS_THAN_OR_EQUAL expr  */
#line 208 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                             { (yyval.s.val) = node4( ENUM_func2_expr, node1b( OP_BIN_LESS_THAN_OR_EQUAL ), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1])) ); }
#line 2150 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 51: /* expr: expr TOKEN_GREATER_THAN_OR_EQUAL expr  */
#line 209 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                { (yyval.s.val) = node4( ENUM_func2_expr, node1b( OP_BIN_GREATER_THAN_OR_EQUAL ), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1])) ); }
#line 2156 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 52: /* expr: expr TOKEN_PLUS expr  */
#line 210 "/usr/src/openmohaa/code/parser/bison_source.txt"
                               { (yyval.s.val) = node4( ENUM_func2_expr, node1b( OP_BIN_PLUS ), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1])) ); }
#line 2162 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 53: /* expr: expr TOKEN_MINUS expr  */
#line 211 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                { (yyval.s.val) = node4( ENUM_func2_expr, node1b( OP_BIN_MINUS ), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1])) ); }
#line 2168 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 54: /* expr: expr TOKEN_MULTIPLY expr  */
#line 212 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                   { (yyval.s.val) = node4( ENUM_func2_expr, node1b( OP_BIN_MULTIPLY ), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1])) ); }
#line 2174 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 55: /* expr: expr TOKEN_DIVIDE expr  */
#line 213 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                 { (yyval.s.val) = node4( ENUM_func2_expr, node1b( OP_BIN_DIVIDE ), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1])) ); }
#line 2180 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 56: /* expr: expr TOKEN_MODULUS expr  */
#line 214 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                  { (yyval.s.val) = node4( ENUM_func2_expr, node1b( OP_BIN_PERCENTAGE ), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1])) ); }
#line 2186 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 57: /* expr: expr TOKEN_SHIFT_LEFT expr  */
#line 215 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                     { (yyval.s.val) = node4( ENUM_func2_expr, node1b( OP_BIN_SHIFT_LEFT ), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1])) ); }
#line 2192 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 58: /* expr: expr TOKEN_SHIFT_RIGHT expr  */
#line 216 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                      { (yyval.s.val) = node4( ENUM_func2_expr, node1b( OP_BIN_SHIFT_RIGHT ), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1])) ); }
#line 2198 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 59: /* expr: expr TOKEN_TERNARY expr TOKEN_COLON expr  */
#line 217 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                   { (yyval.s.val) = node4( ENUM_if_else_statement, (yyvsp[-4].s.val), (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-3])) ); }
#line 2204 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 60: /* expr: TOKEN_EOL expr  */
#line 218 "/usr/src/openmohaa/code/parser/bison_source.txt"
                              { (yyval.s.val) = (yyvsp[0].s.val); }
#line 2210 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 63: /* expr: TOKEN_IDENTIFIER  */
#line 221 "/usr/src/openmohaa/code/parser/bison_source.txt"
                           { (yyval.s.val) = node2(ENUM_string, (yyvsp[0].s.val), TOKPOS((yylsp[0]))); }
#line 2216 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 64: /* func_prim_expr: TOKEN_IDENTIFIER event_parameter_list_need  */
#line 225 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                   { (yyval.s.val) = node3(ENUM_cmd_event_expr, (yyvsp[-1].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1]))); }
#line 2222 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 65: /* func_prim_expr: nonident_prim_expr TOKEN_IDENTIFIER event_parameter_list  */
#line 226 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                                   { (yyval.s.val) = node4(ENUM_method_event_expr, (yyvsp[-2].s.val), (yyvsp[-1].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1]))); }
#line 2228 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 66: /* func_prim_expr: func_expr func_prim_expr  */
#line 227 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                   { (yyval.s.val) = node3(ENUM_func1_expr, (yyvsp[-1].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1]))); }
#line 2234 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 67: /* func_prim_expr: TOKEN_NOT func_prim_expr  */
#line 228 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                   { (yyval.s.val) = node2(ENUM_bool_not, (yyvsp[0].s.val), TOKPOS((yylsp[-1]))); }
#line 2240 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 68: /* func_prim_expr: TOKEN_IDENTIFIER TOKEN_DOUBLE_COLON const_array_list  */
#line 230 "/usr/src/openmohaa/code/parser/bison_source.txt"
                {
			(yyval.s.val) = node3(ENUM_const_array_expr, node2(ENUM_string, (yyvsp[-2].s.val), TOKPOS((yylsp[-2]))), (yyvsp[0].s.val), TOKPOS((yylsp[-1])));
		}
#line 2248 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 69: /* func_prim_expr: nonident_prim_expr TOKEN_DOUBLE_COLON const_array_list  */
#line 234 "/usr/src/openmohaa/code/parser/bison_source.txt"
                {
			(yyval.s.val) = node3(ENUM_const_array_expr, (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1])));
		}
#line 2256 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 70: /* func_prim_expr: TOKEN_MAKEARRAY makearray_statement_list TOKEN_ENDARRAY  */
#line 238 "/usr/src/openmohaa/code/parser/bison_source.txt"
                {
			(yyval.s.val) = node2(ENUM_makearray, (yyvsp[-1].s.val), TOKPOS((yylsp[-2])));
		}
#line 2264 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 71: /* event_parameter_list: %empty  */
#line 244 "/usr/src/openmohaa/code/parser/bison_source.txt"
          { (yyval.s.val) = sval_u{}; (yyval.s.val).node = NULL; }
#line 2270 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 72: /* event_parameter_list: event_parameter  */
#line 245 "/usr/src/openmohaa/code/parser/bison_source.txt"
                          { (yyval.s.val) = (yyvsp[0].s.val); }
#line 2276 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 73: /* event_parameter_list_need: event_parameter  */
#line 249 "/usr/src/openmohaa/code/parser/bison_source.txt"
                          { (yyval.s.val) = (yyvsp[0].s.val); }
#line 2282 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 74: /* event_parameter: prim_expr  */
#line 253 "/usr/src/openmohaa/code/parser/bison_source.txt"
                    { (yyval.s.val) = linked_list_end((yyvsp[0].s.val)); }
#line 2288 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 75: /* event_parameter: event_parameter prim_expr  */
#line 254 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                    { (yyval.s.val) = append_node((yyvsp[-1].s.val), (yyvsp[0].s.val)); }
#line 2294 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 76: /* const_array_list: const_array  */
#line 258 "/usr/src/openmohaa/code/parser/bison_source.txt"
                      { (yyval.s.val) = linked_list_end((yyvsp[0].s.val)); }
#line 2300 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 77: /* const_array_list: const_array_list TOKEN_DOUBLE_COLON const_array  */
#line 259 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                          { (yyval.s.val) = append_node((yyvsp[-2].s.val), (yyvsp[0].s.val)); }
#line 2306 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 79: /* const_array: identifier_prim  */
#line 264 "/usr/src/openmohaa/code/parser/bison_source.txt"
                          { (yyval.s.val) = node2(ENUM_string, (yyvsp[0].s.val), TOKPOS((yylsp[0]))); }
#line 2312 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 81: /* prim_expr: identifier_prim  */
#line 269 "/usr/src/openmohaa/code/parser/bison_source.txt"
                          { (yyval.s.val) = node2(ENUM_string, (yyvsp[0].s.val), TOKPOS((yylsp[0]))); }
#line 2318 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 82: /* prim_expr: prim_expr TOKEN_DOUBLE_COLON const_array_list  */
#line 270 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                        { (yyval.s.val) = node3(ENUM_const_array_expr, (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1]))); }
#line 2324 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 83: /* identifier_prim: TOKEN_IDENTIFIER  */
#line 274 "/usr/src/openmohaa/code/parser/bison_source.txt"
                         { (yyval.s.val) = (yyvsp[0].s.val); (yyloc) = (yylsp[0]); }
#line 2330 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 84: /* identifier: TOKEN_IDENTIFIER  */
#line 278 "/usr/src/openmohaa/code/parser/bison_source.txt"
                           { (yyval.s.val) = (yyvsp[0].s.val); (yyloc) = (yylsp[0]); }
#line 2336 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 85: /* identifier: TOKEN_STRING  */
#line 279 "/usr/src/openmohaa/code/parser/bison_source.txt"
                       { (yyval.s.val) = (yyvsp[0].s.val); (yyloc) = (yylsp[0]); }
#line 2342 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 86: /* nonident_prim_expr: TOKEN_DOLLAR prim_expr  */
#line 283 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                 { (yyval.s.val) = node3(ENUM_func1_expr, node1b(OP_UN_TARGETNAME), (yyvsp[0].s.val), TOKPOS((yylsp[-1]))); }
#line 2348 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 87: /* nonident_prim_expr: nonident_prim_expr TOKEN_PERIOD identifier  */
#line 284 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                     { (yyval.s.val) = node3(ENUM_field, (yyvsp[-2].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[0]))); }
#line 2354 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 88: /* nonident_prim_expr: nonident_prim_expr TOKEN_PERIOD TOKEN_SIZE  */
#line 285 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                     { (yyval.s.val) = node3(ENUM_func1_expr, node1b(OP_UN_SIZE), (yyvsp[-2].s.val), TOKPOS((yylsp[0]))); }
#line 2360 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 89: /* nonident_prim_expr: nonident_prim_expr TOKEN_LEFT_SQUARE_BRACKET expr TOKEN_RIGHT_SQUARE_BRACKET  */
#line 286 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                                                       { (yyval.s.val) = node3(ENUM_array_expr, (yyvsp[-3].s.val), (yyvsp[-1].s.val), TOKPOS((yylsp[-2]))); }
#line 2366 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 90: /* nonident_prim_expr: TOKEN_STRING  */
#line 287 "/usr/src/openmohaa/code/parser/bison_source.txt"
                       { (yyval.s.val) = node2(ENUM_string, (yyvsp[0].s.val), TOKPOS((yylsp[0]))); }
#line 2372 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 91: /* nonident_prim_expr: TOKEN_INTEGER  */
#line 288 "/usr/src/openmohaa/code/parser/bison_source.txt"
                        { (yyval.s.val) = node2(ENUM_integer, (yyvsp[0].s.val), TOKPOS((yylsp[0]))); }
#line 2378 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 92: /* nonident_prim_expr: TOKEN_FLOAT  */
#line 289 "/usr/src/openmohaa/code/parser/bison_source.txt"
                      { (yyval.s.val) = node2(ENUM_float, (yyvsp[0].s.val), TOKPOS((yylsp[0]))); }
#line 2384 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 93: /* nonident_prim_expr: TOKEN_LEFT_BRACKET expr expr expr TOKEN_RIGHT_BRACKET  */
#line 290 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                                                  { (yyval.s.val) = node4(ENUM_vector, (yyvsp[-3].s.val), (yyvsp[-2].s.val), (yyvsp[-1].s.val), TOKPOS((yylsp[-4]))); }
#line 2390 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 94: /* nonident_prim_expr: TOKEN_LISTENER  */
#line 291 "/usr/src/openmohaa/code/parser/bison_source.txt"
                         { (yyval.s.val) = node2(ENUM_listener, (yyvsp[0].s.val), TOKPOS((yylsp[0]))); }
#line 2396 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 95: /* nonident_prim_expr: TOKEN_LEFT_BRACKET expr TOKEN_RIGHT_BRACKET  */
#line 292 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                      { (yyval.s.val) = (yyvsp[-1].s.val); }
#line 2402 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 96: /* nonident_prim_expr: func_expr nonident_prim_expr  */
#line 293 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                       { (yyval.s.val) = node3(ENUM_func1_expr, (yyvsp[-1].s.val), (yyvsp[0].s.val), TOKPOS((yylsp[-1]))); }
#line 2408 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 97: /* nonident_prim_expr: TOKEN_NOT nonident_prim_expr  */
#line 294 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                       { (yyval.s.val) = node2(ENUM_bool_not, (yyvsp[0].s.val), TOKPOS((yylsp[-1]))); }
#line 2414 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 98: /* nonident_prim_expr: TOKEN_NULL  */
#line 295 "/usr/src/openmohaa/code/parser/bison_source.txt"
                     { (yyval.s.val) = node1(ENUM_NULL, TOKPOS((yylsp[0]))); }
#line 2420 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 99: /* nonident_prim_expr: TOKEN_NIL  */
#line 296 "/usr/src/openmohaa/code/parser/bison_source.txt"
                    { (yyval.s.val) = node1(ENUM_NIL, TOKPOS((yylsp[0]))); }
#line 2426 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 100: /* func_expr: TOKEN_NEG  */
#line 300 "/usr/src/openmohaa/code/parser/bison_source.txt"
                    { (yyval.s.val) = node1b(OP_UN_MINUS); }
#line 2432 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 101: /* func_expr: TOKEN_COMPLEMENT  */
#line 301 "/usr/src/openmohaa/code/parser/bison_source.txt"
                           { (yyval.s.val) = node1b(OP_UN_COMPLEMENT); }
#line 2438 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 102: /* makearray_statement_list: %empty  */
#line 305 "/usr/src/openmohaa/code/parser/bison_source.txt"
        { (yyval.s.val) = node0(ENUM_NOP); }
#line 2444 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 103: /* makearray_statement_list: makearray_statement_list makearray_statement TOKEN_EOL  */
#line 306 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                                                { (yyval.s.val) = append_node((yyvsp[-2].s.val), node2(ENUM_makearray, (yyvsp[-1].s.val), TOKPOS((yylsp[-1])))); }
#line 2450 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 104: /* makearray_statement_list: makearray_statement TOKEN_EOL  */
#line 307 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                                 { (yyval.s.val) = linked_list_end(node2(ENUM_makearray, (yyvsp[-1].s.val), TOKPOS((yylsp[-1])))); }
#line 2456 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 105: /* makearray_statement_list: TOKEN_EOL makearray_statement_list  */
#line 308 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                             { (yyval.s.val) = (yyvsp[0].s.val); (yyloc) = (yylsp[0]); }
#line 2462 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 106: /* makearray_statement: prim_expr  */
#line 312 "/usr/src/openmohaa/code/parser/bison_source.txt"
                  { (yyval.s.val) = linked_list_end( (yyvsp[0].s.val) ); }
#line 2468 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 107: /* makearray_statement: makearray_statement prim_expr  */
#line 313 "/usr/src/openmohaa/code/parser/bison_source.txt"
                                        { (yyval.s.val) = append_node( (yyvsp[-1].s.val), (yyvsp[0].s.val) ); }
#line 2474 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;

  case 108: /* line_opt: %empty  */
#line 317 "/usr/src/openmohaa/code/parser/bison_source.txt"
          {}
#line 2480 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"
    break;


#line 2484 "/usr/src/openmohaa/code/parser/generated/yyParser.cpp"

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
  *++yylsp = yyloc;

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
          = {yyssp, yytoken, &yylloc};
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

  yyerror_range[1] = yylloc;
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= TOKEN_EOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == TOKEN_EOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc);
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

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

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
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp);
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

#line 321 "/usr/src/openmohaa/code/parser/bison_source.txt"

