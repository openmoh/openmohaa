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

// navigation -- Modern navigation system using Recast and Detour

#include "g_local.h"
#include "../qcommon/container.h"
#include "../qcommon/vector.h"

struct navIndice_t {
    int indice;

    navIndice_t() noexcept {}
    navIndice_t(int value) noexcept
        : indice(value)
    {
        assert(value >= 0);
    }

    operator int() const noexcept { return indice; };
    operator int* () noexcept { return &indice; };
    operator const int*() const noexcept { return &indice; };
};

struct navMap_t {
    const char* mapname;
    Container<navIndice_t>    indices;
    Container<Vector> vertices;
    Vector bounds[2];
};

void G_Navigation_ProcessBSPForNavigation(const char* mapname, navMap_t& outNavigationMap);
