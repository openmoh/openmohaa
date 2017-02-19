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

// listener.h: Listener

#ifndef __LISTENER_H__
#define __LISTENER_H__

#include "glb_local.h"
#include "class.h"

class Entity;
class Listener;
class ScriptClass;
class ScriptThread;
class ScriptVariable;
class ScriptVariableList;
class ScriptVM;
class SimpleEntity;
class Archiver;
class EventQueueNode;

// entity subclass
#define EF_ENTITY				(1<<0)
#define EF_ANIMATE				(1<<1)
#define EF_SENTIENT				(1<<2)
#define EF_PLAYER				(1<<3)
#define EF_ACTOR				(1<<4)
#define EF_ITEM					(1<<5)
#define EF_INVENTORYITEM		(1<<6)
#define EF_WEAPON				(1<<7)
#define EF_PROJECTILE			(1<<8)
#define EF_DOOR					(1<<9)
#define EF_CAMERA				(1<<10)
#define EF_VEHICLE				(1<<11)
#define EF_VEHICLETANK			(1<<12)
#define EF_VEHICLETURRET		(1<<13)
#define EF_TURRET				(1<<14)
#define EF_PATHNODE				(1<<15)
#define EF_WAYPOINT				(1<<16)
#define EF_TEMPWAYPOINT			(1<<17)
#define EF_VEHICLEPOINT			(1<<18)
#define EF_SPLINEPATH			(1<<19)
#define EF_CRATEOBJECT			(1<<20)
#define EF_BOT					(1<<21)

// Event flags
#define EV_CONSOLE				(1<<0)		// Allow entry from console
#define EV_CHEAT				(1<<1)		// Only allow entry from console if cheats are enabled
#define EV_CODEONLY				(1<<2)		// Hide from eventlist
#define EV_CACHE				(1<<3)		// This event is used to cache data in
#define EV_TIKIONLY				(1<<4)		// This command only applies to TIKI files
#define EV_SCRIPTONLY			(1<<5)		// This command only applies to SCRIPT files
#define EV_SERVERCMD			(1<<6)		// Client : server command
#define EV_DEFAULT				-1			// default flag

// Event types
#define EV_NORMAL				0	// Normal command
#define EV_RETURN				1	// Return as a function (local.var = local ReturnCommand)
#define EV_GETTER				2	// Return as a variable (local.var = local.listener.some_getter)
#define EV_SETTER				3	// Set as a variable (local.listener.some_setter = "value")

// times for posting events
// Even though negative times technically don't make sense, the effect is to 
// sort events that take place at the start of a map so that they are executed
// in the proper order.  For example, spawnargs must occur before any script
// commands take place, while unused entities must be removed before the spawnargs
// are parsed.

#define EV_REMOVE					-12.0f		// remove any unused entities before spawnargs are parsed
#define EV_PRIORITY_SPAWNARG		-11.0f		// for priority spawn args passed in by the bsp file
#define EV_SPAWNARG					-10.0f		// for spawn args passed in by the bsp file
#define EV_LINKDOORS				-9.0f		// for finding out which doors are linked together
#define EV_LINKBEAMS				-9.0f		// for finding out the endpoints of beams
#define EV_VEHICLE					-9.0f
#define EV_SETUP_ROPEPIECE			-8.0f
#define EV_SETUP_ROPEBASE			-7.0f
#define EV_PROCESS_INIT				-6.0f
#define EV_POSTSPAWN				-5.0f		// for any processing that must occur after all objects are spawned
#define EV_SPAWNENTITIES			-4.0f
#define EV_PRIORITY_SPAWNACTOR		-3.0f
#define EV_SPAWNACTOR				-2.0f

// Posted Event Flags
#define EVENT_LEGS_ANIM    (1<<0)      // this event is associated with an animation for the legs
#define EVENT_TORSO_ANIM   (1<<1)      // this event is associated with an animation for the torso
#define EVENT_DIALOG_ANIM  (1<<2)      // this event is associated with an animation for dialog lip syncing

typedef enum
{
	IS_STRING,
	IS_VECTOR,
	IS_BOOLEAN,
	IS_INTEGER,
	IS_FLOAT,
	IS_ENTITY,
	IS_LISTENER
} vartype;

class EventArgDef : public Class
{
private:
	int                     type;
	str                     name;
	float                   minRange[ 3 ];
	qboolean                minRangeDefault[ 3 ];
	float                   maxRange[ 3 ];
	qboolean                maxRangeDefault[ 3 ];
	qboolean                optional;
public:

	EventArgDef()
	{
		type = IS_INTEGER;
		//name        = "undefined";
		optional = false;
	};
	void              Setup( const char * eventName, const char *argName, const char *argType, const char *argRange );
	void              PrintArgument( FILE *event_file = NULL );
	void              PrintRange( FILE *event_file = NULL );
	int               getType( void );
	const char        *getName( void );
	qboolean          isOptional( void );

	float GetMinRange( int index )
	{
		if( index < 3 )
			return minRange[ index ];
		return 0.0;
	}

	qboolean GetMinRangeDefault( int index )
	{
		if( index < 3 )
			return minRangeDefault[ index ];
		return false;
	}

	float GetMaxRange( int index )
	{
		if( index < 3 )
			return maxRange[ index ];
		return 0.0;
	}

	qboolean GetMaxRangeDefault( int index )
	{
		if( index < 3 )
			return maxRangeDefault[ index ];
		return false;
	}
};

inline int EventArgDef::getType
(
void
)

{
	return type;
}

inline const char *EventArgDef::getName
(
void
)

{
	return name.c_str();
}

inline qboolean EventArgDef::isOptional
(
void
)

{
	return optional;
}

class EventDef {
public:
	str		 		command;
	int				flags;
	const char		*formatspec;
	const char		*argument_names;
	const char		*documentation;
	uchar			type;
	Container<EventArgDef> *definition;

	EventDef() { definition = NULL; }

	void Error( const char *format, ... );

	void PrintDocumentation( FILE *event_file, bool html );
	void PrintEventDocumentation( FILE *event_file, bool html );

	void DeleteDocumentation( void );
	void SetupDocumentation( void );
};

class command_t {
public:
	str			command;
	int			flags;
	uchar		type;

	friend bool		operator==( const str &name, const command_t &command );
	friend bool		operator==( const command_t &cmd1, const command_t &cmd2 );
};

inline bool operator==( const str &name, const command_t &command )
{
	return command.command == name;
}

#ifndef NO_SCRIPTENGINE
inline bool operator==( const command_t &cmd1, const command_t &cmd2 )
{
	return ( cmd2.command == cmd1.command && ( cmd2.type == ( uchar )-1 || cmd2.type == cmd1.type ) );
}
#else
inline bool operator==( const command_t &cmd1, const command_t &cmd2 )
{
	return ( cmd2.command == cmd1.command );
}
#endif

class Event : public Class
{
public:
	qboolean			fromScript;
	short unsigned int	eventnum;
	short unsigned int	dataSize;
	ScriptVariable		*data;

#ifdef _DEBUG
	// should be used only for debugging purposes
	str					name;
#endif

public:
	CLASS_PROTOTYPE( Event );

	static con_map< Event *, EventDef > eventDefList;
	static con_arrayset< command_t, command_t > commandList;

	static con_map< const_str, unsigned int > normalCommandList;
	static con_map< const_str, unsigned int > returnCommandList;
	static con_map< const_str, unsigned int > getterCommandList;
	static con_map< const_str, unsigned int > setterCommandList;

	static void LoadEvents( void );

	static EventQueueNode EventQueue;

	static int totalevents;
	static int NumEventCommands();

	static void       ListCommands( const char *mask = NULL );
	static void       ListDocumentation( const char *mask, qboolean print_to_file = qfalse );
	static void       PendingEvents( const char *mask = NULL );

	static int GetEvent( str name, uchar type = EV_NORMAL );
	static int GetEventWithFlags( str name, int flags, uchar type = EV_NORMAL );

	static command_t *GetEventInfo( int eventnum );
	static int GetEventFlags( int eventnum );
	static str& GetEventName( int index );

	static int compareEvents( const void *arg1, const void *arg2 );
	static void SortEventList( Container< int > *sortedList );

	virtual void ErrorInternal( Listener *l, str text );

	static unsigned int	FindEventNum( str s );
	static unsigned int FindNormalEventNum( const_str s );
	static unsigned int FindNormalEventNum( str s );
	static unsigned int FindReturnEventNum( const_str s );
	static unsigned int FindReturnEventNum( str s );
	static unsigned int FindSetterEventNum( const_str s );
	static unsigned int FindSetterEventNum( str s );
	static unsigned int FindGetterEventNum( const_str s );
	static unsigned int FindGetterEventNum( str s );

	bool		operator==( Event ev ) { return eventnum == ev.eventnum; }
	bool		operator!=( Event ev ) { return eventnum != ev.eventnum; }

#ifndef _DEBUG_MEM
	void *operator new( size_t size );
	void operator delete( void *ptr );
#endif

	Event();
	Event( const Event *ev );
	Event( const Event &ev );
	Event( int index );
	Event( str command );
	Event( str command, uchar type );
	Event
	(
		const char *command,
		int flags,
		const char *formatspec,		// Arguments are : 'e' (Entity) 'v' (Vector) 'i' (Integer) 'f' (Float) 's' (String) 'b' (Boolean).
									// Uppercase arguments means optional.
		const char *argument_names,
		const char *documentation,
		uchar type = EV_NORMAL
	);

	virtual ~Event();

	virtual void Archive( Archiver &arc );

#ifdef _GAME_DLL
	eventInfo_t		*getInfo();
#else
	EventDef		*getInfo();
#endif

	str&			getName();

	void		AddContainer( Container< SafePtr< Listener > > *container );
	void		AddEntity( Entity * ent );
	void		AddFloat( float number );
	void		AddInteger( int number );
	void		AddListener( Listener * listener );
	void		AddNil( void );
	void		AddConstString( const_str string );
	void		AddString( str string );
	void		AddToken( str token );
	void		AddTokens( int argc, const char **argv );
	void		AddValue( const ScriptVariable &value );
	void		AddVector( const Vector& vector );

	void		Clear( void );

	void		CheckPos( int pos );

	bool			GetBoolean( int pos );

	int				GetConstString( int pos );

	Entity			*GetEntity( int pos );

	float			GetFloat( int pos );
	int				GetInteger( int pos );
	Listener		*GetListener( int pos );

	class PathNode	*GetPathNode( int pos );

#ifndef NO_SCRIPTENGINE
	SimpleEntity	*GetSimpleEntity( int pos );
#endif

	str				GetString( int pos );
	str				GetToken( int pos );
	ScriptVariable&	GetValue( int pos );
	ScriptVariable&	GetValue( void );
	Vector			GetVector( int pos );

	class Waypoint	*GetWaypoint( int pos );

	qboolean	IsEntityAt( int pos );
	qboolean	IsListenerAt( int pos );
	qboolean	IsNilAt( int pos );
	qboolean	IsNumericAt( int pos );
#ifndef NO_SCRIPTENGINE
	qboolean	IsSimpleEntityAt( int pos );
#endif
	qboolean	IsStringAt( int pos );
	qboolean	IsVectorAt( int pos );

	qboolean	IsFromScript( void );

	const char	*GetFormat();
	int			NumArgs();
};

#ifdef GAME_DLL

class ConsoleEvent : public Event
{
private:
	gentity_t *m_consoleedict;

public:
	CLASS_PROTOTYPE( ConsoleEvent );

	void	*operator new( size_t size );
	void	operator delete( void *ptr );

	ConsoleEvent();
	ConsoleEvent( str name ) : Event( name ) { m_consoleedict = NULL; }

	void			SetConsoleEdict( gentity_t *edict );
	gentity_t		*GetConsoleEdict( void );

	virtual void	ErrorInternal( Listener *l, str text );
};

#endif

typedef struct eventInfo
{
	Event		*ev;
	const char	*command;
	int			flags;
	const char	*formatspec;
	const char	*argument_names;
	const char	*documentation;
	int			type;
	eventInfo	*prev;
} eventInfo_t;

#define					NODE_CANCEL				1
#define					NODE_FIXED_EVENT		2

class EventQueueNode {
public:
	Event *event;
	int	inttime;
	int flags;
	SafePtr< Listener > m_sourceobject;

	EventQueueNode		*prev;
	EventQueueNode		*next;

#ifdef _DEBUG
	str name;
#endif

	EventQueueNode() { prev = this; next = this; }
	Listener	*GetSourceObject( void ) { return m_sourceobject; }
	void		SetSourceObject( Listener *obj ) { m_sourceobject = obj; }
};

template< class Type1, class Type2 >
class con_map;

typedef ContainerClass< SafePtr< Listener > > ConList;
typedef con_map< Event *, EventDef * > eventMap;

typedef SafePtr< Listener > ListenerPtr;

class Listener : public Class
{
public:
#ifndef NO_SCRIPTENGINE
	con_set< const_str, ConList > *m_NotifyList;
	con_set< const_str, ConList > *m_WaitForList;
	con_set< const_str, ConList > *m_EndList;
	ScriptVariableList *vars;
#endif

	static bool		EventSystemStarted;

private:
#ifndef NO_SCRIPTENGINE
	void			ExecuteScriptInternal( Event *ev, ScriptVariable& scriptVariable );
	void			ExecuteThreadInternal( Event *ev, ScriptVariable& returnValue );
	void			WaitExecuteScriptInternal( Event *ev, ScriptVariable& returnValue );
	void			WaitExecuteThreadInternal( Event *ev, ScriptVariable& returnValue );
#endif

	EventQueueNode	*PostEventInternal( Event *ev, float delay, int flags );

public:
	CLASS_PROTOTYPE( Listener );

#ifndef NO_SCRIPTENGINE
	/* Game functions */
	virtual ScriptThread *CreateThreadInternal( const ScriptVariable& label );
	virtual ScriptThread *CreateScriptInternal( const ScriptVariable& label );
	virtual void	StoppedNotify( void );
	virtual void	StartedWaitFor( void );
	virtual void	StoppedWaitFor( const_str name, bool bDeleting );
#endif

	virtual Listener *GetScriptOwner( void );

	Listener();
	virtual ~Listener();

	virtual void Archive( Archiver &arc );

	void					CancelEventsOfType( Event *ev );
	void					CancelEventsOfType( Event &ev );
	void					CancelFlaggedEvents( int flags );
	void					CancelPendingEvents( void );

	qboolean				EventPending( Event &ev );

	void					PostEvent( Event *ev, float delay, int flags = 0 );
	void					PostEvent( const Event &ev, float delay, int flags = 0 );

	qboolean				PostponeAllEvents( float time );
	qboolean				PostponeEvent( Event& ev, float time );

	bool					ProcessEvent( const Event &ev );
	bool					ProcessEvent( Event *ev );
	bool					ProcessEvent( Event &ev );
	ScriptVariable&			ProcessEventReturn( Event * ev );

	void					ProcessContainerEvent( const Container< Event * >& conev );

	qboolean				ProcessPendingEvents( void );

	bool					ProcessScriptEvent( Event &ev );
	bool					ProcessScriptEvent( Event *ev );

#ifndef NO_SCRIPTENGINE

	void					CreateVars( void );
	void					ClearVars( void );
	ScriptVariableList		*Vars( void );

	bool					BroadcastEvent( Event &event, ConList *listeners );
	bool					BroadcastEvent( str name, Event &event );
	bool					BroadcastEvent( const_str name, Event &event );
	void					CancelWaiting( str name );
	void					CancelWaiting( const_str name );
	void					CancelWaitingAll( void );
	void					CancelWaitingSources( const_str name, ConList &listeners, ConList &stoppedListeners );

	void					ExecuteThread( str scriptName, str label, Event *params = NULL );
	void					ExecuteThread( str scriptName, str label, Event &params );

	void					EndOn( str name, Listener *listener );
	void					EndOn( const_str name, Listener *listener );
	void					Notify( const char *name );
	void					Register( str name, Listener *listener );
	void					Register( const_str name, Listener *listener );
	void					RegisterSource( const_str name, Listener *listener );
	void					RegisterTarget( const_str name, Listener *listener );
	void					Unregister( str name );
	void					Unregister( const_str name );
	void					Unregister( str name, Listener *listener );
	void					Unregister( const_str name, Listener *listener );
	void					UnregisterAll( void );
	bool					UnregisterSource( const_str name, Listener *listener );
	bool					UnregisterTarget( const_str name, Listener *listener );
	void					UnregisterTargets( const_str name, ConList &listeners, ConList &stoppedListeners, Container< const_str > &stoppedNames );
	void					AbortRegistration( const_str name, Listener *l );

	int						RegisterSize( const_str name ) const;
	int						RegisterSize( str name ) const;
	int						WaitingSize( const_str name ) const;
	int						WaitingSize( str name ) const;

	bool					WaitTillDisabled( str s );
	bool					WaitTillDisabled( const_str s );
#endif

	int						GetFlags( Event *event ) const;
	qboolean				ValidEvent( str name ) const;

	//
	// Scripting functions
	//
	void					CommandDelay( Event *ev );
	void					EventDelete( Event *ev );
	void					EventInheritsFrom( Event *ev );
	void					EventIsInheritedBy( Event *ev );
	void					GetClassname( Event *ev );

#ifndef NO_SCRIPTENGINE
	void					CancelFor( Event *ev );
	void					CreateReturnThread( Event *ev );
	void					CreateThread( Event *ev );
	void					ExecuteReturnScript( Event *ev );
	void					ExecuteScript( Event *ev );
	void					EventDelayThrow( Event *ev );
	void					EventEndOn( Event *ev );
	void					EventGetOwner( Event *ev );
	void					EventNotify( Event *ev );
	void					EventThrow( Event *ev );
	void					EventUnregister( Event *ev );
	void					WaitCreateReturnThread( Event *ev );
	void					WaitCreateThread( Event *ev );
	void					WaitExecuteReturnScript( Event *ev );
	void					WaitExecuteScript( Event *ev );
	void					WaitTill( Event *ev );
	void					WaitTillTimeout( Event *ev );
	void					WaitTillAny( Event *ev );
	void					WaitTillAnyTimeout( Event *ev );
#endif
};

qboolean IsNumeric( const char *str );

extern Event EV_DelayThrow;
extern Event EV_Delete;
extern Event EV_Remove;
extern Event EV_ScriptRemove;
extern Event EV_Throw;

extern Event EV_Listener_CreateThread;
extern Event EV_Listener_CreateReturnThread;
extern Event EV_Listener_ExecuteReturnScript;
extern Event EV_Listener_ExecuteScript;
extern Event EV_Listener_WaitCreateReturnThread;

extern int	DisableListenerNotify;

extern cvar_t	*g_showevents;
extern cvar_t	*g_eventlimit;
extern cvar_t	*g_timeevents;
extern cvar_t	*g_watch;
extern cvar_t	*g_eventstats;

extern MEM_BlockAlloc< Event, MEM_BLOCKSIZE > Event_allocator;
#ifdef GAME_DLL
extern MEM_BlockAlloc< ConsoleEvent, MEM_BLOCKSIZE > ConsoleEvent_allocator;
#endif

#if defined( GAME_DLL )
//
// game dll specific defines
//
#define EVENT_DebugPrintf gi.DebugPrintf
#define EVENT_DPrintf gi.DPrintf
#define EVENT_Printf gi.Printf
#define EVENT_time   level.time
#define EVENT_realtime gi.Milliseconds()
#define EVENT_Error gi.Error

#define EVENT_FILENAME "events.txt"

#elif defined ( CGAME_DLL )
//
// cgame dll specific defines
//
#define EVENT_DebugPrintf cgi.DebugPrintf
#define EVENT_DPrintf cgi.Printf
#define EVENT_Printf cgi.Printf
#ifndef CGAME_HOOK
#define EVENT_time   ( ( ( float )cg.time / 1000.0f ) )
#else
#define EVENT_time   ( ( ( float )cg->time / 1000.0f ) )
#endif
#define EVENT_realtime   cgi.Milliseconds()
#define EVENT_Error cgi.Error

#define EVENT_FILENAME "cg_events.txt"

#elif defined ( UI_LIB )

#define EVENT_DebugPrintf Com_DebugPrintf
#define EVENT_DPrintf Com_Printf
#define EVENT_Printf Com_Printf
#define EVENT_time   ( ( ( float )cls.realtime / 1000.0f ) )
#define EVENT_realtime   Sys_Milliseconds()
#define EVENT_Error Com_Error

#define EVENT_FILENAME "ui_events.txt"

#else
//
// client specific defines
//
#define EVENT_DebugPrintf Com_DebugPrintf
#define EVENT_DPrintf Com_Printf
#define EVENT_Printf Com_Printf
#define EVENT_time   ( ( ( float )cls.realtime / 1000.0f ) )
#define EVENT_realtime Sys_Milliseconds()
#define EVENT_Error Com_Error

#define EVENT_FILENAME "cl_events.txt"
#endif

void L_ClearEventList();
bool L_EventSystemStarted( void );
void L_InitEvents( void );
void L_ProcessPendingEvents();
void L_ShutdownEvents( void );
void L_ArchiveEvents( Archiver& arc );
void L_UnarchiveEvents( Archiver& arc );

#include "game.h"
#include "level.h"

#endif /* __LISTENER_H__ */
