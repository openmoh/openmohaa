/*
===========================================================================
Copyright (C) 2024 the OpenMoHAA team

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

#include "../server/server.h"

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

void View3D::UpdateLocationPrint(int x, int y, const char *s, float alpha)
{
    m_printstring   = s;
    m_printalpha    = alpha;
    m_printfadetime = 4000.0;
    m_x_coord       = x;
    m_y_coord       = y;
    m_locationprint = qtrue;
}

void View3D::FrameInitialized(void)
{
    Connect(this, W_Activated, W_Activated);
    Connect(this, W_Deactivated, W_Deactivated);
}

void View3D::Pressed(Event *ev)
{
    IN_MouseOff();
    OnActivate(ev);
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

void View3D::OnDeactivate(Event *ev)
{
    Key_SetCatcher(Key_GetCatcher() | KEYCATCH_UI);
}

void View3D::DrawFPS(void)
{
    char string[128];

    setFont("verdana-14");
    if (fps->integer == 2) {
        re.SetColor(UBlack);
        re.DrawBox(
            0.0,
            m_frame.pos.y + m_frame.size.height - m_font->getHeight() * 4.0,
            m_frame.pos.x + m_frame.size.width,
            m_font->getHeight() * 4.0
        );
    }

    Com_sprintf(string, sizeof(string), "FPS %4.1f", currentfps);
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
        m_font->getHeight(getHighResScale()) * 10.0 / getHighResScale()[0],
        (m_frame.pos.y + m_frame.size.height - m_font->getHeight(getHighResScale()) * 3.0) / getHighResScale()[1],
        string,
        -1,
        getHighResScale()
    );

    // Draw elements count
    if (cl_greenfps->integer) {
        m_font->setColor(UGreen);
    } else {
        m_font->setColor(UWhite);
    }

    Com_sprintf(string, sizeof(string), "wt%5d wv%5d cl%d", cls.world_tris, cls.world_verts, cls.character_lights);

    m_font->Print(
        (m_font->getHeight(getHighResScale()) * 10.0) / getHighResScale()[0],
        (m_frame.pos.y + m_frame.size.height - m_font->getHeight(getHighResScale()) * 2.0) / getHighResScale()[1],
        string,
        -1,
        getHighResScale()
    );

    Com_sprintf(
        string,
        sizeof(string),
        "t%5d v%5d Mtex%5.2f",
        cls.total_tris,
        cls.total_verts,
        (float)cls.total_texels * 0.00000095367432
    );

    m_font->Print(
        (m_font->getHeight(getHighResScale()) * 10.0) / getHighResScale()[0],
        (m_frame.pos.y + m_frame.size.height - m_font->getHeight(getHighResScale())) / getHighResScale()[1],
        string,
        -1,
        getHighResScale()
    );

    m_font->setColor(UBlack);
}

/*
void ProfPrint(UIFont* m_font, float minY, int line, char* label, prof_var_t* var, int level)
{

}
*/

void View3D::DrawProf(void)
{
    // FIXME: unimplemented
}

void View3D::PrintSound(int channel, const char *name, float vol, int rvol, float pitch, float base, int& line)
{
    char  buf[255];
    float x;
    float xStep;
    float height;

    height = m_font->getHeight(getHighResScale());
    xStep  = height;

    x = 0;
    Com_sprintf(buf, sizeof(buf), "%d", channel);
    m_font->Print(x, height * line + m_frame.pos.y, buf, -1, getHighResScale());

    x += xStep + xStep;
    Com_sprintf(buf, sizeof(buf), "%s", name);
    m_font->Print(x, height * line + m_frame.pos.y, buf, -1, getHighResScale());

    x += xStep * 30.0;
    Com_sprintf(buf, sizeof(buf), "vol:%.2f", vol);
    m_font->Print(x, height * line + m_frame.pos.y, buf, -1, getHighResScale());

    x += xStep * 8;
    Com_sprintf(buf, sizeof(buf), "rvol:%.2f", (float)(rvol / 128.f));
    m_font->Print(x, height * line + m_frame.pos.y, buf, -1, getHighResScale());

    x += xStep * 5;
    Com_sprintf(buf, sizeof(buf), "pit:%.2f", pitch);
    m_font->Print(x, height * line + m_frame.pos.y, buf, -1, getHighResScale());

    x += xStep * 5;
    Com_sprintf(buf, sizeof(buf), "base:%d", (int)base);
    m_font->Print(x, height * line + m_frame.pos.y, buf, -1, getHighResScale());

    line++;
}

void View3D::DrawSoundOverlay(void)
{
    setFont("verdana-14");
    m_font->setColor(UWhite);

    // FIXME: Unimplemented
    if (sound_overlay->integer) {
        Com_Printf("sound_overlay isn't supported with OpenAL/SDL right now.\n");
        Cvar_Set("sound_overlay", "0");
    }
}

void DisplayServerNetProfileInfo(UIFont *font, float y, netprofclient_t *netprofile)
{
    font->Print(104, y, va("%i", netprofile->inPackets.packetsPerSec));
    font->Print(144, y, va("%i", netprofile->outPackets.packetsPerSec));
    font->Print(184, y, va("%i", netprofile->inPackets.packetsPerSec + netprofile->outPackets.packetsPerSec));
    font->Print(234, y, va("%i", netprofile->inPackets.percentFragmented));
    font->Print(264, y, va("%i", netprofile->outPackets.percentFragmented));
    font->Print(
        294,
        y,
        va("%i",
           (unsigned int)((float)(netprofile->outPackets.numFragmented + netprofile->inPackets.numFragmented)
                          / (float)(netprofile->inPackets.totalProcessed + netprofile->outPackets.totalProcessed))),
        -1
    );
    font->Print(334, y, va("%i", netprofile->inPackets.percentDropped));
    font->Print(364, y, va("%i", netprofile->outPackets.percentDropped));
    font->Print(
        394,
        y,
        va("%i",
           (unsigned int)((float)(netprofile->outPackets.numDropped + netprofile->inPackets.numDropped)
                          / (float)(netprofile->inPackets.totalProcessed + netprofile->outPackets.totalProcessed))),
        -1
    );
    font->Print(434, y, va("%i", netprofile->inPackets.percentDropped));
    font->Print(464, y, va("%i", netprofile->outPackets.percentDropped));
    font->Print(
        494,
        y,
        va("%i",
           (unsigned int)((float)(netprofile->outPackets.totalLengthConnectionLess
                                  + netprofile->inPackets.totalLengthConnectionLess)
                          / (float)(netprofile->outPackets.totalSize + netprofile->inPackets.totalSize))),
        -1
    );
    font->Print(534, y, va("%i", netprofile->inPackets.bytesPerSec));
    font->Print(594, y, va("%i", netprofile->outPackets.bytesPerSec));
    font->Print(654, y, va("%i", netprofile->outPackets.bytesPerSec + netprofile->inPackets.bytesPerSec));
    font->Print(714, y, va("%i", netprofile->rate));
}

void DisplayClientNetProfile(UIFont *font, float x, float y, netprofclient_t *netprofile)
{
    float columns[5];
    float fontHeight;
    float columnHeight;

    fontHeight   = font->getHeight();
    columns[0]   = x + 120;
    columns[1]   = x + 230;
    columns[2]   = x + 330;
    columns[3]   = x + 430;
    columns[4]   = x + 530;
    columnHeight = y;

    font->Print(x, y, va("Rate: %i", netprofile->rate));

    columnHeight += fontHeight * 1.5;
    font->Print(x, columnHeight, "Data Type");
    font->Print(columns[0], columnHeight, "Packets per Sec");
    font->Print(columns[1], columnHeight, "% Fragmented");
    font->Print(columns[2], columnHeight, "% Dropped");
    font->Print(columns[3], columnHeight, "% OOB data");
    font->Print(columns[4], columnHeight, "Data per Sec");

    columnHeight += fontHeight * 0.5;
    font->Print(x, columnHeight, "----------");
    font->Print(columns[0], columnHeight, "----------");
    font->Print(columns[1], columnHeight, "----------");
    font->Print(columns[2], columnHeight, "----------");
    font->Print(columns[3], columnHeight, "----------");
    font->Print(columns[4], columnHeight, "----------");

    columnHeight += fontHeight;
    font->Print(x, columnHeight, "Data In");
    font->Print(columns[0], columnHeight, va("%i", netprofile->outPackets.packetsPerSec));
    font->Print(columns[1], columnHeight, va("%i%%", netprofile->outPackets.percentFragmented));
    font->Print(columns[2], columnHeight, va("%i%%", netprofile->outPackets.percentDropped));
    font->Print(columns[3], columnHeight, va("%i%%", netprofile->outPackets.percentConnectionLess));
    font->Print(columns[4], columnHeight, va("%i", netprofile->outPackets.bytesPerSec));

    columnHeight += fontHeight;
    font->Print(x, columnHeight, "Data Out");
    font->Print(columns[0], columnHeight, va("%i", netprofile->inPackets.packetsPerSec));
    font->Print(columns[1], columnHeight, va("%i%%", netprofile->inPackets.percentFragmented));
    font->Print(columns[2], columnHeight, va("%i%%", netprofile->inPackets.percentDropped));
    font->Print(columns[3], columnHeight, va("%i%%", netprofile->inPackets.percentConnectionLess));
    font->Print(columns[4], columnHeight, va("%i", netprofile->inPackets.bytesPerSec));

    columnHeight += fontHeight;

    font->Print(x, columnHeight, "Total Data");

    font->Print(
        columns[0], columnHeight, va("%i", netprofile->inPackets.packetsPerSec + netprofile->outPackets.packetsPerSec)
    );
    font->Print(
        columns[1],
        columnHeight,
        va("%i%%",
           (unsigned int)((float)(netprofile->outPackets.numFragmented + netprofile->inPackets.numFragmented)
                          / (float)(netprofile->inPackets.totalProcessed + netprofile->outPackets.totalProcessed)))
    );
    font->Print(
        columns[2],
        columnHeight,
        va("%i%%",
           (unsigned int)((float)(netprofile->outPackets.numDropped + netprofile->inPackets.numDropped)
                          / (double)(netprofile->inPackets.totalProcessed + netprofile->outPackets.totalProcessed)))
    );
    font->Print(
        columns[3],
        columnHeight,
        va("%i%%",
           (unsigned int)((float)(netprofile->outPackets.totalLengthConnectionLess
                                  + netprofile->inPackets.totalLengthConnectionLess)
                          / (float)(netprofile->outPackets.totalSize + netprofile->inPackets.totalSize)))
    );
    font->Print(
        columns[4], columnHeight, va("%i", netprofile->inPackets.bytesPerSec + netprofile->outPackets.bytesPerSec)
    );
}

void View3D::DrawNetProfile(void)
{
    float fontHeight;
    float yOffset;
    int   i;

    if (sv_netprofileoverlay->integer && sv_netprofile->integer && com_sv_running->integer) {
        float           columnHeight;
        float           valueHeight;
        float           separatorHeight;
        float           categoryHeight;
        float           currentHeight;
        netprofclient_t netproftotal;

        setFont("verdana-14");
        m_font->setColor(UWhite);

        fontHeight = m_font->getHeight();
        yOffset    = sv_netprofileoverlay->integer + 8;

        if (svs.netprofile.rate) {
            m_font->Print(8, yOffset, va("Server Net Profile          Max Rate: %i", svs.netprofile.rate), -1);
        } else {
            m_font->Print(8, yOffset, "Server Net Profile          Max Rate: none", -1);
        }

        columnHeight    = fontHeight + fontHeight + yOffset;
        valueHeight     = columnHeight + fontHeight;
        separatorHeight = fontHeight * 1.5 + columnHeight;
        categoryHeight  = fontHeight * 0.5 + columnHeight;

        m_font->Print(8, categoryHeight, "Data Source");
        m_font->Print(8, separatorHeight, "---------------");
        m_font->Print(104, columnHeight, "Packets per Sec");
        m_font->Print(104, valueHeight, "In");
        m_font->Print(144, valueHeight, "Out");
        m_font->Print(184, valueHeight, "Total");
        m_font->Print(104, separatorHeight, "---");
        m_font->Print(144, separatorHeight, "-----");
        m_font->Print(184, separatorHeight, "------");
        m_font->Print(234, columnHeight, "% Fragmented");
        m_font->Print(234, valueHeight, "In");
        m_font->Print(264, valueHeight, "Out");
        m_font->Print(294, valueHeight, "Total");
        m_font->Print(234, separatorHeight, "---");
        m_font->Print(264, separatorHeight, "-----");
        m_font->Print(294, separatorHeight, "------");
        m_font->Print(334, columnHeight, "% Dropped");
        m_font->Print(334, valueHeight, "In");
        m_font->Print(364, valueHeight, "Out");
        m_font->Print(394, valueHeight, "Total");
        m_font->Print(334, separatorHeight, "---");
        m_font->Print(364, separatorHeight, "-----");
        m_font->Print(394, separatorHeight, "------");
        m_font->Print(434, columnHeight, "% OOB Data");
        m_font->Print(434, valueHeight, "In");
        m_font->Print(464, valueHeight, "Out");
        m_font->Print(494, valueHeight, "Total");
        m_font->Print(434, separatorHeight, "---");
        m_font->Print(464, separatorHeight, "-----");
        m_font->Print(494, separatorHeight, "------");
        m_font->Print(534, columnHeight, "Data per Sec");
        m_font->Print(534, valueHeight, "In");
        m_font->Print(594, valueHeight, "Out");
        m_font->Print(654, valueHeight, "Total");
        m_font->Print(534, separatorHeight, "---");
        m_font->Print(594, separatorHeight, "-----");
        m_font->Print(654, separatorHeight, "------");
        m_font->Print(714, categoryHeight, "Rate");
        m_font->Print(714, separatorHeight, "------");

        currentHeight = fontHeight * 2.5 + columnHeight;
        SV_NET_CalcTotalNetProfile(&netproftotal, qfalse);
        m_font->Print(8, columnHeight + fontHeight * 2.5, "Total");
        DisplayServerNetProfileInfo(m_font, currentHeight, &netproftotal);

        currentHeight += fontHeight * 1.5;
        m_font->Print(8, currentHeight, "Clientless");
        DisplayServerNetProfileInfo(m_font, currentHeight, &svs.netprofile);

        currentHeight += fontHeight;

        for (i = 0; i < svs.iNumClients; i++) {
            client_t *client = &svs.clients[i];
            if (client->state != CS_ACTIVE || !client->gentity) {
                continue;
            }

            if (client->netchan.remoteAddress.type == NA_LOOPBACK) {
                m_font->Print(8.0, currentHeight, va("#%i-Loopback", i), -1, 0);

            } else {
                m_font->Print(8.0, currentHeight, va("Client #%i", i), -1, 0);
            }

            DisplayServerNetProfileInfo(m_font, currentHeight, &client->netprofile);
            currentHeight = currentHeight + fontHeight;
        }
    } else if (cl_netprofileoverlay->integer && cl_netprofile->integer && com_cl_running->integer) {
        setFont("verdana-14");
        m_font->setColor(UWhite);

        fontHeight = m_font->getHeight();
        yOffset    = cl_netprofileoverlay->integer + 16;

        m_font->Print(16, yOffset, "Client Net Profile", -1);

        NetProfileCalcStats(&cls.netprofile.outPackets, 500);
        NetProfileCalcStats(&cls.netprofile.inPackets, 500);

        DisplayClientNetProfile(m_font, 16, yOffset + fontHeight * 2, &cls.netprofile);
    }
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
            DrawNetProfile();
            DrawSubtitleOverlay();
        }
    }

    if (fps->integer && !cls.no_menus) {
        DrawFPS();
        DrawProf();
    }
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
            getVirtualScale()
        );

        m_font->setColor(UColor(1, 1, 1, alpha));

        frame = getClientFrame();

        m_font->PrintJustified(frame, m_iFontAlignmentHorizontal, m_iFontAlignmentVertical, p, getVirtualScale());

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
    y = (480 - m_font->getHeight(getHighResScale()) - m_y_coord) / 480.f * m_screenframe.size.height;

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
        getVirtualScale()
    );

    m_font->setColor(UColor(1, 1, 1, alpha));
    frame = getClientFrame();

    m_font->PrintJustified(
        UIRect2D(frame.pos.x + x, frame.pos.y + y, frame.size.width, frame.size.height),
        horiz,
        vert,
        p,
        getVirtualScale()
    );

    m_font->setColor(UBlack);
    m_printfadetime -= cls.frametime;

    if (m_printfadetime < 0) {
        m_printfadetime = 0;
    }
}

void View3D::DrawLetterbox(void)
{
    float  frac;
    vec4_t col;

    col[0] = col[1] = col[2] = 0;
    col[3]                   = 1;

    frac = (float)cl.snap.ps.stats[STAT_LETTERBOX] / MAX_LETTERBOX_SIZE;
    if (frac <= 0) {
        m_letterbox_active = false;
        return;
    }

    m_letterbox_active = true;
    re.SetColor(col);

    re.DrawBox(0.0, 0.0, m_screenframe.size.width, m_screenframe.size.height * frac);
    re.DrawBox(
        0.0,
        m_screenframe.size.height - m_screenframe.size.height * frac,
        m_screenframe.size.width,
        m_screenframe.size.height
    );
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

void View3D::Draw(void)
{
    if (clc.state != CA_DISCONNECTED) {
        SCR_DrawScreenField();
    }

    set2D();

    re.SavePerformanceCounters();

    Draw2D();
}

float avWidth = 0.0;

void View3D::InitSubtitle(void)
{
    float totalWidth;

    for (int i = 0; i < 4; i++) {
        subs[i]  = Cvar_Get(va("subtitle%d", i), "", 0);
        teams[i] = Cvar_Get(va("subteam%d", i), "0", 0);
        Q_strncpyz(oldStrings[i], subs[i]->string, sizeof(oldStrings[i]));
        fadeTime[i] = 4000.0;
        subLife[i]  = 4000.0;
    }

    totalWidth = 0.0;
    for (char j = 'A'; j <= 'Z'; j++) {
        totalWidth += m_font->getCharWidth(j);
    }

    avWidth = totalWidth / 26.0;
}

void View3D::DrawSubtitleOverlay(void)
{
    cvar_t *subAlpha;
    int     i;
    float   minX, maxX;
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

            if (subs[i]->string && subs[i]->string[0]) {
                Cvar_Set(va("subtitle%d", i), "");
            }
        }
    }

    minX = m_screenframe.size.height - m_font->getHeight(getHighResScale()) * 10;
    maxX = ((m_frame.pos.x + m_frame.size.width) - (m_frame.pos.x + m_frame.size.width) * 0.2f) / getHighResScale()[0];
    line = 0;

    for (i = 0; i < MAX_SUBTITLES; i++) {
        if (fadeTime[i] <= 0) {
            continue;
        }

        if (m_font->getWidth(subs[i]->string, sizeof(oldStrings[i])) > maxX) {
            char  buf[2048];
            char *c;
            char *end;
            char *start;
            float total;
            float width;
            int   blockcount;

            c = subs[i]->string;

            total  = 0;
            end    = NULL;
            start  = buf;
            buf[0] = 0;

            while (*c) {
                blockcount = m_font->DBCSGetWordBlockCount(c, -1);
                if (!blockcount) {
                    break;
                }

                width = m_font->getWidth(c, blockcount);

                if (total + width > maxX) {
                    m_font->setColor(UColor(0, 0, 0, alpha[i] * subAlpha->value));
                    m_font->Print(
                        18,
                        (m_font->getHeight(getHighResScale()) * line + minX + 1.f) / getHighResScale()[1],
                        buf,
                        -1,
                        getHighResScale()
                    );

                    m_font->setColor(UColor(1, 1, 1, alpha[i] * subAlpha->value));
                    m_font->Print(
                        20,
                        (m_font->getHeight(getHighResScale()) * line + minX) / getHighResScale()[1],
                        buf,
                        -1,
                        getHighResScale()
                    );

                    line++;

                    total = 0;
                    start = buf;
                }

                end = start + blockcount + 1;
                if (end > buf + MAX_STRING_CHARS) {
                    Com_DPrintf("ERROR - word longer than possible line\n");
                    break;
                }

                memcpy(start, c, blockcount);
                start += blockcount;
                total += width;
                *start = 0;

                c += blockcount;
            }

            m_font->setColor(UColor(0, 0, 0, alpha[i] * subAlpha->value));
            m_font->Print(
                18,
                (m_font->getHeight(getHighResScale()) * line + minX + 1.f) / getHighResScale()[1],
                buf,
                -1,
                getHighResScale()
            );

            m_font->setColor(UColor(1, 1, 1, alpha[i] * subAlpha->value));
            m_font->Print(
                20,
                (m_font->getHeight(getHighResScale()) * line + minX) / getHighResScale()[1],
                buf,
                -1,
                getHighResScale()
            );
            line++;
        } else {
            m_font->setColor(UColor(0, 0, 0, alpha[i] * subAlpha->value));
            m_font->Print(
                18,
                (m_font->getHeight(getHighResScale()) * line + minX + 1.f) / getHighResScale()[1],
                subs[i]->string,
                -1,
                getHighResScale()
            );

            m_font->setColor(UColor(1, 1, 1, alpha[i] * subAlpha->value));
            m_font->Print(
                20,
                (m_font->getHeight(getHighResScale()) * line + minX) / getHighResScale()[1],
                subs[i]->string,
                -1,
                getHighResScale()
            );

            line++;
        }
    }
}

void View3D::ClearCenterPrint(void)
{
    m_printfadetime = 0.0;
}

qboolean View3D::LetterboxActive(void)
{
    return m_letterbox_active;
}

CLASS_DECLARATION(UIWidget, ConsoleView, NULL) {
    {NULL, NULL}
};

void ConsoleView::Draw(void) {}
