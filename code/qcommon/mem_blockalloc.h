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

// mem_blockalloc.h: Fast block memory manager

#ifndef __MEM_BLOCKALLOC_H__
#define __MEM_BLOCKALLOC_H__

#include <linklist.h>
#include "dbgheap.h"

#define MEM_BLOCKSIZE			char[ 256 ]

void *MEM_Alloc( int size );
void MEM_Free( void *ptr );

template< typename aclass, typename blocksize >
class MEM_BlockAlloc_enum;

template< typename aclass, typename blocksize >
class block_s {
public:
#ifndef _DEBUG_MEMBLOCK
	unsigned char	data[ sizeof( blocksize ) ][ sizeof( aclass ) + 8 ];
	unsigned char	prev_data[ sizeof( blocksize ) ];
	unsigned char	next_data[ sizeof( blocksize ) ];

	short int		free_data;
	short int		used_data;
#else
	unsigned char	data[ sizeof( aclass ) ];
#endif

	block_s< aclass, blocksize >	*prev_block;
	block_s< aclass, blocksize >	*next_block;
};

template< typename aclass, typename blocksize >
class MEM_BlockAlloc {
	friend class MEM_BlockAlloc_enum< aclass, blocksize >;

#ifndef _DEBUG_MEMBLOCK
	// Free block list [not usable], avoid slow down by using gi.Free
	block_s< aclass, blocksize >	*m_FreeBlock;

	// Starting block list that will be used for further memory allocation
	block_s< aclass, blocksize >	*m_StartUsedBlock;

	// Full block list [not usable], no space available for further memory allocation
	block_s< aclass, blocksize >	*m_StartFullBlock;
#else
	block_s< aclass, blocksize >	*m_Block;
#endif

	unsigned int					m_BlockCount;

private:
	unsigned int Count( block_s< aclass, blocksize > *block );

public:
	MEM_BlockAlloc();

	void				*Alloc( void );
	void				Free( void *ptr );
	void				FreeAll( void );
	unsigned int		Count( void );
	unsigned int		BlockCount( void );
	unsigned int		BlockMemory( void );
};

template< typename aclass, typename blocksize >
class MEM_BlockAlloc_enum {
protected:
	MEM_BlockAlloc< aclass, blocksize >	*m_Owner;
	block_s< aclass, blocksize >		*m_CurrentBlock;
#ifndef _DEBUG_MEMBLOCK
	unsigned char						m_CurrentData;
	int									m_CurrentBlockType;
#endif

public:
	MEM_BlockAlloc_enum( MEM_BlockAlloc< aclass, blocksize >& owner );

	aclass			*NextElement( void );
	aclass			*CurrentElement( void );
};

template< typename a, typename b >
MEM_BlockAlloc< a, b >::MEM_BlockAlloc( void )
{
#ifndef _DEBUG_MEMBLOCK
	m_FreeBlock = NULL;
	m_StartUsedBlock = NULL;
	m_StartFullBlock = NULL;
	m_BlockCount = 0;
#else
	m_Block = NULL;
#endif
}

template< typename a, typename b >
void *MEM_BlockAlloc< a, b >::Alloc( void )
{
#ifdef _DEBUG_MEMBLOCK
	block_s<a, b> *block = ( block_s<a, b> * )malloc( sizeof( block_s <a, b> ) );

	LL_SafeAddFirst( m_Block, block, next_block, prev_block );

	m_BlockCount++;
	return ( void * )block->data;
#else
	unsigned char i;
	block_s< a, b > *block;
	block_s< a, b > *used_block;
	short int free_data;
	short int used_data;
	unsigned char prev_data;
	unsigned char next_data;

	block = m_StartUsedBlock;
	if( block )
	{
		used_block = m_StartUsedBlock;

		free_data = used_block->free_data;
		next_data = used_block->next_data[ free_data ];

		if( next_data == free_data )
		{
			// Move the block to the full block list as there is no space available
			m_StartUsedBlock = block->next_block;

			LL_SafeRemove( block, next_block, prev_block );
			LL_SafeAddFirst( m_StartFullBlock, used_block, next_block, prev_block );

			used_block->free_data = -1;

			goto _ret;
		}
	}
	else
	{
		block = m_FreeBlock;
		if( block )
		{
			m_FreeBlock = NULL;
			used_block = block;
			free_data = block->free_data;
			next_data = block->next_data[ free_data ];
		}
		else
		{
			m_BlockCount++;
			used_block = ( block_s< a, b > * )MEM_Alloc( sizeof( block_s< a, b > ) );

			for( i = sizeof( b ) - 1; i; i-- )
			{
				unsigned char prev = i - 1;
				unsigned char next = prev + 1;
				used_block->data[ prev ][ 0 ] = -82;
				used_block->data[ prev ][ 1 ] = prev;
				*( short * )( &used_block->data[ prev ][ 2 ] ) = sizeof( a );
				used_block->prev_data[ next ] = prev;
				used_block->next_data[ prev ] = next;
			}

			used_block->data[ sizeof( b ) - 1 ][ 0 ] = -82;
			used_block->data[ sizeof( b ) - 1 ][ 1 ] = -1;
			*( short * )( &used_block->data[ sizeof( b ) - 1 ][ 2 ] ) = sizeof( a );
			used_block->prev_data[ 0 ] = -1;
			used_block->next_data[ sizeof( b ) - 1 ] = 0;
			used_block->free_data = 0;
			used_block->used_data = -1;

			free_data = 0;
			next_data = 1;
		}

		LL_SafeAddFirst( m_StartUsedBlock, used_block, next_block, prev_block );
	}

	prev_data = used_block->prev_data[ free_data ];
	used_block->next_data[ prev_data ] = next_data;
	used_block->prev_data[ next_data ] = prev_data;
	used_block->free_data = next_data;

	if( used_block->used_data < 0 )
	{
		used_block->used_data = free_data;
		used_block->next_data[ free_data ] = free_data;
		used_block->prev_data[ free_data ] = free_data;
		return ( void * )&used_block->data[ free_data ][ 8 ];
	}

_ret:
	used_data = used_block->used_data;
	prev_data = used_block->prev_data[ used_data ];
	used_block->prev_data[ used_data ] = free_data;
	used_block->next_data[ prev_data ] = free_data;
	used_block->prev_data[ free_data ] = prev_data;
	used_block->next_data[ free_data ] = used_data;
	return ( void * )&used_block->data[ free_data ][ 8 ];
#endif
}

template< typename a, typename b >
void MEM_BlockAlloc< a, b >::Free( void *ptr )
{
#ifdef _DEBUG_MEMBLOCK
	block_s<a, b> *block = ( block_s<a, b> * )ptr;

	assert( m_Block );
	LL_SafeRemove( block, next_block, prev_block );

	if( m_Block == block )
	{
		m_Block = block->next_block;
	}

	m_BlockCount--;
	free( block );
#else
	block_s< a, b > *block;
	short int free_data;
	short int used_data;
	unsigned char prev_data;
	unsigned char next_data;

	used_data = *( ( unsigned char * )ptr - 7 );
	block = ( block_s< a, b > * )( ( unsigned char * )ptr + -( used_data * ( *( ( unsigned short * )ptr - 3 ) + 8 ) ) - 8 );
	next_data = block->next_data[ used_data ];
	if( next_data == used_data )
	{
		if( m_StartUsedBlock == block )
		{
			m_StartUsedBlock = block->next_block;
		}

		LL_SafeRemove( block, next_block, prev_block );

		if( m_FreeBlock )
		{
			m_BlockCount--;
			MEM_Free( m_FreeBlock );
			m_FreeBlock = NULL;
		}

		m_FreeBlock = block;
		block->used_data = -1;

		free_data = block->free_data;
		prev_data = block->prev_data[ free_data ];

		block->next_data[ prev_data ] = used_data;
		block->prev_data[ free_data ] = used_data;
		block->next_data[ used_data ] = free_data;
		block->prev_data[ used_data ] = prev_data;

		return;
	}

	prev_data = block->prev_data[ used_data ];

	block->next_data[ prev_data ] = next_data;
	block->prev_data[ next_data ] = prev_data;
	block->used_data = next_data;

	if( block->free_data >= 0 )
	{
		free_data = block->free_data;
		prev_data = block->prev_data[ free_data ];

		block->next_data[ prev_data ] = used_data;
		block->prev_data[ free_data ] = used_data;
		block->next_data[ used_data ] = free_data;
		block->prev_data[ used_data ] = prev_data;
		return;
	}

	if( m_StartFullBlock == block )
	{
		m_StartFullBlock = block->next_block;
	}

	LL_SafeRemove( block, next_block, prev_block );
	LL_SafeAddFirst( m_StartUsedBlock, block, next_block, prev_block );

	block->prev_data[ used_data ] = used_data;
	block->next_data[ used_data ] = used_data;
	block->free_data = used_data;
#endif
}

template< typename a, typename b >
void MEM_BlockAlloc< a, b >::FreeAll( void )
{
#ifdef _DEBUG_MEMBLOCK
	block_s<a, b> *block;
	block_s<a, b> *next = m_Block;
	for( block = m_Block; next != NULL; block = next )
	{
		next = block->next_block;
		m_BlockCount--;
		a *ptr = ( a * )block->data;
		ptr->~a();
		free( block );
	}
	m_Block = NULL;
#else
	block_s< a, b > *block;

	block = m_StartFullBlock;
	while( block )
	{
		if( block->used_data >= 0 )
		{
			a *ptr = ( a * )&block->data[ block->used_data ][ 8 ];
			delete ptr;
			block = m_StartFullBlock;
		}
	}

	block = m_StartUsedBlock;
	while( block )
	{
		if( block->used_data >= 0 )
		{
			a *ptr = ( a * )&block->data[ block->used_data ][ 8 ];
			delete ptr;
			block = m_StartUsedBlock;
		}
	}

	if( m_FreeBlock )
	{
		m_BlockCount--;
		MEM_Free( m_FreeBlock );
		m_FreeBlock = NULL;
	}
#endif
}

template< typename a, typename b >
unsigned int MEM_BlockAlloc< a, b >::Count( block_s< a, b > *block )
{
	int count = 0;
#ifdef _DEBUG_MEMBLOCK
	for( ; block != NULL; block = block->next_block )
	{
		count++;
	}
	return count;
#else
	unsigned char used_data;
	unsigned char current_used_data;

	for( ; block != NULL; block = block->next_block )
	{
		used_data = block->used_data;

		if( used_data < 0 )
		{
			continue;
		}

		current_used_data = used_data;

		do
		{
			count++;
			current_used_data = block->next_data[ current_used_data ];
		} while( current_used_data != used_data );
	}

	return count;
#endif
}

template< typename a, typename b >
unsigned int MEM_BlockAlloc< a, b >::Count( void )
{
#ifdef _DEBUG_MEMBLOCK
	return Count( m_Block );
#else
	return Count( m_StartFullBlock ) + Count( m_StartUsedBlock );
#endif
}

template< typename a, typename b >
unsigned int MEM_BlockAlloc< a, b >::BlockCount( void )
{
	return m_BlockCount;
}

template< typename a, typename b >
unsigned int MEM_BlockAlloc< a, b >::BlockMemory( void )
{
	return sizeof( block_s< a, b > );
}


template< typename a, typename b >
MEM_BlockAlloc_enum< a, b >::MEM_BlockAlloc_enum( MEM_BlockAlloc< a, b >& owner )
{
	m_Owner = &owner;
	m_CurrentBlock = NULL;
#ifndef _DEBUG_MEMBLOCK
	m_CurrentBlockType = 0;
#endif
}

template< typename a, typename b >
a *MEM_BlockAlloc_enum< a, b >::NextElement( void )
{
#ifdef _DEBUG_MEMBLOCK
	if( !m_CurrentBlock )
	{
		m_CurrentBlock = m_Owner->m_Block;
	}
	else
	{
		m_CurrentBlock = m_CurrentBlock->next_block;
	}
	return ( a * )m_CurrentBlock;
#else
	block_s< a, b > *block;
	block_s< a, b > *next;

	block = m_CurrentBlock;

	if( block )
	{
		m_CurrentData = block->next_data[ m_CurrentData ];
		if( m_CurrentData == block->used_data )
		{
			block = m_CurrentBlock = block->next_block;
		}
	}

	if( !block )
	{
		while( !block )
		{
			while( !block )
			{
				if( m_CurrentBlockType == 0 )
				{
					block = m_Owner->m_StartFullBlock;
				}
				else if( m_CurrentBlockType == 1 )
				{
					block = m_Owner->m_StartUsedBlock;
				}
				else
				{
					return NULL;
				}

				m_CurrentBlock = block;
				m_CurrentBlockType++;
			}

			while( block->used_data < 0 )
			{
				next = block->next_block;
				block = NULL;
				m_CurrentBlock = next;

				if( !next )
				{
					break;
				}

				block = next;
			}
		}

		m_CurrentData = block->used_data;
	}

	return ( a * )&block->data[ m_CurrentData ][ 8 ];
#endif
}

template< typename a, typename b >
a *MEM_BlockAlloc_enum< a, b >::CurrentElement( void )
{
	return m_CurrentBlock;
}

#endif // __MEM_BLOCKALLOC_H__
