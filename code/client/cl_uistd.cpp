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
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110EV_DEFAULT301  USA
===========================================================================
*/

#include "cl_ui.h"
#include "../qcommon/localization.h"

Event EV_Layout_PlayerStat
(
    "playerstat",
    EV_DEFAULT,
    "i",
    "playerstatValue",
    "set playerstat for layout\n"
);
// Added in 2.0
Event EV_Layout_PlayerStatAlpha
(
    "playerstatalpha",
    EV_DEFAULT,
    "i",
    "playerstatValue",
    "set playerstat to control alpha for layout\n"
);
Event EV_Layout_PlayerStatConfigstring
(
    "playerstatconfigstring",
    EV_DEFAULT,
    "i",
    "playerstatValue",
    "set playerstat for layout - will print out the corresponding configstring\n"
);
Event EV_Layout_MaxPlayerStat
(
    "maxplayerstat",
    EV_DEFAULT,
    "i",
    "maxPlayerstatValue",
    "set the playerstat that holds the max value for that stat.  Used for statbars\n"
);
Event EV_Layout_ItemIndex
(
    "itemstat",
    EV_DEFAULT,
    "i",
    "itemstatValue",
    "set itemstat for layout\n"
);
Event EV_Layout_InventoryRenderModelIndex
(
    "invmodelhand",
    EV_DEFAULT,
    "i",
    "handindex",
    "set the specified hand index to render a 3d model from\n"
);
Event EV_Layout_Statbar
(
    "statbar",
    EV_DEFAULT,
    "sII",
    "orientation min max",
    "make this label present the stat using a graphical bar.  Optionally specify a min and max value"
);
Event EV_Layout_StatbarTileShader
(
    "statbar_tileshader",
    EV_DEFAULT,
    "s",
    "shader",
    "set the tile shader for this statbar"
);
Event EV_Layout_StatbarTileShader_Flash
(
    "statbar_tileshader_flash",
    EV_DEFAULT,
    "s",
    "shader",
    "set the flash tile shader for this statbar"
);
Event EV_Layout_StatbarShader
(
    "statbar_shader",
    EV_DEFAULT,
    "s",
    "shader",
    "set the shader for this statbar"
);
Event EV_Layout_StatbarShader_Flash
(
    "statbar_shader_flash",
    EV_DEFAULT,
    "s",
    "shader",
    "set the flash shader for this statbar"
);
// Added in 2.0
Event EV_Layout_StatbarShader_Marker
(
    "statbar_shader_marker",
    EV_DEFAULT,
    "s",
    "shader",
    "set the marker shader for this statbar\nThis is drawn at the end of the status bar"
);
Event EV_Layout_Statbar_EndAngles
(
    "statbar_endangles",
    EV_DEFAULT,
    "ff",
    "startang endang",
    "Sets the start and end angles for a circular stat bar"
);
Event EV_Layout_Statbar_NeedleWidth
(
    "statbar_needlewidth",
    EV_DEFAULT,
    "f",
    "width",
    "Sets the width of the needle for a needle statbar"
);
Event EV_Layout_Statbar_RotatorSize
(
    "statbar_rotatorsize",
    EV_DEFAULT,
    "f",
    "width",
    "Sets the width of the needle for a needle statbar"
);
Event EV_Layout_DrawModelName
(
    "invmodelname",
    EV_DEFAULT,
    "s",
    "name",
    "set the name of the 3d model to render\n"
);
Event EV_Layout_RenderModel
(
    "rendermodel",
    EV_DEFAULT,
    "b",
    "bool",
    "Render the model specified by the cvar."
);
Event EV_Layout_RenderModelOffset
(
    "modeloffset",
    EV_DEFAULT,
    "v",
    "offset",
    "Render model offset"
);
Event EV_Layout_RenderModelRotateOffset
(
    "modelrotateoffset",
    EV_DEFAULT,
    "v",
    "offset",
    "Render model rotation offset"
);
Event EV_Layout_RenderModelAngles
(
    "modelangles",
    EV_DEFAULT,
    "v",
    "angles",
    "Render model angles"
);
Event EV_Layout_RenderModelScale
(
    "modelscale",
    EV_DEFAULT,
    "f",
    "scale",
    "Render model scale"
);
Event EV_Layout_RenderModelAnim
(
    "modelanim",
    EV_DEFAULT,
    "s",
    "anim",
    "Render model anim"
);
Event EV_ClearInvItemReference
(
    "clearinvitemref",
    EV_DEFAULT,
    NULL,
    NULL,
    "used internally when the inventory is reset\n"
);

CLASS_DECLARATION(UILabel, UIFakkLabel, NULL) {
    {&EV_Layout_PlayerStat,                &UIFakkLabel::LayoutPlayerStat             },
    {&EV_Layout_PlayerStatAlpha,           &UIFakkLabel::LayoutPlayerStatAlpha        },
    {&EV_Layout_PlayerStatConfigstring,    &UIFakkLabel::LayoutPlayerStatConfigstring },
    {&EV_Layout_MaxPlayerStat,             &UIFakkLabel::LayoutMaxPlayerStat          },
    {&EV_Layout_ItemIndex,                 &UIFakkLabel::LayoutItemIndex              },
    {&EV_Layout_InventoryRenderModelIndex, &UIFakkLabel::InventoryRenderModelIndex    },
    {&EV_Layout_Statbar,                   &UIFakkLabel::LayoutStatbar                },
    {&EV_Layout_StatbarTileShader,         &UIFakkLabel::LayoutStatbarTileShader      },
    {&EV_Layout_StatbarShader,             &UIFakkLabel::LayoutStatbarShader          },
    {&EV_Layout_StatbarTileShader_Flash,   &UIFakkLabel::LayoutStatbarTileShader_Flash},
    {&EV_Layout_StatbarShader_Marker,      &UIFakkLabel::LayoutStatbarShader_Marker   },
    {&EV_Layout_StatbarShader_Flash,       &UIFakkLabel::LayoutStatbarShader_Flash    },
    {&EV_Layout_Statbar_EndAngles,         &UIFakkLabel::LayoutStatbarEndAngles       },
    {&EV_Layout_Statbar_NeedleWidth,       &UIFakkLabel::LayoutStatbarNeedleWidth     },
    {&EV_Layout_Statbar_RotatorSize,       &UIFakkLabel::LayoutStatbarRotatorSize     },
    {&EV_Layout_DrawModelName,             &UIFakkLabel::LayoutModelName              },
    {&EV_Layout_RenderModel,               &UIFakkLabel::LayoutRenderModel            },
    {&EV_Layout_RenderModelOffset,         &UIFakkLabel::LayoutRenderModelOffset      },
    {&EV_Layout_RenderModelRotateOffset,   &UIFakkLabel::LayoutRenderModelRotateOffset},
    {&EV_Layout_RenderModelAngles,         &UIFakkLabel::LayoutRenderModelAngles      },
    {&EV_Layout_RenderModelScale,          &UIFakkLabel::LayoutRenderModelScale       },
    {&EV_Layout_RenderModelAnim,           &UIFakkLabel::LayoutRenderModelAnim        },
    {&EV_ClearInvItemReference,            &UIFakkLabel::ClearInvItemReference        },
    {NULL,                                 NULL                                       }
};

UIFakkLabel::UIFakkLabel()
{
    m_statbar_min = m_statbar_max = -1.0;
    m_stat                        = -1;
    m_stat_alpha                  = -1;
    m_stat_configstring           = -1;
    m_maxstat                     = -1;
    m_itemindex                   = -1;
    m_inventoryrendermodelindex   = -1;
    m_rendermodel                 = false;
    m_statbar_or                  = L_STATBAR_NONE;
    m_statbar_material            = NULL;
    m_statbar_material_flash      = NULL;
    m_statbar_material_marker     = NULL;
    m_lastitemindex               = -1;
}

void UIFakkLabel::LayoutPlayerStat(Event *ev)
{
    m_stat = ev->GetInteger(1);
}

void UIFakkLabel::LayoutPlayerStatAlpha(Event *ev)
{
    m_stat_alpha = ev->GetInteger(1);
}

void UIFakkLabel::LayoutPlayerStatConfigstring(Event *ev)
{
    m_stat_configstring = ev->GetInteger(1);
}

void UIFakkLabel::LayoutMaxPlayerStat(Event *ev)
{
    m_maxstat = ev->GetInteger(1);
}

void UIFakkLabel::LayoutItemIndex(Event *ev)
{
    m_itemindex = ev->GetInteger(1);
}

void UIFakkLabel::InventoryRenderModelIndex(Event *ev)
{
    m_inventoryrendermodelindex = ev->GetInteger(1);
}

void UIFakkLabel::LayoutModelName(Event *ev)
{
    m_sDrawModelName = ev->GetString(1);
}

void UIFakkLabel::ClearInvItemReference(Event *ev)
{
    m_lastitem      = NULL;
    m_lastitemindex = -1;
}

void UIFakkLabel::LayoutStatbar(Event *ev)
{
    str _or = ev->GetString(1);

    if (ev->NumArgs() > 1) {
        m_statbar_min = ev->GetFloat(2);
        m_statbar_max = ev->GetFloat(3);
    }

    if (!str::icmp(_or, "horizontal")) {
        m_statbar_or = L_STATBAR_HORIZONTAL;
    } else if (!str::icmp(_or, "vertical_stagger_even")) {
        m_statbar_or = L_STATBAR_VERTICAL_STAGGER_EVEN;
    } else if (!str::icmp(_or, "vertical_stagger_odd")) {
        m_statbar_or = L_STATBAR_VERTICAL_STAGGER_ODD;
    } else if (!str::icmp(_or, "vertical")) {
        m_statbar_or = L_STATBAR_VERTICAL;
    } else if (!str::icmp(_or, "vertical_reverse")) {
        m_statbar_or = L_STATBAR_VERTICAL_REVERSE;
    } else if (!str::icmp(_or, "circular")) {
        m_statbar_or = L_STATBAR_CIRCULAR;

        if (!m_statbar_material) {
            m_statbar_material = uWinMan.RegisterShader("blank");
        }
    } else if (!str::icmp(_or, "needle")) {
        m_statbar_or = L_STATBAR_NEEDLE;

        if (!m_statbar_material) {
            m_statbar_material = uWinMan.RegisterShader("blank");
        }
    } else if (!str::icmp(_or, "rotator")) {
        m_statbar_or = L_STATBAR_ROTATOR;

        if (!m_statbar_material) {
            m_statbar_material = uWinMan.RegisterShader("blank");
        }
    } else if (!str::icmp(_or, "compass")) {
        m_statbar_or = L_STATBAR_COMPASS;

        if (!m_statbar_material) {
            m_statbar_material = uWinMan.RegisterShader("blank");
        }
    } else if (!str::icmp(_or, "spinner")) {
        m_statbar_or = L_STATBAR_SPINNER;

        if (!m_statbar_material) {
            m_statbar_material = uWinMan.RegisterShader("blank");
        }
    } else if (!str::icmp(_or, "headingspinner")) {
        m_statbar_or = L_STATBAR_HEADING_SPINNER;

        if (!m_statbar_material) {
            m_statbar_material = uWinMan.RegisterShader("blank");
        }
    } else {
        warning("LayoutPlayerStat", "Invalid statbar orientation: \"%s\"", _or.c_str());
    }
}

void UIFakkLabel::LayoutStatbarShader(Event *ev)
{
    m_statbar_material = uWinMan.RegisterShader(ev->GetString(1));
}

void UIFakkLabel::LayoutStatbarTileShader(Event *ev)
{
    m_statbar_material = uWinMan.RegisterShader(ev->GetString(1));
    AddFlag(WF_TILESHADER);
}

void UIFakkLabel::LayoutStatbarShader_Flash(Event *ev)
{
    m_statbar_material_flash = uWinMan.RegisterShader(ev->GetString(1));
}

void UIFakkLabel::LayoutStatbarShader_Marker(Event *ev)
{
    m_statbar_material_marker = uWinMan.RegisterShader(ev->GetString(1));
}

void UIFakkLabel::LayoutStatbarEndAngles(Event *ev)
{
    m_angles[0] = ev->GetFloat(1);
    m_angles[1] = ev->GetFloat(2);
}

void UIFakkLabel::LayoutStatbarNeedleWidth(Event *ev)
{
    m_angles[2] = ev->GetFloat(1);
}

void UIFakkLabel::LayoutStatbarRotatorSize(Event *ev)
{
    m_angles[2] = ev->GetFloat(1);

    if (ev->NumArgs() > 1) {
        m_scale = ev->GetFloat(2);
    } else {
        m_scale = m_angles[2];
    }
}

void UIFakkLabel::LayoutStatbarTileShader_Flash(Event *ev)
{
    m_statbar_material_flash = uWinMan.RegisterShader(ev->GetString(1));
    AddFlag(WF_TILESHADER);
}

void UIFakkLabel::LayoutRenderModel(Event *ev)
{
    m_rendermodel = ev->GetBoolean(1);
}

void UIFakkLabel::LayoutRenderModelOffset(Event *ev)
{
    Vector vec = ev->GetVector(1);
    VectorCopy(vec, m_offset);
}

void UIFakkLabel::LayoutRenderModelRotateOffset(Event *ev)
{
    Vector vec = ev->GetVector(1);
    VectorCopy(vec, m_rotateoffset);
}

void UIFakkLabel::LayoutRenderModelAngles(Event *ev)
{
    Vector vec = ev->GetVector(1);
    VectorCopy(vec, m_angles);
}

void UIFakkLabel::LayoutRenderModelScale(Event *ev)
{
    m_scale = ev->GetFloat(1);
}

void UIFakkLabel::LayoutRenderModelAnim(Event *ev)
{
    m_anim = ev->GetString(1);
}

void UIFakkLabel::DrawStatbar(float frac)
{
    vec4_t col;
    float  alpha;

    col[0] = col[1] = col[2] = col[3] = 1.0;

    switch (m_statbar_or) {
    case L_STATBAR_CIRCULAR:
        DrawStatCircle(frac);
        return;
    case L_STATBAR_NEEDLE:
        DrawStatNeedle(frac);
        return;
    case L_STATBAR_ROTATOR:
        DrawStatRotator(frac);
        return;
    case L_STATBAR_COMPASS:
        DrawStatCompass(frac);
        return;
    case L_STATBAR_SPINNER:
    case L_STATBAR_HEADING_SPINNER:
        DrawStatSpinner(frac);
        return;
    default:
        break;
    }

    if (!frac) {
        return;
    }

    if (frac != m_lastfrac) {
        m_flashtime = uid.time;
    }
    m_lastfrac = frac;

    alpha = 1.0 - ((float)uid.time - m_flashtime) / 1500.0;
    if (alpha < 0.0) {
        alpha = 0.0;
    } else if (alpha > 1.0) {
        alpha = 1.0;
    }

    col[3] = alpha;

    if (m_statbar_material) {
        if (m_flags & WF_TILESHADER) {
            switch (m_statbar_or) {
            case L_STATBAR_HORIZONTAL:
                {
                    float width = frac * m_frame.size.width;

                    m_statbar_material->ReregisterMaterial();
                    re.DrawTilePic(0.0, 0.0, width, m_frame.size.height, m_statbar_material->GetMaterial());

                    if (alpha != 0.0 && m_statbar_material_flash) {
                        re.SetColor(col);
                        m_statbar_material_flash->ReregisterMaterial();
                        re.DrawTilePic(0.0, 0.0, width, m_frame.size.height, m_statbar_material_flash->GetMaterial());
                    }
                    break;
                }
            case L_STATBAR_VERTICAL:
            case L_STATBAR_VERTICAL_REVERSE:
            case L_STATBAR_VERTICAL_STAGGER_EVEN:
            case L_STATBAR_VERTICAL_STAGGER_ODD:
                {
                    float y = m_frame.size.height * (1.0 - frac);

                    m_statbar_material->ReregisterMaterial();
                    re.DrawTilePic(0.0, y, m_frame.size.width, m_frame.size.height, m_statbar_material->GetMaterial());

                    if (alpha != 0.0 && m_statbar_material_flash) {
                        re.SetColor(col);
                        m_statbar_material_flash->ReregisterMaterial();
                        re.DrawTilePic(
                            0.0, y, m_frame.size.width, m_frame.size.height, m_statbar_material_flash->GetMaterial()
                        );
                    }
                    break;
                }
            default:
                break;
            }
        } else {
            switch (m_statbar_or) {
            case L_STATBAR_HORIZONTAL:
                {
                    float width = frac * m_frame.size.width;

                    m_statbar_material->ReregisterMaterial();
                    re.DrawStretchPic(
                        0.0, 0.0, width, m_frame.size.height, 0.0, 0.0, 1.0, 1.0, m_statbar_material->GetMaterial()
                    );

                    if (alpha != 0.0 && m_statbar_material_flash) {
                        re.SetColor(col);
                        m_statbar_material_flash->ReregisterMaterial();
                        re.DrawStretchPic(
                            0.0,
                            0.0,
                            width,
                            m_frame.size.height,
                            0.0,
                            0.0,
                            1.0,
                            1.0,
                            m_statbar_material_flash->GetMaterial()
                        );
                    }
                    break;
                }
            case L_STATBAR_VERTICAL:
            case L_STATBAR_VERTICAL_STAGGER_EVEN:
            case L_STATBAR_VERTICAL_STAGGER_ODD:
                {
                    float y      = m_frame.size.height * (1.0 - frac);
                    float height = m_frame.size.height * frac;

                    m_statbar_material->ReregisterMaterial();
                    re.DrawStretchPic(
                        0.0, y, m_frame.size.width, height, 0.0, 1.0 - frac, 1.0, 1.0, m_statbar_material->GetMaterial()
                    );

                    if (alpha != 0.0 && m_statbar_material_flash) {
                        re.SetColor(col);
                        m_statbar_material_flash->ReregisterMaterial();
                        re.DrawStretchPic(
                            0.0,
                            y,
                            m_frame.size.width,
                            height,
                            0.0,
                            1.0 - frac,
                            1.0,
                            1.0,
                            m_statbar_material_flash->GetMaterial()
                        );
                    }
                    break;
                }
            case L_STATBAR_VERTICAL_REVERSE:
                {
                    float height = m_frame.size.height * frac;

                    m_statbar_material->ReregisterMaterial();
                    re.DrawStretchPic(
                        0.0, 0.0, m_frame.size.width, height, 0.0, 0.0, 1.0, frac, m_statbar_material->GetMaterial()
                    );

                    if (alpha != 0.0 && m_statbar_material_flash) {
                        re.SetColor(col);
                        m_statbar_material_flash->ReregisterMaterial();
                        re.DrawStretchPic(
                            0.0,
                            0.0,
                            m_frame.size.width,
                            height,
                            0.0,
                            0.0,
                            1.0,
                            frac,
                            m_statbar_material_flash->GetMaterial()
                        );
                    }
                    break;
                }
            default:
                break;
            }
        }
    } else {
        switch (m_statbar_or) {
        case L_STATBAR_VERTICAL:
        case L_STATBAR_VERTICAL_STAGGER_EVEN:
        case L_STATBAR_VERTICAL_STAGGER_ODD:
            DrawBox(
                0.0,
                m_frame.size.height * (1.0 - frac),
                m_frame.size.width,
                m_frame.size.height,
                m_foreground_color,
                1.0
            );
            break;
        case L_STATBAR_HORIZONTAL:
            DrawBox(0.0, 0.0, frac * m_frame.size.width, m_frame.size.height, m_foreground_color, 1.0);
            break;
        default:
            break;
        }
    }
}

void UIFakkLabel::DrawStatCircle(float frac)
{
    vec4_t col;
    float  alpha;
    float  fCurrAng, fEndAng, fNextAng;
    vec2_t vVerts[3];
    vec2_t vTexCoords[3];

    col[0] = 1.f;
    col[1] = 1.f;
    col[2] = 1.f;
    col[3] = 1.f;

    if (m_lastfrac != frac) {
        m_flashtime = uid.time;
    }
    m_lastfrac = frac;

    alpha = 1.f - (uid.time - m_flashtime) / 1500.f;
    if (alpha < 0.f) {
        alpha = 0.f;
    } else if (alpha > 1.f) {
        alpha = 1.f;
    }

    col[3] = alpha;

    vVerts[0][0]     = m_frame.size.width * 0.5f;
    vVerts[0][1]     = m_frame.size.height * 0.5f;
    vTexCoords[0][0] = 0.5f;
    vTexCoords[0][1] = 0.5f;

    if (ui_health_start->value && ui_health_end->value) {
        m_angles[0] = ui_health_start->value;
        m_angles[1] = ui_health_end->value;
    }

    fCurrAng = m_angles[0];
    fEndAng  = frac * (m_angles[1] - fCurrAng) + fCurrAng;
    if (m_angles[1] > fCurrAng) {
        StatCircleTexCoord(fCurrAng, vTexCoords[1]);
        vVerts[1][0] = m_frame.size.width * vTexCoords[1][0];
        vVerts[1][1] = m_frame.size.height * vTexCoords[1][1];

        if ((int)fCurrAng - (((int)fCurrAng - 45) % 90 + 90) % 90
            == (int)fEndAng - (((int)fEndAng - 45) % 90 + 90) % 90) {
            StatCircleTexCoord(fEndAng, vTexCoords[2]);
            vVerts[2][0] = m_frame.size.width * vTexCoords[2][0];
            vVerts[2][1] = m_frame.size.height * vTexCoords[2][1];

            if (alpha && m_statbar_material_flash) {
                re.SetColor(m_foreground_color);
            }

            m_statbar_material->ReregisterMaterial();
            re.DrawTrianglePic(vVerts, vTexCoords, m_statbar_material->GetMaterial());

            if (alpha && m_statbar_material_flash) {
                re.SetColor(col);
                m_statbar_material_flash->ReregisterMaterial();
                re.DrawTrianglePic(vVerts, vTexCoords, m_statbar_material_flash->GetMaterial());
            }
        } else {
            while (fCurrAng > fEndAng) {
                fNextAng = ((int)fCurrAng - (((int)fCurrAng - 45) % 90 + 90) % 90) + 90.0;
                if (fNextAng > fEndAng) {
                    fNextAng = fEndAng;
                }

                StatCircleTexCoord(fNextAng, vTexCoords[2]);
                vVerts[2][0] = m_frame.size.width * vTexCoords[2][0];
                vVerts[2][1] = m_frame.size.height * vTexCoords[2][1];

                if (alpha && m_statbar_material_flash) {
                    re.SetColor(m_foreground_color);
                }

                m_statbar_material->ReregisterMaterial();
                re.DrawTrianglePic(vVerts, vTexCoords, m_statbar_material->GetMaterial());

                if (alpha && m_statbar_material_flash) {
                    re.SetColor(col);
                    m_statbar_material_flash->ReregisterMaterial();
                    re.DrawTrianglePic(vVerts, vTexCoords, m_statbar_material_flash->GetMaterial());
                }

                vTexCoords[1][0] = vTexCoords[2][0];
                vTexCoords[1][1] = vTexCoords[2][1];
                vVerts[1][0]     = vVerts[2][0];
                vVerts[1][1]     = vVerts[2][1];
                fCurrAng         = fNextAng;
            }
        }
    } else {
        StatCircleTexCoord(fCurrAng, vTexCoords[2]);
        vVerts[2][0] = m_frame.size.width * vTexCoords[2][0];
        vVerts[2][1] = m_frame.size.height * vTexCoords[2][1];
        if ((int)fCurrAng - (((int)fCurrAng - 45) % 90 + 90) % 90
            == (int)fEndAng - (((int)fEndAng - 45) % 90 + 90) % 90) {
            StatCircleTexCoord(fEndAng, vTexCoords[1]);
            vVerts[1][0] = m_frame.size.width * vTexCoords[1][0];
            vVerts[1][1] = m_frame.size.height * vTexCoords[1][1];

            if (alpha && m_statbar_material_flash) {
                re.SetColor(m_foreground_color);
            }

            m_statbar_material->ReregisterMaterial();
            re.DrawTrianglePic(vVerts, vTexCoords, m_statbar_material->GetMaterial());

            if (alpha && m_statbar_material_flash) {
                re.SetColor(col);
                m_statbar_material_flash->ReregisterMaterial();
                re.DrawTrianglePic(vVerts, vTexCoords, m_statbar_material_flash->GetMaterial());
            }
        } else {
            while (fCurrAng > fEndAng) {
                fNextAng = ((int)fCurrAng - (((int)fCurrAng + 45) % 90 + 90) % 90);
                if (fNextAng == fCurrAng) {
                    fNextAng -= 90.f;
                }

                if (fNextAng < fEndAng) {
                    fNextAng = fEndAng;
                }

                StatCircleTexCoord(fNextAng, vTexCoords[1]);
                vVerts[1][0] = m_frame.size.width * vTexCoords[1][0];
                vVerts[1][1] = m_frame.size.height * vTexCoords[1][1];

                if (alpha && m_statbar_material_flash) {
                    re.SetColor(m_foreground_color);
                }

                m_statbar_material->ReregisterMaterial();
                re.DrawTrianglePic(vVerts, vTexCoords, m_statbar_material->GetMaterial());

                if (alpha && m_statbar_material_flash) {
                    re.SetColor(col);
                    m_statbar_material_flash->ReregisterMaterial();
                    re.DrawTrianglePic(vVerts, vTexCoords, m_statbar_material_flash->GetMaterial());
                }

                vTexCoords[2][0] = vTexCoords[1][0];
                vTexCoords[2][1] = vTexCoords[1][1];
                vVerts[2][0]     = vVerts[1][0];
                vVerts[2][1]     = vVerts[1][1];
                fCurrAng         = fNextAng;
            }
        }
    }
}

void UIFakkLabel::DrawStatNeedle(float frac)
{
    vec4_t col;
    float  alpha;
    float  fTargAng;
    float  fSinVal;
    float  fCosVal;
    vec2_t vCenter;
    vec2_t vVerts[3];
    vec2_t vTexCoords[3];
    vec2_t vNeedleDir;
    vec2_t vSideDir;

    col[0] = 1.f;
    col[1] = 1.f;
    col[2] = 1.f;
    col[3] = 1.f;

    if (m_lastfrac != frac) {
        m_flashtime = uid.time;
    }
    m_lastfrac = frac;

    alpha = 1.f - (uid.time - m_flashtime) / 1500.f;
    if (alpha < 0.f) {
        alpha = 0.f;
    } else if (alpha > 1.f) {
        alpha = 1.f;
    }

    col[3] = alpha;

    fTargAng = DEG2RAD(frac * (m_angles[1] - m_angles[0]) + m_angles[0]);
    fSinVal  = sin(fTargAng);
    fCosVal  = cos(fTargAng);

    vSideDir[0]   = m_frame.size.width * 0.5f;
    vSideDir[1]   = m_frame.size.height * 0.5f;
    vCenter[0]    = m_frame.size.width + fSinVal * vSideDir[0];
    vCenter[1]    = m_frame.size.height + -fCosVal * vSideDir[1];
    vNeedleDir[0] = m_angles[2] * fCosVal;
    vNeedleDir[1] = m_angles[2] * fSinVal;

    vVerts[0][0] = vCenter[0] - vNeedleDir[0];
    vVerts[0][1] = vCenter[1] - vNeedleDir[1];
    vVerts[1][0] = vCenter[0] + vNeedleDir[0];
    vVerts[1][1] = vCenter[1] + vNeedleDir[1];
    vVerts[2][0] = vSideDir[0] - vNeedleDir[1] - vNeedleDir[0];
    vVerts[2][1] = vSideDir[1] + vNeedleDir[0] - vNeedleDir[1];

    vTexCoords[0][0] = 0.f;
    vTexCoords[0][1] = 0.f;
    vTexCoords[1][0] = 1.f;
    vTexCoords[1][1] = 0.f;
    vTexCoords[2][0] = 0.f;
    vTexCoords[2][1] = 1.f;

    if (alpha != 0.f) {
        re.SetColor(m_foreground_color);
    }

    m_statbar_material->ReregisterMaterial();
    re.DrawTrianglePic(vVerts, vTexCoords, m_statbar_material->GetMaterial());

    if (alpha != 0.f && m_statbar_material_flash) {
        re.SetColor(col);
        m_statbar_material_flash->ReregisterMaterial();
        re.DrawTrianglePic(vVerts, vTexCoords, m_statbar_material_flash->GetMaterial());
    }

    vVerts[0][0] = vVerts[2][0];
    vVerts[0][1] = vVerts[2][1];
    vVerts[2][0] = m_angles[2] * vCenter[0] + m_frame.size.width * 0.5f - m_angles[2] * vCenter[1];
    vVerts[2][1] = m_angles[2] * vCenter[1] + m_frame.size.height * 0.5f - m_angles[2] * -vCenter[0];

    vTexCoords[0][0] = vTexCoords[2][0];
    vTexCoords[0][1] = vTexCoords[2][1];

    if (alpha != 0.f && m_statbar_material_flash) {
        re.SetColor(m_foreground_color);
    }

    m_statbar_material->ReregisterMaterial();
    re.DrawTrianglePic(vVerts, vTexCoords, m_statbar_material->GetMaterial());

    if (alpha != 0.f && m_statbar_material_flash) {
        m_statbar_material_flash->ReregisterMaterial();
        re.DrawTrianglePic(vVerts, vTexCoords, m_statbar_material_flash->GetMaterial());
    }
}

void UIFakkLabel::DrawStatRotator(float frac)
{
    vec4_t col;
    float  alpha;
    float  fTargAng;
    float  fSinVal, fCosVal;
    vec2_t vCenter;
    vec2_t vVerts[3];
    vec2_t vTexCoords[3];
    vec2_t vNeedleDir;
    vec2_t vSize;

    col[0] = 1.f;
    col[1] = 1.f;
    col[2] = 1.f;
    col[3] = 1.f;

    if (m_lastfrac != frac) {
        m_flashtime = uid.time;
    }
    m_lastfrac = frac;

    alpha = 1.f - (uid.time - m_flashtime) / 1500.f;
    if (alpha < 0.f) {
        alpha = 0.f;
    } else if (alpha > 1.f) {
        alpha = 1.f;
    }

    col[3] = alpha;

    fTargAng      = DEG2RAD(frac * (m_angles[1] - m_angles[0]) + m_angles[0]);
    fSinVal       = sin(fTargAng);
    fCosVal       = cos(fTargAng);
    vNeedleDir[0] = fSinVal;
    vNeedleDir[1] = -fCosVal;

    vSize[0] = (m_frame.size.width + m_frame.size.height) / m_frame.size.width * m_angles[2] * m_vVirtualScale[0];
    vSize[1] = (m_frame.size.width + m_frame.size.height) / m_frame.size.height * m_scale * m_vVirtualScale[1];

    vCenter[0] = (m_frame.size.width * 0.5f - vSize[0]) * vNeedleDir[0] + m_frame.size.width * 0.5f;
    vCenter[1] = (m_frame.size.height * 0.5f - vSize[1]) * vNeedleDir[1] + m_frame.size.height * 0.5f;

    vVerts[0][0] = vCenter[0] - vSize[0];
    vVerts[0][1] = vCenter[1] - vSize[1];
    vVerts[1][0] = vCenter[0] + vSize[0];
    vVerts[1][1] = vCenter[1] - vSize[1];
    vVerts[2][0] = vCenter[0] - vSize[0];
    vVerts[2][1] = vCenter[1] + vSize[1];

    vTexCoords[0][0] = 0.f;
    vTexCoords[0][1] = 0.f;
    vTexCoords[1][0] = 1.f;
    vTexCoords[1][1] = 0.f;
    vTexCoords[2][0] = 0.f;
    vTexCoords[2][1] = 1.f;

    if (alpha != 0.f && m_statbar_material_flash) {
        re.SetColor(m_foreground_color);
    }

    m_statbar_material->ReregisterMaterial();
    re.DrawTrianglePic(vVerts, vTexCoords, m_statbar_material->GetMaterial());

    if (alpha != 0.f && m_statbar_material_flash) {
        m_statbar_material_flash->ReregisterMaterial();
        re.DrawTrianglePic(vVerts, vTexCoords, m_statbar_material_flash->GetMaterial());
    }

    vVerts[0][0] = vVerts[2][0];
    vVerts[0][1] = vVerts[2][1];
    vVerts[2][0] = vCenter[0] + vSize[0];
    vVerts[2][1] = vCenter[1] + vSize[1];

    vTexCoords[0][0] = vTexCoords[2][0];
    vTexCoords[0][1] = vTexCoords[2][1];
    vTexCoords[2][0] = 1.f;
    vTexCoords[2][1] = 1.f;

    if (alpha != 0.f && m_statbar_material_flash) {
        re.SetColor(m_foreground_color);
    }

    m_statbar_material->ReregisterMaterial();
    re.DrawTrianglePic(vVerts, vTexCoords, m_statbar_material->GetMaterial());

    if (alpha != 0.f && m_statbar_material_flash) {
        m_statbar_material_flash->ReregisterMaterial();
        re.DrawTrianglePic(vVerts, vTexCoords, m_statbar_material_flash->GetMaterial());
    }
}

void UIFakkLabel::DrawStatCompass(float frac)
{
    vec4_t col;
    float  fTargAng;
    float  fSinVal, fCosVal;
    vec2_t vCenter;
    vec2_t vVerts[3];
    vec2_t vTexCoords[3];
    vec2_t vCompassDir;
    vec2_t vSideDir;

    col[0] = col[1] = col[2] = col[3] = 1.0;

    if (!cge) {
        return;
    }

    static int   iLastCompassTime = -9999, iLastTimeDelta = 0;
    static float fLastPitch = 0, fLastYaw = 0, fLastYawDelta = 0;
    static float fYawOffset = 0, fYawSpeed = 0;
    static float fNeedleOffset = 0, fNeedleSpeed = 0;
    int          iTimeCount, iTimeDelta;
    float        fYawDelta, fYawDeltaDiff;
    vec3_t       vViewAngles;

    cge->CG_EyeAngles(&vViewAngles);

    vViewAngles[1] -= cl.snap.ps.stats[STAT_COMPASSNORTH] / 182.0f;

    iTimeCount = uid.time - iLastCompassTime;
    if (iTimeCount <= 1000) {
        fYawDelta = AngleSubtract(vViewAngles[1], fLastYaw);
        if (fYawDelta > 0.1) {
            if ((fYawDelta > 0.0 && fLastYawDelta < fYawDelta) || (fYawDelta < 0.0 && fLastYawDelta > fYawDelta)) {
                fYawSpeed += (fYawDelta - fLastYawDelta) * 0.75;
                fYawOffset += (fYawDelta - fLastYawDelta) * 0.75;
            }

            if (fYawSpeed > 90.0) {
                fYawSpeed = 90.0;
            } else if (fYawSpeed < 0.05) {
                fYawSpeed = 0.05;
            }
        }

        while (iTimeCount > 0) {
            iTimeDelta = iTimeCount;
            if (iTimeCount > 15) {
                iTimeDelta = 15;
            }
            iTimeCount -= iTimeDelta;

            if (fabs(fYawOffset) >= 0.1 || fabs(fYawSpeed) >= 0.01) {
                fYawOffset += fYawSpeed * iTimeDelta;
                if (fYawOffset > 0.0) {
                    fYawSpeed -= iTimeDelta * 0.00175;
                } else if (fYawOffset < 0.0) {
                    fYawSpeed += iTimeDelta * 0.00175;
                }

                if (fYawOffset > 40.0) {
                    fYawOffset = 40.0;
                    fYawSpeed  = 0.0;
                } else if (fYawOffset < -40.0) {
                    fYawOffset = -40.0;
                    fYawSpeed  = 0.0;
                }

                fYawSpeed -= iTimeDelta * 0.003;
                if (fYawSpeed > 0.0) {
                    fYawSpeed -= iTimeDelta * 0.0004;
                } else {
                    fYawSpeed += iTimeDelta * 0.0004;
                    if (fYawSpeed > 0.0) {
                        fYawSpeed = 0.0;
                    }
                }
            } else {
                iTimeCount = 0;
                fYawOffset = 0.0;
                fYawSpeed  = 0.0;
            }
        }

        fLastYawDelta  = fYawDelta;
        iLastTimeDelta = uid.time - iLastCompassTime;
    } else {
        fLastYawDelta  = 0;
        fYawOffset     = 0;
        fYawSpeed      = 0;
        iLastTimeDelta = 0;
    }

    fLastPitch       = vViewAngles[0];
    fLastYaw         = vViewAngles[1];
    iLastCompassTime = uid.time;

    if (iLastTimeDelta) {
        if (fabs(fLastYawDelta) > 0.1) {
            fNeedleOffset -= fLastYawDelta;
        }

        iTimeCount = iLastTimeDelta;
        while (iTimeCount > 0) {
            iTimeDelta = iTimeCount;
            if (iTimeCount > 15) {
                iTimeDelta = 15;
            }
            iTimeCount -= iTimeDelta;

            if (fabs(fNeedleOffset) >= 0.1 || fabs(fNeedleSpeed) >= 0.01) {
                fNeedleOffset += iTimeDelta * fNeedleSpeed;
                if (fNeedleOffset > 180.0) {
                    fNeedleOffset -= 360.0;
                } else if (fNeedleOffset < -180.0) {
                    fNeedleOffset += 360.0;
                }

                if (fNeedleOffset > 0.0) {
                    fNeedleSpeed -= iTimeDelta * 0.00175;
                } else if (fNeedleOffset < 0.0) {
                    fNeedleSpeed += iTimeDelta * 0.00175;
                }

                fNeedleSpeed -= fNeedleSpeed * 0.0025 * iTimeDelta;
                if (fNeedleSpeed > 0.0) {
                    fNeedleSpeed -= iTimeDelta * 0.00035;
                    if (fNeedleSpeed < 0.0) {
                        fNeedleSpeed = 0.0;
                    }
                } else {
                    fNeedleSpeed += iTimeDelta * 0.00035;
                    if (fNeedleSpeed > 0.0) {
                        fNeedleSpeed = 0.0;
                    }
                }
            } else {
                iTimeCount    = 0;
                fNeedleOffset = 0.0;
                fNeedleSpeed  = 0.0;
            }
        }
    } else {
        fNeedleOffset = 0.0;
        fNeedleSpeed  = 0.0;
    }

    fSinVal = sin(anglemod(fNeedleOffset + fLastYaw) / (180 / M_PI));
    fCosVal = cos(anglemod(fNeedleOffset + fLastYaw) / (180 / M_PI));

    vCompassDir[0] = fSinVal;
    vCompassDir[1] = -fCosVal;

    vCenter[0]       = m_frame.size.width * 0.5;
    vCenter[1]       = m_frame.size.height * 0.5;
    vTexCoords[0][0] = 0.0;
    vTexCoords[0][1] = 0.0;
    vTexCoords[1][0] = 1.0;
    vTexCoords[1][1] = 0.0;
    vTexCoords[2][0] = 0.0;
    vTexCoords[2][1] = 1.0;

    vVerts[0][0] = vCenter[0] + vCompassDir[0] * vCenter[0] - fCosVal * vCenter[1];
    vVerts[1][0] = vCenter[0] + vCompassDir[0] * vCenter[0] + fCosVal * vCenter[1];
    vVerts[2][0] = vCenter[0] - vCompassDir[0] * vCenter[0] - fCosVal * vCenter[1];
    vVerts[0][1] = vCenter[1] + vCompassDir[1] * vCenter[1] - fSinVal * vCenter[0];
    vVerts[1][1] = vCenter[1] + vCompassDir[1] * vCenter[1] + fSinVal * vCenter[0];
    vVerts[2][1] = vCenter[1] - vCompassDir[1] * vCenter[1] - fSinVal * vCenter[0];

    m_statbar_material->ReregisterMaterial();
    re.DrawTrianglePic(vVerts, vTexCoords, m_statbar_material->GetMaterial());

    vVerts[0][0]     = vVerts[2][0];
    vVerts[0][1]     = vVerts[2][1];
    vTexCoords[0][0] = vTexCoords[2][0];
    vTexCoords[0][1] = vTexCoords[2][1];
    vTexCoords[2][0] = 1.0;
    vTexCoords[2][1] = 1.0;

    vVerts[2][0] = vCenter[0] + fCosVal * vCenter[1] - vCompassDir[0] * vCenter[0];
    vVerts[2][1] = vCenter[1] * fSinVal + vCenter[0] - vCompassDir[1] * vCenter[1];

    m_statbar_material->ReregisterMaterial();
    re.DrawTrianglePic(vVerts, vTexCoords, m_statbar_material->GetMaterial());
}

void UIFakkLabel::DrawStatSpinner(float frac)
{
    vec4_t col;
    float  alpha;
    float  fTargAng;
    float  fSinVal, fCosVal;
    vec2_t vCenter;
    vec2_t vVerts[3];
    vec2_t vTexCoords[3];
    vec2_t vCompassDir;
    vec2_t vSideDir;

    col[0] = 1.f;
    col[1] = 1.f;
    col[2] = 1.f;
    col[3] = 1.f;

    if (m_lastfrac != frac) {
        m_flashtime = uid.time;
    }
    m_lastfrac = frac;

    alpha = 1.f - (uid.time - m_flashtime) / 1500.f;
    if (alpha < 0.f) {
        alpha = 0.f;
    } else if (alpha > 1.f) {
        alpha = 1.f;
    }

    col[3] = alpha;

    if (m_statbar_or == L_STATBAR_HEADING_SPINNER) {
        if (cge) {
            vec3_t vViewAngles;
            float  fAngAlpha;
            cge->CG_EyeAngles(&vViewAngles);

            frac = (AngleSubtract(vViewAngles[1], frac * 360.f + 180.f) + 180.f) / 360.f;
            if (frac < 0.f || frac > 1.f) {
                frac = 0.f;
            }
        }
    }

    fTargAng = DEG2RAD(frac * (m_angles[1] - m_angles[0]) + m_angles[0]);
    fSinVal  = sin(fTargAng);
    fCosVal  = cos(fTargAng);

    vSideDir[0]    = fCosVal;
    vSideDir[1]    = fSinVal;
    vCompassDir[0] = fSinVal;
    vCompassDir[1] = -fCosVal;
    vCenter[0]     = m_frame.size.width * 0.5f;
    vCenter[1]     = m_frame.size.height * 0.5f;

    vVerts[0][0] = (fSinVal * vCenter[0] + vCenter[0]) - (fCosVal * vCenter[1]);
    vVerts[0][1] = (vCompassDir[1] * vCenter[1] + vCenter[1]) - (vSideDir[1] * vCenter[0]);
    vVerts[1][0] = (fSinVal * vCenter[0] + vCenter[0]) + (fCosVal * vCenter[1]);
    vVerts[1][1] = (vSideDir[1] * vCenter[0]) + (vCompassDir[1] * vCenter[1] + vCenter[1]);
    vVerts[2][0] = vCenter[0] - fSinVal * vCenter[0] - fCosVal * vCenter[1];
    vVerts[2][1] = vCenter[1] - vCompassDir[1] * vCenter[1] - vSideDir[1] * vCenter[0];

    vTexCoords[0][0] = 0.f;
    vTexCoords[0][1] = 0.f;
    vTexCoords[1][0] = 1.f;
    vTexCoords[1][1] = 0.f;
    vTexCoords[2][0] = 0.f;
    vTexCoords[2][1] = 1.f;

    if (alpha != 0.f && m_statbar_material_flash) {
        re.SetColor(m_foreground_color);
    }

    m_statbar_material->ReregisterMaterial();
    re.DrawTrianglePic(vVerts, vTexCoords, m_statbar_material->GetMaterial());

    if (alpha != 0.f && m_statbar_material_flash) {
        m_statbar_material_flash->ReregisterMaterial();
        re.DrawTrianglePic(vVerts, vTexCoords, m_statbar_material_flash->GetMaterial());
    }

    vVerts[0][0] = vVerts[2][0];
    vVerts[0][1] = vVerts[2][1];
    vVerts[2][0] = vCenter[1] * fCosVal + vCenter[0] - vCompassDir[0] * vCenter[0];
    vVerts[2][1] = vCenter[0] * vSideDir[1] + vCenter[1] - vCompassDir[1] * vCenter[1];

    vTexCoords[0][0] = vTexCoords[2][0];
    vTexCoords[0][1] = vTexCoords[2][1];
    vTexCoords[2][0] = 1.f;
    vTexCoords[2][1] = 1.f;

    if (alpha != 0.f && m_statbar_material_flash) {
        re.SetColor(m_foreground_color);
    }

    m_statbar_material->ReregisterMaterial();
    re.DrawTrianglePic(vVerts, vTexCoords, m_statbar_material->GetMaterial());

    if (alpha != 0.f && m_statbar_material_flash) {
        m_statbar_material_flash->ReregisterMaterial();
        re.DrawTrianglePic(vVerts, vTexCoords, m_statbar_material_flash->GetMaterial());
    }
}

void UIFakkLabel::StatCircleTexCoord(float fAng, vec3_t vTexCoord)
{
    int   iSector;
    float fSinVal, fCosVal;

    iSector = (AngleMod(fAng) + 45.f) / 90;
    fSinVal = sin(DEG2RAD(fAng));
    fCosVal = cos(DEG2RAD(fAng));

    if (iSector >= 0) {
        if (iSector == 0) {
            vTexCoord[0] = 0.f;
            vTexCoord[1] = 0.5f / fCosVal * fSinVal + 0.5f;
        } else if (iSector == 1) {
            vTexCoord[0] = 1.f;
            vTexCoord[1] = -0.5f / fSinVal * fCosVal + 0.5f;
        } else if (iSector == 2) {
            vTexCoord[0] = fSinVal * (-0.5f * fCosVal) + 0.5f;
            vTexCoord[1] = 1.f;
        } else if (iSector == 4) {
            vTexCoord[0] = 0.5f / fCosVal * fSinVal + 0.5f;
            vTexCoord[1] = 0.f;
        } else {
            vTexCoord[0] = 0.f;
            vTexCoord[1] = 0.5f / fSinVal * fCosVal + 0.5f;
        }
    } else {
        vTexCoord[0] = 0.f;
        vTexCoord[1] = 0.5f / fSinVal * fCosVal + 0.5f;
    }
}

void UIFakkLabel::Draw(void)
{
    if (m_stat_alpha != -1) {
        float frac;
        
        frac = cl.snap.ps.stats[m_stat_alpha] / 100.0;
        frac = Q_clamp_float(frac, 0, 1);
  
        m_alpha = frac;
    }

    if (m_stat == -1 && m_itemindex == -1 && m_inventoryrendermodelindex == -1) {
        if (m_stat_configstring != -1) {
            m_font->setColor(m_foreground_color);
            m_font->PrintJustified(
                getClientFrame(),
                m_iFontAlignmentHorizontal,
                m_iFontAlignmentVertical,
                Sys_LV_CL_ConvertString(va("%s", CL_ConfigString(cl.snap.ps.stats[m_stat_configstring]))),
                m_bVirtual ? m_vVirtualScale : NULL
            );

            return;
        }

        if (!m_rendermodel && m_statbar_or == L_STATBAR_NONE && !m_sDrawModelName.length()) {
            UILabel::Draw();
            return;
        }
    }

    if (m_stat_configstring != -1) {
        m_font->setColor(m_foreground_color);
        m_font->PrintJustified(
            getClientFrame(),
            m_iFontAlignmentHorizontal,
            m_iFontAlignmentVertical,
            Sys_LV_CL_ConvertString(va("%s", CL_ConfigString(cl.snap.ps.stats[m_stat_configstring]))),
            m_bVirtual ? m_vVirtualScale : NULL
        );

        return;
    }

    if (m_stat == -1) {
        float     scale;
        qhandle_t handle;
        vec3_t    origin;
        vec3_t    mins;
        vec3_t    maxs;
        vec3_t    angles;
        vec3_t    offset;
        vec3_t    rotateoffset;
        vec4_t    color;
        str       sAnimName;
        float     height;

        if (m_itemindex != -1) {
            if (m_itemindex < 0) {
                return;
            }

            m_font->setColor(m_foreground_color);
            m_font->PrintJustified(
                getClientFrame(),
                m_iFontAlignmentHorizontal,
                m_iFontAlignmentVertical,
                Sys_LV_CL_ConvertString(va("%s", CL_ConfigString(CS_WEAPONS + cl.snap.ps.activeItems[m_itemindex]))),
                m_bVirtual ? m_vVirtualScale : NULL
            );

            return;
        }

        if (m_inventoryrendermodelindex == -1 && !m_sDrawModelName.length() || !m_rendermodel) {
            float frac = 0.0;

            if (m_statbar_or == L_STATBAR_NONE) {
                return;
            }

            if (m_cvarname.length()) {
                frac = Cvar_VariableValue(m_cvarname) / (m_statbar_max - m_statbar_min);
            }

            if (frac > 1.0) {
                frac = 1.0;
            }
            if (frac < 0.0) {
                frac = 0.0;
            }

            DrawStatbar(frac);
            return;
        }

        VectorSet4(color, 255, 255, 255, 255);

        if (m_rendermodel) {
            if (!m_cvarname.length()) {
                return;
            }

            handle = re.RegisterModel(Cvar_VariableString(m_cvarname));
            if (!handle) {
                return;
            }

            VectorCopy(m_angles, angles);
            VectorCopy(m_rotateoffset, rotateoffset);
            VectorCopy(m_offset, offset);
            scale     = m_scale;
            sAnimName = m_anim;
        } else if (m_sDrawModelName.length()) {
            if (m_lastitemindex != cl.snap.ps.activeItems[m_inventoryrendermodelindex]) {
                if (!m_lastitem) {
                    m_lastitem = CL_GetInvItemByName(&client_inv, m_sDrawModelName);
                }
            }

        _shitlabel01:
            if (!m_lastitem) {
                return;
            }

            VectorCopy(m_lastitem->hudprops.rotateoffset, rotateoffset);
            VectorCopy(m_lastitem->hudprops.offset, offset);

            handle = re.RegisterModel(m_lastitem->hudprops.model);

            VectorCopy(m_lastitem->hudprops.angledeltas, angles);

            if (m_lastitem->hudprops.move == INV_MOVE_BOB) {
                float frac = uid.time / 600.0;
                angles[0] += sin(frac) * m_lastitem->hudprops.angledeltas[0];
                angles[1] += sin(frac) * m_lastitem->hudprops.angledeltas[1];
                angles[2] += sin(frac) * m_lastitem->hudprops.angledeltas[2];
            } else if (m_lastitem->hudprops.model == INV_MOVE_SPIN) {
                float frac = uid.time / 600.0;
                angles[0] += frac * m_lastitem->hudprops.angledeltas[0];
                angles[1] += frac * m_lastitem->hudprops.angledeltas[1];
                angles[2] += frac * m_lastitem->hudprops.angledeltas[2];
            }

            if (m_rendermodel) {
                VectorSet(mins, -16, -16, 0);
                VectorSet(maxs, 16, 16, 96);
            } else {
                re.ModelBounds(handle, mins, maxs);
            }

            origin[1] = (mins[1] + maxs[1]) * 0.5;
            origin[2] = (mins[1] + maxs[1]) * -0.5;

            height = maxs[2] - mins[2];

            if (height < maxs[1] - mins[1]) {
                height = maxs[1] - mins[1];
            }

            if (height <= maxs[0] - mins[0]) {
                height = maxs[0] - mins[0];
            }

            origin[0] = height * scale * 0.5 / 0.268f;

            CL_Draw3DModel(
                m_screenframe.pos.x,
                m_screenframe.pos.y,
                m_screenframe.size.width,
                m_screenframe.size.height,
                handle,
                origin,
                rotateoffset,
                offset,
                angles,
                color,
                sAnimName
            );
            return;
        }

        if (m_lastitemindex == cl.snap.ps.activeItems[m_inventoryrendermodelindex]) {
            goto _shitlabel01;
        }

        if (m_lastitem
            == CL_GetInvItemByName(
                &client_inv, CL_ConfigString(CS_WEAPONS + cl.snap.ps.activeItems[m_inventoryrendermodelindex])
            )) {
            m_lastitemindex = cl.snap.ps.activeItems[m_inventoryrendermodelindex];
            goto _shitlabel01;
        }

        m_lastitem = CL_GetInvItemByName(
            &client_inv, CL_ConfigString(CS_WEAPONS + cl.snap.ps.activeItems[m_inventoryrendermodelindex])
        );
        goto _shitlabel01;
    }

    int delta = cl.snap.ps.stats[m_stat];

    if (m_statbar_or) {
        if (m_statbar_or == L_STATBAR_VERTICAL_STAGGER_EVEN || m_statbar_or == L_STATBAR_VERTICAL_STAGGER_ODD) {
            int stat;

            if (m_maxstat < 0) {
                stat = m_statbar_max;
            } else {
                stat = cl.snap.ps.stats[m_maxstat];
            }

            delta = stat - cl.snap.ps.stats[m_stat];

            if (delta & 1) {
                if (m_statbar_or == L_STATBAR_VERTICAL_STAGGER_EVEN) {
                    delta--;
                } else {
                    delta++;
                }
            }

            delta = stat - delta;

            if (m_maxstat >= 0 || m_statbar_min <= delta) {
                if (delta < 0) {
                    delta = 0;
                }
            } else {
                delta = m_statbar_min;
            }
        }

        float frac;

        if (m_maxstat >= 0) {
            frac = (float)delta / (float)cl.snap.ps.stats[m_maxstat];
        } else {
            frac = (float)delta / (float)(m_statbar_max - m_statbar_min);
        }

        if (frac > 1.0) {
            frac = 1.0;
        } else if (frac < 0.0) {
            frac = 0.0;
        }

        DrawStatbar(frac);

        return;
    }

    m_font->setColor(m_foreground_color);
    m_font->PrintJustified(
        getClientFrame(),
        m_iFontAlignmentHorizontal,
        m_iFontAlignmentVertical,
        va("%d", delta),
        m_bVirtual ? m_vVirtualScale : NULL
    );
}
