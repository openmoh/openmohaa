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

// scriptexception.h: Script Exception

#ifndef __EXCEPT_H__
#define __EXCEPT_H__

#include "str.h"

class ScriptException
{
public:
	str		string;
	int		bAbort;
	int		bIsForAnim;

private:
	void CreateException( const char *data );

public:
	ScriptException( str text );
	ScriptException( const char *format, ... );
	ScriptException( char *text );

	static int next_abort;
	static int next_bIsForAnim;
};

void Error( const char * format, ... );

#define ScriptDeprecated( function ) throw ScriptException( function ": DEPRECATED. DON'T USE IT ANYMORE" )
#define ScriptError throw ScriptException

#endif /* __EXCEPT_H__ */
