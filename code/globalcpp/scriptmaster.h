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

// scriptmaster.h: Handle events, parse scripts, spawn at the beginning of the map

#ifndef __SCRIPTMASTER_H__
#define __SCRIPTMASTER_H__

#include "class.h"
#include "listener.h"
#include "scriptvm.h"

#define MAX_COMMANDS			20
#define MAX_EXECUTION_TIME		3000

void Showmenu( str name, qboolean bForce );
void Hidemenu( str name, qboolean bForce );

class con_timer : public Class
{
public:
	class Element
	{
	public:
		Class		*obj;
		int			inttime;
	};

private:
	Container< con_timer::Element > m_Elements;
	bool	m_bDirty;
	int		m_inttime;

public:
	con_timer();

	void					AddElement( Class *e, int inttime );
	void					RemoveElement( Class *e );

	Class					*GetNextElement( int& foundTime );

	void					SetDirty( void ) { m_bDirty = true; };
	bool					IsDirty( void ) { return m_bDirty; };
	void					SetTime( int inttime ) { m_inttime = inttime; };

	static void				ArchiveElement( Archiver& arc, Element *e );
	virtual void			Archive( Archiver& arc );
};

#define MAX_VAR_STACK		1024
#define MAX_FASTEVENT		10

class ScriptMaster : public Listener
{
public:
	// VM recursions
	int stackCount;

	// Global stack (not needed anymore)
#if 0
	ScriptVariable	avar_Stack[ MAX_VAR_STACK+1 ];	// Global variables stack
	Event			fastEvent[ MAX_FASTEVENT+1 ];	// Event that will be executed

	ScriptVariable	*pTop;							// Top variable on the stack
#endif

	// Command variables
	unsigned int	cmdCount;		// cmd count
	int				cmdTime;		// Elapsed VM execution time
	int				maxTime;		// Maximum VM execution time

	// Thread variables
	SafePtr<ScriptThread> m_PreviousThread;					// parm.previousthread
	SafePtr<ScriptThread> m_CurrentThread;					// current running thread

	con_map< const_str, GameScript * > m_GameScripts;		// compiled gamescripts

	// Miscellaneous
	Container< str >			m_menus;		// Script menus
	con_timer					timerList;		// waiting threads list
	con_arrayset< str, str >	StringDict;		// const strings (improve performance)
	int							iPaused;		// num times paused

	// Context switch variables
	bool									m_bAllowContextSwitch;
	Container< SafePtr< ScriptThread > >	m_contextSwitches;		// Threads to be prepared for context switching

protected:
	static const char *ConstStrings[];

private:
	GameScript				*GetGameScriptInternal( str& filename );
	void					InitConstStrings( void );

public:
	CLASS_PROTOTYPE( ScriptMaster );

	virtual	~ScriptMaster();

	virtual void			Archive( Archiver &arc );
	void					ArchiveString( Archiver& arc, const_str& s );

	void					AddContextSwitch( ScriptThread *thread );

	const_str				AddString( const char *s );
	const_str				AddString( str& s );
	const_str				GetString( const char *s );
	const_str				GetString( str s );

	str&					GetString( const_str s );

	void					AddTiming( ScriptThread *thread, float time );
	void					RemoveTiming( ScriptThread *thread );

	void					AddMenu( str name );
	void					RemoveMenu( str name );
	void					LoadMenus( void );

	ScriptThread			*CreateScriptThread( GameScript *scr, Listener *self, const_str label );
	ScriptThread			*CreateScriptThread( GameScript *scr, Listener *self, str label );
	ScriptThread			*CreateScriptThread( ScriptClass *scriptClass, const_str label );
	ScriptThread			*CreateScriptThread( ScriptClass *scriptClass, str label );
	ScriptThread			*CreateScriptThread( ScriptClass *scriptClass, unsigned char *m_pCodePos );
	ScriptThread			*CreateThread( GameScript *scr, str label, Listener *self = NULL );
	ScriptThread			*CreateThread( str filename, str label, Listener *self = NULL );
	ScriptClass				*CurrentScriptClass( void );

	ScriptThread			*CurrentThread( void );
	ScriptThread			*PreviousThread( void );

	void					ExecuteThread( GameScript *scr, str label = "" );
	void					ExecuteThread( str filename, str label = "" );

	void					ExecuteThread( GameScript *scr, str label, Event &parms );
	void					ExecuteThread( str filename, str label, Event &parms );

	GameScript				*GetTempScript( const char *data );
	GameScript				*GetGameScript( str filename, qboolean recompile = false );
	GameScript				*GetGameScript( const_str filename, qboolean recompile = false );
	GameScript				*GetScript( str filename, qboolean recompile = false );
	GameScript				*GetScript( const_str filename, qboolean recompile = false );

	void					CloseGameScript();
	void					Reset( qboolean samemap = false );

	void					Init();
	void					ExecuteRunning();
	void					SetTime( int time );

	void					PrintStatus( void );
	void					PrintThread( int iThreadNum );
	//
	// Events
	//
	void					RegisterAliasInternal( Event *ev, bool bCache = false );
	void					RegisterAlias( Event *ev );
	void					RegisterAliasAndCache( Event *ev );
	void					Cache( Event *ev );
};

class ScriptThread : public Listener
{
	friend class Flag;
	friend class EndOn;
	friend class Listener;
	friend class ScriptMaster;

private:
	ScriptVM *m_ScriptVM;
	SafePtr< ScriptThread > m_WaitingContext;

private:
	void		ScriptExecuteInternal( ScriptVariable *data = NULL, int dataSize = 0 );

public:
	CLASS_PROTOTYPE( ScriptThread );

#ifndef _DEBUG_MEM
	void *operator new( size_t size );
	void operator delete( void *ptr );
#endif

	virtual void	Archive( Archiver &arc );
	void			ArchiveInternal( Archiver& arc );
	virtual void	StartedWaitFor( void );
	virtual void	StoppedNotify( void );
	virtual void	StoppedWaitFor( const_str name, bool bDeleting );

	ScriptThread();
	ScriptThread( ScriptClass *scriptClass, unsigned char *pCodePos );
	virtual ~ScriptThread();

	void			Execute( Event &ev );
	void			Execute( Event *ev = NULL );
	void			DelayExecute( Event& ev );
	void			DelayExecute( Event *ev = NULL );

	void			AllowContextSwitch( bool allow = true );

	ScriptClass		*GetScriptClass( void );
	int				GetThreadState( void );
	ScriptThread	*GetWaitingContext( void );
	void			SetWaitingContext( ScriptThread *thread );

	void			HandleContextSwitch( ScriptThread *childThread );

	void			Pause( void );
	void			ScriptExecute( ScriptVariable *data, int dataSize, ScriptVariable &returnValue );
	void			Stop( void );
	void			Wait( float time );

	void		CanSwitchTeams( Event *ev );
	void		CharToInt( Event *ev );
	void		Conprintf( Event *ev );
	void		CreateHUD( Event *ev );
	void		Earthquake( Event *ev );
	void		FadeSound( Event *ev );
	void		FileOpen( Event *ev );
	void		FileWrite( Event *ev );
	void		FileRead( Event *ev );
	void		FileClose( Event *ev );
	void		FileEof( Event *ev );
	void		FileSeek( Event *ev );
	void		FileTell( Event *ev );
	void		FileRewind( Event *ev );
	void		FilePutc( Event *ev );
	void		FilePuts( Event *ev );
	void		FileGetc( Event *ev );
	void		FileGets( Event *ev );
	void		FileError( Event *ev );
	void		FileFlush( Event *ev );
	void		FileExists( Event *ev );
	void		FileReadAll( Event *ev );
	void		FileSaveAll( Event *ev );
	void		FileRemove( Event *ev );
	void		FileRename( Event *ev );
	void		FileCopy( Event *ev );
	void		FileReadPak( Event *ev );
	void		FileList( Event *ev );
	void		FileNewDirectory( Event *ev );
	void		FileRemoveDirectory( Event *ev );
	void		FlagClear( Event *ev );
	void		FlagInit( Event *ev );
	void		FlagSet( Event *ev );
	void		FlagWait( Event *ev );
	void		Lock( Event *ev );
	void		UnLock( Event *ev );
	void		GetAreaEntities( Event *ev );
	void		GetArrayKeys( Event *ev );
	void		GetArrayValues( Event *ev );
	void		GetEntArray( Event *ev );
	void		GetPlayerNetname( Event *ev );
	void		GetPlayerIP( Event *ev );
	void		GetPlayerPing( Event *ev );
	void		GetPlayerClientNum( Event *ev );
	void		GetTime( Event *ev );
	void		GetDate( Event *ev );
	void		GetTimeZone( Event *ev );
	void		PregMatch( Event *ev );
	void		EventHudDraw3d( Event *ev );
	void		EventHudDrawTimer( Event *ev );
	void		EventHudDrawShader( Event *ev );
	void		EventHudDrawAlign( Event *ev );
	void		EventHudDrawRect( Event *ev );
	void		EventHudDrawVirtualSize( Event *ev );
	void		EventHudDrawColor( Event *ev );
	void		EventHudDrawAlpha( Event *ev );
	void		EventHudDrawString( Event *ev );
	void		EventHudDrawFont( Event *ev );
	void		EventIHudDraw3d( Event *ev );
	void		EventIHudDrawShader( Event *ev );
	void		EventIHudDrawAlign( Event *ev );
	void		EventIHudDrawRect( Event *ev );
	void		EventIHudDrawVirtualSize( Event *ev );
	void		EventIHudDrawColor( Event *ev );
	void		EventIHudDrawAlpha( Event *ev );
	void		EventIHudDrawString( Event *ev );
	void		EventIHudDrawFont( Event *ev );
	void		EventIHudDrawTimer( Event *ev );
	void		EventIsArray( Event *ev );
	void		EventIsDefined( Event *ev );
	void		EventIsOnGround( Event *ev );
	void		EventIsOutOfBounds( Event *ev );
	void		GetEntity( Event *ev );
	void		MathCos( Event *ev );
	void		MathSin( Event *ev );
	void		MathTan( Event *ev );
	void		MathACos( Event *ev );
	void		MathASin( Event *ev );
	void		MathATan( Event *ev );
	void		MathATan2( Event *ev );
	void		MathCosH( Event *ev );
	void		MathSinH( Event *ev );
	void		MathTanH( Event *ev );
	void		MathExp( Event *ev );
	void		MathFrexp( Event *ev );
	void		MathLdexp( Event *ev );
	void		MathLog( Event *ev );
	void		MathLog10( Event *ev );
	void		MathModf( Event *ev );
	void		MathPow( Event *ev );
	void		MathSqrt( Event *ev );
	void		MathCeil( Event *ev );
	void		MathFloor( Event *ev );
	void		MathFmod( Event *ev );
	void		StringBytesCopy( Event *ev );
	void		Md5File( Event *ev );
	void		Md5String( Event *ev );
	void		RegisterEvent( Event *ev );
	void		RestoreSound( Event *ev );
	void		RemoveArchivedClass( Event *ev );
	void		ServerStufftext( Event *ev );
	void		SetTimer( Event *ev );
	void		TeamGetScore( Event *ev );
	void		TeamSetScore( Event *ev );
	void		TeamSwitchDelay( Event *ev );
	void		TraceDetails( Event *ev );
	void		TypeOfVariable( Event *ev );
	void		UnregisterEvent( Event *ev );
	void		VisionGetNaked( Event *ev );
	void		VisionSetNaked( Event *ev );
	void		CancelWaiting( Event *ev );

	void		Abs( Event *ev );
	void		AddObjective( Event *ev );
	void		AddObjective( int index, int status, str text, Vector location );
	void		ClearObjectiveLocation( Event *ev );
	void		ClearObjectiveLocation( void );
	void		SetObjectiveLocation( Event *ev );
	void		SetObjectiveLocation( Vector vLocation );
	void		SetCurrentObjective( Event *ev );
	void		SetCurrentObjective( int iObjective );
	void		AllAIOff( Event *ev );
	void		AllAIOn( Event *ev );
	void		EventTeamWin( Event *ev );

	void		Angles_PointAt( Event *ev );
	void		Angles_ToForward( Event *ev );
	void		Angles_ToLeft( Event *ev );
	void		Angles_ToUp( Event *ev );

	void		Assert( Event *ev );
	void		Cache( Event *ev );

	void		CastBoolean( Event *ev );
	void		CastEntity( Event *ev );
	void		CastFloat( Event *ev );
	void		CastInt( Event *ev );
	void		CastString( Event *ev );

	void		CreateReturnThread( Event *ev );
	void		CreateThread( Event *ev );
	void		ExecuteReturnScript( Event *ev );
	void		ExecuteScript( Event *ev );

	void		EventCreateListener( Event *ev );
	void		EventDelayThrow( Event *ev );
	void		EventEnd( Event *ev );
	void		EventTimeout( Event *ev );
	void		EventError( Event *ev );
	void		EventGoto( Event *ev );
	void		EventRegisterCommand( Event *ev );
	void		EventGetCvar( Event *ev );
	void		EventSetCvar( Event *ev );
	void		EventSightTrace( Event *ev );
	void		EventTrace( Event *ev );
	void		EventThrow( Event *ev );
	void		EventWait( Event *ev );
	void		EventWaitFrame( Event *ev );

	void		EventIsAlive( Event *ev );

	void		EventEarthquake( Event *ev );
	void		MapEvent( Event *ev );
	void		CueCamera( Event *ev );
	void		CuePlayer( Event *ev );

	void		FreezePlayer( Event *ev );
	void		ReleasePlayer( Event *ev );
	void		EventDrawHud( Event *ev );

	void		EventRadiusDamage( Event *ev );

	void		GetSelf( Event *ev );

	void		IPrintln( Event *ev );
	void		IPrintln_NoLoc( Event *ev );
	void		IPrintlnBold( Event *ev );
	void		IPrintlnBold_NoLoc( Event *ev );
	void		Println( Event *ev );
	void		Print( Event *ev );
	void		MPrintln( Event *ev );
	void		MPrint( Event *ev );
	void		EventPrint3D( Event *ev );

	void		EventBspTransition( Event *ev );
	void		EventLevelTransition( Event *ev );
	void		EventMissionTransition( Event *ev );

	void		EventGetBoundKey1( Event *ev );
	void		EventGetBoundKey2( Event *ev );
	void		EventLocConvertString( Event *ev );

	void		RandomFloat( Event *ev );
	void		RandomInt( Event *ev );

	void		Spawn( Event *ev );
	Listener	*SpawnInternal( Event *ev );
	void		SpawnReturn( Event *ev );

	void		ForceMusicEvent( Event *ev );
	void		SoundtrackEvent( Event *ev );
	void		RestoreSoundtrackEvent( Event *ev );

	void		EventVectorAdd( Event *ev );
	void		EventVectorCloser( Event *ev );
	void		EventVectorCross( Event *ev );
	void		EventVectorDot( Event *ev );
	void		EventVectorLength( Event *ev );
	void		EventVectorNormalize( Event *ev );
	void		EventVectorScale( Event *ev );
	void		EventVectorSubtract( Event *ev );
	void		EventVectorToAngles( Event *ev );
	void		EventVectorWithin( Event *ev );


	void		FadeIn( Event *ev );
	void		FadeOut( Event *ev );
	void		ClearFade( Event *ev );
	void		Letterbox( Event *ev );
	void		ClearLetterbox( Event *ev );
	void		MusicEvent( Event *ev );
	void		MusicVolumeEvent( Event *ev );
	void		RestoreMusicVolumeEvent( Event *ev );
	void		SetCinematic( Event *ev );
	void		SetNonCinematic( Event *ev );
	void		StuffCommand( Event *ev );
	void		KillEnt( Event *ev );
	void		RemoveEnt( Event *ev );
	void		KillClass( Event *ev );
	void		RemoveClass( Event *ev );
	void		CameraCommand( Event *ev );
	void		SetLightStyle( Event *ev );
	void		CenterPrint( Event *ev );
	void		LocationPrint( Event *ev );
	void		TriggerEvent( Event *ev );
	void		ServerEvent( Event *ev );
	void		MissionFailed( Event *ev );
};

extern qboolean disable_team_change;
extern qboolean disable_team_spectate;
extern str vision_current;

extern Event EV_ScriptThread_Trace;
extern Event EV_ScriptThread_SightTrace;
extern Event EV_ScriptThread_VisionSetNaked;
extern Event EV_ScriptThread_CancelWaiting;

extern con_set< str, ScriptThreadLabel > m_scriptCmds;
extern ScriptMaster Director;

class LightStyleClass : public Class
{
private:
	CLASS_PROTOTYPE( LightStyleClass );

	str            styles[ MAX_LIGHTSTYLES ];

public:

	void              SetLightStyle( int index, str style );
	void              Archive( Archiver &arc );
};

extern LightStyleClass lightStyles;

typedef enum scriptedEvType_e
{
	SE_DEFAULT,
	SE_CONNECTED,
	SE_DISCONNECTED,
	SE_SPAWN,
	SE_DAMAGE,
	SE_KILL,
	SE_KEYPRESS,
	SE_INTERMISSION,
	SE_SERVERCOMMAND,
	SE_CHANGETEAM,
	SE_MAX

} scriptedEvType_t;

class ScriptEvent : public Class
{
public:
	ScriptThreadLabel	label;

public:
	CLASS_PROTOTYPE( ScriptEvent );

	virtual void Archive( Archiver& arc );

	bool		IsRegistered( void );
	void		Trigger( Event *ev );
};

inline void ScriptEvent::Archive
	(
	Archiver& arc
	)

{
	label.Archive( arc );
}

inline bool ScriptEvent::IsRegistered
	(
	void
	)

{
	return label.IsSet();
}

inline void ScriptEvent::Trigger
	(
	Event *ev
	)

{
	if( label.IsSet() )
	{
		label.Execute( NULL, ev );
	}

	delete ev;
}

extern ScriptEvent scriptedEvents[];

typedef struct mutex_thread_list_s {
	SafePtr< ScriptThread >			m_pThread;
	struct mutex_thread_list_s		*next;
	struct mutex_thread_list_s		*prev;
} mutex_thread_list_t;

class ScriptMutex : public Listener {
public:
	SafePtr< ScriptThread >		m_pLockThread;
	int							m_iLockCount;
	mutex_thread_list_t			m_list;

private:
	void			setOwner( ScriptThread *pThread );
	void			Lock( mutex_thread_list_t *pList );

public:
	CLASS_PROTOTYPE( ScriptMutex );

	ScriptMutex();
	~ScriptMutex();

	virtual void	StoppedNotify( void );

	void			Lock( void );
	void			Unlock( void );
};


class Flag
{
public:
	char flagName[MAX_QPATH];
	qboolean bSignaled;

private:
	Container< ScriptVM * > m_WaitList;

public:
	Flag();
	~Flag();

	void Reset(void);
	void Set(void);
	void Wait(ScriptThread *Thread);
};

class FlagList
{
	friend class Flag;

private:
	void AddFlag(Flag *flag);
	void RemoveFlag(Flag *flag);

public:
	Container< Flag * > m_Flags;

	Flag *FindFlag(const char * name);
};

extern FlagList flags;

#endif /* scriptmaster.h */
