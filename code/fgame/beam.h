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

// beam.h: Beam

#ifndef __BEAM_H__
#define __BEAM_H__

#include "g_local.h"
#include "scriptslave.h"

extern Event EV_FuncBeam_Activate;
extern Event EV_FuncBeam_Deactivate;
extern Event EV_FuncBeam_Diameter;
extern Event EV_FuncBeam_Maxoffset;
extern Event EV_FuncBeam_Minoffset;
extern Event EV_FuncBeam_Overlap;
extern Event EV_FuncBeam_Color;
extern Event EV_FuncBeam_SetTarget;
extern Event EV_FuncBeam_SetAngle;
extern Event EV_FuncBeam_SetEndPoint;
extern Event EV_FuncBeam_SetLife;
extern Event EV_FuncBeam_Shader;
extern Event EV_FuncBeam_Segments;
extern Event EV_FuncBeam_Delay;
extern Event EV_FuncBeam_NumSphereBeams;
extern Event EV_FuncBeam_SphereRadius;
extern Event EV_FuncBeam_ToggleDelay;
extern Event EV_FuncBeam_FindEndpoint;
extern Event EV_FuncBeam_EndAlpha;

class FuncBeam : public ScriptSlave
{
protected:
	SimpleEntityPtr		end, origin_target;
	float				damage;
	float				life;
	Vector				end_point;
	qboolean  			use_angles;
	float				shootradius;
	str					shader;

	bool				m_bIgnoreWalls;

public:
	CLASS_PROTOTYPE( FuncBeam );

	FuncBeam();

	void		SetAngle( Event *ev );
	void		SetAngles( Event *ev );
	void		SetEndPoint( Event *ev );
	void		SetModel( Event *ev );
	void		SetDamage( Event *ev );
	void		SetOverlap( Event *ev );
	void		SetBeamStyle( Event *ev );
	void		SetLife( Event *ev );
	void		Activate( Event *ev );
	void		Deactivate( Event *ev );
	void		SetDiameter( Event *ev );
	void		SetMaxoffset( Event *ev );
	void		SetMinoffset( Event *ev );
	void		SetColor( Event *ev );
	void		SetSegments( Event *ev );
	void		SetBeamShader( str shader );
	void		SetBeamShader( Event *ev );
	void		SetBeamTileShader( Event *ev );
	void		SetDelay( Event *ev );
	void		SetToggleDelay( Event *ev );
	void		SetSphereRadius( Event *ev );
	void		SetNumSphereBeams( Event *ev );
	void		SetEndAlpha( Event *ev );
	void		SetShootRadius( Event *ev );
	void		SetPersist( Event *ev );
	void		FindEndpoint( Event *ev );
	void		UpdateEndpoint( Event *ev );
	void		UpdateOrigin( Event *ev );
	void		Shoot( Event *ev );
	void		SetIgnoreWalls( Event *ev );

	void setAngles( Vector ang ) override;
	void Archive( Archiver &arc ) override;

	FuncBeam* CreateBeam(const char* model, const char* shader, Vector start, Vector end, int numsegments = 4, float scale = 1.0f, float life = 1.0f, float damage = 0.0f, Entity* origin_target = NULL);
	friend FuncBeam* CreateBeam(const char* model, const char* shader, Vector start, Vector end, int numsegments, float scale, float life, float damage, Entity* origin_target);
};

inline void FuncBeam::Archive
	(
	Archiver &arc
	)
   {
   ScriptSlave::Archive( arc );
   arc.ArchiveSafePointer( &end );
   arc.ArchiveSafePointer( &origin_target );
   arc.ArchiveFloat( &damage );
   arc.ArchiveFloat( &life );
   arc.ArchiveVector( &end_point );
   arc.ArchiveBoolean( &use_angles );
   arc.ArchiveFloat( &shootradius );
   arc.ArchiveString( &shader );
   if ( arc.Loading() )
      {
      SetBeamShader( shader );
      }
   }

#endif // __BEAM_H__
