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

// class.cpp: General class.

#include "glb_local.h"
#include "scriptmaster.h"
#include <dbgheap.h>

#define CLASS_Printf if( level.current_map ) glbs.Printf
#define CLASS_DPrintf if( level.current_map ) glbs.DPrintf
#define CLASS_Error if( level.current_map ) glbs.Error

ClassDef *ClassDef::classlist;
int ClassDef::numclasses;

int ClassDef::dump_numclasses;
int ClassDef::dump_numevents;
Container< int > ClassDef::sortedList;
Container< ClassDef * > ClassDef::sortedClassList;

int ClassDef::compareClasses( const void *arg1, const void *arg2 )
{
	ClassDef *c1 = *( ClassDef ** )arg1;
	ClassDef *c2 = *( ClassDef ** )arg2;

	return Q_stricmp( c1->classname, c2->classname );
}

void ClassDef::SortClassList( Container< ClassDef * > *sortedList )
{
	ClassDef *c;

	sortedList->Resize( numclasses );

	for( c = classlist->next; c != classlist; c = c->next )
	{
		sortedList->AddObject( c );
	}

	qsort( ( void * )sortedList->AddressOfObjectAt( 1 ),
		( size_t )sortedList->NumObjects(),
		sizeof( ClassDef * ), compareClasses );
}

ClassDef *getClassForID( const char *name )
{
	ClassDef *classlist = ClassDef::classlist;
	ClassDef *c;

	for( c = classlist->next; c != classlist; c = c->next )
	{
		if( c->classID && !Q_stricmp( c->classID, name ) )
		{
			return c;
		}
	}

	return NULL;
}

ClassDef *getClass( const char *name )
{
	if( name == NULL || name == "" ) {
		return NULL;
	}

	ClassDef *list = ClassDef::classlist;
	ClassDef *c;

	for( c = list->next; c != list; c = c->next )
	{
		if( Q_stricmp( c->classname, name ) == 0 ) {
			return c;
		}
	}

	return NULL;
}

ClassDef *getClassList
	(
	void
	)

{
	return ClassDef::classlist;
}

void listAllClasses
	(
	void
	)

{
	ClassDef *c;
	ClassDef *list = ClassDef::classlist;

	for( c = list->next; c != list; c = c->next )
	{
		CLASS_DPrintf( "%s\n", c->classname );
	}
}

void listInheritanceOrder
	(
	const char *classname
	)

{
	ClassDef *cls;
	ClassDef *c;

	cls = getClass( classname );
	if( !cls )
	{
		CLASS_DPrintf( "Unknown class: %s\n", classname );
		return;
	}
	for( c = cls; c != NULL; c = c->super )
	{
		CLASS_DPrintf( "%s\n", c->classname );
	}
}

qboolean checkInheritance( const ClassDef *superclass, const ClassDef *subclass )
{
	const ClassDef *c;

	for( c = subclass; c != NULL; c = c->super )
	{
		if ( c == superclass )
		{
			return true;
		}
	}
	return false;
}

qboolean checkInheritance( ClassDef *superclass, const char *subclass )
{
	ClassDef *c;

	c = getClass( subclass );

	if ( c == NULL )
	{
		CLASS_DPrintf( "Unknown class: %s\n", subclass );
		return false;
	}

	return checkInheritance( superclass, c );
}

qboolean checkInheritance( const char *superclass, const char *subclass )
{
	ClassDef *c1;
	ClassDef *c2;

	c1 = getClass( superclass );
	c2 = getClass( subclass );

	if ( c1 == NULL )
	{
		CLASS_DPrintf( "Unknown class: %s\n", superclass );
		return false;
	}

	if ( c2 == NULL )
	{
		CLASS_DPrintf( "Unknown class: %s\n", subclass );
		return false;
	}

	return checkInheritance( c1, c2 );
}

void CLASS_Print( FILE *class_file, const char *fmt, ... )
{
	va_list	argptr;
	char		text[ 1024 ];

	va_start( argptr, fmt );
	vsprintf( text, fmt, argptr );
	va_end( argptr );

	if( class_file )
		fprintf( class_file, text );
	else
		CLASS_DPrintf( text );
}

size_t totalmemallocated = 0;
unsigned int numclassesallocated = 0;

bool classInited = false;

#ifndef _DEBUG_MEM
void *Class::operator new( size_t s )
{
	size_t *p;

	if ( s == 0 )
		return 0;

	s += sizeof( size_t );

#ifdef GAME_DLL
	p = ( size_t * )gi.Malloc( s );
#elif defined ( CGAME_DLL )
	p = ( size_t * )cgi.Malloc( s );
#else
	p = ( size_t * )glbs.Malloc( s );
#endif

#ifdef _DEBUG
	m_Heap.ReferencePointer( p );
#endif

	*p = s;

	totalmemallocated += s;
	numclassesallocated++;

	p++;

	return p;
}

void Class::operator delete( void *ptr )
{
	size_t *p = ( ( size_t * )ptr  ) - 1;

	totalmemallocated -= *p;
	numclassesallocated--;

#ifdef _DEBUG
	m_Heap.DereferencePointer( p );
#endif

#ifdef GAME_DLL
	gi.Free( p );
#elif defined ( CGAME_DLL )
	cgi.Free( p );
#else
	glbs.Free( p );
#endif
}

#endif

Class::Class()
{
	SafePtrList = NULL;
}

Class::~Class()
{
	ClearSafePointers();
}

void Class::Archive( Archiver& arc )
{

}

void Class::ClearSafePointers( void )
{
	while( SafePtrList != NULL ) {
		SafePtrList->Clear();
	}
}

void Class::warning( const char *function, const char *format, ... )
{
	char buffer[ MAX_STRING_CHARS ];
	const char *classname;
	va_list va;

	va_start( va, format );
	vsprintf( buffer, format, va );

	classname = classinfo()->classname;

#ifdef GAME_DLL
	gi.DPrintf(
#elif defined CGAME_DLL
	cgi.DPrintf(
#else
	Com_DPrintf(
#endif
		"%s::%s : %s\n", classname, function, buffer );
}

void Class::error
	(
	const char *function,
	const char *fmt,
	...
	)

{
	va_list	argptr;
	char		text[ 1024 ];

	va_start( argptr, fmt );
	vsprintf( text, fmt, argptr );
	va_end( argptr );

	if( getClassID() )
	{
		CLASS_Error( ERR_DROP, "%s::%s : %s\n", getClassID(), function, text );
	}
	else
	{
		CLASS_Error( ERR_DROP, "%s::%s : %s\n", getClassname(), function, text );
	}
}

ClassDef::ClassDef()
{
	this->classname			= NULL;
	this->classID			= NULL;
	this->superclass		= NULL;
	this->responses			= NULL;
	this->numEvents			= 0;
	this->responseLookup	= NULL;
	this->newInstance		= NULL;
	this->classSize			= 0;
	this->super				= NULL;
	this->prev				= this;
	this->next				= this;

#ifndef NO_SCRIPTENGINE
	this->waitTillSet		= NULL;
#endif
}

ClassDef::ClassDef( const char *classname, const char *classID, const char *superclass, ResponseDef<Class> *responses,
	void *( *newInstance )( void ), int classSize )
{
	ClassDef *node;

	if( classlist == NULL ) {
		classlist = new ClassDef;
	}

	this->classname			= classname;
	this->classID			= classID;
	this->superclass		= superclass;
	this->responses			= responses;
	this->numEvents			= 0;
	this->responseLookup	= NULL;
	this->newInstance		= newInstance;
	this->classSize			= classSize;
	this->super				= getClass( superclass );

#ifndef NO_SCRIPTENGINE
	this->waitTillSet		= NULL;
#endif

	if( !classID ) {
		this->classID = "";
	}

	for( node = classlist->next; node != classlist; node = node->next )
	{
		if( ( node->super == NULL ) && ( !Q_stricmp( node->superclass, this->classname ) ) &&
			( Q_stricmp( node->classname, "Class" ) ) )
		{
			node->super = this;
		}
	}

	// Add to front of list
	LL_Add( classlist, this, prev, next );

	numclasses++;
}

ClassDef::~ClassDef()
{
	ClassDef *node;

	if( classlist != this )
	{
		LL_Remove( this, prev, next );

		// Check if any subclasses were initialized before their superclass
		for( node = classlist->next; node != classlist; node = node->next )
		{
			if( node->super == this )
			{
				node->super = NULL;
			}
		}
	}
	else
	{
		// If the head of the list is deleted before the list is cleared, then we may have problems
		assert( this->next == this->prev );
	}

	if( responseLookup )
	{
		delete[] responseLookup;
		responseLookup = NULL;
	}
}

#ifndef NO_SCRIPTENGINE

void ClassDef::AddWaitTill( str s )
{
	return AddWaitTill( Director.AddString( s ) );
}

void ClassDef::AddWaitTill( const_str s )
{
	if( !waitTillSet )
	{
		waitTillSet = new con_set < const_str, const_str >;
	}

	waitTillSet->addKeyValue( s ) = s;
}

void ClassDef::RemoveWaitTill( str s )
{

	return RemoveWaitTill( Director.AddString( s ) );
}

void ClassDef::RemoveWaitTill( const_str s )
{
	if( waitTillSet )
	{
		waitTillSet->remove( s );
	}
}

bool ClassDef::WaitTillDefined( str s )
{
	return WaitTillDefined( Director.AddString( s ) );
}

bool ClassDef::WaitTillDefined( const_str s )
{
	if( !waitTillSet )
	{
		return false;
	}

	return waitTillSet->findKeyValue( s ) != NULL;
}

#endif

EventDef *ClassDef::GetDef( int eventnum )
{
	ResponseDef< Class > *r = responseLookup[ eventnum ];

	if( r )
	{
		return r->def;
	}
	else
	{
		return NULL;
	}
}

int ClassDef::GetFlags( Event *event )
{
	EventDef *def = GetDef( event->eventnum );

	if( def )
	{
		return def->flags;
	}
	else
	{
		return 0;
	}
}

void ClassDef::BuildResponseList( void )
{
	ClassDef		*c;
	ResponseDef<Class> *r;
	int			ev;
	int			i;
	qboolean		*set;
	int			num;

	if( responseLookup )
	{
		delete[] responseLookup;
		responseLookup = NULL;
	}

	num = Event::NumEventCommands();
	responseLookup = ( ResponseDef< Class > ** )new char[ sizeof( ResponseDef< Class > * ) * num ];
	memset( responseLookup, 0, sizeof( ResponseDef< Class > * ) * num );

	set = new qboolean[ num ];
	memset( set, 0, sizeof( qboolean ) * num );

	this->numEvents = num;

	for( c = this; c != NULL; c = c->super )
	{
		r = c->responses;

		if( r )
		{
			for( i = 0; r[ i ].event != NULL; i++ )
			{
				ev = ( int )r[ i ].event->eventnum;
				r[ i ].def = r[ i ].event->getInfo();

				if( !set[ ev ] )
				{
					set[ ev ] = true;

					if( r[ i ].response )
					{
						responseLookup[ ev ] = &r[ i ];
					}
					else
					{
						responseLookup[ ev ] = NULL;
					}
				}
			}
		}
	}

	delete[] set;
}

void ClassDef::BuildEventResponses( void )
{
	ClassDef *c;
	int amount;
	int numclasses;

	amount = 0;
	numclasses = 0;

	for( c = classlist->next; c != classlist; c = c->next )
	{
		c->BuildResponseList();

		amount += c->numEvents * sizeof( Response * );
		numclasses++;
	}

	CLASS_DPrintf( "\n------------------\nEvent system initialized: "
		"%d classes %d events %d total memory in response list\n\n", numclasses, Event::NumEventCommands(), amount );
}

const char *Class::getClassID( void ) const
{
	return classinfo()->classID;
}

const char *Class::getClassname( void ) const
{
	return classinfo()->classname;
}

const char *Class::getSuperclass( void ) const
{
	return classinfo()->superclass;
}

#define MAX_INHERITANCE 64
void ClassEvents
	(
	const char *classname,
	qboolean print_to_disk
	)

{
	ClassDef		*c;
	ResponseDef<Class> *r;
	int			ev;
	int			i, j;
	qboolean    *set;
	int			num, orderNum;
	Event       **events;
	byte        *order;
	FILE        *class_file;
	str         classNames[ MAX_INHERITANCE ];
	str         class_filename;

	c = getClass( classname );
	if( !c )
	{
		CLASS_DPrintf( "Unknown class: %s\n", classname );
		return;
	}

	class_file = NULL;

	if( print_to_disk )
	{
		class_filename = str( classname ) + ".txt";
		class_file = fopen( class_filename.c_str(), "w" );
		if( class_file == NULL )
			return;
	}

	num = Event::NumEventCommands();

	set = new qboolean[ num ];
	memset( set, 0, sizeof( qboolean ) * num );

	events = new Event *[ num ];
	memset( events, 0, sizeof( Event * ) * num );

	order = new byte[ num ];
	memset( order, 0, sizeof( byte ) * num );

	orderNum = 0;
	for( ; c != NULL; c = c->super )
	{
		if( orderNum < MAX_INHERITANCE )
		{
			classNames[ orderNum ] = c->classname;
		}
		r = c->responses;
		if( r )
		{
			for( i = 0; r[ i ].event != NULL; i++ )
			{
				ev = ( int )r[ i ].event->eventnum;
				if( !set[ ev ] )
				{
					set[ ev ] = true;

					if( r[ i ].response )
					{
						events[ ev ] = r[ i ].event;
						order[ ev ] = orderNum;
					}
				}
			}
		}
		orderNum++;
	}

	CLASS_Print( class_file, "********************************************************\n" );
	CLASS_Print( class_file, "********************************************************\n" );
	CLASS_Print( class_file, "* All Events For Class: %s\n", classname );
	CLASS_Print( class_file, "********************************************************\n" );
	CLASS_Print( class_file, "********************************************************\n\n" );

	for( j = orderNum - 1; j >= 0; j-- )
	{
		CLASS_Print( class_file, "\n********************************************************\n" );
		CLASS_Print( class_file, "* Class: %s\n", classNames[ j ].c_str() );
		CLASS_Print( class_file, "********************************************************\n\n" );
		for( i = 1; i < num; i++ )
		{
			int index;

			index = ClassDef::sortedList.ObjectAt( i );
			if( events[ index ] && ( order[ index ] == j ) )
			{
				Event::eventDefList[ events[ index ] ].PrintEventDocumentation( class_file, qfalse );
			}
		}
	}

	if( class_file != NULL )
	{
		CLASS_DPrintf( "Printed class info to file %s\n", class_filename.c_str() );
		fclose( class_file );
	}

	delete[] events;
	delete[] order;
	delete[] set;
}

void DumpClass
	(
	FILE * class_file,
	const char * className
	)

{
	ClassDef		*c;
	ResponseDef<Class> *r;
	int			ev;
	int			i;
	int			num, num2;
	Event       **events;

	c = getClass( className );
	if( !c )
	{
		return;
	}

	num = Event::commandList.size();
	num2 = Event::NumEventCommands();

	events = new Event *[ num2 ];
	memset( events, 0, sizeof( Event * ) * num2 );

	// gather event responses for this class
	r = c->responses;
	if( r )
	{
		for( i = 0; r[ i ].event != NULL; i++ )
		{
			ev = ( int )r[ i ].event->eventnum;
			if( r[ i ].response )
			{
				events[ ev ] = r[ i ].event;
			}
		}
	}

	CLASS_Print( class_file, "\n" );
	if( c->classID[ 0 ] )
	{
		CLASS_Print( class_file, "<h2> <a name=\"%s\">%s (<i>%s</i>)</a>", c->classname, c->classname, c->classID );
	}
	else
	{
		CLASS_Print( class_file, "<h2> <a name=\"%s\">%s</a>", c->classname, c->classname );
	}

	// print out lineage
	for( c = c->super; c != NULL; c = c->super )
	{
		CLASS_Print( class_file, " -> <a href=\"#%s\">%s</a>", c->classname, c->classname );
	}
	CLASS_Print( class_file, "</h2>\n" );

	ClassDef::dump_numclasses++;

	CLASS_Print( class_file, "<BLOCKQUOTE>\n" );
	for( i = 1; i < num; i++ )
	{
		int index;

		index = ClassDef::sortedList.ObjectAt( i );
		if( events[ index ] )
		{
			Event::eventDefList[ events[ index ] ].PrintEventDocumentation( class_file, qtrue );
			ClassDef::dump_numevents++;
		}
	}
	CLASS_Print( class_file, "</BLOCKQUOTE>\n" );
	delete[] events;
}


#define MAX_CLASSES 1024
void DumpAllClasses
	(
	void
	)

{
	int i, num;
	ClassDef *c;
	FILE * class_file;
	str class_filename;
	str class_title;
	str classes[ MAX_CLASSES ];

#if defined( GAME_DLL )
	class_filename = "g_allclasses.html";
	class_title = "Game Module";
#elif defined( CGAME_DLL )
	class_filename = "cg_allclasses.html";
	class_title = "Client Game Module";
#else
	class_filename = "cl_allclasses.html";
	class_title = "Client Module";
#endif

	class_file = fopen( class_filename.c_str(), "w" );
	if( class_file == NULL )
		return;

	// construct the HTML header for the document
	CLASS_Print( class_file, "<HTML>\n" );
	CLASS_Print( class_file, "<HEAD>\n" );
	CLASS_Print( class_file, "<Title>%s Classes</Title>\n", class_title.c_str() );
	CLASS_Print( class_file, "</HEAD>\n" );
	CLASS_Print( class_file, "<BODY>\n" );
	CLASS_Print( class_file, "<H1>\n" );
	CLASS_Print( class_file, "<center>%s Classes</center>\n", class_title.c_str() );
	CLASS_Print( class_file, "</H1>\n" );
#if defined( GAME_DLL )
	//
	// print out some commonly used classnames
	//
	CLASS_Print( class_file, "<h2>" );
	CLASS_Print( class_file, "<a href=\"#Actor\">Actor</a>, " );
	CLASS_Print( class_file, "<a href=\"#Animate\">Animate</a>, " );
	CLASS_Print( class_file, "<a href=\"#Entity\">Entity</a>, " );
	CLASS_Print( class_file, "<a href=\"#ScriptSlave\">ScriptSlave</a>, " );
	CLASS_Print( class_file, "<a href=\"#ScriptThread\">ScriptThread</a>, " );
	CLASS_Print( class_file, "<a href=\"#Sentient\">Sentient</a>, " );
	CLASS_Print( class_file, "<a href=\"#StateMap\">StateMap</a>, " );
	CLASS_Print( class_file, "<a href=\"#Trigger\">Trigger</a>, " );
	CLASS_Print( class_file, "<a href=\"#World\">World</a>" );
	CLASS_Print( class_file, "</h2>" );
#endif

	ClassDef::dump_numclasses = 0;
	ClassDef::dump_numevents = 0;

	ClassDef::sortedList.ClearObjectList();
	ClassDef::sortedClassList.ClearObjectList();

	Event::SortEventList( &ClassDef::sortedList );
	ClassDef::SortClassList( &ClassDef::sortedClassList );

	num = ClassDef::sortedClassList.NumObjects();

	// go through and process each class from smallest to greatest
	for( i = 1; i <= num; i++ )
	{
		c = ClassDef::sortedClassList.ObjectAt( i );
		DumpClass( class_file, c->classname );
	}

	if( class_file != NULL )
	{
		CLASS_Print( class_file, "<H2>\n" );
		CLASS_Print( class_file, "%d %s Classes.<BR>%d %s Events.\n", ClassDef::dump_numclasses, class_title.c_str(), ClassDef::dump_numevents, class_title.c_str() );
		CLASS_Print( class_file, "</H2>\n" );
		CLASS_Print( class_file, "</BODY>\n" );
		CLASS_Print( class_file, "</HTML>\n" );
		CLASS_DPrintf( "Dumped all classes to file %s\n", class_filename.c_str() );
		fclose( class_file );
	}

}

qboolean Class::inheritsFrom( ClassDef *c ) const
{
	return checkInheritance( c, classinfo() );
}

qboolean Class::inheritsFrom( const char * name ) const
{
	ClassDef *c;

	c = getClass( name );

	if ( c == NULL )
	{
		CLASS_Printf( "Unknown class: %s\n", name );
		return false;
	}

	return checkInheritance( c, classinfo() );
}

qboolean Class::isInheritedBy( const char * name ) const
{
	ClassDef *c;

	c = getClass( name );

	if ( c == NULL )
	{
		CLASS_DPrintf( "Unknown class: %s\n", name );
		return false;
	}

	return checkInheritance( classinfo(), c );
}

qboolean Class::isInheritedBy( ClassDef *c ) const
{
	return checkInheritance( classinfo(), c );
}

ClassDefHook::ClassDefHook()
{
	this->classdef = NULL;
}

CLASS_DECLARATION( NULL, Class, NULL )
{
	{ NULL, NULL }
};
