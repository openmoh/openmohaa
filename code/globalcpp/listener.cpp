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

// listener.cpp : Listener.

#include "glb_local.h"
#include "scriptvariable.h"
#include "linklist.h"
#include "archive.h"

#ifndef NO_SCRIPTENGINE
#include "scriptmaster.h"
#endif

#if defined( GAME_DLL )
#include "../game/player.h"
#include "animate.h"
#elif defined ( CGAME_DLL )
#elif defined ( UI_LIB )
#else
#include "../client/client.h"
#endif

con_map< Event *, EventDef > Event::eventDefList;
con_arrayset< command_t, command_t > Event::commandList;

#ifndef NO_SCRIPTENGINE

con_map< const_str, unsigned int > Event::normalCommandList;
con_map< const_str, unsigned int > Event::returnCommandList;
con_map< const_str, unsigned int > Event::getterCommandList;
con_map< const_str, unsigned int > Event::setterCommandList;

#endif

EventQueueNode Event::EventQueue;

int	DisableListenerNotify = 0;

bool Listener::EventSystemStarted = 0;

int Event::totalevents = 1;

Event EV_Listener_CancelFor
(
	"cancelFor",
	EV_DEFAULT,
	"s",
	"name",
	"Cancel for event of type name.",
	EV_NORMAL
);

Event EV_Listener_CommandDelay
(
	"commanddelay",
	EV_DEFAULT,
	"fsSSSSSS",
	"delay command arg1 arg2 arg3 arg4 arg5 arg6",
	"executes a command after the given delay.",
	EV_NORMAL
);

Event EV_Listener_Classname
(
	"classname",
	EV_DEFAULT,
	NULL,
	NULL,
	"classname variable",
	EV_GETTER
);

Event EV_Listener_SetClassname
(
	"classname",
	EV_DEFAULT,
	"s",
	"classname",
	"classname variable",
	EV_NORMAL
);

Event EV_Listener_CreateReturnThread
(
	"thread",
	EV_DEFAULT,
	"s",
	"label",
	"Creates a thread starting at label.",
	EV_RETURN
);

Event EV_Listener_CreateThread
(
	"thread",
	EV_DEFAULT,
	"s",
	"label",
	"Creates a thread starting at label.",
	EV_NORMAL
);

Event EV_Listener_ExecuteReturnScript
(
	"exec",
	EV_DEFAULT,
	"s",
	"script",
	"Executes the specified script.",
	EV_RETURN
);

Event EV_Listener_ExecuteScript
(
	"exec",
	EV_DEFAULT,
	"s",
	"script",
	"Executes the specified script.",
	EV_NORMAL
);

Event EV_Delete
(
	"delete",
	EV_DEFAULT,
	NULL,
	NULL,
	"Removes this listener immediately.",
	EV_NORMAL
);

Event EV_Remove
(
	"imediateremove",
	EV_DEFAULT,
	NULL,
	NULL,
	"Removes this listener immediately.",
	EV_NORMAL
);

Event EV_ScriptRemove
(
	"remove",
	EV_DEFAULT,
	NULL,
	NULL,
	"Removes this listener the next time events are processed.",
	EV_NORMAL
);

Event EV_Listener_EndOn
(
	"endon",
	EV_DEFAULT,
	"s",
	"name",
	"Ends the function when the specified event is triggered.",
	EV_NORMAL
);

Event EV_Listener_InheritsFrom
(
	"inheritsfrom",
	EV_DEFAULT,
	"s",
	"class",
	"Returns 1 if the class inherits from the specified class. 0 otherwise.",
	EV_RETURN
);

Event EV_Listener_IsInheritedBy
(
	"isinheritedby",
	EV_DEFAULT,
	"s",
	"class",
	"Returns 1 if the class is inherited by the specified class. 0 otherwise.",
	EV_RETURN
);

Event EV_Listener_Notify
(
	"notify",
	EV_DEFAULT,
	"s",
	"name",
	"Triggers an event. An undefined event will be automatically created by calling waittill or endon.",
	EV_NORMAL
);

Event EV_Listener_GetOwner
(
	"owner",
	EV_DEFAULT,
	NULL,
	NULL,
	"Returns the owner.",
	EV_GETTER
);

Event EV_DelayThrow
(
	"delaythrow",
	EV_DEFAULT,
	"s",
	"label",
	"Internal usage.",
	EV_NORMAL
);

Event EV_Throw
(
	"throw",
	EV_DEFAULT,
	"s",
	"label",
	"Throws to the specified label.",
	EV_NORMAL
);

Event EV_Listener_Unregister
(
	"unregister",
	EV_DEFAULT,
	"s",
	"label",
	"Unregisters the label from the event of the same name.",
	EV_NORMAL
);

Event EV_Listener_WaitCreateReturnThread
(
	"waitthread",
	EV_DEFAULT,
	"s",
	"label",
	"Creates a thread starting at label and waits until the called thread is finished.",
	EV_RETURN
);

Event EV_Listener_WaitCreateThread
(
	"waitthread",
	EV_DEFAULT,
	"s",
	"label",
	"Creates a thread starting at label and waits until the called thread is finished.",
	EV_NORMAL
);

Event EV_Listener_WaitExecuteReturnScript
(
	"waitexec",
	EV_DEFAULT,
	"s",
	"script",
	"Executes the specified script and waits until the called thread group is finished.",
	EV_RETURN
);

Event EV_Listener_WaitExecuteScript
(
	"waitexec",
	EV_DEFAULT,
	"s",
	"script",
	"Executes the specified script and waits until the called thread group is finished.",
	EV_NORMAL
);

Event EV_Listener_WaitTill
(
	"waitTill",
	EV_DEFAULT,
	"s",
	"name",
	"Wait until event of type name",
	EV_NORMAL
);

Event EV_Listener_WaitTillTimeout
(
	"waittill_timeout",
	EV_DEFAULT,
	"fs",
	"timeout_time name",
	"Wait until event of type name with a timeout time",
	EV_NORMAL
);

Event EV_Listener_WaitTillAny
(
	"waittill_any",
	EV_DEFAULT,
	"sS",
	"name1 ...",
	"Wait until any event of type name",
	EV_NORMAL
);

Event EV_Listener_WaitTillAnyTimeout
(
	"waittill_any_timeout",
	EV_DEFAULT,
	"fsS",
	"timeout_time name1 ...",
	"Wait until any event of type name with a timeout time",
	EV_NORMAL
);

eventInfo_t *lastEvent = NULL;

cvar_t	*g_showevents;
cvar_t	*g_eventlimit;
cvar_t	*g_timeevents;
cvar_t	*g_watch;
cvar_t	*g_eventstats;

template<>
int HashCode< Event * >( Event * const& key )
{
	// can't use key->eventnum because eventnum will be assigned from con_set
	return ( int )key;
}

template<>
int HashCode< command_t >( const command_t& key )
{
	return HashCode< str >( key.command );
}

void ArchiveListenerPtr
	(
	Archiver& arc,
	SafePtr< Listener > *obj
	)

{
	arc.ArchiveSafePointer( obj );
}

template<>
void ConList::Archive
	(
	Archiver& arc
	)

{
	value.Archive( arc, ArchiveListenerPtr );
}

template<>
void Entry< const_str, ConList >::Archive
	(
	Archiver& arc
	)

{
#ifndef NO_SCRIPTENGINE
	Director.ArchiveString( arc, key );
#endif
	value.Archive( arc );
}

void L_ClearEventList()
{
	EventQueueNode *node = Event::EventQueue.next, *tmp;

	while( node != &Event::EventQueue )
	{
		tmp = node->next;

		delete node->event;
		delete node;

		node = tmp;
	}

	LL_Reset( &Event::EventQueue, next, prev );

	Event_allocator.FreeAll();

#if defined( GAME_DLL )
	AnimationEvent_allocator.FreeAll();
	ConsoleEvent_allocator.FreeAll();
#endif
}

bool L_EventSystemStarted( void )
{
	return Listener::EventSystemStarted;
}

void L_InitEvents( void )
{
#if defined ( GAME_DLL )
	g_showevents	= gi.Cvar_Get( "g_showevents", "0", 0 );
	g_eventlimit	= gi.Cvar_Get( "g_eventlimit", "5000", 0 );
	g_timeevents	= gi.Cvar_Get( "g_timeevents", "0", 0 );
	g_watch			= gi.Cvar_Get( "g_watch", "0", 0 );
	g_eventstats	= gi.Cvar_Get( "g_eventstats", "0", 0 );
#endif

	Event::LoadEvents();
	ClassDef::BuildEventResponses();

	LL_Reset( &Event::EventQueue, next, prev );

	L_ClearEventList();
	Listener::EventSystemStarted = true;
}

void L_ProcessPendingEvents()
{
	EventQueueNode *node;

	while( !LL_Empty( &Event::EventQueue, next, prev ) )
	{
		Listener *obj;

		node = Event::EventQueue.next;

		assert( node );

		obj = node->GetSourceObject();

		assert( obj );

		if( node->inttime > level.inttime )
		{
			break;
		}

		// the event is removed from its list
		LL_Remove( node, next, prev );

		// ProcessEvent will dispose of this event when it is done
		obj->ProcessEvent( node->event );

		delete node;
	}
}

void L_ShutdownEvents( void )
{
	if( !Listener::EventSystemStarted ) {
		return;
	}

	L_ClearEventList();

	eventInfo_t *evi, *prev;
	for( evi = lastEvent; evi != NULL; evi = prev )
	{
		prev = evi->prev;
		free( evi );
	}

	Event::commandList.clear();
	Event::eventDefList.clear();

	Listener::EventSystemStarted = false;
}

void L_ArchiveEvents
	(
	Archiver &arc
	)

{
	EventQueueNode *event;
	int num;

	num = 0;
	for( event = Event::EventQueue.next; event != &Event::EventQueue; event = event->next )
	{
		Listener * obj;

		assert( event );

		obj = event->GetSourceObject();

		assert( obj );

#if defined ( GAME_DLL )
		if( obj->isSubclassOf( Entity ) &&
			( ( ( Entity * )obj )->flags & FL_DONTSAVE ) )
		{
			continue;
		}
#endif

		num++;
	}

	arc.ArchiveInteger( &num );
	for( event = Event::EventQueue.next; event != &Event::EventQueue; event = event->next )
	{
		Listener * obj;

		assert( event );

		obj = event->GetSourceObject();

		assert( obj );

#if defined ( GAME_DLL )
		if( obj->isSubclassOf( Entity ) &&
			( ( ( Entity * )obj )->flags & FL_DONTSAVE ) )
		{
			continue;
		}
#endif

		event->event->Archive( arc );
		arc.ArchiveInteger( &event->inttime );
		arc.ArchiveInteger( &event->flags );
		arc.ArchiveSafePointer( &event->m_sourceobject );
	}
}

void L_UnarchiveEvents
	(
	Archiver &arc
	)

{
	EventQueueNode *node;
	Event *e;
	int i, numEvents;

	// the FreeEvents list would already be allocated at this point
	// clear out any events that may exist
	L_ClearEventList();

	arc.ArchiveInteger( &numEvents );
	for( i = 0; i < numEvents; i++ )
	{
		node = new EventQueueNode();
		e = new Event();
		e->Archive( arc );

		arc.ArchiveInteger( &node->inttime );
		arc.ArchiveInteger( &node->flags );
		arc.ArchiveSafePointer( &node->m_sourceobject );

		LL_Add( &Event::EventQueue, node, next, prev );
	}
}

//===========================================================================
// EventArgDef
//===========================================================================

void EventArgDef::Setup( const char *eventName, const char *argName, const char *argType, const char *argRange )
{
	char        scratch[ 256 ];
	const char  *ptr;
	char        *tokptr;
	const char  *endptr;
	int         index;

	// set name
	name = argName;

	// set optionality
	if ( isupper( argType[ 0 ] ) )
	{
		optional = true;
	}
	else
	{
		optional = false;
	}

	// grab the ranges
	index = 0;
	memset( minRangeDefault, true, sizeof( minRangeDefault ) );
	memset( minRange, 0, sizeof( minRange ) );
	memset( maxRangeDefault, true, sizeof( maxRangeDefault ) );
	memset( maxRange, 0, sizeof( maxRange ) );

	if( argRange && argRange[ 0 ] )
	{
		ptr = argRange;
		while( 1 )
		{
			// find opening '['
			tokptr = ( char * )strchr( ptr, '[' );
			if( !tokptr )
			{
				break;
			}
			// find closing ']'
			endptr = strchr( tokptr, ']' );
			if( !endptr )
			{
				assert( 0 );
				EVENT_Printf( "Argument defintion %s, no matching ']' found for range spec in event %s.\n", name.c_str(), eventName );
				break;
			}
			// point to the next range
			ptr = endptr;
			// skip the '['
			tokptr++;
			// copy off the range spec
			// skip the ']'
			strncpy( scratch, tokptr, endptr - tokptr );
			// terminate the range
			scratch[ endptr - tokptr ] = 0;
			// see if there is one or two parameters here
			tokptr = strchr( scratch, ',' );
			if( !tokptr )
			{
				// just one parameter
				minRange[ index >> 1 ] = ( float )atof( scratch );
				minRangeDefault[ index >> 1 ] = false;
				index++;
				// skip the second parameter
				index++;
			}
			else if( tokptr == scratch )
			{
				// just second parameter
				// skip the first paremeter
				index++;
				tokptr++;
				maxRange[ index >> 1 ] = ( float )atof( scratch );
				maxRangeDefault[ index >> 1 ] = false;
				index++;
			}
			else
			{
				qboolean second;
				// one or two parameters
				// see if there is anything behind the ','
				if( strlen( scratch ) > ( tokptr - scratch + 1 ) )
					second = true;
				else
					second = false;
				// zero out the ','
				*tokptr = 0;
				minRange[ index >> 1 ] = ( float )atof( scratch );
				minRangeDefault[ index >> 1 ] = false;
				index++;
				// skip over the nul character
				tokptr++;
				if( second )
				{
					maxRange[ index >> 1 ] = ( float )atof( tokptr );
					maxRangeDefault[ index >> 1 ] = false;
				}
				index++;
			}
		}
	}

	// figure out the type of variable it is
	switch( tolower( argType[ 0 ] ) )
	{
	case 'e':
		type = IS_ENTITY;
		break;
	case 'v':
		type = IS_VECTOR;
		break;
	case 'i':
		type = IS_INTEGER;
		break;
	case 'f':
		type = IS_FLOAT;
		break;
	case 's':
		type = IS_STRING;
		break;
	case 'b':
		type = IS_BOOLEAN;
		break;
	case 'l':
		type = IS_LISTENER;
		break;
	}
}

void EV_Print( FILE *stream, const char *format, ... )
{
	char buffer[ 1000 ];
	va_list va;

	va_start( va, format );

	vsprintf( buffer, format, va );

	if( stream )
	{
		fprintf( stream, buffer );
	}
	else
	{
		glbs.DPrintf( buffer );
	}

	va_end( va );
}

void EventArgDef::PrintRange( FILE *event_file )
{
	qboolean integer;
	qboolean single;
	int numRanges;
	int i;
	
	single = false;
	integer = true;
	numRanges = 1;
	switch( type )
	{
	case IS_VECTOR:
		integer = false;
		numRanges = 3;
		break;
	case IS_FLOAT:
		integer = false;
		break;
	case IS_STRING:
		single = true;
		break;
	}
	for( i = 0; i < numRanges; i++ )
	{
		if ( single )
		{
			if ( !minRangeDefault[ i ] )
            {
				if ( integer )
				{
					EV_Print( event_file, "<%d>", ( int )minRange[ i ] );
				}
				else
				{
					EV_Print( event_file, "<%.2f>", minRange[ i ] );
				}
            }
		}
		else
		{
			// both non-default
			if ( !minRangeDefault[ i ] && !maxRangeDefault[ i ] )
            {
				if ( integer )
				{
					EV_Print( event_file, "<%d...%d>", ( int )minRange[ i ], ( int )maxRange[ i ] );
				}
				else
				{
					EV_Print( event_file, "<%.2f...%.2f>", minRange[ i ], maxRange[ i ] );
				}
            }
			// max default
			else if ( !minRangeDefault[ i ] && maxRangeDefault[ i ] )
            {
				if ( integer )
				{
					EV_Print( event_file, "<%d...max_integer>", ( int )minRange[ i ] );
				}
				else
				{
					EV_Print( event_file, "<%.2f...max_float>", minRange[ i ] );
				}
            }
			// min default
			else if ( minRangeDefault[ i ] && !maxRangeDefault[ i ] )
            {
				if ( integer )
				{
					EV_Print( event_file, "<min_integer...%d>", ( int )maxRange[ i ] );
				}
				else
				{
					EV_Print( event_file, "<min_float...%.2f>", maxRange[ i ] );
				}
            }
		}
	}
}

void EventArgDef::PrintArgument( FILE *event_file )
{
	if( optional )
	{
		EV_Print( event_file, "[ " );
	}

	switch( type )
	{
	case IS_ENTITY:
		EV_Print( event_file, "Entity " );
		break;
	case IS_VECTOR:
		EV_Print( event_file, "Vector " );
		break;
	case IS_INTEGER:
		EV_Print( event_file, "Integer " );
		break;
	case IS_FLOAT:
		EV_Print( event_file, "Float " );
		break;
	case IS_STRING:
		EV_Print( event_file, "String " );
		break;
	case IS_BOOLEAN:
		EV_Print( event_file, "Boolean " );
		break;
	case IS_LISTENER:
		EV_Print( event_file, "Listener " );
		break;
	}
	EV_Print( event_file, "%s", name.c_str() );

	PrintRange( event_file );
	
	if ( optional )
	{
		EV_Print( event_file, " ]" );
	}
}

void EventDef::Error( const char *format, ... )
{
	char buffer[ 1000 ];
	va_list va;

	va_start( va, format );
	vsprintf( buffer, format, va );
	va_end( va );

	EVENT_Printf( "^~^~^ Game: '%s' : %s\n", command.c_str(), buffer );
}

void EventDef::PrintDocumentation( FILE *event_file, bool html )
{
	int i;
	int p;
	str text;
	const char *name = command.c_str();
	
	if ( !html )
	{
		text = "   ";
		p = 0;
		
		if ( flags & EV_CONSOLE )
		{
			text[ p++ ] = '*';
		}
		if ( flags & EV_CHEAT )
		{
			text[ p++ ] = 'C';
		}
		if ( flags & EV_CACHE )
		{
			text[ p++ ] = '%';
		}
	}
	
	if ( html )
	{
		EV_Print( event_file, "\n<P><tt><B>%s</B>", name );
	}
	else
	{
		if ( text[ 0 ] != ' ' )
		{
			EV_Print( event_file, "%s %s", text.c_str(), name );
		}
		else
		{
			EV_Print( event_file, "%s %s", text.c_str(), name );
		}
	}

	SetupDocumentation();

	if ( definition )
	{
		if ( html )
		{
			EV_Print( event_file, "( <i>" );
		}
		else
		{
			EV_Print( event_file, "( " );
		}

		for( i = 1; i <= definition->NumObjects(); i++ )
		{
			definition->ObjectAt( i ).PrintArgument( event_file );

			if ( i < definition->NumObjects() )
            {
				EV_Print( event_file, ", " );
            }
		}

		if ( html )
		{
			EV_Print( event_file, " </i>)</tt><BR>\n" );
		}
		else
		{
			EV_Print( event_file, " )\n" );
		}

		DeleteDocumentation();
	}
	else
	{
		if ( html )
		{
			EV_Print( event_file, "</tt><BR>\n" );
		}
		else
		{
			EV_Print( event_file, "\n" );
		}
	}

	if ( documentation )
	{
		char new_doc[ 1024 ];
		int old_index;
		int new_index = 0;
		
		for( old_index = 0; old_index < strlen( documentation ); old_index++ )
		{
			if( documentation[ old_index ] == '\n' )
			{
				if ( html )
				{
					new_doc[ new_index ]		= '<';
					new_doc[ new_index + 1 ]	= 'B';
					new_doc[ new_index + 2 ]	= 'R';
					new_doc[ new_index + 3 ]	= '>';
					new_doc[ new_index + 4 ]	= '\n';
					new_index += 5;
				}
				else
				{
					new_doc[ new_index ]		= '\n';
					new_doc[ new_index + 1 ]	= '\t';
					new_doc[ new_index + 2 ]	= '\t';
					new_index += 3;
				}
			}
			else
			{
				new_doc[ new_index ] = documentation[ old_index ];
				new_index++;
			}
			
		}
		
		new_doc[ new_index ] = 0;
		
		if ( html )
		{
			EV_Print( event_file, "<ul>%s</ul>\n", new_doc );
		}
		else
		{
			EV_Print( event_file, "\t\t- %s\n", new_doc );
		}
	}
}

void EventDef::PrintEventDocumentation( FILE *event_file, bool html )
{
	if( flags & EV_CODEONLY ) {
		return;
	}

	// purposely suppressed
	if( command[ 0 ] == '_' )
	{
		return;
	}

	PrintDocumentation( event_file, html );
}

void EventDef::DeleteDocumentation( void )
{
	if( formatspec )
	{
		if( argument_names )
		{
			definition->FreeObjectList();

			delete definition;
			definition = NULL;
		}
	}
}

void EventDef::SetupDocumentation( void )
{
	const char *name = command.c_str();

	// setup documentation
	if ( formatspec )
	{
		if ( argument_names )
		{
			char        argumentNames[ 256 ];
			str         argSpec;
			str         rangeSpec;
			str         argName;
			EventArgDef argDef;
			const char  *namePtr;
			const char  *specPtr;
			size_t       specLength;
			int          index;
			Container<str> argNames;
			
			specLength = strlen( formatspec );
			specPtr = formatspec;
			//
			// store off all the names
			//
			strcpy( argumentNames, argument_names );
			namePtr = strtok( argumentNames, " " );
			while ( namePtr != NULL )
            {
				argNames.AddObject( str( namePtr ) );
				namePtr = strtok( NULL, " " );
            }
			
			index = 0;
			
			//
			// create the definition container
			//
			definition = new Container<EventArgDef>;
			definition->Resize( argNames.NumObjects() );
			
			// go throught he formatspec
			while( specLength )
            {
				// clear the rangeSpec
				rangeSpec = "";
				// get the argSpec
				argSpec = "";
				argSpec += *specPtr;
				specPtr++;
				specLength--;
				// see if there is a range specified
				while ( *specPtr == '[' )
				{
					// add in all the characters until NULL or ']'
					while( specLength && ( *specPtr != ']' ) )
					{
						rangeSpec += *specPtr;
						specPtr++;
						specLength--;
					}
					if ( specLength && ( *specPtr == ']' ) )
					{
						rangeSpec += *specPtr;
						specPtr++;
						specLength--;
					}
				}
				if ( index < argNames.NumObjects() )
				{
					argName = argNames.ObjectAt( index + 1 );
					argDef.Setup( name, argName, argSpec, rangeSpec );
					definition->AddObject( argDef );
				}
				else
				{
					assert( 0 );
					Error( "More format specifiers than argument names for event %s\n", name );
				}
				index++;
            }
			if ( index < argNames.NumObjects() )
            {
				assert( 0 );
				Error( "More argument names than format specifiers for event %s\n", name );
			}
		}
	}
}

//====================================
// Event
//====================================

MEM_BlockAlloc< Event, MEM_BLOCKSIZE > Event_allocator;

CLASS_DECLARATION( Class, Event, NULL )
{
	{ NULL, NULL }
};

#ifndef _DEBUG_MEM

/*
=======================
new Event
=======================
*/
void *Event::operator new( size_t size )
{
	return Event_allocator.Alloc();
}

/*
=======================
delete ptr
=======================
*/
void Event::operator delete( void *ptr )
{
	Event_allocator.Free( ptr );
}

#endif

/*
=======================
FindEventNum
=======================
*/
unsigned int Event::FindEventNum( str s )
{
	command_t cmd;

	cmd.command = s;
	cmd.flags = 0;
	cmd.type = -1;

	return commandList.findKeyIndex( cmd );
}

/*
=======================
NumEventCommands

Number of total events
=======================
*/
int Event::NumEventCommands()
{
	return totalevents;
}

/*
=======================
ListCommands

List event commands
=======================
*/
void Event::ListCommands
	(
	const char *mask
	)

{
	command_t *command;
	int eventnum;
	int num;
	int i;
	int n;
	size_t l;
	int p;
	int hidden;
	str text;
	Container< int > *sortedList;

	if ( !commandList.size() )
	{
		EVENT_DPrintf( "No events.\n" );
		return;
	}

	sortedList = &ClassDef::sortedList;
	SortEventList( sortedList );

	l = 0;
	if ( mask )
	{
		l = strlen( mask );
	}

	hidden = 0;
	num = 0;
	n = sortedList->NumObjects();
	for( i = 1; i <= n; i++ )
	{
		eventnum = sortedList->ObjectAt( i );
		command = &commandList[ eventnum ];

		if ( command->flags & EV_CODEONLY )
		{
			hidden++;
			continue;
		}

		if ( mask && Q_stricmpn( command->command.c_str(), mask, l ) )
		{
			continue;
		}

		num++;

		text = "   ";
		p = 0;
		if( command->flags & EV_CONSOLE )
		{
			text[ p++ ] = '*';
		}
		if( command->flags & EV_CHEAT )
		{
			text[ p++ ] = 'C';
		}
		if( command->flags & EV_CACHE )
		{
			text[ p++ ] = '%';
		}

		EVENT_Printf( "%4d : %s%s\n", eventnum, text.c_str(), command->command.c_str() );
	}

	EVENT_Printf( "\n* = console command.\nC = cheat command.\n% = cache command.\n\n"
		"Printed %d of %d total commands.\n", num, n - hidden );

	if ( developer->integer && hidden )
	{
		EVENT_Printf( "Suppressed %d commands.\n", hidden );
	}
}

/*
=======================
ListDocumentation

List event documentation
=======================
*/
void Event::ListDocumentation
	(
	const char *mask,
	qboolean print_to_disk
	)

{
	int num;
	int n;
	size_t l;
	int flags;
	int hidden;
	str name;
	str text;
	FILE *event_file = NULL;
	str event_filename;
	con_map_enum< Event *, EventDef > en = eventDefList;
	EventDef *def;

	if ( print_to_disk )
	{
		if ( !mask || !mask[0] )
			event_filename = EVENT_FILENAME;
		else
			event_filename = str ( mask ) + ".txt";

		event_file = fopen( event_filename.c_str(), "w" );

		if ( event_file == NULL )
			return;
	}

	l = 0;
	if ( mask )
	{
		l = strlen( mask );
	}


	EV_Print( event_file, "\nCommand Documentation\n" );
	EV_Print( event_file, "=====================\n" );

	hidden = 0;
	num = 0;
	n = 0;
	for( def = en.NextValue(); def != NULL; def = en.NextValue() )
	{
		flags = def->flags;
		name = def->command;
		n++;

		if( flags & EV_CODEONLY )
		{
			hidden++;
			continue;
		}

		if( mask && Q_stricmpn( name, mask, l ) )
		{
			continue;
		}

		num++;

		def->PrintDocumentation( event_file, qfalse );
	}

	EV_Print( event_file, "\n* = console command.\nC = cheat command.\n% = cache command.\n\n"
		"Printed %d of %d total commands.\n", num, n - hidden );

	if ( developer->integer && hidden )
	{
		EV_Print( event_file, "Suppressed %d commands.\n", hidden );
	}

	if ( event_file != NULL )
	{
		EVENT_Printf( "Printed event info to file %s\n", event_filename.c_str() );
		fclose( event_file );
	}
}

/*
=======================
PendingEvents

List pending events
=======================
*/
void Event::PendingEvents
	(
	const char *mask
	)

{
	EventQueueNode *event;
	size_t l;
	int num;

	l = 0;
	if ( mask )
	{
		l = strlen( mask );
	}

	num = 0;
	event = EventQueue.next;
	while( event != &EventQueue )
	{
		assert( event );
		assert( event->m_sourceobject );

		if ( !mask || !Q_stricmpn( event->event->getName().c_str(), mask, l ) )
		{
			num++;
			//Event::PrintEvent( event );
		}

		event = event->next;
	}
	EVENT_Printf( "%d pending events as of %.2f\n", num, EVENT_time );
}

#ifndef NO_SCRIPTENGINE

/*
=======================
FindNormalEventNum
=======================
*/
unsigned int Event::FindNormalEventNum( const_str s )
{
	unsigned int *eventnum = normalCommandList.find( s );
	if( eventnum )
	{
		return *eventnum;
	}
	else
	{
		return 0;
	}
}

/*
=======================
FindReturnEventNum

Finds an event that return a value
=======================
*/
unsigned int Event::FindReturnEventNum( const_str s )
{
	unsigned int *eventnum = returnCommandList.find( s );
	if( eventnum )
	{
		return *eventnum;
	}
	else
	{
		return 0;
	}
}

/*
=======================
FindSetterEventNum

Finds an event that act as a write-variable
=======================
*/
unsigned int Event::FindSetterEventNum( const_str s )
{
	unsigned int *eventnum = setterCommandList.find( s );
	if( eventnum )
	{
		return *eventnum;
	}
	else
	{
		return 0;
	}
}

/*
=======================
FindGetterEventNum

Finds an event that act as a read-variable
=======================
*/
unsigned int Event::FindGetterEventNum( const_str s )
{
	unsigned int *eventnum = getterCommandList.find( s );
	if( eventnum )
	{
		return *eventnum;
	}
	else
	{
		return 0;
	}
}

/*
=======================
FindNormalEventNum
=======================
*/
unsigned int Event::FindNormalEventNum( str s )
{
	s.tolower();
	return FindNormalEventNum( Director.AddString( s ) );
}

/*
=======================
FindReturnEventNum
=======================
*/
unsigned int Event::FindReturnEventNum( str s )
{
	s.tolower();
	return FindReturnEventNum( Director.AddString( s ) );
}

/*
=======================
FindSetterEventNum
=======================
*/
unsigned int Event::FindSetterEventNum( str s )
{
	s.tolower();
	return FindSetterEventNum( Director.AddString( s ) );
}

/*
=======================
FindGetterEventNum
=======================
*/
unsigned int Event::FindGetterEventNum( str s )
{
	s.tolower();
	return FindGetterEventNum( Director.AddString( s ) );
}

/*
=======================
GetEventWithFlags

Returns an event that match the specified flags and the specified type
=======================
*/
int Event::GetEventWithFlags( str name, int flags, uchar type )
{
	unsigned int *index;
	con_map< const_str, unsigned int > *cmdList;

	if( type == EV_NORMAL )
	{
		cmdList = &normalCommandList;
	}
	else if( type == EV_RETURN )
	{
		cmdList = &returnCommandList;
	}
	else if( type == EV_GETTER )
	{
		cmdList = &getterCommandList;
	}
	else if( type == EV_SETTER )
	{
		cmdList = &setterCommandList;
	}
	else
	{
		return 0;
	}

	name.tolower();

	index = cmdList->find( Director.GetString( name ) );

	if( !index || !( GetEventFlags( *index ) & flags ) )
	{
		return 0;
	}
	else
	{
		return *index;
	}
}

/*
=======================
GetEvent
=======================
*/
int Event::GetEvent( str name, uchar type )
{
	return GetEventWithFlags( name, EV_DEFAULT, type );
}

#else

int Event::GetEvent( str name, uchar type )
{
	return FindEventNum( name );
}

#endif

static str str_null;

/*
=======================
GetEventInfo
=======================
*/
command_t *Event::GetEventInfo( int eventnum )
{
	return &commandList[ eventnum ];
}

/*
=======================
GetEventFlags

Returns the specified event flags
=======================
*/
int Event::GetEventFlags( int eventnum )
{
	command_t *cmd = &commandList[ eventnum ];

	if( cmd )
	{
		return cmd->flags;
	}
	else
	{
		return 0;
	}

	return 0;
}

/*
=======================
GetEventName

Returns the specified event name
=======================
*/
str& Event::GetEventName( int eventnum )
{
	command_t *cmd;

	if( eventnum <= 0 ) {
		return str_null;
	}

	cmd = &commandList[ eventnum ];

	if( cmd )
	{
		return cmd->command;
	}
	else
	{
		return str_null;
	}

	return str_null;
}

/*
=======================
compareEvents
=======================
*/
int Event::compareEvents( const void *arg1, const void *arg2 )
{
	int num1 = *( int * )arg1;
	int num2 = *( int * )arg2;
	command_t *cmd1 = &commandList[ num1 ];
	command_t *cmd2 = &commandList[ num2 ];

	return Q_stricmp( cmd1->command.c_str(), cmd2->command.c_str() );
}

/*
=======================
SortEventList

Sort events in alphabetical order
=======================
*/
void Event::SortEventList( Container< int > *sortedList )
{
	int i;
	command_t *cmd;

	sortedList->Resize( commandList.size() );

	for( i = 1; i <= commandList.size(); i++ )
	{
		cmd = &commandList[ i ];

		if( cmd != NULL )
		{
			sortedList->AddObject( i );
		}
	}

	qsort( ( void * )sortedList->AddressOfObjectAt( 1 ),
		( size_t )sortedList->NumObjects(),
		sizeof( int ), compareEvents );
}

/*
=======================
LoadEvents

Event loader routine
=======================
*/
void Event::LoadEvents()
{
	eventInfo_t *evi = lastEvent;
	str lower;
	EventDef *cmd;
	int index;
	command_t c;
	command_t *i;
	Entry< command_t, command_t > *entry;
	con_set_enum< command_t, command_t > en;

	while( evi )
	{
		cmd = &eventDefList[ evi->ev ];

		cmd->command = evi->command;
		cmd->flags = ( ( evi->flags == EV_DEFAULT ) - 1 ) & evi->flags;
		cmd->formatspec = evi->formatspec;
		cmd->argument_names = evi->argument_names;
		cmd->documentation = evi->documentation;
		cmd->type = evi->type;

#ifdef _DEBUG
		evi->ev->name = evi->command;
#endif

		lower = str( evi->command );
		lower.tolower();

		en = commandList;

		index = 0;

		for( entry = en.NextElement(); entry != NULL; entry = en.NextElement() )
		{
			i = &entry->value;

			if( i->command == lower && i->type == cmd->type )
			{
				index = entry->index;
				break;
			}
		}

		if( !index )
		{
			c.command = lower;
			c.flags = evi->flags;
			c.type = cmd->type;

			index = commandList.addKeyIndex( c );
		}

		evi->ev->eventnum = index;

		evi = ( eventInfo_t * )evi->prev;

		totalevents++;
	}
}

/*
=======================
Archive
=======================
*/
void Event::Archive( Archiver &arc )
{
	if( arc.Loading() ) {
		fromScript = false;
	}

	Class::Archive( arc );

	arc.ArchiveUnsignedShort( &eventnum );
	arc.ArchiveUnsignedShort( &dataSize );

	if( arc.Loading() )
	{
		data = new ScriptVariable[ dataSize + 1 ];
	}

	for( int i = dataSize; i > 0; i-- )
	{
		data[ i ].ArchiveInternal( arc );
	}
}

/*
=======================
Event

Creates an event command
=======================
*/
Event::Event
	(
		const char *command,
		int flags,
		const char *formatspec,
		const char *argument_names,
		const char *documentation,
		uchar type
	)

{
	eventInfo_t *evi = ( eventInfo_t * )malloc( sizeof( eventInfo_t ) );

	evi->ev = this;
	evi->command = command;
	evi->flags = flags;
	evi->formatspec = formatspec;
	evi->argument_names = argument_names;
	evi->documentation = documentation;
	evi->type = type;

	evi->prev = lastEvent;

	lastEvent = evi;

	fromScript = false;
	dataSize = 0;
	data = NULL;
	eventnum = 0;
}

/*
=======================
~Event
=======================
*/
Event::~Event()
{
	Clear();
}

/*
=======================
Event
=======================
*/
Event::Event()
{
	fromScript = false;
	eventnum = 0;
	data = NULL;
	dataSize = 0;
}

/*
=======================
ErrorInternal
=======================
*/
void Event::ErrorInternal( Listener *l, str text )
{
	str classname;
	str eventname;

	glbs.DPrintf( "^~^~^ Game" );

	classname = l->getClassname();
	eventname = getName();

	glbs.DPrintf( " (Event: '%s', Object: '%s') : %s\n", eventname.c_str(), classname.c_str(), text.c_str() );
}

/*
=======================
Event
=======================
*/
Event::Event( const Event *ev )
{
	fromScript = ev->fromScript;
	eventnum = ev->eventnum;
	dataSize = ev->dataSize;

	if( dataSize )
	{
		data = new ScriptVariable[ dataSize ];

		for( int i = 0; i < dataSize; i++ )
		{
			data[ i ] = ev->data[ i ];
		}
	}
	else
	{
		data = NULL;
	}

#ifdef _DEBUG
	name = ev->name;
#endif
}

/*
=======================
Event
=======================
*/
Event::Event( const Event &ev )
{
	fromScript = ev.fromScript;
	eventnum = ev.eventnum;
	dataSize = ev.dataSize;

	if( dataSize )
	{
		data = new ScriptVariable[ dataSize ];

		for( int i = 0; i < dataSize; i++ )
		{
			data[ i ] = ev.data[ i ];
		}
	}
	else
	{
		data = NULL;
	}

#ifdef _DEBUG
	name = ev.name;
#endif
}

/*
=======================
Event

Initializes the event with the specified index
=======================
*/
Event::Event( int index )
{
	fromScript = false;
	eventnum = index;
	data = NULL;
	dataSize = 0;

#ifdef _DEBUG
	name = GetEventName( index );
#endif
}

/*
=======================
Event

Initializes the event with the specified command
=======================
*/
Event::Event( str command )
{
	command_t c;

	c.command = command.c_str();
	c.flags = 0;
	c.type = EV_NORMAL;

	c.command.tolower();

	eventnum = commandList.findKeyIndex( c );
	if( !eventnum )
	{
		glbs.DPrintf( "^~^~^ Event '%s' does not exist.\n", command.c_str() );
	}

	fromScript = qfalse;
	dataSize = 0;
	data = NULL;

#ifdef _DEBUG
	name = command;
#endif
}

#ifndef NO_SCRIPTENGINE

/*
=======================
Event

Initializes the event with the specified command and type
=======================
*/
Event::Event( str command, uchar type )
{
	unsigned int *index;

	if( type == EV_NORMAL )
	{
		index = normalCommandList.find( Director.AddString( command ) );
	}
	else if( type == EV_RETURN )
	{
		index = returnCommandList.find( Director.AddString( command ) );
	}
	else if( type == EV_GETTER )
	{
		index = getterCommandList.find( Director.AddString( command ) );
	}
	else if( type == EV_SETTER )
	{
		index = setterCommandList.find( Director.AddString( command ) );
	}
	else
	{
		index = NULL;
	}

	if( index )
	{
		eventnum = *index;
	}
	else
	{
		eventnum = 0;
	}

	if( !eventnum )
	{
		glbs.DPrintf( "^~^~^ Event '%s' does not exist.\n", command.c_str() );
	}

	fromScript = qfalse;
	dataSize = 0;
	data = NULL;

#ifdef _DEBUG
	name = command;
#endif
}

#endif

/*
=======================
AddContainer
=======================
*/
void Event::AddContainer( Container< SafePtr< Listener > > *container )
{
	ScriptVariable& variable = GetValue();
	variable.setContainerValue( container );
}

/*
=======================
AddEntity
=======================
*/
void Event::AddEntity( Entity *ent )
{
	ScriptVariable& variable = GetValue();
	variable.setListenerValue( ( Listener * )ent );
}

/*
=======================
AddFloat
=======================
*/
void Event::AddFloat( float number )
{
	ScriptVariable& variable = GetValue();
	variable.setFloatValue( number );
}

/*
=======================
AddInteger
=======================
*/
void Event::AddInteger( int number )
{
	ScriptVariable& variable = GetValue();
	variable.setIntValue( number );
}

/*
=======================
AddListener
=======================
*/
void Event::AddListener( Listener * listener )
{
	ScriptVariable& variable = GetValue();
	variable.setListenerValue( listener );
}

/*
=======================
AddNil
=======================
*/
void Event::AddNil( void )
{
	ScriptVariable& variable = GetValue();
	variable.Clear();
}

#ifndef NO_SCRIPTENGINE

/*
=======================
AddConstString
=======================
*/
void Event::AddConstString( const_str string )
{
	ScriptVariable& variable = GetValue();
	variable.setConstStringValue( string );
}

#endif

/*
=======================
AddString
=======================
*/
void Event::AddString( str string )
{
	ScriptVariable& variable = GetValue();
	variable.setStringValue( string );
}

/*
=======================
AddToken
=======================
*/
void Event::AddToken( str token )
{
	ScriptVariable& variable = GetValue();
	variable.setStringValue( token );
}

/*
=======================
AddTokens
=======================
*/
void Event::AddTokens( int argc, const char **argv )
{
	for( int i = 0; i < argc; i++ )
	{
		AddToken( argv[ i ] );
	}
}

/*
=======================
AddValue
=======================
*/
void Event::AddValue( const ScriptVariable &value )
{
	ScriptVariable& variable = GetValue();
	variable = value;
}

/*
=======================
AddVector
=======================
*/
void Event::AddVector( const Vector& vector )
{
	ScriptVariable& variable = GetValue();
	variable.setVectorValue( vector );
}

/*
=======================
Clear
=======================
*/
void Event::Clear( void )
{
	if( data )
	{
		delete[] data;

		data = NULL;
		dataSize = 0;
	}
}

/*
=======================
CheckPos
=======================
*/
void Event::CheckPos( int pos )
{
	if( pos > NumArgs() ) {
		ScriptError( "Index %d out of range.", pos );
	}
}

/*
=======================
GetBoolean
=======================
*/
bool Event::GetBoolean( int pos )
{
	ScriptVariable& variable = GetValue( pos );

	return variable.booleanNumericValue();
}

#ifndef NO_SCRIPTENGINE

/*
=======================
GetConstString
=======================
*/
int Event::GetConstString( int pos )
{
	ScriptVariable& variable = GetValue( pos );

	return variable.constStringValue();
}

#endif

/*
=======================
GetEntity
=======================
*/
Entity *Event::GetEntity( int pos )
{
	ScriptVariable& variable = GetValue( pos );

	return variable.entityValue();
}

/*
=======================
GetFloat
=======================
*/
float Event::GetFloat( int pos )
{
	ScriptVariable& variable = GetValue( pos );

	return variable.floatValue();
}

/*
=======================
GetInteger
=======================
*/
int Event::GetInteger( int pos )
{
	ScriptVariable& variable = GetValue( pos );

	return variable.intValue();
}

/*
=======================
GetListener
=======================
*/
Listener * Event::GetListener( int pos )
{
	ScriptVariable& variable = GetValue( pos );

	return variable.listenerValue();
}


#ifndef NO_SCRIPTENGINE

/*
=======================
GetSimpleEntity
=======================
*/
SimpleEntity * Event::GetSimpleEntity( int pos )
{
	ScriptVariable& variable = GetValue( pos );

	return variable.simpleEntityValue();
}

#endif

/*
=======================
GetString
=======================
*/
str Event::GetString( int pos )
{
	ScriptVariable& variable = GetValue( pos );
	return variable.stringValue();
}

/*
=======================
GetToken
=======================
*/
str Event::GetToken( int pos )
{
	ScriptVariable& variable = GetValue( pos );
	return variable.stringValue();
}

/*
=======================
GetValue
=======================
*/
ScriptVariable& Event::GetValue( int pos )
{
	if( pos < 0 )
	{
		pos = NumArgs() + pos + 1;
	}

	CheckPos( pos );

	return data[ pos - 1 ];
}

static ScriptVariable m_null;

/*
=======================
GetValue
=======================
*/
ScriptVariable& Event::GetValue( void )
{
	ScriptVariable *tmp = data;

	data = new ScriptVariable[ dataSize + 1 ];

	if( tmp != NULL )
	{
		for( int i = 0; i < dataSize; i++ ) {
			data[ i ] = tmp[ i ];
		}

		delete[] tmp;
	}

	dataSize++;

	return data[ dataSize - 1 ];
}

/*
=======================
GetVector
=======================
*/
Vector Event::GetVector( int pos )
{
	ScriptVariable& variable = GetValue( pos );

	return variable.vectorValue();
}

#if defined ( GAME_DLL )

/*
=======================
GetPathNode
=======================
*/
PathNode *Event::GetPathNode( int pos )
{
	ScriptVariable& variable = GetValue( pos );

	return variable.pathNodeValue();
}

/*
=======================
GetWaypoint
=======================
*/
Waypoint *Event::GetWaypoint( int pos )
{
	ScriptVariable& variable = GetValue( pos );

	return variable.waypointValue();
}

#endif

/*
=======================
IsEntityAt
=======================
*/
qboolean Event::IsEntityAt( int pos )
{
	CheckPos( pos );

	return data[ pos- 1 ].IsEntity();
}

/*
=======================
IsListenerAt
=======================
*/
qboolean Event::IsListenerAt( int pos )
{
	CheckPos( pos );

	return data[ pos - 1 ].IsListener();
}

/*
=======================
IsNilAt
=======================
*/
qboolean Event::IsNilAt( int pos )
{
	CheckPos( pos );

	return data[ pos - 1 ].GetType() == VARIABLE_NONE;
}

/*
=======================
IsNumericAt
=======================
*/
qboolean Event::IsNumericAt( int pos )
{
	CheckPos( pos );

	return data[ pos - 1 ].IsNumeric();
}

#ifndef NO_SCRIPTENGINE

/*
=======================
IsSimpleEntityAt
=======================
*/
qboolean Event::IsSimpleEntityAt( int pos )
{
	CheckPos( pos );

	return data[ pos - 1 ].IsSimpleEntity();
}

#endif

/*
=======================
IsStringAt
=======================
*/
qboolean Event::IsStringAt( int pos )
{
	CheckPos( pos );

	return data[ pos - 1 ].IsString();
}

/*
=======================
IsVectorAt
=======================
*/
qboolean Event::IsVectorAt( int pos )
{
	CheckPos( pos );

	return data[ pos - 1 ].IsVector();
}

/*
=======================
IsFromScript
=======================
*/
qboolean Event::IsFromScript()
{
	return fromScript;
}

/*
=======================
GetFormat
=======================
*/
const char *Event::GetFormat()
{
	eventInfo_t *evi;

	for( evi = lastEvent; evi != NULL; evi = ( eventInfo_t * )evi->prev )
	{
		if( evi->ev->eventnum == eventnum )
		{
			return evi->formatspec;
		}
	}

	return NULL;
}

/*
=======================
NumArgs
=======================
*/
int Event::NumArgs()
{
	return dataSize;
}

/*
=======================
getInfo
=======================
*/
EventDef *Event::getInfo()
{
	return &eventDefList[ this ];
}

/*
=======================
getName
=======================
*/
str& Event::getName()
{
	return GetEventName( eventnum );
}

#if defined ( GAME_DLL )

//===============================
// ConsoleEvent
//===============================


MEM_BlockAlloc< ConsoleEvent, MEM_BLOCKSIZE > ConsoleEvent_allocator;

CLASS_DECLARATION( Event, ConsoleEvent, NULL )
{
	{ NULL, NULL }
};

/*
=======================
new ConsoleEvent
=======================
*/
void *ConsoleEvent::operator new( size_t size )
{
	return ConsoleEvent_allocator.Alloc();
}

/*
=======================
delete ptr
=======================
*/
void ConsoleEvent::operator delete( void *ptr )
{
	ConsoleEvent_allocator.Free( ptr );
}

/*
=======================
ConsoleEvent
=======================
*/
ConsoleEvent::ConsoleEvent( void )
{
	m_consoleedict = NULL;
}

/*
=======================
SetConsoleEdict
=======================
*/
void ConsoleEvent::SetConsoleEdict( gentity_t *edict )
{
	m_consoleedict = edict;
}

/*
=======================
GetConsoleEdict
=======================
*/
gentity_t *ConsoleEvent::GetConsoleEdict( void )
{
	if( m_consoleedict )
		return m_consoleedict;

	return g_entities;
}

/*
=======================
ErrorInternal
=======================
*/
void ConsoleEvent::ErrorInternal( Listener *l, str text )
{
	gentity_t *edict = GetConsoleEdict();
	str eventname = getName();

	gi.DPrintf( "^~^~^ Game ( Event '%s', Client '%s' ) : %s\n",
		eventname.c_str(),
		edict->client ? edict->client->pers.netname : "",
		text.c_str() );

	gi.SendServerCommand( GetConsoleEdict() - g_entities,
		"print \"Console: '%s' : %s\n\"",
		eventname.c_str(),
		text.c_str() );
}

#endif

//====================================
// Listener
//====================================

qboolean ListenerDelete = false;

/*
=======================
Listener
=======================
*/
Listener::Listener()
{
#ifndef NO_SCRIPTENGINE

	m_EndList = NULL;

	m_NotifyList = NULL;
	m_WaitForList = NULL;

	vars = NULL;

#endif
}

/*
=======================
~Listener
=======================
*/
Listener::~Listener()
{
	CancelPendingEvents();

#ifndef NO_SCRIPTENGINE

	Unregister( "delete" );
	Unregister( "remove" );

	UnregisterAll();
	CancelWaitingAll();

	if( vars ) {
		delete vars;
	}

#endif
}

#define L_ARCHIVE_NOTIFYLIST		1
#define L_ARCHIVE_WAITFORLIST		2
#define L_ARCHIVE_VARLIST			4
#define L_ARCHIVE_ENDLIST			8

/*
=======================
Archive
=======================
*/
void Listener::Archive( Archiver &arc )
{
	Class::Archive( arc );

#ifndef NO_SCRIPTENGINE

	byte flag = 0;

	if( !arc.Loading() )
	{
		if( m_NotifyList )
			flag |= L_ARCHIVE_NOTIFYLIST;
		if( m_WaitForList )
			flag |= L_ARCHIVE_WAITFORLIST;
		if( vars )
			flag |= L_ARCHIVE_VARLIST;
		if( m_EndList )
			flag |= L_ARCHIVE_ENDLIST;
	}

	arc.ArchiveByte( &flag );

	// archive the notify list
	if( flag & L_ARCHIVE_NOTIFYLIST )
	{
		if( arc.Loading() )
		{
			m_NotifyList = new con_set < const_str, ConList > ;
		}

		m_NotifyList->Archive( arc );
	}

	// archive the waiting thread list
	if( flag & L_ARCHIVE_WAITFORLIST )
	{
		if( arc.Loading() )
		{
			m_WaitForList = new con_set < const_str, ConList > ;
		}

		m_WaitForList->Archive( arc );
	}

	// archive the variable list
	if( flag & L_ARCHIVE_VARLIST )
	{
		if( arc.Loading() )
		{
			vars = new ScriptVariableList;
		}

		vars->Archive( arc );
	}

	// archive the end on event list
	if( flag & L_ARCHIVE_ENDLIST )
	{
		if( arc.Loading() )
		{
			m_EndList = new con_set < const_str, ConList >;
		}

		m_EndList->Archive( arc );
	}

#endif
}

/*
=======================
CancelEventsOfType
=======================
*/
void Listener::CancelEventsOfType( Event *ev )
{
	EventQueueNode *node;
	EventQueueNode *next;
	int eventnum;

	node = Event::EventQueue.next;

	eventnum = ev->eventnum;
	while( node != &Event::EventQueue )
	{
		next = node->next;
		if( ( node->GetSourceObject() == this ) && ( node->event->eventnum == eventnum ) )
		{
			LL_Remove( node, next, prev );
			delete node;
		}
		node = next;
	}
}

/*
=======================
CancelEventsOfType
=======================
*/
void Listener::CancelEventsOfType( Event &ev )
{
	this->CancelEventsOfType( &ev );
}

/*
=======================
CancelFlaggedEvents
=======================
*/
void Listener::CancelFlaggedEvents( int flags )
{
	EventQueueNode *node;
	EventQueueNode *next;

	node = Event::EventQueue.next;

	while( node != &Event::EventQueue )
	{
		next = node->next;
		if( ( node->GetSourceObject() == this ) && ( node->flags & flags ) )
		{
			LL_Remove( node, next, prev );
			delete node;
		}
		node = next;
	}
}

/*
=======================
CancelPendingEvents
=======================
*/
void Listener::CancelPendingEvents( void )
{
	EventQueueNode *node;
	EventQueueNode *next;

	node = Event::EventQueue.next;

	while( node != &Event::EventQueue )
	{
		next = node->next;
		if( node->GetSourceObject() == this )
		{
			LL_Remove( node, next, prev );
			delete node;
		}
		node = next;
	}
}

/*
=======================
EventPending
=======================
*/
qboolean Listener::EventPending( Event &ev )
{
	EventQueueNode *event;
	int eventnum;

	event = Event::EventQueue.next;

	eventnum = ev.eventnum;

	while( event != &Event::EventQueue )
	{
		if( ( event->GetSourceObject() == this ) && ( event->event->eventnum ) )
		{
			return true;
		}

		event = event->next;
	}

	return false;
}

/*
=======================
PostEventInternal
=======================
*/
EventQueueNode *Listener::PostEventInternal( Event *ev, float delay, int flags )
{
	EventQueueNode *node;
	EventQueueNode *i;
	int inttime;

	if( !classinfo()->responseLookup[ ev->eventnum ] )
	{
		if( !ev->eventnum )
		{
#ifdef _DEBUG
			glbs.DPrintf( "^~^~^ Failed execution of event '%s' for class '%s'\n", ev->name.c_str(), getClassname() );
#else
			glbs.DPrintf( "^~^~^ Failed execution of event for class '%s'\n", getClassname() );
#endif
		}

		delete ev;
		return NULL;
	}

	node = new EventQueueNode;

	i = Event::EventQueue.next;
	inttime = level.inttime + ( delay * 1000.0f + 0.5f );

	while( i != &Event::EventQueue && inttime > i->inttime )
	{
		i = i->next;
	}

	node->inttime = inttime;
	node->event = ev;
	node->flags = flags;
	node->SetSourceObject( this );

#ifdef _DEBUG
	node->name = ev->name;
#endif

	LL_Add( i, node, next, prev );

	return node;
}

/*
=======================
PostEvent
=======================
*/
void Listener::PostEvent( Event *ev, float delay, int flags )
{
	PostEventInternal( ev, delay, flags );
}

/*
=======================
PostEvent
=======================
*/
void Listener::PostEvent( const Event &ev, float delay, int flags )
{
	Event *e = new Event( ev );

	PostEventInternal( e, delay, flags );
}

/*
=======================
PostponeAllEvents
=======================
*/
qboolean Listener::PostponeAllEvents( float time )
{
	EventQueueNode *event;
	EventQueueNode *node;

	event = Event::EventQueue.next;
	while( event != &Event::EventQueue )
	{
		if( event->GetSourceObject() == this )
		{
			event->inttime += time * 1000.0f + 0.5f;

			node = event->next;
			while( ( node != &Event::EventQueue ) && ( event->inttime >= node->inttime ) )
			{
				node = node->next;
			}

			LL_Remove( event, next, prev );
			LL_Add( node, event, next, prev );

			return true;
		}
		event = event->next;
	}

	return false;
}

/*
=======================
PostponeEvent
=======================
*/
qboolean Listener::PostponeEvent( Event &ev, float time )
{
	EventQueueNode *event;
	EventQueueNode *node;
	int eventnum;

	eventnum = ev.eventnum;

	event = Event::EventQueue.next;
	while( event != &Event::EventQueue )
	{
		if( ( event->GetSourceObject() == this ) && ( event->event->eventnum == eventnum ) )
		{
			event->inttime += time * 1000.0f + 0.5f;

			node = event->next;
			while( ( node != &Event::EventQueue ) && ( event->inttime >= node->inttime ) )
			{
				node = node->next;
			}

			LL_Remove( event, next, prev );
			LL_Add( node, event, next, prev );

			return true;
		}
		event = event->next;
	}

	return false;
}

/*
=======================
ProcessEvent
=======================
*/
bool Listener::ProcessEvent( Event *ev )
{
	ClassDef *c = classinfo();
	ResponseDef<Class> *responses = NULL;
	Response response = NULL;

	if( !ev->eventnum )
	{
#ifdef _DEBUG
		EVENT_DPrintf( "^~^~^ Failed execution of event '%s' for class '%s'\n", ev->name.c_str(), c->classname );
#else
		EVENT_DPrintf( "^~^~^ Failed execution of event for class '%s'\n", c->classname );
#endif

		delete ev;
		return false;
	}

	responses = c->responseLookup[ ev->eventnum ];

	if( responses == NULL )
	{
		delete ev;
		return true;
	}

	response = responses->response;

	try
	{
		if( response )
		{
			( this->*response )( ev );
		}
	}
	catch( ScriptException& exc )
	{
		ev->ErrorInternal( this, exc.string );
		glbs.DPrintf( "%s\n", exc.string.c_str() );
	}

	delete ev;

	return true;
}

/*
=======================
ProcessEvent
=======================
*/
bool Listener::ProcessEvent( Event &ev )
{
	return ProcessScriptEvent( ev );
}

/*
=======================
ProcessEventReturn
=======================
*/
ScriptVariable& Listener::ProcessEventReturn( Event *ev )
{
	ClassDef *c = classinfo();
	ResponseDef<Class> *responses = NULL;
	Response response = NULL;
	static ScriptVariable m_Return;

	if( !ev->eventnum )
	{
#ifdef _DEBUG
		EVENT_Printf( "^~^~^ Failed execution of event '%s' for class '%s'\n", ev->name.c_str(), c->classname );
#else
		EVENT_Printf( "^~^~^ Failed execution of event for class '%s'\n", c->classname );
#endif

		delete ev;
		return m_Return;
	}

	responses = c->responseLookup[ ev->eventnum ];

	if( responses == NULL )
	{

		EVENT_Printf( "^~^~^ Failed execution of command '%s' for class '%s'\n", Event::GetEventName( ev->eventnum ).c_str(), c->classname );
		delete ev;
		return m_Return;
	}

	response = responses->response;

	int previousArgs = ev->NumArgs();

	if( response )
	{
		( this->*response )( ev );
	}

	if( previousArgs != ev->NumArgs() && ev->NumArgs() != 0 )
	{
		m_Return = ev->GetValue( ev->NumArgs() );
	}

	delete ev;

	return m_Return;
}

/*
=======================
ProcessEvent
=======================
*/
bool Listener::ProcessEvent( const Event &ev )
{
	Event *event = new Event( ev );
	return ProcessEvent( event );
}

/*
=======================
ProcessScriptEvent
=======================
*/
bool Listener::ProcessScriptEvent( Event *ev )
{
	bool result = ProcessScriptEvent( *ev );

	delete ev;
	return result;
}

/*
=======================
ProcessScriptEvent
=======================
*/
bool Listener::ProcessScriptEvent( Event &ev )
{
	ClassDef *c = classinfo();
	ResponseDef<Class> *responses = NULL;
	Response response = NULL;

	if( !ev.eventnum )
	{
#ifdef _DEBUG
		EVENT_Printf( "^~^~^ Failed execution of event '%s' for class '%s'\n", ev.name.c_str(), c->classname );
#else
		EVENT_Printf( "^~^~^ Failed execution of event for class '%s'\n", c->classname );
#endif

		return false;
	}

	responses = c->responseLookup[ ev.eventnum ];

	if( responses == NULL )
	{
		return true;
	}

	response = responses->response;

	if( response )
	{
		( this->*response )( &ev );
	}

	return true;
}


void Listener::ProcessContainerEvent( const Container< Event * >& conev )
{
	int num = conev.NumObjects();

	for( int i = 1; i <= num; i++ )
	{
		ProcessEvent( conev.ObjectAt( i ) );
	}
}

/*
=======================
ProcessPendingEvents
=======================
*/
qboolean Listener::ProcessPendingEvents( void )
{
	EventQueueNode *event;
	qboolean processedEvents;
	float t;

	processedEvents = false;

	t = level.inttime;

	event = Event::EventQueue.next;
	while( event != &Event::EventQueue )
	{
		Listener *obj;

		assert( event );

		obj = event->GetSourceObject();

		if( event->inttime > t )
		{
			break;
		}

		if( obj != this )
		{
			// traverse normally
			event = event->next;
		}
		else
		{
			// the event is removed from its list and temporarily added to the active list
			LL_Remove( event, next, prev );

			// ProcessEvent will dispose of this event when it is done
			obj->ProcessEvent( event->event );

			// free up the node
			delete event;

			// start over, since can't guarantee that we didn't process any previous or following events
			event = Event::EventQueue.next;

			processedEvents = true;
		}
	}

	return processedEvents;
}

/*
=======================
GetScriptOwner

listener.owner
=======================
*/
Listener *Listener::GetScriptOwner( void )
{
	return NULL;
}

#ifndef NO_SCRIPTENGINE

/*
=======================
CreateVars
=======================
*/
void Listener::CreateVars( void )
{
	vars = new ScriptVariableList;
}

/*
=======================
ClearVars
=======================
*/
void Listener::ClearVars( void )
{
	if( vars )
	{
		delete vars;
		CreateVars();
	}
}

/*
=======================
Vars
=======================
*/
ScriptVariableList *Listener::Vars( void )
{
	if( !vars ) {
		CreateVars();
	}

	return vars;
}

/*
=======================
BroadcastEvent
=======================
*/
bool Listener::BroadcastEvent( str name, Event &event )
{
	return BroadcastEvent( Director.AddString( name ), event );
}

/*
=======================
BroadcastEvent
=======================
*/
bool Listener::BroadcastEvent( const_str name, Event &event )
{
	if( !m_NotifyList )
	{
		return false;
	}

	ConList *listeners = m_NotifyList->findKeyValue( name );

	if( !listeners )
	{
		return false;
	}

	return BroadcastEvent( event, listeners );
}

/*
=======================
BroadcastEvent

Broadcast an event to the notify list
=======================
*/
bool Listener::BroadcastEvent( Event &event, ConList *listeners )
{
	int num = listeners->NumObjects();

	if( !num )
	{
		return false;
	}

	if( num == 1 )
	{
		Listener *listener = listeners->ObjectAt( 1 );

		if( listener )
		{
			listener->ProcessEvent( event );
		}

		return false;
	}

	ConList listenersCopy = *listeners;
	bool found = false;

	for( int i = listenersCopy.NumObjects(); i > 0; i-- )
	{
		Listener *listener = listenersCopy.ObjectAt( i );

		if( listener )
		{
			listener->ProcessEvent( event );
			found = true;
		}
	}

	return found;
}

/*
=======================
CancelWaiting
=======================
*/
void Listener::CancelWaiting( str name )
{
	CancelWaiting( Director.AddString( name ) );
}

/*
=======================
CancelWaiting
=======================
*/
void Listener::CancelWaiting( const_str name )
{
	if( !m_WaitForList )
	{
		return;
	}

	ConList *list = m_WaitForList->findKeyValue( name );
	ConList stoppedListeners;

	if( !list )
	{
		return;
	}

	CancelWaitingSources( name, *list, stoppedListeners );

	m_WaitForList->remove( name );

	if( m_WaitForList->isEmpty() )
	{
		delete m_WaitForList;
		m_WaitForList = NULL;

		if( !DisableListenerNotify )
		{
			StoppedWaitFor( name, false );
		}
	}

	if( !DisableListenerNotify )
	{
		for( int i = stoppedListeners.NumObjects(); i > 0; i-- )
		{
			Listener *listener = stoppedListeners.ObjectAt( i );

			if( listener )
			{
				listener->StoppedNotify();
			}
		}
	}
}

/*
=======================
CancelWaitingAll
=======================
*/
void Listener::CancelWaitingAll()
{
	CancelWaiting( 0 );

	if( !m_WaitForList )
	{
		return;
	}

	con_set_enum< const_str, ConList > en = *m_WaitForList;
	Entry< const_str, ConList > *e;
	ConList stoppedListeners;

	for( e = en.NextElement(); e != NULL; e = en.NextElement() )
	{
		CancelWaitingSources( e->key, e->value, stoppedListeners );
	}

	delete m_WaitForList;
	m_WaitForList = NULL;

	if( !DisableListenerNotify )
	{
		StoppedWaitFor( 0, false );

		for( int i = stoppedListeners.NumObjects(); i > 0; i-- )
		{
			Listener *listener = stoppedListeners.ObjectAt( i );

			if( listener )
			{
				listener->StoppedNotify();
			}
		}
	}
}

/*
=======================
CancelWaitingSources
=======================
*/
void Listener::CancelWaitingSources( const_str name, ConList &listeners, ConList &stoppedListeners )
{
	for( int i = listeners.NumObjects(); i > 0; i-- )
	{
		Listener *listener = listeners.ObjectAt( i );

		if( listener && listener->UnregisterSource( name, this ) )
		{
			stoppedListeners.AddObject( listener );
		}
	}
}

/*
=======================
Notify
=======================
*/
void Listener::Notify( const char *name )
{
	Unregister( name );
}

/*
=======================
StoppedNotify

Called when stopped notifying for a listener
=======================
*/
void Listener::StoppedNotify( void )
{
}

/*
=======================
StartedWaitFor

Called when started waiting for a listener
=======================
*/
void Listener::StartedWaitFor( void )
{
}

/*
=======================
StoppedWaitFor

Called when stopped wait for a listener
=======================
*/
void Listener::StoppedWaitFor( const_str name, bool bDeleting )
{
}

/*
=======================
EndOn
=======================
*/
void Listener::EndOn( str name, Listener *listener )
{
	EndOn( Director.AddString( name ), listener );
}

/*
=======================
EndOn

Removes the specified listener when notifying
=======================
*/
void Listener::EndOn( const_str name, Listener *listener )
{
	if( !m_EndList )
	{
		m_EndList = new con_set < const_str, ConList >;
	}

	ConList& list = m_EndList->addKeyValue( name );

	list.AddUniqueObject( listener );
}

/*
=======================
Register
=======================
*/
void Listener::Register( str name, Listener *listener )
{
	Register( Director.AddString( name ), listener );
}

/*
=======================
Register

Registers a listener, that will be notified with Unregister
Listener is usually a ScriptThread
=======================
*/
void Listener::Register( const_str name, Listener *listener )
{
	RegisterSource( name, listener );
	listener->RegisterTarget( name, this );
}

/*
=======================
RegisterSource
=======================
*/
void Listener::RegisterSource( const_str name, Listener *listener )
{
	if( !m_NotifyList )
	{
		m_NotifyList = new con_set < const_str, ConList >;
	}

	ConList& list = m_NotifyList->addKeyValue( name );

	list.AddObject( listener );
}

/*
=======================
RegisterTarget
=======================
*/
void Listener::RegisterTarget( const_str name, Listener *listener )
{
	if( !m_WaitForList )
	{
		StartedWaitFor();
		m_WaitForList = new con_set < const_str, ConList >;
	}

	ConList& list = m_WaitForList->addKeyValue( name );

	list.AddObject( listener );
}

/*
=======================
Unregister
=======================
*/
void Listener::Unregister( str name )
{
	Unregister( Director.AddString( name ) );
}

/*
=======================
Unregister

Unregister listeners from the specified event
=======================
*/
void Listener::Unregister( const_str name )
{
	if( m_EndList )
	{
		ConList *list = m_EndList->findKeyValue( name );
		bool bDeleteSelf = false;

		if( list )
		{
			ConList listeners = *list;

			m_EndList->remove( name );

			if( m_EndList->isEmpty() )
			{
				delete m_EndList;
				m_EndList = NULL;
			}

			for( int i = listeners.NumObjects(); i > 0; i-- )
			{
				Listener *listener = listeners.ObjectAt( i );

				if( listener )
				{
					if( listener == this && ( name == STRING_REMOVE || name == STRING_DELETE || bDeleteSelf ) )
					{
						continue;
					}

					if( listener == this )
					{
						bDeleteSelf = true;
					}

					delete listener;
				}
			}
		}

		if( bDeleteSelf )
		{
			return;
		}
	}

	if( !m_NotifyList )
	{
		return;
	}

	ConList *list = m_NotifyList->findKeyValue( name );
	ConList stoppedListeners;
	Container< const_str > stoppedNames;

	if( !list )
	{
		return;
	}

	UnregisterTargets( name, *list, stoppedListeners, stoppedNames );

	m_NotifyList->remove( name );

	if( m_NotifyList->isEmpty() )
	{
		delete m_NotifyList;
		m_NotifyList = NULL;

		if( !DisableListenerNotify )
		{
			StoppedNotify();
		}
	}

	if( !DisableListenerNotify )
	{
		for( int i = stoppedListeners.NumObjects(); i > 0; i-- )
		{
			Listener *listener = stoppedListeners.ObjectAt( i );

			if( listener )
			{
				listener->StoppedWaitFor( name, false );
			}
		}
	}
}

/*
=======================
Unregister
=======================
*/
void Listener::Unregister( str name, Listener *listener )
{
	Unregister( Director.AddString( name ) );
}

/*
=======================
Unregister

Unregister a specified listener with the specified label
=======================
*/
void Listener::Unregister( const_str name, Listener *listener )
{
	if( UnregisterSource( name, listener ) )
	{
		if( !DisableListenerNotify ) {
			StoppedNotify();
		}
	}

	if( listener->UnregisterTarget( name, this ) )
	{
		if( !DisableListenerNotify ) {
			listener->StoppedWaitFor( name, false );
		}
	}
}

/*
=======================
UnregisterAll
=======================
*/
void Listener::UnregisterAll( void )
{
	Unregister( 0 );

	if( m_EndList )
	{
		delete m_EndList;
		m_EndList = NULL;
	}

	if( !m_NotifyList )
	{
		return;
	}

	con_set_enum < const_str, ConList > en = *m_NotifyList;
	Entry< const_str, ConList > *e;
	ConList stoppedListeners;
	Container< const_str > stoppedNames;

	en = *m_NotifyList;

	for( e = en.NextElement(); e != NULL; e = en.NextElement() )
	{
		UnregisterTargets( e->key, e->value, stoppedListeners, stoppedNames );
	}

	delete m_NotifyList;
	m_NotifyList = NULL;

	if( !DisableListenerNotify ) {
		StoppedNotify();
	}

	for( int i = stoppedListeners.NumObjects(); i > 0; i-- )
	{
		Listener *listener = stoppedListeners.ObjectAt( i );

		if( listener )
		{
			if( !DisableListenerNotify ) {
				listener->StoppedWaitFor( stoppedNames.ObjectAt( i ), true );
			}
		}
	}
}

/*
=======================
UnregisterSource
=======================
*/
bool Listener::UnregisterSource( const_str name, Listener *listener )
{
	if( !m_NotifyList )
	{
		return false;
	}

	ConList *list = m_NotifyList->findKeyValue( name );
	bool found = false;

	if( !list )
	{
		return false;
	}

	for( int i = list->NumObjects(); i > 0; i-- )
	{
		if( list->ObjectAt( i ) == listener )
		{
			list->RemoveObjectAt( i );
			found = true;
		}
	}

	if( list->NumObjects() == 0 )
	{
		m_NotifyList->remove( name );

		if( m_NotifyList->isEmpty() )
		{
			delete m_NotifyList;
			m_NotifyList = NULL;
		}
	}

	return found;
}

/*
=======================
UnregisterTarget
=======================
*/
bool Listener::UnregisterTarget( const_str name, Listener *listener )
{
	if( !m_WaitForList )
	{
		return false;
	}

	ConList *list = m_WaitForList->findKeyValue( name );
	bool found = false;

	if( !list )
	{
		return false;
	}

	for( int i = list->NumObjects(); i > 0; i-- )
	{
		if( list->ObjectAt( i ) == listener )
		{
			list->RemoveObjectAt( i );
			found = true;
		}
	}

	if( list->NumObjects() == 0 )
	{
		m_WaitForList->remove( name );

		if( m_WaitForList->isEmpty() )
		{
			delete m_WaitForList;
			m_WaitForList = NULL;
		}
	}

	return found;
}

/*
=======================
UnregisterTargets
=======================
*/
void Listener::UnregisterTargets( const_str name, ConList &listeners, ConList &stoppedListeners, Container< const_str > &stoppedNames )
{
	for( int i = listeners.NumObjects(); i > 0; i-- )
	{
		Listener *listener = listeners.ObjectAt( i );

		if( listener && listener->UnregisterTarget( name, this ) )
		{
			stoppedListeners.AddObject( listener );
			stoppedNames.AddObject( name );
		}
	}
}

/*
=======================
AbortRegistration

Abort the listener from registration of the specified label
Doesn't notify
=======================
*/
void Listener::AbortRegistration( const_str name, Listener *l )
{
	UnregisterSource( name, l );
	l->UnregisterTarget( name, this );
}

/*
=======================
RegisterSize
=======================
*/
int Listener::RegisterSize( str name ) const
{
	return RegisterSize( Director.AddString( name ) );
}

/*
=======================
RegisterSize

Returns how many listeners in the notify list
=======================
*/
int Listener::RegisterSize( const_str name ) const
{
	ConList *listeners;

	if( !m_NotifyList ) {
		return 0;
	}

	listeners = m_NotifyList->findKeyValue( name );

	// return the number of listeners waiting for this listener
	if( listeners )
	{
		return listeners->NumObjects();
	}
	else
	{
		return 0;
	}
}

/*
=======================
WaitingSize
=======================
*/
int Listener::WaitingSize( str name ) const
{
	return WaitingSize( Director.AddString( name ) );
}

/*
=======================
WaitingSize

Returns how many listeners in the wait list
=======================
*/
int Listener::WaitingSize( const_str name ) const
{
	ConList *listeners;

	if( !m_WaitForList ) {
		return 0;
	}

	listeners = m_WaitForList->findKeyValue( name );

	// return the number of listeners this listener is waiting for
	if( listeners )
	{
		return listeners->NumObjects();
	}
	else
	{
		return 0;
	}
}

/*
=======================
WaitTillDisabled
=======================
*/
bool Listener::WaitTillDisabled( str s )
{
	return WaitTillDisabled( Director.AddString( s ) );
}

/*
=======================
WaitTillDisabled

Returns true if the specified waittill is disabled
=======================
*/
bool Listener::WaitTillDisabled( const_str s )
{
	for( ClassDef *c = classinfo(); c != NULL; c = c->super )
	{
		if( c->WaitTillDefined( s ) )
		{
			return true;
		}
	}

	return false;
}

#endif

/*
=======================
GetFlags
=======================
*/
int Listener::GetFlags( Event *event ) const
{
	return classinfo()->GetFlags( event );
}

/*
=======================
ValidEvent
=======================
*/
qboolean Listener::ValidEvent( str name ) const
{
	int num;
	EventDef *def;

	num = Event::FindEventNum( name );
	if( !num )
		return qfalse;

	def = classinfo()->GetDef( num );
	if( !def )
		return qfalse;

	return qtrue;
}

//==========================
// Listener's events
//==========================

/*
=======================
EventDelete
=======================
*/
void Listener::EventDelete( Event *ev )
{
	if( ev->NumArgs() )
	{
		ScriptError( "Arguments not allowed." );
	}

	delete this;
}

/*
=======================
EventInheritsFrom
=======================
*/
void Listener::EventInheritsFrom( Event *ev )
{
	ev->AddInteger( inheritsFrom( ev->GetString( 1 ) ) );
}

/*
=======================
EventIsInheritedBy
=======================
*/
void Listener::EventIsInheritedBy( Event *ev )
{
	ev->AddInteger( isInheritedBy( ev->GetString( 1 ) ) );
}

/*
=======================
GetClassname
=======================
*/
void Listener::GetClassname( Event *ev )
{
	ev->AddString( getClassname() );
}

/*
=======================
CommandDelay
=======================
*/
void Listener::CommandDelay( Event *ev )
{
	if( ev->NumArgs() < 2 )
	{
		ScriptError( "Not enough arguments." );
	}

	Event *e = new Event( ev->GetString( 2 ) );

	for( int i = 3; i <= ev->NumArgs(); i++ )
	{
		e->AddValue( ev->GetValue( i ) );
	}

	PostEvent( e, ev->GetFloat( 1 ) );
}

#ifndef NO_SCRIPTENGINE

/*
=======================
CancelFor

Removes all listeners in the notify list
=======================
*/
void Listener::CancelFor( Event *ev )
{
	BroadcastEvent( ev->GetConstString( 1 ), EV_Remove );
}

/*
=======================
EventDelayThrow

Same as EventThrow
=======================
*/
void Listener::EventDelayThrow( Event *ev )
{
	BroadcastEvent( 0, *ev );
}

/*
=======================
EventEndOn
=======================
*/
void Listener::EventEndOn( Event *ev )
{
	const_str name = ev->GetConstString( 1 );

	if( Director.CurrentThread() == this )
	{
		ScriptError( "cannot end for the current thread!" );
	}

	EndOn( name, Director.CurrentThread() );
}

/*
=======================
EventGetOwner
=======================
*/
void Listener::EventGetOwner( Event *ev )
{
	ev->AddListener( GetScriptOwner() );
}

/*
=======================
EventNotify
=======================
*/
void Listener::EventNotify( Event *ev )
{
	str name = ev->GetString( 1 );

	Notify( name );
}

/*
=======================
EventThrow
=======================
*/
void Listener::EventThrow( Event *ev )
{
	BroadcastEvent( 0, *ev );
}

/*
=======================
EventUnregister
=======================
*/
void Listener::EventUnregister( Event *ev )
{
	Unregister( ev->GetConstString( 1 ) );
}

/*
=======================
WaitTill

Wait until event of type name
=======================
*/
void Listener::WaitTill( Event *ev )
{
	const_str name;

	if( Director.CurrentThread() == this )
	{
		ScriptError( "cannot waittill on the current thread!" );
	}

	name = ev->GetConstString( 1 );

	if( WaitTillDisabled( name ) )
	{
		ScriptError( "invalid waittill %s for '%s'", Director.GetString( name ).c_str(), getClassname() );
	}

	Register( name, Director.CurrentThread() );
}

/*
=======================
WaitTillTimeout

Wait until event of type name with a timeout time
=======================
*/
void Listener::WaitTillTimeout( Event *ev )
{
	const_str name;
	float timeout_time;

	if( Director.CurrentThread() == this )
	{
		ScriptError( "cannot waittill on the current thread!" );
	}

	timeout_time = ev->GetFloat( 1 );
	name = ev->GetConstString( 2 );

	if( WaitTillDisabled( name ) )
	{
		ScriptError( "invalid waittill %s for '%s'", Director.GetString( name ).c_str(), getClassname() );
	}

	Register( name, Director.CurrentThread() );
	Director.CurrentThread()->PostEvent( EV_ScriptThread_CancelWaiting, timeout_time );
}

/*
=======================
WaitTillAny

Wait until any event of type name
=======================
*/
void Listener::WaitTillAny( Event *ev )
{
	const_str name;

	if( Director.CurrentThread() == this )
	{
		ScriptError( "cannot waittill any on the current thread!" );
	}

	for( int i = 1; i <= ev->NumArgs(); i++ )
	{
		name = ev->GetConstString( i );

		if( WaitTillDisabled( name ) )
		{
			ScriptError( "invalid waittill %s for '%s'", Director.GetString( name ).c_str(), getClassname() );
		}

		Register( name, Director.CurrentThread() );
	}
}

/*
=======================
WaitTillAnyTimeout

Wait until any event of type name with a timeout time
=======================
*/
void Listener::WaitTillAnyTimeout( Event *ev )
{
	const_str name;
	float timeout_time;

	if( Director.CurrentThread() == this )
	{
		ScriptError( "cannot waittill any on the current thread!" );
	}

	timeout_time = ev->GetFloat( 1 );

	for( int i = 1; i <= ev->NumArgs(); i++ )
	{
		name = ev->GetConstString( i );

		if( WaitTillDisabled( name ) )
		{
			ScriptError( "invalid waittill %s for '%s'", Director.GetString( name ).c_str(), getClassname() );
		}

		Register( name, Director.CurrentThread() );
	}

	Director.CurrentThread()->PostEvent( EV_ScriptThread_CancelWaiting, timeout_time );
}

/*
=======================
ExecuteScriptInternal
=======================
*/
void Listener::ExecuteScriptInternal( Event *ev, ScriptVariable& returnValue )
{
	SafePtr< ScriptThread > thread = CreateScriptInternal( ev->GetValue( 1 ) );
	ScriptThread *currentThread;
	bool fReturn = returnValue.GetType() != VARIABLE_NONE;

	thread->ScriptExecute( &ev->data[ 1 ], ev->dataSize - 1, returnValue );

	currentThread = Director.m_CurrentThread;

	// current thread should request a context switch because its waiting for the child's return value
	if( Director.m_bAllowContextSwitch && currentThread && fReturn && thread )
	{
		currentThread->HandleContextSwitch( thread );
	}
}

/*
=======================
ExecuteThreadInternal
=======================
*/
void Listener::ExecuteThreadInternal( Event *ev, ScriptVariable& returnValue )
{
	SafePtr< ScriptThread > thread = CreateThreadInternal( ev->GetValue( 1 ) );
	ScriptThread *currentThread;
	bool fReturn = returnValue.GetType() != VARIABLE_NONE;

	thread->ScriptExecute( &ev->data[ 1 ], ev->dataSize - 1, returnValue );

	currentThread = Director.m_CurrentThread;

	if( Director.m_bAllowContextSwitch && currentThread && fReturn && thread )
	{
		currentThread->HandleContextSwitch( thread );
	}
}

/*
=======================
WaitExecuteScriptInternal
=======================
*/
void Listener::WaitExecuteScriptInternal( Event *ev, ScriptVariable& returnValue )
{
	ScriptThread *thread = CreateScriptInternal( ev->GetValue( 1 ) );

	thread->GetScriptClass()->Register( 0, Director.CurrentThread() );

	thread->ScriptExecute( &ev->data[ 1 ], ev->dataSize - 1, returnValue );
}

/*
=======================
WaitExecuteThreadInternal
=======================
*/
void Listener::WaitExecuteThreadInternal( Event *ev, ScriptVariable& returnValue )
{
	ScriptThread *thread = CreateThreadInternal( ev->GetValue( 1 ) );

	thread->GetScriptClass()->Register( 0, Director.CurrentThread() );

	thread->ScriptExecute( &ev->data[ 1 ], ev->dataSize - 1, returnValue );
}

/*
=======================
CreateScriptInternal
=======================
*/
ScriptThread *Listener::CreateScriptInternal( const ScriptVariable& label )
{
	GameScript *scr;
	ScriptThread *thread = NULL;

	if( label.GetType() == VARIABLE_STRING || label.GetType() == VARIABLE_CONSTSTRING )
	{
		if( label.GetType() == VARIABLE_CONSTSTRING )
			thread = Director.CreateScriptThread( Director.GetGameScript( label.stringValue() ), this, "" );
		else
			thread = Director.CreateScriptThread( Director.GetGameScript( label.constStringValue() ), this, "" );
	}
	else if( label.GetType() == VARIABLE_CONSTARRAY && label.arraysize() > 1 )
	{
		ScriptVariable *script = label[ 1 ];
		ScriptVariable *labelname = label[ 2 ];

		if( script->GetType() == VARIABLE_CONSTSTRING )
			scr = Director.GetGameScript( script->constStringValue() );
		else
			scr = Director.GetGameScript( script->stringValue() );

		if( labelname->GetType() == VARIABLE_CONSTSTRING )
			thread = Director.CreateScriptThread( scr, this, labelname->constStringValue() );
		else
			thread = Director.CreateScriptThread( scr, this, labelname->stringValue() );
	}
	else
	{
		ScriptError( "Listener::CreateScriptInternal: bad label type '%s'", label.GetTypeName() );
	}

	return thread;
}

/*
=======================
CreateThreadInternal
=======================
*/
ScriptThread *Listener::CreateThreadInternal( const ScriptVariable& label )
{
	GameScript *scr;
	ScriptThread *thread = NULL;

	if( label.GetType() == VARIABLE_STRING || label.GetType() == VARIABLE_CONSTSTRING )
	{
		ScriptClass *scriptClass = Director.CurrentScriptClass();
		scr = scriptClass->GetScript();

		if( label.GetType() == VARIABLE_CONSTSTRING )
			thread = Director.CreateScriptThread( scr, this, label.constStringValue() );
		else
			thread = Director.CreateScriptThread( scr, this, label.stringValue() );
	}
	else if( label.GetType() == VARIABLE_CONSTARRAY && label.arraysize() > 1 )
	{
		ScriptVariable *script = label[ 1 ];
		ScriptVariable *labelname = label[ 2 ];

		if( script->GetType() == VARIABLE_CONSTSTRING )
			scr = Director.GetGameScript( script->constStringValue() );
		else
			scr = Director.GetGameScript( script->stringValue() );

		if( labelname->GetType() == VARIABLE_CONSTSTRING )
			thread = Director.CreateScriptThread( scr, this, labelname->constStringValue() );
		else
			thread = Director.CreateScriptThread( scr, this, labelname->stringValue() );
	}
	else
	{
		ScriptError( "ScriptClass::CreateThreadInternal: bad argument format" );
	}

	return thread;
}

/*
=======================
CreateReturnThread
=======================
*/
void Listener::CreateReturnThread( Event *ev )
{
	ScriptVariable returnValue;

	returnValue.newPointer();

	ExecuteThreadInternal( ev, returnValue );

	ev->AddValue( returnValue );
}

/*
=======================
CreateThread
=======================
*/
void Listener::CreateThread( Event *ev )
{
	if( !Director.m_bAllowContextSwitch )
	{
		ScriptVariable returnValue;

		ExecuteThreadInternal( ev, returnValue );
	}
	else
	{
		ScriptThread *pThread = CreateThreadInternal( ev->GetValue( 1 ) );
		Event event;

		for( int i = 2; i <= ev->NumArgs(); i++ )
		{
			event.AddValue( ev->GetValue( i ) );
		}

		if( pThread )
		{
			pThread->DelayExecute( event );
		}
	}
}

/*
=======================
ExecuteReturnScript
=======================
*/
void Listener::ExecuteReturnScript( Event *ev )
{
	ScriptVariable returnValue;

	returnValue.newPointer();

	ExecuteScriptInternal( ev, returnValue );

	ev->AddValue( returnValue );
}

/*
=======================
ExecuteScript
=======================
*/
void Listener::ExecuteScript( Event *ev )
{
	if( !Director.m_bAllowContextSwitch )
	{
		ScriptVariable returnValue;

		ExecuteScriptInternal( ev, returnValue );
	}
	else
	{
		ScriptThread *pThread = CreateScriptInternal( ev->GetValue( 1 ) );
		Event event;

		for( int i = 2; i <= ev->NumArgs(); i++ )
		{
			event.AddValue( ev->GetValue( i ) );
		}

		if( pThread )
		{
			pThread->DelayExecute( event );
		}
	}
}

/*
=======================
WaitCreateReturnThread
=======================
*/
void Listener::WaitCreateReturnThread( Event *ev )
{
	ScriptVariable returnValue;

	returnValue.newPointer();

	WaitExecuteThreadInternal( ev, returnValue );

	ev->AddValue( returnValue );
}

/*
=======================
WaitCreateThread
=======================
*/
void Listener::WaitCreateThread( Event *ev )
{
	ScriptVariable returnValue;

	WaitExecuteThreadInternal( ev, returnValue );
}

/*
=======================
WaitExecuteReturnScript
=======================
*/
void Listener::WaitExecuteReturnScript( Event *ev )
{
	ScriptVariable returnValue;

	returnValue.newPointer();

	WaitExecuteScriptInternal( ev, returnValue );

	ev->AddValue( returnValue );
}

/*
=======================
WaitExecuteScript
=======================
*/
void Listener::WaitExecuteScript( Event *ev )
{
	ScriptVariable returnValue;

	WaitExecuteScriptInternal( ev, returnValue );
}

/*
=======================
ExecuteThread

Execute a thread with optionally parameters
=======================
*/
void Listener::ExecuteThread( str scriptName, str labelName, Event *params )
{
	ScriptThread *thread = Director.CreateThread( scriptName, labelName, this );

	try
	{
		if( !thread )
		{
			return;
		}

		thread->Execute( params );
	}
	catch( ScriptException& exc )
	{
		glbs.DPrintf( "Listener::ExecuteThread: %s\n", exc.string.c_str() );
	}
}

/*
=======================
ExecuteThread

Execute a thread with optionally parameters
=======================
*/
void Listener::ExecuteThread( str scriptName, str labelName, Event& params )
{
	ScriptThread *thread = Director.CreateThread( scriptName, labelName, this );

	try
	{
		if( !thread )
		{
			return;
		}

		thread->Execute( params );
	}
	catch( ScriptException& exc )
	{
		glbs.DPrintf( "Listener::ExecuteThread: %s\n", exc.string.c_str() );
	}
}

#endif

CLASS_DECLARATION( Class, Listener, NULL )
{
	{ &EV_Listener_CommandDelay,				&Listener::CommandDelay },
	{ &EV_Delete,								&Listener::EventDelete },
	{ &EV_Remove,								&Listener::EventDelete },
	{ &EV_ScriptRemove,							&Listener::EventDelete },
	{ &EV_Listener_Classname,					&Listener::GetClassname },
	{ &EV_Listener_InheritsFrom,				&Listener::EventInheritsFrom },
	{ &EV_Listener_IsInheritedBy,				&Listener::EventIsInheritedBy },

#ifndef NO_SCRIPTENGINE
	{ &EV_Listener_CancelFor,					&Listener::CancelFor },
	{ &EV_Listener_CreateReturnThread,			&Listener::CreateReturnThread },
	{ &EV_Listener_CreateThread,				&Listener::CreateThread },
	{ &EV_Listener_ExecuteReturnScript,			&Listener::ExecuteReturnScript },
	{ &EV_Listener_ExecuteScript,				&Listener::ExecuteScript },
	{ &EV_Listener_EndOn,						&Listener::EventEndOn },
	{ &EV_Listener_GetOwner,					&Listener::EventGetOwner },
	{ &EV_Listener_Notify,						&Listener::EventNotify },
	{ &EV_DelayThrow,							&Listener::EventDelayThrow },
	{ &EV_Throw,								&Listener::EventThrow },
	{ &EV_Listener_Unregister,					&Listener::EventUnregister },
	{ &EV_Listener_WaitCreateReturnThread,		&Listener::WaitCreateReturnThread },
	{ &EV_Listener_WaitCreateThread,			&Listener::WaitCreateThread },
	{ &EV_Listener_WaitExecuteReturnScript,		&Listener::WaitExecuteReturnScript },
	{ &EV_Listener_WaitExecuteScript,			&Listener::WaitExecuteScript },
	{ &EV_Listener_WaitTill,					&Listener::WaitTill },
	{ &EV_Listener_WaitTillTimeout,				&Listener::WaitTillTimeout },
	{ &EV_Listener_WaitTillAny,					&Listener::WaitTillAny },
	{ &EV_Listener_WaitTillAnyTimeout,			&Listener::WaitTillAnyTimeout },
#endif
	{ NULL, NULL }
};

qboolean IsNumeric( const char *str )
{
	size_t len;
	size_t i;
	qboolean dot;

	if( *str == '-' )
	{
		str++;
	}

	dot = qfalse;
	len = strlen( str );
	for( i = 0; i < len; i++ )
	{
		if( !isdigit( str[ i ] ) )
		{
			if( ( str[ i ] == '.' ) && !dot )
			{
				dot = qtrue;
				continue;
			}
			return qfalse;
		}
	}

	return qtrue;
}
