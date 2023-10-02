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

#pragma once

typedef enum {
    L_STATBAR_NONE,
    L_STATBAR_HORIZONTAL,
    L_STATBAR_VERTICAL,
    L_STATBAR_VERTICAL_REVERSE,
    L_STATBAR_VERTICAL_STAGGER_EVEN,
    L_STATBAR_VERTICAL_STAGGER_ODD,
    L_STATBAR_CIRCULAR,
    L_STATBAR_NEEDLE,
    L_STATBAR_ROTATOR,
    L_STATBAR_COMPASS,
    L_STATBAR_SPINNER,
    L_STATBAR_HEADING_SPINNER
} statbar_orientation_t;

class UIFakkLabel : public UILabel
{
protected:
    SafePtr<inventory_item_t> m_lastitem;
    int                       m_lastitemindex;
    int                       m_stat;
    int                       m_stat_alpha;
    int                       m_stat_configstring;
    int                       m_maxstat;
    int                       m_itemindex;
    int                       m_inventoryrendermodelindex;
    str                       m_sDrawModelName;
    qboolean                  m_rendermodel;
    statbar_orientation_t     m_statbar_or;
    float                     m_statbar_min;
    float                     m_statbar_max;
    float                     m_lastfrac;
    float                     m_flashtime;
    vec3_t                    m_offset;
    vec3_t                    m_rotateoffset;
    vec3_t                    m_angles;
    float                     m_scale;
    str                       m_anim;
    UIReggedMaterial         *m_statbar_material;
    UIReggedMaterial         *m_statbar_material_flash;
	UIReggedMaterial         *m_statbar_material_marker;

public:
    CLASS_PROTOTYPE(UIFakkLabel);

    UIFakkLabel();

    void LayoutPlayerStat(Event *ev);
    void LayoutPlayerStatAlpha(Event *ev); // Added in 2.0
    void LayoutPlayerStatConfigstring(Event *ev);
    void LayoutMaxPlayerStat(Event *ev);
    void LayoutItemIndex(Event *ev);
    void InventoryRenderModelIndex(Event *ev);
    void LayoutModelName(Event *ev);
    void ClearInvItemReference(Event *ev);
    void LayoutStatbar(Event *ev);
    void LayoutStatbarShader(Event *ev);
    void LayoutStatbarTileShader(Event *ev);
    void LayoutStatbarShader_Flash(Event *ev);
    void LayoutStatbarShader_Marker(Event *ev); // Added in 2.0
    void LayoutStatbarEndAngles(Event *ev);
    void LayoutStatbarNeedleWidth(Event *ev);
    void LayoutStatbarRotatorSize(Event *ev);
    void LayoutStatbarTileShader_Flash(Event *ev);
    void LayoutRenderModel(Event *ev);
    void LayoutRenderModelOffset(Event *ev);
    void LayoutRenderModelRotateOffset(Event *ev);
    void LayoutRenderModelAngles(Event *ev);
    void LayoutRenderModelScale(Event *ev);
    void LayoutRenderModelAnim(Event *ev);

    void DrawStatbar(float frac);
    void DrawStatCircle(float frac);
    void DrawStatNeedle(float frac);
    void DrawStatRotator(float frac);
    void DrawStatCompass(float frac);
    void DrawStatSpinner(float frac);
    void StatCircleTexCoord(float fAng, vec3_t vTexCoord);

    void Draw(void) override;
};
