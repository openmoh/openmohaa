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

// archive.cpp: OpenMoHAA Archiver

#include "glb_local.h"
#include "archive.h"
#include <lz77.h>

#ifdef GAME_DLL
#include "../game/entity.h"
#endif

enum
{
	ARC_NULL, ARC_Vector, ARC_Vec2, ARC_Vec3, ARC_Vec4, ARC_Integer, ARC_Unsigned, ARC_Byte, ARC_Char, ARC_Short, ARC_UnsignedShort,
	ARC_Float, ARC_Double, ARC_Boolean, ARC_String, ARC_Raw, ARC_Object, ARC_ObjectPointer,
	ARC_SafePointer, ARC_EventPointer, ARC_Quat, ARC_Entity, ARC_Bool,
	ARC_Position, ARC_Size,
	ARC_NUMTYPES
};

static const char *typenames[] =
{
	"NULL", "vector", "vec2", "vec3", "vec4", "int", "unsigned", "byte", "char", "short", "unsigned short",
	"float", "double", "qboolean", "string", "raw data", "object", "objectpointer",
	"safepointer", "eventpointer", "quaternion", "entity", "bool",
	"position", "size"
};

#define ArchiveHeader   ( *( int * )"MHAA" )
#define ArchiveVersion	14							// This must be changed any time the format changes!
#define ArchiveInfo     "MOHAA Archive Version 14"	// This must be changed any time the format changes!

CLASS_DECLARATION( Class, FileRead, NULL )
{
	{ NULL, NULL }
};

FileRead::FileRead()
{
	length = 0;
	buffer = NULL;
	pos = 0;
}

FileRead::~FileRead()
{
	Close();
}

void FileRead::Close
	(
	bool bDoCompression
	)

{
	if( bDoCompression )
	{
		byte *tempbuf;
		size_t out_len;

		tempbuf = ( byte * )glbs.Malloc( ( length >> 6 ) + length + 27 );

		// Set the signature
		tempbuf[ 0 ] = 'C';
		tempbuf[ 1 ] = 'S';
		tempbuf[ 2 ] = 'V';
		tempbuf[ 3 ] = 'G';
		*( size_t * )( tempbuf + 4 ) = length;

		// Compress the data
		if( g_lz77.Compress( buffer, length, tempbuf + 8, &out_len ) )
		{
			glbs.Error( ERR_DROP, "Compression of SaveGame Failed!\n" );
			return;
		}

		glbs.FS_WriteFile( filename.c_str(), tempbuf, out_len + 8 );
		glbs.Free( tempbuf );
	}

	if( buffer )
	{
		glbs.Free( ( void * )buffer );
		buffer = NULL;
	}

	filename = "";
	length = 0;
	pos = 0;
}

const char *FileRead::Filename
(
void
)

{
	return filename.c_str();
}

size_t FileRead::Length
(
void
)

{
	return length;
}

size_t FileRead::Pos
(
void
)

{
	return pos - buffer;
}

qboolean FileRead::Seek
(
size_t newpos
)

{
	if( !buffer )
	{
		return false;
	}

	if( newpos > length )
	{
		return false;
	}

	pos = buffer + newpos;

	return true;
}

qboolean FileRead::Open
	(
	const char *name
	)

{
	byte  *tempbuf;
	assert( name );

	assert( !buffer );
	Close();

	if( !name )
	{
		return false;
	}

	length = glbs.FS_ReadFile( name, ( void ** )&tempbuf, qtrue );
	if( length == ( size_t )( -1 ) )
	{
		return false;
	}
	// create our own space
	buffer = ( byte * )glbs.Malloc( length );
	// copy the file over to our space
	memcpy( buffer, tempbuf, length );
	// free the file
	glbs.FS_FreeFile( tempbuf );
	// set the file name
	filename = name;

	pos = buffer;

	char FileHeader[ 4 ];
	Read( FileHeader, sizeof( FileHeader ) );

	if( FileHeader[ 0 ] != 'C' || FileHeader[ 1 ] != 'S' || FileHeader[ 2 ] != 'V' || FileHeader[ 3 ] != 'G' )
	{
		pos = buffer;
	}
	else
	{
		size_t new_len;
		size_t iCSVGLength;

		Read( &new_len, sizeof( unsigned int ) );
		tempbuf = ( byte * )glbs.Malloc( new_len );

		if( g_lz77.Decompress( pos, length - 8, tempbuf, &iCSVGLength ) || iCSVGLength != new_len )
		{
			glbs.Error( ERR_DROP, "Decompression of save game failed\n" );
			return false;
		}

		glbs.Free( buffer );

		buffer = tempbuf;
		length = iCSVGLength;
		pos = buffer;
	}

	return true;
}

qboolean FileRead::Read
	(
	void *dest,
	size_t size
	)

{
	assert( dest );
	assert( buffer );
	assert( pos );

	if( !dest )
	{
		return false;
	}

	if( size <= 0 )
	{
		return false;
	}

	if( ( pos + size ) > ( buffer + length ) )
	{
		return false;
	}

	memcpy( dest, pos, size );
	pos += size;

	return true;
}

CLASS_DECLARATION( Class, Archiver, NULL )
{
	{ NULL, NULL }
};

Archiver::Archiver()
{
	file = 0;
	fileerror = false;
	harderror = true;
	m_iNumBytesIO = 0;
	silent = false;
	assert( ( sizeof( typenames ) / sizeof( typenames[ 0 ] ) ) == ARC_NUMTYPES );
}

Archiver::~Archiver()
{
	if( file )
	{
		Close();
	}

	readfile.Close();
}

void Archiver::FileError
	(
	const char *fmt,
	...
	)

{
	va_list	argptr;
	char		text[ 1024 ];

	va_start( argptr, fmt );
	vsprintf( text, fmt, argptr );
	va_end( argptr );

	fileerror = true;
	Close();
	if( archivemode == ARCHIVE_READ )
	{
		if( harderror )
		{
			glbs.Error( ERR_DROP, "Error while loading %s : %s\n", filename.c_str(), text );
		}
		else if( !silent )
		{
			glbs.Printf( "Error while loading %s : %s\n", filename.c_str(), text );
		}
	}
	else
	{
		if( harderror )
		{
			glbs.Error( ERR_DROP, "Error while writing to %s : %s\n", filename.c_str(), text );
		}
		else if( !silent )
		{
			glbs.Printf( "Error while writing to %s : %s\n", filename.c_str(), text );
		}
	}
}

void Archiver::Close
	(
	void
	)

{
	if( file )
	{
		if( archivemode == ARCHIVE_WRITE )
		{
			int numobjects;

			// write out the number of classpointers
			glbs.FS_Seek( file, numclassespos, FS_SEEK_SET );
			numclassespos = glbs.FS_Tell( file );
			numobjects = classpointerList.NumObjects();
			ArchiveInteger( &numobjects );
		}

		glbs.FS_FCloseFile( file );
		file = NULL;

		readfile.Close();
		// Re-open and compress the file
		readfile.Open( filename.c_str() );
		readfile.Close( true );
	}

	readfile.Close();

	if( archivemode == ARCHIVE_READ )
	{
		int i, num;
		Class * classptr;
		pointer_fixup_t *fixup;

		num = fixupList.NumObjects();
		for( i = 1; i <= num; i++ )
		{
			fixup = fixupList.ObjectAt( i );
			classptr = classpointerList.ObjectAt( fixup->index );
			if( fixup->type == pointer_fixup_normal )
			{
				Class ** fixupptr;
				fixupptr = ( Class ** )fixup->ptr;
				*fixupptr = classptr;
			}
			else if( fixup->type == pointer_fixup_safe )
			{
				SafePtrBase * fixupptr;
				fixupptr = ( SafePtrBase * )fixup->ptr;
				fixupptr->InitSafePtr( classptr );
			}
			delete fixup;
		}
		fixupList.FreeObjectList();
		classpointerList.FreeObjectList();
	}
}

/****************************************************************************************

File Read/Write functions

*****************************************************************************************/

qboolean Archiver::Read
(
const char *name,
qboolean harderror
)

{
	unsigned header;
	unsigned version;
	str		info;
	int      num;
	int      i;
	Class    *null;

	this->harderror = harderror;

	assert( name );
	if( !name )
	{
		FileError( "NULL pointer for filename in Archiver::Read.\n" );
		return false;
	}

	fileerror = false;

	archivemode = ARCHIVE_READ;

	filename = name;

	if( !readfile.Open( filename.c_str() ) )
	{
		if( harderror )
		{
			FileError( "Couldn't open file." );
		}
		fileerror = true;
		return false;
	}

	ArchiveUnsigned( &header );
	if( header != ArchiveHeader )
	{
		readfile.Close();
		FileError( "Not a valid MOHAA archive." );
		return false;
	}

	ArchiveUnsigned( &version );
	if( version > ArchiveVersion )
	{
		readfile.Close();
		FileError( "Archive is from version %.2f.  Check http://www.x-null.net for an update.", version );
		return false;
	}

	if( version < ArchiveVersion )
	{
		readfile.Close();
		FileError( "Archive is out of date." );
		return false;
	}

	ArchiveString( &info );

	// setup out class pointers
	ArchiveInteger( &num );
	classpointerList.Resize( num );
	null = NULL;
	for( i = 1; i <= num; i++ )
	{
		classpointerList.AddObject( null );
	}

	return true;
}

qboolean Archiver::Create
	(
	const char *name,
	qboolean harderror
	)

{
	unsigned header;
	unsigned version;
	str      info;
	int      numZero = 0;

	this->harderror = harderror;

	assert( name );
	if( !name )
	{
		FileError( "NULL pointer for filename in Archiver::Create.\n" );
		return false;
	}

	fileerror = false;

	archivemode = ARCHIVE_WRITE;

	filename = name;

	file = glbs.FS_FOpenFileWrite( filename.c_str() );
	if( !file )
	{
		FileError( "Couldn't open file." );
		return false;
	}

	header = ArchiveHeader;
	ArchiveUnsigned( &header );
	version = ArchiveVersion;
	ArchiveUnsigned( &version );
	info = ArchiveInfo;
	ArchiveString( &info );

	numclassespos = glbs.FS_Tell( file );
	ArchiveInteger( &numZero );

	m_iNumBytesIO = 0;

	return true;
}


inline void Archiver::CheckRead
(
void
)

{
	assert( archivemode == ARCHIVE_READ );
	if( !fileerror && ( archivemode != ARCHIVE_READ ) )
	{
		FileError( "File read during a write operation." );
	}
}

inline void Archiver::CheckWrite
(
void
)

{
	assert( archivemode == ARCHIVE_WRITE );
	if( !fileerror && ( archivemode != ARCHIVE_WRITE ) )
	{
		FileError( "File write during a read operation." );
	}
}

inline size_t Archiver::ReadSize
(
void
)

{
	size_t s;

	s = 0;
	if( !fileerror )
	{
		readfile.Read( &s, sizeof( s ) );
	}

	return s;
}

inline void Archiver::CheckSize
(
int type,
size_t size
)

{
	size_t s;

	if( !fileerror )
	{
		s = ReadSize();

		if( size != s )
		{
			FileError( "Invalid data size of %d on %s.", s, typenames[ type ] );
		}
	}
}

inline void Archiver::WriteSize
(
size_t size
)

{
	glbs.FS_Write( &size, sizeof( size ), file );
}

inline int Archiver::ReadType
(
void
)

{
	int t;

	if( !fileerror )
	{
		readfile.Read( &t, sizeof( t ) );

		return t;
	}

	return ARC_NULL;
}

inline void Archiver::WriteType
	(
	int type
	)

{
	glbs.FS_Write( &type, sizeof( type ), file );
}


inline void Archiver::CheckType
	(
	int type
	)

{
	int t;

	assert( ( type >= 0 ) && ( type < ARC_NUMTYPES ) );

	if( !fileerror )
	{
		t = ReadType();
		if( t != type )
		{
			if( t < ARC_NUMTYPES )
			{
				FileError( "Expecting %s, Should be %s", typenames[ type ], typenames[ t ] );
				assert( 0 );
			}
			else
			{
				FileError( "Expecting %s, Should be %i (Unknown type)", typenames[ type ], t );
			}
		}
	}
}

/****************************************************************************************

File Archive functions

*****************************************************************************************/

//#define ARCHIVE_USE_TYPES 1

inline void Archiver::ArchiveData
	(
	int type,
	void *data,
	size_t size
	)

{
	if( archivemode == ARCHIVE_READ )
	{
#ifndef NDEBUG
		CheckRead();
#endif
#ifdef ARCHIVE_USE_TYPES
		CheckType( type );
#endif

		if( !fileerror && size )
		{
			m_iNumBytesIO += size;
			readfile.Read( data, size );
		}
	}
	else
	{
#ifndef NDEBUG
		CheckWrite();
#endif
#ifdef ARCHIVE_USE_TYPES
		WriteType( type );
#endif

		if( !fileerror && size )
		{
			m_iNumBytesIO += size;
			glbs.FS_Write( data, size, file );
		}
	}
}


#define ARCHIVE( func, type )							\
void Archiver::Archive##func							\
	(													\
	type * v											\
	)													\
														\
{														\
	ArchiveData( ARC_##func, v, sizeof( type ) );		\
}

ARCHIVE( Vector, Vector );
ARCHIVE( Integer, int );
ARCHIVE( Unsigned, unsigned );
ARCHIVE( Size, long );
ARCHIVE( Byte, byte );
ARCHIVE( Char, char );
ARCHIVE( Short, short );
ARCHIVE( UnsignedShort, unsigned short );
ARCHIVE( Float, float );
ARCHIVE( Double, double );
ARCHIVE( Boolean, qboolean );
ARCHIVE( Quat, Quat );
ARCHIVE( Bool, bool );
ARCHIVE( Position, int );

void Archiver::ArchiveSvsTime
	(
	int *time
	)

{
#ifdef GAME_DLL
	if( archivemode == ARCHIVE_READ )
	{
		ArchiveInteger( time );
		glbs.AddSvsTimeFixup( time );
	}
	else
	{
		*time -= level.svsTime;
		ArchiveInteger( time );
		*time += level.svsTime;
	}
#endif
}

void Archiver::ArchiveVec2
	(
	vec2_t vec
	)

{
	ArchiveData( ARC_Vec2, vec, sizeof( vec2_t ) );
}

void Archiver::ArchiveVec3
	(
	vec3_t vec
	)

{
	ArchiveData( ARC_Vec3, vec, sizeof( vec3_t ) );
}

void Archiver::ArchiveVec4
	(
	vec4_t vec
	)

{
	ArchiveData( ARC_Vec4, vec, sizeof( vec4_t ) );
}

void Archiver::ArchiveObjectPointer
(
Class ** ptr
)

{
	int index = 0;

	if( archivemode == ARCHIVE_READ )
	{
		pointer_fixup_t *fixup;
		ArchiveData( ARC_ObjectPointer, &index, sizeof( index ) );

		// Check for a NULL pointer
		assert( ptr );
		if( !ptr )
		{
			FileError( "NULL pointer in ArchiveObjectPointer." );
		}

		//
		// see if the variable was NULL
		//
		if( index == ARCHIVE_NULL_POINTER )
		{
			*ptr = NULL;
		}
		else
		{
			// init the pointer with NULL until we can fix it
			*ptr = NULL;

			fixup = new pointer_fixup_t;
			fixup->ptr = ( void ** )ptr;
			fixup->index = index;
			fixup->type = pointer_fixup_normal;
			fixupList.AddObject( fixup );
		}
	}
	else
	{
		if( *ptr )
		{
			index = classpointerList.AddUniqueObject( *ptr );
		}
		else
		{
			index = ARCHIVE_NULL_POINTER;
		}
		ArchiveData( ARC_ObjectPointer, &index, sizeof( index ) );
	}
}

void Archiver::ArchiveObjectPosition( void *obj )
{
	int index = 0;

	if( archivemode == ARCHIVE_READ )
	{
		ArchivePosition( &index );
		classpointerList.AddObjectAt( index, ( Class * )obj );
	}
	else
	{
		index = classpointerList.AddUniqueObject( ( Class * )obj );
		ArchivePosition( &index );
	}
}

void Archiver::ArchiveSafePointer
(
SafePtrBase * ptr
)

{
	int index = 0;

	if( archivemode == ARCHIVE_READ )
	{
		pointer_fixup_t *fixup;

		ArchiveData( ARC_SafePointer, &index, sizeof( &index ) );

		// Check for a NULL pointer
		assert( ptr );
		if( !ptr )
		{
			FileError( "NULL pointer in ReadSafePointer." );
		}

		//
		// see if the variable was NULL
		//
		if( index == ARCHIVE_NULL_POINTER )
		{
			ptr->InitSafePtr( NULL );
		}
		else
		{
			// init the pointer with NULL until we can fix it
			ptr->InitSafePtr( NULL );

			// Add new fixup
			fixup = new pointer_fixup_t;
			fixup->ptr = ( void ** )ptr;
			fixup->index = index;
			fixup->type = pointer_fixup_safe;
			fixupList.AddObject( fixup );
		}
	}
	else
	{
		if( ptr->Pointer() )
		{
			Class * obj;

			obj = ptr->Pointer();
			index = classpointerList.AddUniqueObject( obj );
		}
		else
		{
			index = ARCHIVE_NULL_POINTER;
		}
		ArchiveData( ARC_SafePointer, &index, sizeof( index ) );
	}
}

void Archiver::ArchiveEventPointer
(
Event ** ev
)

{
	int index;

	if( archivemode == ARCHIVE_READ )
	{
#ifndef NDEBUG
		CheckRead();
#endif
#ifdef ARCHIVE_USE_TYPES
		CheckType( ARC_EventPointer );
#endif
		ArchiveInteger( &index );

		if( !fileerror )
		{
			if( index == ARCHIVE_POINTER_VALID )
			{
				*ev = new Event;
				( *ev )->Archive( *this );
			}
			else
			{
				( *ev ) = NULL;
			}
		}
	}
	else
	{
#ifndef NDEBUG
		CheckWrite();
#endif
		if( *ev )
		{
			index = ARCHIVE_POINTER_VALID;
		}
		else
		{
			index = ARCHIVE_NULL_POINTER;
		}

#ifdef ARCHIVE_USE_TYPES
		WriteType( ARC_EventPointer );
#endif

		ArchiveInteger( &index );
		if( *ev )
		{
			( *ev )->Archive( *this );
		}
	}
}

void Archiver::ArchiveRaw
(
void *data,
size_t size
)

{
	ArchiveData( ARC_Raw, data, size );
}

void Archiver::ArchiveString
(
str * string
)

{
	if( archivemode == ARCHIVE_READ )
	{
		size_t	s;
		char		*data;

#ifndef NDEBUG
		CheckRead();
#endif
#ifdef ARCHIVE_USE_TYPES
		CheckType( ARC_String );
#endif

		if( !fileerror )
		{
			s = ReadSize();
			if( !fileerror )
			{
				data = new char[ s + 1 ];
				if( data )
				{
					if( s )
					{
						readfile.Read( data, s );
					}
					data[ s ] = 0;

					*string = data;

					delete[] data;
				}
			}
		}
	}
	else
	{
#ifndef NDEBUG
		CheckWrite();
#endif
#ifdef ARCHIVE_USE_TYPES
		WriteType( ARC_String );
#endif
		WriteSize( string->length() );
		glbs.FS_Write( ( void * )string->c_str(), string->length(), file );
	}
}

void Archiver::ArchiveConfigString( int cs )
{
#ifdef GAME_DLL
	str s;

	if( archivemode == ARCHIVE_READ )
	{
		ArchiveString( &s );
		glbs.SetConfigstring( cs, s.c_str() );
	}
	else
	{
		s = glbs.GetConfigstring( cs );
		ArchiveString( &s );
	}
#endif
}

Class * Archiver::ReadObject
	(
	void
	)

{
	ClassDef	*cls;
	Class		*obj;
	str		classname;
	size_t		objstart;
	size_t		endpos;
	int      index;
	size_t	size;
	qboolean isent;
	int      type;

	CheckRead();

	type = ReadType();
	if( ( type != ARC_Object ) && ( type != ARC_Entity ) )
	{
		FileError( "Expecting %s or %s", typenames[ ARC_Object ], typenames[ ARC_Entity ] );
	}

	size = ReadSize();
	ArchiveString( &classname );

	cls = getClass( classname.c_str() );
	if( !cls )
	{
		FileError( "Invalid class %s.", classname.c_str() );
	}

#if defined ( GAME_DLL )
	isent = checkInheritance( &Entity::ClassInfo, cls );
	if( type == ARC_Entity )
	{
		if( !isent )
		{
			FileError( "Non-Entity class object '%s' saved as an Entity based object.", classname.c_str() );
		}

		ArchiveInteger( &level.spawn_entnum );
		//
		// make sure to setup spawnflags properly
		//
		ArchiveInteger( &level.spawnflags );
	}
	else if( isent )
	{
		FileError( "Entity class object '%s' saved as non-Entity based object.", classname.c_str() );
	}
#else
	isent = false;
#endif

	ArchiveInteger( &index );
	objstart = readfile.Pos();


	obj = ( Class * )cls->newInstance();
	if( !obj )
	{
		FileError( "Failed to on new instance of class %s.", classname.c_str() );
	}
	else
	{
		obj->Archive( *this );
	}

	if( !fileerror )
	{
		endpos = readfile.Pos();
		if( ( endpos - objstart ) > size )
		{
			FileError( "Object read past end of object's data" );
		}
		else if( ( endpos - objstart ) < size )
		{
			FileError( "Object didn't read entire data from file" );
		}
	}

	//
	// register this pointer with our list
	//
	classpointerList.AddObjectAt( index, obj );

	return obj;
}

void Archiver::ArchiveObject
	(
	Class *obj
	)

{
	str		classname;
	int      index;
	size_t	size;
	qboolean isent;

	if( archivemode == ARCHIVE_READ )
	{
		ClassDef	*cls;
		size_t		objstart;
		size_t		endpos;
		int      type;

		CheckRead();
		type = ReadType();
		if( ( type != ARC_Object ) && ( type != ARC_Entity ) )
		{
			FileError( "Expecting %s or %s", typenames[ ARC_Object ], typenames[ ARC_Entity ] );
		}

		size = ReadSize();
		ArchiveString( &classname );

		cls = getClass( classname.c_str() );
		if( !cls )
		{
			FileError( "Invalid class %s.", classname.c_str() );
		}

		if( obj->classinfo() != cls )
		{
			FileError( "Archive has a '%s' object, but was expecting a '%s' object.", classname.c_str(), obj->getClassname() );
		}

#if defined ( GAME_DLL )
		isent = obj->isSubclassOf( Entity );
		if( type == ARC_Entity )
		{
			int entnum;
			if( !isent )
			{
				FileError( "Non-Entity class object '%s' saved as an Entity based object.", classname.c_str() );
			}

			ArchiveInteger( &entnum );
			( ( Entity * )obj )->SetEntNum( entnum );
			//
			// make sure to setup spawnflags properly
			//
			ArchiveInteger( &level.spawnflags );
		}
		else if( isent )
		{
			FileError( "Entity class object '%s' saved as non-Entity based object.", classname.c_str() );
		}
#else
		isent = false;
#endif

		ArchiveInteger( &index );
		objstart = readfile.Pos();

		obj->Archive( *this );

		if( !fileerror )
		{
			endpos = readfile.Pos();
			if( ( endpos - objstart ) > size )
			{
				FileError( "Object read past end of object's data" );
			}
			else if( ( endpos - objstart ) < size )
			{
				FileError( "Object didn't read entire data from file" );
			}
		}

		//
		// register this pointer with our list
		//
		classpointerList.AddObjectAt( index, obj );
	}
	else
	{
		long		sizepos;
		long		objstart = 0;
		long		endpos;

		assert( obj );
		if( !obj )
		{
			FileError( "NULL object in WriteObject" );
		}

#if defined ( GAME_DLL )
		isent = obj->isSubclassOf( Entity );
#else
		isent = false;
#endif

		CheckWrite();
		if( isent )
		{
			WriteType( ARC_Entity );
		}
		else
		{
			WriteType( ARC_Object );
		}

		sizepos = glbs.FS_Tell( file );
		size = 0;
		WriteSize( size );

		classname = obj->getClassname();
		ArchiveString( &classname );

#if defined ( GAME_DLL )
		if( isent )
		{
			// Write out the entity number
			ArchiveInteger( &( ( Entity * )obj )->entnum );
			//
			// make sure to setup spawnflags properly
			//
			ArchiveInteger( &( ( Entity * )obj )->spawnflags );
		}
#endif

		// write out pointer index for this class pointer
		index = classpointerList.AddUniqueObject( obj );
		ArchiveInteger( &index );

		if( !fileerror )
		{
			objstart = glbs.FS_Tell( file );
			obj->Archive( *this );
		}

		if( !fileerror )
		{
			endpos = glbs.FS_Tell( file );
			size = endpos - objstart;
			glbs.FS_Seek( file, sizepos, FS_SEEK_SET );
			WriteSize( size );

			if( !fileerror )
			{
				glbs.FS_Seek( file, endpos, FS_SEEK_SET );
			}
		}
	}
}

qboolean Archiver::ObjectPositionExists( void *obj )
{
	return classpointerList.IndexOfObject( ( Class * )obj ) != 0;
}

void Archiver::SetSilent( bool bSilent )
{
	silent = bSilent;
}
