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

// queue.h: Generic Queue object
//

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "class.h"

class QueueNode : public Class
	{
	public:
		void		 *data;
		QueueNode *next;

		QueueNode();
	};

inline QueueNode::QueueNode()
	{
	data = NULL;
	next = NULL;
	}

class Queue : public Class
	{
	private:
		QueueNode *head;
		QueueNode *tail;

	public:
					Queue();
					~Queue();
		void		Clear( void	);
		qboolean Empty( void );
		void		Enqueue( void *data );
		void		*Dequeue( void );
      void     Remove( void *data );
      qboolean Inqueue( void *data );
	};

inline qboolean Queue::Empty
	(
	void
	)

	{
	if ( head == NULL )
		{
		assert( !tail );
		return true;
		}

	assert( tail );
	return false;
	}

inline void Queue::Enqueue
	(
	void *data
	)

	{
	QueueNode *tmp;

	tmp = new QueueNode;
	if ( !tmp )
		{
		assert( NULL );
		gi.Error( ERR_DROP, "Queue::Enqueue : Out of memory" );
		}

	tmp->data = data;

	assert( !tmp->next );
	if ( !head )
		{
		assert( !tail );
		head = tmp;
		}
	else
		{
		assert( tail );
		tail->next = tmp;
		}
	tail = tmp;
	}

inline void *Queue::Dequeue
	(
	void
	)

	{
	void *ptr;
	QueueNode *node;

	if ( !head )
		{
		assert( !tail );
		return NULL;
		}

	node = head;
	ptr = node->data;

	head = node->next;
	if ( head == NULL )
		{
		assert( tail == node );
		tail = NULL;
		}

	delete node;

	return ptr;
	}

inline void Queue::Clear
	(
	void
	)

	{
	while( !Empty() )
		{
		Dequeue();
		}
	}

inline Queue::Queue()
	{
	head = NULL;
	tail = NULL;
	}

inline Queue::~Queue()
	{
	Clear();
	}

inline void Queue::Remove
   (
   void *data
   )

   {
	QueueNode *node;
	QueueNode *prev;

	if ( !head )
		{
		assert( !tail );

      gi.DPrintf( "Queue::Remove : Data not found in queue\n" );
		return;
		}

   for( prev = NULL, node = head; node != NULL; prev = node, node = node->next )
      {
      if ( node->data == data )
         {
         break;
         }
      }

   if ( !node )
      {
      gi.DPrintf( "Queue::Remove : Data not found in queue\n" );
      }
   else
      {
      if ( !prev )
         {
         // at head
         assert( node == head );
         head = node->next;
	      if ( head == NULL )
		      {
		      assert( tail == node );
		      tail = NULL;
		      }
         }
      else
         {
         prev->next = node->next;
         if ( prev->next == NULL )
            {
            // at tail
            assert( tail == node );
            tail = prev;
            }
         }

	   delete node;
      }
   }

inline qboolean Queue::Inqueue
   (
   void *data
   )

   {
	QueueNode *node;

   for( node = head; node != NULL; node = node->next )
      {
      if ( node->data == data )
         {
         return true;
         }
      }

   return false;
   }

#endif /* queue.h */
