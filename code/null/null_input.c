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

static cvar_t in_mouse_real;
cvar_t* in_mouse = NULL;

void IN_Init(void *windowsData) {
    memset(&in_mouse_real, 0, sizeof(in_mouse_real));
    in_mouse_real.integer = 0;
    in_mouse = &in_mouse_real;
}

void IN_Restart( void ) {
}

void IN_Frame (void) {
}

void IN_Shutdown( void ) {
}

void Sys_SendKeyEvents (void) {
}

void Key_KeynameCompletion( void( *callback )( const char *s ) ) {
}

void IN_Activate(qboolean active) {
}

void IN_MouseEvent(int mstate) {

}