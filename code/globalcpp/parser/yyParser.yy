%{
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

%}

%output  "../../../code/globalcpp/parser/yyParser.cpp"
%defines "../../../code/globalcpp/parser/yyParser.h"

%error-verbose

%expect 306

%token END	0
%token TOKEN_EOL

%left TOKEN_COMMA

%union {
	stype_t s;
}

%right	TOKEN_ASSIGNMENT
		TOKEN_PLUS_EQUALS TOKEN_MINUS_EQUALS TOKEN_MULTIPLY_EQUALS TOKEN_DIVIDE_EQUALS TOKEN_MODULUS_EQUALS
		TOKEN_AND_EQUALS TOKEN_EXCL_OR_EQUALS TOKEN_OR_EQUALS
		TOKEN_SHIFT_LEFT_EQUALS TOKEN_SHIFT_RIGHT_EQUALS
		TOKEN_TERNARY

%left TOKEN_LOGICAL_OR
%left TOKEN_LOGICAL_AND

%left TOKEN_BITWISE_OR
%left TOKEN_BITWISE_EXCL_OR
%left TOKEN_BITWISE_AND

%left TOKEN_EQUALITY TOKEN_INEQUALITY
%left TOKEN_LESS_THAN TOKEN_LESS_THAN_OR_EQUAL TOKEN_GREATER_THAN TOKEN_GREATER_THAN_OR_EQUAL

%left TOKEN_SHIFT_LEFT TOKEN_SHIFT_RIGHT
%left TOKEN_PLUS TOKEN_MINUS
%left TOKEN_MULTIPLY TOKEN_DIVIDE TOKEN_MODULUS

%token <s> TOKEN_LISTENER

%right TOKEN_NEG TOKEN_NOT TOKEN_COMPLEMENT

%token <s> TOKEN_FLOAT TOKEN_INTEGER TOKEN_IDENTIFIER TOKEN_STRING TOKEN_NIL TOKEN_NULL

%token TOKEN_LBRACKET TOKEN_RBRACKET

%token TOKEN_COLON TOKEN_SEMICOLON TOKEN_DOLLAR TOKEN_DOUBLE_COLON TOKEN_NUMBER

%left <s> TOKEN_LSQUARE TOKEN_RSQUARE TOKEN_LPAREN TOKEN_RPAREN TOKEN_INCREMENT TOKEN_DECREMENT TOKEN_PERIOD

%token	TOKEN_MAKEARRAY TOKEN_ENDARRAY
		TOKEN_CATCH TOKEN_TRY
		TOKEN_DO TOKEN_FOR TOKEN_IF TOKEN_ELSE TOKEN_SWITCH TOKEN_WHILE
		TOKEN_BREAK TOKEN_CASE TOKEN_CONTINUE
		TOKEN_SIZE
		TOKEN_END TOKEN_RETURN

%type <s.val> event_parameter_list event_parameter_list_need
%type <s.val> statement_list statement makearray_statement_list makearray_statement
%type <s.val> compound_statement
%type <s.val> expr
%type <s.val> func_prim_expr
%type <s.val> prim_expr
%type <s.val> nonident_prim_expr
%type <s.val> number
%type <s> identifier_prim

%start program
 
%%

program:
	newline statement_list { parsedata.val = node1( sval_statement_list, $2 ); }
	;

statement_list:
	statement_list statement newline { $$ = append_node( $1, $2 ); }
	//| statement_list error TOKEN_EOL { yyerrok; }
	| statement newline { $$ = linked_list_end( $1 ); }
	| newline { $$ = node0( sval_none ); }
	;

statement:
	TOKEN_IDENTIFIER event_parameter_list TOKEN_COLON { $$ = node3( sval_label, $1.val, $2, YYLLOC ); }
	| TOKEN_PLUS TOKEN_IDENTIFIER event_parameter_list TOKEN_COLON { $$ = node3( sval_label, $2.val, $3, YYLLOC ); }
	| TOKEN_MINUS TOKEN_IDENTIFIER event_parameter_list TOKEN_COLON { $$ = node3( sval_privatelabel, $2.val, $3, YYLLOC ); }
	| TOKEN_CASE prim_expr event_parameter_list TOKEN_COLON { $$ = node3( sval_case, $2, $3, YYLLOC ); }
	| compound_statement
	| TOKEN_IF prim_expr newline statement[S] newline { $$ = node3( sval_if, $2, $S, YYLLOC ); }
	| TOKEN_IF prim_expr newline statement[S1] newline TOKEN_ELSE newline statement[S2] { $$ = node4( sval_ifelse, $2, $S1, $S2, YYLLOC ); }
	| TOKEN_WHILE prim_expr newline statement { $$ = node4( sval_while, $2, $4, node0( sval_none ), YYLLOC ); }
	| TOKEN_FOR TOKEN_LPAREN statement[SINIT] TOKEN_SEMICOLON expr[E] TOKEN_SEMICOLON statement_list[INC] TOKEN_RPAREN newline statement[S]
	{
		sval_u while_stmt = node4( sval_while, $E, $S, node1( sval_statement_list, $INC ), YYLLOC );
		$$ = node1( sval_statement_list, append_node( linked_list_end( $SINIT ), while_stmt ) );
	}
	| TOKEN_FOR TOKEN_LPAREN TOKEN_SEMICOLON expr[E] TOKEN_SEMICOLON statement_list[INC] TOKEN_RPAREN newline statement[S]
	{
		$$ = node4( sval_while, $E, $S, node1( sval_statement_list, $INC ), YYLLOC );
	}
	| TOKEN_DO newline statement[S] newline TOKEN_WHILE prim_expr[E] { $$ = node3( sval_do, $S, $E, YYLLOC ); }
	| TOKEN_TRY newline compound_statement newline TOKEN_CATCH newline compound_statement { $$ = node3( sval_catch, $3, $7, YYLLOC ); }
	| TOKEN_SWITCH prim_expr newline compound_statement { $$ = node3( sval_switch, $2, $4, YYLLOC ); }
	| TOKEN_BREAK { $$ = node1( sval_break, YYLLOC ); }
	| TOKEN_CONTINUE { $$ = node1( sval_continue, YYLLOC ); }
	| TOKEN_IDENTIFIER event_parameter_list { $$ = node3( sval_cmd, $1.val, node1( sval_none, $2 ), node_pos( $1.sourcePos ) ); }
	//| TOKEN_IDENTIFIER TOKEN_DOUBLE_COLON TOKEN_IDENTIFIER event_parameter_list { $$ = node3( sval_cmd, node_string( parsetree_string( str( $1.stringValue ) + "::" + $3.stringValue ) ), node1( sval_none, $4 ), $1.sourcePos ); }
	| nonident_prim_expr TOKEN_IDENTIFIER event_parameter_list { $$ = node4( sval_cmd_method, $1, $2.val, node1( sval_none, $3 ), node_pos( $2.sourcePos ) ); }
	//| nonident_prim_expr TOKEN_IDENTIFIER TOKEN_DOUBLE_COLON TOKEN_IDENTIFIER event_parameter_list { $$ = node4( sval_cmd_method, $1, node_string( parsetree_string( str( $2.stringValue ) + "::" + $4.stringValue ) ), node1( sval_none, $5 ), $2.sourcePos ); }
	| nonident_prim_expr TOKEN_ASSIGNMENT expr { $$ = node3( sval_assignment, $1, $3, YYLLOC ); }
	| nonident_prim_expr TOKEN_PLUS_EQUALS expr { $$ = node3( sval_assignment, $1, node4( sval_operation, node1b( OP_BIN_PLUS ), $1, $3, YYLLOC ), YYLLOC ); }
	| nonident_prim_expr TOKEN_MINUS_EQUALS expr { $$ = node3( sval_assignment, $1, node4( sval_operation, node1b( OP_BIN_MINUS ), $1, $3, YYLLOC ), YYLLOC ); }
	| nonident_prim_expr TOKEN_MULTIPLY_EQUALS expr { $$ = node3( sval_assignment, $1, node4( sval_operation, node1b( OP_BIN_MULTIPLY ), $1, $3, YYLLOC ), YYLLOC ); }
	| nonident_prim_expr TOKEN_DIVIDE_EQUALS expr { $$ = node3( sval_assignment, $1, node4( sval_operation, node1b( OP_BIN_DIVIDE ), $1, $3, YYLLOC ), YYLLOC ); }
	| nonident_prim_expr TOKEN_MODULUS_EQUALS expr { $$ = node3( sval_assignment, $1, node4( sval_operation, node1b( OP_BIN_PERCENTAGE ), $1, $3, YYLLOC ), YYLLOC ); }
	| nonident_prim_expr TOKEN_AND_EQUALS expr { $$ = node3( sval_assignment, $1, node4( sval_operation, node1b( OP_BIN_BITWISE_AND ), $1, $3, YYLLOC ), YYLLOC ); }
	| nonident_prim_expr TOKEN_EXCL_OR_EQUALS expr { $$ = node3( sval_assignment, $1, node4( sval_operation, node1b( OP_BIN_BITWISE_EXCL_OR ), $1, $3, YYLLOC ), YYLLOC ); }
	| nonident_prim_expr TOKEN_OR_EQUALS expr { $$ = node3( sval_assignment, $1, node4( sval_operation, node1b( OP_BIN_BITWISE_OR ), $1, $3, YYLLOC ), YYLLOC ); }
	| nonident_prim_expr TOKEN_SHIFT_LEFT_EQUALS expr { $$ = node3( sval_assignment, $1, node4( sval_operation, node1b( OP_BIN_SHIFT_LEFT ), $1, $3, YYLLOC ), YYLLOC ); }
	| nonident_prim_expr TOKEN_SHIFT_RIGHT_EQUALS expr { $$ = node3( sval_assignment, $1, node4( sval_operation, node1b( OP_BIN_SHIFT_RIGHT ), $1, $3, YYLLOC ), YYLLOC ); }
	| nonident_prim_expr TOKEN_INCREMENT { $$ = node3( sval_assignment, $1, node2( sval_func1, node1b( OP_UN_INC ), $1 ), YYLLOC ); }
	| nonident_prim_expr TOKEN_DECREMENT { $$ = node3( sval_assignment, $1, node2( sval_func1, node1b( OP_UN_DEC ), $1 ), YYLLOC ); }
	| statement TOKEN_SEMICOLON
	;

compound_statement:
	TOKEN_LBRACKET newline statement_list newline TOKEN_RBRACKET { $$ = node1( sval_statement_list, $3 ); }
	;

expr:
	expr TOKEN_LOGICAL_AND newline expr { $$ = node3( sval_and, $1, $4, YYLLOC ); }
	| expr TOKEN_LOGICAL_OR newline expr { $$ = node3( sval_or, $1, $4, YYLLOC ); }
	| expr TOKEN_BITWISE_AND newline expr { $$ = node4( sval_operation, node1b( OP_BIN_BITWISE_AND ), $1, $4, YYLLOC ); }
	| expr TOKEN_BITWISE_EXCL_OR newline expr { $$ = node4( sval_operation, node1b( OP_BIN_BITWISE_EXCL_OR ), $1, $4, YYLLOC ); }
	| expr TOKEN_BITWISE_OR newline expr { $$ = node4( sval_operation, node1b( OP_BIN_BITWISE_OR ), $1, $4, YYLLOC ); }
	| expr TOKEN_EQUALITY newline expr { $$ = node4( sval_operation, node1b( OP_BIN_EQUALITY ), $1, $4, YYLLOC ); }
	| expr TOKEN_INEQUALITY newline expr { $$ = node4( sval_operation, node1b( OP_BIN_INEQUALITY ), $1, $4, YYLLOC ); }
	| expr TOKEN_LESS_THAN newline expr { $$ = node4( sval_operation, node1b( OP_BIN_LESS_THAN ), $1, $4, YYLLOC ); }
	| expr TOKEN_GREATER_THAN newline expr { $$ = node4( sval_operation, node1b( OP_BIN_GREATER_THAN ), $1, $4, YYLLOC ); }
	| expr TOKEN_LESS_THAN_OR_EQUAL newline expr { $$ = node4( sval_operation, node1b( OP_BIN_LESS_THAN_OR_EQUAL ), $1, $4, YYLLOC ); }
	| expr TOKEN_GREATER_THAN_OR_EQUAL newline expr { $$ = node4( sval_operation, node1b( OP_BIN_GREATER_THAN_OR_EQUAL ), $1, $4, YYLLOC ); }
	| expr TOKEN_PLUS newline expr { $$ = node4( sval_operation, node1b( OP_BIN_PLUS ), $1, $4, YYLLOC ); }
	| expr TOKEN_MINUS newline expr { $$ = node4( sval_operation, node1b( OP_BIN_MINUS ), $1, $4, YYLLOC ); }
	| expr TOKEN_MULTIPLY newline expr { $$ = node4( sval_operation, node1b( OP_BIN_MULTIPLY ), $1, $4, YYLLOC ); }
	| expr TOKEN_DIVIDE newline expr { $$ = node4( sval_operation, node1b( OP_BIN_DIVIDE ), $1, $4, YYLLOC ); }
	| expr TOKEN_MODULUS newline expr { $$ = node4( sval_operation, node1b( OP_BIN_PERCENTAGE ), $1, $4, YYLLOC ); }
	| expr TOKEN_SHIFT_LEFT newline expr { $$ = node4( sval_operation, node1b( OP_BIN_SHIFT_LEFT ), $1, $4, YYLLOC ); }
	| expr TOKEN_SHIFT_RIGHT newline expr { $$ = node4( sval_operation, node1b( OP_BIN_SHIFT_RIGHT ), $1, $4, YYLLOC ); }
	| expr TOKEN_TERNARY expr TOKEN_COLON expr { $$ = node4( sval_ifelse, $1, $3, $5, YYLLOC ); }
	| nonident_prim_expr
	| func_prim_expr
	| identifier_prim { $$ = node1( sval_store_string, $1.val ); }
	;

func_prim_expr:
	TOKEN_IDENTIFIER event_parameter_list_need { $$ = node3( sval_cmd_default_ret, $1.val, node1( sval_none, $2 ), node_pos( $1.sourcePos ) ); }
	//| TOKEN_IDENTIFIER TOKEN_DOUBLE_COLON TOKEN_IDENTIFIER event_parameter_list_need { $$ = node3( sval_cmd_default_ret, node_string( parsetree_string( str( $1.val.stringValue ) + "::" + $3.val.stringValue ) ), node1( sval_none, $4 ), $1.sourcePos ); }
	| nonident_prim_expr TOKEN_IDENTIFIER event_parameter_list { $$ = node4( sval_cmd_method_ret, $1, $2.val, node1( sval_none, $3 ), node_pos( $2.sourcePos ) ); }
	//| nonident_prim_expr TOKEN_IDENTIFIER TOKEN_DOUBLE_COLON TOKEN_IDENTIFIER event_parameter_list { $$ = node4( sval_cmd_method_ret, $1, node_string( parsetree_string( str( $2.val.stringValue ) + "::" + $4.val.stringValue ) ), node1( sval_none, $5 ), $2.sourcePos ); }
	| TOKEN_NEG func_prim_expr { $$ = node3( sval_func1, node1b( OP_UN_MINUS ), $2, YYLLOC ); }
	| TOKEN_COMPLEMENT func_prim_expr { $$ = node3( sval_func1, node1b( OP_UN_COMPLEMENT ), $2, YYLLOC ); }
	| TOKEN_NOT func_prim_expr { $$ = node2( sval_not, $2, YYLLOC ); }
	| TOKEN_IDENTIFIER TOKEN_DOUBLE_COLON prim_expr { $$ = node3( sval_constarray, node2( sval_store_string, $1.val, node_pos( $1.sourcePos ) ), $3, YYLLOC ); }
	| nonident_prim_expr TOKEN_DOUBLE_COLON prim_expr { $$ = node3( sval_constarray, $1, $3, YYLLOC ); }
	| TOKEN_MAKEARRAY newline makearray_statement_list newline TOKEN_ENDARRAY { $$ = node1( sval_makearray, $3 ); }
	;

event_parameter_list:
	{ $$ = node0( sval_none ); }
	| event_parameter_list prim_expr { $$ = append_node( $1, $2 ); }
	| prim_expr { $$ = linked_list_end( $1 ); }
	;

event_parameter_list_need:
	event_parameter_list_need prim_expr { $$ = append_node( $1, $2 ); }
	| prim_expr { $$ = linked_list_end( $1 ); }
	;

prim_expr:
	nonident_prim_expr
	| identifier_prim { $$ = node1( sval_store_string, $1.val ); }
	| prim_expr TOKEN_DOUBLE_COLON prim_expr { $$ = node3( sval_constarray, $1, $3, YYLLOC ); }
	;

nonident_prim_expr:
	TOKEN_DOLLAR TOKEN_LPAREN expr TOKEN_RPAREN { $$ = node3( sval_func1, node1b( OP_UN_TARGETNAME ), $3, YYLLOC ); }
	| TOKEN_DOLLAR TOKEN_IDENTIFIER { $$ = node3( sval_func1, node1b( OP_UN_TARGETNAME ), node1( sval_store_string, $2.val ), YYLLOC ); }
	| TOKEN_DOLLAR TOKEN_STRING { $$ = node3( sval_func1, node1b( OP_UN_TARGETNAME ), node1( sval_store_string, $2.val ), YYLLOC ); }
	| nonident_prim_expr TOKEN_PERIOD TOKEN_IDENTIFIER { $$ = node3( sval_field, $1, $3.val, node_pos( $3.sourcePos ) ); }
	| nonident_prim_expr TOKEN_PERIOD TOKEN_STRING { $$ = node3( sval_field, $1, $3.val, node_pos( $3.sourcePos ) ); }
	| nonident_prim_expr TOKEN_PERIOD TOKEN_SIZE { $$ = node3( sval_func1, node1b( OP_UN_SIZE ), $1, YYLLOC ); }
	| nonident_prim_expr TOKEN_LSQUARE expr TOKEN_RSQUARE { $$ = node3( sval_array, $1, $3, $1 ); }
	| TOKEN_STRING { $$ = node1( sval_store_string, $1.val ); }
	| number
	| TOKEN_LPAREN expr expr expr TOKEN_RPAREN { $$ = node4( sval_calc_vector, $2, $3, $4, YYLLOC ); }
	| TOKEN_LISTENER { $$ = node2( sval_store_method, $1.val, YYLLOC ); }
	| TOKEN_LPAREN expr TOKEN_RPAREN { $$ = $2; }
	| TOKEN_LPAREN TOKEN_RPAREN { $$ = node0( sval_none ); }
	| TOKEN_NEG nonident_prim_expr { $$ = node3( sval_func1, node1b( OP_UN_MINUS ), $2, YYLLOC ); }
	| TOKEN_COMPLEMENT nonident_prim_expr { $$ = node3( sval_func1, node1b( OP_UN_COMPLEMENT ), $2, YYLLOC ); }
	| TOKEN_NOT nonident_prim_expr { $$ = node2( sval_not, $2, YYLLOC ); }
	| TOKEN_NULL { $$ = node1( sval_store_null, YYLLOC ); }
	| TOKEN_NIL { $$ = node1( sval_store_nil, YYLLOC ); }
	;

number:
	TOKEN_FLOAT { $$ = node1( sval_store_float, $1.val ); }
	| TOKEN_INTEGER { $$ = node1( sval_store_integer, $1.val ); }
	;

identifier_prim:
	TOKEN_IDENTIFIER
	;

makearray_statement_list:
	{ $$ = node0( sval_none ); }
	| makearray_statement_list makearray_statement newline { $$ = append_node( $1, node1( sval_makearray, $2 ) ); }
	| makearray_statement newline { $$ = linked_list_end( node1( sval_makearray, $1 ) ); }
	;

makearray_statement:
	prim_expr { $$ = linked_list_end( $1 ); }
	| makearray_statement prim_expr { $$ = append_node( $1, $2 ); }
	;

newline:
	{}
	| TOKEN_EOL
	;
 
%%
