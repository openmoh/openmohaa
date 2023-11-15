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

// player_util.cpp: This file is used to hold the utility functions that are issued by the
// player at the console.  Most of these are developer commands

#include "player.h"
#include "object.h"
#include "g_spawn.h"
#include "scriptexception.h"
#include <utility>

extern Event EV_Entity_Start;

//====================
//Player::ActorInfo
//====================
void Player::ActorInfo(Event *ev)

{
    int     num;
    Entity *ent;

    if (ev->NumArgs() != 1) {
        gi.SendServerCommand(edict - g_entities, "print \"Usage: actorinfo <entity number>\n\"");
        return;
    }

    num = ev->GetInteger(1);
    if ((num < 0) || (num >= globals.max_entities)) {
        gi.SendServerCommand(
            edict - g_entities,
            "print \"Value out of range.  Possible values range from 0 to %d.\n\"",
            globals.max_entities
        );
        return;
    }

    ent = G_GetEntity(num);
    if (!ent || !ent->isSubclassOf(Actor)) {
        gi.SendServerCommand(edict - g_entities, "print \"Entity not an Actor.\n\"");
    } else {
        ((Actor *)ent)->ShowInfo();
    }
}

//====================
//Player::WhatIs
//====================
void Player::WhatIs(Event *ev)

{
    int     num;
    Entity *ent;

    if (ev->NumArgs() != 1) {
        gi.SendServerCommand(edict - g_entities, "print \"Usage: whatis <entity number>\n\"");
        return;
    }

    num = ev->GetInteger(1);
    if ((num < 0) || (num >= globals.max_entities)) {
        gi.SendServerCommand(
            edict - g_entities,
            "print \"Value out of range.  Possible values range from 0 to %d.\n\"",
            globals.max_entities
        );
        return;
    }

    ent = G_GetEntity(num);
    if (!ent) {
        gi.SendServerCommand(edict - g_entities, "print \"Entity not in use.\n\"", globals.max_entities);
    } else {
        const char *animname;

        animname = NULL;
        if (gi.modeltiki(ent->model)) {
            animname = gi.Anim_NameForNum(ent->edict->tiki, ent->edict->s.frameInfo[0].index);
        }

        if (!animname) {
            animname = "( N/A )";
        }

        gi.SendServerCommand(
            edict - g_entities,
            "print \""
            "Entity #   : %d\n"
            "Class ID   : %s\n"
            "Classname  : %s\n"
            "Targetname : %s\n"
            "Modelname  : %s\n"
            "Animname   : %s\n"
            "Origin     : ( %f, %f, %f )\n"
            "Angles     : ( %f, %f, %f )\n"
            "Bounds     : Mins( %.2f, %.2f, %.2f ) Maxs( %.2f, %.2f, %.2f )\n"
            "Velocity   : ( %f, %f, %f )\n"
            "SVFLAGS    : %x\n"
            "Movetype   : %i\n"
            "Solidtype  : %i\n"
            "Contents   : %x\n"
            "Areanum    : %i\n"
            "Parent     : %i\n"
            "Health     : %.1f\n"
            "Max Health : %.1f\n"
            "Edict Owner: %i\n\"",
            num,
            ent->getClassID(),
            ent->getClassname(),
            ent->TargetName().c_str(),
            ent->model.c_str(),
            animname,
            ent->origin.x,
            ent->origin.y,
            ent->origin.z,
            ent->angles.x,
            ent->angles.y,
            ent->angles.z,
            ent->mins.x,
            ent->mins.y,
            ent->mins.z,
            ent->maxs.x,
            ent->maxs.y,
            ent->maxs.z,
            ent->velocity.x,
            ent->velocity.y,
            ent->velocity.z,
            ent->edict->r.svFlags,
            ent->movetype,
            ent->edict->solid,
            ent->edict->r.contents,
            ent->edict->r.areanum,
            ent->edict->s.parent,
            ent->health,
            ent->max_health,
            ent->edict->r.ownerNum
        );
    }
}

//====================
//Player::KillEnt
//====================
void Player::KillEnt(Event *ev)

{
    int     num;
    Entity *ent;

    if (ev->NumArgs() != 1) {
        gi.SendServerCommand(edict - g_entities, "print \"Usage: killent <entity number>\n\"");
        return;
    }

    num = ev->GetInteger(1);
    if ((num < 0) || (num >= globals.max_entities)) {
        gi.SendServerCommand(
            edict - g_entities,
            "print \"Value out of range.  Possible values range from 0 to %d.\n\"",
            globals.max_entities
        );
        return;
    }

    ent = G_GetEntity(num);
    ent->Damage(world, world, ent->max_health + 25, origin, vec_zero, vec_zero, 0, 0, 0);
}

//====================
//Player::RemoveEnt
//====================
void Player::RemoveEnt(Event *ev)

{
    int     num;
    Entity *ent;

    if (ev->NumArgs() != 1) {
        gi.SendServerCommand(edict - g_entities, "print \"Usage: removeent <entity number>\n\"");
        return;
    }

    num = ev->GetInteger(1);
    if ((num < 0) || (num >= globals.max_entities)) {
        gi.SendServerCommand(
            edict - g_entities,
            "print \"Value out of range.  Possible values range from 0 to %d.\n\"",
            globals.max_entities
        );
        return;
    }

    ent = G_GetEntity(num);
    ent->PostEvent(Event(EV_Remove), 0);
}

//====================
//Player::KillClass
//====================
void Player::KillClass(Event *ev)

{
    int        except;
    str        classname;
    gentity_t *from;
    Entity    *ent;

    if (ev->NumArgs() < 1) {
        gi.SendServerCommand(edict - g_entities, "print \"Usage: killclass <classname> [except entity number]\n\"");
        return;
    }

    classname = ev->GetString(1);

    except = 0;
    if (ev->NumArgs() == 2) {
        except = ev->GetInteger(1);
    }

    for (from = this->edict + 1; from < &g_entities[globals.num_entities]; from++) {
        if (!from->inuse) {
            continue;
        }

        assert(from->entity);

        ent = from->entity;

        if (ent->entnum == except) {
            continue;
        }

        if (ent->inheritsFrom(classname.c_str())) {
            ent->Damage(world, world, ent->max_health + 25, origin, vec_zero, vec_zero, 0, 0, 0);
        }
    }
}

//====================
//Player::RemoveClass
//====================
void Player::RemoveClass(Event *ev)

{
    int        except;
    str        classname;
    gentity_t *from;
    Entity    *ent;

    if (ev->NumArgs() < 1) {
        gi.SendServerCommand(edict - g_entities, "print \"Usage: removeclass <classname> [except entity number]\n\"");
        return;
    }

    classname = ev->GetString(1);

    except = 0;
    if (ev->NumArgs() == 2) {
        except = ev->GetInteger(1);
    }

    for (from = this->edict + 1; from < &g_entities[globals.num_entities]; from++) {
        if (!from->inuse) {
            continue;
        }

        assert(from->entity);

        ent = from->entity;

        if (ent->entnum == except) {
            continue;
        }

        if (ent->inheritsFrom(classname.c_str())) {
            ent->PostEvent(Event(EV_Remove), 0);
        }
    }
}

//====================
//Player::TestThread
//====================
void Player::TestThread(Event *ev)

{
    const char *scriptfile;
    str         label;

    if (ev->NumArgs() < 1) {
        gi.SendServerCommand(edict - g_entities, "print \"Syntax: testthread scriptfile <label>.\n\"");
        return;
    }

    scriptfile = ev->GetString(1);
    if (ev->NumArgs() > 1) {
        label = ev->GetString(2);
    }

    Director.ExecuteThread(scriptfile, label);
}

//====================
//Player::SpawnEntity
//====================
void Player::SpawnEntity(Event *ev)

{
    Entity   *ent;
    str       name;
    ClassDef *cls;
    str       text;
    Vector    forward;
    Vector    up;
    Vector    delta;
    Vector    v;
    int       n;
    int       i;
    Event    *e;

    if (ev->NumArgs() < 1) {
        ScriptError("Usage: spawn entityname [keyname] [value]...");
        return;
    }

    name = ev->GetString(1);
    if (!name.length()) {
        ScriptError("Must specify an entity name");
        return;
    }

    // create a new entity
    SpawnArgs args;

    args.setArg("classname", name.c_str());
    args.setArg("model", name.c_str());

    cls = args.getClassDef();
    if (!cls) {
        cls = &Entity::ClassInfo;
    }

    if (!checkInheritance(&Entity::ClassInfo, cls)) {
        ScriptError("%s is not a valid Entity", name.c_str());
        return;
    }

    ent = (Entity *)cls->newInstance();

    e = new Event(EV_Model);
    e->AddString(name.c_str());
    ent->PostEvent(e, EV_PRIORITY_SPAWNARG);

    angles.AngleVectors(&forward, NULL, &up);
    v = origin + (forward + up) * 40;

    e = new Event(EV_SetOrigin);
    e->AddVector(v);
    ent->PostEvent(e, EV_SPAWNARG);

    delta = origin - v;
    v.x   = 0;
    v.y   = delta.toYaw();
    v.z   = 0;

    e = new Event(EV_SetAngles);
    e->AddVector(v);
    ent->PostEvent(e, EV_SPAWNARG);

    if (ev->NumArgs() > 2) {
        n = ev->NumArgs();
        for (i = 2; i <= n; i += 2) {
            e = new Event(ev->GetString(i));
            e->AddToken(ev->GetString(i + 1));
            ent->PostEvent(e, EV_SPAWNARG);
        }
    }

    e = new Event(EV_SetAnim);
    e->AddString("idle");
    ent->PostEvent(e, EV_SPAWNARG);

    ent->ProcessPendingEvents();
    ent->ProcessEvent(EV_Entity_Start);
}

//====================
//Player::SpawnActor
//====================
void Player::SpawnActor(Event *ev)

{
    Entity   *ent;
    str       name;
    str       text;
    Vector    forward;
    Vector    up;
    Vector    delta;
    Vector    v;
    int       n;
    int       i;
    ClassDef *cls;
    Event    *e;

    if (ev->NumArgs() < 1) {
        ScriptError("Usage: actor [modelname] [keyname] [value]...");
        return;
    }

    name = ev->GetString(1);
    if (!name[0]) {
        ScriptError("Must specify a model name");
        return;
    }

    if (!strstr(name.c_str(), ".tik")) {
        name += ".tik";
    }

    // create a new entity
    SpawnArgs args;

    args.setArg("model", name.c_str());

    cls = args.getClassDef();

    if (cls == &Object::ClassInfo) {
        cls = &Actor::ClassInfo;
    }

    if (!cls || !checkInheritance(&Actor::ClassInfo, cls)) {
        ScriptError("%s is not a valid Actor", name.c_str());
        return;
    }

    ent = (Entity *)cls->newInstance();
    e   = new Event(EV_Model);
    e->AddString(name.c_str());
    ent->PostEvent(e, EV_SPAWNARG);

    angles.AngleVectors(&forward, NULL, &up);
    v = origin + (forward + up) * 40;

    e = new Event(EV_SetOrigin);
    e->AddVector(v);
    ent->PostEvent(e, EV_SPAWNARG);

    delta = origin - v;
    v     = delta.toAngles();

    e = new Event(EV_SetAngle);
    e->AddFloat(v[1]);
    ent->PostEvent(e, EV_SPAWNARG);

    if (ev->NumArgs() > 2) {
        n = ev->NumArgs();
        for (i = 2; i <= n; i += 2) {
            e = new Event(ev->GetString(i));
            e->AddToken(ev->GetString(i + 1));
            ent->PostEvent(e, EV_SPAWNARG);
        }
    }
}

void Player::ListInventoryEvent(Event *ev)

{
    ListInventory();
}

void Player::GetStateAnims(Container<const char *> *c)

{
    statemap_Legs->GetAllAnims(c);
    statemap_Torso->GetAllAnims(c);
}

static fileHandle_t logfile = 0;

static void OpenPlayerLogFile(void)

{
    str s, filename;

    s = "playlog_";
    s += level.mapname;
    //filename = gi.GetArchiveFileName( s, "log" );

    logfile = gi.FS_FOpenFileWrite(filename.c_str());
}

void Player::LogStats(Event *ev)

{
    str s;

    if (!logfile) {
        OpenPlayerLogFile();
    }

    if (!logfile) {
        return;
    }

    int b  = AmmoCount("Bullet");
    int p  = AmmoCount("Plasma");
    int g  = AmmoCount("Gas");
    int r  = AmmoCount("Rocket");
    int f  = AmmoCount("Flashbangs");
    int m  = AmmoCount("Meteor");
    int gp = AmmoCount("Gas Pod");

    s = va("%.2f\t", level.time);
    s += va("(%.2f %.2f %.2f)\t", origin.x, origin.y, origin.z);
    s += va("%.2f\t", health);
    s += va("%d\t%d\t%d\t%d\t%d\t%d\t%d\n", b, p, g, r, f, m, gp);

    //gi.FS_Write( s, s.length(), logfile );
    // FIXME: FS
    gi.FS_Flush(logfile);

    Event *ev1 = new Event(std::move(*ev));
    PostEvent(ev1, 1);
}

void ClosePlayerLogFile(void)
{
    if (logfile) {
        gi.FS_FCloseFile(logfile);
        logfile = 0;
    }
}

void Player::SkipCinematic(Event *ev)
{
    if (level.cinematic && (level.RegisterSize(STRING_SKIP))) {
        ScriptThreadLabel skiplabel;
        G_ClearFade();

        Unregister(STRING_SKIP);

        // reset the roll on our view just in case
        v_angle.z = 0;
        SetViewAngles(v_angle);
    }
}

void Player::EventTeleport(Event *ev)
{
    if (ev->NumArgs() == 1) {
        setOrigin(ev->GetVector(1));
    } else {
        setOrigin(Vector(ev->GetFloat(1), ev->GetFloat(2), ev->GetFloat(3)));
    }
}

void Player::EventGetIsEscaping(Event* ev)
{
    ev->AddInteger(m_jailstate == JAILSTATE_ESCAPE);
}

void Player::EventJailEscapeStop(Event* ev)
{
    m_jailstate = JAILSTATE_NONE;
}

void Player::EventJailAssistEscape(Event* ev)
{
	m_jailstate = JAILSTATE_ASSIST_ESCAPE;
}

void Player::EventJailEscape(Event* ev)
{
	m_jailstate = JAILSTATE_ESCAPE;
}

void Player::EventFace(Event *ev)
{
    SetViewAngles(Vector(ev->GetFloat(1), ev->GetFloat(2), ev->GetFloat(3)));
}

void Player::EventCoord(Event *ev)
{
    gi.Printf("location: %.2f %.2f %.2f\nangles: %.2f %.2f %.2f\n(use 'tele' or 'face' to set)\n", origin[0], origin[1], origin[2], v_angle[0], v_angle[1], v_angle[2]);
}
