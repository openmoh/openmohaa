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

#include <glb_local.h>
#include <mem_tempalloc.h>

MEM_TempAlloc::MEM_TempAlloc()
{
	m_CurrentMemoryBlock = NULL;
}

void *MEM_TempAlloc::Alloc( size_t len )
{
	unsigned char *prev_block = m_CurrentMemoryBlock;
	unsigned char *result;

	if( m_CurrentMemoryBlock && m_CurrentMemoryPos + len <= 65536 )
	{
		result = m_CurrentMemoryBlock + m_CurrentMemoryPos + sizeof( unsigned char * );
		m_CurrentMemoryPos += len;
	}
	else
	{
		m_CurrentMemoryPos = len;

		if( len < 65536 )
			len = 65536;

		m_CurrentMemoryBlock = ( unsigned char * )glbs.Malloc( len + sizeof( unsigned char * ) );
		*( unsigned char ** )m_CurrentMemoryBlock = prev_block;
		result = m_CurrentMemoryBlock + sizeof( unsigned char * );
	}

	return result;
}

void MEM_TempAlloc::FreeAll( void )
{
	unsigned char *prev_block;

	while( m_CurrentMemoryBlock )
	{
		prev_block = *( unsigned char ** )m_CurrentMemoryBlock;
		glbs.Free( m_CurrentMemoryBlock );
		m_CurrentMemoryBlock = prev_block;
	}
}
