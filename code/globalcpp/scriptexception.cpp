/*
===========================================================================
Copyright (C) 2015 the OpenMoHAA team

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

// scriptexception.cpp : Script Exception

#include "glb_local.h"
#include "scriptexception.h"

int ScriptException::next_abort = 0;
int ScriptException::next_bIsForAnim = 0;

void ScriptException::CreateException( const char *data )
{
	string = data;

	bAbort = next_abort;
	next_abort = 0;
	bIsForAnim = next_bIsForAnim;
	next_bIsForAnim = 0;
}

ScriptException::ScriptException( str text )
{
	CreateException( text.c_str() );
}

ScriptException::ScriptException( const char *format, ... )
{
	va_list va;
	char data[4100];

	va_start( va, format );
	vsprintf( data, format, va);
	va_end( va );

	CreateException( data );
}

ScriptException::ScriptException( char *text )
{
	CreateException( text );
}

void Error( const char * format, ... )
{
	va_list va;
	char data[4100];

	va_start( va, format );
	vsprintf( data, format, va);
	va_end( va );

	throw new ScriptException( ( const char * )data );
}
