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

// bspline.cpp: Uniform non-rational bspline class.
//

#include "g_local.h"
#include "bspline.h"
#include "game.h"
#include "debuglines.h"
#include "g_phys.h"
#include "scriptexception.h"

void BSpline::Set(Vector *control_points_, int num_control_points_, splinetype_t type)
{
    int i;

    SetType(type);

    has_orientation = false;

    if (control_points) {
        delete[] control_points;
        control_points = NULL;
    }

    num_control_points = num_control_points_;
    if (num_control_points) {
        control_points = new BSplineControlPoint[num_control_points];
        assert(control_points);

        for (i = 0; i < num_control_points; i++) {
            control_points[i].Set(control_points_[i]);
        }
    }
}

void BSpline::Set(
    Vector      *control_points_,
    Vector      *control_orients_,
    float       *control_speeds_,
    int          num_control_points_,
    splinetype_t type
)
{
    int i;

    SetType(type);

    has_orientation = true;

    if (control_points) {
        delete[] control_points;
        control_points = NULL;
    }

    num_control_points = num_control_points_;
    if (num_control_points) {
        control_points = new BSplineControlPoint[num_control_points];
        assert(control_points);

        for (i = 0; i < num_control_points; i++) {
            control_points[i].Set(control_points_[i], control_orients_[i], control_speeds_[i]);
        }
    }
}

void BSpline::Clear(void)
{
    if (control_points) {
        delete[] control_points;
        control_points = NULL;
    }
    num_control_points = 0;
    has_orientation    = false;
}

inline float BSpline::EvalNormal(float u, Vector& pos, Vector& orient)
{
    int    segment_id;
    float  B[4];
    float  tmp;
    float  u_2;
    float  u_3;
    Vector ang;
    float  roll;
    float  speed;

    segment_id = (int)u;
    if (segment_id < 0) {
        segment_id = 0;
    }
    if (segment_id > num_control_points - 4) {
        segment_id = num_control_points - 4;
    }
    u -= (float)segment_id;

    u_2 = u * u;
    u_3 = u * u_2;

    tmp  = 1 - u;
    B[0] = (tmp * tmp * tmp) * (1.0f / 6.0f);
    B[1] = (3.0f * u_3 - 6.0f * u_2 + 4.0f) * (1.0f / 6.0f);
    B[2] = (-3.0f * u_3 + 3.0f * u_2 + 3.0f * u + 1) * (1.0f / 6.0f);
    B[3] = u_3 * (1.0f / 6.0f);

    pos = *control_points[0 + segment_id].GetPosition() * B[0] + *control_points[1 + segment_id].GetPosition() * B[1]
        + *control_points[2 + segment_id].GetPosition() * B[2] + *control_points[3 + segment_id].GetPosition() * B[3];

    ang = *control_points[0 + segment_id].GetOrientation() * B[0]
        + *control_points[1 + segment_id].GetOrientation() * B[1]
        + *control_points[2 + segment_id].GetOrientation() * B[2]
        + *control_points[3 + segment_id].GetOrientation() * B[3];

    roll = *control_points[0 + segment_id].GetRoll() * B[0] + *control_points[1 + segment_id].GetRoll() * B[1]
         + *control_points[2 + segment_id].GetRoll() * B[2] + *control_points[3 + segment_id].GetRoll() * B[3];

    speed = *control_points[0 + segment_id].GetSpeed() * B[0] + *control_points[1 + segment_id].GetSpeed() * B[1]
          + *control_points[2 + segment_id].GetSpeed() * B[2] + *control_points[3 + segment_id].GetSpeed() * B[3];

    orient       = ang.toAngles();
    orient[ROLL] = roll;

    return speed;
}

inline float BSpline::EvalLoop(float t, Vector& pos, Vector& orient)
{
    Vector retval;
    Vector ang;
    float  speed;
    float  roll;
    int    segment_id;
    int    next_id;
    float  B[4];
    float  tmp;
    float  u;
    float  u_2;
    float  u_3;
    int    i;
    int    j;

    segment_id = (int)floor(t);
    u          = t - floor(t);

    segment_id %= num_control_points;
    if (segment_id < 0) {
        segment_id += num_control_points;
    }

    u_2 = u * u;
    u_3 = u * u_2;

    tmp  = 1 - u;
    B[0] = (tmp * tmp * tmp) * (1.0f / 6.0f);
    B[1] = (3.0f * u_3 - 6.0f * u_2 + 4.0f) * (1.0f / 6.0f);
    B[2] = (-3.0f * u_3 + 3.0f * u_2 + 3.0f * u + 1) * (1.0f / 6.0f);
    B[3] = u_3 * (1.0f / 6.0f);

    speed = 0;
    roll  = 0;

    for (i = 0, j = segment_id; i < 4; i++, j++) {
        if (j >= num_control_points) {
            j -= (num_control_points - loop_control_point);
        }

        retval += *control_points[j].GetPosition() * B[i];
        ang += *control_points[j].GetOrientation() * B[i];
        speed += *control_points[j].GetSpeed() * B[i];
        roll += *control_points[j].GetRoll() * B[i];
    }

    pos = retval;

    next_id = segment_id + 1;
    if (next_id >= num_control_points) {
        next_id -= (num_control_points - loop_control_point);
    }
    orient       = ang.toAngles();
    orient[ROLL] = roll;

    return speed;
}

inline float BSpline::EvalClamp(float t, Vector& pos, Vector& orient)
{
    Vector retval;
    Vector ang;
    int    segment_id;
    int    next_id;
    float  B[4];
    float  tmp;
    float  u;
    float  u_2;
    float  u_3;
    int    i;
    int    j;
    float  speed;
    float  roll;

    segment_id = (int)floor(t);
    u          = t - floor(t);

    u_2 = u * u;
    u_3 = u * u_2;

    tmp  = 1 - u;
    B[0] = (tmp * tmp * tmp) * (1.0f / 6.0f);
    B[1] = (3.0f * u_3 - 6.0f * u_2 + 4.0f) * (1.0f / 6.0f);
    B[2] = (-3.0f * u_3 + 3.0f * u_2 + 3.0f * u + 1) * (1.0f / 6.0f);
    B[3] = u_3 * (1.0f / 6.0f);

    speed = 0;
    roll  = 0;
    for (i = 0; i < 4; i++, segment_id++) {
        j = segment_id;
        if (j < 0) {
            j = 0;
        } else if (j >= num_control_points) {
            j = num_control_points - 1;
        }

        retval += *control_points[j].GetPosition() * B[i];
        ang += *control_points[j].GetOrientation() * B[i];
        speed += *control_points[j].GetSpeed() * B[i];
        roll += *control_points[j].GetRoll() * B[i];
    }

    pos = retval;

    next_id = segment_id + 1;
    if (segment_id < 0) {
        segment_id = 0;
    }
    if (segment_id >= num_control_points) {
        segment_id = num_control_points - 1;
    }
    if (next_id < 0) {
        next_id = 0;
    }
    if (next_id >= num_control_points) {
        next_id = num_control_points - 1;
    }
    orient       = ang.toAngles();
    orient[ROLL] = roll;

    return speed;
}

Vector BSpline::Eval(float u)
{
    Vector pos;
    Vector orient;

    switch (curvetype) {
    default:
    case SPLINE_NORMAL:
        EvalNormal(u, pos, orient);
        break;

    case SPLINE_CLAMP:
        EvalClamp(u, pos, orient);
        break;

    case SPLINE_LOOP:
        if (u < 0) {
            EvalClamp(u, pos, orient);
        } else {
            EvalLoop(u, pos, orient);
        }
        break;
    }
    return pos;
}

float BSpline::Eval(float u, Vector& pos, Vector& orient)
{
    switch (curvetype) {
    default:
    case SPLINE_NORMAL:
        return EvalNormal(u, pos, orient);
        break;

    case SPLINE_CLAMP:
        return EvalClamp(u, pos, orient);
        break;

    case SPLINE_LOOP:
        if (u < 0) {
            return EvalClamp(u, pos, orient);
        } else {
            return EvalLoop(u, pos, orient);
        }
        break;
    }
}

void BSpline::DrawControlSegments(void)
{
    int i;

    G_BeginLine();
    for (i = 0; i < num_control_points; i++) {
        G_Vertex(*control_points[i].GetPosition());
    }
    G_EndLine();
}

void BSpline::DrawCurve(int num_subdivisions)
{
    float u;
    float du;

    if (!num_control_points) {
        return;
    }

    du = 1.0f / (float)num_subdivisions;

    G_BeginLine();
    for (u = -2.0f; u <= (float)num_control_points; u += du) {
        G_Vertex((Vector)Eval(u));
    }
    G_EndLine();
}

void BSpline::DrawCurve(Vector offset, int num_subdivisions)
{
    float u;
    float du;

    du = 1.0f / (float)num_subdivisions;

    G_BeginLine();
    for (u = -2.0f; u <= (float)num_control_points; u += du) {
        G_Vertex(offset + (Vector)Eval(u));
    }
    G_EndLine();
}

void BSpline::AppendControlPoint(const Vector& new_control_point)
{
    BSplineControlPoint *old_control_points;
    int                  i;

    old_control_points = control_points;
    num_control_points++;

    control_points = new BSplineControlPoint[num_control_points];
    assert(control_points);

    if (old_control_points) {
        for (i = 0; i < num_control_points - 1; i++) {
            control_points[i] = old_control_points[i];
        }
        delete[] old_control_points;
    }

    control_points[num_control_points - 1].Set(new_control_point);
}

void BSpline::AppendControlPoint(const Vector& new_control_point, const float& speed)
{
    BSplineControlPoint *old_control_points;
    int                  i;

    old_control_points = control_points;
    num_control_points++;

    control_points = new BSplineControlPoint[num_control_points];
    assert(control_points);

    if (old_control_points) {
        for (i = 0; i < num_control_points - 1; i++) {
            control_points[i] = old_control_points[i];
        }
        delete[] old_control_points;
    }

    control_points[num_control_points - 1].Set(new_control_point, speed);
}

void BSpline::AppendControlPoint(
    const Vector& new_control_point, const Vector& new_control_orient, const float& new_control_speed
)
{
    BSplineControlPoint *old_control_points;
    int                  i;

    has_orientation = true;

    old_control_points = control_points;
    num_control_points++;

    control_points = new BSplineControlPoint[num_control_points];
    assert(control_points);

    if (old_control_points) {
        for (i = 0; i < num_control_points - 1; i++) {
            control_points[i] = old_control_points[i];
        }
        delete[] old_control_points;
    }

    control_points[num_control_points - 1].Set(new_control_point, new_control_orient, new_control_speed);
}

void BSpline::SetLoopPoint(const Vector& pos)
{
    int i;

    for (i = 0; i < num_control_points; i++) {
        if (pos == *control_points[i].GetPosition()) {
            loop_control_point = i;
            break;
        }
    }
}

int BSpline::PickControlPoint(const Vector& window_point, float pick_size)
{
    int    i;
    float  closest_dist_2;
    int    closest_index;
    float  dist_2;
    Vector delta;

    closest_index  = -1;
    closest_dist_2 = 1000000.0f;
    for (i = 0; i < num_control_points; i++) {
        delta  = window_point - *control_points[i].GetPosition();
        dist_2 = delta * delta;
        if (dist_2 < closest_dist_2) {
            closest_dist_2 = dist_2;
            closest_index  = i;
        }
    }

    if (pick_size * pick_size >= closest_dist_2) {
        return closest_index;
    } else {
        return -1;
    }
}

Event EV_SplinePath_Create
(
    "SplinePath_create",
    EV_DEFAULT,
    NULL,
    NULL,
    "Creates the spline path from the target list.",
    EV_NORMAL
);
Event EV_SplinePath_Loop
(
    "loop",
    EV_CONSOLE,
    "s",
    "loop_name",
    "Sets the loop name.",
    EV_NORMAL
);
Event EV_SplinePath_Speed
(
    "speed",
    EV_DEFAULT,
    "f",
    "speed",
    "Sets the path speed.",
    EV_NORMAL
);
Event EV_SplinePath_SetTriggerTarget
(
    "triggertarget",
    EV_DEFAULT,
    "s",
    "target",
    "Sets the trigger target.",
    EV_NORMAL
);
Event EV_SplinePath_SetWatch
(
    "watch",
    EV_CONSOLE,
    "s",
    "watchEntity",
    "Sets the entity to watch at this node.",
    EV_NORMAL
);

Event EV_SplinePath_SetFov
(
    "fov",
    EV_CONSOLE,
    "f",
    "cameraFOV",
    "Sets the fov at this node.",
    EV_NORMAL
);

Event EV_SplinePath_SetFadeTime
(
    "fadetime",
    EV_DEFAULT,
    "f",
    "fadeTime",
    "Sets the fadetime at this node.",
    EV_NORMAL
);

CLASS_DECLARATION(Entity, SplinePath, "info_splinepath") {
    {&EV_SplinePath_Create,           &SplinePath::CreatePath      },
    {&EV_SplinePath_Loop,             &SplinePath::SetLoop         },
    {&EV_SplinePath_Speed,            &SplinePath::SetSpeed        },
    {&EV_SplinePath_SetTriggerTarget, &SplinePath::SetTriggerTarget},
    {&EV_SplinePath_SetWatch,         &SplinePath::SetWatch        },
    {&EV_SplinePath_SetFov,           &SplinePath::SetFov          },
    {&EV_SplinePath_SetFadeTime,      &SplinePath::SetFadeTime     },
    {NULL,                            NULL                         }
};

SplinePath::SplinePath()
{
    entflags |= ECF_SPLINEPATH;

    AddWaitTill(STRING_REACH);

    owner    = this;
    next     = NULL;
    loop     = NULL;
    speed    = 1;
    doWatch  = false;
    watchEnt = "";
    fov      = 0;
    fadeTime = -1;

    setMoveType(MOVETYPE_NONE);
    setSolidType(SOLID_NOT);
    hideModel();

    if (!LoadingSavegame) {
        PostEvent(EV_SplinePath_Create, FRAMETIME);
    }
}

SplinePath::~SplinePath()
{
    // disconnect from the chain
    if (owner != this) {
        owner->SetNext(next);
    } else if (next) {
        next->SetPrev(NULL);
        next = NULL;
    }

    assert(owner == this);
    assert(next == NULL);
}

void SplinePath::SetLoop(Event *ev)
{
    loop_name = ev->GetString(1);
}

void SplinePath::SetSpeed(Event *ev)
{
    speed = ev->GetFloat(1);
}

void SplinePath::SetTriggerTarget(Event *ev)
{
    SetTriggerTarget(ev->GetString(1));
}

void SplinePath::CreatePath(Event *ev)
{
    const char *target;
    Entity     *ent;

    // Make the path from the targetlist.
    target = Target();
    if (target[0]) {
        ent = (Entity *)G_FindTarget(NULL, target);
        if (ent) {
            next        = (SplinePath *)ent;
            next->owner = this;
        } else {
            ScriptError("SplinePath::CreatePath: target %s not found\n", target);
        }
    }
    if (loop_name.length()) {
        ent = (Entity *)G_FindTarget(NULL, loop_name.c_str());
        if (ent) {
            loop = (SplinePath *)ent;
        }
    }
}

SplinePath *SplinePath::GetNext(void)
{
    return next;
}

SplinePath *SplinePath::GetPrev(void)
{
    if (owner == this) {
        return NULL;
    }

    return owner;
}

void SplinePath::SetNext(SplinePath *node)
{
    if (next) {
        // remove ourselves from the chain
        next->owner = next;
    }

    next = node;
    if (next) {
        // disconnect next from it's previous node
        if (next->owner != next) {
            next->owner->next = NULL;
        }
        next->owner = this;
    }
}

void SplinePath::SetPrev(SplinePath *node)
{
    if (owner != this) {
        owner->next = NULL;
    }

    if (node && (node != this)) {
        // safely remove the node from its chain
        if (node->next) {
            node->next->owner = node->next;
        }
        node->next = this;
        owner      = node;
    } else {
        owner = this;
    }
}

SplinePath *SplinePath::GetLoop(void)
{
    return loop;
}

void SplinePath::SetWatch(const char *name)
{
    if (watchEnt != name) {
        watchEnt = name;
        if (watchEnt.length()) {
            doWatch = true;
        } else {
            doWatch = false;
        }
    }
}

void SplinePath::SetWatch(Event *ev)
{
    SetWatch(ev->GetString(1));
}

void SplinePath::NoWatch(void)
{
    doWatch  = true;
    watchEnt = "none";
}

str SplinePath::GetWatch(void)
{
    return watchEnt;
}

void SplinePath::SetFov(float newFov)
{
    fov = newFov;
}

void SplinePath::SetFov(Event *ev)
{
    fov = ev->GetFloat(1);
}

float SplinePath::GetFov(void)
{
    return fov;
}

void SplinePath::SetFadeTime(float newFadeTime)
{
    fadeTime = newFadeTime;
}

void SplinePath::SetFadeTime(Event *ev)
{
    fadeTime = ev->GetFloat(1);
}

float SplinePath::GetFadeTime(void)
{
    return fadeTime;
}
