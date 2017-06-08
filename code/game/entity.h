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

// entity.h: Base class for all enities that are controlled by Sin.  If you have any
// object that should be called on a periodic basis and it is not an entity,
// then you have to have an dummy entity that calls it.
//
// An entity in Sin is any object that is not part of the world->  Any non-world
// object that is visible in Sin is an entity, although it is not required that
// all entities be visible to the player.  Some objects are basically just virtual
// constructs that act as an instigator of certain actions, for example, some
// triggers are invisible and cannot be touched, but when activated by other
// objects can cause things to happen.
//
// All entities are capable of receiving messages from Sin or from other entities.
// Messages received by an entity may be ignored, passed on to their superclass,
// or acted upon by the entity itself.  The programmer must decide on the proper
// action for the entity to take to any message.  There will be many messages
// that are completely irrelevant to an entity and should be ignored.  Some messages
// may require certain states to exist and if they are received by an entity when
// it these states don't exist may indicate a logic error on the part of the
// programmer or map designer and should be reported as warnings (if the problem is
// not severe enough for the game to be halted) or as errors (if the problem should
// not be ignored at any cost).
//

#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "g_local.h"
#include "class.h"
#include "vector.h"
#include "script.h"
#include "listener.h"
#include "simpleentity.h"

// modification flags
#define FLAG_IGNORE  0
#define FLAG_CLEAR   1
#define FLAG_ADD     2

typedef enum
	{
	DAMAGE_NO,
	DAMAGE_YES,			// will take damage if hit
	DAMAGE_AIM			// auto targeting recognizes this
	} damage_t;

typedef enum
{
	legs,
	torso,
} bodypart_t;

//deadflag
#define DEAD_NO						0
#define DEAD_DYING					1
#define DEAD_DEAD						2
#define DEAD_RESPAWNABLE			3

// Generic entity events
extern Event EV_SoundDone;
extern Event EV_Classname;
extern Event EV_Activate;
extern Event EV_ForceActivate;
extern Event EV_Use;
extern Event EV_FadeNoRemove;
extern Event EV_FadeOut;
extern Event EV_FadeIn;
extern Event EV_Fade;
extern Event EV_Killed;
extern Event EV_GotKill;
extern Event EV_Pain;
extern Event EV_Damage;
extern Event EV_Stun;
extern Event EV_Gib;
extern Event EV_Kill;
extern Event EV_DeathSinkStart;
extern Event EV_Entity_AddImmunity;
extern Event EV_Entity_RemoveImmunity;

// Physics events
extern Event EV_MoveDone;
extern Event EV_Touch;
extern Event EV_Blocked;
extern Event EV_Attach;
extern Event EV_AttachModel;
extern Event EV_RemoveAttachedModel;
extern Event EV_Detach;
extern Event EV_DetachAllChildren;
extern Event EV_UseBoundingBox;
extern Event EV_IsTouching;

// Animation events
extern Event EV_NewAnim;
extern Event EV_LastFrame;
extern Event EV_TakeDamage;
extern Event EV_NoDamage;

// script stuff
extern Event EV_Model;
extern Event EV_Hide;
extern Event EV_Show;
extern Event EV_BecomeSolid;
extern Event EV_BecomeNonSolid;
extern Event EV_Sound;
extern Event EV_StopSound;
extern Event EV_Bind;
extern Event EV_Unbind;
extern Event EV_Glue;
extern Event EV_Unglue;
extern Event EV_JoinTeam;
extern Event EV_QuitTeam;
extern Event EV_SetHealth;
extern Event EV_SetSize;
extern Event EV_SetAlpha;
extern Event EV_SetTargetName;
extern Event EV_SetTarget;
extern Event EV_SetKillTarget;
extern Event EV_StartAnimating;
extern Event EV_SurfaceModelEvent;
extern Event EV_Stop;
extern Event EV_StopLoopSound;
extern Event EV_SetControllerAngles;
extern Event EV_CanSee;

// dir is 1
// power is 2
// minsize is 3
// maxsize is 4
// percentage is 5
// thickness 6
// entity is 7
// origin 8

// AI sound events
extern Event EV_BroadcastAIEvent;
extern Event EV_Hurt;
extern Event EV_Heal;


// Define ScriptMaster
class ScriptMaster;

//
// Spawn args
//
// "spawnflags"
// "alpha" default 1.0
// "model"
// "origin"
// "targetname"
// "target"
//

#define MAX_MODEL_CHILDREN 8
#define MAX_GLUE_CHILDREN 8

#define GL_USEANGLES	1

class Entity;

typedef SafePtr<Entity> EntityPtr;

class Entity : public SimpleEntity
{
public:
	CLASS_PROTOTYPE( Entity );

	// spawning variables
	int			entnum;
	int			radnum;
	gentity_t	*edict;
	gclient_t	*client;
	int			spawnflags;

	// standard variables
	str model;

	// physics variables
	Vector		mins;
	Vector		maxs;
	Vector		absmin;
	Vector		absmax;
	Vector		velocity;
	Vector		accel;
	Vector		avelocity;
	Vector		aaccel;
	Vector		size;
	int			movetype;
	int			mass;
	float		gravity;
	float		orientation[ 3 ][ 3 ];
	gentity_t	*groundentity;
	cplane_t	groundplane;
	int			groundcontents;

	// Model Binding variables
	int numchildren;
	int	children[ MAX_MODEL_CHILDREN ];

	// Light variables
	float lightRadius;

	// Team variables
	str				moveteam;
	class Entity	*teamchain;
	class Entity	*teammaster;

	// Binding variables
	class Entity	*bindmaster;
	qboolean		bind_use_my_angles;
	Vector			localorigin;
	Vector			localangles;

	// targeting variables
	str killtarget;

	// Character state
	float	health;
	float	max_health;
	int		deadflag;
	int		flags;

	// underwater variables
	int	watertype;
	int waterlevel;

	// Pain and damage variables
	damage_t	takedamage;
	EntityPtr	enemy;
	float		pain_finished;
	float		damage_debounce_time;
	int			damage_type;

	// Glue variables
	int				m_iNumGlues;
	EntityPtr		m_pGlues[ MAX_GLUE_CHILDREN ];
	int				m_pGluesFlags[ MAX_GLUE_CHILDREN ];
	class Entity	*m_pGlueMaster;
	bool			m_bGlueAngles;
	qboolean		detach_at_death;

	// Path variables
	float				stealthMovementScale; // how much it will notify AIs
	class pathway_ref	*m_BlockedPaths;
	int					 m_iNumBlockedPaths;

	// immune list
	Container<int>		immunities;

	// miscellaneous
	qboolean			m_bBindChilds;
	bool m_bHintRequiresLookAt;
	str m_HintString;
	
	Entity();
	virtual				~Entity();

	void              SetEntNum( int num );
	void              ClassnameEvent( Event *ev );
	void              SpawnFlagsEvent( Event *ev );

	qboolean			CheckEventFlags( Event *event );

	qboolean				DistanceTo( Vector pos );
	qboolean				DistanceTo( Entity *ent );
	qboolean				WithinDistance( Vector pos, float dist );
	qboolean				WithinDistance( Entity *ent, float dist );

	qboolean				Targeted( void );
	void              SetKillTarget( const char *killtarget );
	const char        *KillTarget( void );

	virtual void			setLocalOrigin( Vector org );

	virtual void			setModel( const char *model );
	void					setModel( str &mdl );
	qboolean				setModel( void );
	void					SetModelEvent( Event *ev );
	void					GetBrushModelEvent( Event *ev );
	void					GetModelEvent( Event *ev );
	void              SetTeamEvent( Event *ev );
	virtual void      TriggerEvent( Event *ev );
	void					hideModel( void );
	void					EventHideModel( Event *ev );
	virtual void		showModel( void );
	void					EventShowModel( Event *ev );
	qboolean				hidden( void );
	void              ProcessInitCommands( void );

	void					setAlpha( float alpha );
	float					alpha( void );

	void					setMoveType( int type );
	int					getMoveType( void );

	void					setSolidType( solid_t type );
	int					getSolidType( void );

	virtual void		setContentsSolid( void );

	virtual Vector		getParentVector( Vector vec );
	Vector				getLocalVector( Vector vec );

	virtual void		setSize( Vector min, Vector max );
	virtual void		setOrigin( Vector org );
	virtual void		setOriginEvent( Vector org );
	virtual void		setOrigin( void );
	virtual void		addOrigin( Vector org );
	virtual void		updateOrigin( void );

	void              GetRawTag( int tagnum, orientation_t * orient );
	qboolean          GetRawTag( const char * tagname, orientation_t * orient );

	void				GetTag( int tagnum, orientation_t * orient );
	qboolean			GetTag( const char *name, orientation_t * orient );
	void  				GetTag( int tagnum, Vector *pos, Vector *forward = NULL, Vector *left = NULL, Vector *up = NULL );
	qboolean			GetTag( const char *name, Vector *pos, Vector *forward = NULL, Vector *left = NULL, Vector *up = NULL );
	qboolean			GetTagPositionAndOrientation( str tagname, orientation_t *new_or );
	void				GetTagPositionAndOrientation( int tagnum, orientation_t *new_or );

	void				GetTagAngles( Event *ev );
	void				GetTagPosition( Event *ev );

	virtual int			CurrentFrame( bodypart_t part = legs );
	virtual int			CurrentAnim( bodypart_t part = legs );

	virtual void		setAngles( Vector ang );
	virtual void		setAngles( void );
	void				SetOrigin( Event *ev );

	virtual void		ShowInfo( float fDot, float fDist );

	void				GetControllerAngles( Event *ev );
	Vector				GetControllerAngles( int num );
	void				SetControllerAngles( int num, vec3_t angles );
	void				SetControllerAngles( Event *ev );
	void				SetControllerTag( int num, int tag_num );

	void					link( void );
	void					unlink( void );

	void					setContents( int type );
	int					getContents( void );
	void              setScale( float scale );

	qboolean				droptofloor( float maxfall );
	qboolean				isClient( void );

	virtual void		SetDeltaAngles( void );
	virtual void		DamageEvent( Event *event );
	void              Damage( Entity *inflictor,
		Entity *attacker,
		float damage,
		Vector position,
		Vector direction,
		Vector normal,
		int knockback,
		int flags,
		int meansofdeath,
		int location = -1 );
	void              Stun( float time );

	void              DamageType( Event *ev );
	virtual qboolean	CanDamage( Entity *target, Entity *skip_ent = NULL );

	virtual void		AddImmunity( Event *ev );
	virtual void		RemoveImmunity( Event *ev );
	qboolean			Immune( int meansofdeath );

	virtual void		Delete( void );
	void				Remove( Event *ev );
	void				EventSoundDone( Event *ev );

	void				VolumeDamage( float damage );
	void				EventVolumeDamage( Event *ev );
	void				EventInPVS( Event *ev );

	qboolean				FovCheck( float *delta, float fovdot );
	virtual bool			CanSee( Entity *ent, float fov, float vision_distance );
	void					CanSee( Event *ev );

	virtual qboolean		IsDead();
	qboolean				IsTouching( Entity *e1 );
	void					IsTouching( Event *ev );

	void					GetVelocity( Event *ev );
	void					GetAVelocity( Event *ev );
	void					SetVelocity( Event *ev );

	void					FadeNoRemove( Event *ev );
	void					FadeOut( Event *ev );
	void					FadeIn( Event *ev );
	void					Fade( Event *ev );

	virtual void		CheckGround( void );
	virtual qboolean	HitSky( trace_t *trace );
	virtual qboolean	HitSky( void );

	void					BecomeSolid( Event *ev );
	void					BecomeNonSolid( Event *ev );
	void					EventSetHealthOnly( Event *ev );
	void					EventSetMaxHealth( Event *ev );
	void					EventGetMaxHealth( Event *ev );
	void					GetHealth( Event *ev );
	void					SetHealth( Event *ev );
	void					SetSize( Event *ev );
	void					SetSize( void );
	void					SetMins( Event *ev );
	void					SetMaxs( Event *ev );
	void					SetScale( Event *ev );
	void					GetScale( Event *ev );
	void					SetAlpha( Event *ev );
	void					SetKillTarget( Event *ev );
	void					SetAngles( Event *ev );
	void					SetAngleEvent( Event *ev );
	void					TouchTriggersEvent( Event *ev );

	str               GetRandomAlias( str name, AliasListNode_t **ret );
	void              SetWaterType( void );

	// model binding functions
	qboolean          attach( int parent_entity_num, int tag_num, qboolean use_angles = qtrue, Vector attach_offset = Vector( "0 0 0" ) );
	void              detach( void );

	qboolean				GlobalAliasExists( const char *name );
	qboolean				AliasExists( const char *name );

	void				ProcessSoundEvent( Event *ev, qboolean checkSubtitle );
	void              Sound( Event *ev );
	virtual void		Sound( str sound_name, int channel = CHAN_BODY, float volume = -1.0,
		float min_dist = -1.0, Vector *origin = NULL, float pitch = -1.0f, int argstype = 0,
		int doCallback = 0, int checkSubtitle = 1, float max_dist = -1.0f );
	void					StopSound( int channel );
	void					StopSound( Event *ev );
	void					LoopSound( Event *ev );
	void					LoopSound( str sound_name, float volume = -1.0, float min_dist = -1.0, float max_dist = -1.0, float pitch = -1.0 );
	void					StopLoopSound( Event *ev );
	void					StopLoopSound( void );

	void				SetLight( Event *ev );
	void				LightOn( Event *ev );
	void				LightOff( Event *ev );
	void				LightRed( Event *ev );
	void				LightGreen( Event *ev );
	void				LightBlue( Event *ev );
	void				LightRadius( Event *ev );
	void				LightStyle( Event *ev );
	void				Flags( Event *ev );
	void				Effects( Event *ev );
	void				RenderEffects( Event *ev );
	void				SVFlags( Event *ev );

	void					BroadcastAIEvent( int iType = 8, float rad = SOUND_RADIUS );
	void					BroadcastAIEvent( Event *ev );
	void				Kill( Event *ev );
	void				SurfaceModelEvent( Event *ev );
	void				SurfaceCommand( const char * surf_name, const char * token );
	void				SetShaderData( Event *ev );

	void				DoForceActivate( void );
	virtual void		ClientThink( void );
	virtual void		Postthink( void );
	virtual void		Think( void );

	void              DamageSkin( trace_t * trace, float damage );

	void					AttachEvent( Event *ev );
	void					AttachModelEvent( Event *ev );
	void					RemoveAttachedModelEvent( Event *ev );
	void					DetachEvent( Event *ev );
	void              TakeDamageEvent( Event *ev );
	void              NoDamageEvent( Event *ev );
	void              Gravity( Event *ev );
	void              GiveOxygen( float time );
	void              UseBoundingBoxEvent( Event *ev );
	void              HurtEvent( Event *ev );
	void              HealEvent( Event *ev );
	void              SetMassEvent( Event *ev );
	void              Censor( Event *ev );
	void              Ghost( Event *ev );

	void					StationaryEvent( Event *ev );
	void              Explosion( Event *ev );

	void              Shader( Event *ev );

	void              KillAttach( Event *ev );
	void					SetBloodModel( Event *ev );

	void					DropToFloorEvent( Event *ev );

	// Binding methods
	void					joinTeam( Entity *teammember );
	void					quitTeam( void );
	qboolean          isBoundTo( Entity *master );
	virtual void		bind( Entity *master, qboolean use_my_angles = false, qboolean bBindChilds = false );
	virtual void		unbind( void );
	virtual void		glue( Entity *master, qboolean use_my_angles = true );
	virtual void		unglue( void );

	void					JoinTeam( Event *ev );
	void					EventQuitTeam( Event *ev );
	void					BindEvent( Event *ev );
	void					EventUnbind( Event *ev );
	void					GlueEvent( Event *ev );
	void					EventUnglue( Event *ev );
	void					AddToSoundManager( Event *ev );
	void              NoLerpThisFrame( void );

	virtual void		addAngles( Vector add );

	void					DeathSinkStart( Event *ev );
	void					DeathSink( Event *ev );

	void              DetachAllChildren( void );
	void              DetachAllChildren( Event *ev );
	void				SetMovementStealth( float fStealthScale );
	void				EventMovementStealth( Event *ev );

	void				GetYaw( Event *ev );
	void				PusherEvent( Event *ev );

	void				NeverDraw( Event *ev );
	void				NormalDraw( Event *ev );
	void				AlwaysDraw( Event *ev );

	void				GetMins( Event *ev );
	void				GetMaxs( Event *ev );

	void				ForceActivate( Event *ev );
	void				EventTrace( Event *ev );
	void				EventSightTrace( Event *ev );

	void				ConnectPaths( void );
	void				DisconnectPaths( void );

	void				EventDisconnectPaths( Event *ev );
	void				EventConnectPaths( Event *ev );

	void				EventGetEntnum( Event *ev );
	void				EventSetRadnum( Event *ev );
	void				EventGetRadnum( Event *ev );
	void				EventSetRotatedBbox( Event *ev );
	void				EventGetRotatedBbox( Event *ev );

	virtual void		EndFrame( void );
	virtual void		CalcBlend( void );

	virtual void		VelocityModified( void );

	virtual void		PreAnimate( void );
	virtual void		PostAnimate( void );

	virtual void		Archive( Archiver &arc );
	virtual bool		AutoArchiveModel( void );
	virtual void		PathnodeClaimRevoked( class PathNode *node );
	virtual qboolean	BlocksAIMovement( void ) const;
	virtual qboolean	AIDontFace( void ) const;

	void ProcessHint(gentity_t* client, bool bShow);
	void				GetZone( Event *ev );
	void				IsInZone( Event *ev );
	void				SetDepthHack( Event *ev );
	void				SetHintRequireLookAt( Event *ev );
	void				SetHintString( Event *ev );
	void				SetShader( Event * ev );

	void				DrawBoundingBox( int showbboxes );
};

inline int Entity::getSolidType
	(
	void
	)

	{
	return edict->solid;
	}


inline qboolean Entity::DistanceTo
	(
	Vector pos
	)

	{
	Vector delta;

   delta = origin - pos;
	return delta.length();
	}

inline qboolean Entity::DistanceTo
	(
	Entity *ent
	)

	{
	Vector delta;

	assert( ent );

	if ( !ent )
		{
		// "Infinite" distance
		return 999999;
		}

   delta = origin - ent->origin;
	return delta.length();
	}

inline qboolean Entity::WithinDistance
	(
	Vector pos,
	float dist
	)

	{
	Vector delta;

   delta = origin - pos;

	// check squared distance
	return ( ( delta * delta ) < ( dist * dist ) );
	}

inline qboolean Entity::WithinDistance
	(
	Entity *ent,
	float dist
	)

	{
	Vector delta;

	assert( ent );

	if ( !ent )
		{
		return false;
		}

   delta = origin - ent->origin;

	// check squared distance
	return ( ( delta * delta ) < ( dist * dist ) );
	}

inline qboolean Entity::Targeted
	(
	void
	)

	{
	if ( !targetname.length() )
		{
		return false;
		}
	return true;
	}

inline const char * Entity::KillTarget
	(
	void
	)

	{
	return killtarget.c_str();
	}

inline qboolean Entity::hidden
	(
	void
	)

{
	if( edict->s.renderfx & RF_DONTDRAW )
	{
		return true;
	}
	return false;
}

inline void Entity::setModel
	(
	str &mdl
	)

	{
	setModel( mdl.c_str() );
	}

inline void Entity::SetModelEvent
	(
	Event *ev
	)

	{
	setModel( ev->GetString( 1 ) );
	}

inline void Entity::GetBrushModelEvent
	(
	Event *ev
	)

{
	ev->AddString( model );
}

inline void Entity::GetModelEvent
	(
	Event *ev
	)

{
	if( !edict->tiki )
	{
		ev->AddNil();
		return;
	}

	const char *name = gi.TIKI_NameForNum( edict->tiki );

	if( !name )
	{
		if( model != "" )
		{
			ev->AddString( model );
		}
		else
		{
			ev->AddNil();
			return;
		}
	}
	else
	{
		ev->AddString( name );
	}
}

inline void Entity::hideModel
	(
	void
	)

{
	edict->s.renderfx |= RF_DONTDRAW;
	if( getSolidType() <= SOLID_TRIGGER )
	{
		edict->r.svFlags |= SVF_NOCLIENT;
	}
}

inline void Entity::showModel
	(
	void
	)

{
	edict->s.renderfx &= ~RF_DONTDRAW;
	edict->r.svFlags &= ~SVF_NOCLIENT;
}

inline float Entity::alpha
	(
	void
	)

	{
	return edict->s.alpha;
	}

inline void Entity::setMoveType
	(
	int type
	)

	{
	movetype = type;
	}

inline int Entity::getMoveType
	(
	void
	)

	{
	return movetype;
	}

inline void Entity::unlink
	(
	void
	)

	{
	gi.UnlinkEntity( edict );
	}

inline void Entity::setContents
	(
	int type
	)

	{
	edict->r.contents = type;
	}

inline int Entity::getContents
	(
	void
	)

{
	return edict->r.contents;
}

inline qboolean Entity::isClient
	(
	void
	)

{
	if( client )
	{
		return true;
	}
	return false;
}

inline void Entity::SetDeltaAngles
   (
   void
   )

   {
   int i;

   if ( client )
      {
	   for( i = 0; i < 3; i++ )
         {
		   client->ps.delta_angles[ i ] = ANGLE2SHORT( client->ps.viewangles[ i ] );
         }
      }
   }

inline qboolean Entity::GlobalAliasExists
	(
	const char *name
	)

	{
	AliasListNode_t *ret;

	assert( name );

	return ( gi.GlobalAlias_FindRandom( name, &ret ) != NULL );
	}

inline qboolean Entity::AliasExists
	(
	const char *name
	)

	{
	AliasListNode_t *ret;

	assert( name );

	return ( gi.Alias_FindRandom( edict->tiki, name, &ret ) != NULL );
	}

inline str Entity::GetRandomAlias( str name, AliasListNode_t **ret )
{
	str realname;
	const char *s;

	if( edict->tiki )
	{
		s = gi.Alias_FindRandom( edict->tiki, name.c_str(), ret );
	}
	else
	{
		s = NULL;
	}

	if( s )
	{
		realname = s;
	}
	else
	{
		s = gi.GlobalAlias_FindRandom( name.c_str(), ret );
		if( s )
		{
			realname = s;
		}
	}

	return realname;
}

inline int Entity::CurrentFrame
	(
   bodypart_t part
	)

	{
   part = part;
	return 0;
	}

inline int Entity::CurrentAnim
	(
   bodypart_t part
	)

	{
   part = part;
	return 0;
	}

#include <world.h>

#endif

