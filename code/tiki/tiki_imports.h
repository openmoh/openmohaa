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

// tiki_imports.h : TIKI Imports

#ifndef __TIKI_IMPORTS_H__
#define __TIKI_IMPORTS_H__

#ifdef __cplusplus
extern "C" {
#endif

void TIKI_DPrintf( const char *fmt, ... );

#ifndef _DEBUG_MEM
void TIKI_Free( void *ptr );
void *TIKI_Alloc( size_t size );
#else
#define TIKI_Free(ptr) free(ptr)
#define TIKI_Alloc(size) malloc(size)
#endif

void TIKI_FreeFile( void *buffer );
int TIKI_ReadFileEx( const char *qpath, void **buffer, qboolean quiet );

#ifdef __cplusplus
}
#endif

#endif // __TIKI_IMPORTS_H__
