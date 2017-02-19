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

#ifndef __PARSETREE_H__
#define __PARSETREE_H__

#if defined ( GAME_DLL )
#define showopcodes g_showopcodes
#elif defined( CGAME_DLL )
#define showopcodes cg_showopcodes
#else
#define showopcodes g_showopcodes
#endif

typedef enum
{
	sval_none,
	sval_next,
	sval_statement_list,
	sval_label,
	sval_case,
	sval_negative,
	sval_assignment,
	sval_if,
	sval_ifelse,
	sval_while,
	sval_and,
	sval_or,
	sval_cmd_method,
	sval_cmd_method_ret,
	sval_cmd,
	sval_cmd_default_ret,
	sval_field,
	sval_store_method,
	sval_store_string,
	sval_store_integer,
	sval_store_float,
	sval_calc_vector,
	sval_store_null,
	sval_store_nil,
	sval_func1,
	sval_operation,
	sval_not,
	sval_array,
	sval_constarray,
	sval_makearray,
	sval_catch,
	sval_switch,
	sval_break,
	sval_continue,
	sval_do,
	sval_privatelabel,
	sval_define
} sval_type_e;

typedef union sval_u {
	int				type;
	char			*stringValue;
	float			floatValue;
	int				intValue;
	char			charValue;
	unsigned char	byteValue;
	unsigned char	*posValue;
	int				MaxVarStackOffset;
	int				HasExternal;
	union sval_u	*node;
	unsigned int	sourcePosValue;
} sval_t;

typedef struct {
	sval_t val;
	unsigned int sourcePos;
} stype_t;

void		parsetree_freeall();
void		parsetree_init();
size_t		parsetree_length();
char		*parsetree_malloc( size_t s );

int			node_compare( void *pe1, void *pe2 );

sval_u		append_lists( sval_u val1, sval_u val2 );
sval_u		append_node( sval_u val1, sval_u val2 );
sval_u		prepend_node( sval_u val1, sval_u val2 );

sval_u		linked_list_end( sval_u val );

sval_u		node1_( int val1 );
sval_u		node1b( int val1 );
sval_u		node_pos( unsigned int pos );
sval_u		node_string( char *text );

sval_u		node0( int type );
sval_u		node1( int type, sval_u val1 );
sval_u		node2( int type, sval_u val1, sval_u val2 );
sval_u		node3( int type, sval_u val1, sval_u val2, sval_u val3 );
sval_u		node4( int type, sval_u val1, sval_u val2, sval_u val3, sval_u val4 );
sval_u		node5( int type, sval_u val1, sval_u val2, sval_u val3, sval_u val4, sval_u val5 );
sval_u		node6( int type, sval_u val1, sval_u val2, sval_u val3, sval_u val4, sval_u val5, sval_u val6 );

struct yyexception {
	int			yylineno;
	str			yytext;
	str			yytoken;

	yyexception() { yylineno = 0; }
};

struct yyparsedata {
	size_t			total_length;

	int				braces_count;
	int				line_count;
	unsigned int	pos;
	sval_t			val;

	char				*sourceBuffer;
	class GameScript	*gameScript;

	yyexception		exc;

	yyparsedata() { total_length = 0, braces_count = 0, line_count = 0, pos = 0; val = sval_t(); sourceBuffer = NULL; gameScript = NULL; }
};

extern yyparsedata parsedata;

#endif