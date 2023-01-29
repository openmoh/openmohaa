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

// archive.h: OpenMoHAA Archiver

#ifndef __ARCHIVE_H__
#define __ARCHIVE_H__

#include "glb_local.h"
#include "class.h"
#include "str.h"

#define ARCHIVE_NULL_POINTER ( -654321 )
#define ARCHIVE_POINTER_VALID ( 0 )
#define ARCHIVE_POINTER_NULL ( ARCHIVE_NULL_POINTER )
#define ARCHIVE_POINTER_SELF_REFERENTIAL ( -123456 )
#define ARCHIVE_USE_TYPES

#define ARCHIVE_WRITE 0
#define ARCHIVE_READ  1

enum
{
	pointer_fixup_normal,
	pointer_fixup_safe
};

typedef struct
{
	void **ptr;
	int  index;
	int  type;
} pointer_fixup_t;

class FileRead : public Class
{
protected:
	str				filename;
	size_t         length;
	byte           *buffer;
	byte           *pos;

public:
	CLASS_PROTOTYPE( FileRead );

	FileRead();
	~FileRead();
	void				Close( bool bDoCompression = false );
	const char     *Filename( void );
	size_t         Length( void );
	size_t         Pos( void );
	qboolean       Seek( size_t newpos );
	qboolean       Open( const char *name );
	qboolean       Read( void *dest, size_t size );
};

class Archiver : public Class
{
private:
	Container<Class *>				classpointerList;
	Container<pointer_fixup_t *>	fixupList;

protected:
	str					filename;
	qboolean			fileerror;
	fileHandle_t		file;
	FileRead			readfile;
	int					archivemode;
	int					numclassespos;
	qboolean			harderror;
	size_t				m_iNumBytesIO;
	qboolean			silent;

	void				CheckRead( void );
	void				CheckType( int type );
	int            ReadType( void );
	size_t			ReadSize( void );
	void				CheckSize( int type, size_t size );
	void				ArchiveData( int type, void *data, size_t size );

	void				CheckWrite( void );
	void				WriteType( int type );
	void				WriteSize( size_t size );

public:
	CLASS_PROTOTYPE( Archiver );

	Archiver();
	~Archiver();
	void				FileError( const char *fmt, ... );
	void				Close( void );

	qboolean       Read( str &name, qboolean harderror = true );
	qboolean       Read( const char *name, qboolean harderror = true );
	Class				*ReadObject( void );

	qboolean       Create( str &name, qboolean harderror = true );
	qboolean       Create( const char *name, qboolean harderror = true );

	qboolean       Loading( void );
	qboolean       Saving( void );
	qboolean       NoErrors( void );

	void  			ArchiveVector( Vector * vec );
	void  			ArchiveQuat( Quat * quat );
	void			ArchiveInteger( int * num );
	void  			ArchiveUnsigned( unsigned * unum );
	void  			ArchiveSize( long * unum );
	void			ArchiveByte( byte * num );
	void			ArchiveChar( char * ch );
	void			ArchiveShort( short * num );
	void			ArchiveUnsignedShort( unsigned short * num );
	void			ArchiveFloat( float * num );
	void			ArchiveDouble( double * num );
	void  			ArchiveBoolean( qboolean * boolean );
	void  			ArchiveString( str * string );
	void			ArchiveConfigString( int cs );
	void			ArchiveObjectPointer( Class ** ptr );
	void			ArchiveObjectPosition( void *obj );
	void			ArchiveSafePointer( SafePtrBase * ptr );
	void			ArchiveEventPointer( Event ** ev );
	void  			ArchiveBool( bool * boolean );
	void			ArchivePosition( int * pos );
	void			ArchiveSvsTime( int * time );
	void  			ArchiveVec2( vec2_t vec );
	void  			ArchiveVec3( vec3_t vec );
	void  			ArchiveVec4( vec4_t vec );

	void			ArchiveRaw( void *data, size_t size );
	void			ArchiveObject( Class *obj );

	qboolean		ObjectPositionExists( void * obj );

	void			SetSilent( bool bSilent );

};

inline qboolean Archiver::Read
(
str &name,
qboolean harderror
)

{
	return Read( name.c_str(), harderror );
}

inline qboolean Archiver::Create
(
str &name,
qboolean harderror
)

{
	return Create( name.c_str(), harderror );
}

inline qboolean Archiver::Loading
(
void
)
{
	return ( archivemode == ARCHIVE_READ );
}

inline qboolean Archiver::Saving
(
void
)
{
	return ( archivemode == ARCHIVE_WRITE );
}

inline qboolean Archiver::NoErrors
(
void
)
{
	return ( !fileerror );
}

template< class Type >
inline void Container<Type>::Archive
	(
	Archiver& arc,
	void( *ArchiveFunc )( Archiver& arc, Type *obj )
	)

{
	int num;
	int i;

	if( arc.Loading() )
	{
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
		if( num > numobjects ) {
			numobjects = num;
		}

		ArchiveFunc( arc, &objlist[ i ] );
	}
}

template<>
inline void Container<str>::Archive
(
Archiver &arc
)
{
	int i, num;

	if( arc.Loading() )
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

template<>
inline void Container<Vector>::Archive
(
Archiver &arc
)
{
	int i, num;

	if( arc.Loading() )
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

template<>
inline void Container<int>::Archive
(
Archiver &arc
)
{
	int i, num;

	if( arc.Loading() )
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

template<>
inline void Container<float>::Archive
(
Archiver &arc
)
{
	int i, num;

	if( arc.Loading() )
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

template< typename c >
inline void ArchiveClass
	(
	Archiver& arc,
	c *obj
	)

{
	arc.ArchiveObject( obj );
}

#ifndef NO_ARCHIVE

template< typename key, typename value >
void con_set< key, value >::Archive
	(
	Archiver& arc
	)

{
	Entry< key, value > *e;
	int hash;
	int i;

	arc.ArchiveUnsigned( &tableLength );
	arc.ArchiveUnsigned( &threshold );
	arc.ArchiveUnsigned( &count );
	arc.ArchiveUnsignedShort( &tableLengthIndex );

	if( arc.Loading() )
	{
		if( tableLength != 1 )
		{
			table = new Entry< key, value > *[ tableLength ]();
			memset( table, 0, tableLength * sizeof( Entry< key, value > * ) );
		}

		for( i = 0; i < count; i++ )
		{
			e = new Entry< key, value >;
			e->Archive( arc );

			hash = HashCode< key >( e->key ) % tableLength;

			e->index = i;
			e->next = table[ hash ];
			table[ hash ] = e;
		}
	}
	else
	{
		for( i = tableLength - 1; i >= 0; i-- )
		{
			for( e = table[ i ]; e != NULL; e = e->next )
			{
				e->Archive( arc );
			}
		}
	}
}

template< typename key, typename value >
void con_map< key, value >::Archive
	(
	Archiver& arc
	)

{
	m_con_set.Archive( arc );
}

#endif

#define ArchiveEnum( thing, type )  \
   {                                \
   int tempInt;                     \
                                    \
   tempInt = ( int )( thing );      \
   arc.ArchiveInteger( &tempInt );  \
   ( thing ) = ( type )tempInt;     \
   } 

#endif // __ARCHIVE_H__
