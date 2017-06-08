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
// player.h: Class definition of the player.

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "g_local.h"
#include "vector.h"
#include "entity.h"
#include "weapon.h"
#include "sentient.h"
#include "navigate.h"
#include "misc.h"
#include "bspline.h"
#include "camera.h"
#include "specialfx.h"
#include "characterstate.h"
#include "actor.h"
#include "vehicle.h"
#include "dm_team.h"

extern Event EV_Player_EndLevel;
extern Event EV_Player_GiveCheat;
extern Event EV_Player_GodCheat;
extern Event EV_Player_NoTargetCheat;
extern Event EV_Player_NoClipCheat;
extern Event EV_Player_GameVersion;
extern Event EV_Player_Fov;
extern Event EV_Player_WhatIs;
extern Event EV_Player_Respawn;
extern Event EV_Player_WatchActor;
extern Event EV_Player_StopWatchingActor;
extern Event EV_Player_DoStats;
extern Event EV_Player_EnterIntermission;
extern Event EV_GetViewangles;
extern Event EV_SetViewangles;
extern Event EV_Player_AutoJoinDMTeam;
extern Event EV_Player_JoinDMTeam;
extern Event EV_Player_Respawn;
extern Event EV_Player_PrimaryDMWeapon;
extern Event EV_Player_StuffText;

enum painDirection_t 
   { 
   PAIN_NONE, 
   PAIN_FRONT,
   PAIN_LEFT,
   PAIN_RIGHT,
   PAIN_REAR
   };

typedef enum
{
	PVT_NONE_SET,
	PVT_ALLIED_START,
	PVT_ALLIED_AIRBORNE,
	PVT_ALLIED_MANON,
	PVT_ALLIED_SAS,
	PVT_ALLIED_PILOT,
	PVT_ALLIED_ARMY,
	PVT_ALLIED_RANGER,
	PVT_ALLIED_END,
	PVT_AXIS_START,
	PVT_AXIS_AXIS1,
	PVT_AXIS_AXIS2,
	PVT_AXIS_AXIS3,
	PVT_AXIS_AXIS4,
	PVT_AXIS_AXIS5,
	PVT_AXIS_END
} voicetype_t;

typedef void ( Player::*movecontrolfunc_t )( void );

typedef struct vma_s {
	str				name;
	float			speed;
} vma_t;

#define MAX_SPEED_MULTIPLIERS			4
#define MAX_ANIM_SLOT					16
#define MAX_TRAILS						2

class Player : public Sentient
{
	friend class Camera;
	friend class Vehicle;
	friend class TurretGun;
	friend class VehicleTurretGun;

private:
	static Condition<Player> Conditions[];
	static movecontrolfunc_t MoveStartFuncs[];

	StateMap *statemap_Legs;
	StateMap *statemap_Torso;

	State *currentState_Legs; 
	State *currentState_Torso;

	str		last_torso_anim_name;
	str		last_leg_anim_name;
	str		partAnim[ 2 ];
	int		m_iPartSlot[ 2 ];
	float	m_fPartBlends[ 2 ];
	str		partOldAnim[ 2 ];
	float	partBlendMult[ 2 ];

	bool						animdone_Legs;
	bool						animdone_Torso;
	Container<Conditional *>	legs_conditionals;
	Container<Conditional *>	torso_conditionals;
	Conditional					*m_pLegsPainCond;
	Conditional					*m_pTorsoPainCond;

	float		m_fLastDeltaTime;
	qboolean	m_bActionAnimPlaying;
	int			m_iBaseActionAnimSlot;
	str			m_sActionAnimName;
	int			m_iActionAnimType;
	qboolean	m_bActionAnimDone;
	str			m_sOldActionAnimName;
	float		m_fOldActionAnimWeight;
	float		m_fOldActionAnimFadeTime;
	int			m_iOldActionAnimType;
	qboolean	m_bMovementAnimPlaying;
	int			m_iBaseMovementAnimSlot;
	str			m_sMovementAnimName;
	str			m_sOldMovementAnimName;
	float		m_fOldMovementWeight;
	float		m_fOldMovementFadeTime;
	str			m_sSlotAnimNames[ MAX_ANIM_SLOT ];

	movecontrol_t	movecontrol;
	int				m_iMovePosFlags;
	int				last_camera_type;

	Vector	oldvelocity;
	Vector	old_v_angle;
	Vector	oldorigin;
	float	animspeed;
	float	airspeed;
	Vector	m_vPushVelocity;

	// blend
	float		blend[ 4 ];
	float		fov;
	float		selectedfov;
	qboolean	m_iInZoomMode;

	// aiming direction
	Vector v_angle;
	Vector m_vViewPos;
	Vector m_vViewAng;

	int buttons;
	int new_buttons;
	int server_new_buttons;

	float	respawn_time;
	int		last_attack_button;

	// damage blend
	float	damage_blood;
	float	damage_alpha;
	Vector	damage_blend;
	Vector	damage_from;
	Vector	damage_angles;
	float	damage_count;
	float	damage_yaw;
	float	next_painsound_time;
	str		waitForState;

	SafePtr<Camera>	camera;
	SafePtr<Camera>	actor_camera;
	SimpleActorPtr	actor_to_watch;

	qboolean actor_camera_right;
	qboolean starting_actor_camera_right;

	// music stuff
	int		music_current_mood;
	int		music_fallback_mood;
	float	music_current_volume;
	float	music_saved_volume;
	float	music_volume_fade_time;

	int		reverb_type;
	float	reverb_level;

	qboolean		gibbed;
	float			pain;
	painDirection_t	pain_dir;
	meansOfDeath_t	pain_type;
	int				pain_location;
	bool		take_pain;
	int			nextpaintime;
	bool		knockdown;

	bool	canfall;
	bool	falling;
	int		feetfalling;
	Vector	falldir;

	bool mediumimpact;
	bool hardimpact;

	qboolean music_forced;

	usercmd_t	last_ucmd;
	usereyes_t	last_eyeinfo;

	// movement variables
	float	animheight;
	Vector	yaw_forward;
	Vector	yaw_left;

	SafePtr<Entity>	atobject;
	float			atobject_dist;
	Vector			atobject_dir;

	SafePtr<Entity>	toucheduseanim;
	int				useanim_numloops;
	SafePtr<Entity>	useitem_in_use;

	float	move_left_vel;
	float	move_right_vel;
	float	move_backward_vel;
	float	move_forward_vel;
	float	move_up_vel;
	float	move_down_vel;
	int		moveresult;

	float damage_multiplier;

	voicetype_t m_voiceType;

	int num_deaths;
	int num_kills;
	int num_won_matches;
	int num_lost_matches;

	bool	m_bTempSpectator;
	bool	m_bSpectator;
	bool	m_bAllowFighting;
	bool	m_bReady;
	int		m_iPlayerSpectating;

	teamtype_t				dm_team;
	class SafePtr<DM_Team>	current_team;
	float					m_fTeamSelectTime;
	class PlayerStart		*m_pLastSpawnpoint;

	bool	voted;
	int		votecount;

	float			m_fWeapSelectTime;
	float			fAttackerDispTime;
	SafePtr<Entity>	pAttackerDistPointer;

	int		m_iInfoClient;
	int		m_iInfoClientHealth;
	float	m_fInfoClientTime;

	bool m_bShowingHint;

public:
	int m_iNumObjectives;
	int m_iObjectivesCompleted;

	str m_sPerferredWeaponOverride;

	Vector mvTrail[ MAX_TRAILS ];
	Vector mvTrailEyes[ MAX_TRAILS ];
	int mCurTrailOrigin;
	int mLastTrailTime;
	int m_iNumHitsTaken;
	int m_iNumEnemiesKilled;
	int m_iNumObjectsDestroyed;
	int m_iNumShotsFired;
	int m_iNumHits;
	float m_fAccuracy;
	float m_fTimeUsed;
	int m_iNumHeadShots;
	int m_iNumTorsoShots;
	int m_iNumLeftLegShots;
	int m_iNumRightLegShots;
	int m_iNumGroinShots;
	int m_iNumLeftArmShots;
	int m_iNumRightArmShots;

	qboolean yawing_left;
	qboolean yawing_right;
	qboolean yawing;

	Vector headAngles;
	Vector torsoAngles;
	Vector headAimAngles;
	Vector torsoAimAngles;

	// new variables
	str						m_sVision;				// current vision
	str						m_sStateFile;			// custom statefile
	bool					disable_spectate;
	bool					disable_team_change;
	bool					m_bFrozen;				// if player is frozen
	bool					animDoneVM;
	float					speed_multiplier[ MAX_SPEED_MULTIPLIERS ];
	Event					*m_pKilledEvent;
	con_map< str, vma_t >	vmalist;
	str						m_sVMAcurrent;
	str						m_sVMcurrent;
	float					m_fVMAtime;
	dtiki_t					*m_fpsTiki;
	bool					m_bConnected;
	str						m_lastcommand;

public:
	qboolean			returnfalse( Conditional &condition );
	qboolean			returntrue( Conditional &condition );
	qboolean			checkturnleft( Conditional &condition );
	qboolean			checkturnright( Conditional &condition );
	qboolean			checkforward( Conditional &condition );
	qboolean			checkbackward( Conditional &condition );
	qboolean			checkstrafeleft( Conditional &condition );
	qboolean			checkstraferight( Conditional &condition );
	qboolean			checkjump( Conditional &condition );
	qboolean			checkcrouch( Conditional &condition );
	qboolean			checkjumpflip( Conditional &condition );
	qboolean			checkanimdone_legs( Conditional &condition );
	qboolean			checkanimdone_torso( Conditional &condition );
	qboolean			checkattackleft( Conditional &condition );
	qboolean			checkattackright( Conditional &condition );
	qboolean			checkattackbuttonleft( Conditional &condition );
	qboolean			checkattackbuttonright( Conditional &condition );
	qboolean			checksneak( Conditional &condition );
	qboolean			checkrun( Conditional &condition );
	qboolean			checkholsterweapon( Conditional &condition );
	qboolean			checkuse( Conditional &condition );
	qboolean			checkcanturn( Conditional &condition );
	qboolean			checkcanmoveright( Conditional &condition );
	qboolean			checkcanmoveleft( Conditional &condition );
	qboolean			checkcanmovebackward( Conditional &condition );
	qboolean			checkcanmoveforward( Conditional &condition );
	qboolean			checkcanwallhug( Conditional &condition );
	qboolean			checkblocked( Conditional &condition );
	qboolean			checkhasvelocity( Conditional &condition );
	qboolean			checkheight( Conditional &condition );
	qboolean			checkonground( Conditional &condition );
	qboolean			check22degreeslope( Conditional &condition );
	qboolean			check45degreeslope( Conditional &condition );
	qboolean			checklookingup( Conditional &condition );
	qboolean			checkrightleghigh( Conditional &condition );
	qboolean			checkleftleghigh( Conditional &condition );
	qboolean			checkcanfall( Conditional &condition );
	qboolean			checkatdoor( Conditional &condition );
	qboolean			checkfalling( Conditional &condition );
	qboolean			checkgroundentity( Conditional &condition );
	qboolean			checkhardimpact( Conditional &condition );
	qboolean			checkmediumimpact( Conditional &condition );
	qboolean			checkdead( Conditional &condition );
	qboolean			checkhealth( Conditional &condition );
	qboolean			checkpain( Conditional &condition );
	qboolean			checkpaindirection( Conditional &condition );
	qboolean			checkpainlocation( Conditional &condition );
	qboolean			checkpaintype( Conditional &condition );
	qboolean			checkpainthreshold( Conditional &condition );
	qboolean			checkknockdown( Conditional &condition );
	qboolean			checklegsstate( Conditional &condition );
	qboolean			checktorsostate( Conditional &condition );
	qboolean			checkatuseanim( Conditional &condition );
	qboolean			checktouchuseanim( Conditional &condition );
	qboolean			checkatuseobject( Conditional &condition );
	qboolean			checkloopuseobject( Conditional &condition );
	qboolean			checkuseweaponleft( Conditional &condition );
	qboolean			checknewweapon( Conditional &condition );
	qboolean			checkreload( Conditional &condition );
	qboolean			checkuseweapon( Conditional &condition );
	qboolean			checkuseweaponclass( Conditional &condition );
	qboolean			checkhasweapon( Conditional &condition );
	qboolean			checkweaponactive( Conditional &condition );
	qboolean			checkweaponclassactive( Conditional &condition );
	qboolean			checkweaponreadytofire( Conditional &condition );
	qboolean			checkweaponclassreadytofire( Conditional &condition );
	qboolean			checkweaponreadytofire_nosound( Conditional &condition );
	qboolean			checkweaponsemiauto( Conditional &condition );
	qboolean			checkmuzzleclear( Conditional &condition );
	qboolean			checkputawayleft( Conditional &condition );
	qboolean			checkputawayright( Conditional &condition );
	qboolean			checkanyweaponactive( Conditional &condition );
	qboolean			checkstatename( Conditional &condition );
	qboolean			checkattackblocked( Conditional &condition );
	qboolean			checkblockdelay( Conditional &condition );
	qboolean			checkcanstand( Conditional &condition );
	qboolean			checkpush( Conditional &condition );
	qboolean			checkpull( Conditional &condition );
	qboolean			checkladder( Conditional &condition );
	qboolean			checktopladder( Conditional &condition );
	qboolean			checkcangetoffladdertop( Conditional &condition );
	qboolean			checkcangetoffladderbottom( Conditional &condition );
	qboolean			checkfeetatladder( Conditional &condition );
	qboolean			checkcanclimbupladder( Conditional &condition );
	qboolean			checkcanclimbdownladder( Conditional &condition );
	qboolean			checkonladder( Conditional &condition );
	qboolean			checkuseanimfinished( Conditional &condition );
	qboolean			checkchance( Conditional &condition );
	qboolean			checkfacingupslope( Conditional &condition );
	qboolean			checkfacingdownslope( Conditional &condition );
	qboolean			checkinturret( Conditional &condition );
	qboolean			checkinvehicle( Conditional &condition );
	qboolean			checkturrettype( Conditional &condition );
	qboolean			checkduckedviewinwater( Conditional &condition );
	qboolean			checkviewinwater( Conditional &condition );
	qboolean			checkwaterlevel( Conditional &condition );
	qboolean			checksolidforward( Conditional &condition );
	qboolean			checkholstercomplete( Conditional &condition );
	qboolean			checkweaponhasammo( Conditional &condition );
	qboolean			checkweaponhasammoinclip( Conditional &condition );
	qboolean			checkrise( Conditional &condition );
	qboolean			checkweaponsholstered( Conditional &condition );
	qboolean			checkmovementtype( Conditional &condition );
	qboolean			checkpositiontype( Conditional &condition );
	qboolean			checkforwardvelocity( Conditional &condition );
	qboolean			checkminchargetimemet( Conditional &condition );
	qboolean			checkmaxchargetimemet( Conditional &condition );
	qboolean			checkimmediateswitch( Conditional &condition );
	qboolean			CondAnimDoneVM( Conditional &condition );
	qboolean			CondClientCommand( Conditional &condition );
	qboolean			CondVMAnim( Conditional &condition );
	qboolean			CondVariable( Conditional &condition );

	// movecontrol functions
	void              StartPush( void );
	void              StartClimbWall( void );
	void              StartUseAnim( void );
	void              StartLoopUseAnim( void );
	void              SetupUseObject( void );

	void              StartUseObject( Event *ev );
	void              FinishUseObject( Event *ev );
	void              FinishUseAnim( Event *ev );
	void					Turn( Event *ev );
	void					TurnUpdate( Event *ev );
	void					TurnLegs( Event *ev );

	CLASS_PROTOTYPE( Player );

	Player();
	~Player();
	void				Init( void );

	void				InitSound( void );
	void				InitEdict( void );
	void				InitClient( void );
	void				InitPhysics( void );
	void				InitPowerups( void );
	void				InitWorldEffects( void );
	void				InitWeapons( void );
	void				InitView( void );
	void				InitModel( void );
	void				InitState( void );
	void				InitHealth( void );
	void				InitWaterPower( void );
	void				InitInventory( void );
	void				InitDeathmatch( void );
	void				InitMaxAmmo( void );
	void				InitStats( void );
	void				ChooseSpawnPoint( void );

	void					Disconnect( void );

	void		         EndLevel( Event *ev );
	void		         Respawn( Event *ev );

	virtual Vector		GunTarget( bool bNoCollision = false );
	void		         SetDeltaAngles( void );
	virtual void		setAngles( Vector ang );

	qboolean				canUse();
	qboolean				canUse( Entity *entity, bool requiresLookAt );
	int					getUseableEntities( int *touch, int maxcount, bool requiresLookAt = true );
	void		         DoUse( Event *ev );
	void		         Obituary( Entity *attacker, Entity *inflictor, int meansofdeath, int iLocation );
	void		         Killed( Event *ev );
	void		         KilledPlayerInDeathmatch( Player *killed );
	void		         Dead( Event *ev );
	void		         ArmorDamage( Event *ev );
	void		         Pain( Event *ev );
	void				DeadBody( void );
	void				DeadBody( Event *ev );

	void		         TouchStuff( pmove_t *pm );

	void		         GetMoveInfo( pmove_t *pm );
	void		         SetMoveInfo( pmove_t *pm, usercmd_t *ucmd );
	pmtype_t	         GetMovePlayerMoveType( void );
	void		         ClientMove( usercmd_t *ucmd );
	void		         VehicleMove( usercmd_t *ucmd );
	void		         TurretMove( usercmd_t *ucmd );
	//      qboolean          feetOnGround( Vector pos );
	//      Vector            findBestFallPos( Vector pos );
	//      void              CheckFeet( void );
	void				CheckMoveFlags( void );
	void				ClientInactivityTimer( void );
	virtual void		ClientThink( void );
	void				UpdateEnemies( void );

	virtual void			Think();
	virtual void			Postthink();

	void			InitLegsStateTable( void );
	void			InitTorsoStateTable( void );
	void              LoadStateTable( void );
	void              ResetState( Event *ev );
	void              EvaluateState( State *forceTorso = NULL, State *forceLegs = NULL );

	void              CheckGround( void );
	void              UpdateViewAngles( usercmd_t *cmd );
	qboolean          AnimMove( Vector &move, Vector *endpos = NULL );
	qboolean          TestMove( Vector &pos, Vector *endpos = NULL );
	qboolean          CheckMove( Vector &move, Vector *endpos = NULL );

	float             CheckMoveDist( Vector &delta );
	float             TestMoveDist( Vector &pos );

	void				EndAnim_Legs( Event *ev );
	void				EndAnim_Torso( Event *ev );
	void				EndActionAnim( Event *ev );
	void				SetPartAnim( const char *anim, bodypart_t slot = legs );
	void				StopPartAnimating( bodypart_t part );
	void				PausePartAnim( bodypart_t part );
	void				AdjustAnimBlends( void );
	void				PlayerAnimDelta( float *vDelta );

	void		         TouchedUseAnim( Entity * ent );

	void				GiveCheat( Event *ev );
	void				GiveWeaponCheat( Event *ev );
	void				GiveAllCheat( Event *ev );
	void				GodCheat( Event *ev );
	void				FullHeal( Event *ev );
	void				EventFace( Event *ev );
	void				NoTargetCheat( Event *ev );
	void				NoclipCheat( Event *ev );
	void				EventTeleport( Event *ev );
	void				Kill( Event *ev );
	void				GibEvent( Event *ev );
	void				SpawnEntity( Event *ev );
	void				SpawnActor( Event *ev );
	void				ListInventoryEvent( Event *ev );

	void					GameVersion( Event *ev );
	void					Fov( Event *ev );
	void					SafeZoomed( Event *ev );
	void					ToggleZoom( int iZoom );
	void					ZoomOff( void );
	void					ZoomOffEvent( Event *ev );
	qboolean				IsZoomed( void );

	void              GetPlayerView( Vector *pos, Vector *angle );

	float		         CalcRoll( void );
	void		         WorldEffects( void );
	void		         AddBlend( float r, float g, float b, float a );
	void		         CalcBlend( void );
	void		         DamageFeedback( void );

	void				CopyStats( Player *player );
	void              UpdateStats( void );
	void				UpdateStatus( const char * s );
	void              StatCount( int index, int count );
	void              UpdateMusic( void );
	void					UpdateReverb( void );
	void              UpdateMisc( void );

	void					SetReverb( str type, float level );
	void					SetReverb( int type, float level );
	void					SetReverb( Event *ev );

	Camera            *CurrentCamera( void );
	void              SetCamera( Camera *ent, float switchTime );
	void              CameraCut( void );
	void              CameraCut( Camera *ent );

	void		         SetPlayerView( Camera * camera, Vector position, float cameraoffset, Vector ang, Vector vel, float camerablend[ 4 ], float camerafov );
	void		         SetupView( void );

	void              ProcessPmoveEvents( int event );

	void              SwingAngles( float destination, float swingTolerance, float clampTolerance, float speed, float *angle, qboolean *swinging );
	void              PlayerAngles( void );
	void		         FinishMove( void );
	virtual void         EndFrame( void );

	void		         TestThread( Event *ev );
	void              useWeapon( const char *weaponname, weaponhand_t hand = WEAPON_MAIN );
	void              useWeapon( Weapon *weapon, weaponhand_t hand = WEAPON_MAIN );

	void		         GotKill( Event *ev );
	void              SetPowerupTimer( Event *ev );
	void              UpdatePowerupTimer( Event *ev );

	void		         WhatIs( Event *ev );
	void              ActorInfo( Event *ev );
	void		         Taunt( Event *ev );

	void              ChangeMusic( const char * current, const char * fallback, qboolean force );
	void		         ChangeMusicVolume( float volume, float fade_time );
	void		         RestoreMusicVolume( float fade_time );

	void              Archive( Archiver &arc );
	void              ArchivePersistantData( Archiver &arc );

	void              GiveOxygen( float time );

	void		         KillEnt( Event *ev );
	void		         RemoveEnt( Event *ev );
	void		         KillClass( Event *ev );
	void		         RemoveClass( Event *ev );

	void              Jump( Event *ev );
	void              JumpXY( Event *ev );

	virtual Vector		EyePosition( void );

	void				GetViewangles( Event *ev );
	void				SetViewangles( Event *ev );
	virtual void		SetViewAngles( Vector angles );
	virtual Vector		GetViewAngles( void );
	void				SetFov( float newFov );
	void				EventSetSelectedFov( Event *ev );
	void				SetSelectedFov( float newFov );

	void              AdjustAnglesForAttack( void );
	Entity            *FindEnemyInFOV( float fov, float maxdist );
	Entity            *FindEnemyInFOVFromTagWithOffset( float fov, float maxdist, str tagname, Vector offset );
	void              DumpState( Event *ev );
	void              ForceLegsState( Event *ev );
	void              ForceTorsoState( Event *ev );

	Vector            GetAngleToTarget( Entity *ent, str tag, float yawclamp, float pitchclamp, Vector baseangles );

	void              AutoAim( void );
	void              AcquireTarget( void );
	void					RemoveTarget( Entity *ent_to_remove );
	void					AcquireHeadTarget( void );

	void              SetCurrentCombo( Event *ev );

	qboolean          GetTagPositionAndOrientation( str tagname, orientation_t *new_or );
	qboolean          GetTagPositionAndOrientation( int tagnum, orientation_t *new_or );

	void              DebugWeaponTags( int controller_tag, Weapon *weapon, str weapon_tagname );
	void              CheckReloadWeapons( void );
	void              NextPainTime( Event *ev );
	void              SetTakePain( Event *ev );

	void              SetMouthAngle( Event *ev );

	void              EnterVehicle( Event *ev );
	void              ExitVehicle( Event *ev );
	void              Holster( Event *ev );
	void              HolsterToggle( Event *ev );
	void              SetWaterPower( Event *ev );

	// team stuff
	int				GetNumKills() const;
	int				GetNumDeaths() const;
	void				AddKills( int num );
	void				AddDeaths( int num );

	DM_Team			*GetDM_Team();
	void				SetDM_Team( DM_Team *team );
	teamtype_t		GetTeam();
	void				SetTeam( teamtype_t team );

	PlayerStart			*GetLastSpawnpoint() const { return m_pLastSpawnpoint; }
	Vector				GetLastSpawnPos() const { return ( ( Entity * )m_pLastSpawnpoint )->origin; }

	void				BeginTempSpectator( void );
	void				EndSpectator( void );
	void				Spectator( void );
	void				Spectator( Event *ev );
	bool				IsSpectator( void );
	bool				IsValidSpectatePlayer( Player *pPlayer );
	void				SetPlayerSpectate( void );
	void				BeginFight( void );
	void				EndFight( void );

	void				EventGetUseHeld( Event *ev );
	void				EventGetFireHeld( Event *ev );

	void				Score( Event *ev );

	void				WonMatch( void );
	void				LostMatch( void );

	void				HUDPrint( const char *s );

	void				GetIsDisguised( Event *ev );
	void				GetHasDisguise( Event *ev );
	void				SetHasDisguise( Event *ev );
	void				SetObjectiveCount( Event *ev );

	void				EventDMDeathDrop( Event *ev );
	void				EventStopwatch( Event *ev );
	void				SetStopwatch( int iDuration );
	void				CallVote( Event *ev );
	void				Vote( Event *ev );
	void				EventCoord( Event *ev );
	void				EventStuffText( Event *ev );
	void				EventSetVoiceType( Event *ev );

	void				EventEnterIntermission( Event *ev );

	void				EnterTurret( Event *ev );
	void				EnterTurret( TurretGun *ent );
	void				ExitTurret( Event *ev );
	void				ExitTurret( void );

	void					WatchActor( Event *ev );
	void					StopWatchingActor( Event *ev );
	void              WeaponCommand( Event *ev );
	void              PlayerDone( Event *ev );
	painDirection_t   Pain_string_to_int( str pain );
	inline Vector     GetVAngles( void ){ return v_angle; }
	void              SpawnDamageEffect( meansOfDeath_t mod );
	virtual void      GetStateAnims( Container<const char *> *c );
	virtual void      VelocityModified( void );
	int					GetKnockback( int original_knockback, qboolean blocked );
	int					GetMoveResult( void );
	virtual void      ReceivedItem( Item * item );
	virtual void      RemovedItem( Item * item );
	virtual void      AmmoAmountChanged( Ammo * ammo, int inclip = 0 );

	void PhysicsOn( Event *ev );
	void PhysicsOff( Event *ev );

	// ladder stuff
	void AttachToLadder( Event *ev );
	void UnattachFromLadder( Event *ev );
	void TweakLadderPos( Event *ev );
	void EnsureOverLadder( Event *ev );
	void EnsureForwardOffLadder( Event *ev );

	void				RemoveFromVehiclesAndTurrets( void );
	void              WaitForState( Event *ev );
	void              SkipCinematic( Event *ev );
	void              SetDamageMultiplier( Event *ev );
	void              LogStats( Event *ev );
	void				Stats( Event *ev );
	void              WeaponsHolstered( void );
	void              WeaponsNotHolstered( void );
	void              Loaded( void );
	void					PlayerShowModel( Event *ev );
	virtual void			showModel( void );
	void					ResetHaveItem( Event *ev );
	void					Join_DM_Team( Event *ev );
	void					Auto_Join_DM_Team( Event *ev );
	void					Leave_DM_Team( Event *ev );
	void					EventPrimaryDMWeapon( Event *ev );
	void					PlayerReload( Event *ev );
	void					EventCorrectWeaponAttachments( Event *ev );
	void					SelectNextItem( Event *ev );
	void					SelectPreviousItem( Event *ev );
	void					SelectNextWeapon( Event *ev );
	void					SelectPreviousWeapon( Event *ev );
	void					DropCurrentWeapon( Event *ev );
	void					ModifyHeight( Event *ev );
	void					SetMovePosFlags( Event *ev );
	void					GetPositionForScript( Event *ev );
	void					GetMovementForScript( Event *ev );
	void					EventSetViewModelAnim( Event *ev );
	void					EventDMMessage( Event *ev );
	void					EventIPrint( Event *ev );
	qboolean				ViewModelAnim( str anim, qboolean force_restart, qboolean bFullAnim );
	void					FindAlias( str &output, str name, AliasListNode_t **node );
	virtual void			Spawned( void );

	bool					IsReady( void ) const;
	void					EventGetReady( Event *ev );
	void					EventSetReady( Event *ev );
	void					EventSetNotReady( Event *ev );
	void					EventGetDMTeam( Event *ev );
	void					EventSetPerferredWeapon( Event *ev );

	// reborn stuff
	void		AddKills( Event *ev );
	void		AddDeaths( Event *ev );
	void		AdminRights( Event *ev );
	void		BindWeap( Event *ev );
	void		CanSwitchTeams( Event *ev );
	void		ClearCommand( Event *ev );
	void		Dive( Event *ev );
	void		EventEarthquake( Event *ev );
	void		EventIsSpectator( Event *ev );
	void		EventReplication( Event *ev );
	void		EventSetTeam( Event *ev );
	void		EventGetViewModelAnim( Event *ev );
	void		EventGetViewModelAnimFinished( Event *ev );
	void		EventGetViewModelAnimValid( Event *ev );
	void		FreezeControls( Event *ev );
	void		GetConnState( Event *ev );
	void		GetDamageMultiplier( Event *ev );
	void		GetKills( Event *ev );
	void		GetDeaths( Event *ev );
	void		GetKillHandler( Event *ev );
	void		GetMoveSpeedScale( Event *ev );
	void		GetLegsState( Event *ev );
	void		GetStateFile( Event *ev );
	void		GetTorsoState( Event *ev );
	void		HideEntity( Event *ev );
	void		Inventory( Event *ev );
	void		InventorySet( Event *ev );
	void		IsAdmin( Event *ev );
	void		JoinDMTeamReal( Event *ev );
	void		JoinDMTeam( Event *ev );
	void		LeanLeftHeld( Event *ev );
	void		LeanRightHeld( Event *ev );
	void		ModifyHeightFloat( Event *ev );
	void		PlayLocalSound( Event *ev );
	void		RunHeld( Event *ev );
	void		SecFireHeld( Event *ev );
	void		SetAnimSpeed( Event *ev );
	void		SetClientFlag( Event *ev );
	void		SetEntityShader( Event *ev );
	void		SetKillHandler( Event *ev );
	void		SetLocalSoundRate( Event *ev );
	void		SetSpeed( Event *ev );
	void		SetStateFile( Event *ev );
	void		SetVMASpeed( Event *ev );
	void		ShowEntity( Event *ev );
	void		StopLocalSound( Event *ev );
	void		UseHeld( Event *ev );
	void		Userinfo( Event *ev );
	void		VisionGetNaked( Event *ev );
	void		VisionSetBlur( Event *ev );
	void		VisionSetNaked( Event *ev );
};

inline void Player::Archive
	(
	Archiver &arc
	)

{
	str tempStr;

	Sentient::Archive( arc );

	arc.ArchiveInteger( &m_iPartSlot[ 0 ] );
	arc.ArchiveInteger( &m_iPartSlot[ 1 ] );

	arc.ArchiveFloat( &m_fPartBlends[ 0 ] );
	arc.ArchiveFloat( &m_fPartBlends[ 1 ] );
	arc.ArchiveFloat( &partBlendMult[ 0 ] );
	arc.ArchiveFloat( &partBlendMult[ 0 ] );

	arc.ArchiveString( &last_torso_anim_name );
	arc.ArchiveString( &last_leg_anim_name );
	arc.ArchiveString( &partAnim[ 0 ] );
	arc.ArchiveString( &partAnim[ 1 ] );
	arc.ArchiveString( &partOldAnim[ 0 ] );
	arc.ArchiveString( &partOldAnim[ 1 ] );

	arc.ArchiveString( &m_sPerferredWeaponOverride );

	arc.ArchiveBool( &animdone_Legs );
	arc.ArchiveBool( &animdone_Torso );
	arc.ArchiveBoolean( &m_bActionAnimPlaying );
	arc.ArchiveInteger( &m_iBaseActionAnimSlot );
	arc.ArchiveString( &m_sActionAnimName );
	arc.ArchiveInteger( &m_iActionAnimType );
	arc.ArchiveBoolean( &m_bActionAnimDone );
	arc.ArchiveString( &m_sOldActionAnimName );
	arc.ArchiveFloat( &m_fOldActionAnimWeight );
	arc.ArchiveFloat( &m_fOldActionAnimFadeTime );
	arc.ArchiveInteger( &m_iOldActionAnimType );
	arc.ArchiveBoolean( &m_iBaseActionAnimSlot );
	arc.ArchiveInteger( &m_iBaseMovementAnimSlot );
	arc.ArchiveString( &m_sMovementAnimName );
	arc.ArchiveString( &m_sOldMovementAnimName );
	arc.ArchiveFloat( &m_fOldMovementWeight );
	arc.ArchiveFloat( &m_fOldMovementFadeTime );

	for( int i = 0; i < MAX_ANIM_SLOT; i++ )
	{
		arc.ArchiveString( &m_sSlotAnimNames[ i ] );
	}

	arc.ArchiveInteger( &m_iMovePosFlags );
	ArchiveEnum( movecontrol, movecontrol_t );
	arc.ArchiveInteger( &last_camera_type );

	arc.ArchiveVector( &oldvelocity );
	arc.ArchiveVector( &old_v_angle );
	arc.ArchiveVector( &oldorigin );
	arc.ArchiveFloat( &animspeed );
	arc.ArchiveFloat( &airspeed );

	arc.ArchiveVector( &m_vPushVelocity );

	arc.ArchiveRaw( blend, sizeof( blend ) );
	arc.ArchiveFloat( &fov );
	arc.ArchiveFloat( &selectedfov );
	arc.ArchiveInteger( &m_iInZoomMode );

	arc.ArchiveVector( &v_angle );
	arc.ArchiveVector( &m_vViewPos );
	arc.ArchiveVector( &m_vViewAng );

	arc.ArchiveInteger( &buttons );
	arc.ArchiveInteger( &new_buttons );
	arc.ArchiveFloat( &respawn_time );

	arc.ArchiveInteger( &last_attack_button );

	arc.ArchiveFloat( &damage_blood );
	arc.ArchiveFloat( &damage_alpha );
	arc.ArchiveVector( &damage_blend );
	arc.ArchiveVector( &damage_from );
	arc.ArchiveVector( &damage_angles );
	arc.ArchiveFloat( &damage_count );
	arc.ArchiveFloat( &next_painsound_time );

	arc.ArchiveSafePointer( &camera );
	arc.ArchiveSafePointer( &actor_camera );
	arc.ArchiveSafePointer( &actor_to_watch );

	arc.ArchiveBoolean( &actor_camera_right );
	arc.ArchiveBoolean( &starting_actor_camera_right );

	arc.ArchiveInteger( &music_current_mood );
	arc.ArchiveInteger( &music_fallback_mood );

	arc.ArchiveFloat( &music_current_volume );
	arc.ArchiveFloat( &music_saved_volume );
	arc.ArchiveFloat( &music_volume_fade_time );

	arc.ArchiveInteger( &reverb_type );
	arc.ArchiveFloat( &reverb_level );

	arc.ArchiveBoolean( &gibbed );
	arc.ArchiveFloat( &pain );

	ArchiveEnum( pain_dir, painDirection_t );
	ArchiveEnum( pain_type, meansOfDeath_t );

	arc.ArchiveInteger( &pain_location );
	arc.ArchiveBool( &take_pain );
	arc.ArchiveInteger( &nextpaintime );

	arc.ArchiveBool( &knockdown );
	arc.ArchiveBool( &canfall );
	arc.ArchiveBool( &falling );

	arc.ArchiveInteger( &feetfalling );
	arc.ArchiveVector( &falldir );

	arc.ArchiveBool( &mediumimpact );
	arc.ArchiveBool( &hardimpact );

	arc.ArchiveBoolean( &music_forced );

	arc.ArchiveRaw( &last_ucmd, sizeof( usercmd_t ) );
	arc.ArchiveRaw( &last_eyeinfo, sizeof( usereyes_t ) );

	arc.ArchiveFloat( &animheight );

	arc.ArchiveVector( &yaw_forward );
	arc.ArchiveVector( &yaw_left );

	arc.ArchiveSafePointer( &atobject );
	arc.ArchiveFloat( &atobject_dist );
	arc.ArchiveVector( &atobject_dir );

	arc.ArchiveSafePointer( &toucheduseanim );
	arc.ArchiveInteger( &useanim_numloops );
	arc.ArchiveSafePointer( &useitem_in_use );

	arc.ArchiveFloat( &move_left_vel );
	arc.ArchiveFloat( &move_right_vel );
	arc.ArchiveFloat( &move_backward_vel );
	arc.ArchiveFloat( &move_forward_vel );
	arc.ArchiveFloat( &move_up_vel );
	arc.ArchiveFloat( &move_down_vel );
	arc.ArchiveInteger( &moveresult );

	arc.ArchiveFloat( &damage_multiplier );

	arc.ArchiveString( &waitForState );
	arc.ArchiveInteger( &m_iNumObjectives );
	arc.ArchiveInteger( &m_iObjectivesCompleted );

	for( int i = 0; i < MAX_TRAILS; i++ )
	{
		arc.ArchiveVector( &mvTrail[ i ] );
	}

	for( int i = 0; i < MAX_TRAILS; i++ )
	{
		arc.ArchiveVector( &mvTrailEyes[ i ] );
	}

	arc.ArchiveInteger( &mCurTrailOrigin );
	arc.ArchiveInteger( &mLastTrailTime );

	arc.ArchiveInteger( &m_iNumHitsTaken );
	arc.ArchiveInteger( &m_iNumEnemiesKilled );
	arc.ArchiveInteger( &m_iNumObjectsDestroyed );
	arc.ArchiveInteger( &m_iNumShotsFired );
	arc.ArchiveInteger( &m_iNumHits );
	arc.ArchiveFloat( &m_fAccuracy );
	arc.ArchiveFloat( &m_fTimeUsed );
	arc.ArchiveInteger( &m_iNumHeadShots );
	arc.ArchiveInteger( &m_iNumTorsoShots );
	arc.ArchiveInteger( &m_iNumLeftLegShots );
	arc.ArchiveInteger( &m_iNumRightLegShots );
	arc.ArchiveInteger( &m_iNumGroinShots );
	arc.ArchiveInteger( &m_iNumLeftArmShots );
	arc.ArchiveInteger( &m_iNumRightArmShots );

	arc.ArchiveFloat( &m_fLastDeltaTime );

	// make sure we have the state machine loaded up
	if( arc.Loading() )
	{
		LoadStateTable();
	}

	if( arc.Saving() )
	{
		if( currentState_Legs )
		{
			tempStr = currentState_Legs->getName();
		}
		else
		{
			tempStr = "NULL";
		}
		arc.ArchiveString( &tempStr );

		if( currentState_Torso )
		{
			tempStr = currentState_Torso->getName();
		}
		else
		{
			tempStr = "NULL";
		}
		arc.ArchiveString( &tempStr );
	}
	else
	{
		arc.ArchiveString( &tempStr );
		if( tempStr != "NULL" )
		{
			currentState_Legs = statemap_Legs->FindState( tempStr );
		}
		else
		{
			currentState_Legs = NULL;
		}
		arc.ArchiveString( &tempStr );
		if( tempStr != "NULL" )
		{
			currentState_Torso = statemap_Torso->FindState( tempStr );
		}
		else
		{
			currentState_Torso = NULL;
		}
	}

	if( arc.Loading() )
	{
		UpdateWeapons();
	}
}

inline Camera *Player::CurrentCamera
   (
   void
   )

   {
   return camera;
   }

inline void Player::CameraCut
	(
   void
	)

	{
   //
   // toggle the camera cut bit
   //
   client->ps.camera_flags = 
      ( ( client->ps.camera_flags & CF_CAMERA_CUT_BIT ) ^ CF_CAMERA_CUT_BIT ) | 
      ( client->ps.camera_flags & ~CF_CAMERA_CUT_BIT );
	}

inline void Player::CameraCut
	(
   Camera * ent
	)

	{
   if ( ent == camera )
      {
      // if the camera we are currently looking through cut, than toggle the cut bits
      CameraCut();
      }
	}

inline void Player::SetCamera
	(
	Camera *ent,
	float switchTime
	)

{
	camera = ent;
	client->ps.camera_time = switchTime;
	if( switchTime <= 0.0f )
	{
		CameraCut();
	}
}

#endif /* player.h */
