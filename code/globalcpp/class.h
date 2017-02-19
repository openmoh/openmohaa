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

// class.h: General class

#ifndef __CLASS_H__
#define __CLASS_H__

#if defined ( GAME_DLL )

#include "g_local.h"

#else

#include "glb_local.h"

#endif

class Class;
class Event;

#define isSubclassOf( classname )inheritsFrom( &classname::ClassInfo )
#define isSuperclassOf( classname )isInheritedBy( &classname::ClassInfo )

#ifndef NO_SCRIPTENGINE

#define CLASS_DECLARATION( parentclass, classname, classid )	\
	ClassDef classname::ClassInfo								\
	(															\
		#classname, classid, #parentclass,						\
		(ResponseDef<Class> *)classname::Responses,				\
		classname::_newInstance, sizeof( classname )			\
	);															\
	void *classname::_newInstance( void )						\
	{															\
		return new classname;									\
	}															\
	ClassDef *classname::classinfo( void ) const				\
	{															\
		return &(classname::ClassInfo);							\
	}															\
	ClassDef *classname::classinfostatic( void )				\
	{															\
		return &(classname::ClassInfo);							\
	}															\
	void classname::AddWaitTill( str s )						\
	{															\
		classname::ClassInfo.AddWaitTill( s );					\
	}															\
	void classname::AddWaitTill( const_str s )					\
	{															\
		classname::ClassInfo.AddWaitTill( s );					\
	}															\
	void classname::RemoveWaitTill( str s )						\
	{															\
		classname::ClassInfo.RemoveWaitTill( s );				\
	}															\
	void classname::RemoveWaitTill( const_str s )				\
	{															\
		classname::ClassInfo.RemoveWaitTill( s );				\
	}															\
	bool classname::WaitTillDefined( str s )					\
	{															\
		return classname::ClassInfo.WaitTillDefined( s );		\
	}															\
	bool classname::WaitTillDefined( const_str s )				\
	{															\
		return classname::ClassInfo.WaitTillDefined( s );		\
	}															\
	ResponseDef<classname> classname::Responses[] =

#define CLASS_PROTOTYPE( classname )								\
	public:															\
	static	ClassDef				ClassInfo;						\
	static	ClassDefHook			_ClassInfo_;					\
	static	void					*_newInstance( void );			\
	static	ClassDef				*classinfostatic(void);			\
	virtual	ClassDef				*classinfo(void) const;			\
	static void						AddWaitTill( str s );			\
	static void						AddWaitTill( const_str s );		\
	static void						RemoveWaitTill( str s );		\
	static void						RemoveWaitTill( const_str s );	\
	static bool						WaitTillDefined( str s );		\
	static bool						WaitTillDefined( const_str s );	\
	static	ResponseDef<classname>	Responses[]

#else

#define CLASS_DECLARATION( parentclass, classname, classid )	\
	ClassDef classname::ClassInfo								\
	(															\
		#classname, classid, #parentclass,						\
		(ResponseDef<Class> *)classname::Responses,				\
		classname::_newInstance, sizeof( classname )			\
	);															\
	void *classname::_newInstance( void )						\
	{															\
		return new classname;									\
	}															\
	ClassDef *classname::classinfo( void ) const				\
	{															\
		return &(classname::ClassInfo);							\
	}															\
	ClassDef *classname::classinfostatic( void )				\
	{															\
		return &(classname::ClassInfo);							\
	}															\
	ResponseDef<classname> classname::Responses[] =

#define CLASS_PROTOTYPE( classname )								\
	public:															\
	static	ClassDef				ClassInfo;						\
	static	ClassDefHook			_ClassInfo_;					\
	static	void					*_newInstance( void );			\
	static	ClassDef				*classinfostatic(void);			\
	virtual	ClassDef				*classinfo(void) const;			\
	static	ResponseDef<classname>	Responses[]

#endif

typedef void ( Class::*Response )( Event *ev );

class EventDef;

template< class Type >
struct ResponseDef
{
	Event			*event;
	void			( Type::*response )( Event *ev );
	EventDef		*def;
};

class ClassDef
{
public:
	const char			*classname;
	const char			*classID;
	const char			*superclass;
	void				*(*newInstance)(void);
	int					classSize;
	ResponseDef<Class>	*responses;
	ResponseDef<Class>	**responseLookup;
	ClassDef			*super;
	ClassDef			*next;
	ClassDef			*prev;

#ifndef NO_SCRIPTENGINE
	con_set<const_str, const_str>	*waitTillSet;
#endif

	int					numEvents;

	static ClassDef		*classlist;
	static int			numclasses;

	static void			BuildEventResponses();

	void				BuildResponseList();

	static				int dump_numclasses;
	static				int dump_numevents;
	static				Container< int > sortedList;
	static				Container< ClassDef * > sortedClassList;

	ClassDef();
	~ClassDef();

	static		int compareClasses( const void *arg1, const void *arg2 );
	static		void SortClassList( Container< ClassDef * > *sortedList );

#ifndef NO_SCRIPTENGINE
	void		AddWaitTill( str s );
	void		AddWaitTill( const_str s );
	void		RemoveWaitTill( str s );
	void		RemoveWaitTill( const_str s );
	bool		WaitTillDefined( str s );
	bool		WaitTillDefined( const_str s );
#endif

	/* Create-a-class function */
	ClassDef( const char *classname, const char *classID, const char *superclass, ResponseDef<Class> *responses,
		void *( *newInstance )( void ), int classSize );
	void CreateInternal( const char *classname, const char *classID, const char *superclass, ResponseDef<Class> *responses,
		void *( *newInstance )( void ), int classSize );
	void CreateInternalWin( const char *classname, const char *classID, const char *superclass, ResponseDef<Class> *responses,
		void *( *newInstance )( void ), int classSize );

	EventDef	*GetDef( int eventnum );
	int			GetFlags( Event *event );

	void Destroy();
};

ClassDef *getClassList( void );
qboolean checkInheritance( const ClassDef *superclass, const ClassDef *subclass );
qboolean checkInheritance( ClassDef *superclass, const char *subclass );
qboolean checkInheritance( const char *superclass, const char *subclass );
void CLASS_Print( FILE *class_file, const char *fmt, ... );
void ClassEvents( const char *classname, qboolean print_to_disk );
void DumpClass( FILE * class_file, const char * className );
void DumpAllClasses( void );

class ClassDefHook
{
private:
	ClassDef *classdef;

public:
	//void * operator new( size_t );
	//void operator delete( void * );

	ClassDefHook();
	~ClassDefHook();

	/* Hook-a-class function */
	ClassDefHook( ClassDef * classdef, ResponseDef<Class> *responses );
};

ClassDef *getClassForID( const char *name );
ClassDef *getClass( const char * name );
ClassDef *getClassList( void );
void listAllClasses( void );
void listInheritanceOrder( const char *classname );

class SafePtrBase;
class Archiver;

class Class
{
public:
	SafePtrBase *SafePtrList;

private:
	void			ClearSafePointers();

public:
	CLASS_PROTOTYPE( Class );

#ifndef _DEBUG_MEM
	void * operator	new( size_t );
	void operator delete( void * );
#endif

	Class();
	virtual ~Class();

	virtual void	Archive( Archiver& arc );

	const char		*getClassID( void ) const;
	const char		*getClassname( void ) const;
	const char		*getSuperclass( void ) const;

	qboolean		inheritsFrom( ClassDef *c ) const;
	qboolean		inheritsFrom( const char * name ) const;
	qboolean		isInheritedBy( const char * name ) const;
	qboolean		isInheritedBy( ClassDef *c ) const;

	void			warning( const char *function, const char *format, ... );
	void			error( const char *function, const char *format, ... );
};

#include "safeptr.h"

#endif /* __CLASS_H__ */
