//-----------------------------------------------------------------------------
//
//  $Logfile:: /fakk2_code/fakk2_new/fgame/prioritystack.h                    $
// $Revision:: 1                                                              $
//   $Author:: Jimdose                                                        $
//     $Date:: 9/10/99 10:54a                                                 $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// $Log:: /fakk2_code/fakk2_new/fgame/prioritystack.h                         $
// 
// 1     9/10/99 10:54a Jimdose
// 
// 1     9/08/99 3:16p Aldie
// 
// DESCRIPTION:
// Stack based object that pushes and pops objects in a priority based manner.
// 

#ifndef __PRIORITYSTACK_H__
#define __PRIORITYSTACK_H__

#include "g_local.h"
#include "class.h"

template <class Type>
class PriorityStackNode
	{
	public:
		int					priority;
		Type					data;
		PriorityStackNode *next;

		PriorityStackNode( Type d, int p );
	};

template <class Type>
inline PriorityStackNode<Type>::PriorityStackNode( Type d, int p ) : data( d )
	{
	priority = p;
	next = NULL; 
	}

template <class Type>
class PriorityStack
	{
	private:
		PriorityStackNode<Type> *head;

	public:
					PriorityStack();
					~PriorityStack<Type>();
		void		Clear( void	);
		qboolean Empty( void );
		void		Push( Type data, int priority );
		Type		Pop( void );
	};

template <class Type>
inline PriorityStack<Type>::PriorityStack()
	{
	head = NULL;
	}

template <class Type>
inline PriorityStack<Type>::~PriorityStack<Type>()
	{
	Clear();
	}

template <class Type>
inline void PriorityStack<Type>::Clear
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
inline qboolean PriorityStack<Type>::Empty
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
inline void PriorityStack<Type>::Push
	(
	Type data,
	int priority
	)

	{
	PriorityStackNode<Type> *tmp;
	PriorityStackNode<Type> *next;
	PriorityStackNode<Type> *prev;

	tmp = new PriorityStackNode<Type>( data, priority );
	if ( !tmp )
		{
		assert( NULL );
		gi.error( "PriorityStack::Push : Out of memory" );
		}

	if ( !head || ( priority >= head->priority ) )
		{
		tmp->next = head;
		head = tmp;
		}
	else
		{
		for( prev = head, next = head->next; next; prev = next, next = next->next )
			{
			if ( priority >= next->priority )
				{
				break;
				}
			}

		tmp->next = prev->next;
		prev->next = tmp;
		}
	}

template <class Type>
inline Type PriorityStack<Type>::Pop
	(
	void
	)

	{
	Type ret;
	PriorityStackNode<Type> *node;

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

#endif /* prioritystack.h */
