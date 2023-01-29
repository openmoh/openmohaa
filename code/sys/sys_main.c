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

#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <localization.h>
#include "../client/snd_public.h"

#ifdef WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#ifndef DEDICATED
#ifdef USE_LOCAL_HEADERS
#	include "SDL.h"
#	include "SDL_cpuinfo.h"
#else
#	include <SDL.h>
#	include <SDL_cpuinfo.h>
#endif
#endif

#include "sys_local.h"
#include "sys_loadlib.h"

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

static char binaryPath[ MAX_OSPATH ] = { 0 };
static char installPath[ MAX_OSPATH ] = { 0 };

/*
=================
Sys_SetBinaryPath
=================
*/
void Sys_SetBinaryPath(const char *path)
{
	Q_strncpyz(binaryPath, path, sizeof(binaryPath));
}

/*
=================
Sys_BinaryPath
=================
*/
char *Sys_BinaryPath(void)
{
	return binaryPath;
}

/*
=================
Sys_SetDefaultInstallPath
=================
*/
void Sys_SetDefaultInstallPath(const char *path)
{
	Q_strncpyz(installPath, path, sizeof(installPath));
}

/*
=================
Sys_DefaultInstallPath
=================
*/
char *Sys_DefaultInstallPath(void)
{
	if (*installPath)
		return installPath;
	else
		return Sys_Cwd();
}

/*
=================
Sys_DefaultAppPath
=================
*/
char *Sys_DefaultAppPath(void)
{
	return Sys_BinaryPath();
}

/*
=================
Sys_In_Restart_f

Restart the input subsystem
=================
*/
void Sys_In_Restart_f( void )
{
	IN_Restart();
}

/*
=================
Sys_ConsoleInput

Handle new console input
=================
*/
char *Sys_ConsoleInput(void)
{
	return CON_Input( );
}

/*
=================
Sys_Exit

Single exit point (regular exit or in case of error)
=================
*/
void Sys_Exit( int ex )
{
	CON_Shutdown( );

#ifndef DEDICATED
	SDL_Quit( );
#endif

#ifdef NDEBUG
	exit( ex );
#else
	// Cause a backtrace on error exits
	assert( ex == 0 );
	exit( ex );
#endif
}

/*
=================
Sys_Quit
=================
*/
void Sys_Quit( void )
{
	CL_Shutdown( );
	Sys_Exit( 0 );
}

/*
=================
Sys_GetProcessorFeatures
=================
*/
cpuFeatures_t Sys_GetProcessorFeatures( void )
{
	cpuFeatures_t features = 0;

#ifndef DEDICATED
	if( SDL_HasRDTSC( ) )    features |= CF_RDTSC;
	if( SDL_HasMMX( ) )      features |= CF_MMX;
	if( SDL_HasSSE( ) )      features |= CF_SSE;
	if( SDL_HasSSE2( ) )     features |= CF_SSE2;
#endif

	return features;
}

/*
=================
Sys_Init
=================
*/
void Sys_Init(void)
{
	Cmd_AddCommand( "in_restart", Sys_In_Restart_f );
	Cvar_Set( "arch", OS_STRING " " ARCH_STRING );
	Cvar_Set( "username", Sys_GetCurrentUser( ) );
}

/*
=================
Sys_AnsiColorPrint

Transform Q3 colour codes to ANSI escape sequences
=================
*/
void Sys_AnsiColorPrint( const char *msg )
{
	static char buffer[ MAXPRINTMSG ];
	int         length = 0;
	static int  q3ToAnsi[ 8 ] =
	{
		30, // COLOR_BLACK
		31, // COLOR_RED
		32, // COLOR_GREEN
		33, // COLOR_YELLOW
		34, // COLOR_BLUE
		36, // COLOR_CYAN
		35, // COLOR_MAGENTA
		0   // COLOR_WHITE
	};

	while( *msg )
	{
		if( Q_IsColorString( msg ) || *msg == '\n' )
		{
			// First empty the buffer
			if( length > 0 )
			{
				buffer[ length ] = '\0';
				fputs( buffer, stderr );
				length = 0;
			}

			if( *msg == '\n' )
			{
				// Issue a reset and then the newline
				fputs( "\033[0m\n", stderr );
				msg++;
			}
			else
			{
				// Print the color code
				Com_sprintf( buffer, sizeof( buffer ), "\033[%dm",
						q3ToAnsi[ ColorIndex( *( msg + 1 ) ) ] );
				fputs( buffer, stderr );
				msg += 2;
			}
		}
		else
		{
			if( length >= MAXPRINTMSG - 1 )
				break;

			buffer[ length ] = *msg;
			length++;
			msg++;
		}
	}

	// Empty anything still left in the buffer
	if( length > 0 )
	{
		buffer[ length ] = '\0';
		fputs( buffer, stderr );
	}
}

/*
=================
Sys_Print
=================
*/
void Sys_Print( const char *msg )
{
	CON_LogWrite( msg );
	CON_Print( msg );
}

/*
=================
SyScriptError
=================
*/
void SyScriptError( const char *error, ... )
{
	va_list argptr;
	char    string[1024];

	CL_Shutdown ();

	va_start (argptr,error);
	Q_vsnprintf (string, sizeof(string), error, argptr);
	va_end (argptr);

	//SyScriptErrorDialog( string );

	Sys_Exit( 1 );
}

/*
=================
Sys_Warn
=================
*/
void Sys_Warn( char *warning, ... )
{
	va_list argptr;
	char    string[1024];

	va_start (argptr,warning);
	Q_vsnprintf (string, sizeof(string), warning, argptr);
	va_end (argptr);

	CON_Print( va( "Warning: %s", string ) );
}

/*
============
Sys_FileTime

returns -1 if not present
============
*/
int Sys_FileTime( char *path )
{
	struct stat buf;

	if (stat (path,&buf) == -1)
		return -1;

	return buf.st_mtime;
}

/*
=================
Sys_UnloadDll
=================
*/
void Sys_UnloadDll( void *dllHandle )
{
	if( !dllHandle )
	{
		Com_Printf("Sys_UnloadDll(NULL)\n");
		return;
	}

	Sys_UnloadLibrary(dllHandle);
}

static void *cgame_library;
static void *game_library;

/*
=================
Sys_UnloadGame
=================
*/
void Sys_UnloadGame( void )
{
	Com_Printf( "------ Unloading Game ------\n" );

	if( game_library ) {
		Sys_UnloadLibrary( game_library );
	}

	game_library = NULL;
}

/*
=================
Sys_GetGameAPI
=================
*/
void *Sys_GetGameAPI( void *parms )
{
	void	*( *GetGameAPI ) ( void * );
	const char	*basepath;
	const char	*cdpath;
	const char	*gamedir;
	const char	*homepath;
#ifdef MACOS_X
	const char *apppath;
#endif
	const char	*fn;
	const char *gamename = "game" ARCH_STRING DLL_EXT;

	if( game_library )
		Com_Error( ERR_FATAL, "Sys_GetGameAPI without calling Sys_UnloadGame" );

	// check the current debug directory first for development purposes
	homepath = Cvar_VariableString( "fs_homepath" );
	basepath = Cvar_VariableString( "fs_basepath" );
	cdpath = Cvar_VariableString( "fs_cdpath" );
	gamedir = Cvar_VariableString( "fs_game" );
#ifdef MACOS_X
	apppath = Cvar_VariableString( "fs_apppath" );
#endif

	fn = FS_BuildOSPath( basepath, gamedir, gamename );

	game_library = Sys_LoadLibrary( fn );

	//First try in mod directories. basepath -> homepath -> cdpath
	if( !game_library ) {
		if( homepath[ 0 ] ) {
			Com_Printf( "Sys_GetGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
			fn = FS_BuildOSPath( homepath, gamedir, gamename );
			game_library = Sys_LoadLibrary( fn );
		}
	}
#ifdef MACOS_X
	if( !game_library ) {
		if( apppath[ 0 ] ) {
			Com_Printf( "Sys_GetGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
			fn = FS_BuildOSPath( apppath, gamedir, gamename );
			game_library = Sys_LoadLibrary( fn );
		}
	}
#endif
	if( !game_library ) {
		if( cdpath[ 0 ] ) {
			Com_Printf( "Sys_GetGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
			fn = FS_BuildOSPath( cdpath, gamedir, gamename );
			game_library = Sys_LoadLibrary( fn );
		}
	}

	//Now try in base. basepath -> homepath -> cdpath
	if( !game_library ) {
		Com_Printf( "Sys_GetGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
		fn = FS_BuildOSPath( basepath, PRODUCT_NAME, gamename );
		game_library = Sys_LoadLibrary( fn );
	}

	if( !game_library ) {
		if( homepath[ 0 ] ) {
			Com_Printf( "Sys_GetGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
			fn = FS_BuildOSPath( homepath, PRODUCT_NAME, gamename );
			game_library = Sys_LoadLibrary( fn );
		}
	}
#ifdef MACOS_X
	if( !game_library ) {
		if( apppath[ 0 ] ) {
			Com_Printf( "Sys_GetGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
			fn = FS_BuildOSPath( apppath, OPENJKGAME, gamename );
			game_library = Sys_LoadLibrary( fn );
		}
	}
#endif
	if( !game_library ) {
		if( cdpath[ 0 ] ) {
			Com_Printf( "Sys_GetGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
			fn = FS_BuildOSPath( cdpath, PRODUCT_NAME, gamename );
			game_library = Sys_LoadLibrary( fn );
		}
	}

	//Still couldn't find it.
	if( !game_library ) {
		Com_Printf( "Sys_GetGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
		Com_Error( ERR_FATAL, "Couldn't load game" );
	}

	Com_Printf( "Sys_GetGameAPI(%s): succeeded ...\n", fn );
	GetGameAPI = ( void *( *)( void * ) )Sys_LoadFunction( game_library, "GetGameAPI" );

	if( !GetGameAPI )
	{
		Sys_UnloadGame();
		return NULL;
	}

	return GetGameAPI( parms );
}

/*
=================
Sys_UnloadCGame
=================
*/
void Sys_UnloadCGame( void )
{
	Com_Printf( "------ Unloading ClientGame ------\n" );

	if( cgame_library ) {
		Sys_UnloadLibrary( cgame_library );
	}

	cgame_library = NULL;
}

/*
=================
Sys_GetCGameAPI
=================
*/
void *Sys_GetCGameAPI( void *parms )
{
	void	*( *GetCGameAPI ) ( void * );
	const char	*basepath;
	const char	*cdpath;
	const char	*gamedir;
	const char	*homepath;
#ifdef MACOS_X
	const char *apppath;
#endif
	const char	*fn;
	const char *gamename = "cgame" ARCH_STRING DLL_EXT;

	if( cgame_library )
		Com_Error( ERR_FATAL, "Sys_GetCGameAPI without calling Sys_UnloadCGame" );

	// check the current debug directory first for development purposes
	homepath = Cvar_VariableString( "fs_homepath" );
	basepath = Cvar_VariableString( "fs_basepath" );
	cdpath = Cvar_VariableString( "fs_cdpath" );
	gamedir = Cvar_VariableString( "fs_game" );
#ifdef MACOS_X
	apppath = Cvar_VariableString( "fs_apppath" );
#endif

	fn = FS_BuildOSPath( basepath, gamedir, gamename );

	cgame_library = Sys_LoadLibrary( fn );

	//First try in mod directories. basepath -> homepath -> cdpath
	if( !cgame_library ) {
		if( homepath[ 0 ] ) {
			Com_Printf( "Sys_GetCGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
			fn = FS_BuildOSPath( homepath, gamedir, gamename );
			cgame_library = Sys_LoadLibrary( fn );
		}
	}
#ifdef MACOS_X
	if( !cgame_library ) {
		if( apppath[ 0 ] ) {
			Com_Printf( "Sys_GetCGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
			fn = FS_BuildOSPath( apppath, gamedir, gamename );
			cgame_library = Sys_LoadLibrary( fn );
		}
	}
#endif
	if( !cgame_library ) {
		if( cdpath[ 0 ] ) {
			Com_Printf( "Sys_GetCGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
			fn = FS_BuildOSPath( cdpath, gamedir, gamename );
			cgame_library = Sys_LoadLibrary( fn );
		}
	}

	//Now try in base. basepath -> homepath -> cdpath
	if( !cgame_library ) {
		Com_Printf( "Sys_GetCGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
		fn = FS_BuildOSPath( basepath, PRODUCT_NAME, gamename );
		cgame_library = Sys_LoadLibrary( fn );
	}

	if( !cgame_library ) {
		if( homepath[ 0 ] ) {
			Com_Printf( "Sys_GetCGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
			fn = FS_BuildOSPath( homepath, PRODUCT_NAME, gamename );
			cgame_library = Sys_LoadLibrary( fn );
		}
	}
#ifdef MACOS_X
	if( !cgame_library ) {
		if( apppath[ 0 ] ) {
			Com_Printf( "Sys_GetCGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
			fn = FS_BuildOSPath( apppath, OPENJKGAME, gamename );
			cgame_library = Sys_LoadLibrary( fn );
		}
	}
#endif
	if( !cgame_library ) {
		if( cdpath[ 0 ] ) {
			Com_Printf( "Sys_GetCGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
			fn = FS_BuildOSPath( cdpath, PRODUCT_NAME, gamename );
			cgame_library = Sys_LoadLibrary( fn );
		}
	}

	//Still couldn't find it.
	if( !cgame_library ) {
		Com_Printf( "Sys_GetCGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
		Com_Error( ERR_FATAL, "Couldn't load game" );
	}

	Com_Printf( "Sys_GetCGameAPI(%s): succeeded ...\n", fn );
	GetCGameAPI = ( void *( *)( void * ) )Sys_LoadFunction( cgame_library, "GetCGameAPI" );

	if( !GetCGameAPI )
	{
		Sys_UnloadCGame();
		return NULL;
	}

	return GetCGameAPI( parms );
}

/*
=================
Sys_TryLibraryLoad
=================
*/
static void* Sys_TryLibraryLoad(const char* base, const char* gamedir, const char* fname, char* fqpath )
{
	void* libHandle;
	char* fn;

	*fqpath = 0;

	fn = FS_BuildOSPath( base, gamedir, fname );
	Com_Printf( "Sys_LoadDll(%s)... \n", fn );

	libHandle = Sys_LoadLibrary(fn);

	if(!libHandle) {
		Com_Printf( "Sys_LoadDll(%s) failed:\n\"%s\"\n", fn, Sys_LibraryError() );
		return NULL;
	}

	Com_Printf ( "Sys_LoadDll(%s): succeeded ...\n", fn );
	Q_strncpyz ( fqpath , fn , MAX_QPATH ) ;

	return libHandle;
}

/*
=================
Sys_LoadDll

Used to load a development dll instead of a virtual machine
#1 look down current path
#2 look in fs_homepath
#3 look in fs_basepath
=================
*/
void *Sys_LoadDll( const char *name, char *fqpath ,
	intptr_t (**entryPoint)(int, ...),
	intptr_t (*systemcalls)(intptr_t, ...) )
{
	void  *libHandle;
	void  (*dllEntry)( intptr_t (*syscallptr)(intptr_t, ...) );
	char  fname[MAX_OSPATH];
	char  *basepath;
	char  *homepath;
#ifdef MACOS_X
	char  *apppath;
#endif
	char  *pwdpath;
	char  *gamedir;

	assert( name );

	Q_snprintf (fname, sizeof(fname), "%s" ARCH_STRING "opm" DLL_EXT, name);

	// TODO: use fs_searchpaths from files.c
	pwdpath = Sys_Cwd();
	basepath = Cvar_VariableString( "fs_basepath" );
	homepath = Cvar_VariableString( "fs_homepath" );
#ifdef MACOS_X
	apppath  = Cvar_VariableString( "fs_apppath" );
#endif
	gamedir = Cvar_VariableString( "fs_game" );

	libHandle = Sys_TryLibraryLoad(pwdpath, gamedir, fname, fqpath);

	if(!libHandle && homepath)
		libHandle = Sys_TryLibraryLoad(homepath, gamedir, fname, fqpath);

	if(!libHandle && basepath)
		libHandle = Sys_TryLibraryLoad(basepath, gamedir, fname, fqpath);
#ifdef MACOS_X
	if(!libHandle && apppath)
		libHandle = Sys_TryLibraryLoad(apppath, gamedir, fname, fqpath);
#endif

	if(!libHandle) {
		Com_Printf ( "Sys_LoadDll(%s) failed to load library\n", name );
		return NULL;
	}

	dllEntry = Sys_LoadFunction( libHandle, "dllEntry" );
	*entryPoint = Sys_LoadFunction( libHandle, "vmMain" );

	if ( !*entryPoint || !dllEntry )
	{
		Com_Printf ( "Sys_LoadDll(%s) failed to find vmMain function:\n\"%s\" !\n", name, Sys_LibraryError( ) );
		Sys_UnloadLibrary(libHandle);

		return NULL;
	}

	Com_Printf ( "Sys_LoadDll(%s) found vmMain function at %p\n", name, *entryPoint );
	dllEntry( systemcalls );

	return libHandle;
}

/*
=================
Sys_ParseArgs
=================
*/
void Sys_ParseArgs( int argc, char **argv )
{
	if( argc == 2 )
	{
		if( !strcmp( argv[1], "--version" ) ||
				!strcmp( argv[1], "-v" ) )
		{
			const char* date = __DATE__;
#ifdef DEDICATED
			fprintf( stdout, PRODUCT_VERSION " dedicated server (%s)\n", date );
#else
			fprintf( stdout, PRODUCT_VERSION " client (%s)\n", date );
#endif
			Sys_Exit(0);
		}
	}
}

#ifndef DEFAULT_BASEDIR
#	ifdef MACOS_X
#		define DEFAULT_BASEDIR Sys_StripAppBundle(Sys_BinaryPath())
#	else
#		define DEFAULT_BASEDIR Sys_BinaryPath()
#	endif
#endif

/*
=================
Sys_SigHandler
=================
*/
void Sys_SigHandler( int signal )
{
	static qboolean signalcaught = qfalse;

	if( signalcaught )
	{
		fprintf( stderr, "DOUBLE SIGNAL FAULT: Received signal %d, exiting...\n",
			signal );
	}
	else
	{
		signalcaught = qtrue;
		fprintf( stderr, "Received signal %d, exiting...\n", signal );
#ifndef DEDICATED
		CL_Shutdown();
#endif
		SV_Shutdown( "Signal caught" );
	}

	Sys_Exit( 0 ); // Exit with 0 to avoid recursive signals
}

/*
=================
main
=================
*/
int main( int argc, char **argv )
{
	int   i;
	char  commandLine[ MAX_STRING_CHARS ] = { 0 };
	char  szPath[ MAX_QPATH ];

#ifndef DEDICATED
	// SDL version check

	// Compile time
#	if !SDL_VERSION_ATLEAST(MINSDL_MAJOR,MINSDL_MINOR,MINSDL_PATCH)
#		error A more recent version of SDL is required
#	endif

	// Run time
	SDL_version ver;
	SDL_GetVersion( &ver );

#define MINSDL_VERSION \
	XSTRING(MINSDL_MAJOR) "." \
	XSTRING(MINSDL_MINOR) "." \
	XSTRING(MINSDL_PATCH)

	if( SDL_VERSIONNUM( ver.major, ver.minor, ver.patch ) <
		SDL_VERSIONNUM( MINSDL_MAJOR, MINSDL_MINOR, MINSDL_PATCH ) )
	{
		Sys_Dialog( DT_ERROR, va( "SDL version " MINSDL_VERSION " or greater is required, "
			"but only version %d.%d.%d was found. You may be able to obtain a more recent copy "
			"from http://www.libsdl.org/.", ver.major, ver.minor, ver.patch ), "SDL Library Too Old" );

		Sys_Exit( 1 );
	}
#endif

	Sys_ParseArgs( argc, argv );
	GetCurrentDir( szPath, sizeof( szPath ) );
	Sys_SetBinaryPath( szPath );
	Sys_SetDefaultInstallPath( DEFAULT_BASEDIR );
	SetProgramPath( DEFAULT_BASEDIR );

	// Concatenate the command line for passing to Com_Init
	for( i = 1; i < argc; i++ )
	{
		Q_strcat( commandLine, sizeof( commandLine ), argv[ i ] );
		Q_strcat( commandLine, sizeof( commandLine ), " " );
	}

	Com_Init( commandLine );
	Sys_InitLocalization();
	NET_Init( );

	CON_Init( );

#ifndef _DEBUG

#ifndef _WIN32
	// Windows doesn't have these signals
	// see CON_CtrlHandler() in con_win32.c
	signal( SIGHUP, Sys_SigHandler );
	signal( SIGQUIT, Sys_SigHandler );
	signal( SIGTRAP, Sys_SigHandler );
	signal( SIGIOT, Sys_SigHandler );
	signal( SIGBUS, Sys_SigHandler );
#endif

	signal( SIGILL, Sys_SigHandler );
	signal( SIGFPE, Sys_SigHandler );
	signal( SIGSEGV, Sys_SigHandler );
	signal( SIGTERM, Sys_SigHandler );

#endif

	while( 1 )
	{
		IN_Frame( );
		Com_Frame( );
	}

	return 0;
}

