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
// animate.cpp : Animate Class

#include "animate.h"
#include "../game/player.h"
#include "../qcommon/qfiles.h"
#include <tiki.h>

// Leg Animation events
Event EV_SetAnim
(
	"anim",
	0,
	"sIF",
	"animName slot weight",
	"Exec anim commands on server or client.",
	EV_NORMAL
);
Event EV_SetSyncTime
(
	"setsynctime",
	0,
	"f",
	"synctime",
	"Set sync time for entity."
);
Event EV_ViewAnim
(
	"viewanim",
	EV_DEFAULT,
	"s",
	"anim",
	"testing"
);
Event EV_Animate_IsLoopingAnim
(
	"isloopinganim",
	EV_DEFAULT,
	"s",
	"anim_name",
	"returns 1 if the anim is a looping anim, or 0 otherwise",
	EV_RETURN
);

MEM_BlockAlloc< AnimationEvent, MEM_BLOCKSIZE > AnimationEvent_allocator;

void *AnimationEvent::operator new( size_t size )
{
	return AnimationEvent_allocator.Alloc();
}

void AnimationEvent::operator delete( void *ptr )
{
	return AnimationEvent_allocator.Free( ptr );
}

CLASS_DECLARATION( Event, AnimationEvent, NULL )
{
	{ NULL, NULL }
};

Animate::Animate()
{
	entflags |= EF_ANIMATE;

	pauseSyncTime = 0.0f;
	syncTime = 0.0f;
	syncRate = 1.0f;

	frame_delta = vec_zero;


	if( !LoadingSavegame )
	{
		edict->s.actionWeight = 1.0f;
		for( int i = 0; i < 16; i++ )
		{
			edict->s.frameInfo[ i ].index = 0;
			edict->s.frameInfo[ i ].time = 0.0f;
			edict->s.frameInfo[ i ].weight = 0.0f;

			animFlags[ i ] = 57;
			doneEvents[ i ] = NULL;
			animtimes[ i ] = 0.0f;
			frametimes[ i ] = 0.0f;
		}

		flags |= FL_ANIMATE;
	}
}

Animate::~Animate()
{
	entflags &= ~EF_ANIMATE;
}

void Animate::Pause( int slot, int pause )
{
	if( pause )
	{
		animFlags[ slot ] |= ANIM_PAUSED;
	}
	else
	{
		if( ( animFlags[ slot ] & ANIM_PAUSED ) )
		{
			if( ( animFlags[ slot ] & ANIM_SYNC ) )
			{
				animFlags[ slot ] = ( animFlags[ slot ] | ANIM_NODELTA ) & ~ANIM_FINISHED;
			}

			animFlags[ slot ] &= ~ANIM_PAUSED;
		}
	}
}

void Animate::StopAnimating( int slot )
{
	DoExitCommands( slot );

	if( doneEvents[ slot ] )
	{
		delete doneEvents[ slot ];
		doneEvents[ slot ] = NULL;
	}

	CancelFlaggedEvents( 1 << slot );

	if( edict->s.frameInfo[ slot ].index || gi.TIKI_NumAnims( edict->tiki ) <= 1 )
	{
		edict->s.frameInfo[ slot ].index = 0;
	}
	else
	{
		edict->s.frameInfo[ slot ].index = 1;
	}

	edict->s.frameInfo[ slot ].weight = 0.0f;

	animFlags[ slot ] = ANIM_LOOP | ANIM_NODELTA | ANIM_NOEXIT | ANIM_PAUSED;
	animtimes[ slot ] = 0.0f;
}

void Animate::StopAnimatingAtEnd( int slot )
{
}

void Animate::DoExitCommands( int slot )
{
	tiki_cmd_t cmds;
	AnimationEvent *ev;

	if( animFlags[ slot ] & ANIM_NOEXIT ) {
		return;
	}

	// exit the previous animation
	if( gi.Frame_Commands( edict->tiki, edict->s.frameInfo[ slot ].index, TIKI_FRAME_EXIT, &cmds ) )
	{
		int ii, j;

		for( ii = 0; ii < cmds.num_cmds; ii++ )
		{
			ev = new AnimationEvent( cmds.cmds[ ii ].args[ 0 ] );

			ev->SetAnimationNumber( edict->s.frameInfo[ slot ].index );
			ev->SetAnimationFrame( 0 );

			for( j = 1; j < cmds.cmds[ ii ].num_args; j++ )
			{
				ev->AddToken( cmds.cmds[ ii ].args[ j ] );
			}

			PostEvent( ev, 0 );
		}
	}

	animFlags[ slot ] |= ANIM_NOEXIT;
}

void Animate::NewAnim( int animnum, int slot, float weight )
{
	qboolean			newanim = edict->s.frameInfo[ slot ].index != animnum;
	tiki_cmd_t			cmds;
	float				time;
	int					numframes;
	AnimationEvent		*ev;
	int					i;

	if( newanim )
	{
		DoExitCommands( slot );
	}

	if( doneEvents[ slot ] )
	{
		delete doneEvents[ slot ];
		doneEvents[ slot ] = NULL;
	}

	CancelFlaggedEvents( 1 << slot );

	edict->s.frameInfo[ slot ].index = animnum;

	animFlags[ slot ] = ANIM_LOOP | ANIM_NODELTA | ANIM_NOEXIT;

	if( !( gi.Anim_FlagsSkel( edict->tiki, animnum ) & ANIM_LOOP ) )
	{
		animFlags[ slot ] &= ~ANIM_LOOP;
	}

	edict->s.frameInfo[ slot ].weight = weight;

	animtimes[ slot ] = gi.Anim_Time( edict->tiki, animnum );
	frametimes[ slot ] = gi.Anim_Frametime( edict->tiki, animnum );

	if( edict->s.eType == ET_GENERAL )
	{
		edict->s.eType = ET_MODELANIM;
	}

	qboolean hascommands = gi.Anim_HasCommands( edict->tiki, animnum );

	// enter this animation
	if( newanim )
	{
		if( !hascommands ) {
			return;
		}

		if( gi.Frame_Commands( edict->tiki, animnum, TIKI_FRAME_ENTRY, &cmds ) )
		{
			int ii, j;

			for( ii = 0; ii < cmds.num_cmds; ii++ )
			{
				ev = new AnimationEvent( cmds.cmds[ ii ].args[ 0 ] );

				ev->SetAnimationNumber( animnum );
				ev->SetAnimationFrame( 0 );

				for( j = 1; j < cmds.cmds[ ii ].num_args; j++ )
				{
					ev->AddToken( cmds.cmds[ ii ].args[ j ] );
				}

				ProcessEvent( ev );
			}
		}
	}

	if( !hascommands )
	{
		return;
	}

	if( !edict->tiki )
	{
		return;
	}

	time = 0.0f;
	numframes = gi.Anim_NumFrames( edict->tiki, animnum );

	for( i = 0; i < numframes; i++, time += frametimes[ slot ] )
	{
		// we want normal frame commands to occur right on the frame
		if( gi.Frame_Commands( edict->tiki, animnum, i, &cmds ) )
		{
			int ii, j;

			for( ii = 0; ii < cmds.num_cmds; ii++ )
			{
				ev = new AnimationEvent( cmds.cmds[ ii ].args[ 0 ] );

				ev->SetAnimationNumber( animnum );
				ev->SetAnimationFrame( i );

				for( j = 1; j < cmds.cmds[ ii ].num_args; j++ )
				{
					ev->AddToken( cmds.cmds[ ii ].args[ j ] );
				}

				PostEvent( ev, time, 1 << slot );
			}
		}
	}
}

void Animate::NewAnim( int animnum, Event &newevent, int slot, float weight )
{
	if( animnum == -1 )
	{
		PostEvent( newevent, level.frametime );
	}

	NewAnim( animnum, slot );
	SetAnimDoneEvent( newevent, slot );
}

void Animate::NewAnim( int animnum, Event *newevent, int slot, float weight )
{
	if( animnum == -1 )
	{
		if( newevent )
		{
			PostEvent( newevent, level.frametime );
		}
	}

	NewAnim( animnum, slot );
	SetAnimDoneEvent( newevent, slot );
}

void Animate::NewAnim( const char *animname, int slot, float weight )
{
	int animnum = gi.Anim_Random( edict->tiki, animname );

	if( animnum != -1 )
	{
		NewAnim( animnum, slot );
	}
}

void Animate::NewAnim( const char *animname, Event *endevent, int slot, float weight )
{
	int animnum = gi.Anim_Random( edict->tiki, animname );

	if( animnum != -1 )
	{
		NewAnim( animnum, endevent, slot );
	}
}

void Animate::NewAnim( const char *animname, Event &endevent, int slot, float weight )
{
	int animnum = gi.Anim_Random( edict->tiki, animname );

	if( animnum != -1 )
	{
		NewAnim( animnum, endevent, slot );
	}
}

void Animate::FrameDeltaEvent( Event *ev )
{

}

void Animate::SetAnimDoneEvent( Event *event, int slot )
{
	if( doneEvents[ slot ] )
	{
		delete doneEvents[ slot ];
	}

	doneEvents[ slot ] = event;
}

void Animate::SetAnimDoneEvent( Event &event, int slot )
{
	SetAnimDoneEvent( new Event( event ), slot );
}

void Animate::SetFrame( void )
{
	edict->s.frameInfo[ 0 ].time = 0;
	animFlags[ 0 ] = ( animFlags[ 0 ] | ANIM_NODELTA ) & ~ANIM_FINISHED;
}

qboolean Animate::HasAnim( const char *animname )
{
	int num;

	num = gi.Anim_Random( edict->tiki, animname );
	return ( num >= 0 );
}

void Animate::AnimFinished( int slot )
{
	animFlags[ slot ] &= ~ANIM_FINISHED;

	if( doneEvents[ slot ] )
	{
		Event *ev = doneEvents[ slot ];
		doneEvents[ slot ] = NULL;

		ProcessEvent( ev );
	}
}

float Animate::GetTime( int slot )
{
	return edict->s.frameInfo[ slot ].time;
}

void Animate::SetNormalTime( int slot, float normal )
{
	if( normal < 0.0f || normal > 1.0f )
	{
		Com_Printf( "ERROR:  Animate::SetNormalTime: Normal must be between 0 and 1\n" );
	}
	else
	{
		edict->s.frameInfo[ slot ].time = animtimes[ slot ] * normal;
	}
}

float Animate::GetNormalTime( int slot )
{
	return edict->s.frameInfo[ slot ].time / animtimes[ slot ];
}

void Animate::SetWeight( int slot, float weight )
{
	edict->s.frameInfo[ slot ].weight = weight;
}

float Animate::GetWeight( int slot )
{
	return edict->s.frameInfo[ slot ].weight;
}

void Animate::SetRepeatType( int slot )
{
	animFlags[ slot ] |= ANIM_LOOP;
}

void Animate::SetOnceType( int slot )
{
	animFlags[ slot ] &= ~ANIM_LOOP;
}

void Animate::SetTime( int slot, float time )
{
	if( time < 0.0 )
	{
		Com_Printf( "ERROR:  SetTime %f lesser than anim length %f\n", time, animtimes[ slot ] );
		return;
	}

	if( time > animtimes[ slot ] )
	{
		Com_Printf( "ERROR:  SetTime %f greater than anim length %f\n", time, animtimes[ slot ] );
		return;
	}

	edict->s.frameInfo[ slot ].time = time;
}

void Animate::SetSyncRate( float rate )
{
	if( rate < 0.001f )
	{
		Com_Printf( "ERROR SetSyncRate:  canot set syncrate below 0.001.\n" );
		syncRate = 0.001f;
	}
	else
	{
		syncRate = rate;
	}
}

void Animate::SetSyncTime( float s )
{
	if( s < 0.0f || s > 1.0f )
	{
		Com_Printf( "\nERROR SetSyncTime:  synctime must be 0 to 1 - attempt to set to %f\n" );
		return;
	}

	syncTime = s;

	for( int i = 0; i < MAX_FRAMEINFOS; i++ )
	{
		if( !( animFlags[ i ] & ANIM_SYNC ) ) {
			continue;
		}

		animFlags[ i ] = ( animFlags[ i ] | ANIM_NODELTA ) & ANIM_FINISHED;
	}
}

void Animate::UseSyncTime(int slot, int sync)
{
	if (sync)
	{
		if (animFlags[slot] & ANIM_SYNC)
			return;
		
		animFlags[slot] = (animFlags[slot] | (ANIM_SYNC | ANIM_NODELTA)) & ~ANIM_FINISHED;
	}
	else
	{
		if (animFlags[slot] & ANIM_SYNC)
		{
			animFlags[slot] = (animFlags[slot] | ANIM_NODELTA) & ~(ANIM_FINISHED | ANIM_SYNC);
		}
	}
}

void Animate::PreAnimate( void )
{
	int i;

	for( i = 0; i < MAX_FRAMEINFOS; i++ )
	{
		if( animFlags[ i ] & ANIM_FINISHED ) {
			AnimFinished( i );
		}
	}
}

void Animate::PostAnimate( void )
{
	if( !edict->tiki ) {
		return;
	}

	float		startTime;
	float		deltaSyncTime;
	float		total_weight;
	Vector		vFrameDelta;
	bool		hasAction = false;

	deltaSyncTime = syncTime;

	if( !pauseSyncTime )
	{
		syncTime = 1.0f / syncRate * level.frametime + deltaSyncTime;
	}

	total_weight = 0.0f;

	for( int i = 0; i < MAX_FRAMEINFOS; i++ )
	{
		if( edict->s.frameInfo[ i ].weight > 0.0f )
		{
			if( !( animFlags[ i ] & ANIM_NOACTION ) )
			{
				hasAction = true;
			}
		}

		if( animFlags[ i ] & ANIM_PAUSED ) {
			continue;
		}

		if( animFlags[ i ] & ANIM_SYNC )
		{
			startTime = deltaSyncTime * animtimes[ i ];
			edict->s.frameInfo[ i ].time = animtimes[ i ] * syncTime;
		}
		else
		{
			startTime = edict->s.frameInfo[ i ].time;
			edict->s.frameInfo[ i ].time += level.frametime;
		}

		if( animtimes[ i ] == 0.0f )
		{
			animFlags[ i ] = animFlags[ i ] & ~ANIM_NODELTA | ANIM_FINISHED;
			edict->s.frameInfo[ i ].time = 0.0f;
		}
		else
		{
			if( !( animFlags[ i ] & ANIM_NODELTA ) )
			{
				if( gi.Anim_HasDelta( edict->tiki, edict->s.frameInfo[ i ].index ) )
				{
					float vDelta[ 3 ];

					gi.Anim_DeltaOverTime( edict->tiki,
						edict->s.frameInfo[ i ].index,
						startTime,
						edict->s.frameInfo[ i ].time,
						vDelta );

					vFrameDelta += Vector( vDelta ) * edict->s.frameInfo[ i ].weight;

					total_weight += edict->s.frameInfo[ i ].weight;
				}
			}

			animFlags[ i ] &= ~ANIM_NODELTA;

			bool bTemp;
			if (animFlags[i] & ANIM_SYNC)
			{
				bTemp = edict->s.frameInfo[i].time < animtimes[i];
			}
			else
			{
				bTemp = edict->s.frameInfo[i].time < animtimes[i] - 0.01f;
			}
			if( !bTemp )
			{
				if (animFlags[i] & ANIM_LOOP)
				{
					animFlags[i] |= ANIM_FINISHED;

					do 
					{
						edict->s.frameInfo[i].time -= animtimes[i];
					} while (edict->s.frameInfo[i].time >= animtimes[i]);

					if (edict->s.frameInfo[i].time < 0)
					{
						edict->s.frameInfo[i].time = 0;
					}

				}
				else
				{
					if (startTime != animtimes[i])
					{
						animFlags[i] |= ANIM_FINISHED;
					}

					edict->s.frameInfo[i].time = animtimes[i];
				}

			}
		}

		if( total_weight != 0.0f )
			vFrameDelta *= 1.0f / total_weight;

		MatrixTransformVector( vFrameDelta, orientation, frame_delta );

		while( syncTime > 1.0f )
		{
			syncTime -= 1.0f;
		}

		total_weight = level.frametime * 4.0f;

		if( hasAction )
		{
			edict->s.actionWeight += total_weight;

			if( edict->s.actionWeight > 1.0f )
			{
				edict->s.actionWeight = 1.0f;
			}
		}
		else
		{
			edict->s.actionWeight -= total_weight;

			if( edict->s.actionWeight < 0.0f )
			{
				edict->s.actionWeight = 0.0f;
			}
		}
	}
}

void Animate::DumpAnimInfo
	(
	void
	)

{
	MPrintf( "----------------------------------------\n" );

	for( int i = 0; i < MAX_FRAMEINFOS; i++ )
	{
		if( edict->s.frameInfo[ i ].weight <= 0.0f ) {
			continue;
		}

		str animname = gi.Anim_NameForNum( edict->tiki, CurrentAnim( i ) );

		MPrintf( "slot: %d  anim: %s weight: %f  time: %f  len: %f\n",
			i, animname.c_str(), edict->s.frameInfo[ i ].weight, edict->s.frameInfo[ i ].time, animtimes[ i ] );
	}

	MPrintf( "actionWeight: %f\n", edict->s.actionWeight );
}

void Animate::ForwardExec
	(
	Event *ev
	)

{
	int slot = 0;
	float weight = 1.0f;

	if( !edict->tiki )
	{
		ScriptError( "trying to play animation on( entnum: %d, targetname : '%s', classname : '%s' ) which does not have a model",
			entnum,
			targetname.c_str(),
			getClassname()
			);
	}

	if( ev->NumArgs() > 1 )
	{
		slot = ev->GetInteger( 2 );
	}

	if( ev->NumArgs() > 2 )
	{
		weight = ev->GetFloat( 3 );
	}

	NewAnim( ev->GetString( 1 ), slot, weight );
	SetTime( slot );
}


void Animate::EventSetSyncTime
	(
	Event *ev
	)

{
	SetSyncTime( ev->GetFloat( 1 ) );
}

void Animate::EventIsLoopingAnim
	(
	Event *ev
	)

{
	str anim_name = ev->GetString( 1 );
	int animnum;

	if( !edict->tiki )
	{
		ScriptError( "^~^~^ no tiki set" );
	}

	animnum = gi.Anim_NumForName( edict->tiki, anim_name.c_str() );
	if( animnum < 0 )
	{
		ScriptError( "anim '%s' not found, so can't tell if it is looping", anim_name.c_str() );
	}

	if( gi.Anim_FlagsSkel( edict->tiki, animnum ) & ANIM_LOOP )
	{
		ev->AddInteger( 1 );
	}
	else
	{
		ev->AddInteger( 0 );
	}
}

void Animate::StopAnimating( Event *ev )
{
	StopAnimating( ev->GetInteger( 1 ) );
}

CLASS_DECLARATION( Entity, Animate, "animate" )
{
	{ &EV_SetControllerAngles,			&Animate::SetControllerAngles },
	{ &EV_SetAnim,						&Animate::ForwardExec },
	{ &EV_SetSyncTime,					&Animate::EventSetSyncTime },
	{ &EV_Animate_IsLoopingAnim,		&Animate::EventIsLoopingAnim },
	{ NULL, NULL }
};
