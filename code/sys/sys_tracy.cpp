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

// This file is compiled as part of the tracy target and contains C-linkage wrappers
// for advanced Tracy profiler functionality that isn't exposed via the TracyC API.

// We are also still "inside" the Tracy client, so we need to patch up the import/export macros.
#undef TRACY_IMPORTS
#define TRACY_EXPORTS

#include <tracy/Tracy.hpp>
#include <thread>

extern "C" {
    TRACY_API void Sys_TracyShutdown()
    {
        tracy::GetProfiler().RequestShutdown();
        while (!tracy::GetProfiler().HasShutdownFinished()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}
