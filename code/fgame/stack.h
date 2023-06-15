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

// stack.h: Generic Stack object.
// 

#ifndef __STACK_H__
#define __STACK_H__

#include "g_local.h"
#include "class.h"

template <class Type>
class StackNode : public Class
	{
	public:
		Type		 data;
		StackNode *next;

		StackNode( Type d );
	};

template <class Type>
inline StackNode<Type>::StackNode( Type d ) : data( d )
	{
	next = NULL; 
	}

template <class Type>
class Stack : public Class
	{
	private:
		StackNode<Type> *head;

	public:
					Stack();
					~Stack<Type>();
		void		Clear( void	);
		qboolean Empty( void );
		void		Push( Type data );
		Type		Pop( void );
	};

template <class Type>
inline Stack<Type>::Stack()
	{
	head = NULL;
	}

template <class Type>
inline Stack<Type>::~Stack()
	{
	Clear();
	}

template <class Type>
inline void Stack<Type>::Clear
	(
	void
	)

	{
	while( !Empty() )
		{
		Pop();
		}
	}

template <class Type>
inline qboolean Stack<Type>::Empty
	(
	void
	)

	{
	if ( head == NULL )
		{
		return true;
		}
	return false;
	}

template <class Type>
inline void Stack<Type>::Push
	(
	Type data
	)

	{
	StackNode<Type> *tmp;

	tmp = new StackNode<Type>( data );
	if ( !tmp )
		{
		assert( NULL );
		gi.Error( ERR_DROP, "Stack::Push : Out of memory" );
		}

	tmp->next = head;
	head = tmp;
	}

template <class Type>
inline Type Stack<Type>::Pop
	(
	void
	)

	{
	Type ret;
	StackNode<Type> *node;

	if ( !head )
		{
		return NULL;
		}

	node	= head;
	ret	= node->data;
	head	= node->next;

	delete node;

	return ret;
	}

#endif /* stack.h */
