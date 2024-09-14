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

#include "cl_ui.h"
#include "../qcommon/localization.h"

CLASS_DECLARATION(UIWidget, View3D, NULL) {
    {&W_Activated,     &View3D::OnActivate  },
    {&W_Deactivated,   &View3D::OnDeactivate},
    {&W_LeftMouseDown, &View3D::Pressed     },
    {NULL,             NULL                 }
};

cvar_t *subs[MAX_SUBTITLES];
cvar_t *teams[MAX_SUBTITLES];
float   fadeTime[MAX_SUBTITLES];
float   subLife[MAX_SUBTITLES];
float   alpha[MAX_SUBTITLES];
char    oldStrings[MAX_SUBTITLES][2048];

View3D::View3D()
{
    // set as transparent
    setBackgroundColor(UClear, true);
    // no border
    setBorderStyle(border_none);
    AllowActivate(true);

    m_printfadetime = 0.0;
    m_print_mat     = NULL;
    m_locationprint = qfalse;
}

void View3D::UpdateLocationPrint(int x, int y, const char *s, float alpha)
{
    m_printstring   = s;
    m_printalpha    = alpha;
    m_printfadetime = 4000.0;
    m_x_coord       = x;
    m_y_coord       = y;
    m_locationprint = qtrue;
}

void View3D::UpdateCenterPrint(const char *s, float alpha)
{
    m_printstring = s;

    if (s[0] == '@') {
        m_print_mat = uWinMan.RegisterShader(s + 1);
    } else {
        m_print_mat = NULL;
    }

    m_printalpha    = alpha;
    m_printfadetime = 4000.0;
    m_locationprint = qfalse;
}

void View3D::PrintSound(int channel, const char *name, float vol, int rvol, float pitch, float base, int& line)
{
    char  buf[255];
    float x;
    float xStep;
    float height;

    height = m_font->getHeight(false);
    xStep  = height;

    x = 0;
    sprintf(buf, "%d", channel);
    m_font->Print(x, height * line + m_frame.pos.y, buf, -1, false);

    x += xStep + xStep;
    sprintf(buf, "%s", name);
    m_font->Print(x, height * line + m_frame.pos.y, buf, -1, false);

    x += xStep * 30.0;
    sprintf(buf, "vol:%.2f", vol);
    m_font->Print(x, height * line + m_frame.pos.y, buf, -1, false);

    x += xStep * 8;
    sprintf(buf, "rvol:%.2f", (float)(rvol / 128.f));
    m_font->Print(x, height * line + m_frame.pos.y, buf, -1, false);

    x += xStep * 5;
    sprintf(buf, "pit:%.2f", pitch);
    m_font->Print(x, height * line + m_frame.pos.y, buf, -1, false);

    x += xStep * 5;
    sprintf(buf, "base:%d", (int)base);
    m_font->Print(x, height * line + m_frame.pos.y, buf, -1, false);

    line++;
}

void View3D::Pressed(Event *ev)
{
    IN_MouseOff();
    OnActivate(ev);
}

void View3D::OnDeactivate(Event *ev)
{
    Key_SetCatcher(Key_GetCatcher() | KEYCATCH_UI);
}

void View3D::OnActivate(Event *ev)
{
    UIWidget         *wid;
    UList<UIWidget *> widgets;

    UI_CloseInventory();
    Key_SetCatcher(Key_GetCatcher() & ~KEYCATCH_UI);

    for (wid = getParent()->getFirstChild(); wid; wid = getParent()->getNextChild(wid)) {
        if (wid->getAlwaysOnBottom() && wid != this) {
            widgets.AddTail(wid);
        }
    }

    widgets.IterateFromHead();
    while (widgets.IsCurrentValid()) {
        widgets.getCurrent()->BringToFrontPropogated();
        widgets.IterateNext();
    }
}

void View3D::LocationPrint(void)
{
    fonthorzjustify_t horiz;
    fontvertjustify_t vert;
    int               x, y;
    const char       *p;
    float             alpha;
    UIRect2D          frame;

    if (!m_printfadetime) {
        m_locationprint = false;
        return;
    }

    horiz = FONT_JUSTHORZ_LEFT;
    vert  = FONT_JUSTVERT_TOP;

    p = Sys_LV_CL_ConvertString(m_printstring);
    if (m_printfadetime > 3250) {
        alpha = 1.f - (m_printfadetime - 3250.f) / 750.f * m_printalpha;
    } else if (m_printfadetime >= 750) {
        alpha = 1.f;
    } else {
        alpha = m_printfadetime / 750.f * m_printalpha;
    }

    alpha = Q_clamp_float(alpha, 0, 1);

    x = m_x_coord / 640.f * m_screenframe.size.width;
    y = (480 - m_font->getHeight(false) - m_y_coord) / 480.f * m_screenframe.size.height;

    if (m_x_coord == -1) {
        horiz = FONT_JUSTHORZ_CENTER;
        x     = 0;
    }
    if (m_y_coord == -1) {
        vert = FONT_JUSTVERT_CENTER;
        y    = 0;
    }

    m_font->setColor(UColor(0, 0, 0, alpha));
    frame = getClientFrame();

    m_font->PrintJustified(
        UIRect2D(frame.pos.x + x + 1, frame.pos.y + y + 1, frame.size.width, frame.size.height),
        horiz,
        vert,
        p,
        m_bVirtual ? m_vVirtualScale : NULL
    );

    m_font->setColor(UColor(1, 1, 1, alpha));
    frame = getClientFrame();

    m_font->PrintJustified(
        UIRect2D(frame.pos.x + x, frame.pos.y + y, frame.size.width, frame.size.height),
        horiz,
        vert,
        p,
        m_bVirtual ? m_vVirtualScale : NULL
    );

    m_font->setColor(UBlack);
    m_printfadetime -= cls.frametime;

    if (m_printfadetime < 0) {
        m_printfadetime = 0;
    }
}

qboolean View3D::LetterboxActive(void)
{
    return m_letterbox_active;
}

float avWidth = 0.0;

void View3D::InitSubtitle(void)
{
    float totalWidth;

    for (int i = 0; i < 4; i++) {
        subs[i]  = Cvar_Get(va("subtitle%d", i), "", 0);
        teams[i] = Cvar_Get(va("subteam%d", i), "0", 0);
        strcpy(oldStrings[i], subs[i]->string);
        fadeTime[i] = 4000.0;
        subLife[i]  = 4000.0;
    }

    totalWidth = 0.0;
    for (char j = 'A'; j <= 'Z'; j++) {
        totalWidth += m_font->getCharWidth(j);
    }

    avWidth = totalWidth / 26.0;
}

void View3D::FrameInitialized(void)
{
    Connect(this, W_Activated, W_Activated);
    Connect(this, W_Deactivated, W_Deactivated);
}

void View3D::DrawSubtitleOverlay(void)
{
    cvar_t *subAlpha;
    int     i;
    float   minX, maxY;
    int     line;

    subAlpha = Cvar_Get("subAlpha", "0.5", 0);

    setFont("facfont-20");
    m_font->setColor(URed);

    for (i = 0; i < MAX_SUBTITLES; i++) {
        if (strcmp(oldStrings[i], subs[i]->string)) {
            fadeTime[i] = 2500 * ((strlen(subs[i]->string) / 68) + 1.f) + 1500;
            subLife[i]  = fadeTime[i];
            Q_strncpyz(oldStrings[i], subs[i]->string, sizeof(oldStrings[i]));
        }

        if (fadeTime[i] > subLife[i] - 750.f) {
            alpha[i] = 1.f - (fadeTime[i] - (subLife[i] - 750.f)) / 750.f;
        } else if (fadeTime[i] < 750) {
            alpha[i] = fadeTime[i] / 750.f;
        } else {
            alpha[i] = 1.f;
        }

        fadeTime[i] -= cls.frametime;
        if (fadeTime[i] < 0) {
            // Clear the subtitle
            fadeTime[i]      = 0;
            oldStrings[i][0] = 0;
            Cvar_Set(va("subtitle%d", i), "");
        }
    }

    minX = m_screenframe.size.height - m_font->getHeight(false) * 10;
    maxY = (m_frame.pos.x + m_frame.size.width) - (m_frame.pos.x + m_frame.size.width) * 0.2f;
    line = 0;

    for (i = 0; i < MAX_SUBTITLES; i++) {
        if (m_font->getWidth(subs[i]->string, sizeof(oldStrings[i])) > maxY) {
            char  buf[2048];
            char *c;
            char *end;
            char *start;
            float total;
            int   blockcount;

            c = buf;
            strcpy(buf, subs[i]->string);

            total = 0;
            end   = NULL;
            start = buf;
            while (*c) {
                blockcount = m_font->DBCSGetWordBlockCount(c, -1);
                if (!blockcount) {
                    break;
                }

                total += m_font->getWidth(c, blockcount);
                c += blockcount;

                if (total > maxY) {
                    total = 0;
                    if (end < start) {
                        Com_DPrintf("ERROR - word longer than possible line\n");
                        break;
                    }

                    m_font->setColor(UColor(0, 0, 0, alpha[i] * subAlpha->value));
                    m_font->Print(18, m_font->getHeight(false) * line + minX + 1.f, start, -1, false);

                    m_font->setColor(UColor(1, 1, 1, alpha[i] * subAlpha->value));
                    m_font->Print(20, m_font->getHeight(false) * line + minX, start, -1, false);

                    start = end + 1;
                    c     = end + 1;
                    line++;
                }
            }

            m_font->setColor(UColor(0, 0, 0, alpha[i] * subAlpha->value));
            m_font->Print(18, m_font->getHeight(false) * line + minX + 1.f, start, -1, qfalse);

            m_font->setColor(UColor(1, 1, 1, alpha[i] * subAlpha->value));
            m_font->Print(20, m_font->getHeight(false) * line + minX, start, -1, qfalse);
            line++;
        } else {
            m_font->setColor(UColor(0, 0, 0, alpha[i] * subAlpha->value));
            m_font->Print(18, m_font->getHeight(false) * line + minX + 1.f, subs[i]->string, -1, qfalse);

            m_font->setColor(UColor(1, 1, 1, alpha[i] * subAlpha->value));
            m_font->Print(20, m_font->getHeight(false) * line + minX, subs[i]->string, -1, qfalse);

            line++;
        }
    }
}

void View3D::DrawSoundOverlay(void)
{
    setFont("verdana-14");
    m_font->setColor(UWhite);

    // FIXME: TODO
    if (sound_overlay->integer) {
        Com_Printf("sound_overlay isn't supported with OpenAL/SDL right now.\n");
        Cvar_Set("sound_overlay", "0");
    }
}

void View3D::DrawProf(void)
{
    // Normal empty function
}

void View3D::DrawLetterbox(void)
{
    float  frac;
    vec4_t col;

    col[0] = col[1] = col[2] = 0;
    col[3]                   = 1;

    frac = cl.snap.ps.stats[STAT_LETTERBOX] / 32767.0;
    if (frac > 0.0) {
        m_letterbox_active = true;
        re.SetColor(col);

        re.DrawBox(0.0, 0.0, m_screenframe.size.width, m_screenframe.size.height);
        re.DrawBox(
            0.0,
            m_screenframe.size.height - m_screenframe.size.height * frac,
            m_screenframe.size.width,
            m_screenframe.size.height
        );
    } else {
        m_letterbox_active = false;
    }
}

void View3D::DrawFades(void)
{
    if (cl.snap.ps.blend[3] > 0) {
        re.SetColor(cl.snap.ps.blend);
        if (cl.snap.ps.stats[STAT_ADDFADE]) {
            re.AddBox(0.0, 0.0, m_screenframe.size.width, m_screenframe.size.height);
        } else {
            re.DrawBox(0.0, 0.0, m_screenframe.size.width, m_screenframe.size.height);
        }
    }
}

void View3D::DrawFPS(void)
{
    char string[128];

    setFont("verdana-14");
    if (fps->integer == 2) {
        re.SetColor(UBlack);
        re.DrawBox(
            0.0,
            m_frame.pos.y + m_frame.size.height - m_font->getHeight(qfalse) * 4.0,
            m_frame.pos.x + m_frame.size.width,
            m_font->getHeight(qfalse) * 4.0
        );
    }

    sprintf(string, "FPS %4.1f", currentfps);
    if (currentfps > 23.94) {
        if (cl_greenfps->integer) {
            m_font->setColor(UGreen);
        } else {
            m_font->setColor(UWhite);
        }
    } else if (currentfps > 18.0) {
        m_font->setColor(UYellow);
    } else {
        // low fps
        m_font->setColor(URed);
    }

    m_font->Print(
        m_font->getHeight(qfalse) * 10.0,
        m_frame.pos.y + m_frame.size.height - m_font->getHeight(qfalse) * 3.0,
        string,
        -1,
        qfalse
    );

    // Draw elements count
    if (cl_greenfps->integer) {
        m_font->setColor(UGreen);
    } else {
        m_font->setColor(UWhite);
    }

    sprintf(string, "wt%5d wv%5d cl%d", cls.world_tris, cls.world_verts, cls.character_lights);

    m_font->Print(
        m_font->getHeight(qfalse) * 10.0,
        m_frame.pos.y + m_frame.size.height - m_font->getHeight(qfalse) * 2.0,
        string,
        -1,
        qfalse
    );

    sprintf(string, "t%5d v%5d Mtex%5.2f", cls.total_tris, cls.total_verts, (float)cls.total_texels * 0.00000095367432);

    m_font->Print(
        m_font->getHeight(qfalse) * 10.0,
        m_frame.pos.y + m_frame.size.height - m_font->getHeight(qfalse),
        string,
        -1,
        qfalse
    );

    m_font->setColor(UBlack);
}

void View3D::Draw2D(void)
{
    if (!cls.no_menus) {
        DrawFades();
    }

    DrawLetterbox();

    if ((cl_debuggraph->integer || cl_timegraph->integer) && !cls.no_menus) {
        SCR_DrawDebugGraph();
    } else if (!cls.no_menus) {
        if (cge) {
            cge->CG_Draw2D();
        }

        if (m_locationprint) {
            LocationPrint();
        } else {
            CenterPrint();
        }

        if (!cls.no_menus) {
            DrawSoundOverlay();
            DrawSubtitleOverlay();
        }
    }

    if (fps->integer && !cls.no_menus) {
        DrawFPS();
        DrawProf();
    }
}

void View3D::Draw(void)
{
    if (clc.state != CA_DISCONNECTED) {
        SCR_DrawScreenField();
    }

    set2D();

    re.SavePerformanceCounters();

    Draw2D();
}

void View3D::ClearCenterPrint(void)
{
    m_printfadetime = 0.0;
}

void View3D::CenterPrint(void)
{
    float       alpha;
    const char *p;
    qhandle_t   mat;
    float       x, y;
    float       w, h;

    if (!m_printfadetime) {
        return;
    }

    p = Sys_LV_CL_ConvertString(m_printstring);
    if (m_printfadetime > 3250) {
        alpha = 1.f - (m_printfadetime - 3250.f) / 750.f * m_printalpha;
    } else if (m_printfadetime >= 750) {
        alpha = 1.f;
    } else {
        alpha = m_printfadetime / 750.f * m_printalpha;
    }

    alpha = Q_clamp_float(alpha, 0, 1);

    if (!m_print_mat) {
        UIRect2D frame;
        m_font->setColor(UColor(0, 0, 0, alpha));

        frame = getClientFrame();

        m_font->PrintJustified(
            UIRect2D(frame.pos.x + 1, frame.pos.y + 1, frame.size.width, frame.size.height),
            m_iFontAlignmentHorizontal,
            m_iFontAlignmentVertical,
            p,
            m_bVirtual ? m_vVirtualScale : NULL
        );

        m_font->setColor(UColor(1, 1, 1, alpha));

        frame = getClientFrame();

        m_font->PrintJustified(
            frame, m_iFontAlignmentHorizontal, m_iFontAlignmentVertical, p, m_bVirtual ? m_vVirtualScale : NULL
        );

        m_font->setColor(UBlack);
    } else if ((mat = m_print_mat->GetMaterial())) {
        vec4_t col {alpha, alpha, alpha, alpha};

        re.SetColor(col);

        w = re.GetShaderWidth(mat);
        h = re.GetShaderHeight(mat);
        x = (m_frame.pos.x + m_frame.size.width - w) * 0.5f;
        y = (m_frame.pos.y + m_frame.size.height - h) * 0.5f;

        re.DrawStretchPic(x, y, w, h, 0, 0, 1, 1, mat);
    }

    m_printfadetime -= cls.frametime;

    if (m_printfadetime < 0) {
        m_printfadetime = 0;
    }
}

CLASS_DECLARATION(UIWidget, ConsoleView, NULL) {
    {NULL, NULL}
};

void ConsoleView::Draw(void) {}
