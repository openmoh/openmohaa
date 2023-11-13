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
void CG_AdjustFrom640(float *x, float *y, float *w, float *h)
{
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
void CG_TileClearBox(int x, int y, int w, int h, qhandle_t hShader)
{
    float s1, t1, s2, t2;

    s1 = x / 64.0;
    t1 = y / 64.0;
    s2 = (x + w) / 64.0;
    t2 = (y + h) / 64.0;
    cgi.R_DrawStretchPic(x, y, w, h, s1, t1, s2, t2, hShader);
}

/*
==============
CG_TileClear

Clear around a sized down screen
==============
*/
void CG_TileClear(void)
{
    int top, bottom, left, right;
    int w, h;

    w = cgs.glconfig.vidWidth;
    h = cgs.glconfig.vidHeight;

    if (cg.refdef.x == 0 && cg.refdef.y == 0 && cg.refdef.width == w && cg.refdef.height == h) {
        return; // full screen rendering
    }

    top    = cg.refdef.y;
    bottom = top + cg.refdef.height - 1;
    left   = cg.refdef.x;
    right  = left + cg.refdef.width - 1;

    // clear above view screen
    CG_TileClearBox(0, 0, w, top, cgs.media.backTileShader);

    // clear below view screen
    CG_TileClearBox(0, bottom, w, h - bottom, cgs.media.backTileShader);

    // clear left of view screen
    CG_TileClearBox(0, top, left, bottom - top + 1, cgs.media.backTileShader);

    // clear right of view screen
    CG_TileClearBox(right, top, w - right, bottom - top + 1, cgs.media.backTileShader);
}

/*
===============================================================================

LAGOMETER

===============================================================================
*/

#define LAG_SAMPLES 128

typedef struct {
    int frameSamples[LAG_SAMPLES];
    int frameCount;
    int snapshotFlags[LAG_SAMPLES];
    int snapshotSamples[LAG_SAMPLES];
    int snapshotCount;
} lagometer_t;

lagometer_t lagometer;

/*
==============
CG_AddLagometerFrameInfo

Adds the current interpolate / extrapolate bar for this frame
==============
*/
void CG_AddLagometerFrameInfo(void)
{
    int offset;

    offset                                                           = cg.time - cg.latestSnapshotTime;
    lagometer.frameSamples[lagometer.frameCount & (LAG_SAMPLES - 1)] = offset;
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
void CG_AddLagometerSnapshotInfo(snapshot_t *snap)
{
    // dropped packet
    if (!snap) {
        lagometer.snapshotSamples[lagometer.snapshotCount & (LAG_SAMPLES - 1)] = -1;
        lagometer.snapshotCount++;
        return;
    }

    // add this snapshot's info
    lagometer.snapshotSamples[lagometer.snapshotCount & (LAG_SAMPLES - 1)] = snap->ping;
    lagometer.snapshotFlags[lagometer.snapshotCount & (LAG_SAMPLES - 1)]   = snap->snapFlags;
    lagometer.snapshotCount++;
}

/*
==============
CG_DrawDisconnect
==============
*/
void CG_DrawDisconnect(void)
{
    float     x, y;
    float     w, h;
    int       cmdNum;
    qhandle_t handle;
    usercmd_t cmd;

    // draw the phone jack if we are completely past our buffers
    cmdNum = cgi.GetCurrentCmdNumber() - CMD_BACKUP + 1;
    cgi.GetUserCmd(cmdNum, &cmd);
    if (!cg.snap || cmd.serverTime <= cg.snap->ps.commandTime
        || cmd.serverTime > cg.time) { // special check for map_restart
        return;
    }

    // blink it
    if ((cg.time >> 9) & 1) {
        return;
    }

    handle = cgi.R_RegisterShader("gfx/2d/net.tga");
    w      = cgi.R_GetShaderWidth(handle);
    h      = cgi.R_GetShaderHeight(handle);
    x      = ((float)cgs.glconfig.vidWidth - w) * 0.5;
    y      = (float)cgs.glconfig.vidHeight - h;

    cgi.R_DrawStretchPic(x, y, w, h, 0, 0, 1, 1, handle);
}

#define MAX_LAGOMETER_PING  900
#define MAX_LAGOMETER_RANGE 300

/*
==============
CG_DrawLagometer
==============
*/
void CG_DrawLagometer(void)
{
    int   a, i;
    float v;
    float ax, ay, aw, ah, mid, range;
    int   color;
    float vscale;

    if (!cg_lagometer->integer) {
        CG_DrawDisconnect();
        return;
    }

    //
    // draw the graph
    //
    ax = 272.0;
    ay = 432.0;
    aw = 96.0;
    ah = 48.0;
    CG_AdjustFrom640(&ax, &ay, &aw, &ah);

    cgi.R_SetColor(NULL);
    cgi.R_DrawStretchPic(ax, ay, aw, ah, 0, 0, 1, 1, cgs.media.lagometerShader);

    color = -1;
    range = ah / 3;
    mid   = ay + range;

    vscale = range / MAX_LAGOMETER_RANGE;

    // draw the frame interpoalte / extrapolate graph
    for (a = 0; a < aw; a++) {
        i = (lagometer.frameCount - 1 - a) & (LAG_SAMPLES - 1);
        v = lagometer.frameSamples[i];
        v *= vscale;
        if (v > 0) {
            if (color != 1) {
                color = 1;
                cgi.R_SetColor(g_color_table[ColorIndex(COLOR_YELLOW)]);
            }
            if (v > range) {
                v = range;
            }
            cgi.R_DrawBox(ax + aw - a, mid - v, 1, v);
        } else if (v < 0) {
            if (color != 2) {
                color = 2;
                cgi.R_SetColor(g_color_table[ColorIndex(COLOR_BLUE)]);
            }
            v = -v;
            if (v > range) {
                v = range;
            }
            cgi.R_DrawBox(ax + aw - a, mid, 1, v);
        }
    }

    // draw the snapshot latency / drop graph
    range  = ah / 2;
    vscale = range / MAX_LAGOMETER_PING;

    for (a = 0; a < aw; a++) {
        i = (lagometer.snapshotCount - 1 - a) & (LAG_SAMPLES - 1);
        v = lagometer.snapshotSamples[i];
        if (v > 0) {
            if (lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED) {
                if (color != 5) {
                    color = 5; // YELLOW for rate delay
                    cgi.R_SetColor(g_color_table[ColorIndex(COLOR_YELLOW)]);
                }
            } else {
                if (color != 3) {
                    color = 3;
                    cgi.R_SetColor(g_color_table[ColorIndex(COLOR_GREEN)]);
                }
            }
            v = v * vscale;
            if (v > range) {
                v = range;
            }
            cgi.R_DrawBox(ax + aw - a, ay + ah - v, 1, v);
        } else if (v < 0) {
            if (color != 4) {
                color = 4; // RED for dropped snapshots
                cgi.R_SetColor(g_color_table[ColorIndex(COLOR_RED)]);
            }
            cgi.R_DrawBox(ax + aw - a, ay + ah - range, 1, range);
        }
    }

    cgi.R_SetColor(NULL);

    CG_DrawDisconnect();
}

static void CG_DrawPauseIcon()
{
    qhandle_t handle;
    float     x, y, w, h;

    if (paused->integer) {
        handle = cgs.media.pausedShader;
    } else {
        if (cg.predicted_player_state.pm_flags & PMF_LEVELEXIT) {
            // blink it
            if ((cg.time >> 9) & 1) {
                return;
            }
            handle = cgs.media.levelExitShader;
        } else {
            return;
        }
    }
    w = cgi.R_GetShaderWidth(handle);
    h = cgi.R_GetShaderHeight(handle);
    if (cg.snap && cg.snap->ps.blend[3] > 0) {
        y = cgs.glconfig.vidHeight * 0.45f - h / 2.f;
    } else {
        y = cgs.glconfig.vidHeight * 0.75f - h / 2.f;
    }
    x = (cgs.glconfig.vidWidth - w) / 2.f;

    cgi.R_SetColor(colorWhite);
    cgi.R_DrawStretchPic(x, y, w, h, 0, 0, 1, 1, handle);
}

static void CG_DrawServerLag()
{
    float     x, y;
    float     w, h;
    qhandle_t handle;

    if (!cg_drawsvlag->integer) {
        return;
    }

    if (!developer->integer && !cgs.gametype) {
        return;
    }

    if (!cgs.serverLagTime) {
        return;
    }

    if (cg.time - cgs.serverLagTime > 3000) {
        return;
    }

    // blink it
    if ((cg.time >> 9) & 1) {
        return;
    }

    handle = cgi.R_RegisterShader("gfx/2d/slowserver");
    w      = (float)cgi.R_GetShaderWidth(handle) / 4;
    h      = (float)cgi.R_GetShaderHeight(handle) / 4;
    x      = ((float)cgs.glconfig.vidWidth - w) / 2;
    y      = (float)cgs.glconfig.vidHeight - h;
    cgi.R_DrawStretchPic(x, y, w, h, 0.0, 0.0, 1.0, 1.0, handle);
}

/*
==============
CG_DrawIcons
==============
*/
void CG_DrawIcons(void)
{
    if (!cg_hud->integer) {
        return;
    }

    CG_DrawPauseIcon();
    CG_DrawServerLag();
}

void CG_DrawOverlayTopBottom(qhandle_t handleTop, qhandle_t handleBottom, float fAlpha)
{
    int    iHalfWidth;
    int    iWidthOffset;
    vec4_t color;

    color[0] = 1.0;
    color[1] = 1.0;
    color[2] = 1.0;
    color[3] = fAlpha;
    cgi.R_SetColor(color);

    iHalfWidth   = cgs.glconfig.vidHeight >> 1;
    iWidthOffset = (cgs.glconfig.vidWidth - cgs.glconfig.vidHeight) >> 1;

    cgi.R_DrawStretchPic(iWidthOffset, 0.0, iHalfWidth, iHalfWidth, 1.0, 0.0, 0.0, 1.0, handleTop);
    cgi.R_DrawStretchPic(iWidthOffset + iHalfWidth, 0.0, iHalfWidth, iHalfWidth, 0.0, 0.0, 1.0, 1.0, handleTop);
    cgi.R_DrawStretchPic(iWidthOffset, iHalfWidth, iHalfWidth, iHalfWidth, 1.0, 0.0, 0.0, 1.0, handleBottom);
    cgi.R_DrawStretchPic(
        iWidthOffset + iHalfWidth, iHalfWidth, iHalfWidth, iHalfWidth, 0.0, 0.0, 1.0, 1.0, handleBottom
    );

    color[0] = 0.0;
    color[1] = 0.0;
    color[2] = 0.0;
    cgi.R_SetColor(color);

    cgi.R_DrawStretchPic(0.0, 0.0, iWidthOffset, cgs.glconfig.vidHeight, 0.0, 0.0, 1.0, 1.0, cgs.media.lagometerShader);
    cgi.R_DrawStretchPic(
        cgs.glconfig.vidWidth - iWidthOffset,
        0.0,
        iWidthOffset,
        cgs.glconfig.vidHeight,
        0.0,
        0.0,
        1.0,
        1.0,
        cgs.media.lagometerShader
    );
}

void CG_DrawOverlayMiddle(qhandle_t handle, float fAlpha)
{
    int    iHalfWidth;
    int    iWidthOffset;
    vec4_t color;

    color[0] = 1.0;
    color[1] = 1.0;
    color[2] = 1.0;
    color[3] = fAlpha;
    cgi.R_SetColor(color);

    iHalfWidth   = cgs.glconfig.vidHeight >> 1;
    iWidthOffset = (cgs.glconfig.vidWidth - cgs.glconfig.vidHeight) >> 1;

    cgi.R_DrawStretchPic(iWidthOffset, 0.0, iHalfWidth, iHalfWidth, 0.0, 0.0, 1.0, 1.0, handle);
    cgi.R_DrawStretchPic(iWidthOffset + iHalfWidth, 0.0, iHalfWidth, iHalfWidth, 1.0, 0.0, 0.0, 1.0, handle);
    cgi.R_DrawStretchPic(iWidthOffset, iHalfWidth, iHalfWidth, iHalfWidth, 0.0, 1.0, 1.0, 0.0, handle);
    cgi.R_DrawStretchPic(iWidthOffset + iHalfWidth, iHalfWidth, iHalfWidth, iHalfWidth, 1.0, 1.0, 0.0, 0.0, handle);

    color[0] = 0.0;
    color[1] = 0.0;
    color[2] = 0.0;
    cgi.R_SetColor(color);

    cgi.R_DrawStretchPic(0.0, 0.0, iWidthOffset, cgs.glconfig.vidHeight, 0.0, 0.0, 1.0, 1.0, cgs.media.lagometerShader);
    cgi.R_DrawStretchPic(
        cgs.glconfig.vidWidth - iWidthOffset,
        0.0,
        iWidthOffset,
        cgs.glconfig.vidHeight,
        .0,
        0.0,
        1.0,
        1.0,
        cgs.media.lagometerShader
    );
}

void CG_DrawOverlayFullScreen(qhandle_t handle, float fAlpha)
{
    int    iHalfWidth, iHalfHeight;
    vec4_t color;

    color[0] = 1.0;
    color[1] = 1.0;
    color[2] = 1.0;
    color[3] = fAlpha;
    cgi.R_SetColor(color);

    iHalfHeight = cgs.glconfig.vidHeight >> 1;
    iHalfWidth  = cgs.glconfig.vidWidth >> 1;

    cgi.R_DrawStretchPic(0.0, 0.0, iHalfWidth, iHalfHeight, 0.0, 0.0, 1.0, 1.0, handle);
    cgi.R_DrawStretchPic(iHalfWidth, 0.0, iHalfWidth, iHalfHeight, 1.0, 0.0, 0.0, 1.0, handle);
    cgi.R_DrawStretchPic(0.0, iHalfHeight, iHalfWidth, iHalfHeight, 0.0, 1.0, 1.0, 0.0, handle);
    cgi.R_DrawStretchPic(iHalfWidth, iHalfHeight, iHalfWidth, iHalfHeight, 1.0, 1.0, 0.0, 0.0, handle);
}

void CG_DrawZoomOverlay()
{
    static int   zoomType;
    static float fAlpha;
    const char  *weaponstring;
    qboolean     bDrawOverlay;

    weaponstring = "";
    bDrawOverlay = qtrue;

    if (!cg.snap) {
        return;
    }

    if (cg.snap->ps.activeItems[1] >= 0) {
        weaponstring = CG_ConfigString(CS_WEAPONS + cg.snap->ps.activeItems[1]);
    }

    if (!Q_stricmp(weaponstring, "Spy Camera")) {
        zoomType = 2;
    } else if (!Q_stricmp(weaponstring, "Binoculars")) {
        zoomType = 3;
    } else {
        if (cg.snap->ps.stats[STAT_INZOOM] && cg.snap->ps.stats[STAT_INZOOM] <= 30) {
            if (!Q_stricmp(weaponstring, "KAR98 - Sniper")) {
                zoomType = 1;
            } else {
                zoomType = 0;
            }
        } else {
            bDrawOverlay = qfalse;
        }
    }

    if (bDrawOverlay) {
        fAlpha += cg.frametime * 0.015;
        if (fAlpha > 1.0) {
            fAlpha = 1.0;
        }
    } else {
        fAlpha -= cg.frametime * 0.015;
        if (fAlpha < 0.0) {
            fAlpha = 0.0;
        }

        if (!fAlpha) {
            return;
        }
    }

    switch (zoomType) {
    case 1:
        CG_DrawOverlayTopBottom(cgs.media.kar98TopOverlayShader, cgs.media.kar98BottomOverlayShader, fAlpha);
        break;
    case 3:
        CG_DrawOverlayFullScreen(cgs.media.binocularsOverlayShader, fAlpha);
        break;
    default:
        CG_DrawOverlayMiddle(cgs.media.zoomOverlayShader, fAlpha);
        break;
    }
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
    int   i;
    float fX, fY;
    float fWidth, fHeight;

    if (!cg_huddraw_force->integer && !cg_hud->integer) {
        return;
    }

    for (i = 0; i < MAX_HUDDRAW_ELEMENTS; i++) {
        if ((!cgi.HudDrawElements[i].hShader && !cgi.HudDrawElements[i].string[0])
            || !cgi.HudDrawElements[i].vColor[3]) {
            // skip invisible elements
            continue;
        }

        fX      = cgi.HudDrawElements[i].iX;
        fY      = cgi.HudDrawElements[i].iY;
        fWidth  = cgi.HudDrawElements[i].iWidth;
        fHeight = cgi.HudDrawElements[i].iHeight;

        if (cgi.HudDrawElements[i].iHorizontalAlign == HUD_ALIGN_X_CENTER) {
            if (cgi.HudDrawElements[i].bVirtualScreen) {
                fX += 320.0 - fWidth * 0.5;
            } else {
                fX += cgs.glconfig.vidWidth * 0.5 - fWidth * 0.5;
            }
        } else if (cgi.HudDrawElements[i].iHorizontalAlign == HUD_ALIGN_X_RIGHT) {
            if (cgi.HudDrawElements[i].bVirtualScreen) {
                fX += 640.0;
            } else {
                fX += cgs.glconfig.vidWidth;
            }
        }

        if (cgi.HudDrawElements[i].iVerticalAlign == HUD_ALIGN_Y_CENTER) {
            if (cgi.HudDrawElements[i].bVirtualScreen) {
                fY += 240.0 - fHeight * 0.5;
            } else {
                fY += cgs.glconfig.vidHeight * 0.5 - fHeight * 0.5;
            }
        } else if (cgi.HudDrawElements[i].iVerticalAlign == HUD_ALIGN_Y_BOTTOM) {
            if (cgi.HudDrawElements[i].bVirtualScreen) {
                fY += 480.0;
            } else {
                fY += cgs.glconfig.vidHeight;
            }
        }

        cgi.R_SetColor(cgi.HudDrawElements[i].vColor);
        if (cgi.HudDrawElements[i].string[0]) {
            if (cgi.HudDrawElements[i].pFont) {
                cgi.R_DrawString(
                    cgi.HudDrawElements[i].pFont,
                    cgi.LV_ConvertString(cgi.HudDrawElements[i].string),
                    fX,
                    fY,
                    -1,
                    cgi.HudDrawElements[i].bVirtualScreen
                );
            } else {
                cgi.R_DrawString(
                    cgs.media.hudDrawFont,
                    cgi.LV_ConvertString(cgi.HudDrawElements[i].string),
                    fX,
                    fY,
                    -1,
                    cgi.HudDrawElements[i].bVirtualScreen
                );
            }
        } else {
            if (cgi.HudDrawElements[i].bVirtualScreen) {
                CG_AdjustFrom640(&fX, &fY, &fWidth, &fHeight);
            }

            cgi.R_DrawStretchPic(fX, fY, fWidth, fHeight, 0.0, 0.0, 1.0, 1.0, cgi.HudDrawElements[i].hShader);
        }
    }
}

void CG_InitializeObjectives()
{
    int i;

    cg.ObjectivesAlphaTime    = 0.0;
    cg.ObjectivesBaseAlpha    = 0.0;
    cg.ObjectivesDesiredAlpha = 0.0;
    cg.ObjectivesCurrentAlpha = 0.0;

    for (i = 0; i < MAX_OBJECTIVES; i++) {
        cg.Objectives[i].flags   = 0;
        cg.Objectives[i].text[0] = 0;
    }
}

void CG_DrawObjectives()
{
    float        vColor[4];
    float        fX, fY;
    static float fWidth;
    float        fHeight;
    int          iNumLines[20];
    int          iTotalNumLines;
    int          i;
    int          ii;
    int          iCurrentObjective;
    float        fTimeDelta;

    iTotalNumLines = 0;
    for (i = CS_OBJECTIVES; i < CS_OBJECTIVES + MAX_OBJECTIVES; ++i) {
        CG_ProcessConfigString(i, qfalse);
    }

    iCurrentObjective         = atoi(CG_ConfigString(CS_CURRENT_OBJECTIVE));
    fTimeDelta                = cg.ObjectivesAlphaTime - cg.time;
    cg.ObjectivesCurrentAlpha = cg.ObjectivesDesiredAlpha;
    if (fTimeDelta > 0) {
        cg.ObjectivesCurrentAlpha =
            (cg.ObjectivesBaseAlpha - cg.ObjectivesDesiredAlpha) * sin(fTimeDelta / (M_PI * 50.f + 2.f))
            + cg.ObjectivesDesiredAlpha;
    }

    if (cg.ObjectivesCurrentAlpha < 0.02) {
        return;
    }

    for (i = 0; i < MAX_OBJECTIVES; i++) {
        if ((cg.Objectives[i].flags & 0xFFFFFFFE)) {
            iNumLines[i] = 0;

            for (ii = 0; ii < MAX_STRING_CHARS; ii++) {
                if (cg.Objectives[i].text[ii] == '\n') {
                    iTotalNumLines++;
                    iNumLines[i]++;
                } else if (!cg.Objectives[i].text[ii]) {
                    iTotalNumLines++;
                    iNumLines[i]++;
                    break;
                }
            }
        }
    }

    fX        = 25.0;
    fY        = 125.0;
    fWidth    = (float)(25 * iTotalNumLines + 155) + 5.0 - 130.0;
    vColor[2] = 0.2f;
    vColor[1] = 0.2f;
    vColor[0] = 0.2f;
    vColor[3] = cg.ObjectivesCurrentAlpha * 0.75;
    cgi.R_SetColor(vColor);
    cgi.R_DrawStretchPic(fX, fY, 450.0, fWidth, 0.0, 0.0, 1.0, 1.0, cgs.media.objectivesBackShader);

    fX        = 30.0;
    fY        = 130.0;
    vColor[0] = 1.0;
    vColor[1] = 1.0;
    vColor[2] = 1.0;
    vColor[3] = cg.ObjectivesCurrentAlpha;
    cgi.R_SetColor(vColor);
    cgi.R_DrawString(cgs.media.objectiveFont, cgi.LV_ConvertString("Mission Objectives:"), fX, fY, -1, 0);
    fY = fY + 5.0;

    cgi.R_DrawString(cgs.media.objectiveFont, "_______________________________________________________", fX, fY, -1, 0);
    fHeight = 155.0;

    for (i = 0; i < MAX_OBJECTIVES; ++i) {
        qhandle_t hBoxShader;

        if ((cg.Objectives[i].flags & OBJ_FLAG_HIDDEN) || !cg.Objectives[i].flags) {
            continue;
        }

        if ((cg.Objectives[i].flags & OBJ_FLAG_CURRENT) != 0) {
            vColor[0]  = 1.0;
            vColor[1]  = 1.0;
            vColor[2]  = 1.0;
            vColor[3]  = cg.ObjectivesCurrentAlpha;
            hBoxShader = cgs.media.uncheckedBoxShader;
        } else if ((cg.Objectives[i].flags & OBJ_FLAG_COMPLETED) != 0) {
            vColor[0]  = 0.75;
            vColor[1]  = 0.75;
            vColor[2]  = 0.75;
            vColor[3]  = cg.ObjectivesCurrentAlpha;
            hBoxShader = cgs.media.checkedBoxShader;
        } else {
            vColor[0]  = 1.0;
            vColor[1]  = 1.0;
            vColor[2]  = 1.0;
            vColor[3]  = cg.ObjectivesCurrentAlpha;
            hBoxShader = cgs.media.uncheckedBoxShader;
        }
        if (i == iCurrentObjective && !(cg.Objectives[i].flags & OBJ_FLAG_COMPLETED)) {
            vColor[0] = 1.0;
            vColor[1] = 1.0;
            vColor[2] = 0.0;
            vColor[3] = cg.ObjectivesCurrentAlpha;
        }

        cgi.R_SetColor(vColor);
        fX = 55.0;
        fY = fHeight;
        cgi.R_DrawString(cgs.media.objectiveFont, cgi.LV_ConvertString(cg.Objectives[i].text), 55.0, fHeight, -1, 0);

        fX        = 30.0;
        fY        = fHeight;
        vColor[0] = 1.0;
        vColor[1] = 1.0;
        vColor[2] = 1.0;
        vColor[3] = cg.ObjectivesCurrentAlpha;
        cgi.R_SetColor(vColor);
        cgi.R_DrawStretchPic(fX, fY, 16.0, 16.0, 0.0, 0.0, 1.0, 1.0, hBoxShader);

        fHeight += 25 * iNumLines[i];
    }
}

void CG_DrawPlayerTeam()
{
    qhandle_t handle;
    if (!cg_hud->integer) {
        return;
    }

    if (!cg.snap || cgs.gametype <= GT_FFA) {
        return;
    }

    handle = 0;
    if (cg.snap->ps.stats[STAT_TEAM] == 3) {
        handle = cgi.R_RegisterShader("textures/hud/allies");
    } else if (cg.snap->ps.stats[STAT_TEAM] == 4) {
        handle = cgi.R_RegisterShader("textures/hud/axis");
    }

    if (handle) {
        cgi.R_SetColor(NULL);
        cgi.R_DrawStretchPic(96.0, cgs.glconfig.vidHeight - 46, 24.0, 24.0, 0.0, 0.0, 1.0, 1.0, handle);
    }
}

void CG_DrawPlayerEntInfo()
{
    int         iClientNum;
    const char *pszClientInfo;
    const char *pszName;
    float       fX, fY;
    float       color[4];
    qhandle_t   handle;

    if (!cg_hud->integer) {
        return;
    }

    if (!cg.snap || cg.snap->ps.stats[STAT_INFOCLIENT] == -1) {
        return;
    }

    iClientNum    = cg.snap->ps.stats[STAT_INFOCLIENT];
    handle        = 0;
    pszClientInfo = CG_ConfigString(iClientNum + CS_PLAYERS);
    pszName       = Info_ValueForKey(pszClientInfo, "name");

    color[0] = 0.5;
    color[1] = 1.0;
    color[2] = 0.5;
    color[3] = 1.0;

    fX = 56.0;
    fY = (float)cgs.glconfig.vidHeight * 0.5;

    if (cg.clientinfo[iClientNum].team == TEAM_ALLIES) {
        handle = cgi.R_RegisterShader("textures/hud/allies");
    } else if (cg.clientinfo[iClientNum].team == TEAM_AXIS) {
        handle = cgi.R_RegisterShader("textures/hud/axis");
    }

    if (handle) {
        cgi.R_SetColor(0);
        cgi.R_DrawStretchPic(56.0, fY, 16.0, 16.0, 0.0, 0.0, 1.0, 1.0, handle);
        fX = 56.0 + 24.0;
    }

    cgi.R_SetColor(color);
    cgi.R_DrawString(cgs.media.hudDrawFont, (char *)pszName, fX, fY, -1, 0);
    cgi.R_DrawString(cgs.media.hudDrawFont, va("%i", cg.snap->ps.stats[STAT_INFOCLIENT_HEALTH]), fX, fY + 20.0, -1, 0);
}

void CG_UpdateAttackerDisplay()
{
    int         iClientNum;
    const char *pszClientInfo;
    const char *pszName;
    float       fX, fY;
    float       color[4];

    if (!cg_hud->integer) {
        return;
    }

    if (!cg.snap || cg.snap->ps.stats[STAT_ATTACKERCLIENT] == -1) {
        return;
    }

    iClientNum    = cg.snap->ps.stats[STAT_ATTACKERCLIENT];
    pszClientInfo = CG_ConfigString(CS_PLAYERS + iClientNum);
    pszName       = Info_ValueForKey(pszClientInfo, "name");

    color[3] = 1.0;
    fY       = (float)(cgs.glconfig.vidHeight - 90);
    fX       = 56.0;

    if (cgs.gametype > GT_FFA) {
        qhandle_t handle;

        handle = 0;
        if (cg.clientinfo[iClientNum].team == TEAM_ALLIES) {
            handle = cgi.R_RegisterShader("textures/hud/allies");
        } else if (cg.clientinfo[iClientNum].team == TEAM_AXIS) {
            handle = cgi.R_RegisterShader("textures/hud/axis");
        }

        if (handle) {
            cgi.R_SetColor(0);
            cgi.R_DrawStretchPic(56.0, fY, 24.0, 24.0, 0.0, 0.0, 1.0, 1.0, handle);
        }

        if ((cg.snap->ps.stats[STAT_TEAM] == TEAM_ALLIES || cg.snap->ps.stats[STAT_TEAM] == TEAM_AXIS)
            && cg.clientinfo[iClientNum].team == cg.snap->ps.stats[STAT_TEAM]) {
            color[0] = 0.5;
            color[1] = 1.0;
            color[2] = 0.5;
        } else {
            color[0] = 1.0;
            color[1] = 0.5;
            color[2] = 0.5;
        }

        fX = 56.0 + 32.0;
    } else {
        color[0] = 1.0;
        color[1] = 0.5;
        color[2] = 0.5;
    }

    cgi.R_SetColor(color);
    cgi.R_DrawString(cgs.media.attackerFont, pszName, fX, fY, -1, 0);
}

void CG_UpdateCountdown()
{
    if (!cg.snap) {
        return;
    }

    if (cg.matchStartTime != -1) {
        int iSecondsLeft, iMinutesLeft;

        iSecondsLeft = (cgs.matchEndTime - cg.time) / 1000;
        if (iSecondsLeft >= 0) {
            iMinutesLeft = iSecondsLeft / 60;
            cgi.Cvar_Set(
                "ui_timemessage", va("%s %2i:%02i", cgi.LV_ConvertString("Time Left:"), iMinutesLeft, iSecondsLeft % 60)
            );
        } else if (!cgs.matchEndTime) {
            cgi.Cvar_Set("ui_timemessage", "");
        }
    } else {
        // The match has not started yet
        cgi.Cvar_Set("ui_timemessage", "Waiting For Players");
    }
}

void CG_DrawStopwatch()
{
    int iFraction;

    if (!cg_hud->integer) {
        cgi.Cmd_Execute(EXEC_NOW, "ui_removehud hud_stopwatch\n");
        return;
    }

    if (!cgi.stopWatch->iStartTime) {
        cgi.Cmd_Execute(EXEC_NOW, "ui_removehud hud_stopwatch\n");
        return;
    }

    if (cgi.stopWatch->iStartTime >= cgi.stopWatch->iEndTime) {
        cgi.Cmd_Execute(EXEC_NOW, "ui_removehud hud_stopwatch\n");
        return;
    }

    if (cgi.stopWatch->iEndTime <= cg.time) {
        cgi.Cmd_Execute(EXEC_NOW, "ui_removehud hud_stopwatch\n");
        return;
    }

    if (cg.ObjectivesCurrentAlpha >= 0.02) {
        cgi.Cmd_Execute(EXEC_NOW, "ui_removehud hud_stopwatch\n");
        return;
    }

    if (cg.snap && cg.snap->ps.stats[STAT_HEALTH] <= 0) {
        cgi.Cmd_Execute(EXEC_NOW, "ui_removehud hud_stopwatch\n");
        return;
    }

    iFraction = cgi.stopWatch->iEndTime - cg.time;
    cgi.Cvar_Set("ui_stopwatch", va("%i", iFraction));
    cgi.Cmd_Execute(EXEC_NOW, "ui_addhud hud_stopwatch\n");
}

void CG_DrawInstantMessageMenu()
{
    float     w, h;
    float     x, y;
    qhandle_t handle;

    if (!cg.iInstaMessageMenu) {
        return;
    }

    if (cg.iInstaMessageMenu > 0) {
        handle = cgi.R_RegisterShader(va("textures/hud/instamsg_group_%c", cg.iInstaMessageMenu + 96));
    } else {
        handle = cgi.R_RegisterShader("textures/hud/instamsg_main");
    }

    w = cgi.R_GetShaderWidth(handle);
    h = cgi.R_GetShaderHeight(handle);
    x = 8.0;
    y = ((float)cgs.glconfig.vidHeight - h) * 0.5;
    cgi.R_SetColor(0);
    cgi.R_DrawStretchPic(x, y, w, h, 0.0, 0.0, 1.0, 1.0, handle);
}

void CG_DrawSpectatorView_ver_15()
{
    const char *pszString;
    int         iKey1, iKey2;
    int         iKey1b, iKey2b;
    float       fX, fY;
    qboolean    bOnTeam;

    if (!(cg.predicted_player_state.pm_flags & PMF_SPECTATING)) {
        return;
    }

    bOnTeam = qfalse;
    if (cg.snap->ps.stats[STAT_TEAM] == TEAM_ALLIES || cg.snap->ps.stats[STAT_TEAM] == TEAM_AXIS) {
        bOnTeam = 1;
    }

    if (!bOnTeam) {
        cgi.Key_GetKeysForCommand("+attackprimary", &iKey1, &iKey2);
        pszString = cgi.LV_ConvertString(va("Press Fire(%s) to join the battle!", cgi.Key_KeynumToBindString(iKey1)));
        fX = (float)(cgs.glconfig.vidWidth - cgi.UI_FontStringWidth(cgs.media.attackerFont, pszString, -1)) * 0.5;
        fY = cgs.glconfig.vidHeight - 64.0;
        cgi.R_SetColor(NULL);
        cgi.R_DrawString(cgs.media.attackerFont, pszString, fX, fY, -1, qfalse);
    }

    if (cg.predicted_player_state.pm_flags & PMF_CAMERA_VIEW) {
        cgi.Key_GetKeysForCommand("+moveup", &iKey1, &iKey2);
        cgi.Key_GetKeysForCommand("+movedown", &iKey1b, &iKey2b);

        pszString = cgi.LV_ConvertString(
            va("Press Jump(%s) or Duck(%s) to follow a different player.",
               cgi.Key_KeynumToBindString(iKey1),
               cgi.Key_KeynumToBindString(iKey1b))
        );

        fX = (float)(cgs.glconfig.vidWidth - cgi.UI_FontStringWidth(cgs.media.attackerFont, pszString, -1)) * 0.5;
        fY = (float)cgs.glconfig.vidHeight - 40.0;
        cgi.R_SetColor(0);
        cgi.R_DrawString(cgs.media.attackerFont, pszString, fX, fY, -1, 0);
    }

    if (!bOnTeam && (cg.predicted_player_state.pm_flags & PMF_CAMERA_VIEW)) {
        cgi.Key_GetKeysForCommand("+use", &iKey1, &iKey2);
        pszString =
            cgi.LV_ConvertString(va("Press Use(%s) to enter free spectate mode.", cgi.Key_KeynumToBindString(iKey1)));

        fX = (float)(cgs.glconfig.vidWidth - cgi.UI_FontStringWidth(cgs.media.attackerFont, pszString, -1)) * 0.5;
        fY = (float)cgs.glconfig.vidHeight - 24.0;
        cgi.R_SetColor(0);
        cgi.R_DrawString(cgs.media.attackerFont, pszString, fX, fY, -1, 0);
    }

    if (!(cg.predicted_player_state.pm_flags & PMF_CAMERA_VIEW)) {
        cgi.Key_GetKeysForCommand("+use", &iKey1, &iKey2);

        pszString = cgi.LV_ConvertString(
            va("Press Use(%s) to enter player following spectate mode.", cgi.Key_KeynumToBindString(iKey1))
        );

        fX = (float)(cgs.glconfig.vidWidth - cgi.UI_FontStringWidth(cgs.media.attackerFont, pszString, -1)) * 0.5;
        fY = (float)cgs.glconfig.vidHeight - 24.0;
        cgi.R_SetColor(0);
        cgi.R_DrawString(cgs.media.attackerFont, pszString, fX, fY, -1, 0);
    }

    if ((cg.predicted_player_state.pm_flags & 0x80) != 0 && cg.snap && cg.snap->ps.stats[STAT_INFOCLIENT] != -1) {
        int       iClientNum;
        qhandle_t hShader;
        vec4_t    color;
        char      buf[128];

        iClientNum = cg.snap->ps.stats[STAT_INFOCLIENT];
        sprintf(buf, "%s : %i", cg.clientinfo[iClientNum].name, cg.snap->ps.stats[STAT_INFOCLIENT_HEALTH]);

        hShader  = 0;
        color[0] = 0.5;
        color[1] = 1.0;
        color[2] = 0.5;
        color[3] = 1.0;

        fX = (float)(cgs.glconfig.vidWidth - cgi.UI_FontStringWidth(cgs.media.attackerFont, pszString, -1) - 16) * 0.5;
        fY = (float)cgs.glconfig.vidHeight - 80.0;
        cgi.R_SetColor(color);
        cgi.R_DrawString(cgs.media.attackerFont, buf, fX, fY, -1, 0);

        if (cg.clientinfo[iClientNum].team == TEAM_ALLIES) {
            hShader = cgi.R_RegisterShader("textures/hud/allies");
        } else if (cg.clientinfo[iClientNum].team == TEAM_AXIS) {
            hShader = cgi.R_RegisterShader("textures/hud/axis");
        }

        if (hShader) {
            fX -= 20.0;
            cgi.R_SetColor(NULL);
            cgi.R_DrawStretchPic(fX, fY, 16.0, 16.0, 0.0, 0.0, 1.0, 1.0, hShader);
        }
    }
}

void CG_DrawSpectatorView_ver_6()
{
    const char *pszString;
    int         iKey1, iKey2;
    int         iKey1b, iKey2b;
    float       fX, fY;
    qboolean    bOnTeam;

    if (!(cg.predicted_player_state.pm_flags & PMF_SPECTATING)) {
        return;
    }

    bOnTeam = qfalse;
    if (cg.snap->ps.stats[STAT_TEAM] == TEAM_ALLIES || cg.snap->ps.stats[STAT_TEAM] == TEAM_AXIS) {
        bOnTeam = 1;
    }

    // retrieve keys for +use
    cgi.Key_GetKeysForCommand("+use", &iKey1, &iKey2);

    if (cg.predicted_player_state.pm_flags & PMF_CAMERA_VIEW) {
        pszString =
            cgi.LV_ConvertString(va("Press Use(%s) to follow a different player.", cgi.Key_KeynumToBindString(iKey1)));
    } else {
        pszString = cgi.LV_ConvertString(va("Press Use(%s) to follow a player.", cgi.Key_KeynumToBindString(iKey1)));
    }

    fX = (float)(cgs.glconfig.vidWidth - cgi.UI_FontStringWidth(cgs.media.attackerFont, pszString, -1)) * 0.5;
    fY = (float)cgs.glconfig.vidHeight - 40.0;
    cgi.R_SetColor(0);
    cgi.R_DrawString(cgs.media.attackerFont, pszString, fX, fY, -1, 0);

    if (!bOnTeam && (cg.predicted_player_state.pm_flags & PMF_CAMERA_VIEW)) {
        cgi.Key_GetKeysForCommand("+moveup", &iKey1, &iKey2);
        cgi.Key_GetKeysForCommand("+movedown", &iKey1b, &iKey2b);
        pszString = cgi.LV_ConvertString(
            va("Press Jump(%s) or Duck(%s) to free spectate.",
               cgi.Key_KeynumToBindString(iKey1),
               cgi.Key_KeynumToBindString(iKey1b))
        );

        fX = (float)(cgs.glconfig.vidWidth - cgi.UI_FontStringWidth(cgs.media.attackerFont, pszString, -1)) * 0.5;
        fY = (float)cgs.glconfig.vidHeight - 24.0;
        cgi.R_SetColor(0);
        cgi.R_DrawString(cgs.media.attackerFont, pszString, fX, fY, -1, 0);
    }
}

void CG_DrawSpectatorView()
{
    if (cg_protocol >= PROTOCOL_MOHTA_MIN) {
        CG_DrawSpectatorView_ver_15();
    } else {
        CG_DrawSpectatorView_ver_6();
    }
}

void CG_DrawCrosshair()
{
    centity_t *friendEnt;
    qhandle_t  shader;
    vec3_t     forward;
    vec3_t     end;
    vec3_t     mins, maxs;
    trace_t    trace;
    float      x, y;
    float      width, height;

    shader = NULL;

    if (!cg_hud->integer || !ui_crosshair->integer) {
        return;
    }

    if (!cg.snap) {
        return;
    }

    if ((cg.snap->ps.pm_flags & PMF_NO_LEAN) || (cg.snap->ps.pm_flags & PMF_INTERMISSION)) {
        return;
    }

    if (!cg.snap->ps.stats[STAT_CROSSHAIR]
        && (!cg.snap->ps.stats[STAT_INZOOM] || cg.snap->ps.stats[STAT_INZOOM] > 30)) {
        return;
    }

    if (cgs.gametype != GT_FFA) {
        AngleVectorsLeft(cg.refdefViewAngles, forward, NULL, NULL);

        VectorMA(cg.refdef.vieworg, 8192, forward, end);
        VectorClear(mins);
        VectorClear(maxs);

        CG_Trace(&trace, cg.refdef.vieworg, mins, maxs, end, 9999, MASK_SOLID, qfalse, qtrue, "CG_DrawCrosshair");

        // ENTITYNUM_WORLD check added in OPM
        if ((trace.entityNum != ENTITYNUM_NONE && trace.entityNum != ENTITYNUM_WORLD)
            && trace.entityNum != cg.snap->ps.clientNum) {
            int myFlags;

            friendEnt = &cg_entities[trace.entityNum];
            if (cgs.gametype != GT_SINGLE_PLAYER) {
                myFlags = cg_entities[cg.snap->ps.clientNum].currentState.eFlags & EF_ANY_TEAM;
            } else {
                // the player will always be considered as an allied
                // in single-player
                myFlags = EF_ALLIES;
            }

            if (((myFlags & EF_ALLIES) && (friendEnt->currentState.eFlags & EF_ALLIES))
                || ((myFlags & EF_AXIS) && (friendEnt->currentState.eFlags & EF_AXIS))) {
                // friend
                if (cg.snap->ps.stats[STAT_CROSSHAIR]) {
                    shader = cgi.R_RegisterShader(cg_crosshair_friend->string);
                }
            } else {
                // enemy
                if (cg.snap->ps.stats[STAT_CROSSHAIR]) {
                    shader = cgi.R_RegisterShader(cg_crosshair->string);
                }
            }
        } else {
            if (cg.snap->ps.stats[STAT_CROSSHAIR]) {
                shader = cgi.R_RegisterShader(cg_crosshair->string);
            }
        }
    } else {
        // FFA
        if (cg.snap->ps.stats[STAT_CROSSHAIR]) {
            shader = cgi.R_RegisterShader(cg_crosshair->string);
        }
    }

    if (shader) {
        width  = cgi.R_GetShaderWidth(shader);
        height = cgi.R_GetShaderHeight(shader);
        x      = (cgs.glconfig.vidWidth - width) * 0.5f;
        y      = (cgs.glconfig.vidHeight - height) * 0.5f;

        cgi.R_SetColor(NULL);
        cgi.R_DrawStretchPic(x, y, width, height, 0, 0, 1, 1, shader);
    }
}

void CG_DrawVote()
{
    const char *text;
    int         seconds;
    int         percentYes;
    int         percentNo;
    int         percentUndecided;
    float       x, y;
    vec4_t      col;

    if (!cgs.voteTime) {
        return;
    }

    if (cgs.voteRefreshed) {
        cgs.voteRefreshed = qfalse;
    }

    seconds = (30000 - (cg.time - cgs.voteTime)) / 1000 + 1;
    if (seconds < 0) {
        seconds = 0;
    }

    percentYes       = cgs.numVotesYes * 100 / (cgs.numUndecidedVotes + cgs.numVotesNo + cgs.numVotesYes);
    percentNo        = cgs.numVotesNo * 100 / (cgs.numUndecidedVotes + cgs.numVotesNo + cgs.numVotesYes);
    percentUndecided = cgs.numUndecidedVotes * 100 / (cgs.numUndecidedVotes + cgs.numVotesNo + cgs.numVotesYes);

    x = 8;
    y = (cgs.glconfig.vidHeight > 480) ? (cgs.glconfig.vidHeight * 0.725f) : (cgs.glconfig.vidHeight * 0.75f);

    cgi.R_SetColor(NULL);

    text = va("%s: %s", cgi.LV_ConvertString("Vote Running"), cgs.voteString);
    cgi.R_DrawString(cgs.media.attackerFont, text, x, y, -1, qfalse);

    y += 12;

    text =
        va("%s: %isec  %s: %i%%  %s: %i%%  %s: %i%%",
           cgi.LV_ConvertString("Time"),
           seconds,
           cgi.LV_ConvertString("Yes"),
           percentYes,
           cgi.LV_ConvertString("No"),
           percentNo,
           cgi.LV_ConvertString("Undecided"),
           percentUndecided);
    cgi.R_DrawString(cgs.media.attackerFont, text, x, y, -1, qfalse);

    if (cg.snap && !cg.snap->ps.voted) {
        col[0] = 0.5;
        col[1] = 1.0;
        col[2] = 0.5;
        col[3] = 1.0;
        cgi.R_SetColor(col);

        y += 12;

        text = cgi.LV_ConvertString("Vote now, it's your patriotic duty!");
        cgi.R_DrawString(cgs.media.attackerFont, text, x, y, -1, qfalse);

        y += 12;

        text = cgi.LV_ConvertString("To vote Yes, press F1. To vote No, press F2.");
        cgi.R_DrawString(cgs.media.attackerFont, text, x, y, -1, qfalse);
        cgi.R_SetColor(NULL);
    }
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
    CG_DrawVote();
    CG_DrawInstantMessageMenu();
    CG_DrawCrosshair();
}
