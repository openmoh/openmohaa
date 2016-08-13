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

// con_arrayset.h: con_set with an index table

#ifndef __CON_ARRAYSET_H__
#define __CON_ARRAYSET_H__

template< typename k, typename v >
class con_arrayset : public con_set< k, v >
{
private:
	Entry< k, v >				**reverseTable;		// the index table

protected:
	virtual Entry< k, v >		*addNewKeyEntry( const k& key );

public:
	con_arrayset();
	~con_arrayset();

	virtual void				clear();
	virtual void				resize( int count );

	unsigned int				findKeyIndex( const k& key );
	unsigned int				addKeyIndex( const k& key );
	unsigned int				addNewKeyIndex( const k& key );
	bool						remove( const k& key );

	v&							operator[]( unsigned int index );
};

template< typename key, typename value >
con_arrayset<key, value>::con_arrayset()
{
	reverseTable = ( &this->defaultEntry ) - 1;
}

template< typename key, typename value >
con_arrayset<key, value>::~con_arrayset()
{

}

template< typename key, typename value >
void con_arrayset<key, value>::resize( int count )
{
	Entry< key, value > **oldReverseTable = reverseTable;
	unsigned int oldTableLength = this->tableLength;
	int i;

	con_set< key, value >::resize( count );

	// allocate a bigger reverse table
	reverseTable = ( new Entry< key, value > *[ this->tableLength ]() ) - 1;

	for( i = 1; i <= oldTableLength; i++ )
	{
		reverseTable[ i ] = oldReverseTable[ i ];
	}

	if( oldTableLength > 1 )
	{
		oldReverseTable++;
		delete[] oldReverseTable;
	}
}

template< typename key, typename value >
void con_arrayset<key, value>::clear()
{
	if( this->tableLength > 1 )
	{
		reverseTable++;
		delete[] reverseTable;
		reverseTable = ( &this->defaultEntry ) - 1;
	}

	con_set< key, value >::clear();
}


template< typename k, typename v >
Entry< k, v > *con_arrayset< k, v >::addNewKeyEntry( const k& key )
{
	Entry< k, v > *entry = con_set< k, v >::addNewKeyEntry( key );

	entry->index = this->count;

	reverseTable[ this->count ] = entry;

	return entry;
}

template< typename k, typename v >
unsigned int con_arrayset< k, v >::addKeyIndex( const k& key )
{
	Entry< k, v > *entry = this->addKeyEntry( key );

	return entry->index;
}

template< typename k, typename v >
unsigned int con_arrayset< k, v >::addNewKeyIndex( const k& key )
{
	Entry< k, v > *entry = this->addNewKeyEntry( key );

	return entry->index;
}

template< typename k, typename v >
unsigned int con_arrayset< k, v >::findKeyIndex( const k& key )
{
	Entry< k, v > *entry = this->findKeyEntry( key );

	if( entry != NULL ) {
		return entry->index;
	} else {
		return 0;
	}
}

template< typename k, typename v >
bool con_arrayset< k, v >::remove( const k& key )
{
	int i;

	for( i = 1; i <= this->tableLength; i++ )
	{
		if( reverseTable[ i ] &&
			reverseTable[ i ]->key == key )
		{
			reverseTable[ i ] = NULL;
		}
	}

	return con_set< k, v >::remove( key );
}

template< typename key, typename value >
value& con_arrayset< key, value >::operator[]( unsigned int index )
{
	return reverseTable[ index ]->key;
}

#endif /* __CON_ARRAYSET_H__ */
