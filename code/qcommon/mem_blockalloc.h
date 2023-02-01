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

#pragma once

#include "Linklist.h"
#include "q_shared.h"

#include <cstddef>
#include <type_traits>
#include <new>

void *MEM_Alloc( int size );
void MEM_Free( void *ptr );

static constexpr size_t DefaultBlock = 256;

enum class alloc_source_e
{
	SourceBlock = 174,
	SourceMalloc
};

template<typename aclass, size_t blocksize>
class MEM_BlockAlloc_enum;

template<typename aclass, size_t blocksize>
class block_s {
private:
	static constexpr size_t bitsNeeded =
		blocksize <= 0x80 ? 8 :
		blocksize <= 0x8000 ? 16 :
		blocksize <= 0x80000000 ? 32 :
		64;

public:
	block_s();

#if !_DEBUG_MEMBLOCK
	bool usedDataAvailable() const;
	bool freeDataAvailable() const;
#endif

public:
	template<size_t bits>
	struct selectType_t;

	template<> struct selectType_t<8> { using type = uint8_t; };
	template<> struct selectType_t<16> { using type = uint16_t; };
	template<> struct selectType_t<32> { using type = uint32_t; };
	template<> struct selectType_t<64> { using type = uint64_t; };

	using offset_t = typename selectType_t<bitsNeeded>::type;

	struct info_t {
		offset_t index;
		alloc_source_e source;
		static constexpr uint16_t typeSize = sizeof(aclass);
		alignas(alignof(aclass)) char data[sizeof(aclass)];
	};

public:
#if !_DEBUG_MEMBLOCK
	info_t data[blocksize];
	offset_t prev_data[blocksize];
	offset_t next_data[blocksize];

	offset_t free_data;
	offset_t used_data;
	bool has_free_data : 1;
	bool has_used_data : 1;
#else
	offset_t data[sizeof(aclass)];
#endif

	block_s<aclass, blocksize>* prev_block;
	block_s<aclass, blocksize>* next_block;

public:
	static constexpr size_t headersize = offsetof(info_t, data);
	static constexpr size_t dataoffset = 0;
	static constexpr size_t datasize = sizeof(info_t);
};

template<typename aclass, size_t blocksize>
block_s<aclass, blocksize>::block_s()
#if !_DEBUG_MEMBLOCK
{
	info_t* header;
	offset_t curr;
	for (curr = 0; curr < blocksize - 1; ++curr)
	{
		offset_t next = curr + 1;
		header = &data[curr];
		header->source = alloc_source_e::SourceBlock;
		header->index = curr;
		prev_data[next] = curr;
		next_data[curr] = next;
	}

	header = &data[curr];
	header->source = alloc_source_e::SourceBlock;
	header->index = blocksize - 1;
	prev_data[0] = blocksize - 1;
	next_data[blocksize - 1] = 0;
	free_data = 0;
	prev_block = next_block = nullptr;

	has_free_data = true;
	has_used_data = false;
}
#else
	: prev_block(nullptr)
	, next_block(nullptr)
{

}
#endif

#if !_DEBUG_MEMBLOCK
template<typename aclass, size_t blocksize>
bool block_s<aclass, blocksize>::usedDataAvailable() const {
	return has_used_data;
}

template<typename aclass, size_t blocksize>
bool block_s<aclass, blocksize>::freeDataAvailable() const {
	return has_free_data;
}
#endif

template<typename aclass, size_t blocksize = DefaultBlock>
class MEM_BlockAlloc
{
	static_assert(blocksize >= 2, "Minimum 2x class preallocation required!!");
public:
	MEM_BlockAlloc();
	~MEM_BlockAlloc();

	void* Alloc();
	void Free(void* ptr) noexcept;
	void FreeAll() noexcept;
	size_t Count();
	size_t BlockCount();
	size_t BlockMemory();

private:
	friend class MEM_BlockAlloc_enum<aclass, blocksize>;
	using block_t = block_s<aclass, blocksize>;
	using block_offset_t = typename block_t::offset_t;

#if !_DEBUG_MEMBLOCK
	block_t* m_FreeBlock;
	block_t* m_StartUsedBlock;
	block_t* m_StartFullBlock;
#else
	block_t* m_Block;
#endif
	size_t m_BlockCount;

private:
	void* TakeFree(block_t* block, uintptr_t free_data);
	size_t Count(const block_t* block);
};

template<typename aclass, size_t blocksize = DefaultBlock>
class MEM_BlockAlloc_enum
{
public:
	MEM_BlockAlloc_enum(MEM_BlockAlloc<aclass, blocksize>& owner);

	aclass* NextElement();
	aclass* CurrentElement();

	enum blockType_e {
		used,
		full
	};

private:
	using block_t = block_s<aclass, blocksize>;
	using offset_t = typename block_t::offset_t;

	MEM_BlockAlloc<aclass, blocksize>* m_Owner;
	block_t* m_CurrentBlock;

#if !_DEBUG_MEMBLOCK
	offset_t m_CurrentData;
	blockType_e m_CurrentBlockType;
#endif
};

template<typename a, size_t b>
MEM_BlockAlloc<a, b>::MEM_BlockAlloc()
#if !_DEBUG_MEMBLOCK
	: m_StartUsedBlock()
	, m_StartFullBlock()
{
	m_FreeBlock = nullptr;
	m_BlockCount = 0;
}
#else
	: m_Block()
{
	m_BlockCount = 0;
}
#endif

template<typename a, size_t b>
MEM_BlockAlloc<a, b>::~MEM_BlockAlloc()
{
	FreeAll();
}

template<typename a, size_t b>
void* MEM_BlockAlloc<a, b>::Alloc()
{
#if _DEBUG_MEMBLOCK
	block_t* block = new(MEM_Alloc(sizeof(block_t))) block_t();

	m_Block.AddFirst(block);

	m_BlockCount++;
	return (void*)block->data;
#else
	block_t* used_block;
	block_offset_t free_data;
	block_offset_t next_data;

	if (m_StartUsedBlock)
	{
		used_block = m_StartUsedBlock;

		free_data = used_block->free_data;
		next_data = used_block->next_data[free_data];

		if (next_data == free_data)
		{
			// Move the block to the next block list as there is no space available
			m_StartUsedBlock = used_block->next_block;

			LL_SafeRemoveRoot(m_StartUsedBlock, used_block, next_block, prev_block);
			LL_SafeAddFirst(m_StartFullBlock, used_block, next_block, prev_block);

			used_block->has_free_data = false;
			return TakeFree(used_block, free_data);
		}
	}
	else
	{
		if (m_FreeBlock)
		{
			// start from the free block
			used_block = m_FreeBlock;
			m_FreeBlock = nullptr;
			free_data = used_block->free_data;
			next_data = used_block->next_data[free_data];
		}
		else
		{
			m_BlockCount++;
			// allocate and construct a new block
			used_block = new(MEM_Alloc(sizeof(block_t))) block_t();

			free_data = 0;
			next_data = 1;
		}

		LL_SafeAddFirst(m_StartUsedBlock, used_block, next_block, prev_block);
	}

	const block_offset_t prev_data = used_block->prev_data[free_data];

	used_block->next_data[prev_data] = next_data;
	used_block->prev_data[next_data] = prev_data;
	used_block->free_data = next_data;
	used_block->has_free_data = true;

	if (!used_block->usedDataAvailable())
	{
		used_block->used_data = free_data;
		used_block->has_used_data = true;
		used_block->next_data[free_data] = free_data;
		used_block->prev_data[free_data] = free_data;
		return used_block->data[free_data].data;
	}

	return TakeFree(used_block, free_data);
#endif
}

template< typename aclass, size_t blocksize>
void* MEM_BlockAlloc<aclass, blocksize>::TakeFree(block_t* block, uintptr_t free_data)
{
	const block_offset_t used_data = block->used_data;
	const block_offset_t prev_data = block->prev_data[used_data];

	block->next_data[prev_data] = (block_offset_t)free_data;
	block->prev_data[used_data] = (block_offset_t)free_data;
	block->next_data[free_data] = used_data;
	block->prev_data[free_data] = prev_data;
	return block->data[free_data].data;
}

template<typename a, size_t b>
void MEM_BlockAlloc<a, b>::Free(void* ptr) noexcept
{
#if _DEBUG_MEMBLOCK
	block_s<a, b>* block = (block_s<a, b> *)ptr;

	m_Block.Remove(block);

	m_BlockCount--;
	MEM::Free(block);
#else
	// get the header of the pointer
	typename block_t::info_t* header = reinterpret_cast<typename block_t::info_t*>(static_cast<unsigned char*>(ptr) - block_t::headersize);
	const block_offset_t used_data = header->index;
	// get the block from the header
	block_t* const block = (block_t*)((uint8_t*)header - used_data * block_t::datasize - block_t::dataoffset);
	const block_offset_t next_data = block->next_data[used_data];
	if (next_data == used_data)
	{
		LL_SafeRemoveRoot(m_StartUsedBlock, block, next_block, prev_block);

		if (m_FreeBlock)
		{
			// deallocate the free block because of another deallocation
			--m_BlockCount;
			MEM_Free(m_FreeBlock);
			m_FreeBlock = nullptr;
		}

		m_FreeBlock = block;
		block->has_used_data = false;

		const block_offset_t free_data = block->free_data;
		const block_offset_t prev_data = block->prev_data[free_data];

		block->next_data[prev_data] = used_data;
		block->prev_data[free_data] = used_data;
		block->next_data[used_data] = free_data;
		block->prev_data[used_data] = prev_data;
	}
	else
	{
		const block_offset_t prev_data = block->prev_data[used_data];

		block->next_data[prev_data] = next_data;
		block->prev_data[next_data] = prev_data;
		block->used_data = next_data;
		block->has_used_data = true;

		if (block->freeDataAvailable())
		{
			const block_offset_t free_data = block->free_data;
			const block_offset_t prev_data = block->prev_data[free_data];

			block->next_data[prev_data] = used_data;
			block->prev_data[free_data] = used_data;
			block->next_data[used_data] = free_data;
			block->prev_data[used_data] = prev_data;
			return;
		}

		if (m_StartFullBlock == block)
		{
			m_StartFullBlock = block->next_block;
		}

		LL_SafeRemoveRoot(m_StartFullBlock, block, next_block, prev_block);
		LL_SafeAddFirst(m_StartUsedBlock, block, next_block, prev_block);

		block->free_data = used_data;
		block->has_free_data = true;
		block->prev_data[used_data] = used_data;
		block->next_data[used_data] = used_data;
	}
#endif
}

template<typename a, size_t b>
void MEM_BlockAlloc<a, b>::FreeAll() noexcept
{
	block_t* block;
#if _DEBUG_MEMBLOCK
	block_t* next = m_Block.Root();
	for (block = next; block; block = next)
	{
		next = block->next_block;
		m_BlockCount--;
		a* ptr = (a*)block->data;
		ptr->~a();
		MEM::Free(block);
	}
	m_Block.Reset();
#else
	while((block = m_StartFullBlock) != nullptr)
	{
		if (block->usedDataAvailable())
		{
			a* ptr = (a*)block->data[block->used_data].data;
			ptr->~a();
			Free(ptr);
			block = m_StartFullBlock;
		}
	}

	while ((block = m_StartUsedBlock) != nullptr)
	{
		if (block->usedDataAvailable())
		{
			a* ptr = (a*)block->data[block->used_data].data;
			ptr->~a();
			Free(ptr);
		}
	}

	if (m_FreeBlock)
	{
		m_BlockCount--;
		MEM_Free(m_FreeBlock);
		m_FreeBlock = nullptr;
	}
#endif
}

template<typename a, size_t b>
size_t MEM_BlockAlloc<a, b>::Count(const block_t* list)
{
	int count = 0;
#if _DEBUG_MEMBLOCK
	for (const block_t* block = list; block; block = block->next_block)
	{
		count++;
	}
	return count;
#else

	for (const block_t* block = list; block; block = block->next_block)
	{
		if (!block->usedDataAvailable())
		{
			continue;
		}

		const block_offset_t used_data = block->used_data;
		block_offset_t current_used_data = used_data;

		do
		{
			count++;
			current_used_data = block->next_data[current_used_data];
		} while (current_used_data != used_data);
	}

	return count;
#endif
}

template<typename a, size_t b>
size_t MEM_BlockAlloc<a, b>::Count()
{
#if _DEBUG_MEMBLOCK
	return Count(m_Block);
#else
	return Count(m_StartFullBlock) + Count(m_StartUsedBlock);
#endif
}

template<typename a, size_t b>
size_t MEM_BlockAlloc<a, b>::BlockCount()
{
	return m_BlockCount;
}

template<typename a, size_t b>
size_t MEM_BlockAlloc<a, b>::BlockMemory()
{
	return sizeof(block_s<a, b>);
}


template<typename a, size_t b>
MEM_BlockAlloc_enum<a, b >::MEM_BlockAlloc_enum(MEM_BlockAlloc< a, b>& owner)
{
	m_Owner = &owner;
	m_CurrentBlock = nullptr;
#if !_DEBUG_MEMBLOCK
	m_CurrentBlockType = MEM_BlockAlloc_enum::used;
#endif
}

template<typename a, size_t b>
a* MEM_BlockAlloc_enum<a, b>::NextElement()
{
#if _DEBUG_MEMBLOCK
	if (!m_CurrentBlock)
	{
		m_CurrentBlock = m_Owner->m_Block.Root();
	}
	else
	{
		m_CurrentBlock = m_CurrentBlock->next_block;
	}
	return (a*)m_CurrentBlock;
#else
	// search for a valid block type
	while (!m_CurrentBlock)
	{
		switch (m_CurrentBlockType)
		{
		case blockType_e::used:
			m_CurrentBlock = m_Owner->m_StartUsedBlock;
			break;
		case blockType_e::full:
			m_CurrentBlock = m_Owner->m_StartFullBlock;
			break;
		default:
			return nullptr;
		}

		reinterpret_cast<uint8_t&>(m_CurrentBlockType)++;

	_label:
		for (; m_CurrentBlock; m_CurrentBlock = m_CurrentBlock->next_block)
		{
			if (m_CurrentBlock->usedDataAvailable())
			{
				m_CurrentData = m_CurrentBlock->used_data;
				return reinterpret_cast<a*>(m_CurrentBlock->data[m_CurrentData].data);
			}
		}
	}

	m_CurrentData = m_CurrentBlock->next_data[m_CurrentData];

	if (m_CurrentData == m_CurrentBlock->used_data)
	{
		// found an object
		m_CurrentBlock = m_CurrentBlock->next_block;
		goto _label;
	}

	return reinterpret_cast<a*>(m_CurrentBlock->data[m_CurrentData].data);
#endif
}

template<typename a, size_t b>
a* MEM_BlockAlloc_enum<a, b>::CurrentElement()
{
	return m_CurrentBlock;
}

template<typename a, size_t b>
void* operator new(size_t, MEM_BlockAlloc<a, b>& allocator)
{
	return allocator.Alloc();
}

template<typename a, size_t b>
void operator delete(void* ptr, MEM_BlockAlloc<a, b>& allocator) noexcept
{
	return allocator.Free(ptr);
}
