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

// parm.cpp : Event parameters

#include "glb_local.h"
#include "parm.h"
#include "scriptmaster.h"
#include "scriptthread.h"
#include "scriptvm.h"

Parm parm;

Event EV_Parm_GetPreviousThread
(
    "previousthread",
    EV_DEFAULT,
    NULL,
    NULL,
    "previousthread",
    EV_GETTER
);
Event EV_Parm_GetOther
(
    "other",
    EV_DEFAULT,
    NULL,
    NULL,
    "other",
    EV_GETTER
);
Event EV_Parm_GetOwner
(
    "owner",
    EV_DEFAULT,
    NULL,
    NULL,
    "owner",
    EV_GETTER
);
Event EV_Parm_Movedone
(
    "movedone",
    EV_DEFAULT,
    NULL,
    NULL,
    "movedone",
    EV_GETTER
);
Event EV_Parm_Movefail
(
    "movefail",
    EV_DEFAULT,
    NULL,
    NULL,
    "movefail",
    EV_GETTER
);
Event EV_Parm_Motionfail
(
    "motionfail",
    EV_DEFAULT,
    NULL,
    NULL,
    "motionfail",
    EV_GETTER
);
Event EV_Parm_Upperfail
(
    "upperfail",
    EV_DEFAULT,
    NULL,
    NULL,
    "upperfail",
    EV_GETTER
);
Event EV_Parm_Sayfail
(
    "sayfail",
    EV_DEFAULT,
    NULL,
    NULL,
    "sayfail",
    EV_GETTER
);

void Parm::Archive(Archiver& arc)
{
    Listener::Archive(arc);

    arc.ArchiveSafePointer(&other);
    arc.ArchiveSafePointer(&owner);
}

void Parm::GetMotionFail(Event *ev)
{
    ev->AddInteger(motionfail);
}

void Parm::GetMovedone(Event *ev)
{
    ev->AddInteger(movedone);
}

void Parm::GetMovefail(Event *ev)
{
    ev->AddInteger(movefail);
}

void Parm::GetOther(Event *ev)
{
    ev->AddListener(other);
}

void Parm::GetOwner(Event *ev)
{
    ev->AddListener(owner);
}

void Parm::GetPreviousThread(Event *ev)
{
    ev->AddListener(Director.PreviousThread());
}

void Parm::GetSayFail(Event *ev)
{
    ev->AddInteger(sayfail);
}

void Parm::GetUpperFail(Event *ev)
{
    ev->AddInteger(upperfail);
}

CLASS_DECLARATION(Listener, Parm, NULL) {
    {&EV_Parm_GetPreviousThread, &Parm::GetPreviousThread},
    {&EV_Parm_GetOther,          &Parm::GetOther         },
    {&EV_Parm_GetOwner,          &Parm::GetOwner         },
    {&EV_Parm_Movedone,          &Parm::GetMovedone      },
    {&EV_Parm_Movefail,          &Parm::GetMovefail      },
    {&EV_Parm_Motionfail,        &Parm::GetMotionFail    },
    {&EV_Parm_Upperfail,         &Parm::GetUpperFail     },
    {&EV_Parm_Sayfail,           &Parm::GetSayFail       },
    {NULL,                       NULL                    }
};
