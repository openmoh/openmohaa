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

// parsetree.cpp: Abstract Syntax Layer for Lexer/Parser

#include "glb_local.h"
#include "parsetree.h"
#include <mem_tempalloc.h>

MEM_TempAlloc			parsetree_allocator;

yyparsedata				parsedata;
sval_u					node_none = { 0 };

char *str_replace( char *orig, char *rep, char *with ) {
	char *result; // the return string
	char *ins;    // the next insert point
	char *tmp;    // varies
	size_t len_rep;  // length of rep
	size_t len_with; // length of with
	size_t len_front; // distance between rep and end of last rep
	int count;    // number of replacements

	if( !orig )
		return NULL;
	if( !rep )
		rep = "";
	len_rep = strlen( rep );
	if( !with )
		with = "";
	len_with = strlen( with );

	ins = orig;
	for( count = 0; tmp = strstr( ins, rep ); ++count ) {
		ins = tmp + len_rep;
	}

	// first time through the loop, all the variable are set correctly
	// from here on,
	//    tmp points to the end of the result string
	//    ins points to the next occurrence of rep in orig
	//    orig points to the remainder of orig after "end of rep"
	tmp = result = ( char * )parsetree_allocator.Alloc( strlen( orig ) + ( len_with - len_rep ) * count + 1 );

	if( !result )
		return NULL;

	while( count-- ) {
		ins = strstr( orig, rep );
		len_front = ins - orig;
		tmp = strncpy( tmp, orig, len_front ) + len_front;
		tmp = strcpy( tmp, with ) + len_with;
		orig += len_front + len_rep; // move to next "end of rep"
	}
	strcpy( tmp, orig );
	return result;
}

void parsetree_freeall()
{
	parsetree_allocator.FreeAll();

	if( showopcodes->integer )
	{
		glbs.DPrintf( "%d bytes freed\n", parsedata.total_length );
	}
}

void parsetree_init()
{
	parsedata.total_length = 0;
}

size_t parsetree_length()
{
	return parsedata.total_length;
}

#if 0
char *parsetree_string( const char *string )
{
	//char *pszString = ( char * )parsetree_allocator.Alloc( strlen( string ) + 1 );
	//strcpy( pszString, string );

	char *buffer = str_replace( ( char * )string, "\\\"", "\"" );

	if( buffer )
	{
		char *ptr = buffer;

		if( ptr[ 0 ] == '"' )
		{
			ptr++;
		}

		int len = strlen( buffer );

		if( buffer[ len - 1 ] == '"' )
		{
			buffer[ len - 1 ] = 0;
		}

		buffer = ptr;
	}

	return buffer;
}
#endif

extern size_t yyleng;
extern size_t prev_yyleng;

char *parsetree_malloc( size_t s )
{
	parsedata.total_length += s;
	return ( char * )parsetree_allocator.Alloc( s );
}

int node_compare( void *pe1, void *pe2 )
{
	// wtf
	// fixme
	return 0;
}

sval_u append_lists( sval_u val1, sval_u val2 )
{
	val1.node[ 1 ].node[ 1 ] = val2.node[ 0 ];
	val1.node[ 1 ] = val2.node[ 1 ];

	return val1;
}

sval_u append_node( sval_u val1, sval_u val2 )
{
	sval_u *node;

	node = ( sval_u * )parsetree_malloc( sizeof( sval_u ) * 2 );

	node[ 1 ].node = NULL;
	node[ 0 ] = val2;

	val1.node[ 1 ].node[ 1 ].node = node;
	val1.node[ 1 ].node = node;

	return val1;
}

sval_u prepend_node( sval_u val1, sval_u val2 )
{
	sval_u *node;

	node = ( sval_u * )parsetree_malloc( sizeof( sval_u ) * 2 );

	node[ 0 ] = val1;
	node[ 1 ] = val2;

	val2.node = node;

	return val2;
}

sval_u linked_list_end( sval_u val )
{
	sval_u *node;
	sval_u end;

	node = ( sval_u * )parsetree_malloc( sizeof( sval_u ) * 2 );

	node[ 0 ] = val;
	node[ 1 ].node = NULL;

	end.node = ( sval_u * )parsetree_malloc( sizeof( sval_u ) * 2 );

	end.node[ 0 ].node = node;
	end.node[ 1 ].node = node;

	return end;
}

sval_u node1_( int val1 )
{
	sval_u val;

	val.intValue = val1;

	return val;
}

sval_u node1b( int val1 )
{
	sval_u val;

	val.byteValue = val1;

	return val;
}

sval_u node_pos( unsigned int pos )
{
	sval_u val;

	val.sourcePosValue = pos;

	return val;
}

sval_u node_string( char *text )
{
	sval_u val;

	val.stringValue = text;

	return val;
}

sval_u node0( int type )
{
	sval_u val;

	if( type == sval_none )
	{
		// memory optimization
		val.node = &node_none;
	}
	else
	{
		val.node = ( sval_u * )parsetree_malloc( sizeof( sval_u ) * 1 );

		val.node[ 0 ].node = NULL;
		val.node[ 0 ].type = type;
	}

	return val;
}

sval_u node1( int type, sval_u val1 )
{
	sval_u val;

	val.node = ( sval_u * )parsetree_malloc( sizeof( sval_u ) * 2 );

	val.node[ 0 ].type = type;
	val.node[ 1 ] = val1;

	return val;
}

sval_u node2( int type, sval_u val1, sval_u val2 )
{
	sval_u val;

	val.node = ( sval_u * )parsetree_malloc( sizeof( sval_u ) * 3 );

	val.node[ 0 ].type = type;
	val.node[ 1 ] = val1;
	val.node[ 2 ] = val2;

	return val;
}

sval_u node3( int type, sval_u val1, sval_u val2, sval_u val3 )
{
	sval_u val;

	val.node = ( sval_u * )parsetree_malloc( sizeof( sval_u ) * 4 );

	val.node[ 0 ].type = type;
	val.node[ 1 ] = val1;
	val.node[ 2 ] = val2;
	val.node[ 3 ] = val3;

	return val;
}

sval_u node4( int type, sval_u val1, sval_u val2, sval_u val3, sval_u val4 )
{
	sval_u val;

	val.node = ( sval_u * )parsetree_malloc( sizeof( sval_u ) * 5 );

	val.node[ 0 ].type = type;
	val.node[ 1 ] = val1;
	val.node[ 2 ] = val2;
	val.node[ 3 ] = val3;
	val.node[ 4 ] = val4;

	return val;
}

sval_u node5( int type, sval_u val1, sval_u val2, sval_u val3, sval_u val4, sval_u val5 )
{
	sval_u val;

	val.node = ( sval_u * )parsetree_malloc( sizeof( sval_u ) * 6 );

	val.node[ 0 ].type = type;
	val.node[ 1 ] = val1;
	val.node[ 2 ] = val2;
	val.node[ 3 ] = val3;
	val.node[ 4 ] = val4;
	val.node[ 5 ] = val5;

	return val;
}

sval_u node6( int type, sval_u val1, sval_u val2, sval_u val3, sval_u val4, sval_u val5, sval_u val6 )
{
	sval_u val;

	val.node = ( sval_u * )parsetree_malloc( sizeof( sval_u ) * 7 );

	val.node[ 0 ].type = type;
	val.node[ 1 ] = val1;
	val.node[ 2 ] = val2;
	val.node[ 3 ] = val3;
	val.node[ 4 ] = val4;
	val.node[ 5 ] = val5;
	val.node[ 6 ] = val6;

	return val;
}
