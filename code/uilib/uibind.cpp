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

#include "ui_local.h"
#include "uibindlist.h"
#include "../qcommon/localization.h"

Event EV_UIBindButton_Command
(
    "bindcommand",
    EV_DEFAULT,
    "s",
    "cmd",
    "Command to bind"
);

CLASS_DECLARATION(UIButton, UIBindButton, NULL) {
    {&W_AnyMouseDown,          &UIBindButton::Pressed   },
    {&EV_UIBindButton_Command, &UIBindButton::SetCommand},
    {NULL,                     NULL                     }
};

UIBindButton::UIBindButton()
    : m_getkey(qfalse)
    , m_bindindex(-1)
    , m_alternate(qfalse)
    , m_mat(NULL)
{}

UIBindButton::UIBindButton(str entersound, str activesound)
    : m_getkey(qfalse)
    , m_bindindex(-1)
    , m_alternate(qfalse)
    , m_mat(NULL)
{
    m_entersound  = entersound;
    m_activesound = activesound;

    m_mat      = uWinMan.RegisterShader("textures/bind/emptykey");
    m_material = m_mat;
}

void UIBindButton::SetAlternate(qboolean a)
{
    m_alternate = a;
}

void UIBindButton::SetCommand(Event *ev)
{
    SetCommand(ev->GetString(1));
}

void UIBindButton::SetCommand(str s)
{
    m_bindcommand = s;
    m_bindindex   = uWinMan.AddBinding(m_bindcommand);
}

void UIBindButton::Pressed(void)
{
    m_getkey = qtrue;
    uWinMan.SetBindActive(this);
}

void UIBindButton::Pressed(Event *ev)
{
    int i, j;
    int buttons;

    buttons = ev->GetInteger(3);

    if (m_getkey) {
        for (i = K_MOUSE1, j = 0; i <= K_MOUSE5; i++, j++) {
            if (buttons & (1 << j)) {
                KeyEvent(i, 0);
            }
        }
        return;
    }

    if (!(buttons & 1)) {
        // Left mouse button is not held
        return;
    }

    if (m_parent->isSubclassOf(UIFakkBindList)) {
        UIFakkBindList *fbl = static_cast<UIFakkBindList *>(m_parent);

        if (!fbl->SetActiveRow(this)) {
            return;
        }
    }

    m_getkey = true;
    uWinMan.SetBindActive(this);
    uii.Snd_PlaySound(m_activesound);
}

qboolean UIBindButton::KeyEvent(int key, unsigned int time)
{
    if (!m_getkey) {
        if (m_parent) {
            return m_parent->KeyEvent(key, 0);
        }

        return qfalse;
    }

    if (key != K_ESCAPE && key != K_PAUSE && key != 0) {
        uWinMan.BindKeyToCommand(m_bindcommand, key, m_bindindex, m_alternate);
    }

    m_getkey = false;
    uWinMan.SetBindActive(NULL);
    uii.Snd_PlaySound(m_entersound);

    return qtrue;
}

void UIBindButton::DrawPressed(void)
{
    DrawUnpressed();
}

void UIBindButton::Clear(void)
{
    int key1 = -1, key2 = -1;
    str s;

    s = uWinMan.GetKeyStringForCommand(m_bindcommand, m_bindindex, m_alternate, &key1, &key2);
    if (m_alternate) {
        if (key2 > 0) {
            uWinMan.BindKeyToCommand(str(), key2, m_bindindex, m_alternate);
        }
    } else {
        if (key1 > 0) {
            uWinMan.BindKeyToCommand(str(), key1, m_bindindex, m_alternate);
        }
    }
}

void UIBindButton::DrawUnpressed(void)
{
    str      keyname;
    UIRect2D clientFrame;

    clientFrame = getClientFrame();
    if (uWinMan.BindActive() == this) {
        DrawBoxWithSolidBorder(clientFrame, m_background_color, m_border_color.original, 2, 2, 1.f);
    }

    if (m_bindindex <= 0) {
        return;
    }

    str s = uWinMan.GetKeyStringForCommand(m_bindcommand, m_bindindex, m_alternate, NULL, NULL);

    m_font->setColor(m_background_color);

    if (str::cmp(s, m_last_keyname)) {
        m_mat = uWinMan.RegisterShader("textures/bind/" + s);
        if (m_mat) {
            if (m_mat->GetMaterial()) {
                m_material = m_mat;
            } else {
                m_material = uWinMan.RegisterShader("textures/bind/emptykey");
            }
        } else {
            m_material = uWinMan.RegisterShader("textures/bind/emptykey");
        }

        m_last_keyname = s;
    }

    if (m_mat) {
        if (m_mat->GetMaterial()) {
            return;
        }
    }

    m_font->PrintJustified(
        UIRect2D(clientFrame.pos.x + 1, clientFrame.pos.y + 1, clientFrame.size.width, clientFrame.size.height),
        m_iFontAlignmentHorizontal,
        m_iFontAlignmentVertical,
        Sys_LV_CL_ConvertString(s),
        m_bVirtual ? m_vVirtualScale : NULL
    );
}
