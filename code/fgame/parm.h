/*
===========================================================================
Copyright (C) 2008 the OpenMoHAA team

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

// parm.h: Event parameters

#pragma once

#include "listener.h"

class Parm : public Listener
{
public:
    // General trigger variables
    SafePtr<Listener> other;
    SafePtr<Listener> owner;

    // Failure variables
    qboolean movedone;
    qboolean movefail;
    qboolean motionfail;
    qboolean upperfail;
    qboolean sayfail;

public:
    CLASS_PROTOTYPE(Parm);

    void GetPreviousThread(Event *ev);
    void GetOther(Event *ev);
    void GetOwner(Event *ev);
    void GetPath(Event *ev);
    void GetMovedone(Event *ev);
    void GetMovefail(Event *ev);
    void GetMotionFail(Event *ev);
    void GetUpperFail(Event *ev);
    void GetSayFail(Event *ev);

    void Archive(Archiver& arc) override;
};

extern Parm parm;
