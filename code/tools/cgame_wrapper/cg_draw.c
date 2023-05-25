/*
===========================================================================
Copyright (C) 2012 Michael Rieder

This file is part of OpenMohaa source code.

OpenMohaa source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMohaa source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMohaa source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
#include "qcommon.h"

fontheader_t *facfont;

#define CROSS_LINE_LEN	10
void CG_MakeCross( vec_t *point ) {
	vec3_t start;
	vec3_t end;

	// X
	VectorCopy( point, start );
	VectorCopy( point, end );
	start[0]	-= CROSS_LINE_LEN;
	end[0]		+= CROSS_LINE_LEN;
	cgi.R_DebugLine( start, end, 1, 0, 0, 1 );

	// Y
	VectorCopy( point, start );
	VectorCopy( point, end );
	start[1]	-= CROSS_LINE_LEN;
	end[1]		+= CROSS_LINE_LEN;
	cgi.R_DebugLine( start, end, 0, 1, 0, 1 );

	// Z
	VectorCopy( point, start );
	VectorCopy( point, end );
	start[2]	-= CROSS_LINE_LEN;
	end[2]		+= CROSS_LINE_LEN;
	cgi.R_DebugLine( start, end, 0, 0, 1, 1 );
}

// su44: used for drawing bone orientations
#define AX_LEN 4.f
void CG_DrawAxisAtPoint(vec3_t p, vec3_t axis[3]) {
	vec3_t f,r,u;
	VectorMA(p,AX_LEN,axis[0],f);
	VectorMA(p,AX_LEN,axis[1],r);
	VectorMA(p,AX_LEN,axis[2],u);

	cgi.R_DebugLine( p, f, 1, 0, 0, 1 );
	cgi.R_DebugLine( p, r, 0, 1, 0, 1 );
	cgi.R_DebugLine( p, u, 0, 0, 1, 1 );
}

int GetSnapshot ( int snapshotNumber, snapshot_t *snap ) {
	snapshot = snap;
	return cgi.GetSnapshot( snapshotNumber, snap );
}

qhandle_t R_RegisterModel ( char *name ) {
	qhandle_t ret;

	ret = cgi.R_RegisterModel( name );
	return ret;
}


void CG_Draw2D() {
	cge.CG_Draw2D();

	// 2D drawing on top of cgame's 2D
	cgi.R_DrawString( facfont, "Hooked", 10, 140, 7, qtrue );
}

void CG_DrawActiveFrame ( int serverTime, int frametime, stereoFrame_t stereoView, qboolean demoPlayback ) {
	// Issue 3D drawing here before letting cgame draw the frame

	cge.CG_DrawActiveFrame( serverTime, frametime, stereoView, demoPlayback );
}