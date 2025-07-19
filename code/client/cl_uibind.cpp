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

#include "cl_ui.h"

bind_t::bind_t()
{
    int i;

    commandwidth       = 100;
    commandheight      = 20;
    primarykeywidth    = 100;
    primarykeyheight   = 20;
    alternatekeywidth  = 100;
    alternatekeyheight = 20;
    fillwidth          = 16;
    fillmaterial       = NULL;

    for (i = 0; i < ARRAY_LEN(headermats); i++) {
        headermats[i] = NULL;
    }
}

bind_t::~bind_t()
{
    Clear();
}

void bind_t::Clear(void)
{
    for (int i = binds.NumObjects(); i > 0; i--) {
        bind_item_t *bi;

        bi = binds.ObjectAt(i);
        binds.RemoveObjectAt(i);

        if (bi) {
            delete bi;
        }
    }
}

Event EV_Bind_Header("header", EV_DEFAULT, "iss", "column_num name material", "Set the header of each column");
Event EV_Bind_Width("bind_width", EV_DEFAULT, "i", "width", "Width of the bind menu");
Event EV_Bind_Height("bind_height", EV_DEFAULT, "i", "height", "Height of the bind menu");
Event EV_Bind_FillWidth("bind_fillwidth", EV_DEFAULT, "iS", "width material", "Width of the filling between widgets");
Event EV_Bind_CommandWidth("bind_commandwidth", EV_DEFAULT, "i", "width", "Width of the command item");
Event EV_Bind_CommandHeight("bind_commandheight", EV_DEFAULT, "i", "height", "Height of the command item");
Event EV_Bind_PrimaryKeyWidth("bind_primarykeywidth", EV_DEFAULT, "i", "width", "Width of the primary key item");
Event EV_Bind_PrimaryKeyHeight("bind_primarykeyheight", EV_DEFAULT, "i", "height", "Height of the primary key item");
Event EV_Bind_AlternateKeyWidth("bind_alternatekeywidth", EV_DEFAULT, "i", "width", "Width of the alternate key item");
Event EV_Bind_AlternateKeyHeight(
    "bind_alternatekeyheight", EV_DEFAULT, "i", "height", "Height of the alternate key item"
);
Event EV_Bind_NewItem("binditem", EV_DEFAULT, "ss", "name command", "Create a new bind item in the interface");
Event
    EV_Bind_Align("align", EV_DEFAULT, "sSSSSS", "arg1 arg2 arg3 arg4 arg5 arg6", "Set the alignment on the bind menu");
Event EV_Bind_TitleForegroundColor(
    "titleforegroundcolor", EV_DEFAULT, "ffff", "r g b a", "Set the color for the title foreground"
);
Event EV_Bind_TitleBackgroundColor(
    "titlebackgroundcolor", EV_DEFAULT, "ffff", "r g b a", "Set the color for the title background"
);
Event EV_Bind_InactiveForegroundColor(
    "inactiveforegroundcolor", EV_DEFAULT, "ffff", "r g b a", "Set the color for the inactive button foreground"
);
Event EV_Bind_InactiveBackgroundColor(
    "inactivebackgroundcolor", EV_DEFAULT, "ffff", "r g b a", "Set the color for the inactive button background"
);
Event EV_Bind_ActiveForegroundColor(
    "activeforegroundcolor", EV_DEFAULT, "ffff", "r g b a", "Set the color for the active button foreground"
);
Event EV_Bind_ActiveBackgroundColor(
    "activebackgroundcolor", EV_DEFAULT, "ffff", "r g b a", "Set the color for the active button background"
);
Event EV_Bind_ActiveBorderColor(
    "activebordercolor", EV_DEFAULT, "ffff", "r g b a", "Set the color for the active button border"
);
Event EV_Bind_HighlightForegroundColor(
    "highlightforegroundcolor", EV_DEFAULT, "ffff", "r g b a", "Set the color for the highlighted button foreground"
);
Event EV_Bind_HighlightBackgroundColor(
    "highlightbackgroundcolor", EV_DEFAULT, "ffff", "r g b a", "Set the color for the highlighted button background"
);
Event EV_Bind_SelectForegroundColor(
    "selectforegroundcolor", EV_DEFAULT, "ffff", "r g b a", "Set the color for the selected button foreground"
);
Event EV_Bind_SelectBackgroundColor(
    "selectbackgroundcolor", EV_DEFAULT, "ffff", "r g b a", "Set the color for the selected button background"
);
Event EV_Bind_ChangeSound(
    "changesound", EV_DEFAULT, "s", "soundname", "Set the name of the sound to play when the highlighted item changes"
);
Event EV_Bind_ActiveSound(
    "activesound", EV_DEFAULT, "s", "soundname", "Set the name of the sound to play when the item is made active"
);
Event EV_Bind_EnterSound(
    "entersound", EV_DEFAULT, "s", "soundname", "Set the name of the sound to play when the key is entered"
);

CLASS_DECLARATION(Listener, bindlistener, NULL) {
    {&EV_Bind_Width,                    &bindlistener::Width                   },
    {&EV_Bind_Height,                   &bindlistener::Height                  },
    {&EV_Bind_FillWidth,                &bindlistener::FillWidth               },
    {&EV_Bind_CommandWidth,             &bindlistener::CommandWidth            },
    {&EV_Bind_CommandHeight,            &bindlistener::CommandHeight           },
    {&EV_Bind_PrimaryKeyWidth,          &bindlistener::PrimaryKeyWidth         },
    {&EV_Bind_PrimaryKeyHeight,         &bindlistener::PrimaryKeyHeight        },
    {&EV_Bind_AlternateKeyWidth,        &bindlistener::AlternateKeyWidth       },
    {&EV_Bind_AlternateKeyHeight,       &bindlistener::AlternateKeyHeight      },
    {&EV_Bind_Header,                   &bindlistener::Header                  },
    {&EV_Bind_NewItem,                  &bindlistener::NewItem                 },
    {&EV_Bind_Align,                    &bindlistener::Align                   },
    {&EV_Bind_TitleForegroundColor,     &bindlistener::TitleForegroundColor    },
    {&EV_Bind_TitleBackgroundColor,     &bindlistener::TitleBackgroundColor    },
    {&EV_Bind_InactiveForegroundColor,  &bindlistener::InactiveForegroundColor },
    {&EV_Bind_InactiveBackgroundColor,  &bindlistener::InactiveBackgroundColor },
    {&EV_Bind_ActiveForegroundColor,    &bindlistener::ActiveForegroundColor   },
    {&EV_Bind_ActiveBackgroundColor,    &bindlistener::ActiveBackgroundColor   },
    {&EV_Bind_ActiveBorderColor,        &bindlistener::ActiveBorderColor       },
    {&EV_Bind_HighlightForegroundColor, &bindlistener::HighlightForegroundColor},
    {&EV_Bind_HighlightBackgroundColor, &bindlistener::HighlightBackgroundColor},
    {&EV_Bind_SelectForegroundColor,    &bindlistener::SelectForegroundColor   },
    {&EV_Bind_SelectBackgroundColor,    &bindlistener::SelectBackgroundColor   },
    {&EV_Bind_ChangeSound,              &bindlistener::ChangeSound             },
    {&EV_Bind_ActiveSound,              &bindlistener::ActiveSound             },
    {&EV_Bind_EnterSound,               &bindlistener::EnterSound              },
    {NULL,                              NULL                                   }
};

bindlistener::bindlistener() {}

bindlistener::bindlistener(bind_t *b)
{
    bind = b;
}

void bindlistener::Header(Event *ev)
{
    int num;

    num = ev->GetInteger(1) - 1;
    if (num > ARRAY_LEN(bind_t::headers)) {
        return;
    }

    bind->headers[num] = ev->GetString(2);
    if (ev->NumArgs() > 2) {
        bind->headermats[num] = uWinMan.RegisterShader(ev->GetString(3));
    }
}

void bindlistener::ChangeSound(Event *ev)
{
    bind->changesound = ev->GetString(1);
}

void bindlistener::ActiveSound(Event *ev)
{
    bind->activesound = ev->GetString(1);
}

void bindlistener::EnterSound(Event *ev)
{
    bind->entersound = ev->GetString(1);
}

void bindlistener::SelectForegroundColor(Event *ev)
{
    bind->selectfgcolor.r = ev->GetFloat(1);
    bind->selectfgcolor.g = ev->GetFloat(2);
    bind->selectfgcolor.b = ev->GetFloat(3);
    bind->selectfgcolor.a = ev->GetFloat(4);
}

void bindlistener::SelectBackgroundColor(Event *ev)
{
    bind->selectbgcolor.r = ev->GetFloat(1);
    bind->selectbgcolor.g = ev->GetFloat(2);
    bind->selectbgcolor.b = ev->GetFloat(3);
    bind->selectbgcolor.a = ev->GetFloat(4);
}

void bindlistener::TitleForegroundColor(Event *ev)
{
    bind->titlefgcolor.r = ev->GetFloat(1);
    bind->titlefgcolor.g = ev->GetFloat(2);
    bind->titlefgcolor.b = ev->GetFloat(3);
    bind->titlefgcolor.a = ev->GetFloat(4);
}

void bindlistener::TitleBackgroundColor(Event *ev)
{
    bind->titlebgcolor.r = ev->GetFloat(1);
    bind->titlebgcolor.g = ev->GetFloat(2);
    bind->titlebgcolor.b = ev->GetFloat(3);
    bind->titlebgcolor.a = ev->GetFloat(4);
}

void bindlistener::ActiveBorderColor(Event *ev)
{
    bind->activebordercolor.r = ev->GetFloat(1);
    bind->activebordercolor.g = ev->GetFloat(2);
    bind->activebordercolor.b = ev->GetFloat(3);
    bind->activebordercolor.a = ev->GetFloat(4);
}

void bindlistener::ActiveForegroundColor(Event *ev)
{
    bind->activefgcolor.r = ev->GetFloat(1);
    bind->activefgcolor.g = ev->GetFloat(2);
    bind->activefgcolor.b = ev->GetFloat(3);
    bind->activefgcolor.a = ev->GetFloat(4);
}

void bindlistener::ActiveBackgroundColor(Event *ev)
{
    bind->activebgcolor.r = ev->GetFloat(1);
    bind->activebgcolor.g = ev->GetFloat(2);
    bind->activebgcolor.b = ev->GetFloat(3);
    bind->activebgcolor.a = ev->GetFloat(4);
}

void bindlistener::InactiveForegroundColor(Event *ev)
{
    bind->inactivefgcolor.r = ev->GetFloat(1);
    bind->inactivefgcolor.g = ev->GetFloat(2);
    bind->inactivefgcolor.b = ev->GetFloat(3);
    bind->inactivefgcolor.a = ev->GetFloat(4);
}

void bindlistener::InactiveBackgroundColor(Event *ev)
{
    bind->inactivebgcolor.r = ev->GetFloat(1);
    bind->inactivebgcolor.g = ev->GetFloat(2);
    bind->inactivebgcolor.b = ev->GetFloat(3);
    bind->inactivebgcolor.a = ev->GetFloat(4);
}

void bindlistener::HighlightForegroundColor(Event *ev)
{
    bind->highlightfgcolor.r = ev->GetFloat(1);
    bind->highlightfgcolor.g = ev->GetFloat(2);
    bind->highlightfgcolor.b = ev->GetFloat(3);
    bind->highlightfgcolor.a = ev->GetFloat(4);
}

void bindlistener::HighlightBackgroundColor(Event *ev)
{
    bind->highlightbgcolor.r = ev->GetFloat(1);
    bind->highlightbgcolor.g = ev->GetFloat(2);
    bind->highlightbgcolor.b = ev->GetFloat(3);
    bind->highlightbgcolor.a = ev->GetFloat(4);
}

void bindlistener::Align(Event *ev)
{
    int i;

    for (i = 1; i <= ev->NumArgs(); i++) {
        str align = ev->GetString(i);

        if (!str::icmp(align, "left")) {
            bind->align |= WA_LEFT;
        } else if (!str::icmp(align, "right")) {
            bind->align |= WA_RIGHT;
        } else if (!str::icmp(align, "top")) {
            bind->align |= WA_TOP;
        } else if (!str::icmp(align, "bottom")) {
            bind->align |= WA_BOTTOM;
        } else if (!str::icmp(align, "centerx")) {
            bind->align |= WA_CENTERX;
        } else if (!str::icmp(align, "centery")) {
            bind->align |= WA_CENTERY;
        }
    }
}

void bindlistener::NewItem(Event *ev)
{
    bind_item_t      *bt;
    UIReggedMaterial *mat = NULL;

    if (ev->NumArgs() > 2) {
        mat = uWinMan.RegisterShader(ev->GetString(3));
    }

    str n = ev->GetString(1);
    str c = ev->GetString(2);

    bt               = new bind_item_t;
    bt->name         = n;
    bt->command      = c;
    bt->nameMaterial = mat;

    bind->binds.AddObject(bt);
}

void bindlistener::Width(Event *ev)
{
    bind->width = ev->GetInteger(1);
}

void bindlistener::Height(Event *ev)
{
    bind->height = ev->GetInteger(1);
}

void bindlistener::FillWidth(Event *ev)
{
    bind->fillwidth = ev->GetInteger(1);

    if (ev->NumArgs() > 1) {
        bind->fillmaterial = uWinMan.RegisterShader(ev->GetString(2));
    }
}

void bindlistener::CommandWidth(Event *ev)
{
    bind->commandwidth = ev->GetInteger(1);
}

void bindlistener::CommandHeight(Event *ev)
{
    bind->commandheight = ev->GetInteger(1);
}

void bindlistener::PrimaryKeyWidth(Event *ev)
{
    bind->primarykeywidth = ev->GetInteger(1);
}

void bindlistener::PrimaryKeyHeight(Event *ev)
{
    bind->primarykeyheight = ev->GetInteger(1);
}

void bindlistener::AlternateKeyWidth(Event *ev)
{
    bind->alternatekeywidth = ev->GetInteger(1);
}

void bindlistener::AlternateKeyHeight(Event *ev)
{
    bind->alternatekeyheight = ev->GetInteger(1);
}

bool bindlistener::Load(Script& script)
{
    str token;
    str errortext;

    while (script.TokenAvailable(true)) {
        token = script.GetToken(true);

        if (!token.length() || !ValidEvent(token)) {
            throw "invalid token";
        }

        Event *event = new Event(token);

        while (script.TokenAvailable(false)) {
            event->AddToken(script.GetToken(false));
        }

        ProcessEvent(event);
    }

    return true;
}

bool CL_LoadBind(const char *filename, bind_t *bind)
{
    Script script;

    bind->Clear();

    bindlistener listener(bind);
    script.LoadFile(filename);
    // Load the bind file
    return listener.Load(script);
}
