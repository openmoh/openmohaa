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

// tiki_skel.h : TIKI Skeletor

#ifndef __TIKI_SKEL_H__
#define __TIKI_SKEL_H__

#ifdef __cplusplus
extern "C" {
#endif

void TIKI_CacheFileSkel( skelHeader_t *pHeader, skelcache_t *cache, int length );
void TIKI_CalcLodConsts( lodControl_t *LOD );
qboolean TIKI_LoadSKB( const char *path, skelcache_t *cache );
qboolean TIKI_LoadSKD( const char *path, skelcache_t *cache );
skelHeaderGame_t *TIKI_GetSkel( int index );
int TIKI_GetSkelCache( skelHeaderGame_t *pSkel );
void TIKI_FreeSkel( int index );
void TIKI_FreeSkelCache( skelcache_t *cache );
skelcache_t *TIKI_FindSkel( const char *path );
skelcache_t *TIKI_FindFreeSkel( void );
int TIKI_RegisterSkel( const char *path, dtiki_t *tiki );

void SaveLODFile( const char *path, lodControl_t *LOD );
void GetLODFile( skelcache_t *cache );

#ifdef __cplusplus
}
#endif

#endif // __TIKI_SKEL_H__
