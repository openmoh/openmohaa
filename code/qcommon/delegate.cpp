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

#include "delegate.h"

uint64_t DelegateHandle::currentHandle = 0;

DelegateHandle::DelegateHandle()
    : handle(GenerateDelegateID())
{}

bool DelegateHandle::operator==(const DelegateHandle& other) const
{
    return handle == other.handle;
}

bool DelegateHandle::operator!=(const DelegateHandle& other) const
{
    return handle != other.handle;
}

uint64_t DelegateHandle::GenerateDelegateID()
{
    uint64_t handle;

    handle = ++currentHandle;

    if (handle == 0) {
        handle = ++currentHandle;
    }

    return handle;
}
