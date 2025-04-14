/*
===========================================================================
Copyright (C) 2025 the OpenMoHAA team

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
#include "cl_instantAction.h"
#include "cl_uiserverlist.h"

Event EV_UIInstantAction_AcceptServer
(
    "acceptserver",
    EV_DEFAULT,
    NULL,
    NULL,
    "Connect to the current server"
);

Event EV_UIInstantAction_RejectServer
(
    "rejectserver",
    EV_DEFAULT,
    NULL,
    NULL,
    "Reject the current server"
);

Event EV_UIInstantAction_Cancel
(
    "ia_cancel",
    EV_DEFAULT,
    NULL,
    NULL,
    "cancel the server update"
);

Event EV_UIInstantAction_Refresh
(
    "ia_refresh",
    EV_DEFAULT,
    NULL,
    NULL,
    "Refresh the server list"
);

CLASS_DECLARATION(UIWidget, UIInstantAction, NULL) {
    {&EV_UIInstantAction_AcceptServer, &UIInstantAction::Connect      },
    {&EV_UIInstantAction_RejectServer, &UIInstantAction::Reject       },
    {&EV_UIInstantAction_Cancel,       &UIInstantAction::CancelRefresh},
    {&EV_UIInstantAction_Refresh,      NULL                           },
    {NULL,                             NULL                           }
};

UIInstantAction::UIInstantAction()
{
    // FIXME: unimplemented
}

UIInstantAction::~UIInstantAction()
{
    // FIXME: unimplemented
}

void UIInstantAction::CleanUp()
{
    // FIXME: unimplemented
}

void UIInstantAction::Init()
{
    // FIXME: unimplemented
}

int UIInstantAction::GetServerIndex(int maxPing, int gameType)
{
    // FIXME: unimplemented
    return 0;
}

void UIInstantAction::ReadIniFile()
{
    // FIXME: unimplemented
}

void UIInstantAction::FindServer()
{
    // FIXME: unimplemented
}

void UIInstantAction::Connect(Event *ev)
{
    // FIXME: unimplemented
}

void UIInstantAction::Reject(Event *ev)
{
    // FIXME: unimplemented
}

void UIInstantAction::Draw()
{
    // FIXME: unimplemented
}

void UIInstantAction::Update()
{
    // FIXME: unimplemented
}

void UIInstantAction::AddServer(GServer server, const ServerGame_t& serverGame)
{
    // FIXME: unimplemented
}

void UIInstantAction::CancelRefresh(Event *ev)
{
    // FIXME: unimplemented
}

void UIInstantAction::EnableServerInfo(bool enable)
{
    // FIXME: unimplemented
}
