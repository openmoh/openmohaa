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

// movegrid.h: Move grid
//

#ifndef __MOVEGRID_H__
#define __MOVEGRID_H__

#include "g_local.h"
#include "../qcommon/class.h"
#include "../fgame/archive.h"

typedef struct vehicleState_s {
	float origin[ 3 ];
	float velocity[ 3 ];
	int groundEntityNum;
	qboolean walking;
	qboolean groundPlane;
	trace_t groundTrace;
	int entityNum;
	float desired_dir[ 2 ];
	qboolean hit_obstacle;
	float hit_origin[ 3 ];
	float obstacle_normal[ 3 ];
	qboolean useGravity;
} vehicleState_t;

typedef struct vmove_s {
	vehicleState_t *vs;
	float frametime;
	float desired_speed;
	int tracemask;
	int numtouch;
	int touchents[ 32 ];
	float mins[ 3 ];
	float maxs[ 3 ];
} vmove_t;

typedef struct gridpoint_s {
	Vector origin;
	Vector neworigin;
	Vector origindelta;
	Vector changed;
	Vector newvel;
	gentity_t *groundentity;
	vmove_t vm;
	vehicleState_t vs;
} gridpoint_t;

class cMoveGrid : public Class
{
public:
	gridpoint_t *GridPoints;

private:
	vmove_t v;
	float orientation[ 3 ][ 3 ];
	int m_iXRes;
	int m_iYRes;
	int m_iZRes;

public:
	cMoveGrid( int x, int y, int z );
	virtual ~cMoveGrid();

	void			SetOrientation( float( *v)[ 3 ] );
	void			SetMoveInfo( vmove_t *vm );
	void			CalculateBoxPoints( void );
	gridpoint_t		*GetGridPoint( int x, int y, int z );
	void			Move( void );
	qboolean		CheckStuck( void );
	void			GetMoveInfo( vmove_t *vm );
	void			Archive( Archiver& arc );
};

inline void cMoveGrid::Archive
	(
	Archiver& arc
	)

{
	arc.ArchiveInteger( &m_iXRes );
	arc.ArchiveInteger( &m_iYRes );
	arc.ArchiveInteger( &m_iZRes );
	arc.ArchiveRaw( &v, sizeof( vmove_t ) );
	arc.ArchiveVec3( orientation[ 0 ] );
	arc.ArchiveVec3( orientation[ 1 ] );
	arc.ArchiveVec3( orientation[ 2 ] );

	if( arc.Loading() )
	{
		if( GridPoints )
		{
			if( m_iXRes != 3 && m_iYRes != 3 && m_iZRes != 1 )
			{
				gi.Free( GridPoints );
				GridPoints = ( gridpoint_t * )gi.Malloc( sizeof( gridpoint_t ) * m_iXRes * m_iYRes * m_iZRes );
			}
		}
	}
}

#endif
