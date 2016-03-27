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


CLASS_DECLARATION( Class, FileRead, NULL )
{
	{ NULL, NULL }
};

FileRead::FileRead()
{
}

FileRead::~FileRead()
{
}

void FileRead::Close
	(
	bool bDoCompression
	)

{
}

const char *FileRead::Filename
(
void
)

{
	return "";
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
	return 0;
}

qboolean FileRead::Seek
(
size_t newpos
)

{
	return qfalse;
}

qboolean FileRead::Open
	(
	const char *name
	)

{
	return qfalse;
}

qboolean FileRead::Read
	(
	void *dest,
	size_t size
	)

{
	return qfalse;
}

CLASS_DECLARATION( Class, Archiver, NULL )
{
	{ NULL, NULL }
};

Archiver::Archiver()
{
}

Archiver::~Archiver()
{
}

void Archiver::FileError
	(
	const char *fmt,
	...
	)

{
}

void Archiver::Close
	(
	void
	)

{
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
	return qfalse;
}

qboolean Archiver::Create
	(
	const char *name,
	qboolean harderror
	)

{
	return qfalse;
}


inline void Archiver::CheckRead
(
void
)

{
}

inline void Archiver::CheckWrite
(
void
)

{
}

inline size_t Archiver::ReadSize
(
void
)

{
}

inline void Archiver::CheckSize
(
int type,
size_t size
)

{
}

inline void Archiver::WriteSize
(
size_t size
)

{
}

inline int Archiver::ReadType
(
void
)

{
}

inline void Archiver::WriteType
	(
	int type
	)

{
}


inline void Archiver::CheckType
	(
	int type
	)

{
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
}


#define ARCHIVE( func, type )							\
void Archiver::Archive##func							\
	(													\
	type * v											\
	)													\
														\
{														\
}

ARCHIVE( Vector, Vector );
ARCHIVE( Integer, int );
ARCHIVE( Unsigned, unsigned );
ARCHIVE( Size, size_t );
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
}

void Archiver::ArchiveVec2
	(
	vec2_t vec
	)

{
}

void Archiver::ArchiveVec3
	(
	vec3_t vec
	)

{
}

void Archiver::ArchiveVec4
	(
	vec4_t vec
	)

{
}

void Archiver::ArchiveObjectPointer
(
Class ** ptr
)

{
}

void Archiver::ArchiveObjectPosition( void *obj )
{
}

void Archiver::ArchiveSafePointer
(
SafePtrBase * ptr
)

{
}

void Archiver::ArchiveEventPointer
(
Event ** ev
)

{
}

void Archiver::ArchiveRaw
(
void *data,
size_t size
)

{
}

void Archiver::ArchiveString
(
str * string
)

{
}

void Archiver::ArchiveConfigString( int cs )
{
}

Class * Archiver::ReadObject
	(
	void
	)

{
	return NULL;
}

void Archiver::ArchiveObject
	(
	Class *obj
	)

{
}

qboolean Archiver::ObjectPositionExists( void *obj )
{
	return qfalse;
}

void Archiver::SetSilent( bool bSilent )
{
}
