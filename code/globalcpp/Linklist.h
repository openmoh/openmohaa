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

// linklist.h:
//
// WARNING: This file is shared between fgame, cgame and possibly the user interface.
// It is instanced in each one of these directories because of the way that SourceSafe works.
// 

#ifndef __linklist_h
#define __linklist_h
#ifdef __cplusplus
extern "C" {
#endif


#define NewNode(type)  ((type *)Z_Malloc(sizeof(type)))

#define LL_New(rootnode,type,next,prev) 			\
   { 																				\
   (rootnode) = NewNode(type);                            		\
   (rootnode)->prev = (rootnode);                         		\
   (rootnode)->next = (rootnode);                         		\
   }

#define LL_Add(rootnode, newnode, next, prev) 			\
   {                                              			\
   (newnode)->next = (rootnode);                  			\
   (newnode)->prev = (rootnode)->prev;                	\
   (rootnode)->prev->next = (newnode);                	\
   (rootnode)->prev = (newnode);                      	\
   }
//MED
#define LL_AddFirst(rootnode, newnode, next, prev) 			\
   {                                              			\
   (newnode)->prev = (rootnode);                  			\
   (newnode)->next = (rootnode)->next;                	\
   (rootnode)->next->prev = (newnode);                	\
   (rootnode)->next = (newnode);                      	\
   }

#define LL_Transfer(oldroot,newroot,next,prev)  \
   {                                                \
   if (oldroot->prev != oldroot)                    \
      {                                             \
      oldroot->prev->next = newroot;                \
      oldroot->next->prev = newroot->prev;          \
      newroot->prev->next = oldroot->next;          \
      newroot->prev = oldroot->prev;                \
      oldroot->next = oldroot;                      \
      oldroot->prev = oldroot;                      \
      }                                             \
   }

#define LL_Reverse(root,type,next,prev)              \
   {                                                     \
   type *newend,*trav,*tprev;                            \
                                                         \
   newend = root->next;                                  \
   for(trav = root->prev; trav != newend; trav = tprev)  \
      {                                                  \
      tprev = trav->prev;                                \
      LL_Move(trav,newend,next,prev);                \
      }                                                  \
   }


#define LL_Remove(node,next,prev) \
   {                                  \
   node->prev->next = node->next;     \
   node->next->prev = node->prev;     \
   node->next = node;                 \
   node->prev = node;                 \
   }

#define LL_SortedInsertion(rootnode,insertnode,next,prev,type,sortparm) \
   {                                                                    \
   type *hoya;                                                          \
                                                                        \
   hoya = rootnode->next;                                               \
   while((hoya != rootnode) && (insertnode->sortparm > hoya->sortparm)) \
      {                                                                 \
      hoya = hoya->next;                                                \
      }                                                                 \
   LL_Add(hoya,insertnode,next,prev);                               \
   }

#define LL_Move(node,newroot,next,prev) \
   {                                        \
   LL_Remove(node,next,prev);           \
   LL_Add(newroot,node,next,prev);      \
   }

#define LL_Empty(list,next,prev) \
   (                                 \
   ((list)->next == (list)) &&       \
   ((list)->prev == (list))          \
   )

#define LL_Free(list)   Z_Free(list)
#define LL_Reset(list,next,prev)    (list)->next = (list)->prev = (list)

#define LL_SafeRemove(node,next,prev)				\
	{												\
	if( node->prev ) node->prev->next = node->next;	\
	if( node->next ) node->next->prev = node->prev;	\
	}

#define LL_SafeAdd(rootnode, newnode, next, prev)		\
	{														\
	(newnode)->next = NULL;									\
	(newnode)->prev = (rootnode);							\
	if((rootnode)) (rootnode)->next = (newnode);			\
	(rootnode) = (newnode);									\
	}

#define LL_SafeAddFirst(rootnode, newnode, next, prev)		\
	{														\
	(newnode)->prev = NULL;									\
	(newnode)->next = (rootnode);							\
	if((rootnode)) (rootnode)->prev = (newnode);			\
	(rootnode) = (newnode);									\
	}

#ifdef __cplusplus
}

#endif
#endif
