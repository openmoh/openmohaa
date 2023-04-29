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
// testemitter functions. Used for designing emitters in game

#include "cg_commands.h"

cvar_t         *cg_te_xangles;
cvar_t         *cg_te_yangles;
cvar_t         *cg_te_zangles;
cvar_t         *cg_te_alpha;
cvar_t         *cg_te_dietouch;
cvar_t         *cg_te_bouncefactor;
cvar_t         *cg_te_scale;
cvar_t         *cg_te_scalemin;
cvar_t         *cg_te_scalemax;
cvar_t         *cg_te_model;
cvar_t         *cg_te_life;
cvar_t         *cg_te_color_r;
cvar_t         *cg_te_color_g;
cvar_t         *cg_te_color_b;
cvar_t         *cg_te_startcolor_r;
cvar_t         *cg_te_startcolor_g;
cvar_t         *cg_te_startcolor_b;
cvar_t         *cg_te_endcolor_r;
cvar_t         *cg_te_endcolor_g;
cvar_t         *cg_te_endcolor_b;
cvar_t         *cg_te_accel_x;
cvar_t         *cg_te_accel_y;
cvar_t         *cg_te_accel_z;
cvar_t         *cg_te_count;
cvar_t         *cg_te_fade;
cvar_t         *cg_te_fadein;
cvar_t         *cg_te_spawnrate;
cvar_t         *cg_te_orgoffset_x;
cvar_t         *cg_te_orgoffset_y;
cvar_t         *cg_te_orgoffset_z;
cvar_t         *cg_te_rorgx;
cvar_t         *cg_te_rorgy;
cvar_t         *cg_te_rorgz;
cvar_t         *cg_te_scalerate;
cvar_t         *cg_te_circle;
cvar_t         *cg_te_sphere;
cvar_t         *cg_te_insphere;
cvar_t         *cg_te_radius;
cvar_t         *cg_te_swarm;
cvar_t         *cg_te_wavy;
cvar_t         *cg_te_align;
cvar_t         *cg_te_flickeralpha;
cvar_t         *cg_te_collision;
cvar_t         *cg_te_globalfade;
cvar_t         *cg_te_randomroll;
cvar_t         *cg_te_angles_pitch;
cvar_t         *cg_te_angles_yaw;
cvar_t         *cg_te_angles_roll;
cvar_t         *cg_te_randpitch;
cvar_t         *cg_te_randyaw;
cvar_t         *cg_te_randroll;
cvar_t         *cg_te_forwardvelocity;
cvar_t         *cg_te_randvelx;
cvar_t         *cg_te_randvely;
cvar_t         *cg_te_randvelz;
cvar_t         *cg_te_rvxtype;
cvar_t         *cg_te_rvytype;
cvar_t         *cg_te_rvztype;

spawnthing_t *testspawnthing=NULL;

//===============
//UpdateTestEmitter
//===============
void ClientGameCommandManager::UpdateTestEmitter
   (
   void
   )

   {
   int    count;
   Vector delta;
   Vector end,end2,perp;

   emittertime_t  *et=NULL;

   if ( !testspawnthing )
      return;

   SetTestEmitterValues();

   et = testspawnthing->GetEmitTime( 1 );
   
   if ( !et->active )
      return;

   testspawnthing->cgd.createTime        = cg.time;

   end = testspawnthing->cgd.origin + testspawnthing->forward * 50;

   cgi.R_DebugLine( testspawnthing->cgd.origin, end, 1,0,0,1 );
   
   PerpendicularVector( perp, testspawnthing->forward );
   perp.normalize();

   end2 = end + ( perp * 16 ) + ( m_spawnthing->forward * -16 );
   cgi.R_DebugLine( end, end2, 1,0,0,1 );
   
   end2 = end + ( perp * -16 ) + ( m_spawnthing->forward * -16 );
   cgi.R_DebugLine( end, end2, 1,0,0,1 );

   if ( ( et->last_emit_time > 0 ) && ( testspawnthing->spawnRate ) )
      {
      float dtime = cg.time - et->last_emit_time;
      float lerp, lerpfrac;

      count = dtime / testspawnthing->spawnRate;

      // This is kind of a nasty bit of code.  If the count is 1, just spawn
      // a single tempmodel, if it's greater than 1, then spawn the number
      // of tempmodels over a period of time, and adjust their create times
      // accordingly.  Also lerp the origins so they appear where they are 
      // supposed to.  This helps smoothing out low frame rate situations 
      // where this is only get called a few times a second, but the spawn
      // rate is high, and it will look a lot smoother.
      if ( !count )
         {
         return;
         }
      else if ( count == 1 )
         {
         SpawnTempModel( 1 );
         et->last_emit_time = cg.time;
         }
      else
         {
         lerpfrac = 1.0f / (float)count;

         lerp = 0; 
         while ( dtime > testspawnthing->spawnRate )
            {
            float last_time = et->last_emit_time;
            et->last_emit_time = cg.time;
            dtime -= testspawnthing->spawnRate;
            if ( et->lerp_emitter )
               testspawnthing->cgd.origin = et->oldorigin + ( delta * lerp );            
            SpawnTempModel( 1, last_time + dtime );
            lerp += lerpfrac;
            }
         }
      }
   else
      {
      et->last_emit_time = cg.time;
      }
   }

void ClientGameCommandManager::DumpEmitter
   (
   void
   )

   {
   cvar_t      *filename;
   str         buff;
   int         i;

   if ( !testspawnthing )
      return;

   filename = cgi.Cvar_Get( "cg_te_filename", "dump/emitter.txt", 0 );

   buff += va( "model %s\n", testspawnthing->m_modellist.ObjectAt( 1 ).c_str() );

   if ( testspawnthing->cgd.alpha != 1.0 )
      buff += va( "alpha %0.2f\n", testspawnthing->cgd.alpha );
   
   if ( testspawnthing->cgd.bouncefactor != 1 )
      buff += va( "bouncefactor %0.2f\n", testspawnthing->cgd.bouncefactor );

   if ( testspawnthing->cgd.scale != 1 )
      buff += va( "scale %0.2f\n", testspawnthing->cgd.scale );

   buff += va( "life %0.2f\n", cg_te_life->value );

   if ( testspawnthing->cgd.accel[0] || testspawnthing->cgd.accel[1] || testspawnthing->cgd.accel[2] )
      buff += va( "accel %0.2f %0.2f %0.2f\n", testspawnthing->cgd.accel[0], testspawnthing->cgd.accel[1], testspawnthing->cgd.accel[2] );

   buff += va( "spawnrate %0.2f\n", cg_te_spawnrate->value );
   
   if ( testspawnthing->cgd.scaleRate )
      buff += va( "scalerate %0.2f\n", testspawnthing->cgd.scaleRate );

   if ( testspawnthing->sphereRadius )
      buff += va( "radius %0.2f\n", testspawnthing->sphereRadius );

   if ( testspawnthing->forwardVelocity )
      buff += va( "velocity %0.2f\n", testspawnthing->forwardVelocity );

   if ( testspawnthing->origin_offset[0] || testspawnthing->origin_offset[1] || testspawnthing->origin_offset[2] )
      {
      buff += "offset ";

      for ( i=0; i<3; i++ )
         {
         if ( testspawnthing->randorg[i] == NOT_RANDOM )
            buff += va( "%0.2f ", testspawnthing->origin_offset[i] );
         if ( testspawnthing->randorg[i] == RANDOM )
            buff += va( "random %0.2f ", testspawnthing->origin_offset[i] );
         if ( testspawnthing->randorg[i] == CRANDOM )
            buff += va( "crandom %0.2f ", testspawnthing->origin_offset[i] );
         }
      buff += "\n";
      }

   if ( testspawnthing->cgd.angles[0] || testspawnthing->cgd.angles[1] || testspawnthing->cgd.angles[2] )
      {
      buff += "angles ";

      for ( i=0; i<3; i++ )
         {
         if ( testspawnthing->randangles[i] == NOT_RANDOM )
            buff += va( "%0.2f ", testspawnthing->cgd.angles[i] );
         if ( testspawnthing->randangles[i] == RANDOM )
            buff += va( "random %0.2f ", testspawnthing->cgd.angles[i] );
         if ( testspawnthing->randangles[i] == CRANDOM )
            buff += va( "crandom %0.2f ", testspawnthing->cgd.angles[i] );
         }
      buff += "\n";
      }

   if ( testspawnthing->velocityVariation[0] || testspawnthing->velocityVariation[1] || testspawnthing->velocityVariation[2] )
      {
      buff += "randvel ";

      for ( i=0; i<3; i++ )
         {
         if ( testspawnthing->randvel[i] == NOT_RANDOM )
            buff += va( "%0.2f ", testspawnthing->velocityVariation[i] );
         if ( testspawnthing->randvel[i] == RANDOM )
            buff += va( "random %0.2f ", testspawnthing->velocityVariation[i] );
         if ( testspawnthing->randvel[i] == CRANDOM )
            buff += va( "crandom %0.2f ", testspawnthing->velocityVariation[i] );
         }
      buff += "\n";
      }

   if ( testspawnthing->cgd.scalemin )
      buff += va( "scalemin %0.2f\n", testspawnthing->cgd.scalemin );

   if ( testspawnthing->cgd.scalemax )
      buff += va( "scalemax %0.2f\n", testspawnthing->cgd.scalemax );
   
   if ( testspawnthing->cgd.flags & T_DIETOUCH )
      buff +="dietouch\n";
   if ( testspawnthing->cgd.flags & T_FADE )
      buff +="fade\n";
   if ( testspawnthing->cgd.flags & T_CIRCLE )
      buff +="circle\n";
   if ( testspawnthing->cgd.flags & T_SPHERE )
      buff +="sphere\n";
   if ( testspawnthing->cgd.flags & T_INWARDSPHERE )
      buff +="inwardsphere\n";
   if ( testspawnthing->cgd.flags & T_ALIGN )
      buff +="align\n";
   if ( testspawnthing->cgd.flags & T_FLICKERALPHA )
      buff +="flickeralpha\n";
   if ( testspawnthing->cgd.flags & T_RANDOMROLL )
      buff +="randomroll\n";

   cgi.FS_WriteTextFile( filename->string, buff.c_str(), buff.length() + 1 );
   }

void ClientGameCommandManager::SetTestEmitterValues
   (
   void
   )

   {   
   vec3_t   angles;
   vec3_t   axis[3];

   testspawnthing->cgd.flags2 |= (T2_MOVE|T2_AMOVE|T2_ACCEL);

   angles[0] = cg_te_xangles->value;
   angles[1] = cg_te_yangles->value;
   angles[2] = cg_te_zangles->value;

   AnglesToAxis( angles, axis );
   testspawnthing->SetModel( cg_te_model->string );
   
   testspawnthing->forward                 = axis[0];
   testspawnthing->right                   = axis[1];
   testspawnthing->up                      = axis[2];
   testspawnthing->cgd.alpha               = cg_te_alpha->value;
   testspawnthing->cgd.color[3]            = cg_te_alpha->value * 255;
   testspawnthing->cgd.bouncefactor        = cg_te_bouncefactor->value;
   testspawnthing->cgd.scale               = cg_te_scale->value;
   testspawnthing->cgd.life                = cg_te_life->value * 1000;
   testspawnthing->cgd.color[0]            = cg_te_color_r->value * 255;
   testspawnthing->cgd.color[1]            = cg_te_color_g->value * 255;
   testspawnthing->cgd.color[2]            = cg_te_color_b->value * 255;
   testspawnthing->cgd.accel[0]            = cg_te_accel_x->value;
   testspawnthing->cgd.accel[1]            = cg_te_accel_y->value;
   testspawnthing->cgd.accel[2]            = cg_te_accel_z->value;
   testspawnthing->count                   = cg_te_count->value;
   testspawnthing->spawnRate               = ( 1.0f / cg_te_spawnrate->value ) * 1000.0f;
   testspawnthing->cgd.scaleRate           = cg_te_scalerate->value;
   testspawnthing->sphereRadius            = cg_te_radius->value;
   testspawnthing->forwardVelocity         = cg_te_forwardvelocity->value;

   // Orgin Offset
   testspawnthing->origin_offset[0]    = cg_te_orgoffset_x->value;
   testspawnthing->origin_offset[1]    = cg_te_orgoffset_y->value;
   testspawnthing->origin_offset[2]    = cg_te_orgoffset_z->value;

   if ( !strcmp( cg_te_rorgx->string, "No" ) )
      testspawnthing->randorg[0] = NOT_RANDOM;
   else if ( !strcmp( cg_te_rorgx->string, "Cr" ) )
      testspawnthing->randorg[0] = CRANDOM;
   else if ( !strcmp( cg_te_rorgx->string, "Ra" ) )
      testspawnthing->randorg[0] = RANDOM;

   if ( !strcmp( cg_te_rorgy->string, "No" ) )
      testspawnthing->randorg[1] = NOT_RANDOM;
   else if ( !strcmp( cg_te_rorgy->string, "Cr" ) )
      testspawnthing->randorg[1] = CRANDOM;
   else if ( !strcmp( cg_te_rorgy->string, "Ra" ) )
      testspawnthing->randorg[1] = RANDOM;

   if ( !strcmp( cg_te_rorgz->string, "No" ) )
      testspawnthing->randorg[2] = NOT_RANDOM;
   else if ( !strcmp( cg_te_rorgz->string, "Cr" ) )
      testspawnthing->randorg[2] = CRANDOM;
   else if ( !strcmp( cg_te_rorgz->string, "Ra" ) )
      testspawnthing->randorg[2] = RANDOM;

   // Random angles
   testspawnthing->cgd.angles[PITCH]       = cg_te_angles_pitch->value;
   testspawnthing->cgd.angles[YAW]         = cg_te_angles_yaw->value;
   testspawnthing->cgd.angles[ROLL]        = cg_te_angles_roll->value;

   if ( !strcmp( cg_te_randpitch->string, "No" ) )
      testspawnthing->randangles[0] = NOT_RANDOM;
   else if ( !strcmp( cg_te_randpitch->string, "Cr" ) )
      testspawnthing->randangles[0] = CRANDOM;
   else if ( !strcmp( cg_te_randpitch->string, "Ra" ) )
      testspawnthing->randangles[0] = RANDOM;

   if ( !strcmp( cg_te_randyaw->string, "No" ) )
      testspawnthing->randangles[0] = NOT_RANDOM;
   else if ( !strcmp( cg_te_randyaw->string, "Cr" ) )
      testspawnthing->randangles[0] = CRANDOM;
   else if ( !strcmp( cg_te_randyaw->string, "Ra" ) )
      testspawnthing->randangles[0] = RANDOM;

   if ( !strcmp( cg_te_randroll->string, "No" ) )
      testspawnthing->randangles[0] = NOT_RANDOM;
   else if ( !strcmp( cg_te_randroll->string, "Cr" ) )
      testspawnthing->randangles[0] = CRANDOM;
   else if ( !strcmp( cg_te_randroll->string, "Ra" ) )
      testspawnthing->randangles[0] = RANDOM;
   
   // Random Velocity
   testspawnthing->velocityVariation[0] = cg_te_randvelx->value;
   testspawnthing->velocityVariation[1] = cg_te_randvely->value;
   testspawnthing->velocityVariation[2] = cg_te_randvelz->value;

   if ( !strcmp( cg_te_rvxtype->string, "No" ) )
      testspawnthing->randvel[0] = NOT_RANDOM;
   else if ( !strcmp( cg_te_rvxtype->string, "Cr" ) )
      testspawnthing->randvel[0] = CRANDOM;
   else if ( !strcmp( cg_te_rvxtype->string, "Ra" ) )
      testspawnthing->randvel[0] = RANDOM;
   
   if ( !strcmp( cg_te_rvytype->string, "No" ) )
      testspawnthing->randvel[1] = NOT_RANDOM;
   else if ( !strcmp( cg_te_rvytype->string, "Cr" ) )
      testspawnthing->randvel[1] = CRANDOM;
   else if ( !strcmp( cg_te_rvytype->string, "Ra" ) )
      testspawnthing->randvel[1] = RANDOM;

   if ( !strcmp( cg_te_rvztype->string, "No" ) )
      testspawnthing->randvel[2] = NOT_RANDOM;
   else if ( !strcmp( cg_te_rvztype->string, "Cr" ) )
      testspawnthing->randvel[2] = CRANDOM;
   else if ( !strcmp( cg_te_rvztype->string, "Ra" ) )
      testspawnthing->randvel[2] = RANDOM;

   testspawnthing->cgd.scalemin            = cg_te_scalemin->value;
   testspawnthing->cgd.scalemax            = cg_te_scalemax->value;
  
   if ( testspawnthing->cgd.scalemin || testspawnthing->cgd.scalemax )
      testspawnthing->cgd.flags |= T_RANDSCALE;
   else
      testspawnthing->cgd.flags &= ~T_RANDSCALE;

   if ( cg_te_dietouch->integer )
      testspawnthing->cgd.flags |= T_DIETOUCH;
   else
      testspawnthing->cgd.flags &= ~T_DIETOUCH;

   if ( cg_te_fade->integer )
      testspawnthing->cgd.flags |= T_FADE;
   else
      testspawnthing->cgd.flags &= ~T_FADE;
   
   if ( cg_te_circle->integer )
      testspawnthing->cgd.flags |= T_CIRCLE;
   else
      testspawnthing->cgd.flags &= ~T_CIRCLE;

   if ( cg_te_sphere->integer )
      testspawnthing->cgd.flags |= T_SPHERE;
   else
      testspawnthing->cgd.flags &= ~T_SPHERE;
   
   if ( cg_te_insphere->integer )
      testspawnthing->cgd.flags |= T_INWARDSPHERE;
   else
      testspawnthing->cgd.flags &= ~T_INWARDSPHERE;
   
   if ( cg_te_align->integer )
      testspawnthing->cgd.flags |= T_ALIGN;
   else
      testspawnthing->cgd.flags &= ~T_ALIGN;

   if ( cg_te_flickeralpha->integer )
      testspawnthing->cgd.flags |= T_FLICKERALPHA;
   else
      testspawnthing->cgd.flags &= ~T_FLICKERALPHA;

   if ( cg_te_randomroll->integer )
      testspawnthing->cgd.flags |= T_RANDOMROLL;
   else
      testspawnthing->cgd.flags &= ~T_RANDOMROLL;

#if 0
      testspawnthing->cgd.cg_te_globalfade;
      testspawnthing->cgd.cg_te_collision;
      testspawnthing->cgd.cg_te_swarm;
      testspawnthing->cgd.cg_te_wavy;
#endif
   }

void ClientGameCommandManager::TestEmitter
   (
   void
   )

   {
   // Create a test emitter that can be modified by the user

   // Init the emitter
   testspawnthing = CreateNewEmitter();
   if( !testspawnthing )
      {
      return;
      }

   // Get the emitter's name
   testspawnthing->emittername = "TestEmitter";

   // Set the origin of the emitter
   VectorMA( cg.refdef.vieworg, 100, cg.refdef.viewaxis[0], testspawnthing->cgd.origin );

   float    angles[3];
   vec3_t   axis[3];

 	angles[PITCH]  = 0;
	angles[YAW]    = cg.refdefViewAngles[1];
	angles[ROLL]   = 0;

	AnglesToAxis( angles, axis );

   testspawnthing->forward         = axis[0];
   testspawnthing->right           = axis[1];
   testspawnthing->up              = axis[2];
   testspawnthing->cgd.tikihandle  = 9999999;
   SetTestEmitterValues();
   }

void CG_TestEmitter_f
   (
   void
   )

   {
   commandManager.TestEmitter();
   }

void CG_DumpEmitter_f
   (
   void
   )

   {
   commandManager.DumpEmitter();
   }

void CG_UpdateTestEmitter
   (
   void
   )

   {
   commandManager.UpdateTestEmitter();
   }


void CG_InitTestEmitter
   (
   void
   )

   {
   cg_te_xangles        = cgi.Cvar_Get( "cg_te_xangles",       "0", 0 ); 
   cg_te_yangles        = cgi.Cvar_Get( "cg_te_yangles",       "0", 0 ); 
   cg_te_zangles        = cgi.Cvar_Get( "cg_te_zangles",       "0", 0 ); 
   cg_te_life           = cgi.Cvar_Get( "cg_te_life",          "1", 0 );
   cg_te_alpha          = cgi.Cvar_Get( "cg_te_alpha",         "1", 0 );
   cg_te_dietouch       = cgi.Cvar_Get( "cg_te_dietouch",      "0", 0 );
   cg_te_bouncefactor   = cgi.Cvar_Get( "cg_te_bouncefactor",  "1", 0 );
   cg_te_scale          = cgi.Cvar_Get( "cg_te_scale",         "1", 0 );
   cg_te_scalemin       = cgi.Cvar_Get( "cg_te_scalemin",      "0", 0 );
   cg_te_scalemax       = cgi.Cvar_Get( "cg_te_scalemax",      "0", 0 );
   cg_te_model          = cgi.Cvar_Get( "cg_te_model",         "none", 0 );
   cg_te_life           = cgi.Cvar_Get( "cg_te_life",          "1", 0 );
   cg_te_color_r        = cgi.Cvar_Get( "cg_te_color_r",       "1", 0 );
   cg_te_color_g        = cgi.Cvar_Get( "cg_te_color_g",       "1", 0 );
   cg_te_color_b        = cgi.Cvar_Get( "cg_te_color_b",       "1", 0 );
   cg_te_startcolor_r   = cgi.Cvar_Get( "cg_te_startcolor_r",  "1", 0 );
   cg_te_startcolor_g   = cgi.Cvar_Get( "cg_te_startcolor_g",  "1", 0 );
   cg_te_startcolor_b   = cgi.Cvar_Get( "cg_te_startcolor_b",  "1", 0 );
   cg_te_endcolor_r     = cgi.Cvar_Get( "cg_te_endcolor_r",    "1", 0 );
   cg_te_endcolor_g     = cgi.Cvar_Get( "cg_te_endcolor_g",    "1", 0 );
   cg_te_endcolor_b     = cgi.Cvar_Get( "cg_te_endcolor_b",    "1", 0 );
   cg_te_accel_x        = cgi.Cvar_Get( "cg_te_accel_x",       "0", 0 );
   cg_te_accel_y        = cgi.Cvar_Get( "cg_te_accel_y",       "0", 0 );
   cg_te_accel_z        = cgi.Cvar_Get( "cg_te_accel_z",       "0", 0 );
   cg_te_count          = cgi.Cvar_Get( "cg_te_count",         "1", 0 );
   cg_te_fade           = cgi.Cvar_Get( "cg_te_fade",          "0", 0 );
   cg_te_fadein         = cgi.Cvar_Get( "cg_te_fadein",        "0", 0 );
   cg_te_spawnrate      = cgi.Cvar_Get( "cg_te_spawnrate",     "1", 0 );   
   cg_te_scalerate      = cgi.Cvar_Get( "cg_te_scalerate",     "0", 0 );
   cg_te_circle         = cgi.Cvar_Get( "cg_te_circle",        "0", 0 );
   cg_te_sphere         = cgi.Cvar_Get( "cg_te_sphere",        "0", 0 );
   cg_te_insphere       = cgi.Cvar_Get( "cg_te_insphere",      "0", 0 );
   cg_te_radius         = cgi.Cvar_Get( "cg_te_radius",        "0", 0 );
   cg_te_swarm          = cgi.Cvar_Get( "cg_te_swarm",         "0", 0 );
   cg_te_wavy           = cgi.Cvar_Get( "cg_te_wavy",          "0", 0 );
   cg_te_align          = cgi.Cvar_Get( "cg_te_align",         "0", 0 );
   cg_te_flickeralpha   = cgi.Cvar_Get( "cg_te_flickeralpha",  "0", 0 );
   cg_te_collision      = cgi.Cvar_Get( "cg_te_collision",     "0", 0 );
   cg_te_globalfade     = cgi.Cvar_Get( "cg_te_globalfade",    "0", 0 );
   cg_te_randomroll     = cgi.Cvar_Get( "cg_te_randomroll",    "0", 0 );

   cg_te_orgoffset_x    = cgi.Cvar_Get( "cg_te_orgoffset_x",   "0", 0 );
   cg_te_orgoffset_y    = cgi.Cvar_Get( "cg_te_orgoffset_y",   "0", 0 );
   cg_te_orgoffset_z    = cgi.Cvar_Get( "cg_te_orgoffset_z",   "0", 0 );
   cg_te_rorgx          = cgi.Cvar_Get( "cg_te_rorgx",         "No", 0 );
   cg_te_rorgy          = cgi.Cvar_Get( "cg_te_rorgy",         "No", 0 );
   cg_te_rorgz          = cgi.Cvar_Get( "cg_te_rorgz",         "No", 0 );

   cg_te_angles_pitch   = cgi.Cvar_Get( "cg_te_angles_pitch",  "0", 0 );
   cg_te_angles_yaw     = cgi.Cvar_Get( "cg_te_angles_yaw",    "0", 0 );
   cg_te_angles_roll    = cgi.Cvar_Get( "cg_te_angles_roll",   "0", 0 );
   cg_te_randpitch      = cgi.Cvar_Get( "cg_te_randpitch",     "No", 0 );
   cg_te_randyaw        = cgi.Cvar_Get( "cg_te_randyaw",       "No", 0 );
   cg_te_randroll       = cgi.Cvar_Get( "cg_te_randroll",      "No", 0 );

   cg_te_randvelx       = cgi.Cvar_Get( "cg_te_randvelx",      "0", 0 );
   cg_te_randvely       = cgi.Cvar_Get( "cg_te_randvely",      "0", 0 );
   cg_te_randvelz       = cgi.Cvar_Get( "cg_te_randvelz",      "0", 0 );
   cg_te_rvxtype        = cgi.Cvar_Get( "cg_te_rvxtype",       "No", 0 );
   cg_te_rvytype        = cgi.Cvar_Get( "cg_te_rvytype",       "No", 0 );
   cg_te_rvztype        = cgi.Cvar_Get( "cg_te_rvztype",       "No", 0 );
   
   cg_te_forwardvelocity = cgi.Cvar_Get( "cg_te_forwardvel",   "0", 0 );
   }


