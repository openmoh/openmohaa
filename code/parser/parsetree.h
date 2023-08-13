/*
===========================================================================
Copyright (C) 2008 the OpenMoHAA team

This file is part of OpenMoHAA source code.

OpenMoHAA source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMoHAA source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMoHAA source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// parsetree.h: Abstract Syntax Layer for Lexer/Parser

#pragma once

#include "str.h"

#if defined(GAME_DLL)
#    define showopcodes g_showopcodes
#elif defined(CGAME_DLL)
#    define showopcodes cg_showopcodes
#else
#    define showopcodes g_showopcodes
#endif

typedef enum {
    ENUM_NOP,
    ENUM_ptr,
    ENUM_statement_list,
    ENUM_labeled_statement,
    ENUM_int_labeled_statement,
    ENUM_neg_int_labeled_statement,
    ENUM_assignment_statement,
    ENUM_if_statement,
    ENUM_if_else_statement,
    ENUM_while_statement,
    ENUM_logical_and,
    ENUM_logical_or,
    ENUM_method_event_statement,
    ENUM_method_event_expr,
    ENUM_cmd_event_statement,
    ENUM_cmd_event_expr,
    ENUM_field,
    ENUM_listener,
    ENUM_string,
    ENUM_integer,
    ENUM_float,
    ENUM_vector,
    ENUM_NULL,
    ENUM_NIL,
    ENUM_func1_expr,
    ENUM_func2_expr,
    ENUM_bool_not,
    ENUM_array_expr,
    ENUM_const_array_expr,
    ENUM_makearray,
    ENUM_try,
    ENUM_switch,
    ENUM_break,
    ENUM_continue,
    ENUM_do,
    ENUM_privatelabel,
    ENUM_define
} sval_type_e;

typedef union sval_u {
    int            type;
    const char    *stringValue;
    float          floatValue;
    int            intValue;
    char           charValue;
    unsigned char  byteValue;
    unsigned char *posValue;
    int            MaxVarStackOffset;
    int            HasExternal;
    union sval_u  *node;
    unsigned int   sourcePosValue;
} sval_t;

struct stype_t {
    sval_t       val;
    unsigned int sourcePos;
};

enum parseStage_e {
    PS_TYPE,
    PS_BODY,
    PS_BODY_END
};

void   parsetree_freeall();
void   parsetree_init();
size_t parsetree_length();
char  *parsetree_malloc(size_t s);

sval_u append_lists(sval_u val1, sval_u val2);
sval_u append_node(sval_u val1, sval_u val2);
sval_u prepend_node(sval_u val1, sval_u val2);

sval_u linked_list_end(sval_u val);

sval_u node1_(int val1);
sval_u node1b(int val1);
sval_u node_pos(unsigned int pos);
sval_u node_string(char *text);

sval_u node0(int type);
sval_u node1(int type, sval_u val1);
sval_u node2(int type, sval_u val1, sval_u val2);
sval_u node3(int type, sval_u val1, sval_u val2, sval_u val3);
sval_u node4(int type, sval_u val1, sval_u val2, sval_u val3, sval_u val4);
sval_u node5(int type, sval_u val1, sval_u val2, sval_u val3, sval_u val4, sval_u val5);
sval_u node6(int type, sval_u val1, sval_u val2, sval_u val3, sval_u val4, sval_u val5, sval_u val6);

typedef struct parse_pos_s {
    int sourcePos;
    int first_line;
    int first_column;
    int last_line;
    int last_column;
} parse_pos_t;

struct yyexception {
    int yylineno;
    str yytext;
    str yytoken;

    yyexception() { yylineno = 0; }
};

struct yyparsedata {
    size_t total_length;

    int          braces_count;
    int          line_count;
    unsigned int pos;
    sval_t       val;

    char             *sourceBuffer;
    class GameScript *gameScript;

    yyexception exc;

    yyparsedata()
    {
        total_length = 0, braces_count = 0, line_count = 0, pos = 0;
        val          = sval_t();
        sourceBuffer = NULL;
        gameScript   = NULL;
    }
};

extern yyparsedata parsedata;
