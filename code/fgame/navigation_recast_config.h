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

/**
 * @brief Configuration
 * 
 */

#pragma once

#include "bg_public.h"

namespace NavigationMapConfiguration
{
    static const float recastCellSize   = 10.25;
    static const float recastCellHeight = 1.0;
    static const float agentHeight      = MAXS_Z;
    static const float agentMaxClimb    = STEPSIZE;

    // normal of { 0.714142799, 0, 0.700000048 }, or an angle of -44.4270058
    static const float agentMaxSlope = 45.5729942f;

    static const float agentRadius          = 1.0;
    static const int   regionMinSize        = 5;
    static const int   regionMergeSize      = 20;
    static const float edgeMaxLen           = 100.0;
    static const float edgeMaxError         = 1.3f;
    static const int   vertsPerPoly         = 6;
    static const float detailSampleDist     = 12.0;
    static const float detailSampleMaxError = 1.3f;
} // namespace NavigationMapConfiguration

// Polyflags
static constexpr unsigned int RECAST_POLYFLAG_WALKABLE = (1 << 0);
static constexpr unsigned int RECAST_POLYFLAG_BUSY     = (1 << 1);
