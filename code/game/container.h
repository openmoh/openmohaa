//-----------------------------------------------------------------------------
//
//  $Logfile:: /fakk2_code/fakk2_new/fgame/container.h                        $
// $Revision:: 9                                                              $
//   $Author:: Steven                                                         $
//     $Date:: 7/26/00 1:08p                                                  $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// $Log:: /fakk2_code/fakk2_new/fgame/container.h                             $
// 
// 9     7/26/00 1:08p Steven
// In ClearObjectList make sure we don't have 0 maxelements when creating a new
// array.
// 
// 8     7/25/00 2:32p Aldie
// Undo checkout for new operator
// 
// 7     7/25/00 2:18p Aldie
// changed the new operator for containers
// 
// 6     5/26/00 7:44p Markd
// 2nd phase save games
// 
// 5     1/06/00 11:18p Jimdose
// put sort back in
// 
// 4     1/06/00 11:08p Jimdose
// cleaning up unused code
// 
// 3     10/05/99 2:03p Markd
// Added warning about files being in multiple projects
// 
// 2     9/28/99 4:26p Markd
// merged listener, class and vector between 3 projects
// 
// 1     9/10/99 10:53a Jimdose
// 
// 1     9/08/99 3:15p Aldie
//
// DESCRIPTION:
// Base class for a dynamic array.  Allows adding, removing, index of,
// and finding of entries with specified value.  NOTE: indices in container
// are 1 based, not 0.  This means that loops must check from 1 up to and including
// NumObjects()  (ei. for( i = 1; i <= list.NumObjects(); i++ ) ).
//
// FIXME: Someday make this 0 based and update all code to match.
//
// WARNING: This file is shared between fgame, cgame and possibly the user interface.
// It is instanced in each one of these directories because of the way that SourceSafe works.
//

#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#if defined( GAME_DLL )
//
// game dll specific defines
//
#include "g_local.h"

#define CONTAINER_Error gi.Error
#define CONTAINER_DPrintf gi.DPrintf

#elif defined ( CGAME_DLL )
//
// cgame dll specific defines
//
#define CONTAINER_Error cgi.Error
#define CONTAINER_DPrintf cgi.DPrintf

#else

//
// client specific defines
//
#define CONTAINER_Error Com_Error
#define CONTAINER_DPrintf Com_DPrintf
#endif

#include <stdlib.h>

class Archiver;
template< class Type >
class Container
	{
	private:
		Type	*objlist;
		int	numobjects;
		int	maxobjects;

	public:
					Container();
					~Container<Type>();

		void		FreeObjectList( void );
		void		ClearObjectList( void );
		int		NumObjects( void );
		void		Resize( int maxelements );
		void		SetObjectAt( int index, Type& obj );
		int		AddObject( Type& obj );
		int		AddUniqueObject( Type& obj );
		void  	AddObjectAt( int index, Type& obj );
		int		IndexOfObject( Type& obj );
		qboolean	ObjectInList( Type& obj );
		Type&		ObjectAt( int index );
		Type		*AddressOfObjectAt( int index );
		void		RemoveObjectAt( int index );
		void		RemoveObject( Type& obj );
		void		Sort( int ( __cdecl *compare )( const void *elem1, const void *elem2 ) );
#if defined( GAME_DLL )
      void     Archive( Archiver &arc );
#endif
	};

template< class Type >
Container<Type>::Container()
	{
   objlist = NULL;
	FreeObjectList();
	}

template< class Type >
Container<Type>::~Container<Type>()
	{
	FreeObjectList();
	}

template< class Type >
void Container<Type>::FreeObjectList
	(
	void
	)

	{
	if ( objlist )
		{
		delete[] objlist;
		}
	objlist = NULL;
	numobjects = 0;
	maxobjects = 0;
	}

template< class Type >
void Container<Type>::ClearObjectList
	(
	void
	)

	{
	// only delete the list if we have objects in it
	if ( objlist && numobjects )
		{
		delete[] objlist;

		if ( maxobjects == 0 )
			{
			objlist = NULL;
			return;
			}

		objlist = new Type[ maxobjects ];
		numobjects = 0;
		}
	}

template< class Type >
int Container<Type>::NumObjects
	(
	void
	)

	{
	return numobjects;
	}

template< class Type >
void Container<Type>::Resize
	(
	int maxelements
	)

	{
	Type *temp;
	int i;

   assert( maxelements >= 0 );

   if ( maxelements <= 0 )
      {
      FreeObjectList();
      return;
      }

	if ( !objlist )
		{
		maxobjects = maxelements;
		objlist = new Type[ maxobjects ];
		}
	else
		{
		temp = objlist;
		maxobjects = maxelements;
		if ( maxobjects < numobjects )
			{
			maxobjects = numobjects;
			}

		objlist = new Type[ maxobjects ];
		for( i = 0; i < numobjects; i++ )
			{
			objlist[ i ] = temp[ i ];
			}
		delete[] temp;
		}
	}

template< class Type >
void Container<Type>::SetObjectAt
	(
	int index,
	Type& obj
	)

	{
	if ( ( index <= 0 ) || ( index > numobjects ) )
		{
		CONTAINER_Error( ERR_DROP, "Container::SetObjectAt : index out of range" );
		}

	objlist[ index - 1 ] = obj;
	}

template< class Type >
int Container<Type>::AddObject
	(
	Type& obj
	)

	{
	if ( !objlist )
		{
		Resize( 10 );
		}

	if ( numobjects == maxobjects )
		{
		Resize( maxobjects * 2 );
		}

	objlist[ numobjects ] = obj;
	numobjects++;

	return numobjects;
	}

template< class Type >
int Container<Type>::AddUniqueObject
	(
	Type& obj
	)

	{
   int index;

   index = IndexOfObject( obj );
   if ( !index )
      index = AddObject( obj );
   return index;
	}

template< class Type >
void Container<Type>::AddObjectAt
	(
	int index,
	Type& obj
	)

	{
   //
   // this should only be used when reconstructing a list that has to be identical to the original
   //
   if ( index > maxobjects )
      {
      Resize( index );
      }
   if ( index > numobjects )
      {
      numobjects = index;
      }
   SetObjectAt( index, obj );
	}

template< class Type >
int Container<Type>::IndexOfObject
	(
	Type& obj
	)

	{
	int i;

	for( i = 0; i < numobjects; i++ )
		{
		if ( objlist[ i ] == obj )
			{
			return i + 1;
			}
		}

	return 0;
	}

template< class Type >
qboolean Container<Type>::ObjectInList
	(
	Type& obj
	)

	{
	if ( !IndexOfObject( obj ) )
		{
		return false;
		}

	return true;
	}

template< class Type >
Type& Container<Type>::ObjectAt
	(
	int index
	)

	{
	if ( ( index <= 0 ) || ( index > numobjects ) )
		{
		CONTAINER_Error( ERR_DROP, "Container::ObjectAt : index out of range" );
		}

	return objlist[ index - 1 ];
	}

template< class Type >
Type * Container<Type>::AddressOfObjectAt
	(
	int index
	)

	{
   //
   // this should only be used when reconstructing a list that has to be identical to the original
   //
   if ( index > maxobjects )
      {
      CONTAINER_Error( ERR_DROP, "Container::AddressOfObjectAt : index is greater than maxobjects" );
      }
   if ( index > numobjects )
      {
      numobjects = index;
      }
	return &objlist[ index - 1 ];
	}

template< class Type >
void Container<Type>::RemoveObjectAt
	(
	int index
	)

	{
	int i;

	if ( !objlist )
		{
      CONTAINER_DPrintf( "Container::RemoveObjectAt : Empty list\n" );
		return;
		}

	if ( ( index <= 0 ) || ( index > numobjects ) )
		{
		CONTAINER_Error( ERR_DROP, "Container::RemoveObjectAt : index out of range" );
		return;
		}

	i = index - 1;
	numobjects--;
	for( i = index - 1; i < numobjects; i++ )
		{
		objlist[ i ] = objlist[ i + 1 ];
		}
	}

template< class Type >
void Container<Type>::RemoveObject
	(
	Type& obj
	)

	{
	int index;

	index = IndexOfObject( obj );
	if ( !index )
		{
      CONTAINER_DPrintf( "Container::RemoveObject : Object not in list\n" );
		return;
		}

	RemoveObjectAt( index );
	}

template< class Type >
void Container<Type>::Sort
	(
	int ( __cdecl *compare )( const void *elem1, const void *elem2 )
	)

	{
	if ( !objlist )
		{
      CONTAINER_DPrintf( "Container::Sort : Empty list\n" );
		return;
		}

	qsort( ( void * )objlist, ( size_t )numobjects, sizeof( Type ), compare );
	}

#if 0
#if defined( GAME_DLL )

#include "str.h"
void Container<str>::Archive
	(
   Archiver &arc
	)
	{
   int i, num;

   if ( arc.Loading() )             
      {                             
      ClearObjectList();              
      arc.ArchiveInteger( &num );    
      Resize( num );                
      }                             
   else                             
      {                             
      num = numobjects;             
      arc.ArchiveInteger( &num );    
      }
   for( i = 1; i <= num; i++ )
      {
      arc.ArchiveString( AddressOfObjectAt( i ) );
      }
	}

#include "vector.h"
void Container<Vector>::Archive
	(
   Archiver &arc
	)
	{
   int i, num;

   if ( arc.Loading() )             
      {                             
      ClearObjectList();              
      arc.ArchiveInteger( &num );    
      Resize( num );                
      }                             
   else                             
      {                             
      num = numobjects;             
      arc.ArchiveInteger( &num );    
      }
   for( i = 1; i <= num; i++ )
      {
      arc.ArchiveVector( AddressOfObjectAt( i ) );
      }
	}

void Container<int>::Archive
	(
   Archiver &arc
	)
	{
   int i, num;

   if ( arc.Loading() )             
      {                             
      ClearObjectList();              
      arc.ArchiveInteger( &num );    
      Resize( num );                
      }                             
   else                             
      {                             
      num = numobjects;             
      arc.ArchiveInteger( &num );    
      }
   for( i = 1; i <= num; i++ )
      {
      arc.ArchiveInteger( AddressOfObjectAt( i ) );
      }
	}

void Container<float>::Archive
	(
   Archiver &arc
	)
	{
   int i, num;

   if ( arc.Loading() )             
      {                             
      ClearObjectList();              
      arc.ArchiveInteger( &num );    
      Resize( num );                
      }                             
   else                             
      {                             
      num = numobjects;             
      arc.ArchiveInteger( &num );    
      }
   for( i = 1; i <= num; i++ )
      {
      arc.ArchiveFloat( AddressOfObjectAt( i ) );
      }
	}

#endif
#endif

#endif /* container.h */
