/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay

#include "cg_local.h"

vec4_t menu_text_color		= {1.0f, 1.0f, 1.0f, 1.0f};
vec4_t menu_dim_color		= {0.0f, 0.0f, 0.0f, 0.75f};
vec4_t color_black			= {0.00f, 0.00f, 0.00f, 1.00f};
vec4_t color_white			= {1.00f, 1.00f, 1.00f, 1.00f};
vec4_t color_yellow			= {1.00f, 1.00f, 0.00f, 1.00f};
vec4_t color_blue			= {0.00f, 0.00f, 1.00f, 1.00f};
vec4_t color_lightOrange	= {1.00f, 0.68f, 0.00f, 1.00f };
vec4_t color_orange			= {1.00f, 0.43f, 0.00f, 1.00f};
vec4_t color_red			= {1.00f, 0.00f, 0.00f, 1.00f};
vec4_t color_transred		= {1.00f, 0.00f, 0.00f, 0.60f};
vec4_t color_transreddk		= {1.00f, 0.00f, 0.30f, 0.60f};
vec4_t color_dim			= {0.00f, 0.00f, 0.00f, 0.25f};
vec4_t color_gray			= {0.00f, 0.00f, 0.00f, 0.6f};
vec4_t color_green			= {0.10f, 0.70f, 0.10f, 1.00f};
vec4_t color_transgreen		= {0.10f, 0.70f, 0.10f, 0.60f};
vec4_t color_transgreendk	= {0.10f, 0.70f, 0.30f, 0.60f};
vec4_t color_dkgreen		= {0.40f, 0.40f, 0.40f, 1.00f};

// for printing game messages
vec4_t	color_hud			= {0.70f, 0.60f, 0.05f, 1.00f};
vec4_t	color_grey			= {0.75f, 0.75f, 0.75f, 1.00f};
vec4_t	color_lightRed		= {0.70f, 0.50f, 0.50f, 1.00f};


int drawTeamOverlayModificationCount = -1;


int sortedTeamPlayers[TEAM_MAXOVERLAY];
int	numSortedTeamPlayers;

char systemChat[256];
char teamChat1[256];
char teamChat2[256];

/*
================
CG_Draw3DModel

================
*/
void CG_Draw3DModel( float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles ) {
	refdef_t		refdef;
	refEntity_t		ent;

	if ( !cg_draw3dIcons->integer || !cg_drawIcons->integer ) {
		return;
	}

	CG_AdjustFrom640( &x, &y, &w, &h );

	memset( &refdef, 0, sizeof( refdef ) );

	memset( &ent, 0, sizeof( ent ) );
	AnglesToAxis( angles, ent.axis );
	VectorCopy( origin, ent.origin );
	ent.model = model;
	ent.customSkin = skin;
	ent.renderfx &= ~RF_SHADOW;		// no stencil shadows

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear( refdef.viewaxis );

	refdef.fov_x = 30;
	refdef.fov_y = 30;

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.time = cg.time;

	cgi.R_ClearScene();
	cgi.R_AddRefEntityToScene( &ent );
	cgi.R_RenderScene( &refdef );
}


/*
===========================================================================================

  UPPER RIGHT CORNER

===========================================================================================
*/

/*
================
CG_DrawAttacker

su44: draw attacker client name and team
flag in the left down corner of the screen
================
*/
#define ATTACKER_X 80
#define ATTACKER_Y 400
static void CG_DrawAttacker() {
	int clientNum;
	clientInfo_t *info;
	qhandle_t handle;

	if ( cg_drawAttacker->integer == 0)
		return;

	clientNum = cg.predictedPlayerState.stats[STAT_ATTACKERCLIENT];
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS || clientNum == cg.snap->ps.clientNum ) {
		return;
	}
	
	info = &cgs.clientinfo[clientNum];

	cgi.R_SetColor( color_red );
	cgi.R_Text_Paint(&cgs.media.verdana, ATTACKER_X,ATTACKER_Y,1,0,info->name,0,0,qfalse,qtrue);
	cgi.R_SetColor( NULL );

	if(info->team == TEAM_AXIS) {
		handle = cgi.R_RegisterShader("textures/hud/axis");
		CG_DrawPic(ATTACKER_X-30,ATTACKER_Y-4,24,24,handle);
	} else if(info->team == TEAM_ALLIES) {
		handle = cgi.R_RegisterShader("textures/hud/allies");
		CG_DrawPic(ATTACKER_X-30,ATTACKER_Y-4,24,24,handle);
	}
}

/*
==================
CG_DrawSnapshot
==================
*/
static float CG_DrawSnapshot( float y ) {
	char		*s;
	int			w;

	s = va( "time:%i snap:%i cmd:%i", cg.snap->serverTime, 
		cg.latestSnapshotNum, cgs.serverCommandSequence );
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

	CG_DrawBigString( 635 - w, y + 2, s, 1.0F);

	return y + BIGCHAR_HEIGHT + 4;
}

/*
==================
CG_DrawFPS
==================
*/
#define	FPS_FRAMES	4
static float CG_DrawFPS( float y ) {
	char		*s;
	int			w;
	static int	previousTimes[FPS_FRAMES];
	static int	index;
	int		i, total;
	int		fps;
	static	int	previous;
	int		t, frameTime;

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = cgi.Milliseconds();
	frameTime = t - previous;
	previous = t;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;
	if ( index > FPS_FRAMES ) {
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for ( i = 0 ; i < FPS_FRAMES ; i++ ) {
			total += previousTimes[i];
		}
		if ( !total ) {
			total = 1;
		}
		fps = 1000 * FPS_FRAMES / total;

		s = va( "%ifps", fps );
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

		CG_DrawBigString( 635 - w, y + 2, s, 1.0F);
	}

	return y + BIGCHAR_HEIGHT + 4;
}

/*
=================
CG_DrawTimer
=================
*/
static float CG_DrawTimer( float y ) {
	char		*s;
	int			w;
	int			mins, seconds, tens;
	int			msec;

	msec = cg.time - cgs.levelStartTime;

	seconds = msec / 1000;
	mins = seconds / 60;
	seconds -= mins * 60;
	tens = seconds / 10;
	seconds -= tens * 10;

	s = va( "%i:%i%i", mins, tens, seconds );
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

	CG_DrawBigString( 635 - w, y + 2, s, 1.0F);

	return y + BIGCHAR_HEIGHT + 4;
}


/*
=================
CG_DrawTeamOverlay
=================
*/

static float CG_DrawTeamOverlay( float y, qboolean right, qboolean upper ) {
//	int x, w, h, xx;
//	int i, j, len;
//	const char *p;
//	vec4_t		hcolor;
//	int pwidth, lwidth;
//	int plyrs;
//	char st[16];
//	clientInfo_t *ci;
//	int ret_y, count;
//
//	if ( !cg_drawTeamOverlay->integer ) {
//		return y;
//	}
//
//	if ( cg.snap->ps.stats[STAT_TEAM] != TEAM_RED && cg.snap->ps.stats[STAT_TEAM] != TEAM_BLUE ) {
//		return y; // Not on any team
//	}
//
//	plyrs = 0;
//
//	// max player name width
//	pwidth = 0;
//	count = (numSortedTeamPlayers > 8) ? 8 : numSortedTeamPlayers;
//	for (i = 0; i < count; i++) {
//		ci = cgs.clientinfo + sortedTeamPlayers[i];
//		if ( ci->infoValid && ci->team == cg.snap->ps.stats[STAT_TEAM]) {
//			plyrs++;
//			len = CG_DrawStrlen(ci->name);
//			if (len > pwidth)
//				pwidth = len;
//		}
//	}
//
//	if (!plyrs)
//		return y;
//
//	if (pwidth > TEAM_OVERLAY_MAXNAME_WIDTH)
//		pwidth = TEAM_OVERLAY_MAXNAME_WIDTH;
//
//	// max location name width
//	lwidth = 0;
//	for (i = 1; i < MAX_LOCATIONS; i++) {
//		p = CG_ConfigString(CS_LOCATIONS + i);
//		if (p && *p) {
//			len = CG_DrawStrlen(p);
//			if (len > lwidth)
//				lwidth = len;
//		}
//	}
//
//	if (lwidth > TEAM_OVERLAY_MAXLOCATION_WIDTH)
//		lwidth = TEAM_OVERLAY_MAXLOCATION_WIDTH;
//
//	w = (pwidth + lwidth + 4 + 7) * TINYCHAR_WIDTH;
//
//	if ( right )
//		x = 640 - w;
//	else
//		x = 0;
//
//	h = plyrs * TINYCHAR_HEIGHT;
//
//	if ( upper ) {
//		ret_y = y + h;
//	} else {
//		y -= h;
//		ret_y = y;
//	}
//
//	if ( cg.snap->ps.stats[STAT_TEAM] == TEAM_RED ) {
//		hcolor[0] = 1.0f;
//		hcolor[1] = 0.0f;
//		hcolor[2] = 0.0f;
//		hcolor[3] = 0.33f;
//	} else { // if ( cg.snap->ps.stats[STAT_TEAM] == TEAM_BLUE )
//		hcolor[0] = 0.0f;
//		hcolor[1] = 0.0f;
//		hcolor[2] = 1.0f;
//		hcolor[3] = 0.33f;
//	}
//	cgi.R_SetColor( hcolor );
//	CG_DrawPic( x, y, w, h, cgs.media.teamStatusBar );
//	cgi.R_SetColor( NULL );
//
//	for (i = 0; i < count; i++) {
//		ci = cgs.clientinfo + sortedTeamPlayers[i];
//		if ( ci->infoValid && ci->team == cg.snap->ps.stats[STAT_TEAM]) {
//
//			hcolor[0] = hcolor[1] = hcolor[2] = hcolor[3] = 1.0;
//
//			xx = x + TINYCHAR_WIDTH;
//
//			CG_DrawStringExt( xx, y,
//				ci->name, hcolor, qfalse, qfalse,
//				TINYCHAR_WIDTH, TINYCHAR_HEIGHT, TEAM_OVERLAY_MAXNAME_WIDTH);
//
//			if (lwidth) {
//				p = CG_ConfigString(CS_LOCATIONS + ci->location);
//				if (!p || !*p)
//					p = "unknown";
//				len = CG_DrawStrlen(p);
//				if (len > lwidth)
//					len = lwidth;
//
////				xx = x + TINYCHAR_WIDTH * 2 + TINYCHAR_WIDTH * pwidth + 
////					((lwidth/2 - len/2) * TINYCHAR_WIDTH);
//				xx = x + TINYCHAR_WIDTH * 2 + TINYCHAR_WIDTH * pwidth;
//				CG_DrawStringExt( xx, y,
//					p, hcolor, qfalse, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
//					TEAM_OVERLAY_MAXLOCATION_WIDTH);
//			}
//
//			CG_GetColorForHealth( ci->health, ci->armor, hcolor );
//
//			Com_sprintf (st, sizeof(st), "%3i %3i", ci->health,	ci->armor);
//
//			xx = x + TINYCHAR_WIDTH * 3 + 
//				TINYCHAR_WIDTH * pwidth + TINYCHAR_WIDTH * lwidth;
//
//			CG_DrawStringExt( xx, y,
//				st, hcolor, qfalse, qfalse,
//				TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0 );
//
//			// draw weapon icon
//			xx += TINYCHAR_WIDTH * 3;
//
//			if ( cg_weapons[ci->curWeapon].weaponIcon ) {
//				CG_DrawPic( xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 
//					cg_weapons[ci->curWeapon].weaponIcon );
//			} else {
//				CG_DrawPic( xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 
//					cgs.media.deferShader );
//			}
//
//			// Draw powerup icons
//			if (right) {
//				xx = x;
//			} else {
//				xx = x + w - TINYCHAR_WIDTH;
//			}
//			for (j = 0; j <= PW_NUM_POWERUPS; j++) {
//				if (ci->powerups & (1 << j)) {
//
//					item = BG_FindItemForPowerup( j );
//
//					if (item) {
//						CG_DrawPic( xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 
//						cgi.R_RegisterShader( item->icon ) );
//						if (right) {
//							xx -= TINYCHAR_WIDTH;
//						} else {
//							xx += TINYCHAR_WIDTH;
//						}
//					}
//				}
//			}
//
//			y += TINYCHAR_HEIGHT;
//		}
//	}
//
//	return ret_y;
////#endif
return y;
}


/*
=====================
CG_DrawUpperRight

=====================
*/
static void CG_DrawUpperRight( void ) {
	float	y;

	y = 0;

	if ( cgs.gametype >= GT_TEAM && cg_drawTeamOverlay->integer == 1 ) {
		y = CG_DrawTeamOverlay( y, qtrue, qtrue );
	} 
	if ( cg_drawSnapshot->integer ) {
		y = CG_DrawSnapshot( y );
	}
	if ( cg_drawFPS->integer ) {
		y = CG_DrawFPS( y );
	}
	if ( cg_drawTimer->integer ) {
		y = CG_DrawTimer( y );
	}
}

/*
===========================================================================================

  LOWER RIGHT CORNER

===========================================================================================
*/


//
///*
//=====================
//CG_DrawLowerRight
//
//=====================
//*/
//#ifndef MISSIONPACK
//static void CG_DrawLowerRight( void ) {
//	float	y;
//
//	y = 480 - ICON_SIZE;
//
//	if ( cgs.gametype >= GT_TEAM && cg_drawTeamOverlay->integer == 2 ) {
//		y = CG_DrawTeamOverlay( y, qtrue, qfalse );
//	} 
//
//	y = CG_DrawScores( y );
//	y = CG_DrawPowerups( y );
//}
//#endif // MISSIONPACK

//===========================================================================================


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

Should we draw something differnet for long lag vs no packets?
==============
*/
static void CG_DrawDisconnect( void ) {
	float		x, y;
	int			cmdNum;
	usercmd_t	cmd;
	const char		*s;
	int			w;

	// draw the phone jack if we are completely past our buffers
	cmdNum = cgi.GetCurrentCmdNumber() - CMD_BACKUP + 1;
	cgi.GetUserCmd( cmdNum, &cmd );
	if ( cmd.serverTime <= cg.snap->ps.commandTime
		|| cmd.serverTime > cg.time ) {	// special check for map_restart
		return;
	}

	// also add text in center of screen
	s = "Connection Interrupted";
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
	CG_DrawBigString( 320 - w/2, 100, s, 1.0F);

	// blink the icon
	if ( ( cg.time >> 9 ) & 1 ) {
		return;
	}

	x = 640 - 48;
	y = 480 - 48;

	CG_DrawPic( x, y, 48, 48, cgi.R_RegisterShader("gfx/2d/net.tga" ) );
}


#define	MAX_LAGOMETER_PING	900
#define	MAX_LAGOMETER_RANGE	300

/*
==============
CG_DrawLagometer
==============
*/
static void CG_DrawLagometer( void ) {
	int		a, x, y, i;
	float	v;
	float	ax, ay, aw, ah, mid, range;
	int		color;
	float	vscale;

	if ( !cg_lagometer->integer || cgs.localServer ) {
		CG_DrawDisconnect();
		return;
	}

	//
	// draw the graph
	//
#ifdef MISSIONPACK
	x = 640 - 48;
	y = 480 - 144;
#else
	x = 640 - 48;
	y = 480 - 48;
#endif

	cgi.R_SetColor( NULL );
//	CG_DrawPic( x, y, 48, 48, cgs.media.lagometerShader );

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
			cgi.R_DrawStretchPic ( ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		} else if ( v < 0 ) {
			if ( color != 2 ) {
				color = 2;
				cgi.R_SetColor( g_color_table[ColorIndex(COLOR_BLUE)] );
			}
			v = -v;
			if ( v > range ) {
				v = range;
			}
			cgi.R_DrawStretchPic( ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
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
			cgi.R_DrawStretchPic( ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		} else if ( v < 0 ) {
			if ( color != 4 ) {
				color = 4;		// RED for dropped snapshots
				cgi.R_SetColor( g_color_table[ColorIndex(COLOR_RED)] );
			}
			cgi.R_DrawStretchPic( ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, cgs.media.whiteShader );
		}
	}

	cgi.R_SetColor( NULL );

	if ( cg_nopredict->integer || cg_synchronousClients->integer ) {
		CG_DrawBigString( ax, ay, "snc", 1.0 );
	}

	CG_DrawDisconnect();
}



/*
===============================================================================

CENTER PRINTING

===============================================================================
*/


/*
==============
CG_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void CG_CenterPrint( const char *str, int y, int charWidth ) {
	char	*s;

	Q_strncpyz( cg.centerPrint, str, sizeof(cg.centerPrint) );

	cg.centerPrintTime = cg.time;
	cg.centerPrintY = y;
	cg.centerPrintCharWidth = charWidth;

	// count the number of lines for centering
	cg.centerPrintLines = 1;
	s = cg.centerPrint;
	while( *s ) {
		if (*s == '\n')
			cg.centerPrintLines++;
		s++;
	}
}

void CG_LocationPrint( const char *str, int x, int y, int charWidth ) {
	char	*s;

	Q_strncpyz( cg.locationPrint, str, sizeof(cg.locationPrint) );

	cg.locationPrintTime = cg.time;
	cg.locationPrintX = x;
	cg.locationPrintY = y;
	cg.locationPrintCharWidth = charWidth;

	// count the number of lines for centering
	cg.locationPrintLines = 1;
	s = cg.locationPrint;
	while( *s ) {
		if (*s == '\n')
			cg.locationPrintLines++;
		s++;
	}
}


/*
===================
CG_DrawCenterString
===================
*/
static void CG_DrawCenterString( void ) {
	char	*start;
	int		l;
	int		x, y, w;
	float	*color;

	if ( !cg.centerPrintTime ) {
		return;
	}

	color = CG_FadeColor( cg.centerPrintTime, 1000 * cg_centertime->value );
	if ( !color ) {
		return;
	}

	cgi.R_SetColor( color );

	start = cg.centerPrint;

	y = cg.centerPrintY - cg.centerPrintLines * BIGCHAR_HEIGHT / 2;

	while ( 1 ) {
		char linebuffer[1024];

		for ( l = 0; l < 50; l++ ) {
			if ( !start[l] || start[l] == '\n' ) {
				break;
			}
			linebuffer[l] = start[l];
		}
		linebuffer[l] = 0;


		w = cg.centerPrintCharWidth * CG_DrawStrlen( linebuffer );

		x = ( SCREEN_WIDTH - w ) / 2;

		CG_DrawStringExt( x, y, linebuffer, color, qfalse, qtrue,
			cg.centerPrintCharWidth, (int)(cg.centerPrintCharWidth * 1.5), 0 );

		y += cg.centerPrintCharWidth * 1.5;

		while ( *start && ( *start != '\n' ) ) {
			start++;
		}
		if ( !*start ) {
			break;
		}
		start++;
	}

	cgi.R_SetColor( NULL );
}

/*
===================
CG_DrawLocationString
===================
*/
static void CG_DrawLocationString( void ) {
	char	*start;
	int		l;
	int		x, y, w;
	float	*color;

	if ( !cg.locationPrintTime ) {
		return;
	}

	color = CG_FadeColor( cg.locationPrintTime, 1000 * cg_locationtime->value );
	if ( !color ) {
		return;
	}

	cgi.R_SetColor( color );

	start = cg.locationPrint;

	y = cg.locationPrintY - cg.locationPrintLines * BIGCHAR_HEIGHT / 2;

	while ( 1 ) {
		char linebuffer[1024];

		for ( l = 0; l < 50; l++ ) {
			if ( !start[l] || start[l] == '\n' ) {
				break;
			}
			linebuffer[l] = start[l];
		}
		linebuffer[l] = 0;

		w = cg.locationPrintCharWidth * CG_DrawStrlen( linebuffer );

		x = ( SCREEN_WIDTH - w ) / 2;

		CG_DrawStringExt( x, y, linebuffer, color, qfalse, qtrue,
			cg.locationPrintCharWidth, (int)(cg.locationPrintCharWidth * 1.5), 0 );

		y += cg.locationPrintCharWidth * 1.5;

		while ( *start && ( *start != '\n' ) ) {
			start++;
		}
		if ( !*start ) {
			break;
		}
		start++;
	}

	cgi.R_SetColor( NULL );
}

/*
===================
CG_DrawGameMessages
===================
*/
#define CG_GAMEMESSAGETIMEOUT 4000
static void CG_DrawGameMessages( void ) {
	int i;

	if ( cg.time - cg.gameMessageTime > CG_GAMEMESSAGETIMEOUT
		&& cg.gameMessagePtr1 != cg.gameMessagePtr2 ) {
		cg.gameMessagePtr1=(cg.gameMessagePtr1+1)%MAX_GAMEMESSAGES;
		cg.gameMessageTime = cg.time;
		if ( cg.gameMessagePtr1 == (cg.gameMessagePtr2+1)%MAX_GAMEMESSAGES )
			cg.gameMessagePtr2 = (cg.gameMessagePtr2+1)%MAX_GAMEMESSAGES;
	}
	for (i=0;i<MAX_GAMEMESSAGES;i++) {
		int ptr;

		ptr = (i+cg.gameMessagePtr1)%MAX_GAMEMESSAGES;
		if ( ptr == cg.gameMessagePtr2 )
			break;

		if ( cg.gameMessageTypes[ptr] == SMT_YELLOW )
			cgi.R_SetColor( color_hud );
		else
			cgi.R_SetColor( color_white );
		if ( cg.gameMessages[ptr] ) {
			cgi.R_Text_Paint( &cgs.media.facfont, 0, 128 + i*14, 1, 1, cg.gameMessages[ptr], 1, 0, qfalse, qtrue );
		}
	}
	cgi.R_SetColor( NULL );
}

/*
===================
CG_DrawChatDeathMessages
===================
*/
static void CG_DrawChatDeathMessages( void ) {
	int i;

	if ( cg.time - cg.chatDeathMessageTime > CG_GAMEMESSAGETIMEOUT
		&& cg.chatDeathMessagePtr1 != cg.chatDeathMessagePtr2 ) {
		cg.chatDeathMessagePtr1 = (cg.chatDeathMessagePtr1+1)%MAX_CHATDEATHMESSAGES;
		cg.chatDeathMessageTime = cg.time;
		if ( cg.chatDeathMessagePtr1 == (cg.chatDeathMessagePtr2+1)%MAX_CHATDEATHMESSAGES )
			cg.chatDeathMessagePtr2 = (cg.chatDeathMessagePtr2+1)%MAX_CHATDEATHMESSAGES;
	}
	for (i=0;i<MAX_CHATDEATHMESSAGES;i++) {
		int ptr;

		ptr = (i+cg.chatDeathMessagePtr1)%MAX_CHATDEATHMESSAGES;
		if ( ptr == cg.chatDeathMessagePtr2 )
			break;

		if ( cg.chatDeathMessageTypes[ptr] == SMT_DEATH )
			cgi.R_SetColor( color_lightRed );
		else
			cgi.R_SetColor( color_grey );

		if ( cg.chatDeathMessages[ptr] ) {
			cgi.R_Text_Paint( &cgs.media.facfont, 128, i*14, 1, 1, cg.chatDeathMessages[ptr], 1, 0, qfalse, qtrue );
		}
	}
	cgi.R_SetColor( NULL );
}


/*
================================================================================

CROSSHAIR

Wombat: Crosshair is drawn by the UI module,
since MOHAA defines its crosshair in a menu file

================================================================================
*/


//==============================================================================

/*
=================
CG_DrawSpectator
=================
*/
static void CG_DrawSpectator(void) {
	CG_DrawBigString(320 - 9 * 8, 440, "SPECTATOR", 1.0F);
}

/*
=================
CG_DrawIntermission
=================
*/
static void CG_DrawIntermission( void ) {
//	int key;

	if ( cgs.gametype == GT_SINGLE_PLAYER ) {
		CG_DrawCenterString();
		CG_DrawLocationString();
		return;
	}
	cg.scoreFadeTime = cg.time;
}

/*
=================
CG_DrawFollow
=================
*/
static qboolean CG_DrawFollow( void ) {
	float		x;
	vec4_t		color;
	const char	*name;

	if ( !(cg.snap->ps.pm_flags & PMF_SPECTATE_FOLLOW) ) {
		return qfalse;
	}
	color[0] = 1;
	color[1] = 1;
	color[2] = 1;
	color[3] = 1;


	CG_DrawBigString( 320 - 9 * 8, 24, "following", 1.0F );

	name = cgs.clientinfo[ cg.snap->ps.clientNum ].name;

	x = 0.5 * ( 640 - GIANT_WIDTH * CG_DrawStrlen( name ) );

	CG_DrawStringExt( x, 40, name, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );

	return qtrue;
}




/*
=================
CG_DrawWarmup
=================
*/
static void CG_DrawWarmup( void ) {
	int			w;
	int			sec;
	float scale;
	int			cw;
	const char	*s;

	sec = cg.warmup;
	if ( !sec ) {
		return;
	}

	if ( sec < 0 ) {
		s = "Waiting for players";		
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
		CG_DrawBigString(320 - w / 2, 24, s, 1.0F);
		cg.warmupCount = 0;
		return;
	}


	{
		if ( cgs.gametype == GT_FFA ) {
			s = "Free For All";
		} else if ( cgs.gametype == GT_TEAM ) {
			s = "Team Deathmatch";
		} else {
			s = "";
		}
#ifdef MISSIONPACK
		w = CG_Text_Width(s, 0.6f, 0);
		CG_Text_Paint(320 - w / 2, 90, 0.6f, colorWhite, s, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE);
#else
		w = CG_DrawStrlen( s );
		if ( w > 640 / GIANT_WIDTH ) {
			cw = 640 / w;
		} else {
			cw = GIANT_WIDTH;
		}
		CG_DrawStringExt( 320 - w * cw/2, 25,s, colorWhite, 
				qfalse, qtrue, cw, (int)(cw * 1.1f), 0 );
#endif
	}

	sec = ( sec - cg.time ) / 1000;
	if ( sec < 0 ) {
		cg.warmup = 0;
		sec = 0;
	}
	s = va( "Starts in: %i", sec + 1 );
	if ( sec != cg.warmupCount ) {
		cg.warmupCount = sec;
		switch ( sec ) {
		case 0:
//			cgi.S_StartLocalSound( cgs.media.count1Sound, CHAN_ANNOUNCER );
			break;
		case 1:
//			cgi.S_StartLocalSound( cgs.media.count2Sound, CHAN_ANNOUNCER );
			break;
		case 2:
//			cgi.S_StartLocalSound( cgs.media.count3Sound, CHAN_ANNOUNCER );
			break;
		default:
			break;
		}
	}
	scale = 0.45f;
	switch ( cg.warmupCount ) {
	case 0:
		cw = 28;
		scale = 0.54f;
		break;
	case 1:
		cw = 24;
		scale = 0.51f;
		break;
	case 2:
		cw = 20;
		scale = 0.48f;
		break;
	default:
		cw = 16;
		scale = 0.45f;
		break;
	}

#ifdef MISSIONPACK
		w = CG_Text_Width(s, scale, 0);
		CG_Text_Paint(320 - w / 2, 125, scale, colorWhite, s, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE);
#else
	w = CG_DrawStrlen( s );
	CG_DrawStringExt( 320 - w * cw/2, 70, s, colorWhite, 
			qfalse, qtrue, cw, (int)(cw * 1.5), 0 );
#endif
}

//==================================================================================

char* extractValues( char **start ) {
	char *end, *ret;

	ret = *start;
	end = strstr( *start, " " );
	if (end) {
		*end = 0;
		*start = end+1;
	}

	return ret;
}

/*
=================
CG_DrawScores

Draw the normal in-game scoreboard
=================
*/
void CG_DrawScores( void ) {
	int i, numScores;
	char *ptr;
	char buffer[MAX_STRING_CHARS];
	fontInfo_t *font;
	int facfontheight, verdanaheight;

	if ( cg.scoresRequestTime + 1000 < cg.time ) {
		// the scores are more than two seconds out of data,
		// so request new ones
		cg.scoresRequestTime = cg.time;
		cgi.SendClientCommand( "score" );
	}

	Q_strncpyz( buffer, cg.aScore, sizeof(buffer) );
	CG_DrawStdBox( 32, 56, 384, 392, qfalse );

	ptr = buffer;
	numScores = atoi( extractValues( &ptr ) );

	facfontheight = 20;
	verdanaheight = 14;

	if ( cgs.gametype == GT_FFA || cgs.gametype == GT_SINGLE_PLAYER ) {
		font = &cgs.media.verdana;
		for ( i=0; i<numScores; i++ ) {
			int clientNum;

			if ( 56+i*verdanaheight >= 448 )
				break;
			clientNum = atoi( extractValues( &ptr ) );
			cgi.R_Text_Paint( font, 32, 56 + i*verdanaheight, 1, 1, Info_ValueForKey( CG_ConfigString( CS_PLAYERS + clientNum ), "name" ), 1, 32, qtrue, qfalse ); // name
			cgi.R_Text_Paint( font, 160, 56 + i*verdanaheight, 1, 1, extractValues( &ptr ), 1, 32, qtrue, qfalse );
			cgi.R_Text_Paint( font, 224, 56 + i*verdanaheight, 1, 1, extractValues( &ptr ), 1, 32, qtrue, qfalse );
			cgi.R_Text_Paint( font, 288, 56 + i*verdanaheight, 1, 1, extractValues( &ptr ), 1, 32, qtrue, qfalse );
			cgi.R_Text_Paint( font, 352, 56 + i*verdanaheight, 1, 1, extractValues( &ptr ), 1, 32, qtrue, qfalse );
		}
	} else {
		for ( i=0; i<numScores; i++ ) {
			int clientNum;
			teamtype_t team;

			if ( 56+i*verdanaheight >= 448 )
				break;
			clientNum = atoi( extractValues( &ptr ) );
			team = (teamtype_t)atoi( extractValues( &ptr ) );
			switch ( team ) {
				case TEAM_AXIS:
					if ( clientNum == -1 )
						cgi.R_SetColor( color_transreddk );
					else
						cgi.R_SetColor( color_transred );
					CG_DrawBox( 33,56+i*verdanaheight,384,verdanaheight,qfalse);
					cgi.R_SetColor( NULL );
					break;
				case TEAM_ALLIES:
					if ( clientNum == -1 )
						cgi.R_SetColor( color_transgreendk );
					else
						cgi.R_SetColor( color_transgreen );
					CG_DrawBox( 33,56+i*verdanaheight,384,verdanaheight,qfalse);
					cgi.R_SetColor( NULL );
					break;
				default: break;
			}
			if ( clientNum == -1 ) {
				font = &cgs.media.facfont;
				if ( team == TEAM_ALLIES )
					cgi.R_Text_Paint( font, 32, 56 + i*verdanaheight, 1, 1, "Allies", 1, 32, qtrue, qfalse );
				else if ( team == TEAM_AXIS )
					cgi.R_Text_Paint( font, 32, 56 + i*verdanaheight, 1, 1, "Axis", 1, 32, qtrue, qfalse );
				else if ( team == TEAM_SPECTATOR )
					cgi.R_Text_Paint( font, 32, 56 + i*verdanaheight, 1, 1, "Spectators", 1, 32, qtrue, qfalse );
			} else {
				font = &cgs.media.verdana;
				cgi.R_Text_Paint( font, 32, 56 + i*verdanaheight, 1, 1, Info_ValueForKey( CG_ConfigString( CS_PLAYERS + clientNum ), "name" ), 1, 32, qtrue, qfalse ); // name
			}
			cgi.R_Text_Paint( font, 160, 56 + i*verdanaheight, 1, 1, extractValues( &ptr ), 1, 32, qtrue, qfalse );
			cgi.R_Text_Paint( font, 224, 56 + i*verdanaheight, 1, 1, extractValues( &ptr ), 1, 32, qtrue, qfalse );
			cgi.R_Text_Paint( font, 288, 56 + i*verdanaheight, 1, 1, extractValues( &ptr ), 1, 32, qtrue, qfalse );
			cgi.R_Text_Paint( font, 352, 56 + i*verdanaheight, 1, 1, extractValues( &ptr ), 1, 32, qtrue, qfalse );
		}
	}
}
/*
=================
CG_DrawPlayerTeam

Draws team icon of local player
=================
*/
void CG_DrawPlayerTeam() {
	//if ( cg_hud->integer )
	{
		if ( cg.snap ) {
			if ( cgs.gametype > GT_FFA ) {
				qhandle_t handle = 0;

				if ( cg.snap->ps.stats[STAT_TEAM] == TEAM_ALLIES ) {
					handle = cgi.R_RegisterShader("textures/hud/allies");
				} else {
					if ( cg.snap->ps.stats[STAT_TEAM] == TEAM_AXIS )
						handle = cgi.R_RegisterShader("textures/hud/axis");
				}
				if ( handle ) {
					cgi.R_SetColor(0);
					cgi.R_DrawStretchPic(96.f, (cgs.glconfig.vidHeight - 46), 24.f, 24.f, 0.f, 0.f, 1.f, 1.f, handle);
				}
			}
		}
	}
}
static void CG_DrawOverlayTopBottom(qhandle_t handleTop, qhandle_t handleBottom, float fAlpha) {
	float halfH;
	float ofs;
	float ofsHR;
	vec4_t color;

	color[0] = 1.f;
	color[1] = 1.f;
	color[2] = 1.f;
	color[3] = fAlpha;
	cgi.R_SetColor(color);

	halfH = cgs.glconfig.vidHeight >> 1;
	ofs = (cgs.glconfig.vidWidth - cgs.glconfig.vidHeight) >> 1;
	ofsHR = ofs + halfH;

	// draw overlay square
	cgi.R_DrawStretchPic( ofs, 0, halfH, halfH, 1.f, 0, 0, 1.f, handleTop);
	cgi.R_DrawStretchPic( ofsHR, 0, halfH, halfH, 0, 0, 1.f, 1.f, handleTop);
	cgi.R_DrawStretchPic( ofs, halfH, halfH, halfH, 1.f, 0, 0, 1.f, handleBottom);
	cgi.R_DrawStretchPic( ofsHR, halfH, halfH, halfH, 0, 0, 1.f, 1.f, handleBottom);

	// draw black borders
	VectorSet(color,0,0,0);
	cgi.R_SetColor(color);
	cgi.R_DrawStretchPic( 0, 0, ofs, cgs.glconfig.vidHeight, 0, 0, 1.f, 1.f, cgs.media.blackShader);
	cgi.R_DrawStretchPic( cgs.glconfig.vidWidth - ofs, 0, ofs, cgs.glconfig.vidHeight,
		0, 0, 1.f, 1.f, cgs.media.blackShader);
}

static void CG_DrawOverlayMiddle(qhandle_t handle, float fAlpha ) {
	float halfH;
	float ofs;
	float ofsHR;
	float color[4];

	color[0] = 1.f;
	color[1] = 1.f;
	color[2] = 1.f;
	// su44: thats a hack, such a thing is not done in MoHAA.
	// probably something is wrong with our shader system
	// maybe "textures/hud/zoomoverlay" cGen is broken?
	color[3] = 1.f - fAlpha; // should be: "color[3] = fAlpha;
	cgi.R_SetColor(color);

	halfH = cgs.glconfig.vidHeight >> 1;
	ofs = (cgs.glconfig.vidWidth - cgs.glconfig.vidHeight) >> 1;
	ofsHR = ofs + halfH;

	// draw overlay square
	cgi.R_DrawStretchPic(ofs, 0, halfH, halfH, 0, 0, 1.f, 1.f, handle);
	cgi.R_DrawStretchPic( ofsHR, 0, halfH, halfH, 1.f, 0, 0, 1.f, handle);
	cgi.R_DrawStretchPic( ofs, halfH, halfH, halfH, 0, 1.f, 1.f, 0, handle);
	cgi.R_DrawStretchPic( ofsHR, halfH, halfH, halfH, 1.f, 1.f, 0, 0, handle);

	// draw black borders
	VectorSet(color,0,0,0);
	color[3] = fAlpha;
	cgi.R_SetColor(color);
	cgi.R_DrawStretchPic( 0, 0, ofs, cgs.glconfig.vidHeight, 0, 0, 1.f, 1.f, cgs.media.blackShader);
	cgi.R_DrawStretchPic(cgs.glconfig.vidWidth - ofs, 0, cgs.glconfig.vidWidth - ofs,
		cgs.glconfig.vidHeight, 0, 0, 1.f, 1.f, cgs.media.blackShader);
}
static void CG_DrawOverlayFullScreen(qhandle_t handle, float fAlpha) {
	float halfH;
	float halfW;
	vec4_t color;

	VectorSet(color,1.f,1.f,1.f);
	color[3] = fAlpha;
	cgi.R_SetColor(color);

	halfH = cgs.glconfig.vidHeight >> 1;
	halfW = cgs.glconfig.vidWidth >> 1;

	cgi.R_DrawStretchPic( 0, 0, halfW, halfH, 1.f, 0, 0, 1.f, handle );
	cgi.R_DrawStretchPic( halfW, 0, halfW, halfH, 1.f, 0, 0, 1.f, handle );
	cgi.R_DrawStretchPic( 0, halfH, halfW, halfH, 0, 1.f, 1.f, 0, handle );
	cgi.R_DrawStretchPic( halfW, halfH, halfW, halfH, 1.f, 1.f, 0, 0, handle );
}

/*
=================
CG_DrawZoomOverlay
=================
*/
static float cg_alpha;
static int cg_zoomType;
static void CG_DrawZoomOverlay() {
	const char *itemName;
	int drawOverlay;
	int itemIndex;
	signed int inZoom;
	snapshot_t *snap;

	snap = cg.snap;
	itemName = "";
	drawOverlay = 1;
	if ( snap ) {
		itemIndex = snap->ps.activeItems[ITEM_WEAPON];
		if ( itemIndex >= 0 ) {
			itemName = CG_ConfigString(CS_WEAPONS + itemIndex);
		}

		if ( strcmp(itemName, "Spy Camera") ) {
			if ( strcmp(itemName, "Binoculars") ) {
				inZoom = cg.snap->ps.stats[STAT_INZOOM];
				if ( inZoom && inZoom <= 30 ) {
					if ( strcmp(itemName, "KAR98 - Sniper") )
						cg_zoomType = 0;
					else
						cg_zoomType = 1;
				} else {
					drawOverlay = 0;
				}
			} else {
				cg_zoomType = 3;
			}
		} else {
			cg_zoomType = 2;
		}
		if ( drawOverlay ) {
			cg_alpha += cg.frametime * 0.015;
			if ( cg_alpha > 1.0 )
				cg_alpha = 1.0;
drawOverlayLabel:
			if ( cg_zoomType == 1 ) {
				CG_DrawOverlayTopBottom(cgs.media.kar98TopOverlayShader, cgs.media.kar98BottomOverlayShader, cg_alpha);
			} else {
				if ( cg_zoomType > 1 ) {
					if ( cg_zoomType != 2 ) {
						if ( cg_zoomType == 3 )
							CG_DrawOverlayFullScreen(cgs.media.binocularsOverlayShader, cg_alpha);
					}
				} else {
					if ( !cg_zoomType )
						CG_DrawOverlayMiddle(cgs.media.zoomOverlayShader, cg_alpha);
				}
			}
			return;
		}
		cg_alpha -= cg.frametime * 0.015;
		if(cg_alpha < 0) {
			cg_alpha = 0;
		} else {
			goto drawOverlayLabel;
		}
	}
}
/*
=================
CG_DrawCrosshairPlayerInfo
=================
*/
static void CG_DrawCrosshairPlayerInfo() {
	int clientNum;
	clientInfo_t *info;

	clientNum = cg.snap->ps.stats[STAT_INFOCLIENT];
	
	if(clientNum == -1)
		return;

	info = &cgs.clientinfo[clientNum];

	cgi.R_SetColor(color_green);
	cgi.R_Text_Paint( &cgs.media.facfont, 64, 280, 1, 1, info->name, 1, 0, qfalse, qfalse );
	cgi.R_SetColor(color_green);
	cgi.R_Text_Paint( &cgs.media.facfont, 64, 296, 1, 1, va("%i",
		cg.snap->ps.stats[ STAT_INFOCLIENT_HEALTH ] ), 1, 0, qfalse, qfalse );

}
/*
=================
CG_Draw2D

Draws CG related 2D stuff on top of HUD
=================
*/
void CG_Draw2D( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback ) {

	// if we are taking a levelshot for the menu, don't draw anything
	if ( cg.levelShot ) {
		return;
	}

	if ( cg_draw2D->integer == 0 ) {
		return;
	}

	CG_DrawLagometer();
	CG_DrawGameMessages();
	CG_DrawChatDeathMessages();

	// su44: draw player's team icon (axis or allies)
	CG_DrawPlayerTeam();
	// su44: draw custom hud elements set by cg messages
	CG_HudDrawElements();
	// su44: draw sniper rifles/binoculars zoom overlay
	CG_DrawZoomOverlay();
	// su44: draw info about the player local client is currently aiming at
	CG_DrawCrosshairPlayerInfo();
	// su44: draw attacker client name and team flag in the left down corner of the screen
	CG_DrawAttacker();




	CG_DrawUpperRight();


	//CG_DrawLowerRight();
	//CG_DrawLowerLeft();

/*	if ( !CG_DrawFollow() ) {
		CG_DrawWarmup();
	}*/

	// don't draw center string if scoreboard is up
	if ( !cg.scoreBoardShowing) {
		CG_DrawCenterString();
		CG_DrawLocationString();
	}
	else
		CG_DrawScores();
}


static void CG_DrawTourneyScoreboard( void ) {

}

/*
=====================
CG_DrawActive

Perform all drawing needed to completely fill the screen
=====================
*/
void CG_DrawActive( stereoFrame_t stereoView ) {
	float		separation;
	vec3_t		baseOrg;

	// optionally draw the info screen instead
	if ( !cg.snap ) {
		CG_DrawInformation();
		return;
	}

	switch ( stereoView ) {
	case STEREO_CENTER:
		separation = 0;
		break;
	case STEREO_LEFT:
		separation = -cg_stereoSeparation->value / 2;
		break;
	case STEREO_RIGHT:
		separation = cg_stereoSeparation->value / 2;
		break;
	default:
		separation = 0;
		CG_Error( "CG_DrawActive: Undefined stereoView" );
	}


	// clear around the rendered view if sized down
	CG_TileClear();

	// offset vieworg appropriately if we're doing stereo separation
	VectorCopy( cg.refdef.vieworg, baseOrg );
	if ( separation != 0 ) {
		VectorMA( cg.refdef.vieworg, -separation, cg.refdef.viewaxis[1], cg.refdef.vieworg );
	}

	// draw 3D view
	cgi.R_RenderScene( &cg.refdef );

	// restore original viewpoint if running stereo
	if ( separation != 0 ) {
		VectorCopy( baseOrg, cg.refdef.vieworg );
	}

	// draw status bar and other floating elements
// 	CG_Draw2D();
	// Draw2D is now called by client directly after UI
}

// Wombat
/*
=================
UI_DrawBox
UI_DrawStdBox - gray box with borders
ctrCoord == qtrue: draw box with center at x,y
ctrCoord == qfalse: box with upper left corner at x,y
=================
*/
#define LINE_THICKNESS	1
void	CG_DrawBox( int x, int y, int w, int h, qboolean ctrCoord ) {

	if ( ctrCoord ) {
		x -= w/2;
		y -= h/2;
	}

//	cgi.R_SetColor( color_gray );
	cgi.R_DrawStretchPic( x*cgs.screenXScale, y*cgs.screenYScale, w*cgs.screenXScale, h*cgs.screenYScale, 0, 0, 16, 16, cgs.media.whiteShader );
//	cgi.R_SetColor( color_white );
//	cgi.R_DrawStretchPic( x*cgs.screenXScale, y*cgs.screenYScale, w*cgs.screenXScale, LINE_THICKNESS*cgs.screenYScale, 0, 0, 32, 32, cgs.media.blackShader );
//	cgi.R_DrawStretchPic( x*cgs.screenXScale, y*cgs.screenYScale, LINE_THICKNESS*cgs.screenXScale, h*cgs.screenYScale, 0, 0, 32, 32, cgs.media.blackShader );
//	cgi.R_DrawStretchPic( x*cgs.screenXScale, (y+h)*cgs.screenYScale, w*cgs.screenXScale, LINE_THICKNESS*cgs.screenYScale, 0, 0, 32, 32, cgs.media.blackShader );
//	cgi.R_DrawStretchPic( (x+w)*cgs.screenXScale, y*cgs.screenYScale, LINE_THICKNESS*cgs.screenXScale, (h+LINE_THICKNESS)*cgs.screenYScale, 0, 0, 32, 32, cgs.media.blackShader );
//	cgi.R_SetColor( NULL );
}

void	CG_DrawStdBox( int x, int y, int w, int h, qboolean ctrCoord ) {

	if ( ctrCoord ) {
		x -= w/2;
		y -= h/2;
	}

	cgi.R_SetColor( color_gray );
	cgi.R_DrawStretchPic( x*cgs.screenXScale, y*cgs.screenYScale, w*cgs.screenXScale, h*cgs.screenYScale, 0, 0, 16, 16, cgs.media.blackShader );
	cgi.R_SetColor( color_white );
	cgi.R_DrawStretchPic( x*cgs.screenXScale, y*cgs.screenYScale, w*cgs.screenXScale, LINE_THICKNESS*cgs.screenYScale, 0, 0, 32, 32, cgs.media.blackShader );
	cgi.R_DrawStretchPic( x*cgs.screenXScale, y*cgs.screenYScale, LINE_THICKNESS*cgs.screenXScale, h*cgs.screenYScale, 0, 0, 32, 32, cgs.media.blackShader );
	cgi.R_DrawStretchPic( x*cgs.screenXScale, (y+h)*cgs.screenYScale, w*cgs.screenXScale, LINE_THICKNESS*cgs.screenYScale, 0, 0, 32, 32, cgs.media.blackShader );
	cgi.R_DrawStretchPic( (x+w)*cgs.screenXScale, y*cgs.screenYScale, LINE_THICKNESS*cgs.screenXScale, (h+LINE_THICKNESS)*cgs.screenYScale, 0, 0, 32, 32, cgs.media.blackShader );
	cgi.R_SetColor( NULL );
}





