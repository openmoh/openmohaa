//-----------------------------------------------------------------------------
//
//  $Logfile:: /fakk2_code/fakk2_new/cgame/container.cpp                      $
// $Revision:: 2                                                              $
//   $Author:: Markd                                                          $
//     $Date:: 10/05/99 2:03p                                                 $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// $Log:: /fakk2_code/fakk2_new/cgame/container.cpp                           $
// 
// 2     10/05/99 2:03p Markd
// Added warning about files being in multiple projects
// 
// 1     9/10/99 10:53a Jimdose
// 
// 1     9/08/99 3:15p Aldie
//
// DESCRIPTION:
// Base class for a dynamic array.  Allows adding, removing, index of,
// and finding of entries with specified value.  Originally created for
// cataloging entities, but pointers to objects that may be removed at
// any time are bad to keep around, so only entity numbers should be
// used in the future.
//
// WARNING: This file is shared between fgame, cgame and possibly the user interface.
// It is instanced in each one of these directories because of the way that SourceSafe works.
//

#if 0

#include "g_local.h"
#include "class.h"
#include "container.h"

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
		delete objlist;
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
	if ( objlist )
		{
		memset( objlist, 0, maxobjects * sizeof( Type ) );
		}
	numobjects = 0;
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

	if ( !objlist )
		{
		maxobjects = maxelements;
		objlist = new Type[ maxobjects ];
		memset( objlist, 0, maxobjects * sizeof( Type ) );
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
		memset( objlist, 0, maxobjects * sizeof( Type ) );
		memcpy( objlist, temp, numobjects * sizeof( Type ) );
		delete temp;
		}
	}

template< class Type >
void Container<Type>::AddObject
	(
	Type obj
	)

	{
	Type *temp;

	if ( !objlist )
		{
		Resize( 10 );
		}

	if ( numobjects == maxobjects )
		{
		Resize( maxobjects * 2 );
		}

	objlist[ numobjects++ ] = obj;
	}

template< class Type >
int Container<Type>::IndexOfObject
	(
	Type obj
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
	Type obj
	)

	{
	if ( !IndexOfObject( obj ) )
		{
		return false;
		}

	return true;
	}

template< class Type >
Type Container<Type>::ObjectAt
	(
	int index
	)

	{
	if ( ( index <= 0 ) || ( index > numobjects ) )
		{
      gi.DPrintf( "Container::ObjectAt : index out of range" );
		return 0;
		}

	return objlist[ index - 1 ];
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
      gi.DPrintf( "Container::RemoveObjectAt : Empty list" );
		return;
		}

	if ( ( index <= 0 ) || ( index > numobjects ) )
		{
      gi.DPrintf( "Container::RemoveObjectAt : index out of range" );
		return;
		}

	i = index - 1;
	memcpy( &objlist[ i ], &objlist[ i + 1 ], ( numobjects - 1 - i ) * sizeof( Type ) );
	memset( &objlist[ numobjects - 1 ], 0, sizeof( Type ) );
	numobjects--;
	}

template< class Type >
void Container<Type>::RemoveObject
	(
	Type obj
	)

	{
	int index;

	index = IndexOfObject( obj );
	if ( !index )
		{
      gi.DPrintf( "Container::RemoveObject : Object not in list" );
		return;
		}

	RemoveObjectAt( index );
	}

#endif