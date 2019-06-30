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

#include "movegrid.h"

// FIXME: undone class

cMoveGrid::cMoveGrid( int x, int y, int z )
{
	m_iXRes = x;
	m_iYRes = y;
	m_iZRes = z;

	GridPoints = ( gridpoint_t * )gi.Malloc( sizeof( gridpoint_t ) * x * y * z );
}

cMoveGrid::~cMoveGrid()
{
	gi.Free( GridPoints );
}

void cMoveGrid::SetOrientation
	(
	float( *v )[ 3 ]
	)

{
	for( int i = 0; i < 3; i++ )
	{
		VectorCopy( v[ i ], orientation[ i ] );
	}
}

void cMoveGrid::SetMoveInfo
	(
	vmove_t *vm
	)

{
	memcpy( &v, vm, sizeof( vmove_t ) );
}

void cMoveGrid::CalculateBoxPoints
	(
	void
	)

{
	int x, y, z;
	int rx, ry, rz;
	int xdelta, ydelta, zdelta;

	Vector vBoxSize = vBoxSize = Vector(v.maxs) - Vector(v.mins);

	rx = (int)(vBoxSize[0] / (float)m_iXRes);
	ry = (int)(vBoxSize[1] / (float)m_iYRes);
	rz = (int)(vBoxSize[2] / (float)m_iZRes);

	xdelta = ydelta = zdelta = 0;

	for (x = 0; x < m_iXRes; x++)
	{
		for (y = 0; y < m_iYRes; y++)
		{
			for (z = 0; z < m_iZRes; z++)
			{
				gridpoint_t* gridPoint = GetGridPoint(x, y, z);

				gridPoint->origin = Vector(
					(float)(xdelta - rx),
					(float)(ydelta - ry),
					(float)rz * 0.5f
				);
			}
		}
	}

	for (x = 0; x < m_iXRes; x++)
	{
		for (y = 0; y < m_iYRes; y++)
		{
			for (z = 0; z < m_iZRes; z++)
			{
				gridpoint_t* gridPoint = GetGridPoint(x, y, z);

				Vector vTmp;
				MatrixTransformVector(gridPoint->origin, orientation, vTmp);

				gridPoint->origin = vTmp;
				memcpy(&gridPoint->vm, &v, sizeof(gridPoint->vm));
				gridPoint->vm.vs = &gridPoint->vs;

				if (rx >= ry)
				{
					VectorSet(
						gridPoint->vm.mins,
						(float)-ry * 0.5f,
						(float)-ry * 0.5f,
						(float)-rz * 0.5f
					);

					VectorSet(
						gridPoint->vm.maxs,
						(float)ry * 0.5f,
						(float)ry * 0.5f,
						(float)rz * 0.5f
					);
				}
				else
				{
					VectorSet(
						gridPoint->vm.mins,
						(float)-rx * 0.5f,
						(float)-rx * 0.5f,
						(float)-rz * 0.5f
					);

					VectorSet(
						gridPoint->vm.maxs,
						(float)rx * 0.5f,
						(float)rx * 0.5f,
						(float)rz * 0.5f
					);
				}
			}
		}
	}
}

gridpoint_t *cMoveGrid::GetGridPoint
	(
	int x,
	int y,
	int z
	)

{
	return &GridPoints[ x * m_iZRes + y * m_iXRes + z ];
}

void cMoveGrid::Move
	(
	void
	)

{
	int x;
	int y;
	int z;
	Vector vBoxSize;
	Vector old_origin;

	CalculateBoxPoints();

	for( x = 0; x < m_iXRes; x++ )
	{
		for( y = 0; y < m_iYRes; y++ )
		{
			for( z = 0; z < m_iZRes; z++ )
			{
			}
		}
	}

	// this is not called anyways
	// FIXME: stub
	STUB();
}

qboolean cMoveGrid::CheckStuck
	(
	void
	)

{
	int x;
	int y;
	int z;
	trace_t trace;

	for( x = 0; x < m_iXRes; x++ )
	{
		for( y = 0; y < m_iYRes; y++ )
		{
			for( z = 0; z < m_iZRes; z++ )
			{
				gi.Trace( &trace, v.vs->origin, v.mins, v.maxs, v.vs->origin, v.vs->entityNum, v.tracemask, false, false );

				if( trace.allsolid || trace.startsolid || trace.fraction == 0.0f )
				{
					return true;
				}
			}
		}
	}

	return false;
}

void cMoveGrid::GetMoveInfo
	(
	vmove_t *vm
	)

{
	memcpy( vm, &v, sizeof( vmove_t ) );
}

