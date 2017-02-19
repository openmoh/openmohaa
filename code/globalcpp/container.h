/*
===========================================================================
Copyright (C) 2008 the OpenMoHAA team

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

// container.h: C++ Container

#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#ifdef GAME_DLL
#ifdef WIN32
#define glbprintf( text ) gi.Printf( text )
#else
#define glbprintf( text )
#endif
#elif defined CGAME_DLL
#define glbprintf( text ) cgi.Printf( text )
#else
#define glbprintf( text ) printf( text )
#endif

#ifdef _DEBUG
#define CONTAINER_Error( id, text ) glbprintf( text ); assert( 0 );
#else
#define CONTAINER_Error( id, text ) throw( text ) //gi.Error
#endif
#define CONTAINER_DPrintf( text ) glbprintf( text )
#define CONTAINER_WDPrintf( text ) glbprintf( text )

class Archiver;

template< class Type >
class Container
{
private:
	Type	*objlist;
	int		numobjects;
	int		maxobjects;

private:
	void				Copy( const Container<Type>& container );

public:
	Container();
	Container( const Container<Type>& container );
	~Container();

	void				Archive( Archiver& arc );
	void				Archive( Archiver& arc, void( *ArchiveFunc )( Archiver &arc, Type *obj ) );

	int					AddObject( const Type& obj );
	int					AddUniqueObject( const Type& obj );
	void				AddObjectAt( int index, const Type& obj );
	Type				*AddressOfObjectAt( int index );
//	void				Archive( Archiver &arc );
	void				ClearObjectList( void );
	void				Fix( void );
	void				FreeObjectList( void );
	int					IndexOfObject( const Type& obj );
	void				InsertObjectAt( int index, const Type& obj );
	int					MaxObjects( void ) const;
	int					NumObjects( void ) const;
	Type&				ObjectAt( const size_t index ) const;
	bool				ObjectInList( const Type& obj );
	void				RemoveObjectAt( int index );
	void				RemoveObject( const Type& obj );
	void				Reset( void );
	void				Resize( int maxelements );
	void				SetObjectAt( int index, const Type& obj );
	void				Sort( int( *compare )( const void *elem1, const void *elem2 ) );
	Type&				operator[]( const int index ) const;
	Container<Type>&	operator=( const Container<Type>& container );
};

template< class Type >
Container<Type>::Container()
{
	objlist = NULL;
	numobjects = 0;
	maxobjects = 0;
}

template< class Type >
Container<Type>::Container( const Container<Type>& container )
{
	objlist = NULL;

	Copy( container );
}

template< class Type >
Container<Type>::~Container()
{
	FreeObjectList();
}

template< class Type >
int Container<Type>::AddObject( const Type& obj )
{
	if ( !objlist )
		Resize( 10 );

	if ( numobjects >= maxobjects ) {
		Resize( numobjects * 2 );
	}

	objlist[numobjects] = obj;
	numobjects++;

	return numobjects;
}

template< class Type >
int Container<Type>::AddUniqueObject( const Type& obj )
{
	int index;

	index = IndexOfObject( obj );

	if ( !index ) {
		index = AddObject( obj );
	}

	return index;
}

template< class Type >
void Container<Type>::AddObjectAt( int index, const Type& obj )
{
	if ( index > maxobjects )
		Resize( index );

	if ( index > numobjects )
		numobjects = index;

	SetObjectAt( index, obj );
}

template< class Type >
Type *Container<Type>::AddressOfObjectAt( int index )
{
	if ( index > maxobjects ) {
		CONTAINER_Error( ERR_DROP, "Container::AddressOfObjectAt : index is greater than maxobjects" );
	}

	if ( index > numobjects ) {
		numobjects = index;
	}

	return &objlist[index - 1];
}

/*template< class Type >
void Container<Type>::Archive( Archiver &arc )
{

}*/

template< class Type >
void Container<Type>::ClearObjectList( void )
{
	if ( objlist && numobjects )
	{
		delete[] objlist;

		if ( maxobjects == 0 )
		{
			objlist = NULL;
			return;
		}

		objlist = new Type[maxobjects];
		numobjects = 0;
	}
}

template< class Type >
void Container<Type>::Fix( void )
{
	if( !objlist || !numobjects ) {
		return;
	}

	Type *newlist = new Type[ numobjects ];
	int j = 0;

	for( int i = 0; i < numobjects; i++ )
	{
		if( objlist[ i ] == NULL ) {
			continue;
		}

		newlist[ j ] = objlist[ i ];
		j++;
	}

	numobjects = j;

	delete[] objlist;
	objlist = newlist;

	if( !numobjects ) {
		ClearObjectList();
	}
}

template< class Type >
void Container<Type>::FreeObjectList( void )
{
	if( objlist ) {
		delete[] objlist;
	}

	objlist = NULL;
	numobjects = 0;
	maxobjects = 0;
}

template< class Type >
int Container<Type>::IndexOfObject( const Type& obj )
{
	int i;

	if ( !objlist ) {
		return 0;
	}

	for ( i = 0; i < numobjects; i++ )
	{
		if ( objlist[i] == obj )
		{
			return i + 1;
		}
	}

	return 0;
}

template< class Type >
void Container<Type>::InsertObjectAt( int index, const Type& obj )
{
	if ( ( index <= 0 ) || ( index > numobjects + 1 ) )
	{
		CONTAINER_Error( ERR_DROP, "Container::InsertObjectAt : index out of range" );
		return;
	}

	numobjects++;
	int arrayIndex = index - 1;

	if ( numobjects > maxobjects )
	{
		maxobjects = numobjects;
		if ( !objlist ) {
			objlist = new Type[maxobjects];
			objlist[arrayIndex] = obj;
			return;
		}
		else
		{
			Type *temp = objlist;
			if ( maxobjects < numobjects )
			{
				maxobjects = numobjects;
			}

			objlist = new Type[maxobjects];

			int i;
			for ( i = arrayIndex - 1; i >= 0; i-- ) {
				objlist[i] = temp[i];
			}

			objlist[arrayIndex] = obj;
			for ( i = numobjects - 1; i > arrayIndex; i-- ) {
				objlist[i] = temp[i - 1];
			}

			delete[] temp;
		}
	}
	else
	{
		for ( int i = numobjects - 1; i > arrayIndex; i-- ) {
			objlist[i] = objlist[i - 1];
		}
		objlist[arrayIndex] = obj;
	}
}

template< class Type >
int Container<Type>::MaxObjects( void ) const
{
	return maxobjects;
}

template< class Type >
int Container<Type>::NumObjects( void ) const
{
	return numobjects;
}

template< class Type >
Type& Container<Type>::ObjectAt( const size_t index ) const
{
	if( ( index <= 0 ) || ( index > numobjects ) ) {
		CONTAINER_Error( ERR_DROP, "Container::ObjectAt : index out of range" );
	}

	return objlist[index - 1];
}

template< class Type >
bool Container<Type>::ObjectInList( const Type& obj )
{
	if ( !IndexOfObject( obj ) ) {
		return false;
	}

	return true;
}

template< class Type >
void Container<Type>::RemoveObjectAt( int index )
{
	int i;

	if ( !objlist )
		return;

	if ( ( index <= 0 ) || ( index > numobjects ) )
		return;

	i = index - 1;
	numobjects--;

	for ( i = index - 1; i < numobjects; i++ )
		objlist[i] = objlist[i + 1];
}

template< class Type >
void Container<Type>::RemoveObject( const Type& obj )
{
	int index;

	index = IndexOfObject( obj );

	assert( index );
	if ( !index )
	{
		CONTAINER_WDPrintf( "Container::RemoveObject : Object not in list\n" );
		return;
	}

	RemoveObjectAt( index );
}

template< class Type >
void Container<Type>::Reset()
{
	objlist = NULL;
	numobjects = 0;
	maxobjects = 0;
}

template< class Type >
void Container<Type>::Resize( int maxelements )
{
	Type* temp;
	int i;

	if ( maxelements <= 0 )
	{
		FreeObjectList();
		return;
	}

	if ( !objlist )
	{
		maxobjects = maxelements;
		objlist = new Type[maxobjects];
	}
	else
	{
		temp = objlist;

		maxobjects = maxelements;

		if ( maxobjects < numobjects ) {
			maxobjects = numobjects;
		}

		objlist = new Type[maxobjects];

		for ( i = 0; i < numobjects; i++ ) {
			objlist[i] = temp[i];
		}

		delete[] temp;
	}
}

template< class Type >
void Container<Type>::SetObjectAt( int index, const Type& obj )
{
	if ( !objlist )
		return;

	if( ( index <= 0 ) || ( index > numobjects ) ) {
		CONTAINER_Error( ERR_DROP, "Container::SetObjectAt : index out of range" );
	}

	objlist[index - 1] = obj;
}

template< class Type >
void Container<Type>::Sort( int( *compare )( const void *elem1, const void *elem2 ) )

{
	if ( !objlist ) {
		return;
	}

	qsort( ( void * )objlist, ( size_t )numobjects, sizeof( Type ), compare );
}

template< class Type >
Type& Container<Type>::operator[]( const int index ) const
{
	return ObjectAt( index + 1 );
}

template< class Type >
void Container<Type>::Copy( const Container<Type>& container )
{
	int i;

	if( &container == this ) {
		return;
	}

	FreeObjectList();

	numobjects = container.numobjects;
	maxobjects = container.maxobjects;
	objlist = NULL;

	if( container.objlist == NULL || !container.maxobjects ) {
		return;
	}

	Resize( maxobjects );

	if( !container.numobjects ) {
		return;
	}

	for( i = 0; i < container.numobjects; i++ ) {
		objlist[ i ] = container.objlist[ i ];
	}

	return;
}

template< class Type >
Container<Type>& Container<Type>::operator=( const Container<Type>& container )
{
	Copy( container );

	return *this;
}

#endif /* __CONTAINER_H__ */
