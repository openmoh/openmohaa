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

// windows.h: Window class
//

#pragma once

#include "entity.h"

#define WINDOW_BROKEN_BLOCK 1

enum {
    WINDOW_GLASS
};

class WindowObject : public Entity
{
private:
    int m_iDebrisType;
    str m_sBrokenModel;

public:
    CLASS_PROTOTYPE(WindowObject);

    WindowObject();

    void Archive(Archiver& arc) override;

    void WindowSetup(Event *ev);
    void WindowDebrisType(Event *ev);
    void WindowDamaged(Event *ev);
    void WindowKilled(Event *ev);
};