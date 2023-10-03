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

#pragma once

#include "effectentity.h"

class Sentient;

class SmokeGrenade : public EffectEntity {
    CLASS_PROTOTYPE(SmokeGrenade);

public:
    SmokeGrenade();

    void EventSetSpawnrate(Event* ev);
    void EventSetBouncefactor(Event* ev);
    void EventSetLife(Event* ev);
    void EventSetSpawnLife(Event* ev);
    void EventSetScalerate(Event* ev);
    void EventSetVelocity(Event* ev);
    void EventSetSpin(Event* ev);
    void EventSetOffset(Event* ev);
    void EventSetScale(Event* ev);
    void EventSetFadein(Event* ev);
    void EventSetFadedelay(Event* ev);
    void EventSetMaxAlpha(Event* ev);

    void Think() override;

    void setOwner(Sentient* other);
    Sentient* getOwner() const;

private:
    float nextRemoveTime;
    int spawnRate;
    float bounceFactor;
    float spawnLife;
    float scaleRate;
    Vector velocity;
    float spinRate;
    Vector offset;
    float scale;
    float fadeIn;
    float fadeDelay;
    float maxAlpha;
    int lastSpawnTime;
    SafePtr<Sentient> owner;
};
