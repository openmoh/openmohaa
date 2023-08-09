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

// object.h: Object (used by common TIKIs)

#pragma once

#include "animate.h"

class Object : public Animate
{
public:
    CLASS_PROTOTYPE(Object);

    void EventRemoveObjectModel(Event *ev);
    void EventHandleSpawn(Event *ev);
};

class InteractObject : public Animate
{
private:
    str m_sHitEffect;
    str m_sKilledEffect;

public:
    CLASS_PROTOTYPE(InteractObject);

    InteractObject();
    void Damaged(Event *ev);
    void Killed(Event *ev);
    void Setup(Event *ev);
    void EventHitEffect(Event *ev);
    void EventKilledEffect(Event *ev);
    void Archive(Archiver& arc) override;
};

extern Event EV_ThrowObject_Pickup;
extern Event EV_ThrowObject_Throw;

class ThrowObject : public Object
{
private:
    int    owner;
    Vector pickup_offset;
    str    throw_sound;

public:
    CLASS_PROTOTYPE(ThrowObject);

    ThrowObject();
    void Touch(Event *ev);
    void Throw(Event *ev);
    void Pickup(Event *ev);
    void PickupOffset(Event *ev);
    void ThrowSound(Event *ev);
    void Archive(Archiver& arc) override;
};

class HelmetObject : public Entity
{
public:
    CLASS_PROTOTYPE(HelmetObject);

    HelmetObject();
    void HelmetTouch(Event *ev);
};
