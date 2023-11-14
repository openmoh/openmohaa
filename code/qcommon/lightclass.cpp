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

#include "lightclass.h"

#if defined(GAME_DLL)

#    include "../fgame/g_local.h"

#    define LIGHTCLASS_Printf  gi.Printf
#    define LIGHTCLASS_DPrintf gi.DPrintf
#    define LIGHTCLASS_Error   gi.Error

#elif defined(CGAME_DLL)

#    include "../cgame/cg_local.h"

#    define LIGHTCLASS_Printf  cgi.Printf
#    define LIGHTCLASS_DPrintf cgi.DPrintf
#    define LIGHTCLASS_Error   cgi.Error

#else

#    include "qcommon.h"

#    define LIGHTCLASS_Printf  Com_Printf
#    define LIGHTCLASS_DPrintf Com_DPrintf
#    define LIGHTCLASS_Error   Com_Error

#endif

size_t       totalmemallocated = 0;
unsigned int numclassesallocated = 0;

void DisplayMemoryUsage() {
    LIGHTCLASS_Printf("Classes %-5d Class memory used: %zu\n", numclassesallocated, totalmemallocated);
}

#ifndef _DEBUG_MEM
void *LightClass::operator new(size_t s)
{
    size_t *p;

    if (s == 0) {
        static void *empty_memory = nullptr;
        return &empty_memory;
    }

    s += sizeof(size_t);

#    ifdef GAME_DLL
    p = (size_t *)gi.Malloc(s);
#    elif defined(CGAME_DLL)
    p = (size_t *)cgi.Malloc(s);
#    else
    p = (size_t *)Z_Malloc(s);
#    endif

    totalmemallocated += s;
    numclassesallocated++;

    p++;

    return p;
}

void LightClass::operator delete(void *ptr)
{
    size_t *p = ((size_t *)ptr) - 1;

    totalmemallocated -= *p;
    numclassesallocated--;

#    ifdef GAME_DLL
    gi.Free(p);
#    elif defined(CGAME_DLL)
    cgi.Free(p);
#    else
    Z_Free(p);
#    endif
}
#endif
