/*
===========================================================================
Copyright (C) 2018 the OpenMoHAA team

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

#include "vehicle.h"
#include "player.h"

CLASS_DECLARATION(DrivableVehicle, VehicleWheelsX4, "VehicleWheelsX4") {
    {&EV_Damage, &VehicleWheelsX4::EventDamage},
    {&EV_Killed, &VehicleWheelsX4::Killed     },
    {NULL,       NULL                         }
};

VehicleWheelsX4::VehicleWheelsX4()
{
    m_iGear              = 1;
    m_iRPM               = 0;
    gravity              = 1.0;
    m_fDifferentialRatio = 4.8800001f;
    m_fGearEfficiency    = 0.69999999f;
    m_fGearRatio[0]      = -2.9400001f;
    m_fGearRatio[1]      = 2.9400001f;
    m_fGearRatio[2]      = 1.9400001f;
    m_fGearRatio[3]      = 1.0f;
    m_fAccelerator       = 0.0f;
    m_bBackSlipping      = qfalse;
    m_bAutomatic         = qtrue;
    m_bFrontSlipping     = qfalse;
}

void VehicleWheelsX4::UpdateVariables(
    Vector *acceleration, Vector *vpn, Vector *vup, Vector *vright, Vector *t_vpn, Vector *t_vup, Vector *t_vright
)
{
    // FIXME: STUB
}

float VehicleWheelsX4::TorqueLookup(int rpm)
{
    if (rpm <= 2099) {
        return 20.0;
    }
    if (rpm > 3599) {
        return 0.0;
    }
    return (20 * (3600 - rpm)) * 0.001;
}

void VehicleWheelsX4::Think()
{
    vmove_t vm;
    flags |= FL_POSTTHINK;

    prev_velocity = velocity;

    angles -= m_vAnglesOffset;
    origin -= m_vOriginOffset;

    mins = m_vOldMins;
    maxs = m_vOldMaxs;

    SetMoveInfo(&vm);
    VmoveSingle(&vm);
    GetMoveInfo(&vm);
}

void VehicleWheelsX4::Postthink()
{
    // FIXME: STUB
}

void VehicleWheelsX4::Killed(Event *ev)
{
    deadflag = DEAD_DEAD;
}
