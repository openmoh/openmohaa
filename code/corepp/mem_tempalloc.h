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

// mem_tempalloc.h: Fast temporary memory manager

#pragma once

#include <cstddef>

class tempBlock_t;

class MEM_TempAlloc
{
public:
    MEM_TempAlloc();

    void *Alloc(size_t len);
    void *Alloc(size_t len, size_t alignment);
    void  FreeAll(void);
    // This was added to fix issues with alignment
    void *CreateBlock(size_t len);

private:
    tempBlock_t *m_CurrentMemoryBlock;
    size_t       m_CurrentMemoryPos;
    size_t       m_BlockSize;
    size_t       m_LastPos;
};
