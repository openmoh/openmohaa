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
#include "../sys_update_checker.h"

// a pointer to the last piece of data retrieved from the clipboard is stored here,
// so that it can be cleaned up when new data is retrieved, preventing memory leaks
static void* clipboard_text = NULL;

static void* game_library = NULL;
static void* cgame_library = NULL;
qboolean	GLimp_SpawnRenderThread(void (*function)(void))
{
    return qfalse;
}

void* GLimp_RendererSleep(void)
{
    return NULL;
}

void		GLimp_FrontEndSleep(void)
{
}

void		GLimp_WakeRenderer(void* data)
{
}

/*
==============
Sys_ShowConsole
==============
*/
void Sys_ShowConsole(int visLevel, qboolean quitOnClose)
{
}

/*
==============
SaveRegistryInfo

Used to save product info stuff into the registry.
Not useful anymore, so no need to implement.
==============
*/
qboolean SaveRegistryInfo(qboolean user, const char* pszName, void* pvBuf, long lSize)
{
    return qfalse;
}

/*
==============
LoadRegistryInfo

Used to load product info stuff from the registry.
Not useful anymore, so no need to implement.
==============
*/
qboolean LoadRegistryInfo(qboolean user, const char* pszName, void* pvBuf, long* plSize)
{
    return qfalse;
}

/*
==============
IsFirstRun

Returns whether or not this is the first time the game is started.
Not used anymore.
==============
*/
qboolean IsFirstRun(void)
{
    return qfalse;
}

/*
==============
IsNewConfig

Returns whether or not a new hardware change is detected.
Not used anymore.
==============
*/
qboolean IsNewConfig(void)
{
    return qfalse;
}

/*
==============
IsSafeMode

Returns whether or not the game was started in safe mode.
Not used anymore.
==============
*/
qboolean IsSafeMode(void)
{
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
#ifndef DEDICATED
    char *data = NULL;
    char *cliptext;

    if ((cliptext = SDL_GetClipboardText()) != NULL) {
        if (cliptext[0] != 0) {
            // It's necessary to limit buffersize to 4096 as each character
            // is pasted via CharEvent, which is very-very slow and jams up the EventQueue.
            // A smaller buffer doesn't jam the EventQueue up as much and avoids dropping
            // characters that otherwise happens when the EventQueue is overloaded.
            // FIXME: speed up paste logic so this restriction can be removed
            size_t bufsize = Q_min(strlen(cliptext) + 1, 4096);

            if (clipboard_text != NULL) {
                // clean up previously allocated clipboard buffer
                Z_Free(clipboard_text);
                clipboard_text = NULL;
            }

            data = clipboard_text = Z_Malloc(bufsize);
            // Changed in OPM:
            // original game skips the Windows-specific '\r' (carriage return) char here!
            Q_strncpyz(data, cliptext, bufsize);
        }
        SDL_free(cliptext);
    }
    return data;
#else
    return NULL;
#endif
}

/*
==============
Sys_SetClipboard
==============
*/
void Sys_SetClipboard(const char *contents)
{
#ifndef DEDICATED
    if (!contents || !contents[0]) {
        return;
    }

    SDL_SetClipboardText(contents);
#endif
}

/*
================
RecoverLostAutodialData

This functions changes the current user setting so the modem
automatically dials up whenever an attempt to connect is detected.

There is no need to implement this anymore nowadays.
================
*/
void RecoverLostAutodialData(void)
{
}

/*
==============
Sys_CloseMutex

Closes the global mutex used to check if another instance is already running.
Not used anymore, as multiple instances could be useful for testing.
==============
*/
void Sys_CloseMutex(void)
{
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
    const char* gamename = "game" DLL_SUFFIX DLL_EXT;

    if (game_library)
        Com_Error(ERR_FATAL, "Sys_GetGameAPI without calling Sys_UnloadGame");

    game_library = Sys_LoadDll(gamename, qfalse);

    //Still couldn't find it.
    if (!game_library) {
        Com_Printf("Sys_GetGameAPI(%s) failed: \"%s\"\n", gamename, Sys_LibraryError());
        Com_Error(ERR_FATAL, "Couldn't load game");
    }

    Com_Printf("Sys_GetGameAPI(%s): succeeded ...\n", gamename);
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
    const char* gamename = "cgame" DLL_SUFFIX DLL_EXT;

    if (cgame_library)
        Com_Error(ERR_FATAL, "Sys_GetCGameAPI without calling Sys_UnloadCGame");

    cgame_library = Sys_LoadDll(gamename, qfalse);

    //Still couldn't find it.
    if (!cgame_library) {
        Com_Printf("Sys_GetCGameAPI(%s) failed: \"%s\"\n", gamename, Sys_LibraryError());
        Com_Error(ERR_FATAL, "Couldn't load game");
    }

    Com_Printf("Sys_GetCGameAPI(%s): succeeded ...\n", gamename);
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
    Sys_UpdateChecker_Init();
}

void Sys_ShutdownEx()
{
    Sys_UpdateChecker_Shutdown();
}

void Sys_ProcessBackgroundTasks()
{
    Sys_UpdateChecker_Process();
}
