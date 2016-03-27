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

#ifndef __BASEIMP_H__
#define __BASEIMP_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct aliasListNode_s aliasListNode_t;
typedef struct cvar_s cvar_t;

typedef struct baseImport_s
{
	void ( *Printf )( const char *format, ... );
	void ( *DPrintf )( const char *format, ... );
	const char * ( *LV_ConvertString )( const char *string );
	cvar_t * ( *Cvar_Get )( const char *varName, const char *varValue, int varFlags );
	void( *Cvar_Set )( const char *varName, const char *varValue );
	int( *FS_ReadFile )( const char *qpath, void **buffer, qboolean quiet );
	void( *FS_FreeFile )( void *buffer );
	size_t( *FS_WriteFile )( const char *qpath, const void *buffer, size_t size );
	fileHandle_t( *FS_FOpenFileWrite )( const char *fileName );
	fileHandle_t( *FS_FOpenFileAppend )( const char *fileName );
	fileHandle_t( *FS_FOpenFile )( const char *fileName );
	const char *( *FS_PrepFileWrite )( const char *fileName );
	size_t( *FS_Write )( const void *buffer, size_t len, fileHandle_t fileHandle );
	size_t( *FS_Read )( void *buffer, size_t len, fileHandle_t fileHandle );
	void( *FS_FCloseFile )( fileHandle_t fileHandle );
	int( *FS_Tell )( fileHandle_t fileHandle );
	int( *FS_Seek )( fileHandle_t fileHandle, long int offset, fsOrigin_t origin );
	void( *FS_Flush )( fileHandle_t fileHandle );
	int( *FS_FileNewer )( const char *source, const char *destination );
	void( *FS_CanonicalFilename )( char *fileName );
	char **( *FS_ListFiles )( const char *qpath, const char *extension, qboolean wantSubs, int *numFiles );
	void( *FS_FreeFileList )( char **list );
	int( *Milliseconds )( );
	double( *MillisecondsDbl )( );
	void( *Error )( int errortype, const char *format, ... );
	void *( *Malloc )( size_t size );
	void( *Free )( void *ptr );
	int( *Key_StringToKeynum )( const char *str );
	char * ( *Key_KeynumToBindString )( int keyNum );
	void( *Key_GetKeysForCommand )( const char *command, int *key1, int *key2 );
	void( *SendConsoleCommand )( const char *text );
	void( *SendServerCommand )( int client, const char *format, ... );
	qboolean( *GlobalAlias_Add )( const char *alias, const char *name, const char *parameters );
	char * ( *GlobalAlias_FindRandom )( const char *alias, aliasListNode_t **ret );
	void( *GlobalAlias_Dump )( );
	void( *GlobalAlias_Clear )( );
	void( *SetConfigstring )( int index, const char *val );
	char *( *GetConfigstring )( int index );
	void( *AddSvsTimeFixup )( int *piTime );

} baseImport_t;

extern baseImport_t bi;
extern cvar_t *developer;
extern cvar_t *precache;
extern cvar_t *sv_scriptfiles;
extern cvar_t *g_scriptcheck;
extern cvar_t *g_showopcodes;

void CacheResource( const char *name );

#ifdef __cplusplus
}
#endif

#endif
