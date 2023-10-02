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

#include "ui_local.h"
#include "../qcommon/localization.h"

Event EV_UILabel_LinkString
(
    "linkstring",
    EV_DEFAULT,
    "is",
    "value string",
    "Creates a link from the specified value to a string. "
    "Use this if you want the label to display a string different from the value of the cvar"

);
Event EV_UILabel_LinkCvarToShader
(
    "linkcvartoshader",
    EV_DEFAULT,
    NULL,
    NULL,
    "Links the label's cvar to its shader"
);
// Added in 2.0
//====
Event EV_UILabel_OneShotSound
(
    "oneshotsound",
    EV_DEFAULT,
    "sf",
    "soundName soundTime",
    "Set the sound to play"
);
Event EV_UILabel_LoopingSound
(
    "loopingsound",
    EV_DEFAULT,
    "sffff",
    "soundName start_volume hold_start_time ending_volume ramp_time",
    "Set the sound to play"

);
Event EV_UILabel_OutlinedText
(
    "outlinedtext",
    EV_DEFAULT,
    "i",
    "outlined",
    "Set/clear outline text"
);
//====

static const char *localizableVars[] = {
    "cl_langamerefreshstatus",
    "ui_timemessage",
    "cg_obj_axistext1",
    "cg_obj_axistext2",
    "cg_obj_axistext3",
    "cg_obj_axistext4",
    "cg_obj_axistext5"
    "cg_obj_alliedtext1",
    "cg_obj_alliedtext2",
    "cg_obj_alliedtext3",
    "cg_obj_alliedtext4",
    "cg_obj_alliedtext5",
    "g_gametypestring",
    "dm_serverstatus",
    "com_errormessage",
    NULL};

CLASS_DECLARATION(UIWidget, UILabel, NULL) {
    {&W_MouseEntered,              &UILabel::MouseEntered         },
    {&W_MouseExited,               &UILabel::MouseExited          },
    {&EV_UILabel_LinkString,       &UILabel::LinkString           },
    {&EV_UILabel_LinkCvarToShader, &UILabel::SetLinkCvarToShader  },
    {&EV_UILabel_OneShotSound,     &UILabel::OneShotSound         },
    {&EV_UILabel_LoopingSound,     &UILabel::LoopingSound         },
    {&EV_Layout_Shader,            &UILabel::LabelLayoutShader    },
    {&EV_Layout_TileShader,        &UILabel::LabelLayoutTileShader},
    {&EV_UILabel_OutlinedText,     &UILabel::EventSetOutlinedText },
    {NULL,                         NULL                           }
};

UILabel::UILabel()
{
    AllowActivate(false);
    m_bLinkCvarToShader = false;
    m_bOneShotTriggered = false;
    m_iNextOneShotTime  = 0;
    m_iNextStateTime    = 0;
    m_iLastDrawTime     = 0;
    m_bOutlinedText     = false;
}

void UILabel::MouseEntered(Event *ev)
{
    SetHovermaterialActive(true);
}

void UILabel::MouseExited(Event *ev)
{
    SetHovermaterialActive(false);
    SetPressedmaterialActive(false);
}

void UILabel::LinkString(Event *ev)
{
    linkstring *ls;
    str         value;
    str         string;

    value  = ev->GetString(1);
    string = ev->GetString(2);

    ls = new linkstring(value, string);

    m_linkstrings.AddObject(ls);
}

void UILabel::SetLabel(str lab)
{
    label = lab;
}

int UILabel::FindLinkString(str val)
{
    int i;

    for (i = 1; i <= m_linkstrings.NumObjects(); i++) {
        if (val == m_linkstrings.ObjectAt(i)->value) {
            return i;
        }
    }

    return 0;
}

void UILabel::LabelLayoutShader(Event *ev)
{
    m_sCurrentShaderName = ev->GetString(1);
    setMaterial(uWinMan.RegisterShader(m_sCurrentShaderName));

    m_flags &= ~WF_TILESHADER;
}

void UILabel::LabelLayoutTileShader(Event *ev)
{
    m_sCurrentShaderName = ev->GetString(1);
    setMaterial(uWinMan.RegisterShader(m_sCurrentShaderName));

    m_flags |= WF_TILESHADER;
}

void UILabel::SetLinkCvarToShader(Event *ev)
{
    m_bLinkCvarToShader = true;
}

void UILabel::OneShotSound(Event *ev)
{
    m_sOneShotSoundName = ev->GetString(1);
    m_fOneShotSoundTime = ev->GetFloat(2);
}

void UILabel::LoopingSound(Event *ev)
{
    m_sLoopingSoundName = ev->GetString(1);

    switch (ev->NumArgs()) {
    case 1:
        m_fLoopingSoundHoldStartTime = 0.0;
        m_fLoopingSoundStartVolume   = 1.0;
        m_fLoopingSoundEndVolume     = 1.0;
        m_fLoopingSoundRampTime      = 0.0;
        break;
    case 2:
    case 3:
        m_fLoopingSoundStartVolume   = ev->GetFloat(2);
        m_fLoopingSoundHoldStartTime = 0.0;
        m_fLoopingSoundEndVolume     = ev->GetFloat(2);
        m_fLoopingSoundRampTime      = 0.0;
        break;
    case 4:
        m_fLoopingSoundStartVolume   = ev->GetFloat(2);
        m_fLoopingSoundHoldStartTime = ev->GetFloat(3);
        m_fLoopingSoundEndVolume     = ev->GetFloat(4);
        m_fLoopingSoundRampTime      = 4.0;
        break;
    case 5:
        m_fLoopingSoundStartVolume   = ev->GetFloat(2);
        m_fLoopingSoundHoldStartTime = ev->GetFloat(3);
        m_fLoopingSoundEndVolume     = ev->GetFloat(4);
        m_fLoopingSoundRampTime      = ev->GetFloat(5);
        break;
    default:
        break;
    }
}

void UILabel::EventSetOutlinedText(Event *ev)
{
    if (ev->NumArgs() > 0) {
        m_bOutlinedText = ev->GetInteger(1) ? true : false;
    } else {
        m_bOutlinedText = true;
    }
}

void UILabel::Draw(void)
{
    const char *string;
    str         val;

    ProcessSounds();

    m_iLastDrawTime = uii.Sys_Milliseconds();

    if (m_cvarname.length()) {
        string = uii.Cvar_GetString(m_cvarname, "");

        //
        // localize the cvar value when possible
        //
        for (int i = 0; localizableVars[i]; i++) {
            if (!strcmp(localizableVars[i], string)) {
                string = Sys_LV_CL_ConvertString(localizableVars[i]);
            }
        }

        if (m_linkstrings.NumObjects()) {
            cvar_t *cvar = UI_FindCvar(m_cvarname);
            if (cvar) {
                int ret;

                if (cvar->latchedString) {
                    val = cvar->latchedString;
                } else {
                    val = cvar->string;
                }

                ret = FindLinkString(val);
                if (ret) {
                    string = m_linkstrings.ObjectAt(ret)->value;
                }
            }
        }

        if (m_bLinkCvarToShader) {
            if (str::icmp(m_sCurrentShaderName, string)) {
                m_sCurrentShaderName = string;

                setMaterial(uWinMan.RegisterShader(m_sCurrentShaderName));
                m_material->ReregisterMaterial();

                if (!m_material->GetMaterial()) {
                    setMaterial(NULL);
                }
            }

            if (label.length()) {
                string = label;
            } else if (m_title.length()) {
                string = m_title;
            } else {
                string = "";
            }
        }
    } else {
        if (label.length()) {
            string = label;
        } else if (m_title.length()) {
            string = m_title;
        } else {
            string = "";
        }
    }

    if (*string) {
        float nr, ng, nb;
        float alpha;

        nr = m_foreground_color.r;
        ng = m_foreground_color.g;
        nb = m_foreground_color.b;

        m_font->setColor(UColor(nr, ng, nb, m_foreground_color.a * m_local_alpha));

        if (m_bOutlinedText) {
            // print the text
            m_font->PrintOutlinedJustified(
                getClientFrame(),
                m_iFontAlignmentHorizontal,
                m_iFontAlignmentVertical,
                string,
                UBlack,
                m_bVirtual ? m_vVirtualScale : NULL
            );
        } else {
            // print the text
            m_font->PrintJustified(
                getClientFrame(),
                m_iFontAlignmentHorizontal,
                m_iFontAlignmentVertical,
                string,
                m_bVirtual ? m_vVirtualScale : NULL
            );
        }
    }
}

void UILabel::FrameInitialized()
{
    if (m_sOneShotSoundName.length() || m_sLoopingSoundName.length()) {
        uii.Cmd_Stuff("tmstop\n");
        m_bOneShotTriggered = false;
        m_iNextOneShotTime  = uii.Sys_Milliseconds() + 2000;
        m_iNextStateTime    = 0;
    }

    m_iLastDrawTime = 0;

    UIWidget::FrameInitialized();
}

void UILabel::ProcessSounds()
{
    char  buffer[64];
    float frac;

    if (paused->integer) {
        if (m_iNextOneShotTime) {
            m_iNextOneShotTime += uii.Sys_Milliseconds() - m_iLastDrawTime;
        }

        if (m_iNextStateTime) {
            m_iNextStateTime += uii.Sys_Milliseconds() - m_iLastDrawTime;
        }
    }

    if (m_iNextOneShotTime && uii.Sys_Milliseconds() > m_iNextOneShotTime) {
        if (m_sOneShotSoundName.length() && !m_bOneShotTriggered) {
            //
            // use the looping sound if the oneshot sound was triggered is non-existent
            //
            uii.Snd_StopAllSound();
            uii.Snd_FadeSound(0);
            uii.Snd_PlaySoundDialog(m_sOneShotSoundName);

            m_bOneShotTriggered = true;
            m_iNextOneShotTime  = Sys_Milliseconds() + m_fOneShotSoundTime * 1000.0f;
        } else if (m_sLoopingSoundName.length()) {
            uii.Cmd_Stuff("tmstartloop " + m_sLoopingSoundName);

            m_iNextOneShotTime = 0;
            m_iSoundState      = 0;
            m_iNextStateTime   = uii.Sys_Milliseconds();
        } else {
            m_iNextOneShotTime = 0;
        }
    }

    if (m_iNextStateTime && uii.Sys_Milliseconds() >= m_iNextStateTime) {
        switch (m_iSoundState) {
        case 0:
            Com_sprintf(buffer, sizeof(buffer), "tmvolume %4.2f\n", m_fLoopingSoundStartVolume);
            uii.Cmd_Stuff(buffer);
            m_iNextStateTime = uii.Sys_Milliseconds() + m_fLoopingSoundHoldStartTime * 1000;
            m_iSoundState    = 1;
            break;
        case 1:
            m_iLastEnterTime = m_iNextStateTime;
            m_iSoundState    = 2;
            m_iNextStateTime = uii.Sys_Milliseconds() + 10;
            break;
        case 2:
            if (m_fLoopingSoundRampTime > 0) {
                frac = (uii.Sys_Milliseconds() - m_iLastEnterTime) / (m_fLoopingSoundRampTime * 1000);
            } else {
                frac = 1.0;
            }

            Com_sprintf(
                buffer,
                sizeof(buffer),
                "tmvolume %4.2f\n",
                frac * (m_fLoopingSoundEndVolume - m_fLoopingSoundStartVolume) + m_fLoopingSoundStartVolume
            );
            uii.Cmd_Stuff(buffer);

            m_iNextStateTime = uii.Sys_Milliseconds() + 10;
            if (frac > 1.0) {
                m_iSoundState = 3;
            }
            break;
        case 3:
            m_iNextStateTime = 0;
            break;
        default:
            break;
        }
    }
}
