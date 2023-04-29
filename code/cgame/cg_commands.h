/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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

// DESCRIPTION:
// client side entity commands

/*
The cg_command system is used for a variety of different functions, but mostly it is 
used for spawning client side temp models.  Either through the use of emitters or
commands that are tied to various frames of animation.

The ctempmodel_t class is the data structure for all of the static tempmodels.  
These are updated every frame and refEntity and refSprite data is created and added
to the renderer for drawing.

The spawnthing_t is the intermediate data holder.  When the TIKI file is processed
the spawnthing_t is cleared out and values are assigned based on the commands issued.

cg_common_data is a list of common data elements that are the same in the ctempmodel_t and
the spawhthing_t

After the m_spawnthing is filled in, 1 or more ctempmodel_t structures are spawned.

The ClientCommandManager is the listener that process all of the commands in the 
TIKI file, similar to ScriptMaster in the server game dll.

*/

#ifndef __CG_COMMANDS_H__
#define __CG_COMMANDS_H__

#include "cg_local.h"
#include "listener.h"
#include "script.h"
#include "vector.h"
#include "../qcommon/qcommon.h"

#define EMITTER_DEFAULT_LIFE 1000
#define MAX_EMITTERS 32
#define MAX_SWIPES 32

#define T_RANDSCALE              (1<<0)
#define T_SCALEANIM              (1<<1)
#define T_SPHERE                 (1<<2)
#define T_INWARDSPHERE           (1<<3)
#define T_CIRCLE                 (1<<4)
#define T_FADE                   (1<<5)
#define T_DIETOUCH               (1<<6)
#define T_ANGLES                 (1<<7)
#define T_WAVE                   (1<<8)
#define T_SWARM                  (1<<9)
#define T_ALIGN                  (1<<10)
#define T_COLLISION              (1<<11)
#define T_FLICKERALPHA           (1<<12)
#define T_DLIGHT                 (1<<13)
#define T_FADEIN                 (1<<14)
#define T_GLOBALFADEIN           (1<<15)
#define T_GLOBALFADEOUT          (1<<16)
#define T_PARENTLINK             (1<<17)
#define T_RANDOMROLL             (1<<18)
#define T_HARDLINK               (1<<19)
#define T_ANIMATEONCE            (1<<20)
#define T_BEAMTHING              (1<<21)
#define T_RANDVELAXIS            (1<<22)
#define T_BOUNCESOUND            (1<<23)
#define T_BOUNCESOUNDONCE        (1<<24)
#define T_TWINKLE                (1<<25)
#define T_TWINKLE_OFF            (1<<26)
#define T_ALIGNONCE              (1<<27)
#define T_SCALEUPDOWN            (1<<28)
#define T_AUTOCALCLIFE           (1<<29)
#define T_ASSIGNED_NUMBER        (1<<30)
#define T_DETAIL                 (1<<31)
                                 
#define T2_MOVE                  (1<<0)
#define T2_AMOVE                 (1<<1)
#define T2_ACCEL                 (1<<2)
#define T2_TRAIL                 (1<<3)
#define T2_PHYSICS_EVERYFRAME    (1<<4)
#define T2_TEMPORARY_DECAL       (1<<5)
#define T2_BOUNCE_DECAL          (1<<6)
#define T2_PARALLEL              (1<<7)

typedef enum { NOT_RANDOM, RANDOM, CRANDOM } randtype_t;

class cg_common_data : public Class
   {
   public:
                     cg_common_data();

      int            life;   
      int            createTime;
      Vector         origin;
      Vector         oldorigin;
      Vector         accel;
      Vector         angles;
      Vector         velocity;
      Vector         avelocity;
      Vector         parentOrigin;
      Vector         parentMins;
      Vector         parentMaxs;
      byte           color[4];
      float          alpha;
      float          scaleRate;
      float          scalemin;
      float          scalemax;   
      float          bouncefactor;
      float          wave;
      int            duplicateCount;
      int            bouncecount;
      int            maxbouncecount;
      str            bouncesound;
      int            bouncesound_delay;
      int            flags;
      int            flags2;
      int            tikihandle;
      int            swarmfreq;
      float          swarmmaxspeed;
      float          swarmdelta;
      float          lightIntensity;
      int            lightType;
      int            fadeintime;
      int            fadedelay;
      int            parent;
      int            collisionmask;
      int            min_twinkletimeoff;
      int            max_twinkletimeoff;
      int            min_twinkletimeon;
      int            max_twinkletimeon;
      int            lightstyle;
      int            physicsRate;
      float          scale;
      str            swipe_shader;
      str            swipe_tag_start;
      str            swipe_tag_end;
      str            shadername;
      float          swipe_life;
      
      float          decal_orientation;
      float          decal_radius;
   };

inline cg_common_data::cg_common_data()
   {
   int i;

   bouncesound_delay    =0;
   life                 =0;
   createTime           =0;
   wave                 =0;
   alpha                =0;
   fadedelay            =0;
   lightIntensity       =0;
   lightType            =(dlighttype_t)0;   
   bouncefactor         =0;
   bouncecount          =0;
   maxbouncecount       =0;
   scaleRate            =0;
   scale                =1;
   scalemin             =0;
   scalemax             =0;
   swarmfreq            =0;
   swarmmaxspeed        =0;
   swarmdelta           =0;
   flags                =0;
   flags2               =0;
   duplicateCount       =0;
   fadeintime           =0;
   parent               =0;
   tikihandle           =0;
   collisionmask        =0;
   min_twinkletimeoff   =0;
   max_twinkletimeoff   =0;
   min_twinkletimeon    =0;
   max_twinkletimeoff   =0;
   lightstyle           =-1;
   physicsRate          =10;

   for (i=0;i<4;i++)
      {
      color[i]       = 0;
      }
   }

class ctempmodel_t : public Class
   {
   public:
          ctempmodel_t();

	class  ctempmodel_t *next;
	class  ctempmodel_t *prev;

   cg_common_data cgd;
   str            modelname;
   
   refEntity_t    lastEnt;
   refEntity_t    ent;
   
   int            number;
   int            lastAnimTime;
   int            lastPhysicsTime;
   int            killTime;
   int            next_bouncesound_time;
   Vector         perp;   
   int            seed;
   int            twinkleTime;
   int            aliveTime;
   qboolean       addedOnce;
   qboolean       lastEntValid;

   void           (*touchfcn)( ctempmodel_t *ct, trace_t *trace );   
   };

inline ctempmodel_t::ctempmodel_t()
   {
   number                  =0;
   lastPhysicsTime         =0;
   lastAnimTime            =0;
   killTime                =0;
   next_bouncesound_time   =0;
   seed                    =0;
   twinkleTime             =0;
   aliveTime               =0;
   addedOnce               =qfalse;
   lastEntValid            =qfalse;
   }

#define LIFE_SWIPE 1
#define MAX_SWIPE_POINTS 64

struct swipepoint_t
   {
   vec3_t points[2];
   float time;
   };

class swipething_t : public Class
   {
   public:
      qboolean       enabled;
      str            tagname_start;
      str            tagname_end;
      int            entitynum;
      float          startcolor[4];
      float          endcolor[4];
      swipepoint_t   swipepoints[MAX_SWIPE_POINTS];
      swipepoint_t   cntPoint;
      int            num_live_swipes;
      int            first_swipe;
      float          life;
      qhandle_t      shader;
      void           Init ();
   };

inline void swipething_t::Init ()
   {
   int i;

   enabled       = qfalse;
   tagname_start = "";
   tagname_end   = "";
   entitynum     = -1;

   for ( i=0; i < 4; i++ )
      {
      startcolor[i] = 1.f;
      endcolor[i] = 0.f;
      }

   for ( i=0; i < MAX_SWIPE_POINTS; i++ )
      {
      VectorSet ( swipepoints[i].points[0], 0.f, 0.f, 0.f );
      VectorSet ( swipepoints[i].points[1], 0.f, 0.f, 0.f );
      swipepoints[i].time = 0.f;
      }

   num_live_swipes = 0;
   first_swipe = 0;
   }

// Enttracker is used to keep track of client side tempmodels.  They are assigned
// a number from a pool of 256.
class enttracker_t : public Class
   {
   public:
                        enttracker_t();
      int               AssignNumber( void );
         
      
   protected:
      qboolean          usedNumbers[256];
      virtual void      RemoveEntity( int entnum );
 
   };

inline enttracker_t::enttracker_t
   (
   )

   {
   memset( usedNumbers, 0, sizeof( usedNumbers ) );
   }

inline void enttracker_t::RemoveEntity
   (
   int entnum
   )

   {
   // If the entnum is a magic number, then clear out the usedNumber field, so that it
   // may be reused for this emitter.

   if ( entnum >= MAGIC_UNUSED_NUMBER )
      {
      entnum -= MAGIC_UNUSED_NUMBER;
      
      assert( entnum >= 0 );
      assert( entnum < 256 );

      usedNumbers[ entnum ] = qfalse;
      }
   }

inline int enttracker_t::AssignNumber
   (
   void
   )

   {
   int i;

   // These numbers are used for client side tempmodels that are emitters themselves.
   // Since they don't have real entity_numbers, they must be assigned a MAGIC number
   // so we can keep track of the last time that the model emitted something. 

   // Search for a number that is not used
   for ( i=0; i<256; i++ )
      {
      if ( !usedNumbers[i] )
         {
         usedNumbers[i] = qtrue;
         return MAGIC_UNUSED_NUMBER + i;
         }
      }

   return -1;
   }

class emittertime_t : public Class
   {
   public:
      int            entity_number;
      int            last_emit_time;
      Vector         oldorigin;
      qboolean       active;
      qboolean       lerp_emitter;
   };

// emitterthing_t is used to keep track of the last time and emitter was updated 
// for a particular entity number.  It inherits from the enttracker_t so it can
// manage client side tempmodels
class emitterthing_t : public enttracker_t
   {
   protected:
      Container<emittertime_t *>  m_emittertimes;  // A list of entity numbers and the last time they emitted

   public:
      emittertime_t  *GetEmitTime( int entnum );
      virtual void   RemoveEntity( int entnum );
      qboolean       startoff;   
      
   };

inline void emitterthing_t::RemoveEntity
   (
   int entnum
   )

   {
   int            num,count;
   emittertime_t  *et;
   
   if ( entnum == -1 )
      return;

   count = m_emittertimes.NumObjects();

   for ( num=count; num>=1; num-- )
      {
      et = m_emittertimes.ObjectAt( num );
      if ( et->entity_number == entnum )
         {
         m_emittertimes.RemoveObjectAt( num );
         delete et;
         }
      }
   
   enttracker_t::RemoveEntity( entnum );
   }

inline emittertime_t *emitterthing_t::GetEmitTime
   (
   int entnum
   )

   {
   int            num, count;
   emittertime_t  *et;

   count = m_emittertimes.NumObjects();

   for ( num=1; num<=count; num++ )
      {
      et = m_emittertimes.ObjectAt( num );
      if ( et->entity_number == entnum )
         {
         return et;
         }
      }

   // Add a new entry if we didn't find it already
   et = new emittertime_t;
   et->entity_number    = entnum;
   et->last_emit_time   = cg.time;
   et->lerp_emitter     = qfalse;

   if ( this->startoff )
      {
      et->active = qfalse;
      }
   else
      {
      et->active = qtrue;
      }

   m_emittertimes.AddObject( et );

   return et;
   }

class commandtime_t : public Class
   {
   public:
      int            entity_number;
      int            command_number;
      int            last_command_time;
   };

// This class is used for keeping track of the last time an entity executed a particular 
// command.  A command number must be assigned externally by the user
class commandthing_t : public enttracker_t
   {

   Container<commandtime_t *>  m_commandtimes;  // A list of entity numbers and the last time they executed a command

   public:
      commandtime_t  *GetLastCommandTime( int entnum, int commandnum );
      virtual void   RemoveEntity( int entnum );
   };

inline void commandthing_t::RemoveEntity
   (
   int entnum
   )

   {
   int             num,count;
   commandtime_t   *ct;
   
   count = m_commandtimes.NumObjects();

   for ( num=count; num>=1; num-- )
      {
      ct = m_commandtimes.ObjectAt( num );
      if ( ct->entity_number == entnum )
         {
         m_commandtimes.RemoveObjectAt( num );
         }
      }
   
   enttracker_t::RemoveEntity( entnum );
   }

inline commandtime_t *commandthing_t::GetLastCommandTime
   (
   int entnum,
   int commandnum
   )

   {
   int            num, count;
   commandtime_t  *ct;

   // Search for this entity number
   count = m_commandtimes.NumObjects();

   for ( num=1; num<=count; num++ )
      {
      ct = m_commandtimes.ObjectAt( num );
      if ( ( ct->entity_number == entnum ) && ( ct->command_number == commandnum ) )
         {
         return ct;
         }
      }

   // Add a new entry if we didn't find it
   ct = new commandtime_t;
   ct->entity_number       = entnum;
   ct->command_number      = commandnum;
   ct->last_command_time   = 0;
   
   m_commandtimes.AddObject( ct );

   return ct;
   }

class spawnthing_t : public emitterthing_t
   {
   public:
      Container<str> m_modellist;     // A list of models that should be spawned from the emitter
      Container<str> m_taglist;       // A list of tags to create beams

      cg_common_data cgd;

      Vector      origin_offset;
      Vector      axis_offset;
      Vector      velocityVariation;
      Vector      forward;
      Vector      right;
      Vector      up;
      vec3_t      axis[3];
      vec3_t      tag_axis[3];
      randtype_t  randvel[3];
      randtype_t  randorg[3];
      randtype_t  randavel[3];
      randtype_t  randangles[3];
      randtype_t  randaxis[3];
      float       forwardVelocity;
      float       sphereRadius;
      float       spawnRate;
      int         lastTime;
      int         count;
      int         trace_count;
      str         tagname;
      str         emittername;
      str         animName;
      float       dcolor[3];
      qboolean    dlight;

      // beam stuff also impact trace stuff
      str         startTag;
      str         endTag;
      float       length;
      float       min_offset;
      float       max_offset;
      float       overlap;
      float       numSubdivisions;
      float       delay;
      float       toggledelay;
      int         beamflags;
      int         numspherebeams;
      float       endalpha;
      float       spreadx;
      float       spready;
		qboolean		use_last_trace_end;
      
      void        (*touchfcn)( ctempmodel_t *ct, trace_t *trace );
      str         GetModel( void );
      void        SetModel( str model );
   };

inline void spawnthing_t::SetModel
   (
   str model
   )

   {
   m_modellist.ClearObjectList();
   m_modellist.AddObject( model );
   }

inline str spawnthing_t::GetModel
   (
   void
   )

   {   
   int num, index;

   num = m_modellist.NumObjects();

   if ( !num )
      {
      return "";
      }

   index = ( num * random() ) + 1;

   if ( index > num )
      index = num;

   return m_modellist.ObjectAt( index );
   }

// Keeps track of beams that are created by entities that need their positions updated every frame 
class beamthing_t : public emitterthing_t
   {
   public:
      str   beamname;
      str   shadername;
      str   startTag;
      str   endTag;
      int   numSubdivisions;
      int   tikihandle;
      float alpha;
      float scale;
      int   flags;
      float length;
      int   life;
      float min_offset;
      float max_offset;
      float overlap;
      int   delay;
      byte  modulate[4];
   };

#define MAX_TEMPMODELS 1024
#define MAX_BEAMS 128

class ClientGameCommandManager : public Listener
   {
   private:
      spawnthing_t     m_localemitter;               // local emitter used by animation commands
      ctempmodel_t	  m_active_tempmodels;
      ctempmodel_t     *m_free_tempmodels;
      ctempmodel_t	  m_tempmodels[MAX_TEMPMODELS];
      spawnthing_t     *m_spawnthing;
      Container<spawnthing_t *>  m_emitters;         // Global emitters set up by client commands
      int		        m_numtempmodels;              // number of tempmodels
      int              m_seed;
      commandthing_t   m_command_time_manager;       // Keeps track of entity numbers and the last time 
                                                     // they executed particular commands

      void (ClientGameCommandManager::*endblockfcn)( void );
      ctempmodel_t *AllocateTempModel( void );
      qboolean TempModelPhysics( ctempmodel_t *p, float ftime, float time2, float scale );
      qboolean TempModelRealtimeEffects( ctempmodel_t *p, float ftime, float time2, float scale );
      qboolean LerpTempModel( refEntity_t *newEnt, ctempmodel_t *p, float frac );
      void Print( Event *ev );
      void StartBlock( Event *ev );
      void EndBlock( Event *ev );
      void UpdateSpawnThing( spawnthing_t *ep );
      void EmitterStartOff( Event *ev );
      void SetAlpha( Event *ev );
      void SetDieTouch( Event *ev );
      void SetBounceFactor( Event *ev );
      void SetBounceSound( Event *ev );
      void SetBounceSoundOnce( Event *ev );
      void SetModel( Event *ev );
      void SetLife( Event *ev );
      void SetColor( Event *ev );
      void SetColorRange( Event *ev );
      void SetLightstyle( Event *ev );
      void SetDuplicateCount( Event *ev );
      void SetVelocity( Event *ev );
      void SetAngularVelocity( Event *ev );
      void SetCount( Event *ev );
      void SetTraceCount( Event *ev );
      void SetScale( Event *ev );
      void SetScaleUpDown( Event *ev );
      void SetScaleMin( Event *ev );
      void SetScaleMax( Event *ev );
      void SetScaleRate( Event *ev );
      void SetRandomVelocity( Event *ev );
      void SetRandomVelocityAlongAxis( Event *ev );
      void SetAccel( Event *ev );
      void SetFade( Event *ev );
      void SetFadeDelay( Event *ev );
      void SetSpawnRate( Event *ev );   
      void SetOriginOffset( Event *ev );
      void SetOffsetAlongAxis( Event *ev );
      void SetCircle( Event *ev );
      void SetSphere( Event *ev );
      void SetInwardSphere( Event *ev );
      void SetWavy( Event *ev );
      void SetRandomRoll( Event *ev );
      void SetSwarm( Event *ev );
      void SetAlign( Event *ev );
      void SetAlignOnce( Event *ev );
      void SetCollision( Event *ev );
      void SetFlickerAlpha( Event *ev );
      void SetFadeIn( Event *ev );
      void SetEntityColor( Event *ev );
      void SetGlobalFade( Event *ev );
      void SetRadius( Event *ev );
      void SetParentLink( Event *ev );
      void SetHardLink( Event *ev );
      void SetAngles( Event *ev );
      void ParentAngles( Event *ev );
      void SetTwinkle( Event *ev );
      void SetTrail( Event *ev );
      void SetPhysicsRate( Event *ev );
      void SetBounceDecal( Event *ev );
      void UpdateSwarm( ctempmodel_t *p );
      void SpawnTempModel( int count, int timealive=0 );
      void FreeTempModel( ctempmodel_t *le );
      void BeginOriginSpawn( Event *ev );
      void EndOriginSpawn( void );
      void BeginOriginBeamSpawn( Event *ev );
      void EndOriginBeamSpawn( void );
      void BeginOriginBeamEmitter( Event *ev );
      void EndOriginBeamEmitter( void );
      void BeginTagSpawn( Event *ev );
      void EndTagSpawn( void );
      void BeginTagBeamSpawn( Event *ev );
      void EndTagBeamSpawn( void );
      void BeginTagEmitter( Event *ev );
      void EndTagEmitter( void );
      void BeginOriginEmitter( Event *ev );
      void EndOriginEmitter( void );
      void BeginTagBeamEmitter( Event *ev );
      void EndTagBeamEmitter( void );
      void EmitterOn( Event *ev );
      void EmitterOff( Event *ev );
      void RainTouch( Event *ev );
		void Sound( Event *ev );
		void StopSound( Event *ev );
		void LoopSound( Event *ev );
		void Cache( Event *ev );
		void AliasCache( Event *ev );
		void Alias( Event *ev );
		void Client( Event *ev );
      void TagDynamicLight( Event *ev );
      void OriginDynamicLight( Event *ev );
      void DynamicLight( Event *ev );
      void GetOrientation( str tagname, spawnthing_t *sp );
      void AnimateTempModel( ctempmodel_t *ent, Vector origin, refEntity_t *newEnt );
      void OtherTempModelEffects( ctempmodel_t *p, Vector origin, refEntity_t *newEnt );
      void Swipe( Event *ev );
      void SwipeOn ( Event *ev );
      void SwipeOff ( Event *ev );
      void AnimateOnce( Event *ev );
      void SetAnim( Event *ev );
      void BeginTagTraceImpactMark( Event *ev );
      void EndTagTraceImpactMark( void );
      void BeginTagTraceImpactSpawn( Event *ev );
      void EndTagTraceImpactSpawn( void );
      void SetDecalRadius( Event *ev );
      void SetDecalOrientation( Event *ev );
      void TagList( Event *ev );
      void TagTraceImpactSound( Event *ev );
      void SetParallel( Event *ev );
      void Footstep( Event *ev );
      void SetDetail( Event *ev );
   
      // Beam stuff
      void SetSubdivisions( Event *ev );      
      void SetMinOffset( Event *ev );
      void SetMaxOffset( Event *ev );
      void SetShader( Event *ev );
      void SetLength( Event *ev );
      void SetBeamDelay( Event *ev );
      void SetBeamToggleDelay( Event *ev );
      void SetBeamPersist( Event *ev );
      void SetBeamOffsetEndpoints( Event *ev );
      void SetBeamSphere( Event *ev );
      void SetSpread( Event *ev );
		void SetUseLastTraceEnd( Event *ev );
      void SetEndAlpha( Event *ev );

      void RandomChance( Event *ev );
      void CommandDelay( Event *ev );
   public:
      CLASS_PROTOTYPE( ClientGameCommandManager );
      
      ClientGameCommandManager();
      void AddTempModels( void );
      void UpdateEmitter( dtiki_t* tiki, vec3_t axis[3], int entity_number, int parent_number, Vector entity_origin );
      void UpdateBeam( int tikihandle, int entity_number, spawnthing_t *beamthing );
      void PlaySound( str sound_name, vec3_t *origin=NULL, int channel = CHAN_AUTO, float volume = -1, float min_distance = -1 );

      spawnthing_t *InitializeSpawnthing( spawnthing_t *ep );
      void SpawnTempModel( int count, class spawnthing_t *sp );
      void FreeAllTempModels( void );
      void RestartAllEmitters( int timedelta );
      void FreeAllEmitters( void );

      void InitializeTempModels( void );
      void InitializeEmitters( void );
      void RemoveClientEntity( int number, int tikihandle, centity_t *cent, ctempmodel_t *p=NULL );
      void TestEmitter( void );
      void UpdateTestEmitter( void );
      void SetTestEmitterValues( void );
      void DumpEmitter( void );
      void ClearSwipes( void );
      void ResetTempModels( void );
      inline void SetSpawnthing( spawnthing_t *st ){ m_spawnthing = st; };
      spawnthing_t *CreateNewEmitter( str emittername );
      spawnthing_t *CreateNewEmitter( void );
      spawnthing_t *GetEmitterByName( str emittername );
      void CGEvent( centity_t *cent );
   };

class EmitterLoader : public Listener
   {
   private:
      bool                       emitterActive;

   public:
      CLASS_PROTOTYPE ( EmitterLoader );

                  EmitterLoader();
      bool        Load( Script & );
      void        ProcessEmitter( Script & );
      void        Emitter( Event *ev );
   };

extern ClientGameCommandManager commandManager;

#endif // __CG_COMMANDS_H__

