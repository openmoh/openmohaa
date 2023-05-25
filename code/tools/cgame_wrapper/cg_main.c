/*
===========================================================================
Copyright (C) 2012 Michael Rieder

This file is part of OpenMohaa source code.

OpenMohaa source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMohaa source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMohaa source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "qcommon.h"
#ifndef __linux__
#include <Windows.h>
// original cgame dll
HMODULE cg_dll;
#else
#include <dlfcn.h>
void	*cg_so;
#endif

// interface function
typedef clientGameExport_t* (*cgapi)();

// cgame definitions
clientGameExport_t cge;
clientGameExport_t cge_out;
clientGameImport_t cgi;
clientGameImport_t cgi_out;

snapshot_t	emptySnap;
snapshot_t	*snapshot = &emptySnap;

void CG_Init( clientGameImport_t *imported, int serverMessageNum, int serverCommandSequence, int clientNum ) {
	memcpy( &cgi, imported, sizeof(cgi) );
	memcpy( &cgi_out, &cgi, sizeof(cgi) );

	facfont = cgi.R_LoadFont( "facfont-20" );

	// Reroute import functions
	cgi_out.R_RegisterModel			= R_RegisterModel;
	cgi_out.TIKI_FindTiki			= TIKI_FindTiki;
	cgi_out.R_Model_GetHandle		= R_Model_GetHandle;
	// su44: extra TIKI function detours, not really needed, but usefull for debugging
#if 1
	cgi_out.TIKI_NumAnims = TIKI_NumAnims;
	cgi_out.TIKI_CalculateBounds = TIKI_CalculateBounds;
	cgi_out.TIKI_Name = TIKI_Name;
	cgi_out.TIKI_GetSkeletor = TIKI_GetSkeletor;
	cgi_out.TIKI_SetEyeTargetPos = TIKI_SetEyeTargetPos;
	cgi_out.Anim_NameForNum = Anim_NameForNum;
	cgi_out.Anim_NumForName = Anim_NumForName;
	cgi_out.Anim_Random = Anim_Random;
	cgi_out.Anim_NumFrames = Anim_NumFrames;
	cgi_out.Anim_Time = Anim_Time;
	cgi_out.Anim_Frametime = Anim_Frametime;
// WARNING: Anim_Delta might be NULL pointer in MOHAA
	if(cgi.Anim_Delta) {
		cgi_out.Anim_Delta = Anim_Delta;
	}
	cgi_out.Anim_Flags = Anim_Flags;
	cgi_out.Anim_FlagsSkel = Anim_FlagsSkel;
	cgi_out.Anim_CrossblendTime = Anim_CrossblendTime;
	cgi_out.Anim_HasCommands = Anim_HasCommands;
	cgi_out.Frame_Commands = Frame_Commands;
	cgi_out.Frame_CommandsTime = Frame_CommandsTime;
	cgi_out.Surface_NameToNum = Surface_NameToNum;
	cgi_out.Tag_NumForName = Tag_NumForName;
	cgi_out.Tag_NameForNum = Tag_NameForNum;
	cgi_out.ForceUpdatePose = ForceUpdatePose;
	cgi_out.TIKI_Orientation = TIKI_Orientation;
	cgi_out.TIKI_IsOnGround = TIKI_IsOnGround;
#endif
	cgi_out.R_AddRefEntityToScene	= R_AddRefEntityToScene;
	cgi_out.GetSnapshot				= GetSnapshot;

	// Call original function
	cge.CG_Init(&cgi_out, serverMessageNum, serverCommandSequence, clientNum);

	// su44: init cvars from cg_skeletor.c
	CG_InitSkeletorCvarsAndCmds();

	cgi.Printf( "\n" );
	cgi.Printf( "=============================================\n" );
	cgi.Printf( "MOHAA cgame wrapper loaded successfully.\n" );
	cgi.Printf( "=============================================\n" );
	cgi.Printf( "\n" );
}

void CG_Shutdown () {
	cge.CG_Shutdown();
#ifndef __linux__
	FreeLibrary( cg_dll );
#else
	dlclose(cg_so);
#endif
}

#ifndef __linux__
__declspec(dllexport) clientGameExport_t *GetCGameAPI() {
	cgapi				cg_dll_proc;
	DWORD				err;
	clientGameExport_t	*ret;

	// Load original DLL
	cg_dll = LoadLibrary( "main\\cgamex86mohaa.dll" );
	if (cg_dll == NULL) {
		err = GetLastError();
		return NULL;
	}

	cg_dll_proc = (cgapi)GetProcAddress( cg_dll, "GetCGameAPI" );
	if (cg_dll_proc == NULL) {
		err = GetLastError();
		return NULL;
	}

	ret = cg_dll_proc();
#else
clientGameExport_t *GetCGameAPI() {
	cgapi				cg_so_proc;
	char				*err;
	clientGameExport_t	*ret;

	cg_so = dlopen("cgame.so", RTLD_LAZY);
	if (cg_so == NULL) {
		err = dlerror();
		return NULL;
	}

	dlerror();

	cg_so_proc = dlsym( cg_so, "GetCGameAPI" );
	err = dlerror();
	if (err != NULL) {
		return NULL;
	}

	ret = cg_so_proc();
#endif

	// Call original GetCGameAPI to get the pointers
	memcpy( &cge, ret, sizeof(cge) );
	memcpy( &cge_out, &cge, sizeof(cge) );

	// reroute exported functions
	cge_out.CG_Init				= CG_Init;
	cge_out.CG_Draw2D			= CG_Draw2D;
	cge_out.CG_DrawActiveFrame	= CG_DrawActiveFrame;
	cge_out.CG_ConsoleCommand	= CG_ConsoleCommand;
	cge_out.CG_Shutdown			= CG_Shutdown;

	return &cge_out;
}

void Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	cgi.Error( "%s", text);
}

void Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	cgi.Printf ("%s", text);
}

