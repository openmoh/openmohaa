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

#include "../client/client.h"

cvar_t *cl_shownet;
clientStatic_t cls;
clientActive_t cl;
clientGameExport_t *cge;

void CL_Shutdown( void ) {
}

void CL_Init( void ) {
	cl_shownet = Cvar_Get ("cl_shownet", "0", CVAR_TEMP );

	memset( &cls, 0, sizeof( clientStatic_t ) );
	memset( &cl, 0, sizeof( clientActive_t ) );
	cge = NULL;
}

void CL_InitClientSavedData( void ) {
}

void CL_MouseEvent( int dx, int dy, int time ) {
}

void CL_ClearButtons( void ) {
}

void Key_WriteBindings( fileHandle_t f ) {
}

void CL_Frame ( int msec ) {
}

void CL_PacketEvent( netadr_t from, msg_t *msg ) {
}

void CL_CharEvent( int key ) {
}

void CL_Disconnect( qboolean showMainMenu ) {
}

void CL_MapLoading( qboolean flush, const char *pszMapName ) {
}

qboolean CL_GameCommand( void ) {
  return qfalse;
}

void CL_KeyEvent (int key, qboolean down, unsigned time) {
}

qboolean UI_GameCommand( void ) {
	return qfalse;
}

void CL_ForwardCommandToServer( const char *string ) {
}

void CL_ConsolePrint( char *txt ) {
}

void CL_JoystickEvent( int axis, int value, int time ) {
}

void CL_InitKeyCommands( void ) {
}

void CL_CDDialog( void ) {
}

void CL_FlushMemory( void ) {
}

void CL_StartHunkUsers( void ) {
}

void CL_ShutdownAll(void) {}

qboolean CL_CDKeyValidate( const char *key, const char *checksum ) { return qtrue; }

void UI_ClearResource( void ) {
}

void UI_LoadResource( const char *name ) {
}

const char *Key_KeynumToString( int keynum ) {
	return NULL;
}

const char *Key_KeynumToBindString( int keynum ) {
	return NULL;
}

void Key_GetKeysForCommand( const char *command, int *key1, int *key2 ) {
}

void SCR_DebugGraph( float value, int color ) {
}

qboolean CL_FinishedIntro( void ) {
	return qtrue;
}
