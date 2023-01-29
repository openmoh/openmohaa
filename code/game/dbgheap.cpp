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

// dbgheap.cpp: Heap Debugger.

#include <glb_local.h>
#include <dbgheap.h>

DbgHeap m_Heap;

refptr_t *DbgHeap::hashTable[ DBG_HEAP_HASHSIZE ];

#ifdef WIN32

extern "C" {

typedef struct _SYMBOL_INFO {
	unsigned long		SizeOfStruct;
	unsigned long		TypeIndex;
	unsigned __int64	Reserved[ 2 ];
	unsigned long		Index;
	unsigned long		Size;
	unsigned __int64	ModBase;
	unsigned long		Flags;
	unsigned __int64	Value;
	unsigned __int64	Address;
	unsigned long		Register;
	unsigned long		Scope;
	unsigned long		Tag;
	unsigned long		NameLen;
	unsigned long		MaxNameLen;
	char				Name[ 1 ];
} SYMBOL_INFO, *PSYMBOL_INFO;

unsigned short __stdcall RtlCaptureStackBackTrace
	(
	unsigned long FramesToSkip,
	unsigned long FramesToCapture,
	void *BackTrace,
	unsigned long *BackTraceHash
	);

void * __stdcall GetCurrentProcess
	(
	void
	);

unsigned int __stdcall SymInitialize
	(
	void		*hProcess,
	const char	*UserSearchPath,
	int			fInvadeProcess
	);


unsigned int __stdcall SymFromAddr
	(
	void				*hProcess,
	unsigned long long	Address,
	unsigned long long	*Displacement,
	PSYMBOL_INFO		Symbol
	);
}

#endif

DbgHeap::DbgHeap()
{

}

DbgHeap::~DbgHeap()
{
/*
	refptr_t *rootptr;
	refptr_t *ref;
	refptr_t *next;

	for( int i = 0; i < DBG_HEAP_HASHSIZE; i++ )
	{
		rootptr = hashTable[ i ];

		for( ref = rootptr; ref != NULL; ref = next )
		{
			next = ref->next;
			free( ref );
		}
	}

	rootptr = NULL;
*/
}

refptr_t *DbgHeap::FindReference
	(
	void *ptr
	)

{
	refptr_t *ref;
	refptr_t *rootptr;

	rootptr = hashTable[ ( ( size_t )ptr / 8 ) % DBG_HEAP_HASHSIZE ];

	for( ref = rootptr; ref != NULL; ref = ref->next )
	{
		if( ref->ptr == ptr )
			return ref;
	}

	return NULL;
}

void DbgHeap::ReferencePointer
	(
	void *ptr
	)

{
	return;

	refptr_t *ref = FindReference( ptr );
	refptr_t **rootptr;

	if( !ref )
	{
		ref = ( refptr_t * )malloc( sizeof( refptr_t ) );

		ref->ptr = ptr;
		ref->refcount = 0;
		ref->next = NULL;

#ifdef WIN32
		RtlCaptureStackBackTrace( 1, DBG_HEAP_MAX_CALLSTACK, ref->callstack, NULL );
#endif

		rootptr = &hashTable[ ( ( size_t )ptr / 8 ) % DBG_HEAP_HASHSIZE ];

		if( !*rootptr )
		{
			*rootptr = ref;
		}
		else
		{
			ref->next = *rootptr;
			*rootptr = ref;
		}
	}

	// Increase the reference count
	ref->refcount++;
}

void DbgHeap::DereferencePointer
	(
	void *ptr
	)

{
	return;

	refptr_t *ref = FindReference( ptr );

	assert( ref );
	if( !ref )
	{
		// Trying to dereference a non-existing pointer
		exit( EXIT_FAILURE );
		return;
	}

	assert( ref->refcount > 0 );
	if( !ref->refcount )
	{
		// Trying to dereference an unreferenced pointer
		exit( EXIT_FAILURE );
		return;
	}

	ref->refcount--;

	/*
	if( !ref->refcount )
	{
		refptr_t *r;
		refptr_t *rootptr;

		rootptr = hashTable[ ( ( size_t )ptr / 8 ) % DBG_HEAP_HASHSIZE ];

		if( ref != rootptr )
		{
			for( r = rootptr; r != NULL; r = r->next )
			{
				if( r->ptr == ptr )
				{
					ref->next = r->next;
					break;
				}
				ref = r;
			}
		}
		else
		{
			hashTable[ ( ( size_t )ptr / 8 ) % DBG_HEAP_HASHSIZE ] = ref->next;
		}
	}
	*/

#ifdef WIN32
	RtlCaptureStackBackTrace( 1, DBG_HEAP_MAX_CALLSTACK, ref->callstack, NULL );
#endif
}

/*
void *operator new( size_t size )
{
	void *ptr = malloc( size );
	m_Heap.ReferencePointer( ptr );
	return ptr;
}

void *operator new[]( size_t size )
{
	void *ptr = malloc( size );
	m_Heap.ReferencePointer( ptr );
	return ptr;
}

void operator delete( void *ptr )
{
	m_Heap.DereferencePointer( ptr );
	free( ptr );
}

void operator delete[]( void *ptr )
{
	m_Heap.DereferencePointer( ptr );
	free( ptr );
}
*/

#ifdef _DEBUG_MEM

#undef new

void *operator new( size_t size, const char* file, int line )
{
	return _malloc_dbg( size, _NORMAL_BLOCK, file, line );
}

void *operator new[]( size_t size, const char* file, int line )
{
	return _malloc_dbg( size, _NORMAL_BLOCK, file, line );
}

void operator delete( void *ptr, const char* file, int line )
{
	return _free_dbg( ptr, _NORMAL_BLOCK );
}

void operator delete[]( void *ptr, const char* file, int line )
{
	return _free_dbg( ptr, _NORMAL_BLOCK );
}

#endif

