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
// animate.h -- Animate class

#ifndef __ANIMATE_H__
#define __ANIMATE_H__

#if defined ( GAME_DLL )
#include "entity.h"
#elif defined ( CGAME_DLL )
#include "script/canimate.h"
#endif

#include "archive.h"

extern Event EV_SetAnim;
extern Event EV_SetSyncTime;

#define ANIM_PAUSED				1
#define ANIM_SYNC				2
#define ANIM_FINISHED			4
#define ANIM_NOEXIT				8
#define ANIM_NODELTA			16
#define ANIM_LOOP				32
#define ANIM_NOACTION			64

#define MINIMUM_DELTA_MOVEMENT 8
#define MINIMUM_DELTA_MOVEMENT_PER_FRAME ( MINIMUM_DELTA_MOVEMENT / 20.0f )

#define FLAGGED_ANIMATE_SLOT		20

class Animate;

typedef SafePtr<Animate> AnimatePtr;

class AnimationEvent : public Event
{
private:
	short	anim_number;
	short	anim_frame;

public:
	CLASS_PROTOTYPE( AnimationEvent );

	void *operator new( size_t size );
	void operator delete( void *ptr );

	AnimationEvent( str name ) : Event( name ) { anim_number = 0; anim_frame = 0; }
	AnimationEvent( const Event& ev ) : Event( ev ) { anim_number = 0; anim_frame = 0; }
	AnimationEvent() {}

	void			SetAnimationFrame( int frame ) { anim_frame = frame; };
	void			SetAnimationNumber( int num ) { anim_number = num; };
};

class Animate : public Entity
{
protected:
	int			animFlags[ MAX_FRAMEINFOS ];

	float		syncTime;
	float		syncRate;
	int			pauseSyncTime;

	Event		*doneEvents[ MAX_FRAMEINFOS ];

	float		animtimes[ MAX_FRAMEINFOS ];
	float		frametimes[ MAX_FRAMEINFOS ];

public:
	Vector		frame_delta;

public:
	CLASS_PROTOTYPE( Animate );

	Animate();
	~Animate();

	void				DoExitCommands( int slot = 0 );
	void				NewAnim( int animnum, int slot = 0, float weight = 1.0f );
	void				NewAnim( int animnum, Event *endevent, int slot = 0, float weight = 1.0f );
	void				NewAnim( int animnum, Event &endevent, int slot = 0, float weight = 1.0f );
	void				NewAnim( const char *animname, int slot = 0, float weight = 1.0f );
	void				NewAnim( const char *animname, Event *endevent, int slot = 0, float weight = 1.0f );
	void				NewAnim( const char *animname, Event &endevent, int slot = 0, float weight = 1.0f );
	void				SetFrame( void );
	qboolean			HasAnim( const char *animname );
	Event				*AnimDoneEvent( int slot = 0 );
	void				SetAnimDoneEvent( Event &event, int slot = 0 );
	void				SetAnimDoneEvent( Event *event, int slot = 0 );
	int					NumFrames( int slot = 0 );
	int					NumAnims( void );
	const char			*AnimName( int slot = 0 );
	float				AnimTime( int slot = 0 );
	virtual void AnimFinished( int slot = 0 );
	void				SetTime( int slot = 0, float time = 0.0f );
	void				SetNormalTime( int slot = 0, float normal = 1.0f );
	float				GetTime( int slot = 0 );
	float				GetNormalTime( int slot = 0 );
	void				SetWeight( int slot = 0, float weight = 1.0f );
	float				GetWeight( int slot = 0 );
	void				SetRepeatType( int slot = 0 );
	void				SetOnceType( int slot = 0 );
	void				SetSyncRate( float rate );
	void				SetSyncTime( float s );
	void				UseSyncTime(int slot, int sync);

	void				Pause( int slot = 0, int pause = 1 );
	void				StopAnimating( int slot = 0 );
	void				StopAnimatingAtEnd( int slot = 0 );
	
	virtual int CurrentAnim( int slot = 0 );
	virtual float CurrentTime( int slot = 0 );

	void PreAnimate( void ) override;
	void PostAnimate( void ) override;

	virtual void DumpAnimInfo( void );
	
	void Archive( Archiver &arc ) override;

	void				FrameDeltaEvent( Event *ev );
	void				ForwardExec( Event *ev );
	void				EventSetSyncTime( Event *ev );
	void				EventIsLoopingAnim( Event *ev );
	void				StopAnimating( Event *ev );
};

extern MEM_BlockAlloc<AnimationEvent> AnimationEvent_allocator;

inline int Animate::CurrentAnim( int slot )
{
	return edict->s.frameInfo[ slot ].index;
}

inline float Animate::CurrentTime( int slot )
{
	return edict->s.frameInfo[ slot ].time;
}

inline int Animate::NumFrames( int slot )
{
	return gi.Anim_NumFrames( edict->tiki, edict->s.frameInfo[ slot ].index );
}

inline float Animate::AnimTime( int slot )
{
	return animtimes[ slot ];
}

inline int Animate::NumAnims( void )
{
	return gi.TIKI_NumAnims( edict->tiki );
}

inline const char *Animate::AnimName( int slot )
{
	return gi.Anim_NameForNum( edict->tiki, edict->s.frameInfo[ slot ].index );
}

inline Event *Animate::AnimDoneEvent( int slot )
{
	return doneEvents[ slot ];
}

inline void Animate::Archive( Archiver &arc )
{
	Entity::Archive( arc );

	for( int i = MAX_FRAMEINFOS - 1; i >= 0; i-- )
	{
		arc.ArchiveInteger( &animFlags[ i ] );
	}
	arc.ArchiveFloat( &syncTime );
	arc.ArchiveFloat( &syncRate );
	arc.ArchiveInteger( &pauseSyncTime );
	for( int i = MAX_FRAMEINFOS - 1; i >= 0; i-- )
	{
		arc.ArchiveEventPointer( &doneEvents[ i ] );
	}
	for( int i = MAX_FRAMEINFOS - 1; i >= 0; i-- )
	{
		arc.ArchiveFloat( &animtimes[ i ] );
	}
	for( int i = MAX_FRAMEINFOS - 1; i >= 0; i-- )
	{
		arc.ArchiveFloat( &frametimes[ i ] );
	}
	arc.ArchiveVector( &frame_delta );
}

#endif /* animate.h */
