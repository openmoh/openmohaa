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

// mem_tempalloc.cpp: Fast temporary memory manager

#include <mem_tempalloc.h>

#ifdef GAME_DLL
#include "g_local.h"

#define MEM_TempAllocate(x) gi.Malloc(x)
#define MEM_TempFree(x)     gi.Free(x)

#elif defined(CGAME_DLL)

#include "cg_local.h"

#define MEM_TempAllocate(x) cgi.Malloc(x)
#define MEM_TempFree(x)     cgi.Free(x)

#else
#include "qcommon.h"

#define MEM_TempAllocate(x) Z_Malloc(x)
#define MEM_TempFree(x)     Z_Free(x)
#endif

class tempBlock_t
{
public:
    void* GetData();
    void* GetData(size_t pos);

public:
    tempBlock_t* prev;
};

MEM_TempAlloc::MEM_TempAlloc()
{
    m_CurrentMemoryBlock = nullptr;
    m_CurrentMemoryPos = 0;
    m_BlockSize = 0;
    m_LastPos = 0;
}

void* MEM_TempAlloc::Alloc(size_t len)
{
    if (m_CurrentMemoryBlock && m_CurrentMemoryPos + len <= m_BlockSize) {
        void* data = m_CurrentMemoryBlock->GetData(m_CurrentMemoryPos);
        m_LastPos = m_CurrentMemoryPos;
        m_CurrentMemoryPos += len;
        return data;
    } else {
        return CreateBlock(len);
    }
}

void* MEM_TempAlloc::Alloc(size_t len, size_t alignment)
{
    if (m_CurrentMemoryBlock) {
        if (m_CurrentMemoryPos % alignment != 0) {
            m_CurrentMemoryPos += alignment - m_CurrentMemoryPos % alignment;
        }

        if (m_CurrentMemoryPos + len <= m_BlockSize) {
            void* data = m_CurrentMemoryBlock->GetData(m_CurrentMemoryPos);
            m_LastPos = m_CurrentMemoryPos;
            m_CurrentMemoryPos += len;
            return data;
        }
    }

    return CreateBlock(len);
}

void MEM_TempAlloc::FreeAll(void)
{
    while (m_CurrentMemoryBlock) {
        tempBlock_t* prev_block = m_CurrentMemoryBlock->prev;
        MEM_TempFree(m_CurrentMemoryBlock);
        m_CurrentMemoryBlock = prev_block;
    }
}

void* MEM_TempAlloc::CreateBlock(size_t len)
{
    m_CurrentMemoryPos = len;

    // allocate a new block
    tempBlock_t* prev_block = m_CurrentMemoryBlock;
    m_CurrentMemoryBlock = (tempBlock_t*)MEM_TempAllocate(
        sizeof(tempBlock_t) + max(m_BlockSize, len));
    m_CurrentMemoryBlock->prev = prev_block;
    return m_CurrentMemoryBlock->GetData();
}

void* tempBlock_t::GetData() { return (void*)(this + 1); }

void* tempBlock_t::GetData(size_t pos) { return (uint8_t*)(this + 1) + pos; }
