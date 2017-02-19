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

// dbgheap.h: Heap Debugger.

#ifndef __DBGHEAP_H__
#define __DBGHEAP_H__

#include <stdio.h>
#include <stdlib.h>

#define DBG_HEAP_HASHSIZE			4096
#define DBG_HEAP_MAX_CALLSTACK		16

typedef struct refptr {
	void			*ptr;
	unsigned char	refcount;
	void			*callstack[ DBG_HEAP_MAX_CALLSTACK ];
	struct refptr	*next;
} refptr_t;

class DbgHeap
{
private:
	static refptr_t		*hashTable[ DBG_HEAP_HASHSIZE ];

private:
	refptr_t	*FindReference( void *ptr );

public:
	DbgHeap();
	~DbgHeap();

	void		ReferencePointer( void *ptr );
	void		DereferencePointer( void *ptr );
};


extern void *operator new( size_t size );
extern void *operator new[]( size_t size );
extern void	operator delete( void *ptr );
extern void	operator delete[]( void *ptr );

#ifdef _DEBUG_MEM
extern void *operator new( size_t size, const char* file, int line );
extern void *operator new[]( size_t size, const char* file, int line );
extern void	operator delete( void *ptr, const char* file, int line );
extern void	operator delete[]( void *ptr, const char* file, int line );

#define DEBUG_NEW new(__FILE__, __LINE__)
#define new DEBUG_NEW
#endif

extern DbgHeap m_Heap;

#endif /* dbgheap.h */
