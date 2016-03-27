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

#ifndef __DUMY_BASE_H__
#define __DUMY_BASE_H__

#include <glb_local.h>
#include "q_shared.h"

#ifdef __cplusplus
extern "C" {
#endif

void Com_FillBaseImports();
void Z_InitMemory();

void Cmd_Init( void );
void Cvar_Init( void );
void FS_InitFilesystem( void );

void	BI_Printf( const char *msg, ... );
void	BI_DPrintf( const char *msg, ... );
void	BI_Error( int level, const char *msg, ... );
int		BI_Milliseconds( void );
void *	BI_Malloc( int size );
void	BI_Free( void *ptr );
int		FS_ReadFile2( const char *name, void **buffer, qboolean bSilent );

//void FS_FreeFile( void *buffer );
//int FS_ReadFile( const char *name, void **buffer, bool bSilent );
//void FS_CanonicalFilename( char *filename );

#ifdef __cplusplus
}
#endif

#endif /* __DUMY_BASE_H__ */
