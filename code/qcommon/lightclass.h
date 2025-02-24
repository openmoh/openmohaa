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

// lightclass.h: Implementation of a light class

#pragma once

#include "q_shared.h"

class LightClass
{
public:
    void *operator new(size_t s);
    void  operator delete(void *ptr);

    void *operator new(size_t size, void *placement);
    void  operator delete(void *ptr, void *placement);

    template<typename T>
    void *operator new(size_t size, T& placement)
    {
        return operator new(size, (void *)&placement);
    }

    template<typename T>
    void operator delete(void *ptr, T& placement)
    {
        return operator delete(ptr, (void *)&placement);
    }
};

void DisplayMemoryUsage();
