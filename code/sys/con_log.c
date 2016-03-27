/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "sys_local.h"

#define MAX_LOG 32768

static char				consoleLog[ MAX_LOG ];
static size_t			writePos = 0;
static size_t			readPos = 0;

/*
==================
CON_LogSize
==================
*/
size_t CON_LogSize( void )
{
	if( readPos <= writePos )
		return writePos - readPos;
	else
		return writePos + MAX_LOG - readPos;
}

/*
==================
CON_LogFree
==================
*/
static size_t CON_LogFree( void )
{
	return MAX_LOG - CON_LogSize( ) - 1;
}

/*
==================
CON_LogWrite
==================
*/
size_t CON_LogWrite( const char *in )
{
	size_t length = strlen( in );
	size_t firstChunk;
	size_t secondChunk;

	while( CON_LogFree( ) < length && CON_LogSize( ) > 0 )
	{
		// Free enough space
		while( consoleLog[ readPos ] != '\n' && CON_LogSize( ) > 1 )
			readPos = ( readPos + 1 ) % MAX_LOG;

		// Skip past the '\n'
		readPos = ( readPos + 1 ) % MAX_LOG;
	}

	if( CON_LogFree( ) < length )
		return 0;

	if( writePos + length > MAX_LOG )
	{
		firstChunk  = MAX_LOG - writePos;
		secondChunk = length - firstChunk;
	}
	else
	{
		firstChunk  = length;
		secondChunk = 0;
	}

	Com_Memcpy( consoleLog + writePos, in, firstChunk );
	Com_Memcpy( consoleLog, in + firstChunk, secondChunk );

	writePos = ( writePos + length ) % MAX_LOG;

	return length;
}

/*
==================
CON_LogRead
==================
*/
size_t CON_LogRead( char *out, size_t outSize )
{
	size_t firstChunk;
	size_t secondChunk;

	if( CON_LogSize( ) < outSize )
		outSize = CON_LogSize( );

	if( readPos + outSize > MAX_LOG )
	{
		firstChunk  = MAX_LOG - readPos;
		secondChunk = outSize - firstChunk;
	}
	else
	{
		firstChunk  = outSize;
		secondChunk = 0;
	}

	Com_Memcpy( out, consoleLog + readPos, firstChunk );
	Com_Memcpy( out + firstChunk, out, secondChunk );

	readPos = ( readPos + outSize ) % MAX_LOG;

	return outSize;
}
