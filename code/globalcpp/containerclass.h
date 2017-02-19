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

// containerclass.h: C++ Class Container for use with SafePtr

#ifndef __CONTAINERCLASS_H__
#define __CONTAINERCLASS_H__

#include "container.h"

template< class Type >
class ContainerClass : public Class {
	Container< Type > value;

public:
	virtual ~ContainerClass() { value.FreeObjectList(); }

	virtual void		Archive( Archiver& arc );

	int					AddObject( const Type& obj ) { return value.AddObject( obj ); }
	int					AddUniqueObject( const Type& obj ) { return value.AddUniqueObject( obj ); }
	void				AddObjectAt( int index, const Type& obj ) { return value.AddObjectAt( index, obj ); }
	Type				*AddressOfObjectAt( int index ) { return value.AddressOfObjectAt( index ); }

	void				ClearObjectList( void ) { return value.ClearObjectList(); }
	void				Fix( void ) { return value.Fix(); }
	void				FreeObjectList( void ) { return value.FreeObjectList(); }
	int					IndexOfObject( const Type& obj ) { return value.IndexOfObject( obj ); }
	void				InsertObjectAt( int index, const Type& obj ) { return value.InsertObjectAt( index, obj ); }
	int					NumObjects( void ) const { return value.NumObjects(); }
	Type&				ObjectAt( const size_t index ) const { return value.ObjectAt( index ); }
	bool				ObjectInList( const Type& obj ) { return value.ObjectInList( obj ); }
	void				RemoveObjectAt( int index ) { return value.RemoveObjectAt( index ); }
	void				RemoveObject( const Type& obj ) { return value.RemoveObject( obj ); }
	void				Reset( void ) { return value.Reset(); }
	void				Resize( int maxelements ) { return value.Resize( maxelements ); }
	void				SetObjectAt( int index, const Type& obj ) { return value.SetObjectAt( index, obj ); }
	void				Sort( int( *compare )( const void *elem1, const void *elem2 ) ) { return value.Sort( compare ); }
	Type&				operator[]( const int index ) const { return value[ index ]; }
	Container<Type>&	operator=( const Container<Type>& container ) { return value = container; }
};

#endif /* __CONTAINERCLASS_H__ */
