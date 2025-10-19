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

#ifdef _WIN32_WINNT_WINXP
  // Use EnumProcesses() with Windows XP compatibility
#  define PSAPI_VERSION 1
#endif

#ifndef STANDALONE
#define STANDALONE
#endif

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "sys_local.h"

#include <windows.h>
#include <lmerr.h>
#include <lmcons.h>
#include <lmwksta.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <conio.h>
#include <wincrypt.h>
#include <shlobj.h>
#include <psapi.h>
#include <float.h>

#ifndef KEY_WOW64_32KEY
#define KEY_WOW64_32KEY 0x0200
#endif

#ifndef DEDICATED
static UINT timerResolution = 0;
#endif

/*
================
Sys_SetFPUCW
Set FPU control word to default value
================
*/

#ifndef _RC_CHOP
// mingw doesn't seem to have these defined :(

  #define _MCW_EM	0x0008001fU
  #define _MCW_RC	0x00000300U
  #define _MCW_PC	0x00030000U
  #define _RC_NEAR      0x00000000U
  #define _PC_53	0x00010000U
  
  unsigned int _controlfp(unsigned int new, unsigned int mask);
#endif

#define FPUCWMASK1 (_MCW_RC | _MCW_EM)
#define FPUCW (_RC_NEAR | _MCW_EM | _PC_53)

#if idx64
#define FPUCWMASK	(FPUCWMASK1)
#else
#define FPUCWMASK	(FPUCWMASK1 | _MCW_PC)
#endif

void Sys_SetFloatEnv(void)
{
	_controlfp(FPUCW, FPUCWMASK);
}

/*
================
Sys_DefaultHomePath
================
*/
static char *Sys_DefaultHomePath( void )
{
	static char homePath[ MAX_OSPATH ] = { 0 };

	if(!*homePath && com_homepath)
	{
		TCHAR szPath[MAX_PATH];

		if( !SUCCEEDED( SHGetFolderPathA( NULL, CSIDL_APPDATA,
						NULL, 0, szPath ) ) )
		{
			Com_Printf("Unable to detect CSIDL_APPDATA\n");
			return NULL;
		}
		
		Com_sprintf(homePath, sizeof(homePath), "%s%c", szPath, PATH_SEP);

		if(com_homepath->string[0])
			Q_strcat(homePath, sizeof(homePath), com_homepath->string);
		else
			Q_strcat(homePath, sizeof(homePath), HOMEPATH_NAME);
	}

	return homePath;
}

char *Sys_DefaultHomeConfigPath(void) { return Sys_DefaultHomePath(); }
char *Sys_DefaultHomeDataPath(void)   { return Sys_DefaultHomePath(); }
char *Sys_DefaultHomeStatePath(void)  { return Sys_DefaultHomePath(); }

/*
================
Sys_SteamPath
================
*/
char *Sys_SteamPath( void )
{
#ifndef STANDALONE

#define STEAMPATH_NAME "Quake 3 Arena"
#define STEAMPATH_APPID "2200"

	static char steamPath[ MAX_OSPATH ] = { 0 };

	HKEY steamRegKey;
	DWORD pathLen = MAX_OSPATH;
	qboolean finishPath = qfalse;

	// Assuming Steam is a 32-bit app
	if (!steamPath[0] && !RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App "
		STEAMPATH_APPID, 0, KEY_QUERY_VALUE | KEY_WOW64_32KEY, &steamRegKey))
	{
		pathLen = MAX_OSPATH;
		if (RegQueryValueEx(steamRegKey, "InstallLocation", NULL, NULL, (LPBYTE)steamPath, &pathLen))
			steamPath[0] = '\0';

		RegCloseKey(steamRegKey);
	}

	if (!steamPath[0] && !RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Valve\\Steam", 0, KEY_QUERY_VALUE, &steamRegKey))
	{
		pathLen = MAX_OSPATH;
		if (RegQueryValueEx(steamRegKey, "SteamPath", NULL, NULL, (LPBYTE)steamPath, &pathLen))
			if (RegQueryValueEx(steamRegKey, "InstallPath", NULL, NULL, (LPBYTE)steamPath, &pathLen))
				steamPath[0] = '\0';

		if (steamPath[0])
			finishPath = qtrue;

		RegCloseKey(steamRegKey);
	}

	if (steamPath[0])
	{
		if (pathLen == MAX_OSPATH)
			pathLen--;

		steamPath[pathLen] = '\0';

		if (finishPath)
			Q_strcat(steamPath, MAX_OSPATH, "\\SteamApps\\common\\" STEAMPATH_NAME );
	}

	return steamPath;
#else
	return "";
#endif
}

/*
================
Sys_GogPath
================
*/
char *Sys_GogPath( void )
{
#ifndef STANDALONE

#define GOGPATH_ID "1441704920"

	static char gogPath[ MAX_OSPATH ] = { 0 };

	HKEY gogRegKey;
	DWORD pathLen = MAX_OSPATH;

	if (!gogPath[0] && !RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\GOG.com\\Games\\" GOGPATH_ID, 0, KEY_QUERY_VALUE | KEY_WOW64_32KEY, &gogRegKey))
	{
		pathLen = MAX_OSPATH;
		if (RegQueryValueEx(gogRegKey, "PATH", NULL, NULL, (LPBYTE)gogPath, &pathLen))
			gogPath[0] = '\0';

		RegCloseKey(gogRegKey);
	}

	if (gogPath[0])
	{
		if (pathLen == MAX_OSPATH)
			pathLen--;

		gogPath[pathLen] = '\0';
	}

	return gogPath;
#else
	return "";
#endif
}

/*
================
Sys_MicrosoftStorePath
================
*/
char* Sys_MicrosoftStorePath(void)
{
#ifndef STANDALONE

#define MSSTORE_PATH "Quake 3"

	static char microsoftStorePath[MAX_OSPATH] = { 0 };

	if (!microsoftStorePath[0]) 
	{
		TCHAR szPath[MAX_PATH];

		if( !SUCCEEDED( SHGetFolderPathA( NULL, CSIDL_PROGRAM_FILES,
						NULL, 0, szPath ) ) )
		{
			Com_Printf("Unable to detect CSIDL_PROGRAM_FILES\n");
			return microsoftStorePath;
		}

		// default: C:\Program Files\ModifiableWindowsApps\Quake 3\EN
		Com_sprintf(microsoftStorePath, sizeof(microsoftStorePath), "%s%cModifiableWindowsApps%c%s%cEN", szPath, PATH_SEP, PATH_SEP, MSSTORE_PATH, PATH_SEP);
	}

	return microsoftStorePath;
#else
	return "";
#endif
}

/*
================
Sys_Milliseconds
================
*/
int sys_timeBase;
int Sys_Milliseconds (void)
{
	int             sys_curtime;
	static qboolean initialized = qfalse;

	if (!initialized) {
		sys_timeBase = timeGetTime();
		initialized = qtrue;
	}
	sys_curtime = timeGetTime() - sys_timeBase;

	return sys_curtime;
}

/*
================
Sys_RandomBytes
================
*/
qboolean Sys_RandomBytes( byte *string, int len )
{
	HCRYPTPROV  prov;

	if( !CryptAcquireContext( &prov, NULL, NULL,
		PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) )  {

		return qfalse;
	}

	if( !CryptGenRandom( prov, len, (BYTE *)string ) )  {
		CryptReleaseContext( prov, 0 );
		return qfalse;
	}
	CryptReleaseContext( prov, 0 );
	return qtrue;
}

/*
================
Sys_GetCurrentUser
================
*/
char *Sys_GetCurrentUser( void )
{
	static char s_userName[1024];
	unsigned long size = sizeof( s_userName );

	if( !GetUserName( s_userName, &size ) )
		strcpy( s_userName, "player" );

	if( !s_userName[0] )
	{
		strcpy( s_userName, "player" );
	}

	return s_userName;
}

#define MEM_THRESHOLD 96*1024*1024

/*
==================
Sys_LowPhysicalMemory
==================
*/
qboolean Sys_LowPhysicalMemory( void )
{
	MEMORYSTATUS stat;
	GlobalMemoryStatus (&stat);
	return (stat.dwTotalPhys <= MEM_THRESHOLD) ? qtrue : qfalse;
}

/*
==============
Sys_Basename
==============
*/
const char *Sys_Basename( char *path )
{
	static char base[ MAX_OSPATH ] = { 0 };
	int length;

	length = strlen( path ) - 1;

	// Skip trailing slashes
	while( length > 0 && path[ length ] == '\\' )
		length--;

	while( length > 0 && path[ length - 1 ] != '\\' )
		length--;

	Q_strncpyz( base, &path[ length ], sizeof( base ) );

	length = strlen( base ) - 1;

	// Strip trailing slashes
	while( length > 0 && base[ length ] == '\\' )
    base[ length-- ] = '\0';

	return base;
}

/*
==============
Sys_Dirname
==============
*/
const char *Sys_Dirname( char *path )
{
	static char dir[ MAX_OSPATH ] = { 0 };
	int length;

	Q_strncpyz( dir, path, sizeof( dir ) );
	length = strlen( dir ) - 1;

	while( length > 0 && dir[ length ] != '\\' )
		length--;

	dir[ length ] = '\0';

	return dir;
}

/*
==============
Sys_FOpen
==============
*/
FILE *Sys_FOpen( const char *ospath, const char *mode ) {
	size_t length;

	// Windows API ignores all trailing spaces and periods which can get around Quake 3 file system restrictions.
	length = strlen( ospath );
	if ( length == 0 || ospath[length-1] == ' ' || ospath[length-1] == '.' ) {
		return NULL;
	}

	return fopen( ospath, mode );
}

/*
==============
Sys_Mkdir
==============
*/
qboolean Sys_Mkdir( const char *path )
{
	if( !CreateDirectory( path, NULL ) )
	{
		if( GetLastError( ) != ERROR_ALREADY_EXISTS )
			return qfalse;
	}

	return qtrue;
}

/*
==================
Sys_Mkfifo
Noop on windows because named pipes do not function the same way
==================
*/
FILE *Sys_Mkfifo( const char *ospath )
{
	return NULL;
}

/*
==============
Sys_Cwd
==============
*/
char *Sys_Cwd( void ) {
	static char cwd[MAX_OSPATH];

	_getcwd( cwd, sizeof( cwd ) - 1 );
	cwd[MAX_OSPATH-1] = 0;

	return cwd;
}

/*
==============
Sys_BinaryPathRelative
==============
*/
char *Sys_BinaryPathRelative(const char *relative)
{
	static char resolved[MAX_OSPATH];
	char combined[MAX_OSPATH];

	snprintf(combined, sizeof(combined), "%s\\%s", Sys_BinaryPath(), relative);

	DWORD len = GetFullPathNameA(combined, MAX_OSPATH, resolved, NULL);
	if (len == 0 || len >= MAX_OSPATH)
		return NULL;

	return resolved;
}

/*
==============================================================

DIRECTORY SCANNING

==============================================================
*/

#define MAX_FOUND_FILES 0x1000

/*
==============
Sys_ListFilteredFiles
==============
*/
void Sys_ListFilteredFiles(
    const char *basedir, const char *subdirs, char *filter, qboolean wantsubs, char **list, int *numfiles
)
{
    char               search[MAX_OSPATH], newsubdirs[MAX_OSPATH];
    char               filename[MAX_OSPATH];
    intptr_t           findhandle;
    struct _finddata_t findinfo;

    if (*numfiles >= MAX_FOUND_FILES - 1) {
        return;
    }

    if (basedir[0] == '\0') {
        return;
    }

    if (strlen(subdirs)) {
        Com_sprintf(search, sizeof(search), "%s\\%s\\*", basedir, subdirs);
    } else {
        Com_sprintf(search, sizeof(search), "%s\\*", basedir);
    }

    findhandle = _findfirst(search, &findinfo);
    if (findhandle == -1) {
        return;
    }

    do {
        // Fixed in OPM:
        // don't show current and parent dir entries twice
        if (!(Q_stricmp(findinfo.name, ".") && Q_stricmp(findinfo.name, "..") && Q_stricmp(findinfo.name, "cvs"))) {
            continue;
        }

        if ((findinfo.attrib & _A_SUBDIR) != 0 && wantsubs) {
            if (strlen(subdirs)) {
                Com_sprintf(newsubdirs, sizeof(newsubdirs), "%s\\%s\\*", subdirs, findinfo.name);
            } else {
                Com_sprintf(newsubdirs, sizeof(newsubdirs), "%s", findinfo.name);
            }

            // recursively iterate into subdirectory
            Sys_ListFilteredFiles(basedir, newsubdirs, filter, wantsubs, list, numfiles);
        }

        if (*numfiles >= MAX_FOUND_FILES - 1) {
            break;
        }

        if (strlen(subdirs)) {
            Com_sprintf(filename, sizeof(filename), "%s\\%s", subdirs, findinfo.name);
        } else {
            Q_strncpyz(filename, findinfo.name, sizeof(filename));
        }

        if (!Com_FilterPath(filter, filename, qfalse)) {
            continue;
        }

        list[*numfiles] = CopyString(filename);

        // replace backslashes with forward slashes, if any
        // FS_ReplaceSeparators would be nice but it does the opposite
        for (char *c = strchr(list[*numfiles], '\\'); c; c = strchr(list[*numfiles], '\\')) {
            *c = '/';
        }

        (*numfiles)++;
    } while (_findnext(findhandle, &findinfo) != -1);

    _findclose(findhandle);
}

/*
==============
strgtr
==============
*/
static qboolean strgtr(const char *s0, const char *s1)
{
    int l0, l1, i;

    l0 = strlen(s0);
    l1 = strlen(s1);

    if (l1 < l0) {
        l0 = l1;
    }

    for (i = 0; i < l0; i++) {
        if (s1[i] > s0[i]) {
            return qtrue;
        }
        if (s1[i] < s0[i]) {
            return qfalse;
        }
    }
    return qfalse;
}

/*
==============
Sys_ListFiles
==============
*/
char **Sys_ListFiles(const char *directory, const char *extension, const char *filter, int *numfiles, qboolean wantsubs)
{
    char               search[MAX_OSPATH];
    int                nfiles;
    char             **listCopy;
    char              *list[MAX_FOUND_FILES] = {NULL};
    struct _finddata_t findinfo;
    intptr_t           findhandle;
    qboolean           swapped;
    int                i;
    char               buffer[64];

    if (directory[0] == NULL) {
        *numfiles = 0;
        return NULL;
    }

    if (!extension) {
        extension = "";
    }

    // passing a slash as extension will find directories,
    // anything else looks only for files with that extension
    if (!filter && (extension[0] != '/' || extension[1])) {
        Q_snprintf(buffer, sizeof(buffer), "*%s", extension);
        filter = buffer;
    }

    if (filter) {
        nfiles = 0;
        Sys_ListFilteredFiles(directory, "", filter, wantsubs, list, &nfiles);

        list[nfiles] = NULL;
        *numfiles    = nfiles;
        if (!nfiles) {
            return NULL;
        }

        listCopy = Z_Malloc((nfiles + 1) * sizeof(*listCopy));
        for (i = 0; i < nfiles; i++) {
            listCopy[i] = list[i];
        }
        listCopy[i] = NULL;

        return listCopy;
    }

    // only enumerate directories from this point onward
    Com_sprintf(search, 256, "%s\\*", directory);

    // search
    nfiles     = 0;
    findhandle = _findfirst(search, &findinfo);
    if (findhandle == -1) {
        *numfiles = 0;
        return NULL;
    }

    do {
        // Fixed in OPM:
        // don't show current and parent dir entries twice
        if (!(Q_stricmp(findinfo.name, ".") && Q_stricmp(findinfo.name, "..") && Q_stricmp(findinfo.name, "cvs"))) {
            continue;
        }

        if ((findinfo.attrib & _A_SUBDIR) == 0) {
            continue;
        }

        if (nfiles >= MAX_FOUND_FILES - 1) {
            break;
        }

        list[nfiles] = CopyString(findinfo.name);

        // replace backslashes with forward slashes
        // FS_ReplaceSeparators would be nice but it does the opposite
        for (char *c = strchr(list[nfiles], '\\'); c; c = strchr(list[nfiles], '\\')) {
            *c = '/';
        }

        ++nfiles;
    } while (_findnext(findhandle, &findinfo) != -1);

    list[nfiles] = NULL;

    _findclose(findhandle);

    // return a copy of the list
    *numfiles = nfiles;

    if (!nfiles) {
        return NULL;
    }

    listCopy = Z_Malloc((nfiles + 1) * sizeof(*listCopy));
    for (i = 0; i < nfiles; i++) {
        listCopy[i] = list[i];
    }
    listCopy[i] = NULL;

    // bubble-sort name entries alphabetically, in ascending order
    do {
        swapped = qfalse;
        for (i = 1; i < nfiles; i++) {
            if (strgtr(listCopy[i - 1], listCopy[i])) {
                char *temp      = listCopy[i];
                listCopy[i]     = listCopy[i - 1];
                listCopy[i - 1] = temp;
                swapped         = qtrue;
            }
        }
    } while (swapped);

    return listCopy;
}

/*
==============
Sys_FreeFileList
==============
*/
void Sys_FreeFileList( char **list )
{
	int i;

	if ( !list ) {
		return;
	}

	for ( i = 0 ; list[i] ; i++ ) {
		Z_Free( list[i] );
	}

	Z_Free( list );
}


/*
==============
Sys_Sleep

Block execution for msec or until input is received.
==============
*/
void Sys_Sleep( int msec )
{
	if( msec == 0 )
		return;

#ifdef DEDICATED
	if( msec < 0 )
		WaitForSingleObject( GetStdHandle( STD_INPUT_HANDLE ), INFINITE );
	else
		WaitForSingleObject( GetStdHandle( STD_INPUT_HANDLE ), msec );
#else
	// Client Sys_Sleep doesn't support waiting on stdin
	if( msec < 0 )
		return;

	Sleep( msec );
#endif
}

/*
==============
Sys_ErrorDialog

Display an error message
==============
*/
void Sys_ErrorDialog( const char *error )
{
	Sys_Print( va( "%s\n", error ) );

	if( Sys_Dialog( DT_YES_NO, va( "%s. Copy console log to clipboard?", error ),
			"Error" ) == DR_YES )
	{
		HGLOBAL memoryHandle;
		char *clipMemory;

		memoryHandle = GlobalAlloc( GMEM_MOVEABLE|GMEM_DDESHARE, CON_LogSize( ) + 1 );
		clipMemory = (char *)GlobalLock( memoryHandle );

		if( clipMemory )
		{
			char *p = clipMemory;
			char buffer[ 1024 ];
			unsigned int size;

			while( ( size = CON_LogRead( buffer, sizeof( buffer ) ) ) > 0 )
			{
				Com_Memcpy( p, buffer, size );
				p += size;
			}

			*p = '\0';

			if( OpenClipboard( NULL ) && EmptyClipboard( ) )
				SetClipboardData( CF_TEXT, memoryHandle );

			GlobalUnlock( clipMemory );
			CloseClipboard( );
		}
	}
}

/*
==============
Sys_Dialog

Display a win32 dialog box
==============
*/
dialogResult_t Sys_Dialog( dialogType_t type, const char *message, const char *title )
{
	UINT uType;

	switch( type )
	{
		default:
		case DT_INFO:      uType = MB_ICONINFORMATION|MB_OK; break;
		case DT_WARNING:   uType = MB_ICONWARNING|MB_OK; break;
		case DT_ERROR:     uType = MB_ICONERROR|MB_OK; break;
		case DT_YES_NO:    uType = MB_ICONQUESTION|MB_YESNO; break;
		case DT_OK_CANCEL: uType = MB_ICONWARNING|MB_OKCANCEL; break;
	}

	switch( MessageBox( NULL, message, title, uType ) )
	{
		default:
		case IDOK:      return DR_OK;
		case IDCANCEL:  return DR_CANCEL;
		case IDYES:     return DR_YES;
		case IDNO:      return DR_NO;
	}
}

/*
==============
Sys_GLimpSafeInit

Windows specific "safe" GL implementation initialisation
==============
*/
void Sys_GLimpSafeInit( void )
{
}

/*
==============
Sys_GLimpInit

Windows specific GL implementation initialisation
==============
*/
void Sys_GLimpInit( void )
{
}

/*
==============
Sys_PlatformInit

Windows specific initialisation
==============
*/
void Sys_PlatformInit( void )
{
#ifndef DEDICATED
	TIMECAPS ptc;
#endif

	Sys_SetFloatEnv();

#ifndef DEDICATED
	if(timeGetDevCaps(&ptc, sizeof(ptc)) == MMSYSERR_NOERROR)
	{
		timerResolution = ptc.wPeriodMin;

		if(timerResolution > 1)
		{
			Com_Printf("Warning: Minimum supported timer resolution is %ums "
				"on this system, recommended resolution 1ms\n", timerResolution);
		}
		
		timeBeginPeriod(timerResolution);				
	}
	else
		timerResolution = 0;
#endif

	Sys_PlatformInit_New();
}

/*
==============
Sys_PlatformExit

Windows specific initialisation
==============
*/
void Sys_PlatformExit( void )
{
#ifndef DEDICATED
	if(timerResolution)
		timeEndPeriod(timerResolution);
#endif
}

/*
==============
Sys_SetEnv

set/unset environment variables (empty value removes it)
==============
*/
void Sys_SetEnv(const char *name, const char *value)
{
	if(value)
		_putenv(va("%s=%s", name, value));
	else
		_putenv(va("%s=", name));
}

/*
==============
Sys_PID
==============
*/
int Sys_PID( void )
{
	return GetCurrentProcessId( );
}

/*
==============
Sys_PIDIsRunning
==============
*/
qboolean Sys_PIDIsRunning( int pid )
{
	DWORD processes[ 1024 ];
	DWORD numBytes, numProcesses;
	int i;

	if( !EnumProcesses( processes, sizeof( processes ), &numBytes ) )
		return qfalse; // Assume it's not running

	numProcesses = numBytes / sizeof( DWORD );

	// Search for the pid
	for( i = 0; i < numProcesses; i++ )
	{
		if( (int)processes[ i ] == pid )
			return qtrue;
	}

	return qfalse;
}

/*
=================
Sys_DllExtension

Check if filename should be allowed to be loaded as a DLL.
=================
*/
qboolean Sys_DllExtension( const char *name ) {
	return COM_CompareExtension( name, DLL_EXT );
}

/*
==============
Sys_OpenFolderInPlatformFileManager
==============
*/
qboolean Sys_OpenFolderInPlatformFileManager( const char *path )
{
	return ShellExecute( NULL, "explore", path, NULL, NULL, SW_SHOWDEFAULT ) > (HINSTANCE)32;
}
