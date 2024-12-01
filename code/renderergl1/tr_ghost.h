/*
===========================================================================
Copyright (C) 2023-2024 the OpenMoHAA team

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

#include "tr_local.h"
#include "../qcommon/vector.h"
#include "../qcommon/container.h"
#include "../qcommon/str.h"

using outcode = unsigned int;

class Particle
{
private:
    float m_startTime;
    float m_dieTime;

    Vector m_velocity;
    Vector m_acceleration;
    Vector m_perp;

    qboolean m_wavy;
    float    m_wavyDist;
    float    m_wavyOffset;

    int   m_srcColor;
    int   m_dstColor;
    int   m_deltaR;
    int   m_deltaG;
    int   m_deltaB;
    int   m_srcR;
    int   m_srcG;
    int   m_srcB;
    int   m_dstR;
    int   m_dstG;
    int   m_dstB;
    float m_colorRate;

    Vector m_realposition;

    float m_life;
    float m_maxspeed;

    qboolean m_swarm;
    int      m_swarmfrequency;
    int      m_swarmdelta;

    Vector m_parentOrigin;

public:
    Vector m_position;
    int    m_color;

public:
    Particle(
        Vector   pos,
        Vector   vel,
        Vector   acc,
        int      srcColor,
        int      dstColor,
        float    colorRate,
        qboolean wavy,
        float    wavyDist,
        float    currentTime,
        float    dieTime,
        float    maxspeed,
        Vector   parentOrg,
        qboolean swarm,
        int      freq,
        int      delta
    );

    void  Update(float currentTime);
    float GetDieTime();
};

class ParticleEmitter
{
private:
    float m_time;
    float m_lasttime;

    Vector m_position;

    int   m_srcColor;
    int   m_dstColor;
    float m_minColorRate;
    float m_maxColorRate;

    float m_minRate;
    float m_maxRate;

    float m_minLife;
    float m_maxLife;

    float m_angle;
    float m_angleVar;

    float m_minSpeed;
    float m_maxSpeed;
    float m_accAngle;
    float m_accAngleVar;
    float m_minAccSpeed;
    float m_maxAccSpeed;

    float    m_minWavyDist;
    float    m_maxWavyDist;
    qboolean m_wavy;

    qboolean m_gravityWell;
    float    m_gravityWellStrength;

    qboolean m_particles;
    Vector   m_gravityEffectVector;

    qboolean m_ballLightning;
    int      m_minBallLightningRadius;
    int      m_maxBallLightningRadius;
    float    m_lightningVar;
    int      m_lightningSubdivisions;

    qboolean m_swarm;
    int      m_swarmfrequency;
    int      m_swarmdelta;

    int m_maxspeed;

public:
    Container<Particle *> particleList;

public:
    ParticleEmitter(
        Vector   position,
        float    minSpeed,
        float    maxSpeed,
        float    angle,
        float    angleVar,
        float    minAccSpeed,
        float    maxAccSpeed,
        float    accAngle,
        float    accAngleVar,
        float    minRate,
        float    maxRate,
        int      srcColor,
        int      dstColor,
        float    minColorRate,
        float    maxColorRate,
        qboolean wavy,
        float    minWavyDist,
        float    maxWavyDist,
        float    minLife,
        float    maxLife,
        qboolean particles,
        qboolean gravityWell,
        float    gravityWellStrength,
        qboolean ballLightning,
        int      minBallLightningRadius,
        int      maxBallLightningRadius,
        float    lightningVar,
        int      lightningSubdivisions,
        qboolean swarm,
        int      swarmfreq,
        int      swarmdelta
    );
    ParticleEmitter();
    ~ParticleEmitter();

    void Emit(float currentTime);
    void UpdateValues();

    Vector GetPosition();
    Vector GetVelocity();
    Vector GetVelocityDirection();
    Vector GetVelocityDirectionMin();
    Vector GetVelocityDirectionMax();
    Vector GetAcceleration();
    Vector GetAccelerationDirection();
    Vector GetAccelerationDirectionMin();
    Vector GetAccelerationDirectionMax();

    int GetSrcColor();
    int GetDstColor();

    float GetGravityWellStrength();
    float GetRate();
    float GetColorRate();

    float GetWavyDistance();
    float GetDieTime(float currentTime);
    int   GetBallLightningRadius();
    float GetLightningVar();
    int   GetLightningSubdivisions();

    qboolean IsWavy();
    qboolean IsGravityWell();
    qboolean IsParticles();
    qboolean IsBallLightning();
    qboolean IsSwarm();

    void SetMinSpeed(float value);
    void SetMaxSpeed(float value);

    void SetAngle(float value);
    void SetAngleVar(float value);

    void SetMinAccSpeed(float value);
    void SetMaxAccSpeed(float value);
    void SetAccAngle(float value);
    void SetAccAngleVar(float value);

    void SetMinRate(float value);
    void SetMaxRate(float value);

    void SetSrcColor(int value);
    void SetDstColor(int value);
    void SetMinColorRate(float value);
    void SetMaxColorRate(float value);

    void SetWavy(int value);

    void SetParticles(int value);
    void SetGravityWell(int value);

    void SetMinWavyDist(float value);
    void SetMaxWavyDist(float value);

    void SetMinLife(float value);
    void SetMaxLife(float value);

    void SetGravityWellStrength(float value);
    void SetGravityEffectVector(Vector value);

    void SetBallLightning(int value);
    void SetMinBallLightningRadius(int value);
    void SetMaxBallLightningRadius(int value);
    void SetLightningVar(float value);
    void SetLightningSubdivisions(int value);

    float RandomizeRange(float minRange, float maxRange);
    float RandomizeAngle(float minAngle, float maxAngle);

    Vector CalculateDirection(float value);

    void Load(char **buf_p);
};

class GhostTexture
{
public:
    Container<ParticleEmitter *> m_emitterList;

    int m_width;
    int m_height;

    image_t      *m_image;
    unsigned int *m_texture;
    str           m_name;

    qboolean m_isburn;
    qboolean m_isfade;
    int      m_faderate;
    int      m_burnrate;

public:
    GhostTexture();

    void   Update();
    void   SetTexel(int x, int y, unsigned int color);
    void   Burn();
    void   GenerateLightning(Vector p1, Vector p2, int color, float angleVar, int numSubdivisions, int maxSubdivisions);
    Vector RotateVector(Vector v, float angle);
    void   ClipAndDrawLine(Vector p0, Vector p1, int color);
    outcode ComputeOutCode(int x, int y, int xmin, int xmax, int ymin, int ymax);
};

class GhostManager
{
public:
    Container<GhostTexture *> m_textureList;
};
