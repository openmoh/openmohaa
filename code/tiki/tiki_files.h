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

// tiki_files.h : TIKI Files

#ifndef __TIKI_FILES_H__
#define __TIKI_FILES_H__

#include "../qcommon/tiki.h"

#ifdef __cplusplus
extern "C" {
#endif

extern qboolean tiki_loading;

extern cvar_t	*dumploadedanims;
extern cvar_t	*low_anim_memory;
extern cvar_t	*showLoad;
extern cvar_t	*convertAnims;

extern dloaddef_t loaddef;

void TIKI_FreeStorage( dloaddef_t *ld );
void *TIKI_AllocateLoadData( size_t length );
char *TIKI_CopyString( const char *s );
dtikianim_t *TIKI_LoadTikiAnim( const char *path );

#ifdef __cplusplus
dtiki_t *TIKI_LoadTikiModel( dtikianim_t *tikianim, const char *name, con_map<str, str> *keyValues );
#endif

void TIKI_CalcRadius( dtiki_t *tiki );
skelAnimDataGameHeader_t *SkeletorCacheFileCallback( const char *path );
skelAnimDataGameHeader_t *SkeletorCacheGetData( int index );

#ifdef __cplusplus
bool SkeletorCacheFindFilename( const char *path, int *indexPtr );
bool SkeletorCacheLoadData( const char *path, bool precache, int newIndex );
void SkeletorCacheUnloadData( int index );
void SkeletorCacheCleanCache();
void TikiAddToBounds( dtikianim_t *tiki, SkelVec3 *newBounds );
#endif

void TIKI_AnimList_f();
void TIKI_FixFrameNum( dtikianim_t *ptiki, skelAnimDataGameHeader_t *animData, dtikicmd_t *cmd, const char *alias );
void TIKI_LoadAnim( dtikianim_t *ptiki );
dtikianim_t *TIKI_InitTiki( dloaddef_t *ld, size_t defsize );
void TIKI_RemoveTiki( dtikianim_t *ptiki );

#ifdef __cplusplus
}
#endif

#endif // __TIKI_FILES_H__