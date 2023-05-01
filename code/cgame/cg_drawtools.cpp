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
// Some tools used to drawing 2d stuff

#include "cg_local.h"

/*
================
CG_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void CG_AdjustFrom640( float *x, float *y, float *w, float *h ) {
#if 0
	// adjust for wide screens
	if ( cgs.glconfig.vidWidth * 480 > cgs.glconfig.vidHeight * 640 ) {
		*x += 0.5 * ( cgs.glconfig.vidWidth - ( cgs.glconfig.vidHeight * 640 / 480 ) );
	}
#endif
	// scale for screen sizes
	*x *= cgs.screenXScale;
	*y *= cgs.screenYScale;
	*w *= cgs.screenXScale;
	*h *= cgs.screenYScale;
}

/*
=============
CG_TileClearBox

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
void CG_TileClearBox( int x, int y, int w, int h, qhandle_t hShader ) {
	float	s1, t1, s2, t2;

	s1 = x/64.0;
	t1 = y/64.0;
	s2 = (x+w)/64.0;
	t2 = (y+h)/64.0;
	cgi.R_DrawStretchPic( x, y, w, h, s1, t1, s2, t2, hShader );
}



/*
==============
CG_TileClear

Clear around a sized down screen
==============
*/
void CG_TileClear( void ) {
	int		top, bottom, left, right;
	int		w, h;

	w = cgs.glconfig.vidWidth;
	h = cgs.glconfig.vidHeight;

	if ( cg.refdef.x == 0 && cg.refdef.y == 0 && 
		cg.refdef.width == w && cg.refdef.height == h ) {
		return;		// full screen rendering
	}

	top = cg.refdef.y;
	bottom = top + cg.refdef.height-1;
	left = cg.refdef.x;
	right = left + cg.refdef.width-1;

	// clear above view screen
	CG_TileClearBox( 0, 0, w, top, cgs.media.backTileShader );

	// clear below view screen
	CG_TileClearBox( 0, bottom, w, h - bottom, cgs.media.backTileShader );

	// clear left of view screen
	CG_TileClearBox( 0, top, left, bottom - top + 1, cgs.media.backTileShader );

	// clear right of view screen
	CG_TileClearBox( right, top, w - right, bottom - top + 1, cgs.media.backTileShader );
}


/*
===============================================================================

LAGOMETER

===============================================================================
*/

#define	LAG_SAMPLES		128


typedef struct {
	int		frameSamples[LAG_SAMPLES];
	int		frameCount;
	int		snapshotFlags[LAG_SAMPLES];
	int		snapshotSamples[LAG_SAMPLES];
	int		snapshotCount;
} lagometer_t;

lagometer_t		lagometer;

/*
==============
CG_AddLagometerFrameInfo

Adds the current interpolate / extrapolate bar for this frame
==============
*/
void CG_AddLagometerFrameInfo( void ) {
	int			offset;

	offset = cg.time - cg.latestSnapshotTime;
	lagometer.frameSamples[ lagometer.frameCount & ( LAG_SAMPLES - 1) ] = offset;
	lagometer.frameCount++;
}

/*
==============
CG_AddLagometerSnapshotInfo

Each time a snapshot is received, log its ping time and
the number of snapshots that were dropped before it.

Pass NULL for a dropped packet.
==============
*/
void CG_AddLagometerSnapshotInfo( snapshot_t *snap ) {
	// dropped packet
	if ( !snap ) {
		lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = -1;
		lagometer.snapshotCount++;
		return;
	}

	// add this snapshot's info
	lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = snap->ping;
	lagometer.snapshotFlags[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = snap->snapFlags;
	lagometer.snapshotCount++;
}

/*
==============
CG_DrawDisconnect
==============
*/
void CG_DrawDisconnect( void ) {
	float		x, y;
	int			cmdNum;
	usercmd_t	cmd;

	// draw the phone jack if we are completely past our buffers
	cmdNum = cgi.GetCurrentCmdNumber() - CMD_BACKUP + 1;
	cgi.GetUserCmd( cmdNum, &cmd );
	if ( !cg.snap || cmd.serverTime <= cg.snap->ps.commandTime
		|| cmd.serverTime > cg.time ) {	// special check for map_restart
		return;
	}

	// blink it
	if ( ( cg.time >> 9 ) & 1 ) {
		return;
	}

	x = 640 - 48;
	y = 480 - 48;

	cgi.R_DrawStretchPic( x, y, 48, 48, 0, 0, 1, 1, cgi.R_RegisterShader("gfx/2d/net.tga" ) );
}


#define	MAX_LAGOMETER_PING	900
#define	MAX_LAGOMETER_RANGE	300

/*
==============
CG_DrawLagometer
==============
*/
void CG_DrawLagometer( void ) {
	int		a, x, y, i;
	float	v;
	float	ax, ay, aw, ah, mid, range;
	int		color;
	float	vscale;

	if ( !cg_lagometer->integer ) {
      // don't draw the disconnect icon in single player
		//CG_DrawDisconnect();
		return;
	}

	//
	// draw the graph
	//
	x = 640 - 48;
	y = 480 - 48;

	cgi.R_SetColor( NULL );
	cgi.R_DrawStretchPic( x, y, 48, 48, 0, 0, 1, 1, cgs.media.lagometerShader );

	ax = x;
	ay = y;
	aw = 48;
	ah = 48;
	CG_AdjustFrom640( &ax, &ay, &aw, &ah );

	color = -1;
	range = ah / 3;
	mid = ay + range;

	vscale = range / MAX_LAGOMETER_RANGE;

	// draw the frame interpoalte / extrapolate graph
	for ( a = 0 ; a < aw ; a++ ) {
		i = ( lagometer.frameCount - 1 - a ) & (LAG_SAMPLES - 1);
		v = lagometer.frameSamples[i];
		v *= vscale;
		if ( v > 0 ) {
			if ( color != 1 ) {
				color = 1;
				cgi.R_SetColor( g_color_table[ColorIndex(COLOR_YELLOW)] );
			}
			if ( v > range ) {
				v = range;
			}
			cgi.R_DrawBox( ax + aw - a, mid - v, 1, v );
		} else if ( v < 0 ) {
			if ( color != 2 ) {
				color = 2;
				cgi.R_SetColor( g_color_table[ColorIndex(COLOR_BLUE)] );
			}
			v = -v;
			if ( v > range ) {
				v = range;
			}
			cgi.R_DrawBox( ax + aw - a, mid, 1, v );
		}
	}

	// draw the snapshot latency / drop graph
	range = ah / 2;
	vscale = range / MAX_LAGOMETER_PING;

	for ( a = 0 ; a < aw ; a++ ) {
		i = ( lagometer.snapshotCount - 1 - a ) & (LAG_SAMPLES - 1);
		v = lagometer.snapshotSamples[i];
		if ( v > 0 ) {
			if ( lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED ) {
				if ( color != 5 ) {
					color = 5;	// YELLOW for rate delay
					cgi.R_SetColor( g_color_table[ColorIndex(COLOR_YELLOW)] );
				}
			} else {
				if ( color != 3 ) {
					color = 3;
					cgi.R_SetColor( g_color_table[ColorIndex(COLOR_GREEN)] );
				}
			}
			v = v * vscale;
			if ( v > range ) {
				v = range;
			}
			cgi.R_DrawBox( ax + aw - a, ay + ah - v, 1, v );
		} else if ( v < 0 ) {
			if ( color != 4 ) {
				color = 4;		// RED for dropped snapshots
				cgi.R_SetColor( g_color_table[ColorIndex(COLOR_RED)] );
			}
			cgi.R_DrawBox( ax + aw - a, ay + ah - range, 1, range );
		}
	}

	cgi.R_SetColor( NULL );

	CG_DrawDisconnect();
}

/*
==============
CG_DrawIcons
==============
*/
void CG_DrawIcons( void ) 
   {
   qhandle_t handle;
   float x,y,w,h;

   if ( paused->integer )
      {
      handle = cgs.media.pausedShader;
      }
   else
      {
      if ( cg.predicted_player_state.pm_flags & PMF_LEVELEXIT )
         {
	      // blink it
	      if ( ( cg.time >> 9 ) & 1 ) 
            {
		      return;
	         }
         handle = cgs.media.levelExitShader;
         }
      else
         {
         return;
         }
      }
   w = cgi.R_GetShaderWidth( handle );
   h = cgi.R_GetShaderHeight( handle );
   x = w / 4;
   y = cgs.glconfig.vidHeight - ( 1.25f * h );

	cgi.R_SetColor( colorWhite );
   cgi.R_DrawStretchPic( x, y, w, h, 0, 0, 1, 1, handle );
   }

void CG_DrawOverlayTopBottom(qhandle_t handleTop, qhandle_t handleBottom, float fAlpha)
{
	int iHalfWidth;
	int iWidthOffset;
	vec4_t color;

	// FIXME: UNIMPLEMENTED
}

void CG_DrawOverlayMiddle(qhandle_t handle, float fAlpha)
{
    int iHalfWidth;
    int iWidthOffset;
    vec4_t color;

    // FIXME: UNIMPLEMENTED
}

void CG_DrawOverlayFullScreen(qhandle_t handle, float fAlpha)
{
    int iHalfWidth;
    int iWidthOffset;
    vec4_t color;

    // FIXME: UNIMPLEMENTED
}

void CG_DrawZoomOverlay()
{
    static int zoomType;
    static float fAlpha;
	const char* weaponstring;
    qboolean bDrawOverlay;

    // FIXME: UNIMPLEMENTED
}

void CG_HudDrawShader(int iInfo)
{
    if (cgi.HudDrawElements[iInfo].shaderName[0]) {
         cgi.HudDrawElements[iInfo].hShader = cgi.R_RegisterShaderNoMip(cgi.HudDrawElements[iInfo].shaderName);
    } else {
         cgi.HudDrawElements[iInfo].hShader = 0;
    }
}

void CG_HudDrawFont(int iInfo)
{
    if (cgi.HudDrawElements[iInfo].fontName[0]) {
         cgi.HudDrawElements[iInfo].pFont = cgi.R_LoadFont(cgi.HudDrawElements[iInfo].fontName);
    } else {
         cgi.HudDrawElements[iInfo].pFont = nullptr;
    }
}

void CG_RefreshHudDrawElements()
{
    int i;

    for (i = 0; i < MAX_HUDDRAW_ELEMENTS; ++i) {
         CG_HudDrawShader(i);
         CG_HudDrawFont(i);
    }
}

void CG_HudDrawElements()
{
    // FIXME: UNIMPLEMENTED
}

void CG_InitializeObjectives()
{
	int i;

    cg.ObjectivesAlphaTime = 0.0;
    cg.ObjectivesBaseAlpha = 0.0;
    cg.ObjectivesDesiredAlpha = 0.0;
    cg.ObjectivesCurrentAlpha = 0.0;

	for (i = 0; i < MAX_OBJECTIVES; i++)
	{
        cg.Objectives[i].flags = 0;
        cg.Objectives[i].text[0] = 0;
	}
}

void CG_DrawObjectives()
{
    // FIXME: unimplemented
}

void CG_DrawPlayerTeam()
{
    // FIXME: unimplemented
}

void CG_DrawPlayerEntInfo()
{
    // FIXME: unimplemented
}

void CG_UpdateAttackerDisplay()
{
    // FIXME: unimplemented
}

void CG_UpdateCountdown()
{
    // FIXME: unimplemented
}

void CG_DrawStopwatch()
{
    // FIXME: unimplemented
}

void CG_DrawInstantMessageMenu()
{
    // FIXME: unimplemented
}

void CG_DrawSpectatorView()
{
    // FIXME: unimplemented
}

/*
==============
CG_Draw2D
==============
*/
void CG_Draw2D(void)
{
	CG_UpdateCountdown();
	CG_DrawZoomOverlay();
	CG_DrawLagometer();
	CG_HudDrawElements();
	CG_DrawObjectives();
	CG_DrawIcons();
	CG_DrawStopwatch();
	CG_DrawSpectatorView();
	CG_DrawPlayerTeam();
	CG_DrawPlayerEntInfo();
	CG_UpdateAttackerDisplay();
	CG_DrawInstantMessageMenu();
}
