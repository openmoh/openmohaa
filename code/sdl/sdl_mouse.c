/*
===========================================================================
Copyright (C) 2025 the OpenMoHAA team

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

#ifdef USE_INTERNAL_SDL_HEADERS
#    include "SDL.h"
#else
#    include <SDL.h>
#endif

#include "../qcommon/qcommon.h"

static SDL_Cursor *cursor = NULL;
static SDL_Surface *cursor_surface = NULL;
static byte *cursor_image_data = NULL;
static pCursorFree cursor_free = NULL;

void IN_GetMousePosition(int *x, int *y) {
    SDL_GetMouseState(x, y);
}

qboolean IN_SetCursorFromImage(const byte *pic, int width, int height, pCursorFree cursorFreeFn) {
    IN_FreeCursor();

    cursor_surface = SDL_CreateRGBSurfaceWithFormatFrom(pic, width, height, 32, 4 * width, SDL_PIXELFORMAT_ABGR8888);
    if (!cursor_surface) {
        return qfalse;
    }

    cursor = SDL_CreateColorCursor(cursor_surface, 0, 0);
    SDL_SetCursor(cursor);

    return qtrue;
}

void IN_FreeCursor() {
    if (cursor) {
        SDL_FreeCursor(cursor);
    }
    if (cursor_surface) {
        SDL_FreeSurface(cursor_surface);
    }
    if (cursor_image_data) {
        cursor_free(cursor_image_data);
        cursor_image_data = NULL;
    }
}

qboolean IN_IsCursorActive()
{
    return SDL_GetRelativeMouseMode() == SDL_TRUE;
}
