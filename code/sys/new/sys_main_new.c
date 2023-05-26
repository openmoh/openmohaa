/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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

#include "../sys_local.h"
#include "../win_localization.h"
#include "../sys_loadlib.h"

static void* game_library = NULL;
static void* cgame_library = NULL;

/*
==============
SaveRegistryInfo
==============
*/
qboolean SaveRegistryInfo(qboolean user, const char* pszName, void* pvBuf, long lSize)
{
    STUB_DESC("not implemented");
    return qfalse;
}

/*
==============
LoadRegistryInfo
==============
*/
qboolean LoadRegistryInfo(qboolean user, const char* pszName, void* pvBuf, long* plSize)
{
    STUB_DESC("not implemented");
    return qfalse;
}

/*
==============
IsFirstRun
==============
*/
qboolean IsFirstRun(void)
{
    STUB_DESC("wtf");
    return qfalse;
}

/*
==============
IsNewConfig
==============
*/
qboolean IsNewConfig(void)
{
    STUB_DESC("wtf");
    return qfalse;
}

/*
==============
IsSafeMode
==============
*/
qboolean IsSafeMode(void)
{
    STUB_DESC("wtf");
    return qfalse;
}

/*
==============
ClearNewConfigFlag
==============
*/
void ClearNewConfigFlag(void)
{
}

/*
==============
Sys_GetWholeClipboard
==============
*/
const char* Sys_GetWholeClipboard(void)
{
    return NULL;
}

/*
==============
Sys_SetClipboard
==============
*/
void Sys_SetClipboard(const char* contents)
{
}

/*
================
RecoverLostAutodialData
================
*/
void RecoverLostAutodialData(void)
{
    // FIXME: stub
}

/*
==============
Sys_CloseMutex
==============
*/
void Sys_CloseMutex(void)
{
    // FIXME: stub
}

/*
=================
Sys_UnloadGame
=================
*/
void Sys_UnloadGame(void)
{
    Com_Printf("------ Unloading Game ------\n");

    if (game_library) {
        Sys_UnloadLibrary(game_library);
    }

    game_library = NULL;
}

/*
=================
Sys_GetGameAPI
=================
*/
void* Sys_GetGameAPI(void* parms)
{
    void* (*GetGameAPI) (void*);
    const char* basepath;
    const char* cdpath;
    const char* gamedir;
    const char* homepath;
#ifdef MACOS_X
    const char* apppath;
#endif
    const char* fn;
    const char* gamename = "game" ARCH_STRING DLL_SUFFIX DLL_EXT;

    if (game_library)
        Com_Error(ERR_FATAL, "Sys_GetGameAPI without calling Sys_UnloadGame");

    // check the current debug directory first for development purposes
    homepath = Cvar_VariableString("fs_homepath");
    basepath = Cvar_VariableString("fs_basepath");
    cdpath = Cvar_VariableString("fs_cdpath");
    gamedir = Cvar_VariableString("fs_game");
#ifdef MACOS_X
    apppath = Cvar_VariableString("fs_apppath");
#endif

    fn = FS_BuildOSPath(basepath, gamedir, gamename);

    game_library = Sys_LoadLibrary(fn);

    //First try in mod directories. basepath -> homepath -> cdpath
    if (!game_library) {
        if (homepath[0]) {
            Com_Printf("Sys_GetGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError());
            fn = FS_BuildOSPath(homepath, gamedir, gamename);
            game_library = Sys_LoadLibrary(fn);
        }
    }
#ifdef MACOS_X
    if (!game_library) {
        if (apppath[0]) {
            Com_Printf("Sys_GetGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError());
            fn = FS_BuildOSPath(apppath, gamedir, gamename);
            game_library = Sys_LoadLibrary(fn);
        }
    }
#endif
    if (!game_library) {
        if (cdpath[0]) {
            Com_Printf("Sys_GetGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError());
            fn = FS_BuildOSPath(cdpath, gamedir, gamename);
            game_library = Sys_LoadLibrary(fn);
        }
    }

    //Now try in base. basepath -> homepath -> cdpath
    if (!game_library) {
        Com_Printf("Sys_GetGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError());
        fn = FS_BuildOSPath(basepath, PRODUCT_NAME, gamename);
        game_library = Sys_LoadLibrary(fn);
    }

    if (!game_library) {
        if (homepath[0]) {
            Com_Printf("Sys_GetGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError());
            fn = FS_BuildOSPath(homepath, PRODUCT_NAME, gamename);
            game_library = Sys_LoadLibrary(fn);
        }
    }
#ifdef MACOS_X
    if (!game_library) {
        if (apppath[0]) {
            Com_Printf("Sys_GetGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError());
            fn = FS_BuildOSPath(apppath, OPENJKGAME, gamename);
            game_library = Sys_LoadLibrary(fn);
        }
    }
#endif
    if (!game_library) {
        if (cdpath[0]) {
            Com_Printf("Sys_GetGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError());
            fn = FS_BuildOSPath(cdpath, PRODUCT_NAME, gamename);
            game_library = Sys_LoadLibrary(fn);
        }
    }

    //Still couldn't find it.
    if (!game_library) {
        Com_Printf("Sys_GetGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError());
        Com_Error(ERR_FATAL, "Couldn't load game");
    }

    Com_Printf("Sys_GetGameAPI(%s): succeeded ...\n", fn);
    GetGameAPI = (void* (*)(void*))Sys_LoadFunction(game_library, "GetGameAPI");

    if (!GetGameAPI)
    {
        Sys_UnloadGame();
        return NULL;
    }

    return GetGameAPI(parms);
}

/*
=================
Sys_UnloadCGame
=================
*/
void Sys_UnloadCGame(void)
{
    Com_Printf("------ Unloading ClientGame ------\n");

    if (cgame_library) {
        Sys_UnloadLibrary(cgame_library);
    }

    cgame_library = NULL;
}

/*
=================
Sys_GetCGameAPI
=================
*/
void* Sys_GetCGameAPI(void* parms)
{
    void* (*GetCGameAPI) (void*);
    const char* basepath;
    const char* cdpath;
    const char* gamedir;
    const char* homepath;
#ifdef MACOS_X
    const char* apppath;
#endif
    const char* fn;
    const char* gamename = "cgame" ARCH_STRING DLL_EXT;

    if (cgame_library)
        Com_Error(ERR_FATAL, "Sys_GetCGameAPI without calling Sys_UnloadCGame");

    // check the current debug directory first for development purposes
    homepath = Cvar_VariableString("fs_homepath");
    basepath = Cvar_VariableString("fs_basepath");
    cdpath = Cvar_VariableString("fs_cdpath");
    gamedir = Cvar_VariableString("fs_game");
#ifdef MACOS_X
    apppath = Cvar_VariableString("fs_apppath");
#endif

    fn = FS_BuildOSPath(basepath, gamedir, gamename);

    cgame_library = Sys_LoadLibrary(fn);

    //First try in mod directories. basepath -> homepath -> cdpath
    if (!cgame_library) {
        if (homepath[0]) {
            Com_Printf("Sys_GetCGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError());
            fn = FS_BuildOSPath(homepath, gamedir, gamename);
            cgame_library = Sys_LoadLibrary(fn);
        }
    }
#ifdef MACOS_X
    if (!cgame_library) {
        if (apppath[0]) {
            Com_Printf("Sys_GetCGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError());
            fn = FS_BuildOSPath(apppath, gamedir, gamename);
            cgame_library = Sys_LoadLibrary(fn);
        }
    }
#endif
    if (!cgame_library) {
        if (cdpath[0]) {
            Com_Printf("Sys_GetCGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError());
            fn = FS_BuildOSPath(cdpath, gamedir, gamename);
            cgame_library = Sys_LoadLibrary(fn);
        }
    }

    //Now try in base. basepath -> homepath -> cdpath
    if (!cgame_library) {
        Com_Printf("Sys_GetCGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError());
        fn = FS_BuildOSPath(basepath, PRODUCT_NAME, gamename);
        cgame_library = Sys_LoadLibrary(fn);
    }

    if (!cgame_library) {
        if (homepath[0]) {
            Com_Printf("Sys_GetCGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError());
            fn = FS_BuildOSPath(homepath, PRODUCT_NAME, gamename);
            cgame_library = Sys_LoadLibrary(fn);
        }
    }
#ifdef MACOS_X
    if (!cgame_library) {
        if (apppath[0]) {
            Com_Printf("Sys_GetCGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError());
            fn = FS_BuildOSPath(apppath, OPENJKGAME, gamename);
            cgame_library = Sys_LoadLibrary(fn);
        }
    }
#endif
    if (!cgame_library) {
        if (cdpath[0]) {
            Com_Printf("Sys_GetCGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError());
            fn = FS_BuildOSPath(cdpath, PRODUCT_NAME, gamename);
            cgame_library = Sys_LoadLibrary(fn);
        }
    }

    //Still couldn't find it.
    if (!cgame_library) {
        Com_Printf("Sys_GetCGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError());
        Com_Error(ERR_FATAL, "Couldn't load game");
    }

    Com_Printf("Sys_GetCGameAPI(%s): succeeded ...\n", fn);
    GetCGameAPI = (void* (*)(void*))Sys_LoadFunction(cgame_library, "GetCGameAPI");

    if (!GetCGameAPI)
    {
        Sys_UnloadCGame();
        return NULL;
    }

    return GetCGameAPI(parms);
}

void VM_Forced_Unload_Start(void) {
}

void VM_Forced_Unload_Done(void) {
}

void Sys_InitEx()
{
    Sys_InitLocalization();
}
