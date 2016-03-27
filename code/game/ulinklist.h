//-----------------------------------------------------------------------------
//
//  $Logfile:: /fakk2_code/fakk2_new/fgame/ulinklist.h                        $
// $Revision:: 2                                                              $
//     $Date:: 9/23/99 3:41p                                                  $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// $Log:: /fakk2_code/fakk2_new/fgame/ulinklist.h                             $
// 
// 2     9/23/99 3:41p Morbid
// 
// 1     9/23/99 10:43a Morbid
// 
// 3     8/19/99 6:56p Morbid
// Added linked list container
// 
// 2     6/11/99 6:46p Morbid
// 
// 1     5/27/99 5:40p Jimdose
// 
// DESCRIPTION:
// 

#ifndef __ULINKLIST_H__
#define __ULINKLIST_H__

template <class T>
class ULinkListItem
   {
   public:
   T item;

   ULinkListItem<T> *next, *prev;
   };

template <class T>
class ULinkList
   {
   private:
      ULinkListItem<T> m_top;
      ULinkListItem<T> *m_at;
      int m_count;
   public:
      ULinkList ();
      ~ULinkList ();

      void AddHead ( T& head );
      void AddTail ( T& tail );

      bool IterateFromHead (); // Returns true if landed on an item
      bool IterateFromTail (); // ...
      bool IterateNext ();
      bool IteratePrev ();     // ...

      T &getCurrent ();
      void RemoveCurrentSetNext ();
      void RemoveCurrentSetPrev ();
      bool IsCurrentValid () { return m_at != NULL; }
      bool IsCurrentHead () { return m_at == m_top.next; }
      bool IsCurrentTail () { return m_at == m_top.prev; }

      void InsertBeforeCurrent ( T & );
      void InsertAfterCurrent ( T & );

      int getCount () { return m_count; }

      void RemoveAllItems ();

      void *getPosition () { return m_at; }
      void setPosition ( void *pos ) { m_at = reinterpret_cast<ULinkListItem<T> *>(pos); }
   };

template <class T>
ULinkList<T>::ULinkList 
   (
   void
   ) : m_at ( NULL ), m_count ( 0 )

   {
   m_top.next = &m_top;
   m_top.prev = &m_top;
   }

template <class T>
ULinkList<T>::~ULinkList 
   (
   void
   )

   {
   RemoveAllItems ();
   }

template <class T>
void ULinkList<T>::RemoveAllItems 
   (
   void
   )

   {
   ULinkListItem<T> *at, *next = NULL;

   for ( at = m_top.next; at && at != &m_top; at = next )
      {
      next = at->next;
      delete at;
      }

   m_top.next = &m_top;
   m_top.prev = &m_top;

   m_count = 0;
   }

template <class T>
void ULinkList<T>::AddHead 
   (
   T &head
   )

   {
   ULinkListItem<T> *newone = new ULinkListItem<T>;

   newone->item = head;
   newone->next = m_top.next;
   newone->prev = &m_top;
   
   newone->next->prev = newone;
   m_top.next = newone;
   m_count++;
   }

template <class T>
void ULinkList<T>::AddTail
   (
   T &tail
   )

   {
   ULinkListItem<T> *newone = new ULinkListItem<T>;

   newone->item = tail;
   newone->next = &m_top;
   newone->prev = m_top.prev;

   newone->next->prev = newone;
   newone->prev->next = newone;
   m_count++;
   }

template <class T>
bool ULinkList<T>::IterateFromHead
   (
   void
   )

   {
   if ( m_top.next == &m_top ) // Empty list
      {
      m_at = NULL; // So we'll GPF if we try to access

      return false;
      }
   else
      {
      m_at = m_top.next;
      return true;
      }
   }

template <class T>
bool ULinkList<T>::IterateFromTail
   (
   void
   )

   {
   if ( m_top.prev == &m_top ) // Empty list
      {
      m_at = NULL; 
      return false;
      }
   else
      {
      m_at = m_top.prev;
      return false;
      }
   }

template <class T>
bool ULinkList<T>::IterateNext
   (
   void
   )

   {
   assert ( m_at );
   m_at = m_at->next;

   if ( m_at == &m_top )
      {
      m_at = NULL;
      return false;
      }
   else
      {
      return true;
      }
   }

template <class T>
bool ULinkList<T>::IteratePrev
   (
   void
   )

   {
   assert ( m_at );
   m_at = m_at->prev;

   if ( m_at == &m_top )
      {
      m_at = NULL;
      return false;
      }
   else
      {
      return true;
      }
   }

template <class T>
T& ULinkList<T>::getCurrent 
   (
   void
   )

   {
   assert ( m_at );
   return m_at->item;
   }

template <class T>
void ULinkList<T>::RemoveCurrentSetNext
   (
   void
   )

   {
   ULinkListItem<T> *toRemove = m_at;
   assert ( toRemove );

   IterateNext ();
   toRemove->next->prev = toRemove->prev;
   toRemove->prev->next = toRemove->next;

   delete toRemove;
   m_count--;
   }

template <class T>
void ULinkList<T>::RemoveCurrentSetPrev 
   (
   void
   )

   {
   ULinkListItem<T> *toRemove = m_at;
   assert ( toRemove );

   IteratePrev ();
   toRemove->next->prev = toRemove->prev;
   toRemove->prev->next = toRemove->next;

   delete toRemove;
   m_count--;
   }

template <class T>
void ULinkList<T>::InsertBeforeCurrent 
   (
   T& which 
   )

   {
   ULinkListItem<T> *newone = new ULinkListItem<T>;

   assert ( m_at );
   newone->item = which;

   newone->next = m_at;
   newone->prev = m_at->prev;

   newone->next->prev = newone;
   newone->prev->next = newone;
   m_count++;
   }

template <class T>
void ULinkList<T>::InsertAfterCurrent
   (
   T& which 
   )

   {
   ULinkListItem<T> *newone = new ULinkListItem<T>;

   assert ( m_at );
   newone->item = which;

   newone->next = m_at->next;
   newone->prev = m_at;

   newone->next->prev = newone;
   newone->prev->next = newone;
   m_count++;
   }

#if 1
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

#endif


#endif /* !__ULINKLIST_H__ */