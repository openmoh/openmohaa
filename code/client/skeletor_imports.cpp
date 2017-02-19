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

// skeletor_imports.cpp : Skeletor imports

#include "q_shared.h"
#include "qcommon.h"
#include "dbgheap.h"

void Skel_DPrintf( const char *fmt, ... )
{
	char msg[ 4096 ];
	va_list va;

	va_start( va, fmt );
	vsprintf( msg, fmt, va );
	va_end( va );

	Com_DPrintf( "%s", msg );
}

#ifndef _DEBUG_MEM

void Skel_Free( void *ptr )
{
	Z_Free( ptr );
}

void *Skel_Alloc( size_t size )
{
	return Z_TagMalloc( size, TAG_SKEL );
}

#endif

void Skel_FreeFile( void *buffer )
{
	FS_FreeFile( buffer );
}

int Skel_ReadFileEx( const char *qpath, void **buffer, qboolean quiet )
{
	return FS_ReadFileEx( qpath, buffer, quiet );
}
