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

#include <q_shared.h>
#include <baseimp.h>
#include <str.h>
#include "dummy_base.h"
#include "../client/client.h"
#include <Windows.h>

clientStatic_t cls;
baseImport_t bi;

cvar_t *developer;
cvar_t *g_scriptcheck;
cvar_t *g_showopcodes;
cvar_t *precache;
cvar_t *sv_scriptfiles;

void Com_FillBaseImports() {
	bi.Printf					= BI_Printf;
	bi.DPrintf					= BI_DPrintf;
	bi.Error					= BI_Error;
	bi.Free						= BI_Free;
	bi.Malloc					= BI_Malloc;

	bi.Milliseconds				= BI_Milliseconds;

	bi.FS_FreeFile				= FS_FreeFile;
	bi.FS_ReadFile				= FS_ReadFile2;
	bi.FS_CanonicalFilename		= FS_CanonicalFilename;
}

void Z_InitMemory() {
}

void Cmd_Init( void ) {
}

void Cvar_Init( void ) {
}

void FS_InitFilesystem( void ) {
}

void FS_InitFilesystem2( void ) {
}

void QDECL Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		*buffer;
	int			size;

	va_start( argptr, error );
	size = vsnprintf( NULL, 0, error, argptr );
	va_end( argptr );

	buffer = ( char * )bi.Malloc( size + 1 );
	buffer[ size ] = 0;

	va_start( argptr, error );
	vsnprintf( buffer, size, error, argptr );
	va_end( argptr );

	bi.Error( level, "%s", buffer );
	bi.Free( buffer );
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		*buffer;
	int			size;

	va_start( argptr, msg );
	size = vsnprintf( NULL, 0, msg, argptr );
	va_end( argptr );

	buffer = ( char * )bi.Malloc( size + 1 );
	buffer[ size ] = 0;

	va_start( argptr, msg );
	vsnprintf( buffer, size, msg, argptr );
	va_end( argptr );

	bi.Printf( "%s", buffer );
	bi.Free( buffer );
}

void QDECL Com_DPrintf( const char *msg, ... ) {
	va_list		argptr;
	char		*buffer;
	int			size;

	va_start( argptr, msg );
	size = vsnprintf( NULL, 0, msg, argptr );
	va_end( argptr );

	buffer = ( char * )bi.Malloc( size + 1 );
	buffer[ size ] = 0;

	va_start( argptr, msg );
	vsnprintf( buffer, size, msg, argptr );
	va_end( argptr );

	bi.DPrintf( "%s", buffer );
	bi.Free( buffer );
}

void BI_Printf( const char *msg, ... )
{
	va_list		argptr;
	char		*buffer;
	int			size;

	va_start( argptr, msg );
	size = vsnprintf( NULL, 0, msg, argptr );
	va_end( argptr );

	buffer = ( char * )bi.Malloc( size + 1 );
	buffer[ size ] = 0;

	va_start( argptr, msg );
	vsnprintf( buffer, size, msg, argptr );
	va_end( argptr );

	printf( "%s", buffer );

	bi.Free( buffer );
}

void BI_DPrintf( const char *msg, ... )
{
#ifdef _DEBUG
	va_list		argptr;
	char		*buffer;
	int			size;

	if( !developer->integer ) {
		return;
	}

	va_start( argptr, msg );
	size = vsnprintf( NULL, 0, msg, argptr );
	va_end( argptr );

	buffer = ( char * )bi.Malloc( size + 1 );
	buffer[ size ] = 0;

	va_start( argptr, msg );
	vsnprintf( buffer, size, msg, argptr );
	va_end( argptr );

	printf( "%s", buffer );

	bi.Free( buffer );
#endif
}

void BI_Error( int level, const char *msg, ... )
{
	va_list		argptr;
	char		*buffer;
	int			size;

	va_start( argptr, msg );
	size = vsnprintf( NULL, 0, msg, argptr );
	va_end( argptr );

	buffer = ( char * )bi.Malloc( size + 1 );
	buffer[ size ] = 0;

	va_start( argptr, msg );
	vsnprintf( buffer, size, msg, argptr );
	va_end( argptr );

	BI_Printf( "%s", buffer );

	bi.Free( buffer );
}

int BI_Milliseconds( void )
{
	return GetTickCount();
}

void *BI_Malloc( int size )
{
	return malloc( size );
}

void BI_Free( void *ptr )
{
	free( ptr );
}

void FS_FreeFile( void *buffer )
{
	bi.Free( buffer );
}

int FS_ReadFile2( const char *name, void **buffer, qboolean bSilent )
{
	str filename = name;
	FILE *file = fopen( filename, "rb+" );
	int size;
	char *p;

	if( file == NULL )
	{
		return -1;
	}

	fseek( file, 0, SEEK_END );
	size = ftell( file );
	rewind( file );

	*buffer = bi.Malloc( size + 1 );
	memset( *buffer, 0, size );
	fread( *buffer, size, 1, file );

	fclose( file );


	p = ( char * )*buffer;
	p[ size ] = 0;

	return size;
}

void FS_CanonicalFilename( char *filename )
{
	char *p = filename;

	while( *p )
	{
		if( p[ 0 ] == '/' && p[ 1 ] == '/' )
		{
			char *p2 = p + 1;

			while( *p2 )
			{
				p2[ 0 ] = p2[ 1 ];
				p2++;
			}
		}

		p++;
	}
}
