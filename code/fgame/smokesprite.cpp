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

#include "smokesprite.h"
#include "archive.h"
#include "level.h"
#include "debuglines.h"

Container<SmokeSprite> g_Sprites;

void G_ResetSmokeSprites()
{
    g_Sprites.ClearObjectList();
}

void G_ArchiveSmokeSpritesFunction(Archiver& arc, SmokeSprite *sp)
{
    arc.ArchiveVector(&sp->origin);
    arc.ArchiveVector(&sp->velocity);
    arc.ArchiveFloat(&sp->scale);
    arc.ArchiveFloat(&sp->spawnTime);
    arc.ArchiveFloat(&sp->spawnLife);
    arc.ArchiveFloat(&sp->bounceFactor);
    arc.ArchiveFloat(&sp->scaleStart);
    arc.ArchiveFloat(&sp->scaleRate);
    arc.ArchiveFloat(&sp->fadeIn);
    arc.ArchiveFloat(&sp->fadeDelay);
    arc.ArchiveFloat(&sp->maxAlpha);
    arc.ArchiveSafePointer(&sp->owner);
}

void G_ArchiveSmokeSprites(Archiver& arc)
{
    g_Sprites.Archive(arc, &G_ArchiveSmokeSpritesFunction);
}

qboolean UpdateSprite(SmokeSprite& sp)
{
    trace_t trace;

    if (sp.spawnTime + sp.spawnLife < level.time) {
        return qfalse;
    }

    trace = G_Trace(
        sp.origin,
        vec_zero,
        vec_zero,
        sp.origin + sp.velocity * level.frametime,
        NULL,
        MASK_SMOKE_SPRITE,
        qfalse,
        "smoke grenade physics"
    );

    sp.origin = trace.endpos;

    if (trace.fraction != 1.0) {
        float  fDot;
        vec3_t vCross;

        fDot = DotProduct(trace.plane.normal, sp.velocity) * 2.0;
        VectorScale(trace.plane.normal, fDot, vCross);

        VectorSubtract(sp.velocity, vCross, sp.velocity);
        VectorScale(sp.velocity, sp.bounceFactor, sp.velocity);
    }

    sp.scale += sp.scaleStart * sp.scaleRate * level.frametime;

    if (g_debugsmoke->integer) {
        G_DebugCircle(sp.origin, sp.scale, 1.0, 0.75, 0.75, 1.0);
    }

    return qtrue;
}

void G_UpdateSmokeSprites()
{
    for (int count = 1; count <= g_Sprites.NumObjects();) {
        if (UpdateSprite(g_Sprites.ObjectAt(count))) {
            count++;
        } else {
            g_Sprites.RemoveObjectAt(count);
        }
    }
}

void G_AddSmokeSprite(const SmokeSprite *sprite)
{
    g_Sprites.AddObject(*sprite);
}

float G_ObfuscationForSmokeSprites(float visibilityAlpha, const Vector& start, const Vector& end)
{
    Vector vDelta       = end - start;
    float  fLength      = vDelta.length();
    Vector vDir         = vDelta * (1.0 / fLength);
    float  fObfuscation = visibilityAlpha;
    int    i;

    for (i = 1; i <= g_Sprites.NumObjects(); i++) {
        const SmokeSprite& sprite       = g_Sprites.ObjectAt(i);
        Vector             vSpriteDelta = sprite.origin - start;
        float              fDot         = vSpriteDelta * vDir;
        float              fTimeAlive;

        if (fDot < -sprite.scale || fDot > fLength + sprite.scale) {
            continue;
        }

        if (fDot <= 0) {
            if (Square(sprite.scale) <= vSpriteDelta.lengthSquared()) {
                continue;
            }
        } else if (fDot >= fLength) {
            Vector vSpriteEndDelta = sprite.origin - end;

            if (Square(sprite.scale) <= vSpriteEndDelta.lengthSquared()) {
                continue;
            }
        } else {
            Vector vSpriteEndDelta = vSpriteDelta - (vDir * fDot);

            if (Square(sprite.scale) <= vSpriteEndDelta.lengthSquared()) {
                continue;
            }
        }

        fTimeAlive = level.time - sprite.spawnTime;
        if (fTimeAlive < sprite.fadeIn) {
            fObfuscation += sprite.maxAlpha * fTimeAlive / sprite.fadeIn;
        } else if (sprite.spawnTime + sprite.fadeDelay >= level.svsTime) {
            fObfuscation += sprite.maxAlpha;
        } else if (sprite.spawnLife - fTimeAlive > 0) {
            fObfuscation = (sprite.spawnLife - fTimeAlive) * sprite.maxAlpha / (sprite.spawnLife - sprite.fadeDelay);
        }

        if (fObfuscation >= 1.0) {
            // Completely obfuscated
            return 1.0;
        }
    }

    return fObfuscation;
}

SmokeSprite *G_GetRandomSmokeSprite()
{
    if (!g_Sprites.NumObjects()) {
        return NULL;
    }

    return &g_Sprites.ObjectAt((rand() % g_Sprites.NumObjects()) + 1);
}
