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

#ifndef __ULIST_H__
#define __ULIST_H__

template< typename type >
class UListItem {
public:
	type					item;
	class UListItem< type >	*next;
	class UListItem< type >	*prev;

	UListItem();
};

template< typename type >
class UList {
	UListItem<type> m_top;
	UListItem<type> *m_at;
	int m_count;

public:
	UList();
	~UList();

	void		AddHead( type& head );
	void		AddTail( const type& tail );
	bool		IterateFromHead( void );
	bool		IterateFromTail( void );
	bool		IterateNext( void );
	bool		IteratePrev( void );
	type&		getCurrent( void );
	void		RemoveCurrentSetNext( void );
	void		RemoveCurrentSetPrev( void );
	bool		IsCurrentValid( void );
	bool		IsCurrentHead( void );
	bool		IsCurrentTail( void );
	void		InsertBeforeCurrent( type& which );
	void		InsertAfterCurrent( type& which );
	int			getCount( void );
	void		RemoveAllItems( void );
	void		*getPosition( void );
	void		setPosition( void *pos );
};

template< typename type >
UListItem< type >::UListItem()
{
	next = prev = NULL;
}

template< typename type >
UList< type >::UList()
{
	m_at = NULL;
	m_count = 0;
	m_top.next = &m_top;
	m_top.prev = &m_top;
}

template< typename type >
UList< type >::~UList()
{
	RemoveAllItems();
}

template< typename type >
void UList< type >::AddHead
	(
	type& head
	)

{
	// FIXME: stub
}

template< typename type >
void UList< type >::AddTail
	(
	const type& tail
	)

{
	UListItem<type> *item = new UListItem <type>;

	item->item = tail;
	item->next = &m_top;
	item->prev = m_top.prev;
	m_top.prev = item;
	m_top.prev->next = item;

	m_count++;
}

template< typename type >
bool UList< type >::IterateFromHead
	(
	void
	)

{
	if( m_top.next == this )
	{
		m_at = NULL;
		return false;
	}
	else
	{
		m_at = m_top.next;
		return true;
	}
}

template< typename type >
bool UList< type >::IterateFromTail
	(
	void
	)

{
	if( m_top.prev == this )
	{
		m_at = NULL;
		return false;
	}
	else
	{
		m_at = m_top.prev;
		return true;
	}
}

template< typename type >
bool UList< type >::IterateNext
	(
	void
	)

{
	m_at = m_at->next;

	if( m_at == this )
	{
		m_at = NULL;
		return false;
	}
	else
	{
		return true;
	}
}

template< typename type >
bool UList< type >::IteratePrev
	(
	void
	)

{
	m_at = m_at->prev;

	if( m_at == this )
	{
		m_at = NULL;
		return false;
	}
	else
	{
		return true;
	}
}

template< typename type >
type& UList< type >::getCurrent
	(
	void
	)

{
	return m_at;
}

template< typename type >
void UList< type >::RemoveCurrentSetNext
	(
	void
	)

{
	UListItem<type> *item;

	item = m_at;
	IterateNext();

	item->next->prev = item->prev;
	item->prev->next = item->next;

	delete item;

	m_count--;
}

template< typename type >
void UList< type >::RemoveCurrentSetPrev
	(
	void
	)

{
	UListItem<type> *item;

	item = m_at;
	IteratePrev();

	item->next->prev = item->prev;
	item->prev->next = item->next;

	delete item;

	m_count--;
}

template< typename type >
bool UList< type >::IsCurrentValid
	(
	void
	)

{
	// FIXME: stub
}

template< typename type >
bool UList< type >::IsCurrentHead
	(
	void
	)

{
	// FIXME: stub
	return false;
}

template< typename type >
bool UList< type >::IsCurrentTail
	(
	void
	)

{
	// FIXME: stub
	return false;
}

template< typename type >
void UList< type >::InsertBeforeCurrent
	(
	type& which
	)

{
	// FIXME: stub
}

template< typename type >
void UList< type >::InsertAfterCurrent
	(
	type& which
	)

{
	UListItem<type> *item = new UListItem <type>;

	item->item = which;
	item->next = m_at->next;
	item->prev = m_at;
	m_at->next->prev = item;
	item->prev->next = item;

	m_count++;
}

template< typename type >
int UList< type >::getCount
	(
	void
	)

{
	// FIXME: stub
	return 0;
}

template< typename type >
void UList< type >::RemoveAllItems
	(
	void
	)

{
	UListItem<type> *item;
	UListItem<type> *next;

	for( item = m_top.next; item && item != &m_top; item = next )
	{
		next = item->next;
		delete item;
	}

	m_top.next = &m_top;
	m_top.prev = &m_top;
	m_count = 0;
}

template< typename type >
void *UList< type >::getPosition
	(
	void
	)

{
	// FIXME: stub
}

template< typename type >
void UList< type >::setPosition
	(
	void *pos
	)

{
	// FIXME: stub
}

#endif /* __ULIST_H__ */
