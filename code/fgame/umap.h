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

// umap.h: Hash table based map class template.  Maps text based keys to previously
// stored data.
// 

#ifndef __UMAP_H__
#define __UMAP_H__

#include "str.h"
#include <stdlib.h>

template< class Type >
class UMap : public Class
	{
	private:
      struct hashnode_s
         {
         str         key;
         Type        value;
         hashnode_s  *next;

         hashnode_s( const char *k, Type v, hashnode_s *n ) : key( k ), value( v ), next( n ) {};
         hashnode_s( const str &k, Type v, hashnode_s *n ) : key( k.c_str() ), value( v ), next( n ) {};
         };

      hashnode_s  **m_heads;
      Type        m_defaultvalue;

      unsigned    m_tablesize;
      unsigned    m_numentries;
      unsigned    m_tablesizemask;

      unsigned    getHash( const char *key );

	public:
		void		   Clear( void );
		unsigned    getNumEntries( void ) const;
      void        DeleteDefaultValues ( void );

      Type&	      operator[]( const char *key );
      Type&	      operator[]( const str &key );
   
      const char *getKeyByIndex ( int index ) const;
      Type&       getValueByIndex ( unsigned index );

	   ~UMap<Type>();

      UMap( Type defaultvalue, unsigned tablesize = 64 ) : m_defaultvalue( defaultvalue ), m_tablesize( tablesize )
	      {
         int i;
         int bits;

         assert( m_tablesize > 0 );

	      m_heads = new hashnode_s *[ m_tablesize ];
         memset( m_heads, 0, sizeof( *m_heads ) * m_tablesize );

         m_numentries = 0;
         m_tablesizemask = 0;

         bits = 0;
         for( i = 0; i < 32; i++ )
            {
            if ( m_tablesize & ( 1 << i ) )
               {
               bits++;
               }
            }

         if ( bits == 1 )
            {
            m_tablesizemask = m_tablesize - 1;
            }
	      };

      UMap( UMap<Type> &map ) : m_defaultvalue( map.m_defaultvalue ), m_tablesize( map.m_tablesize )
	      {
         unsigned i;
         hashnode_s *node;
         hashnode_s **prev;

         assert( m_tablesize > 0 );

	      m_heads = new hashnode_s *[ m_tablesize ];
         m_numentries = map.m_numentries;
         m_tablesizemask = map.m_tablesizemask;

         for( i = 0; i < m_tablesize; i++ )
            {
            if ( !map.m_heads[ i ] )
               {
               m_heads[ i ] = NULL;
               continue;
               }

            prev = &m_heads[ i ];
            for( node = map.m_heads[ i ]; node != NULL; node = node->next )
               {
               *prev = new hashnode_s( node->key, node->value, NULL );
               prev = &( *prev )->next;
               }
            }
	      };
	};

template< class Type >
UMap<Type>::~UMap<Type>()
	{
	Clear();
   delete m_heads;
	}

template< class Type >
unsigned UMap<Type>::getHash
   (
   const char *key
   )

   {
   unsigned h;
   const char *v;

   if ( m_tablesizemask )
      {
      h = 0;
      for( v = key; *v != '\0'; v++ )
         {
         h = ( 64 * h + unsigned( *v ) ) & m_tablesizemask;
         }
      }
   else
      {
      h = 0;
      for( v = key; *v != '\0'; v++ )
         {
         h = ( 64 * h + unsigned( *v ) ) % m_tablesize;
         }
      }

   return h;
   }

template< class Type >
Type& UMap<Type>::operator[]
	(
	const char *key
	)

	{
   hashnode_s **head;
   hashnode_s *node;
   unsigned hash;

   // FIXME
   // if list was always sorted, could just insert new node as soon as we get
   // node whose key is greater than the search key
   hash = getHash( key );
   head = &m_heads[ hash ];
   if ( *head )
      {
      for( node = *head; node != NULL; node = node->next )
         {
         if ( node->key == key )
            {
            return node->value;
            }
         }
      }

   m_numentries++;

   *head = new hashnode_s( key, m_defaultvalue, *head );

	return ( *head )->value;
	}

template< class Type >
const char * UMap<Type>::getKeyByIndex 
   (
   int index
   ) const

   {
   unsigned head;

   for ( head = 0; head < m_tablesize; head++ )
      {
      hashnode_s *node;

      for ( node = m_heads[head]; node; node = node->next )
         {
         if ( !index )
            return node->key.c_str ();

         index--;
         }
      }

   return NULL;
   }

template <class Type>
Type &UMap<Type>::getValueByIndex
   (
   unsigned index
   )

   {
   unsigned head;

   ASSERT ( index < getNumEntries () );

   for ( head = 0; head < m_tablesize; head++ )
      {
      hashnode_s *node;

      for ( node = m_heads[head]; node; node = node->next )
         {
         if ( !index )
            return node->value;

         index--;
         }
      }
   
   // Should never, EVER get here.
   ASSERT ( 0 );
   return m_defaultvalue;
   }

template <class Type>
inline void UMap<Type>::DeleteDefaultValues
   (
   void
   )

   {
   unsigned head;
   
   for ( head = 0; head < m_tablesize; head++ )
      {
      hashnode_s *node;

      while ( m_heads[head] && m_heads[head]->value == m_defaultvalue )
         {
         node = m_heads[head];
         m_heads[head] = node->next;
         delete node;
         m_numentries--;
         }

      if ( !m_heads[head] )
         continue;

      for ( node = m_heads[head]; node->next; )
         {
         hashnode_s *next = node->next;

         if ( next->value == m_defaultvalue )
            {
            node->next = next->next;
            delete next;
            m_numentries--;
            }
         else
            node = node->next;
         }
      }
   }

template< class Type >
inline Type& UMap<Type>::operator[]
	(
	const str &key
	)

	{
   return this->[ key.c_str() ];
   }

template< class Type >
void UMap<Type>::Clear
	(
	void
	)

	{
   unsigned i;
   hashnode_s *node;
   hashnode_s *next;

   for( i = 0; i < m_tablesize; i++ )
      {
      next = m_heads[ i ];
      while( next != NULL )
         {
         node = next;
         next = next->next;
         delete node;
         }

      m_heads[ i ] = NULL;
      }

   m_numentries = 0;
	}

template< class Type >
inline unsigned UMap<Type>::getNumEntries
	(
	void
	) const

	{
	return m_numentries;
	}

#endif /* !__UMAP_H__ */
