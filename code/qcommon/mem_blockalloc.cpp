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

// mem_blockalloc.cpp: Fast block memory manager

#if defined(GAME_DLL)

#    include "../fgame/g_local.h"

void *MEM_Alloc(int size)
{
    return gi.Malloc(size);
}

void MEM_Free(void *ptr)
{
    return gi.Free(ptr);
}

#elif defined(CGAME_DLL)

#    include "../cgame/cg_local.h"

void *MEM_Alloc(int size)
{
    return cgi.Malloc(size);
}

void MEM_Free(void *ptr)
{
    return cgi.Free(ptr);
}

#else

#    include "qcommon.h"

void *MEM_Alloc(int size)
{
    return Z_Malloc(size);
}

void MEM_Free(void *ptr)
{
    return Z_Free(ptr);
}

#endif
