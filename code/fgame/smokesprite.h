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

#include "g_local.h"
#include "../qcommon/vector.h"
#include "../qcommon/class.h"

class SmokeSprite {
public:
    Vector origin;
    Vector velocity;
    float scale;
    float spawnTime;
    float spawnLife;
    float bounceFactor;
    float scaleStart;
    float scaleRate;
    float fadeIn;
    float fadeDelay;
    float maxAlpha;
    SafePtr<class Sentient> owner;
};

void G_ResetSmokeSprites();
void G_ArchiveSmokeSprites(Archiver& arc);
void G_UpdateSmokeSprites();
float G_ObfuscationForSmokeSprites(float, const Vector& end, const Vector& start);
SmokeSprite* G_GetRandomSmokeSprite();
void G_AddSmokeSprite(const SmokeSprite* sprite);
