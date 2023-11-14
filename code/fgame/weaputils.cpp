/*
===========================================================================
Copyright (C) 2015 the OpenMoHAA team

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

// weaputils.cpp: General Weapon Utility Functions

#include "g_phys.h"
#include "g_spawn.h"
#include "weaputils.h"
#include "specialfx.h"
#include "sentient.h"
#include "actor.h"
#include "decals.h"
#include "weapon.h"
#include "player.h"
#include "VehicleCollisionEntity.h"
#include "weapturret.h"
#include "vehicleturret.h"
#include "earthquake.h"
#include "trigger.h"
#include "debuglines.h"
#include "smokegrenade.h"

constexpr unsigned long MAX_TRAVEL_DIST = 16216;

static void FlashPlayers(Vector org, float r, float g, float b, float a, float rad, float time, int type);

qboolean MeleeAttack(
    Vector               pos,
    Vector               end,
    float                damage,
    Entity              *attacker,
    meansOfDeath_t       means_of_death,
    float                attack_width,
    float                attack_min_height,
    float                attack_max_height,
    float                knockback,
    qboolean             hit_dead,
    Container<Entity *> *victimlist
)
{
    trace_t             trace;
    Entity             *victim;
    Vector              dir;
    float               world_dist;
    Vector              new_pos;
    Entity             *skip_ent;
    qboolean            hit_something = false;
    Vector              mins;
    Vector              maxs;
    Container<Entity *> potential_victimlist;
    int                 i;
    int                 num_traces;
    Vector              start;

    // See how far the world is away

    dir        = end - pos;
    world_dist = dir.length();

    new_pos = pos;

    skip_ent = attacker;

    num_traces = 0;

    while (new_pos != end) {
        trace = G_Trace(pos, vec_zero, vec_zero, end, skip_ent, MASK_SOLID, false, "MeleeAttack - World test");

        num_traces++;

        if (trace.fraction < 1) {
            if ((trace.entityNum == ENTITYNUM_WORLD)
                || (trace.ent && trace.ent->entity && !trace.ent->entity->takedamage)) {
                dir        = trace.endpos - pos;
                world_dist = dir.length();
                break;
            } else {
                // Make sure we don't go backwards any in our trace
                if (Vector(new_pos - pos).length() + 0.001 >= Vector(trace.endpos - pos).length()) {
                    break;
                }

                if (num_traces > 10) {
                    // We have done too many traces, stop here
                    dir        = trace.endpos - pos;
                    world_dist = dir.length();
                    break;
                }

                new_pos = trace.endpos;

                if (trace.ent) {
                    skip_ent = trace.ent->entity;
                }
            }
        } else {
            break;
        }
    }

    // Find things hit

    dir = end - pos;
    dir.normalize();
    end = pos + (dir * world_dist);

    start   = pos - dir * (attack_width * 1.2f);
    start.z = pos.z - dir.z * (end.z - start.z);

    victim = NULL;

    mins = Vector(-attack_width, -attack_width, attack_min_height);
    maxs = Vector(attack_width, attack_width, attack_max_height);

    G_TraceEntities(pos, mins, maxs, end, &potential_victimlist, MASK_MELEE);

    /*int previous_contents = attacker->edict->r.contents;
    attacker->edict->r.contents = 0;
    trace = G_Trace( pos, mins, maxs, end, ( ( Sentient * )attacker )->GetActiveWeapon( WEAPON_MAIN ), MASK_MELEE, false, "MeleeAttack" );

    if( trace.ent && trace.ent->entity )
    {
        gi.Printf( "HIT\n" );
        potential_victimlist.AddObject( trace.ent->entity );
    }

    attacker->edict->r.contents = previous_contents;*/

    for (i = 1; i <= potential_victimlist.NumObjects(); i++) {
        victim = potential_victimlist.ObjectAt(i);

        if (victim && victim->takedamage && victim != attacker) {
            dir = end - pos;
            dir.normalize();

            if (dir == vec_zero) {
                dir = victim->centroid - pos;
                dir.normalize();
            }

            if (victim->IsSubclassOfSentient() && !victim->IsDead()) {
                hit_something = true;
            }

            if (victim->health > 0 || hit_dead) {
                if (victimlist && victim->IsSubclassOfSentient() && !victim->IsDead()) {
                    victimlist->AddObject(victim);
                }

                victim->Damage(attacker, attacker, damage, pos, dir, vec_zero, knockback, 0, means_of_death);

                if (victim->edict->r.contents & CONTENTS_SOLID) {
                    victim->Sound("pistol_hit");
                }
            }
        }
    }

    return hit_something;
}

#define DEFAULT_SWORD_DAMAGE    10
#define DEFAULT_SWORD_KNOCKBACK 50

Event EV_Projectile_Speed
(
    "speed",
    EV_DEFAULT,
    "f",
    "projectileSpeed",
    "set the speed of the projectile",
    EV_NORMAL
);
Event EV_Projectile_MinSpeed
(
    "minspeed",
    EV_DEFAULT,
    "f",
    "minspeed",
    "set the minimum speed of the projectile (this is for charge up weapons)",
    EV_NORMAL
);
Event EV_Projectile_ChargeSpeed
(
    "chargespeed",
    EV_DEFAULT,
    NULL,
    NULL,
    "set the projectile's speed to be determined by the charge time",
    EV_NORMAL
);
Event EV_Projectile_Damage
(
    "hitdamage",
    EV_DEFAULT,
    "f",
    "projectileHitDamage",
    "set the damage a projectile does when it hits something",
    EV_NORMAL
);
Event EV_Projectile_Life
(
    "life",
    EV_DEFAULT,
    "f",
    "projectileLife",
    "set the life of the projectile",
    EV_NORMAL
);
Event EV_Projectile_DMLife
(
    "dmlife",
    EV_DEFAULT,
    "f",
    "projectileLife",
    "set the life of the projectile in DM",
    EV_NORMAL
);
Event EV_Projectile_MinLife
(
    "minlife",
    EV_DEFAULT,
    "f",
    "minProjectileLife",
    "set the minimum life of the projectile (this is for charge up weapons)",
    EV_NORMAL
);
Event EV_Projectile_ChargeLife
(
    "chargelife",
    EV_DEFAULT,
    NULL,
    NULL,
    "set the projectile's life to be determined by the charge time",
    EV_NORMAL
);
// Added in 2.0
Event EV_Projectile_SetFuse
(
    "fuse",
    EV_DEFAULT,
    "i",
    "fuse",
    "set the projectile's life to be determined by the charge time",
    EV_NORMAL
);
Event EV_Projectile_Knockback
(
    "knockback",
    EV_DEFAULT,
    "f",
    "projectileKnockback",
    "set the knockback of the projectile when it hits something",
    EV_NORMAL
);
Event EV_Projectile_DLight
(
    "dlight",
    EV_DEFAULT,
    "ffff",
    "red green blue intensity",
    "set the color and intensity of the dynamic light on the projectile",
    EV_NORMAL
);
Event EV_Projectile_Avelocity
(
    "avelocity",
    EV_DEFAULT,
    "SFSFSF",
    "[random|crandom] yaw [random|crandom] pitch [random|crandom] roll",
    "set the angular velocity of the projectile",
    EV_NORMAL
);
Event EV_Projectile_MeansOfDeath
(
    "meansofdeath",
    EV_DEFAULT,
    "s",
    "meansOfDeath",
    "set the meansOfDeath of the projectile",
    EV_NORMAL
);
Event EV_Projectile_BeamCommand
(
    "beam",
    EV_DEFAULT,
    "sSSSSSS",
    "command arg1 arg2 arg3 arg4 arg5 arg6",
    "send a command to the beam of this projectile",
    EV_NORMAL
);
Event EV_Projectile_UpdateBeam
(
    "updatebeam",
    EV_DEFAULT,
    NULL,
    NULL,
    "Update the attached beam",
    EV_NORMAL
);
Event EV_Projectile_BounceTouch
(
    "bouncetouch",
    EV_DEFAULT,
    NULL,
    NULL,
    "Make the projectile bounce when it hits a non-damageable solid",
    EV_NORMAL
);
Event EV_Projectile_BounceSound
(
    "bouncesound",
    EV_DEFAULT,
    NULL,
    NULL,
    "Set the name of the sound that is played when the projectile bounces",
    EV_NORMAL
);
Event EV_Projectile_BounceSound_Metal
(
    "bouncesound_metal",
    EV_DEFAULT,
    NULL,
    NULL,
    "Set the name of the sound that is played when the projectile bounces off metal",
    EV_NORMAL
);
Event EV_Projectile_BounceSound_Hard
(
    "bouncesound_hard",
    EV_DEFAULT,
    NULL,
    NULL,
    "Set the name of the sound that is played when the projectile bounces off hard surfaces",
    EV_NORMAL
);
Event EV_Projectile_BounceSound_Water
(
    "bouncesound_water",
    EV_DEFAULT,
    NULL,
    NULL,
    "Set the name of the sound that is played when the projectile bounces off water",
    EV_NORMAL
);
Event EV_Projectile_Explode
(
    "explode",
    EV_DEFAULT,
    NULL,
    NULL,
    "Make the projectile explode",
    EV_NORMAL
);
Event EV_Projectile_ImpactMarkShader
(
    "impactmarkshader",
    EV_DEFAULT,
    "s",
    "shader",
    "Set the impact mark of the shader",
    EV_NORMAL
);
Event EV_Projectile_ImpactMarkRadius
(
    "impactmarkradius",
    EV_DEFAULT,
    "f",
    "radius",
    "Set the radius of the impact mark",
    EV_NORMAL
);
Event EV_Projectile_ImpactMarkOrientation
(
    "impactmarkorientation",
    EV_DEFAULT,
    "f",
    "degrees",
    "Set the orientation of the impact mark",
    EV_NORMAL
);
Event EV_Projectile_SetExplosionModel
(
    "explosionmodel",
    EV_DEFAULT,
    "s",
    "modelname",
    "Set the modelname of the explosion to be spawned",
    EV_NORMAL
);
Event EV_Projectile_SetAddVelocity
(
    "addvelocity",
    EV_DEFAULT,
    "fff",
    "velocity_x velocity_y velocity_z",
    "Set a velocity to be added to the projectile when it is created",
    EV_NORMAL
);
Event EV_Projectile_AddOwnerVelocity
(
    "addownervelocity",
    EV_DEFAULT,
    "b",
    "bool",
    "Set whether or not the owner's velocity is added to the projectile's velocity",
    EV_NORMAL
);
Event EV_Projectile_HeatSeek
(
    "heatseek",
    EV_DEFAULT,
    NULL,
    NULL,
    "Make the projectile heat seek",
    EV_NORMAL
);
Event EV_Projectile_Drunk
(
    "drunk",
    EV_DEFAULT,
    "ff",
    "amount rate",
    "Make the projectile drunk",
    EV_NORMAL
);
Event EV_Projectile_Prethink
(
    "prethink",
    EV_DEFAULT,
    NULL,
    NULL,
    "Make the projectile think to update it's velocity",
    EV_NORMAL
);
Event EV_Projectile_SetCanHitOwner
(
    "canhitowner",
    EV_DEFAULT,
    NULL,
    NULL,
    "Make the projectile be able to hit its owner",
    EV_NORMAL
);
Event EV_Projectile_ClearOwner
(
    "clearowner",
    EV_DEFAULT,
    NULL,
    NULL,
    "Make the projectile be able to hit its owner now",
    EV_NORMAL
);
Event EV_Projectile_RemoveWhenStopped
(
    "removewhenstopped",
    EV_DEFAULT,
    NULL,
    NULL,
    "Make the projectile get removed when it stops",
    EV_NORMAL
);
Event EV_Projectile_ExplodeOnTouch
(
    "explodeontouch",
    EV_DEFAULT,
    NULL,
    NULL,
    "Make the projectile explode when it touches something damagable",
    EV_NORMAL
);
Event EV_Projectile_NoTouchDamage
(
    "notouchdamage",
    EV_DEFAULT,
    NULL,
    NULL,
    "Makes the projectile not blow up or deal damage when it touches a damagable object",
    EV_NORMAL
);
Event EV_Projectile_SetSmashThroughGlass
(
    "smashthroughglass",
    EV_DEFAULT,
    "i",
    "speed",
    "Makes the projectile smash through windows & other damageble glass objects when going above a set speed",
    EV_NORMAL
);
Event EV_Projectile_SmashThroughGlass
(
    "_smashthroughglass",
    EV_DEFAULT,
    NULL,
    NULL,
    "Think function for smashing through glass",
    EV_NORMAL
);
// Added in 2.0
Event EV_Projectile_ArcToTarget
(
    "arctotarget",
    EV_DEFAULT,
    NULL,
    NULL,
    "Make the projectile follow a normal arc on its way to its target",
    EV_NORMAL
);
// Added in 2.0
Event EV_Projectile_BecomeBomb
(
    "becomebomb",
    EV_DEFAULT,
    NULL,
    NULL,
    "Make the projectile into a bomb",
    EV_NORMAL
);
// Added in 2.30
Event EV_Projectile_DieInWater
(
    "dieinwater",
    EV_DEFAULT,
    NULL,
    NULL,
    "Make the projectile die when gets wet",
    EV_NORMAL
);

CLASS_DECLARATION(Animate, Projectile, NULL) {
    {&EV_Touch,                            &Projectile::Touch                   },
    {&EV_Projectile_Speed,                 &Projectile::SetSpeed                },
    {&EV_Projectile_MinSpeed,              &Projectile::SetMinSpeed             },
    {&EV_Projectile_ChargeSpeed,           &Projectile::SetChargeSpeed          },
    {&EV_Projectile_Damage,                &Projectile::SetDamage               },
    {&EV_Projectile_Life,                  &Projectile::SetLife                 },
    {&EV_Projectile_DMLife,                &Projectile::SetDMLife               },
    {&EV_Projectile_MinLife,               &Projectile::SetMinLife              },
    {&EV_Projectile_ChargeLife,            &Projectile::SetChargeLife           },
    {&EV_Projectile_SetFuse,               &Projectile::SetFuse                 },
    {&EV_Projectile_Knockback,             &Projectile::SetKnockback            },
    {&EV_Projectile_DLight,                &Projectile::SetDLight               },
    {&EV_Projectile_Avelocity,             &Projectile::SetAvelocity            },
    {&EV_Projectile_MeansOfDeath,          &Projectile::SetMeansOfDeath         },
    {&EV_Projectile_BounceTouch,           &Projectile::SetBounceTouch          },
    {&EV_Projectile_BounceSound,           &Projectile::SetBounceSound          },
    {&EV_Projectile_BounceSound_Metal,     &Projectile::SetBounceSoundMetal     },
    {&EV_Projectile_BounceSound_Hard,      &Projectile::SetBounceSoundHard      },
    {&EV_Projectile_BounceSound_Water,     &Projectile::SetBounceSoundWater     },
    {&EV_Projectile_BeamCommand,           &Projectile::BeamCommand             },
    {&EV_Projectile_UpdateBeam,            &Projectile::UpdateBeam              },
    {&EV_Projectile_Explode,               &Projectile::Explode                 },
    {&EV_Projectile_ImpactMarkShader,      &Projectile::SetImpactMarkShader     },
    {&EV_Projectile_ImpactMarkRadius,      &Projectile::SetImpactMarkRadius     },
    {&EV_Projectile_ImpactMarkOrientation, &Projectile::SetImpactMarkOrientation},
    {&EV_Projectile_SetExplosionModel,     &Projectile::SetExplosionModel       },
    {&EV_Projectile_SetAddVelocity,        &Projectile::SetAddVelocity          },
    {&EV_Projectile_AddOwnerVelocity,      &Projectile::AddOwnerVelocity        },
    {&EV_Projectile_HeatSeek,              &Projectile::HeatSeek                },
    {&EV_Projectile_Drunk,                 &Projectile::Drunk                   },
    {&EV_Projectile_Prethink,              &Projectile::Prethink                },
    {&EV_Projectile_SetCanHitOwner,        &Projectile::SetCanHitOwner          },
    {&EV_Projectile_ClearOwner,            &Projectile::ClearOwner              },
    {&EV_Projectile_RemoveWhenStopped,     &Projectile::RemoveWhenStopped       },
    {&EV_Projectile_ExplodeOnTouch,        &Projectile::ExplodeOnTouch          },
    {&EV_Projectile_NoTouchDamage,         &Projectile::SetNoTouchDamage        },
    {&EV_Projectile_SetSmashThroughGlass,  &Projectile::SetSmashThroughGlass    },
    {&EV_Projectile_SmashThroughGlass,     &Projectile::SmashThroughGlassThink  },
    {&EV_Projectile_BecomeBomb,            &Projectile::BecomeBomb              },
    {&EV_Killed,                           &Projectile::Explode                 },
    {&EV_Stop,                             &Projectile::Stopped                 },
    {&EV_Projectile_ArcToTarget,           &Projectile::ArcToTarget             },
    {&EV_Projectile_DieInWater,            &Projectile::DieInWater              },
    {NULL,                                 NULL                                 }
};

Projectile::Projectile()
{
    entflags |= EF_PROJECTILE;

    if (LoadingSavegame) {
        // Archive function will setup all necessary data
        return;
    }

    m_beam               = NULL;
    speed                = 0;
    minspeed             = 0;
    damage               = 0;
    life                 = 5;
    dmlife               = 0;
    knockback            = 0;
    dlight_radius        = 0;
    dlight_color         = Vector(1, 1, 1);
    avelocity            = Vector(0, 0, 0);
    mins                 = Vector(-1, -1, -1);
    maxs                 = Vector(1, 1, 1);
    meansofdeath         = MOD_NONE;
    projFlags            = 0;
    fLastBounceTime      = 0;
    gravity              = 0;
    impactmarkradius     = 10;
    charge_fraction      = 1.0;
    target               = NULL;
    fDrunk               = 0;
    fDrunkRate           = 0;
    m_iSmashThroughGlass = 0;
    addownervelocity     = qtrue;
    can_hit_owner        = false;
    remove_when_stopped  = false;
    m_bExplodeOnTouch    = false;
    m_bHurtOwnerOnly     = false;
    takedamage           = DAMAGE_NO;
    owner                = ENTITYNUM_NONE;
    edict->r.ownerNum    = ENTITYNUM_NONE;
    m_bArcToTarget       = false;
    m_bDieInWater        = false;

    // make this shootable but non-solid on the client
    setContents(CONTENTS_SHOOTONLY);

    //
    // touch triggers by default
    //
    flags |= FL_TOUCH_TRIGGERS;

    m_iTeam           = 0;
    m_bHadPlayerOwner = false;
}

float Projectile::ResolveMinimumDistance(Entity *potential_target, float currmin)
{
    float  currdist;
    float  dot;
    Vector angle;
    Vector delta;
    Vector norm;
    float  sine = 0.4f;

    delta = potential_target->centroid - this->origin;

    norm = delta;
    norm.normalize();

    // Test if the target is in front of the missile
    dot = norm * orientation[0];
    if (dot < 0) {
        return currmin;
    }

    // Test if we're within the rocket's viewcone (45 degree cone)
    dot = norm * orientation[1];
    if (fabs(dot) > sine) {
        return currmin;
    }

    dot = norm * orientation[2];
    if (fabs(dot) > sine) {
        return currmin;
    }

    currdist = delta.length();
    if (currdist < currmin) {
        currmin = currdist;
        target  = potential_target;
    }

    return currmin;
}

float Projectile::AdjustAngle(float maxadjust, float currangle, float targetangle)
{
    float dangle;
    float magangle;

    dangle = currangle - targetangle;

    if (dangle) {
        magangle = (float)fabs(dangle);

        while (magangle >= 360.0f) {
            magangle -= 360.0f;
        }

        if (magangle < maxadjust) {
            currangle = targetangle;
        } else {
            if (magangle > 180.0f) {
                maxadjust = -maxadjust;
            }
            if (dangle > 0) {
                maxadjust = -maxadjust;
            }
            currangle += maxadjust;
        }
    }

    while (currangle >= 360.0f) {
        currangle -= 360.0f;
    }

    while (currangle < 0.0f) {
        currangle += 360.0f;
    }

    return currangle;
}

void Projectile::Drunk(Event *ev)
{
    if (fDrunk) {
        return;
    }

    fDrunk     = ev->GetFloat(1);
    fDrunkRate = ev->GetFloat(2);

    PostEvent(EV_Projectile_Prethink, 0);
}

void Projectile::HeatSeek(Event *ev)
{
    float   mindist;
    Entity *ent;
    trace_t trace;
    Vector  delta;
    Vector  v;
    int     n;
    int     i;

    if ((!target) || (target == world)) {
        mindist = 8192.0f;

        n = SentientList.NumObjects();
        for (i = 1; i <= n; i++) {
            ent = SentientList.ObjectAt(i);
            if (ent->entnum == owner) {
                continue;
            }

            if (((ent->takedamage != DAMAGE_AIM) || (ent->health <= 0))) {
                continue;
            }

            trace = G_Trace(
                this->origin, vec_zero, vec_zero, ent->centroid, this, MASK_SHOT, qfalse, "DrunkMissile::HeatSeek"
            );
            if ((trace.fraction != 1.0) && (trace.ent != ent->edict)) {
                continue;
            }

            mindist = ResolveMinimumDistance(ent, mindist);
        }
    } else {
        float angspeed;

        delta = target->centroid - this->origin;
        v     = delta.toAngles();

        angspeed = 5.0f;
        angles.x = AdjustAngle(angspeed, angles.x, v.x);
        angles.y = AdjustAngle(angspeed, angles.y, v.y);
        angles.z = AdjustAngle(angspeed, angles.z, v.z);
    }
    PostEvent(EV_Projectile_HeatSeek, 0.1f);
    PostEvent(EV_Projectile_Prethink, 0);
}

void Projectile::Prethink(Event *ev)
{
    Vector end;

    if (fDrunk >= 0.0f) {
        if (fDrunk) {
            float rnd1 = G_Random();
            float rnd2 = G_Random();

            angles += Vector((rnd1 + rnd1 - 1.0f) * fDrunk, (rnd2 + rnd2 - 1.0f) * fDrunk, 0);

            speed *= 0.98f;
            fDrunk += fDrunkRate;
            if (speed < 500.0f) {
                fDrunk = -fDrunk;
            }
        }

        setAngles(angles);
        velocity = Vector(orientation[0]) * speed;
    } else {
        fDrunk *= 0.80f;

        if (fDrunk > -1.0f) {
            fDrunk = -1.0f;
        }

        //float rnd1 = rand() & 0x7FFF;
        //float rnd2 = rand() & 0x7FFF;

        //angles += Vector( ( rnd1 * 0.00003f + rnd1 * 0.00003f - 1.0f ) * fDrunk, ( rnd2 * 0.00003f + rnd2 * 0.00003f - 1.0f ) * fDrunk, 0 );
        velocity[2] -= 0.05f * sv_gravity->integer * 0.15f;

        angles = velocity.toAngles();
        setAngles(angles);
    }

    PostEvent(EV_Projectile_Prethink, 0.05f);
}

void Projectile::AddOwnerVelocity(Event *ev)
{
    addownervelocity = ev->GetBoolean(1);
}

void Projectile::SetAddVelocity(Event *ev)
{
    addvelocity.x = ev->GetFloat(1);
    addvelocity.y = ev->GetFloat(2);
    addvelocity.z = ev->GetFloat(3);
}

void Projectile::SetExplosionModel(Event *ev)
{
    explosionmodel = ev->GetString(1);
}

void Projectile::SetImpactMarkShader(Event *ev)
{
    impactmarkshader = ev->GetString(1);
}

void Projectile::SetImpactMarkRadius(Event *ev)
{
    impactmarkradius = ev->GetFloat(1);
}

void Projectile::SetImpactMarkOrientation(Event *ev)
{
    impactmarkorientation = ev->GetString(1);
}

void Projectile::Explode(Event *ev)
{
    Entity *owner;
    Entity *ignoreEnt = NULL;

    if (!CheckTeams()) {
        PostEvent(EV_Remove, EV_REMOVE);
        return;
    }

    if (ev->NumArgs() == 1) {
        ignoreEnt = ev->GetEntity(1);
    }

    // Get the owner of this projectile
    owner = G_GetEntity(this->owner);

    // If the owner's not here, make the world the owner
    if (!owner) {
        owner = world;
    }

    if (owner->IsDead() || owner == world) {
        weap = NULL;
    }

    health     = 0;
    deadflag   = DEAD_DEAD;
    takedamage = DAMAGE_NO;

    // Spawn an explosion model
    if (explosionmodel.length()) {
        // Move the projectile back off the surface a bit so we can see
        // explosion effects.
        Vector dir, v;
        v = velocity;
        v.normalize();
        dir = v;
        v   = origin - v * 36;
        setOrigin(v);

        ExplosionAttack(v, owner, explosionmodel, dir, ignoreEnt, 1.0f, weap, m_bHurtOwnerOnly);
    }

    CancelEventsOfType(EV_Projectile_UpdateBeam);

    // Kill the beam
    if (m_beam) {
        m_beam->ProcessEvent(EV_Remove);
        m_beam = NULL;
    }

    // Remove the projectile
    PostEvent(EV_Remove, level.frametime);
}

void Projectile::SetBounceTouch(Event *ev)
{
    projFlags |= P_BOUNCE_TOUCH;
    setMoveType(MOVETYPE_BOUNCE);
}

void Projectile::SetNoTouchDamage(Event *ev)
{
    projFlags |= P_NO_TOUCH_DAMAGE;
}

void Projectile::SetSmashThroughGlass(Event *ev)
{
    m_iSmashThroughGlass = ev->GetInteger(1);

    PostEvent(EV_Projectile_SmashThroughGlass, level.frametime);
}

void Projectile::SmashThroughGlassThink(Event *ev)
{
    if (velocity.length() > m_iSmashThroughGlass) {
        Vector  vEnd;
        trace_t trace;

        Entity *ent = G_GetEntity(owner);

        vEnd = velocity * level.frametime + velocity * level.frametime + origin;

        trace = G_Trace(origin, vec_zero, vec_zero, vEnd, ent, MASK_BEAM, false, "SmashThroughGlassThink");

        if ((trace.ent) && (trace.ent->entity != world)) {
            Entity *obj = trace.ent->entity;

            if ((trace.surfaceFlags & SURF_GLASS) && (obj->takedamage)) {
                obj->Damage(
                    this, ent, obj->health + 1, origin, velocity, trace.plane.normal, knockback, 0, meansofdeath
                );
            }
        }
    }

    PostEvent(EV_Projectile_SmashThroughGlass, level.frametime);
}

void Projectile::BeamCommand(Event *ev)
{
    int i;

    if (!m_beam) {
        m_beam = new FuncBeam;

        m_beam->setOrigin(this->origin);
        m_beam->Ghost(NULL);
    }

    Event *beamev = new Event(ev->GetToken(1));

    for (i = 2; i <= ev->NumArgs(); i++) {
        beamev->AddToken(ev->GetToken(i));
    }

    m_beam->ProcessEvent(beamev);
    PostEvent(EV_Projectile_UpdateBeam, level.frametime);
}

void Projectile::UpdateBeam(Event *ev)
{
    if (m_beam) {
        m_beam->setOrigin(this->origin);
        PostEvent(EV_Projectile_UpdateBeam, level.frametime);
    }
}

void Projectile::SetBounceSound(Event *ev)
{
    bouncesound = ev->GetString(1);
}

void Projectile::SetBounceSoundMetal(Event *ev)
{
    bouncesound_metal = ev->GetString(1);
}

void Projectile::SetBounceSoundHard(Event *ev)
{
    bouncesound_hard = ev->GetString(1);
}

void Projectile::SetBounceSoundWater(Event *ev)
{
    bouncesound_water = ev->GetString(1);
}

void Projectile::SetChargeLife(Event *ev)
{
    projFlags |= P_CHARGE_LIFE;
}

void Projectile::SetFuse(Event *ev)
{
    if (ev->GetInteger(1)) {
        projFlags |= P_FUSE;
    } else {
        projFlags &= ~P_FUSE;
    }
}

void Projectile::SetMinLife(Event *ev)
{
    minlife = ev->GetFloat(1);
    projFlags |= P_CHARGE_LIFE;
}

void Projectile::SetLife(Event *ev)
{
    life = ev->GetFloat(1);
}

void Projectile::SetDMLife(Event *ev)
{
    dmlife = ev->GetFloat(1);
}

void Projectile::SetSpeed(Event *ev)

{
    speed = ev->GetFloat(1);
}

void Projectile::SetMinSpeed(Event *ev)
{
    minspeed = ev->GetFloat(1);
    projFlags |= P_CHARGE_SPEED;
}

void Projectile::SetChargeSpeed(Event *ev)
{
    projFlags |= P_CHARGE_SPEED;
}

void Projectile::SetAvelocity(Event *ev)
{
    int i = 1;
    int j = 0;
    str vel;

    if (ev->NumArgs() < 3) {
        warning("ClientGameCommandManager::SetAngularVelocity", "Expecting at least 3 args for command randvel");
    }

    while (j < 3) {
        vel = ev->GetString(i++);
        if (vel == "crandom") {
            avelocity[j++] = ev->GetFloat(i++) * crandom();
        } else if (vel == "random") {
            avelocity[j++] = ev->GetFloat(i++) * random();
        } else {
            avelocity[j++] = atof(vel.c_str());
        }
    }
}

void Projectile::SetDamage(Event *ev)
{
    damage = ev->GetFloat(1);
}

void Projectile::SetKnockback(Event *ev)
{
    knockback = ev->GetFloat(1);
}

void Projectile::SetDLight(Event *ev)
{
    dlight_color[0] = ev->GetFloat(1);
    dlight_color[1] = ev->GetFloat(2);
    dlight_color[2] = ev->GetFloat(3);
    dlight_radius   = ev->GetFloat(4);
}

void Projectile::SetMeansOfDeath(Event *ev)
{
    meansofdeath = (meansOfDeath_t)MOD_string_to_int(ev->GetString(1));
}

void Projectile::DoDecal(void)
{
    if (impactmarkshader.length()) {
        Decal *decal = new Decal;
        decal->setShader(impactmarkshader);
        decal->setOrigin(level.impact_trace.endpos);
        decal->setDirection(level.impact_trace.plane.normal);
        decal->setOrientation(impactmarkorientation);
        decal->setRadius(impactmarkradius);
    }
}

void Projectile::Touch(Event *ev)
{
    Entity *other;
    Entity *owner;
    str     realname;

    // Other is what got hit
    other = ev->GetEntity(1);
    assert(other);

    // Don't touch teleporters
    if (other->isSubclassOf(Teleporter)) {
        return;
    }

    // Can't hit yourself with a projectile
    if (other->entnum == this->owner) {
        return;
    }

    // Remove it if we hit the sky
    if (HitSky()) {
        PostEvent(EV_Remove, 0);
        return;
    }

    if (!CheckTeams()) {
        PostEvent(EV_Remove, EV_REMOVE);
        return;
    }

    // Bouncy Projectile
    if ((projFlags & P_BOUNCE_TOUCH)) {
        str snd;

        if (level.time - fLastBounceTime < 0.1f) {
            fLastBounceTime = level.time;
            return;
        }

        if (level.impact_trace.surfaceFlags & SURF_PUDDLE
            || (gi.pointcontents(level.impact_trace.endpos, 0) & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))) {
            if (bouncesound_water.length()) {
                this->Sound(bouncesound_water, CHAN_BODY);
            }
        } else {
            if (bouncesound_metal.length()) {
                snd = bouncesound_metal;
            } else if (bouncesound_hard.length()) {
                snd = bouncesound_hard;
            } else {
                snd = bouncesound;
            }

            int flags = level.impact_trace.surfaceFlags;

            if (flags & SURF_MUD) {
                if (bouncesound.length()) {
                    Sound(bouncesound, CHAN_BODY);
                }
            } else if (flags & SURF_ROCK) {
                if (bouncesound_hard.length()) {
                    Sound(bouncesound_hard, CHAN_BODY);
                }
            } else if (flags & SURF_GRILL) {
                if (bouncesound_metal.length()) {
                    Sound(bouncesound_metal, CHAN_BODY);
                }
            } else if (flags & SURF_WOOD) {
                if (bouncesound_hard.length()) {
                    Sound(bouncesound_hard, CHAN_BODY);
                }
            } else if (flags & SURF_METAL) {
                if (bouncesound_metal.length()) {
                    Sound(bouncesound_metal, CHAN_BODY);
                }
            } else if (flags & SURF_GLASS) {
                if (bouncesound_hard.length()) {
                    Sound(bouncesound_hard, CHAN_BODY);
                }
            } else {
                if (bouncesound.length()) {
                    Sound(bouncesound, CHAN_BODY);
                }
            }
        }

        BroadcastAIEvent(AI_EVENT_WEAPON_IMPACT);
        return;
    }

    if (!m_bExplodeOnTouch && damage == 0.0f) {
        return;
    }

    // Get the owner of this projectile
    owner = G_GetEntity(this->owner);

    // If the owner's not here, make the world the owner
    if (!owner) {
        owner = world;
    }

    if (owner->IsDead() || owner == world) {
        weap = NULL;
    }

    // Damage the thing that got hit
    if (other->takedamage) {
        other->Damage(
            this, owner, damage, origin, velocity, level.impact_trace.plane.normal, knockback, 0, meansofdeath
        );
    }

    if (!g_gametype->integer && weap) {
        if (other->IsSubclassOfPlayer() || other->IsSubclassOfVehicle() || other->IsSubclassOfVehicleTank()
            || other->isSubclassOf(VehicleCollisionEntity)) {
            weap->m_iNumHits++;
            weap->m_iNumTorsoShots++;

            if (weap->IsSubclassOfVehicleTurretGun()) {
                VehicleTurretGun *t = (VehicleTurretGun *)weap.Pointer();
                Player           *p = (Player *)t->GetRemoteOwner().Pointer();

                if (p && p->IsSubclassOfPlayer()) {
                    p->m_iNumHits++;
                    p->m_iNumTorsoShots++;
                }
            }
        }
    }

    if (!m_bExplodeOnTouch) {
        return;
    }

    // Make the projectile not solid
    setSolidType(SOLID_NOT);
    setMoveType(MOVETYPE_NONE);
    hideModel();

    // Do a decal
    DoDecal();

    BroadcastAIEvent(AI_EVENT_WEAPON_FIRE);

    // Remove the projectile
    PostEvent(EV_Remove, 0);

    // Call the explosion event
    Event *explEv;
    explEv = new Event(EV_Projectile_Explode);
    explEv->AddEntity(other);
    ProcessEvent(explEv);
}

void Projectile::SetCanHitOwner(Event *ev)
{
    can_hit_owner = true;
}

void Projectile::ClearOwner(Event *ev)
{
    this->owner       = ENTITYNUM_NONE;
    edict->r.ownerNum = ENTITYNUM_NONE;
}

void Projectile::RemoveWhenStopped(Event *ev)
{
    remove_when_stopped = true;
}

void Projectile::ExplodeOnTouch(Event *ev)
{
    m_bExplodeOnTouch = true;
}

Sentient *Projectile::GetOwner(void)
{
    Sentient *pOwner = (Sentient *)G_GetEntity(owner);
    if (!pOwner || !pOwner->IsSubclassOfSentient()) {
        return NULL;
    } else {
        return pOwner;
    }
}

void Projectile::SetOwner(Entity *owner)
{
    if (owner) {
        this->owner       = owner->entnum;
        edict->r.ownerNum = owner->entnum;

        if (owner->IsSubclassOfPlayer()) {
            Player *p = (Player *)owner;
            m_iTeam   = p->GetTeam();
            // this was added in openmohaa to prevent glitches
            // like when the player disconnects or when the player spectates
            m_bHadPlayerOwner = true;
        }

        m_pOwnerPtr = owner;
    }
}

void Projectile::ArcToTarget(Event *ev)
{
    m_bArcToTarget = true;
    PostEvent(EV_Projectile_Prethink, 0);
}

void Projectile::BecomeBomb(Event *ev)
{
    if (ev->NumArgs() > 0) {
        Entity *ent = ev->GetEntity(1);
        if (ent) {
            owner = ent->entnum;
            setOrigin(ent->origin - Vector(0, 0, 48));
            setAngles(ent->angles);
            velocity = ent->velocity;
        }
    }

    m_bExplodeOnTouch = true;
    gravity           = 1.f;
    setMoveType(MOVETYPE_BOUNCE);
    setSolidType(SOLID_BBOX);
    edict->clipmask = MASK_PROJECTILE;
    setSize(mins, maxs);
}

void Projectile::DieInWater(Event *ev)
{
    m_bDieInWater = true;
}

void Projectile::Stopped(Event *ev)
{
    if (remove_when_stopped) {
        PostEvent(EV_Remove, 0);
    }
}

void Projectile::Think()
{
    float angle, spin;

    avelocity.x *= 1.3f;
    angle = AngleNormalize180(angles.x);
    spin  = avelocity.x * level.frametime + angle;

    if ((angle >= 0 || spin <= 0) && (angle <= 0 || spin >= 0)) {
        angle = AngleNormalize360(angles.x);
        spin  = avelocity.x * level.frametime + angle;

        if ((angle < 180 && spin > 180) || (angle > 180 && spin < 180)) {
            setAngles(Vector(0, angles.y + 180, angles.z));
            avelocity = vec_zero;
            flags &= ~FL_THINK;
        }
    } else {
        setAngles(Vector(0, angles.y, angles.z));
        avelocity = vec_zero;
        flags &= ~FL_THINK;
    }
}

bool Projectile::CheckTeams(void)
{
    Player *pOwner;

    if (g_gametype->integer == GT_SINGLE_PLAYER) {
        // Ignore in single-player mode
        return true;
    }

    pOwner = (Player *)m_pOwnerPtr.Pointer();

    if (!pOwner) {
        // Owner disconnected
        if (m_bHadPlayerOwner) {
            return false;
        } else {
            return true;
        }
    }

    if (pOwner->IsSubclassOfPlayer()) {
        if (m_iTeam != TEAM_NONE && m_iTeam != pOwner->GetTeam()) {
            return false;
        }
    } else if (pOwner->IsSubclassOfSentient()) {
        if (m_iTeam != pOwner->m_Team) {
            return false;
        }
    }

    return true;
}

Event EV_Explosion_Radius
(
    "radius",
    EV_DEFAULT,
    "f",
    "projectileRadius",
    "set the radius for the explosion",
    EV_NORMAL
);
Event EV_Explosion_ConstantDamage
(
    "constantdamage",
    EV_DEFAULT,
    NULL,
    NULL,
    "Makes the explosion do constant damage over the radius",
    EV_NORMAL
);
Event EV_Explosion_DamageEveryFrame
(
    "damageeveryframe",
    EV_DEFAULT,
    NULL,
    NULL,
    "Makes the explosion damage every frame",
    EV_NORMAL
);
Event EV_Explosion_DamageAgain
(
    "damageagain",
    EV_DEFAULT,
    NULL,
    NULL,
    "This event is generated each frame if explosion is set to damage each frame",
    EV_NORMAL
);
Event EV_Explosion_Flash
(
    "flash",
    EV_DEFAULT,
    "fffff",
    "time r g b radius",
    "Flash player screens",
    EV_NORMAL
);
Event EV_Explosion_RadiusDamage
(
    "radiusdamage",
    EV_DEFAULT,
    "f",
    "radiusDamage",
    "set the radius damage an explosion does",
    EV_NORMAL
);
Event EV_Explosion_Effect
(
    "explosioneffect",
    EV_DEFAULT,
    "s",
    "explosiontype",
    "Make an explosionType explosion effect",
    EV_NORMAL
);

CLASS_DECLARATION(Projectile, Explosion, NULL) {
    {&EV_Explosion_Radius,           &Explosion::SetRadius          },
    {&EV_Explosion_ConstantDamage,   &Explosion::SetConstantDamage  },
    {&EV_Explosion_DamageEveryFrame, &Explosion::SetDamageEveryFrame},
    {&EV_Explosion_DamageAgain,      &Explosion::DamageAgain        },
    {&EV_Explosion_Flash,            &Explosion::SetFlash           },
    {&EV_Explosion_RadiusDamage,     &Explosion::SetRadiusDamage    },
    {&EV_Explosion_Effect,           &Explosion::MakeExplosionEffect},
    {NULL,                           NULL                           }
};

Explosion::Explosion()
{
    if (LoadingSavegame) {
        // Archive function will setup all necessary data
        return;
    }

    radius             = 0;
    constant_damage    = false;
    damage_every_frame = false;
    flash_time         = 0;
    flash_type         = 0;
    owner              = ENTITYNUM_NONE;
    hurtOwnerOnly      = false;
}

void Explosion::SetFlash(Event *ev)
{
    flash_time   = ev->GetFloat(1);
    flash_r      = ev->GetFloat(2);
    flash_g      = ev->GetFloat(3);
    flash_b      = ev->GetFloat(4);
    flash_a      = ev->GetFloat(5);
    flash_radius = ev->GetFloat(6);
    flash_type   = 0;

    if (ev->NumArgs() > 6) {
        str t = ev->GetString(7);

        if (!t.icmp("addblend")) {
            flash_type = 1;
        } else if (!t.icmp("alphablend")) {
            flash_type = 0;
        }
    }
}

void Explosion::SetRadius(Event *ev)
{
    radius = ev->GetFloat(1);
}

void Explosion::SetRadiusDamage(Event *ev)
{
    radius_damage = ev->GetFloat(1);
}

void Explosion::SetConstantDamage(Event *ev)
{
    constant_damage = true;
}

void Explosion::SetDamageEveryFrame(Event *ev)
{
    damage_every_frame = true;
}

void Explosion::DamageAgain(Event *ev)
{
    Entity *owner_ent;
    Weapon *weapon;
    float   dmg;
    float   rad;

    if (!CheckTeams()) {
        PostEvent(EV_Remove, EV_REMOVE);
        return;
    }

    owner_ent = G_GetEntity(owner);

    if (!owner_ent) {
        owner_ent = world;
    }

    weapon = weap;

    if (owner_ent == world || owner_ent->IsDead()) {
        weap = NULL;
    }

    dmg = radius_damage;
    rad = radius;

    if (rad == 0.0f) {
        rad = radius_damage + 60.0f;
    }

    RadiusDamage(origin, this, owner_ent, dmg, NULL, meansofdeath, rad, knockback, constant_damage, weapon, false);

    PostEvent(EV_Explosion_DamageAgain, level.frametime);
}

void Explosion::MakeExplosionEffect(Event *ev)
{
    str sEffect = ev->GetString(1);

    gi.SetBroadcastAll();

    if (!sEffect.icmp("grenade")) {
        gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_EXPLOSION_EFFECT_1));
    } else {
        if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
            if (!sEffect.icmp("heavyshell")) {
                gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_EXPLOSION_EFFECT_3));
            } else if (!sEffect.icmp("tank")) {
                gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_EXPLOSION_EFFECT_4));
            } else {
                gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_EXPLOSION_EFFECT_2));
            }
        } else {
            gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_EXPLOSION_EFFECT_2));
        }
    }

    gi.MSG_WriteCoord(origin[0]);
    gi.MSG_WriteCoord(origin[1]);
    gi.MSG_WriteCoord(origin[2]);
    gi.MSG_EndCGM();
}

Entity *FindDefusableObject(const Vector& dir, Entity *owner, float maxdist)
{
    Vector startOrg;
    Vector endOrg;
    Vector mins, maxs;
    float  fCos;
    int    numAreaEntities;
    int    i;
    int    entNums[MAX_GENTITIES];

    startOrg = owner->origin;
    endOrg   = startOrg + dir * maxdist;

    if (startOrg.z > endOrg.z) {
        endOrg.z -= 10;
        startOrg.z += 10;
    } else {
        startOrg.z -= 10;
        endOrg.z += 10;
    }

    fCos = cos(M_PI / 3);

    for (i = 0; i < 3; i++) {
        if (endOrg[i] > startOrg[i]) {
            mins[i] = startOrg[i] - 1;
            maxs[i] = endOrg[i] + 1;
        } else {
            mins[i] = endOrg[i] - 1;
            maxs[i] = startOrg[i] + 1;
        }
    }

    if (endOrg.x > startOrg.x) {
        mins[0] = startOrg.x - 1;
        maxs[0] = endOrg.x + 1;
    } else {
        mins[0] = endOrg.x - 1;
        maxs[0] = startOrg.x + 1;
    }

    numAreaEntities = gi.AreaEntities(mins, maxs, entNums, ARRAY_LEN(entNums));
    for (i = 0; i < numAreaEntities; i++) {
        gentity_t *pgEnt = &g_entities[entNums[i]];
        Entity    *pEnt;
        vec3_t     delta;

        if (!pgEnt->solid) {
            continue;
        }

        pEnt = pgEnt->entity;

        VectorSubtract(pEnt->centroid, startOrg, delta);
        VectorNormalize(delta);

        if (pEnt == owner) {
            continue;
        }

        if (DotProduct(delta, dir) > fCos && pEnt->Vars()->VariableExists("defuseThread")) {
            return pEnt;
        }
    }

    return NULL;
}

void DefuseObject(const Vector& dir, Entity *owner, float maxdist)
{
    Entity         *defusableObj = FindDefusableObject(dir, owner, maxdist);
    ScriptVariable *defuseThreadVar;
    str             defuseThreadName;

    if (!defusableObj) {
        return;
    }

    defuseThreadVar = defusableObj->Vars()->GetVariable("defuseThread");
    if (defuseThreadVar) {
        defuseThreadName = defuseThreadVar->stringValue();
    }

    ScriptThreadLabel label;
    label.Set(defuseThreadName);
    label.Execute(defusableObj, ListenerPtr());
}

qboolean CanPlaceLandmine(const Vector& origin, Entity *owner)
{
    Vector  vEnd;
    Vector  vDelta;
    trace_t trace;

    if (!level.RoundStarted()) {
        gi.DPrintf("Can't place landmine until the round starts\n");
        return qfalse;
    }

    vEnd = origin - Vector(0, 0, 256);

    trace = G_Trace(origin, vec_zero, vec_zero, vEnd, owner, MASK_LANDMINE_PLACE, qfalse, "LandminePlace", qtrue);

    vDelta = origin - trace.endpos;
    if (vDelta.length() > 90) {
        gi.DPrintf("Too high to place landmine\n");
        return qfalse;
    }

    if (trace.surfaceFlags & SURF_WOOD) {
        gi.DPrintf("Can't place landmine on wood\n");
        return qfalse;
    } else if (trace.surfaceFlags & SURF_METAL) {
        gi.DPrintf("Can't place landmine on metal\n");
        return qfalse;
    } else if (trace.surfaceFlags & SURF_ROCK) {
        gi.DPrintf("Can't place landmine on rock\n");
        return qfalse;
    } else if (trace.surfaceFlags & SURF_GRILL) {
        gi.DPrintf("Can't place landmine on grill\n");
        return qfalse;
    } else if (trace.surfaceFlags & SURF_GLASS) {
        gi.DPrintf("Can't place landmine on glass\n");
        return qfalse;
    } else if (trace.surfaceFlags & SURF_CARPET) {
        gi.DPrintf("Can't place landmine on carpet\n");
        return qfalse;
    }

    if (!(trace.contents & CONTENTS_SOLID)) {
        gi.DPrintf("Can't place landmine on nonsolid things\n");
        return qfalse;
    }

    return qtrue;
}

void PlaceLandmine(const Vector& origin, Entity *owner, const str& model, Weapon *weap)
{
    SpawnArgs        args;
    Listener        *l;
    TriggerLandmine *trigger;

    if (!model.length()) {
        gi.DPrintf("PlaceLandmine : No model specified for PlaceLandmine");
        return;
    }

    args.setArg("model", model.c_str());
    args.setArg("setthread", "global/landmine.scr::steppedOn");
    args.setArg("$defuseThread", "global/landmine.scr::defuse");
    args.setArg("targetname", "landmine");

    l = args.Spawn();
    if (!l) {
        gi.DPrintf("PlaceLandmine model '%s' not found\n", model.c_str());
        return;
    }

    if (!l->inheritsFrom(&TriggerLandmine::ClassInfo)) {
        gi.DPrintf("%s is not of class trigger_landmine\n", model.c_str());
        return;
    }

    trigger = static_cast<TriggerLandmine *>(l);
    trigger->droptofloor(256);
    trigger->ProcessInitCommands();
    trigger->SetDamageable(qtrue);
    if (owner) {
        trigger->edict->r.ownerNum = owner->entnum;
    }

    trigger->setOrigin(origin);

    if (owner->inheritsFrom(&Player::ClassInfo)) {
        if (g_gametype->integer >= GT_TEAM) {
            Player *p = static_cast<Player *>(owner);
            trigger->SetTeam(p->GetTeam());
        } else {
            trigger->SetTeam(0);
        }
    }

    trigger->NewAnim("idle");

    if (g_gametype->integer == GT_SINGLE_PLAYER) {
        if (owner) {
            if (owner->IsDead()) {
                weap = NULL;
            }
        } else {
            weap = NULL;
        }

        if (weap) {
            weap->m_iNumShotsFired++;

            if (owner) {
                if (owner->IsSubclassOfPlayer() && weap->IsSubclassOfTurretGun()) {
                    TurretGun *turret = static_cast<TurretGun *>(weap);
                    // FIXME: find what to increment
                }
            }
        }
    }
}

Projectile *ProjectileAttack(
    Vector start, Vector dir, Entity *owner, str projectileModel, float fraction, float real_speed, Weapon *weap
)
{
    Event      *ev;
    Projectile *proj = NULL;
    float       newspeed, newlife;
    SpawnArgs   args;
    Entity     *obj;
    float       dot = 0;

    if (!projectileModel.length()) {
        gi.DPrintf("ProjectileAttack : No model specified for ProjectileAttack");
        return NULL;
    }

    args.setArg("model", projectileModel);
    obj = (Entity *)args.Spawn();

    if (!obj) {
        gi.DPrintf("projectile model '%s' not found\n", projectileModel.c_str());
        return NULL;
    }

    if (obj->isSubclassOf(Projectile)) {
        proj = (Projectile *)obj;
    } else {
        gi.DPrintf("%s is not of class projectile\n", projectileModel.c_str());
    }

    if (!proj) {
        return NULL;
    }

    // Create a new projectile entity and set it off
    proj->setMoveType(MOVETYPE_BOUNCE);
    proj->ProcessInitCommands();
    proj->SetOwner(owner);
    proj->edict->r.ownerNum = owner->entnum;
    proj->angles            = dir.toAngles();
    proj->charge_fraction   = fraction;

    if (!real_speed) {
        if (proj->projFlags & P_CHARGE_SPEED) {
            newspeed = proj->minspeed + (proj->speed - proj->minspeed) * fraction;
        } else {
            newspeed = proj->speed;
        }
    } else {
        newspeed = real_speed;
    }

    if (proj->addownervelocity) {
        dot = DotProduct(owner->velocity, dir);
        if (dot < 0) {
            dot = 0;
        }
    }

    proj->velocity = dir * (newspeed + dot);
    proj->velocity += proj->addvelocity;
    proj->setAngles(proj->angles);
    proj->setSolidType(SOLID_BBOX);

    proj->edict->clipmask = MASK_PROJECTILE;

    proj->setSize(proj->mins, proj->maxs);
    proj->setOrigin(start);
    proj->origin.copyTo(proj->edict->s.origin2);

    if (proj->m_beam) {
        proj->m_beam->setOrigin(start);
        proj->m_beam->origin.copyTo(proj->m_beam->edict->s.origin2);
    }

    if (proj->dlight_radius) {
        G_SetConstantLight(
            &proj->edict->s.constantLight,
            &proj->dlight_color[0],
            &proj->dlight_color[1],
            &proj->dlight_color[2],
            &proj->dlight_radius
        );
    }

    // Calc the life of the projectile
    if (proj->projFlags & P_CHARGE_LIFE) {
        if (g_gametype->integer != GT_SINGLE_PLAYER && proj->dmlife) {
            newlife = proj->dmlife * (1 - fraction);
        } else {
            newlife = proj->life * (1 - fraction);
        }

        if (newlife < proj->minlife) {
            newlife = proj->minlife;
        }
    } else {
        if (g_gametype->integer != GT_SINGLE_PLAYER && proj->dmlife) {
            newlife = proj->dmlife;
        } else {
            newlife = proj->life;
        }
    }

    // Remove the projectile after it's life expires
    ev = new Event(EV_Projectile_Explode);
    proj->PostEvent(ev, newlife);

    proj->NewAnim("idle");

    // If can hit owner clear the owner of this projectile in a second

    if (proj->can_hit_owner) {
        proj->PostEvent(EV_Projectile_ClearOwner, 1);
    }

    if (owner) {
        if (owner->IsDead() || owner == world) {
            // clear the weapon as the owner died
            weap = NULL;
        }
    } else {
        // clear the weapon as there is no owner
        weap = NULL;
    }

    if (g_gametype->integer == GT_SINGLE_PLAYER) {
        if (weap) {
            weap->m_iNumShotsFired++;
            if (owner->IsSubclassOfPlayer() && weap->IsSubclassOfTurretGun()) {
                Player *p = (Player *)owner;
                p->m_iNumShotsFired++;
            }
        }
    }

    return proj;
}

void BulletAttack_Stat(Entity *owner, Entity *target, trace_t *trace, Weapon *weap)
{
    Sentient *targetSen;

    if (!target->IsSubclassOfSentient() || !weap) {
        return;
    }

    targetSen                 = static_cast<Sentient *>(target);
    targetSen->m_iLastHitTime = level.inttime;
    weap->m_iNumHits++;

    switch (trace->location) {
    case HITLOC_HEAD:
    case HITLOC_HELMET:
    case HITLOC_NECK:
        weap->m_iNumHeadShots++;
        break;
    case HITLOC_TORSO_UPPER:
    case HITLOC_TORSO_MID:
    case HITLOC_TORSO_LOWER:
        weap->m_iNumTorsoShots++;
        break;
    case HITLOC_PELVIS:
        weap->m_iNumGroinShots++;
        break;
    case HITLOC_R_ARM_UPPER:
    case HITLOC_R_ARM_LOWER:
    case HITLOC_R_HAND:
        weap->m_iNumRightArmShots++;
        break;
    case HITLOC_L_ARM_UPPER:
    case HITLOC_L_ARM_LOWER:
    case HITLOC_L_HAND:
        weap->m_iNumLeftArmShots++;
        break;
    case HITLOC_R_LEG_UPPER:
    case HITLOC_R_LEG_LOWER:
    case HITLOC_R_FOOT:
        weap->m_iNumRightLegShots++;
        break;
    case HITLOC_L_LEG_UPPER:
    case HITLOC_L_LEG_LOWER:
    case HITLOC_L_FOOT:
        weap->m_iNumLeftLegShots++;
        break;
    default:
        weap->m_iNumTorsoShots++;
        break;
    }

    if (owner && owner->IsSubclassOfPlayer() && weap->IsSubclassOfTurretGun()) {
        Player *p = static_cast<Player *>(owner);

        p->m_iNumHits++;

        switch (trace->location) {
        case HITLOC_HEAD:
        case HITLOC_HELMET:
        case HITLOC_NECK:
            p->m_iNumHeadShots++;
            break;
        case HITLOC_TORSO_UPPER:
        case HITLOC_TORSO_MID:
        case HITLOC_TORSO_LOWER:
            p->m_iNumTorsoShots++;
            break;
        case HITLOC_PELVIS:
            p->m_iNumGroinShots++;
            break;
        case HITLOC_R_ARM_UPPER:
        case HITLOC_R_ARM_LOWER:
        case HITLOC_R_HAND:
            p->m_iNumRightArmShots++;
            break;
        case HITLOC_L_ARM_UPPER:
        case HITLOC_L_ARM_LOWER:
        case HITLOC_L_HAND:
            p->m_iNumLeftArmShots++;
            break;
        case HITLOC_R_LEG_UPPER:
        case HITLOC_R_LEG_LOWER:
        case HITLOC_R_FOOT:
            p->m_iNumRightLegShots++;
            break;
        case HITLOC_L_LEG_UPPER:
        case HITLOC_L_LEG_LOWER:
        case HITLOC_L_FOOT:
            p->m_iNumLeftLegShots++;
            break;
        default:
            p->m_iNumTorsoShots++;
            break;
        }
    }
}

float BulletAttack(
    Vector  start,
    Vector  vBarrel,
    Vector  dir,
    Vector  right,
    Vector  up,
    float   range,
    float   damage,
    int     bulletlarge,
    float   knockback,
    int     dflags,
    int     meansofdeath,
    Vector  spread,
    int     count,
    Entity *owner,
    int     iTracerFrequency,
    int    *piTracerCount,
    float   bulletthroughwood,
    float   bulletthroughmetal,
    Weapon *weap,
    float   tracerspeed
)
{
    Vector  vDir;
    Vector  vTmpEnd;
    Vector  vTraceStart;
    Vector  vTraceEnd;
    int     i;
    int     iTravelDist;
    trace_t trace;
    Entity *ent;
    Entity *newowner;
    //Entity        *tmpSkipEnt;
    float    damage_total = 0;
    float    original_value;
    qboolean bBulletDone;
    qboolean bThroughThing;
    int      iContinueCount;
    float    vEndArray[64][3];
    int      iTracerCount = 0;
    int      iNumHit;
    int      lastSurfaceFlags;
    float    bulletdist;
    float    newdamage;
    float    throughThingFrac;
    float    oldfrac;
    int      bulletbits;

    lastSurfaceFlags = 0;
    iNumHit          = 0;

    if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
        bulletbits = 2;
    } else {
        bulletlarge = damage >= 41.f;
        bulletbits  = 1;
    }

    if (count > 63) {
        count = 63;
    }

    if (!owner || owner->IsDead() || owner == world) {
        weap = NULL;
    }

    for (i = 0; i < count; i++) {
        trace_t tracethrough;

        vTraceEnd = start + (dir * range) + (right * grandom() * spread.x) + (up * grandom() * spread.y);

        vDir = vTraceEnd - start;

        VectorNormalizeFast(vDir);

        iContinueCount = 0;
        iTravelDist    = 0;
        bBulletDone    = qfalse;
        bThroughThing  = qfalse;
        newowner       = owner;
        newdamage      = damage;

        while (!bBulletDone && iTravelDist < MAX_TRAVEL_DIST) {
            iTravelDist += MAX_TRAVEL_DIST;
            vTraceStart = start;
            vTraceEnd   = start + vDir * iTravelDist;

            memset(&trace, 0, sizeof(trace_t));

            oldfrac = -1;

            while (trace.fraction < 1.0f) {
                trace = G_Trace(
                    vTraceStart, vec_zero, vec_zero, vTraceEnd, newowner, MASK_SHOT_TRIG, false, "BulletAttack", true
                );

                vTmpEnd = trace.endpos;

                if (bThroughThing) {
                    bThroughThing = qfalse;

                    tracethrough = G_Trace(
                        vTmpEnd,
                        vec_zero,
                        vec_zero,
                        vTraceStart + vDir * -4,
                        newowner,
                        MASK_SHOT,
                        qfalse,
                        "BulletAttack2",
                        qtrue
                    );

                    if (!(tracethrough.surfaceFlags & (SURF_FOLIAGE | SURF_GLASS | SURF_PUDDLE | SURF_PAPER))
                        && (!(tracethrough.surfaceFlags & SURF_WOOD) || !bulletthroughwood)
                        && (!(tracethrough.surfaceFlags & (SURF_GRILL | SURF_METAL)) || !bulletthroughmetal)) {
                        vTmpEnd        = vTraceStart + vDir * -4;
                        trace.fraction = 1.f;
                        bBulletDone    = qtrue;

                        if (g_showbullettrace->integer) {
                            bThroughThing = qtrue;
                        }
                        break;
                    }

                    if (lastSurfaceFlags & SURF_WOOD) {
                        if (tracethrough.surfaceFlags & SURF_WOOD) {
                            throughThingFrac = 1.f / bulletthroughwood;
                        } else {
                            throughThingFrac = 2.f / (bulletthroughwood + bulletthroughmetal);
                        }
                    } else {
                        if (tracethrough.surfaceFlags & SURF_WOOD) {
                            throughThingFrac = 2.f / (bulletthroughwood + bulletthroughmetal);
                        } else {
                            throughThingFrac = 1.f / bulletthroughmetal;
                        }
                    }

                    bulletdist = (tracethrough.endpos - vTraceStart).length() + 4.f;

                    if (g_showbullettrace->integer) {
                        gi.Printf("Bullet damage: %.2f : %.2f -> ", bulletdist, damage);
                    }

                    newdamage -= damage * bulletdist * throughThingFrac;
                    if (newdamage < 1.f) {
                        vTmpEnd        = vTraceStart + vDir * -4;
                        trace.fraction = 1.f;
                        bBulletDone    = qtrue;

                        if (g_showbullettrace->integer) {
                            bThroughThing = qtrue;
                        }
                        break;
                    }

                    if (g_showbullettrace->integer) {
                        G_DebugLine(
                            tracethrough.endpos + Vector(8, 0, 0),
                            tracethrough.endpos - Vector(8, 0, 0),
                            0.5f,
                            0.5f,
                            1.f,
                            1.f
                        );
                        G_DebugLine(
                            tracethrough.endpos + Vector(0, 8, 0),
                            tracethrough.endpos - Vector(0, 8, 0),
                            0.5f,
                            0.5f,
                            1.f,
                            1.f
                        );
                        G_DebugLine(
                            tracethrough.endpos + Vector(0, 0, 8),
                            tracethrough.endpos - Vector(0, 0, 8),
                            0.5f,
                            0.5f,
                            1.f,
                            1.f
                        );
                    }
                }

                if (trace.ent) {
                    ent = trace.ent->entity;
                } else {
                    ent = NULL;
                }

                if (ent && ent != world && ent != newowner) {
                    if (ent->takedamage) {
                        if (g_gametype->integer == GT_SINGLE_PLAYER && !iNumHit) {
                            BulletAttack_Stat(newowner, ent, &trace, weap);
                        }

                        iNumHit++;

                        // Get the original value of the victims health or water

                        original_value = ent->health;

                        ent->Damage(
                            world,
                            newowner,
                            newdamage,
                            trace.endpos,
                            dir,
                            trace.plane.normal,
                            knockback,
                            dflags,
                            meansofdeath,
                            trace.location
                        );

                        // Get the new value of the victims health or water

                        damage_total += original_value - ent->health;
                    }

                    if (ent->edict->solid == SOLID_BBOX && !(trace.contents & CONTENTS_CLAYPIDGEON)) {
                        if (trace.surfaceFlags & MASK_SURF_TYPE) {
                            gi.SetBroadcastVisible(vTmpEnd, NULL);
                            gi.MSG_StartCGM(CGM_BULLET_6);
                            gi.MSG_WriteCoord(vTmpEnd[0]);
                            gi.MSG_WriteCoord(vTmpEnd[1]);
                            gi.MSG_WriteCoord(vTmpEnd[2]);
                            gi.MSG_WriteDir(trace.plane.normal);
                            gi.MSG_WriteBits(bulletlarge, bulletbits);
                            gi.MSG_EndCGM();
                        } else if (trace.location >= 0 && ent->IsSubclassOfPlayer()) {
                            gi.SetBroadcastVisible(vTmpEnd, NULL);
                            gi.MSG_StartCGM(CGM_BULLET_8);
                            gi.MSG_WriteCoord(vTmpEnd[0]);
                            gi.MSG_WriteCoord(vTmpEnd[1]);
                            gi.MSG_WriteCoord(vTmpEnd[2]);
                            gi.MSG_WriteDir(trace.plane.normal);
                            gi.MSG_WriteBits(bulletlarge, bulletbits);
                            gi.MSG_EndCGM();
                        } else if (ent->edict->r.contents & CONTENTS_SOLID) {
                            gi.SetBroadcastVisible(vTmpEnd, NULL);
                            gi.MSG_StartCGM(CGM_BULLET_7);
                            gi.MSG_WriteCoord(vTmpEnd[0]);
                            gi.MSG_WriteCoord(vTmpEnd[1]);
                            gi.MSG_WriteCoord(vTmpEnd[2]);
                            gi.MSG_WriteDir(trace.plane.normal);
                            gi.MSG_WriteBits(bulletlarge, bulletbits);
                            gi.MSG_EndCGM();
                        }
                    } else if (ent->edict->solid == SOLID_BSP && !(trace.contents & CONTENTS_CLAYPIDGEON)) {
                        gi.SetBroadcastVisible(vTmpEnd, NULL);
                        gi.MSG_StartCGM(CGM_BULLET_6);
                        gi.MSG_WriteCoord(vTmpEnd[0]);
                        gi.MSG_WriteCoord(vTmpEnd[1]);
                        gi.MSG_WriteCoord(vTmpEnd[2]);
                        gi.MSG_WriteDir(trace.plane.normal);
                        gi.MSG_WriteBits(bulletlarge, bulletbits);
                        gi.MSG_EndCGM();
                    }
                }

                if (trace.fraction < 1.0f) {
                    if (trace.surfaceFlags & (SURF_FOLIAGE | SURF_GLASS | SURF_PUDDLE | SURF_PAPER)
                        || trace.contents & (CONTENTS_CLAYPIDGEON | CONTENTS_WATER)
                        || (bulletlarge && trace.ent && trace.ent->r.contents & CONTENTS_BBOX && !trace.ent->r.bmodel
                            && trace.ent->entity->takedamage)
                        || ((trace.surfaceFlags & SURF_WOOD) && bulletthroughwood)
                        || ((trace.surfaceFlags & (SURF_GRILL | SURF_METAL)) && bulletthroughmetal)
                               && iContinueCount < 5) {
                        if (((trace.surfaceFlags & SURF_WOOD) && bulletthroughwood)
                            || ((trace.surfaceFlags & (SURF_GRILL | SURF_METAL)) && bulletthroughmetal)) {
                            if (trace.contents & CONTENTS_FENCE) {
                                float damageMultiplier;

                                if (lastSurfaceFlags & SURF_WOOD) {
                                    damageMultiplier = 1.f / bulletthroughwood;
                                } else {
                                    damageMultiplier = 1.f / bulletthroughmetal;
                                }

                                newdamage -= damageMultiplier * 2 * damage;

                                if (newdamage < 0) {
                                    trace.fraction = 1;
                                    bBulletDone    = qtrue;

                                    if (g_showbullettrace->integer) {
                                        bThroughThing = qtrue;
                                        VectorScale(vDir, 2, tracethrough.endpos);
                                        VectorAdd(tracethrough.endpos, vTmpEnd, tracethrough.endpos);
                                    }
                                } else {
                                    trace.fraction = 1.f;
                                    bBulletDone    = qtrue;

                                    if (g_showbullettrace->integer) {
                                        bThroughThing = qtrue;
                                        VectorScale(vDir, 2, tracethrough.endpos);
                                        VectorAdd(tracethrough.endpos, vTmpEnd, tracethrough.endpos);
                                    }
                                }
                            } else {
                                bThroughThing    = qtrue;
                                lastSurfaceFlags = trace.surfaceFlags;
                            }
                        }

                        if (!bBulletDone) {
                            vTraceStart = vTmpEnd + vDir * 4;

                            if (trace.ent) {
                                newowner = trace.ent->entity;
                            } else {
                                newowner = NULL;
                            }

                            if (g_showbullettrace->integer) {
                                G_DebugLine(vTmpEnd + Vector(8, 0, 0), vTmpEnd - Vector(8, 0, 0), 1, 0.5f, 0.5f, 1.f);
                                G_DebugLine(vTmpEnd + Vector(0, 8, 0), vTmpEnd - Vector(0, 8, 0), 1, 0.5f, 0.5f, 1.f);
                                G_DebugLine(vTmpEnd + Vector(0, 0, 8), vTmpEnd - Vector(0, 0, 8), 1, 0.5f, 0.5f, 1.f);
                            }

                            iContinueCount++;
                        }
                    } else {
                        trace.fraction = 1.f;
                        bBulletDone    = qtrue;
                    }

                    if (oldfrac != trace.fraction) {
                        oldfrac = trace.fraction;
                    } else {
                        trace.fraction = 1.f;
                    }
                }
            }
        }

        if (bBulletDone && g_showbullettrace->integer && bThroughThing) {
            G_DebugLine(
                tracethrough.endpos + Vector(8, 0, 0), tracethrough.endpos - Vector(8, 0, 0), 0.25f, 0.25f, 0.5f, 1.f
            );
            G_DebugLine(
                tracethrough.endpos + Vector(0, 8, 0), tracethrough.endpos - Vector(0, 8, 0), 0.25f, 0.25f, 0.5f, 1.f
            );
            G_DebugLine(
                tracethrough.endpos + Vector(0, 0, 8), tracethrough.endpos - Vector(0, 0, 8), 0.25f, 0.25f, 0.5f, 1.f
            );
        }

        VectorCopy(vTraceEnd, vEndArray[i]);

        if (iTracerFrequency && piTracerCount) {
            (*piTracerCount)++;

            if (*piTracerCount == iTracerFrequency) {
                iTracerCount++;
                *piTracerCount = 0;
            }
        }

        // Draw a debug trace line to show bullet fire
        if (g_showbullettrace->integer) {
            G_DebugLine(start, vTmpEnd, 1, 1, 1, 1);
            G_DebugLine(vTmpEnd + Vector(8, 0, 0), vTmpEnd - Vector(8, 0, 0), 0.5f, 0.25f, 0.25f, 1.f);
            G_DebugLine(vTmpEnd + Vector(0, 8, 0), vTmpEnd - Vector(0, 8, 0), 0.5f, 0.25f, 0.25f, 1.f);
            G_DebugLine(vTmpEnd + Vector(0, 0, 8), vTmpEnd - Vector(0, 0, 8), 0.5f, 0.25f, 0.25f, 1.f);
            G_DebugLine(vTmpEnd, vTraceEnd, 0.4f, 0.4f, 0.4f, 1.f);
        }
    }

    if (g_gametype->integer == GT_SINGLE_PLAYER && weap) {
        weap->m_iNumShotsFired++;
        if (owner && owner->IsSubclassOfPlayer() && weap->IsSubclassOfTurretGun()) {
            Player *p = (Player *)owner;
            p->m_iNumShotsFired++;
        }
    }

    gi.SetBroadcastVisible(start, trace.endpos);

    if (count == 1) {
        if (iTracerCount) {
            gi.MSG_StartCGM(CGM_BULLET_1);

            gi.MSG_WriteCoord(vBarrel[0]);
            gi.MSG_WriteCoord(vBarrel[1]);
            gi.MSG_WriteCoord(vBarrel[2]);
        } else {
            gi.MSG_StartCGM(CGM_BULLET_2);
        }

        gi.MSG_WriteCoord(start[0]);
        gi.MSG_WriteCoord(start[1]);
        gi.MSG_WriteCoord(start[2]);
        gi.MSG_WriteCoord(trace.endpos[0]);
        gi.MSG_WriteCoord(trace.endpos[1]);
        gi.MSG_WriteCoord(trace.endpos[2]);
        gi.MSG_WriteBits(bulletlarge, bulletbits);

        if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
            if (tracerspeed == 1.f) {
                gi.MSG_WriteBits(0, 1);
            } else {
                int speed;

                speed = tracerspeed * (1 << 9);

                gi.MSG_WriteBits(1, 1);
                gi.MSG_WriteBits(Q_clamp(speed, 1, 1023), 10);
            }
        }
    } else {
        if (iTracerCount) {
            gi.MSG_StartCGM(CGM_BULLET_3);

            gi.MSG_WriteCoord(vBarrel[0]);
            gi.MSG_WriteCoord(vBarrel[1]);
            gi.MSG_WriteCoord(vBarrel[2]);

            if (iTracerCount > 63) {
                iTracerCount = 63;
            }

            gi.MSG_WriteBits(Q_min(iTracerCount, 63), 6);
        } else {
            gi.MSG_StartCGM(CGM_BULLET_4);
        }

        gi.MSG_WriteCoord(start[0]);
        gi.MSG_WriteCoord(start[1]);
        gi.MSG_WriteCoord(start[2]);
        gi.MSG_WriteBits(bulletlarge, bulletbits);

        if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
            if (tracerspeed == 1.f) {
                gi.MSG_WriteBits(0, 1);
            } else {
                gi.MSG_WriteBits(1, 1);
                gi.MSG_WriteBits(Q_clamp(tracerspeed, 1, 1023), 10);
            }
        }

        gi.MSG_WriteBits(count, 6);

        for (int i = count; i > 0; i--) {
            gi.MSG_WriteCoord(vEndArray[i][0]);
            gi.MSG_WriteCoord(vEndArray[i][1]);
            gi.MSG_WriteCoord(vEndArray[i][2]);
        }
    }

    gi.MSG_EndCGM();

    if (damage_total > 0) {
        return damage_total;
    } else {
        return 0;
    }
}

void FakeBulletAttack(
    Vector  start,
    Vector  vBarrel,
    Vector  dir,
    Vector  right,
    Vector  up,
    float   range,
    float   damage,
    int     large,
    Vector  spread,
    int     count,
    Entity *owner,
    int     iTracerFrequency,
    int    *piTracerCount,
    float   tracerspeed
)
{
    Vector vDir;
    Vector vTraceEnd;
    int    i;
    int    bulletbits;

    if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
        bulletbits = 2;
    } else {
        large      = damage >= 41.f;
        bulletbits = 1;
    }

    for (i = 0; i < count; i++) {
        vTraceEnd = start + (dir * range) + (right * grandom() * spread.x) + (up * grandom() * spread.y);

        vDir = vTraceEnd - start;
        VectorNormalize(vDir);
        vTraceEnd = start + vDir * 9216.0f;

        gi.SetBroadcastVisible(start, vTraceEnd);
        if (iTracerFrequency && piTracerCount) {
            (*piTracerCount)++;
            if (*piTracerCount == iTracerFrequency) {
                gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_BULLET_NO_BARREL_1));
                gi.MSG_WriteCoord(vBarrel[0]);
                gi.MSG_WriteCoord(vBarrel[1]);
                gi.MSG_WriteCoord(vBarrel[2]);
                *piTracerCount = 0;
            } else {
                gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_BULLET_NO_BARREL_2));
            }
        } else {
            gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_BULLET_NO_BARREL_2));
        }

        gi.MSG_WriteCoord(start[0]);
        gi.MSG_WriteCoord(start[1]);
        gi.MSG_WriteCoord(start[2]);
        gi.MSG_WriteCoord(vTraceEnd[0]);
        gi.MSG_WriteCoord(vTraceEnd[1]);
        gi.MSG_WriteCoord(vTraceEnd[2]);
        gi.MSG_WriteBits(large, bulletbits);

        if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
            if (tracerspeed == 1.f) {
                gi.MSG_WriteBits(0, 1);
            } else {
                gi.MSG_WriteBits(1, 1);
                gi.MSG_WriteBits(Q_clamp(tracerspeed, 1, 1023), 10);
            }
        }

        gi.MSG_EndCGM();
    }
}

void ClickItemAttack(Vector vStart, Vector vForward, float fRange, Entity *pOwner)
{
    Vector  vEnd;
    trace_t trace;

    vEnd = vStart + vForward * fRange;

    trace = G_Trace(vStart, vec_zero, vec_zero, vEnd, pOwner, MASK_ALL, qfalse, "ClickItemAttack");

    if (g_showbullettrace->integer) {
        G_DebugLine(vStart, vEnd, 1, 1, 1, 1);
    }

    if (trace.entityNum != ENTITYNUM_NONE && trace.ent && trace.ent->entity
        && trace.ent->entity->isSubclassOf(TriggerClickItem)) {
        Event *ev = new Event(EV_Activate);
        ev->AddEntity(pOwner);
        trace.ent->entity->PostEvent(ev, 0);
    } else {
        ScriptThreadLabel failThread;

        // Try to execute a fail thread
        if (failThread.TrySet("clickitem_fail")) {
            failThread.Execute();
        }
    }
}

Projectile *HeavyAttack(Vector start, Vector dir, str projectileModel, float real_speed, Entity *owner, Weapon *weap)
{
    Event      *ev;
    Projectile *proj = NULL;
    float       newspeed, newlife;
    SpawnArgs   args;
    Entity     *obj;
    float       dot = 0;

    if (!projectileModel.length()) {
        gi.DPrintf("ProjectileAttack : No model specified for ProjectileAttack");
        return NULL;
    }

    args.setArg("model", projectileModel);
    obj = (Entity *)args.Spawn();

    if (!obj) {
        gi.DPrintf("projectile model '%s' not found\n", projectileModel.c_str());
        return NULL;
    }

    if (obj->IsSubclassOfProjectile()) {
        proj = (Projectile *)obj;
    } else {
        gi.DPrintf("%s is not of class projectile\n", projectileModel.c_str());
    }

    if (!proj) {
        return NULL;
    }

    // Create a new projectile entity and set it off
    proj->setModel(projectileModel);
    proj->setMoveType(MOVETYPE_BOUNCE);
    proj->ProcessInitCommands();
    proj->SetOwner(owner);
    proj->edict->r.ownerNum = owner->entnum;
    proj->angles            = dir.toAngles();
    proj->charge_fraction   = 1.0f;

    newspeed = real_speed;
    if (real_speed == 0.0f) {
        newspeed = proj->speed;
    }

    if (proj->addownervelocity) {
        dot = DotProduct(owner->velocity, dir);
        if (dot < 0) {
            dot = 0;
        }
    }

    proj->velocity = dir * (newspeed + dot);
    proj->velocity += proj->addvelocity;
    proj->setAngles(proj->angles);
    proj->setSolidType(SOLID_BBOX);

    proj->edict->clipmask = MASK_PROJECTILE;

    proj->setSize(proj->mins, proj->maxs);
    proj->setOrigin(start);
    proj->origin.copyTo(proj->edict->s.origin2);

    if (proj->m_beam) {
        proj->m_beam->setOrigin(start);
        proj->m_beam->origin.copyTo(proj->m_beam->edict->s.origin2);
    }

    if (proj->dlight_radius) {
        G_SetConstantLight(
            &proj->edict->s.constantLight,
            &proj->dlight_color[0],
            &proj->dlight_color[1],
            &proj->dlight_color[2],
            &proj->dlight_radius
        );
    }

    // Calc the life of the projectile
    if (proj->projFlags & P_CHARGE_LIFE) {
        if (g_gametype->integer && proj->dmlife) {
            newlife = proj->dmlife;
        } else {
            newlife = proj->life;
        }

        if (newlife < proj->minlife) {
            newlife = proj->minlife;
        }
    } else {
        if (g_gametype->integer && proj->dmlife) {
            newlife = proj->dmlife;
        } else {
            newlife = proj->life;
        }
    }

    // Remove the projectile after it's life expires
    ev = new Event(EV_Projectile_Explode);
    proj->PostEvent(ev, newlife);

    proj->NewAnim("idle");

    if (!g_gametype->integer) {
        if (weap) {
            weap->m_iNumShotsFired++;
            if (owner->IsSubclassOfPlayer() && weap->IsSubclassOfTurretGun()) {
                Player *p = (Player *)owner;
                p->m_iNumShotsFired++;
            }
        }
    }

    return proj;
}

void ExplosionAttack(
    Vector  pos,
    Entity *owner,
    str     explosionModel,
    Vector  dir,
    Entity *ignore,
    float   scale,
    Weapon *weap,
    bool    hurtOwnerOnly
)
{
    Explosion *explosion;
    Event     *ev;

    if (!owner) {
        owner = world;
    }

    if (owner->IsDead() || owner == world) {
        weap = NULL;
    }

    if (explosionModel.length() && gi.modeltiki(CanonicalTikiName(explosionModel))) {
        SpawnArgs sp;
        ClassDef *c;

        sp.setArg("model", explosionModel);
        c = sp.getClassDef();
        if (c && c != Explosion::classinfostatic() && checkInheritance(Entity::classinfostatic(), c)) {
            Entity *newent = static_cast<Entity *>(c->newInstance());

            newent->setModel(explosionModel);
            newent->setSolidType(SOLID_NOT);
            newent->setOrigin(pos);
            newent->setAngles(dir.toAngles());
            newent->ProcessInitCommands();

            if (c == SmokeGrenade::classinfostatic()) {
                //
                // try to set the owner of the smoke grenade
                //
                SmokeGrenade *smoke = static_cast<SmokeGrenade *>(newent);

                if (owner && owner->IsSubclassOfSentient()) {
                    smoke->setOwner(static_cast<Sentient *>(owner));
                }
            }
        } else {
            explosion = new Explosion;

            // Create a new explosion entity and set it off
            explosion->setModel(explosionModel);

            explosion->setSolidType(SOLID_NOT);

            // Process the INIT commands right away
            explosion->ProcessInitCommands();

            explosion->SetOwner(owner);

            explosion->edict->r.ownerNum = owner->entnum;
            explosion->angles            = dir.toAngles();
            explosion->velocity          = dir * explosion->speed;
            explosion->edict->s.scale    = scale;
            explosion->setAngles(explosion->angles);
            explosion->setMoveType(MOVETYPE_FLYMISSILE);
            explosion->edict->clipmask = MASK_PROJECTILE;
            explosion->setSize(explosion->mins, explosion->maxs);
            explosion->setOrigin(pos);
            explosion->origin.copyTo(explosion->edict->s.origin2);
            explosion->hurtOwnerOnly = hurtOwnerOnly;

            if (explosion->dlight_radius) {
                G_SetConstantLight(
                    &explosion->edict->s.constantLight,
                    &explosion->dlight_color[0],
                    &explosion->dlight_color[1],
                    &explosion->dlight_color[2],
                    &explosion->dlight_radius
                );
            }

            explosion->BroadcastAIEvent(AI_EVENT_WEAPON_FIRE);
            explosion->NewAnim("idle");

            RadiusDamage(
                explosion->origin,
                explosion,
                owner,
                explosion->radius_damage * scale,
                ignore,
                explosion->meansofdeath,
                explosion->radius * scale,
                explosion->knockback,
                explosion->constant_damage,
                weap,
                explosion->hurtOwnerOnly
            );

            if (explosion->flash_radius) {
                FlashPlayers(
                    explosion->origin,
                    explosion->flash_r,
                    explosion->flash_g,
                    explosion->flash_b,
                    explosion->flash_a,
                    explosion->flash_radius * scale,
                    explosion->flash_time,
                    explosion->flash_type
                );
            }

            if (explosion->damage_every_frame) {
                explosion->PostEvent(EV_Explosion_DamageAgain, FRAMETIME);
            }

            // Remove explosion after the life has expired
            if (explosion->life || (g_gametype->integer && explosion->dmlife)) {
                ev = new Event(EV_Remove);

                if (g_gametype->integer && explosion->dmlife) {
                    explosion->PostEvent(ev, explosion->dmlife);
                } else {
                    explosion->PostEvent(ev, explosion->life);
                }
            }
        }
    }
}

static float radiusdamage_origin[3];

static int radiusdamage_compare(const void *elem1, const void *elem2)
{
    Entity *e1, *e2;
    float   delta[3];
    float   d1, d2;

    e1 = *(Entity **)elem1;
    e2 = *(Entity **)elem2;

    VectorSubtract(radiusdamage_origin, e1->origin, delta);
    d1 = VectorLengthSquared(delta);

    VectorSubtract(radiusdamage_origin, e2->origin, delta);
    d2 = VectorLengthSquared(delta);

    if (d2 <= d1) {
        return d1 > d2;
    } else {
        return -1;
    }
}

void RadiusDamage(
    Vector   origin,
    Entity  *inflictor,
    Entity  *attacker,
    float    damage,
    Entity  *ignore,
    int      mod,
    float    radius,
    float    knockback,
    qboolean constant_damage,
    Weapon  *weap,
    bool     hurtOwnerOnly
)
{
    float               points;
    Entity             *ent;
    Vector              org;
    Vector              dir;
    float               dist;
    int                 i;
    Container<Entity *> ents;

    if (g_showdamage->integer) {
        Com_Printf("radiusdamage");
        Com_Printf("{\n");
        Com_Printf("origin: %f %f %f\n", origin[0], origin[1], origin[2]);

        if (inflictor) {
            Com_Printf("inflictor: (entnum %d, radnum %d)\n", inflictor->entnum, inflictor->radnum);
        }

        if (attacker) {
            Com_Printf("attacker: (entnum %d, radnum %d)\n", attacker->entnum, attacker->radnum);
        }

        Com_Printf("damage: %f\n", damage);

        if (ignore) {
            Com_Printf("ignore: (entnum %d, radnum %d)\n", ignore->entnum, ignore->radnum);
        }

        Com_Printf("mod: %d\n", mod);
        Com_Printf("radius: %f\n", radius);
        Com_Printf("knockback: %f\n", knockback);
        Com_Printf("constant_damage: %d\n", constant_damage);

        if (weap) {
            Com_Printf("weapon %s\n", weap->getName().c_str());
        }

        Com_Printf("hurtOwnerOnly: %d\n", hurtOwnerOnly);
        Com_Printf("}\n");
    }

    ent = findradius(NULL, origin, radius);

    while (ent) {
        // Add ents that has contents
        if (ent->getContents()) {
            ents.AddObject(ent);
        }

        ent = findradius(ent, origin, radius);
    }

    // Sort by the nearest to the fartest entity
    if (ents.NumObjects()) {
        VectorCopy(origin, radiusdamage_origin);
        ents.Sort(radiusdamage_compare);
    }

    for (i = 1; i <= ents.NumObjects(); i++) {
        ent = ents.ObjectAt(i);

        if (ent == ignore || !(ent->takedamage) || (hurtOwnerOnly && ent != attacker)) {
            continue;
        }

        // Add this in for deathmatch maybe

        if (ent->getContents() == CONTENTS_CLAYPIDGEON
            || G_SightTrace(
                origin, vec_zero, vec_zero, ent->centroid, inflictor, ent, MASK_EXPLOSION, false, "RadiusDamage"
            )) {
            if (constant_damage) {
                points = damage;
            } else {
                float ent_rad;

                ent_rad = fabs(ent->maxs[0] - ent->mins[0]);

                if (ent_rad < fabs(ent->maxs[1] - ent->mins[1])) {
                    ent_rad = fabs(ent->maxs[1] - ent->mins[1]);
                }

                if (ent_rad < fabs(ent->maxs[2] - ent->mins[2])) {
                    ent_rad = fabs(ent->maxs[2] - ent->mins[2]);
                }

                org = ent->centroid;
                dir = org - origin;

                dist = dir.length() - ent_rad;
                if (dist < 0.0f) {
                    dist = 0.0f;
                }

                points = damage - damage * (dist / radius);

                knockback -= knockback * (dist / radius);

                if (points < 0) {
                    points = 0;
                }

                if (knockback < 0) {
                    knockback = 0;
                }
            }

            // reduce the damage a little for self-damage
            if (ent == attacker) {
                points *= 0.9f;
            }

            if (points > 0) {
                ent->Damage(inflictor, attacker, points, org, dir, vec_zero, knockback, DAMAGE_RADIUS, mod);

                if (g_gametype->integer == GT_SINGLE_PLAYER && weap) {
                    if (ent->IsSubclassOfPlayer() || ent->IsSubclassOfVehicle() || ent->IsSubclassOfVehicleTank()
                        || ent->isSubclassOf(VehicleCollisionEntity)) {
                        weap->m_iNumHits++;
                        weap->m_iNumTorsoShots++;

                        if (attacker && attacker->IsSubclassOfPlayer() && weap->IsSubclassOfTurretGun()) {
                            Player *player = static_cast<Player *>(attacker);

                            player->m_iNumHits++;
                            player->m_iNumTorsoShots++;
                        }
                    }
                }
            }
        }
    }

    if (mod == MOD_EXPLOSION) {
        // Create an earthquake
        new ViewJitter(
            origin, radius + 128.0f, 0.05f, Vector(damage * 0.05f, damage * 0.05f, damage * 0.06f), 0, vec_zero, 0
        );
    }
}

void FlashPlayers(Vector org, float r, float g, float b, float a, float radius, float time, int type)
{
    trace_t    trace;
    Vector     delta;
    float      length;
    Player    *player;
    gentity_t *ed;
    int        i;
    Entity    *ent;
    float      newa = 1;

    for (i = 0; i < game.maxclients; i++) {
        ed = &g_entities[i];
        if (!ed->inuse || !ed->entity) {
            continue;
        }

        ent = ed->entity;
        if (!ent->IsSubclassOfPlayer()) {
            continue;
        }

        player = (Player *)ent;

        if (!player->WithinDistance(org, radius)) {
            continue;
        }

        trace = G_Trace(org, vec_zero, vec_zero, player->origin, player, MASK_OPAQUE, qfalse, "FlashPlayers");
        if (trace.fraction != 1.0) {
            continue;
        }

        delta  = org - trace.endpos;
        length = delta.length();

        // If alpha is specified, then modify it by the amount of distance away from the flash the player is
        if (a != -1) {
            newa = a * (1 - length / radius);
        }

        level.m_fade_alpha      = newa;
        level.m_fade_color[0]   = r;
        level.m_fade_color[1]   = g;
        level.m_fade_color[2]   = b;
        level.m_fade_time       = time;
        level.m_fade_time_start = time;

        if (type == 1) {
            level.m_fade_style = additive;
        } else {
            level.m_fade_style = alphablend;
        }

        //      gi.SendServerCommand( NULL, va( "fadein %0.2f %0.2f %0.2f %0.2f %i",time*1000,r*newa,g*newa,b*newa,type ) );
    }
}

const char *G_LocationNumToDispString(int iLocation)
{
    switch (iLocation) {
    case -2:
    case -1:
        return "";
    case 0:
        return "head";
    case 1:
        return "helmet";
    case 2:
        return "neck";
    case 3:
        return "upper torso";
    case 4:
        return "middle torso";
    case 5:
        return "lower torso";
    case 6:
        return "pelvis";
    case 7:
        return "upper right arm";
    case 8:
        return "upper left arm";
    case 9:
        return "upper right leg";
    case 10:
        return "upper left leg";
    case 11:
        return "lower right arm";
    case 12:
        return "lower left arm";
    case 13:
        return "lower right leg";
    case 14:
        return "lower left leg";
    case 15:
        return "right hand";
    case 16:
        return "left hand";
    case 17:
        return "right foot";
    case 18:
        return "left foot";
    default:
        return "";
    }
}
