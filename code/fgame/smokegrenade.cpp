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

#include "smokegrenade.h"
#include "smokesprite.h"
#include "level.h"
#include "sentient.h"

Event EV_SmokeGrenade_SetSpawnRate
(
    "spawnrate",
    EV_DEFAULT,
    "f",
    "rate",
    "sets how many smoke volumes are spawned per second"
);
Event EV_SmokeGrenade_SetBouncefactor
(
    "bouncefactor",
    EV_DEFAULT,
    "f",
    "factor",
    "sets how far smoke volumes bounce when they collide with something"
);
Event EV_SmokeGrenade_SetLife
(
    "life",
    EV_DEFAULT,
    "f",
    "life",
    "sets how long the smoke emitter lives"
);
Event EV_SmokeGrenade_SetSpawnLife
(
    "spawnlife",
    EV_DEFAULT,
    "f",
    "life",
    "sets how long each smoke volume lives"
);
Event EV_SmokeGrenade_SetScalerate
(
    "scalerate",
    EV_DEFAULT,
    "f",
    "rate",
    "sets how fast the smoke volume grows"
);
Event EV_SmokeGrenade_SetVelocity
(
    "velocity",
    EV_DEFAULT,
    "fff",
    "vel_x vel_y vel_z",
    "sets how fast the smoke volume moves away, with x forward and z up"
);
Event EV_SmokeGrenade_SetSpin
(
    "spin",
    EV_DEFAULT,
    "f",
    "avel",
    "sets the number of rotations per second for this entity around its z axis"
);
Event EV_SmokeGrenade_SetOffset
(
    "offset",
    EV_DEFAULT,
    "fff",
    "offset_x offset_y offset_z",
    "sets offset of the smoke volume from the entity"
);
Event EV_SmokeGrenade_SetScale
(
    "scale",
    EV_DEFAULT,
    "f",
    "scale",
    "sets initial scale of each smoke volume"
);
Event EV_SmokeGrenade_SetFadein
(
    "fadein",
    EV_DEFAULT,
    "f",
    "time",
    "sets fadein time from 0 alpha to 1 alpha, in seconds"
);
Event EV_SmokeGrenade_SetFadedelay
(
    "fadedelay",
    EV_DEFAULT,
    "f",
    "time",
    "sets how long the entity waits until fading out from 1 alpha to 0 alpha, in seconds"
);
Event EV_SmokeGrenade_SetMaxAlpha
(
    "maxalpha",
    EV_DEFAULT,
    "f",
    "alpha",
    "sets maximum opacity of an individual smoke particle, in 0-1 range"
);

CLASS_DECLARATION(EffectEntity, SmokeGrenade, "smokegrenade")
{
    { &EV_SmokeGrenade_SetSpawnRate,        &SmokeGrenade::EventSetSpawnrate },
    { &EV_SmokeGrenade_SetBouncefactor,     &SmokeGrenade::EventSetBouncefactor },
    { &EV_SmokeGrenade_SetLife,             &SmokeGrenade::EventSetLife },
    { &EV_SmokeGrenade_SetSpawnLife,        &SmokeGrenade::EventSetSpawnLife },
    { &EV_SmokeGrenade_SetScalerate,        &SmokeGrenade::EventSetScalerate },
    { &EV_SmokeGrenade_SetVelocity,         &SmokeGrenade::EventSetVelocity },
    { &EV_SmokeGrenade_SetSpin,             &SmokeGrenade::EventSetSpin },
    { &EV_SmokeGrenade_SetOffset,           &SmokeGrenade::EventSetOffset },
    { &EV_SmokeGrenade_SetScale,            &SmokeGrenade::EventSetScale },
    { &EV_SmokeGrenade_SetFadein,           &SmokeGrenade::EventSetFadein },
    { &EV_SmokeGrenade_SetFadedelay,        &SmokeGrenade::EventSetFadedelay },
    { &EV_SmokeGrenade_SetMaxAlpha,         &SmokeGrenade::EventSetMaxAlpha },
    { NULL, NULL }
};

void SmokeGrenade::EventSetSpawnrate(Event* ev)
{
    float rate = ev->GetFloat(1);
    if (rate < 0.001f) rate = 0.001f;

    spawnRate = 1000.0f / rate;
}

void SmokeGrenade::EventSetBouncefactor(Event* ev)
{
    bounceFactor = ev->GetFloat(1);
}

void SmokeGrenade::EventSetLife(Event* ev)
{
    nextRemoveTime = level.time + ev->GetFloat(1);
}

void SmokeGrenade::EventSetSpawnLife(Event* ev)
{
    spawnLife = ev->GetFloat(1);
}

void SmokeGrenade::EventSetScalerate(Event* ev)
{
    scaleRate = ev->GetFloat(1);
}

void SmokeGrenade::EventSetVelocity(Event* ev)
{
    velocity[0] = ev->GetFloat(1);
    velocity[1] = ev->GetFloat(2);
    velocity[2] = ev->GetFloat(3);
}

void SmokeGrenade::EventSetSpin(Event* ev)
{
    spinRate = ev->GetFloat(1);
}

void SmokeGrenade::EventSetOffset(Event* ev)
{
    offset[0] = ev->GetFloat(1);
    offset[1] = ev->GetFloat(2);
    offset[2] = ev->GetFloat(3);
}

void SmokeGrenade::EventSetScale(Event* ev)
{
    scale = ev->GetFloat(1) * 0.5;
}

void SmokeGrenade::EventSetFadein(Event* ev)
{
    fadeIn = ev->GetFloat(1);
}

void SmokeGrenade::EventSetFadedelay(Event* ev)
{
    fadeDelay = ev->GetFloat(1);
}

void SmokeGrenade::EventSetMaxAlpha(Event* ev)
{
    maxAlpha = ev->GetFloat(1);
}

SmokeGrenade::SmokeGrenade()
{
    flags           |= FL_THINK;
    nextRemoveTime  = level.time + 1.0;
    spawnRate       = 1000;
    bounceFactor    = 0.0;
    spawnLife       = 1.0;
    scaleRate       = 1.0;
    velocity        = vec_zero;
    offset          = vec_zero;
    spinRate        = 0.0;
    scale           = 1.0;
    fadeIn          = 0.0;
    fadeDelay       = spawnLife;
    maxAlpha        = 1.0;
    lastSpawnTime   = level.inttime + 150;
}

void SmokeGrenade::Think()
{
    if (level.time >= nextRemoveTime) {
        Delete();
        return;
    }

    if (level.inttime >= lastSpawnTime + spawnRate) {
        SmokeSprite sp;
        float degrees;

        lastSpawnTime += spawnRate;

        sp.origin = origin;
        sp.spawnTime = level.time;
        sp.scale = scale;

        degrees = angles.y + (((level.inttime / 1000.0 - 176.0) * spinRate) * -0.36);
        
        RotatePointAroundAxis(
            sp.velocity,
            2,
            velocity,
            degrees
        );

        sp.spawnLife = spawnLife;
        sp.bounceFactor = bounceFactor;
        sp.scaleStart = scale;
        sp.scaleRate = scaleRate;
        sp.fadeIn = fadeIn;
        sp.fadeDelay = fadeDelay;
        sp.maxAlpha = maxAlpha;
        sp.owner = owner;

        G_AddSmokeSprite(&sp);
    }
}

void SmokeGrenade::setOwner(Sentient* other)
{
    owner = other;
}

Sentient* SmokeGrenade::getOwner() const
{
    return owner;
}
