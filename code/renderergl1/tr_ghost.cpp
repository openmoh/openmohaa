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

// tr_ghost.cpp

#include "tr_local.h"
#include "tr_ghost.h"

GhostManager ghostManager;

int frameTime;
int lastTime;

/*
====================
RandomizeRange

====================
*/
float RandomizeRange(float min, float max)
{
    return min + (max - min) * (rand() / (float)RAND_MAX);
}

/*
====================
Particle::Particle

Initializes the particle with the specified parameters
====================
*/
Particle::Particle(
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
)
{
    m_position     = pos;
    m_realposition = pos;
    m_velocity     = vel;
    m_acceleration = acc;

    m_srcColor  = srcColor;
    m_dstColor  = dstColor;
    m_color     = srcColor;
    m_colorRate = colorRate;

    m_wavy       = wavy;
    m_wavyDist   = wavyDist;
    m_wavyOffset = RandomizeRange(-wavyDist, wavyDist);

    m_swarm          = swarm;
    m_swarmfrequency = freq;
    m_swarmdelta     = delta;

    m_startTime = currentTime;
    m_dieTime   = dieTime;
    m_life      = dieTime - currentTime;

    m_maxspeed     = maxspeed;
    m_parentOrigin = parentOrg;

    Vector tempvec = m_velocity;
    VectorNormalize(tempvec);

    m_perp.x = tempvec.x * cos(M_PI / 2) - tempvec.y * sin(M_PI / 2);
    m_perp.y = tempvec.x * sin(M_PI / 2) + tempvec.y * cos(M_PI / 2);

    m_srcR = (m_srcColor & 0xff);
    m_srcG = (m_srcColor & 0xff00) >> 8;
    m_srcB = (m_srcColor & 0xff0000) >> 16;

    m_dstR = (m_dstColor & 0xff);
    m_dstG = (m_dstColor & 0xff00) >> 8;
    m_dstB = (m_dstColor & 0xff0000) >> 16;

    m_deltaR = m_dstR - m_srcR;
    m_deltaG = m_dstG - m_srcG;
    m_deltaB = m_dstB - m_srcB;
}

/*
====================
Particle::Update

Updates the particle based on the current time
====================
*/
void Particle::Update(float currentTime)
{
    int    r, g, b;
    float  ftime;
    float  time2;
    float  factor;
    Vector offset;

    ftime = frameTime / 1000.0;
    time2 = Square(ftime) * 0.5;

    if (m_swarm) {
        if (!(rand() % m_swarmfrequency)) {
            m_velocity[0] = crandom() * m_maxspeed;
            m_velocity[1] = crandom() * m_maxspeed;
            m_velocity[2] = crandom() * m_maxspeed;
        }

        if (m_parentOrigin[0] > m_position[0]) {
            m_velocity[0] += m_swarmdelta;
        } else {
            m_velocity[0] -= m_swarmdelta;
        }
        if (m_parentOrigin[1] > m_position[1]) {
            m_velocity[1] += m_swarmdelta;
        } else {
            m_velocity[1] -= m_swarmdelta;
        }
        if (m_parentOrigin[2] > m_position[2]) {
            m_velocity[2] += m_swarmdelta;
        } else {
            m_velocity[2] -= m_swarmdelta;
        }
    }

    m_position = m_realposition;

    m_position += m_velocity * ftime + m_acceleration * time2;
    m_velocity += m_acceleration * ftime;
    m_realposition = m_position;

    if (m_wavy) {
        offset = m_perp * sin(currentTime + m_wavyOffset) * (m_wavyOffset + m_wavyDist);
        m_position += offset;
    }

    factor = 1.0 - (m_dieTime - currentTime) / m_life;
    r      = m_deltaR * factor;
    g      = m_deltaG * factor;
    b      = m_deltaB * factor;

    m_color = (r) | (g << 8) | (b << 8);
}

/*
====================
Particle::GetDieTime

Returns the time at which the particle stops
====================
*/
float Particle::GetDieTime()
{
    return m_dieTime;
}

/*
====================
ParticleEmitter::ParticleEmitter

Initializes the particle emitter with specified parameters
====================
*/
ParticleEmitter::ParticleEmitter(
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
)
{
    m_position = position;

    m_minSpeed = minSpeed;
    m_maxSpeed = maxSpeed;

    m_angle       = angle;
    m_angleVar    = angleVar;
    m_minAccSpeed = minAccSpeed;
    m_maxAccSpeed = maxAccSpeed;
    m_accAngle    = accAngle;
    m_accAngleVar = accAngleVar;

    m_maxRate = 1.0 / maxRate;
    m_minRate = 1.0 / minRate;

    m_srcColor     = srcColor;
    m_dstColor     = dstColor;
    m_minColorRate = minColorRate;
    m_maxColorRate = maxColorRate;

    m_minWavyDist = minWavyDist;
    m_maxWavyDist = maxWavyDist;
    m_minLife     = minLife;
    m_maxLife     = maxLife;

    m_wavy = wavy;

    m_particles = particles;

    m_gravityWellStrength = gravityWellStrength;
    m_gravityWell         = gravityWell;

    m_ballLightning          = ballLightning;
    m_minBallLightningRadius = minBallLightningRadius;
    m_lightningVar           = lightningVar;
    m_maxBallLightningRadius = maxBallLightningRadius;
    m_lightningSubdivisions  = lightningSubdivisions;

    m_swarm          = swarm;
    m_swarmfrequency = swarmfreq;
    m_swarmdelta     = swarmdelta;

    m_time     = tr.refdef.time;
    m_lasttime = m_time;

    m_gravityEffectVector = vec_zero;
}

/*
====================
ParticleEmitter::ParticleEmitter
====================
*/
ParticleEmitter::ParticleEmitter()
{
    m_lasttime = tr.refdef.time / 1000.0;
}

/*
====================
ParticleEmitter::~ParticleEmitter
====================
*/
ParticleEmitter::~ParticleEmitter() {}

/*
====================
ParticleEmitter::Emit

Creates new particles based on the current time
====================
*/
void ParticleEmitter::Emit(float currentTime)
{
    Particle *p;
    float     rate;
    float     delta;

    rate  = GetRate();
    delta = currentTime = m_lasttime;

    if (!rate) {
        m_lasttime = currentTime;
        return;
    }

    for (; delta > rate; delta -= rate) {
        p = new Particle(
            m_position,
            GetVelocity(),
            GetAcceleration(),
            GetSrcColor(),
            GetDstColor(),
            GetColorRate(),
            IsWavy(),
            GetWavyDistance(),
            currentTime,
            GetDieTime(currentTime),
            m_maxSpeed,
            GetPosition(),
            IsSwarm(),
            m_swarmfrequency,
            m_swarmdelta
        );

        particleList.AddObject(p);

        m_lasttime = currentTime;
    }
}

/*
====================
ParticleEmitter::UpdateValues

Updates all particles with the last renderer time
====================
*/
void ParticleEmitter::UpdateValues()
{
    int i;
    int numParticles;

    numParticles = particleList.NumObjects();
    for (i = 1; i <= numParticles; i++) {
        Particle *p = particleList.ObjectAt(i);

        p->Update(m_lasttime);
    }
}

/*
====================
ParticleEmitter::GetPosition

Returns the current particle position
====================
*/
Vector ParticleEmitter::GetPosition()
{
    return m_position;
}

/*
====================
ParticleEmitter::GetVelocity

Returns the random velocity based on random angle, and random speed
====================
*/
Vector ParticleEmitter::GetVelocity()
{
    float  var;
    float  angle;
    Vector v;

    //
    // Calculate the direction from random angle
    //
    angle = RandomizeAngle(m_angle, m_angleVar);
    v     = CalculateDirection(angle);

    //
    // Calculate the velocity from random speed
    //
    var = RandomizeRange(m_minSpeed, m_maxSpeed);
    v *= var;

    return v;
}

/*
====================
ParticleEmitter::GetVelocityDirection

Returns the direction of the random velocity
====================
*/
Vector ParticleEmitter::GetVelocityDirection()
{
    float  var;
    float  angle;
    Vector v;

    //
    // Calculate the direction from random angle
    //
    angle = RandomizeAngle(m_angle, m_angleVar);
    v     = CalculateDirection(angle);

    //
    // Calculate the velocity from random speed
    //
    var = RandomizeRange(m_minSpeed, m_maxSpeed);
    v *= var;

    v.normalize();

    return v;
}

/*
====================
ParticleEmitter::GetVelocityDirectionMin

Returns the random velocity direction based on random angle, and minimum speed
====================
*/
Vector ParticleEmitter::GetVelocityDirectionMin()
{
    float  angle;
    Vector v;

    //
    // Calculate the direction from random angle
    //
    angle = RandomizeAngle(m_angle, m_angleVar);
    v     = CalculateDirection(angle);

    // Use the minimum speed
    v *= m_minSpeed;

    v.normalize();

    return v;
}

/*
====================
ParticleEmitter::GetVelocityDirectionMax

Returns the random velocity direction based on random angle, and maximum speed
====================
*/
Vector ParticleEmitter::GetVelocityDirectionMax()
{
    float  angle;
    Vector v;

    //
    // Calculate the direction from random angle
    //
    angle = RandomizeAngle(m_angle, m_angleVar);
    v     = CalculateDirection(angle);

    // Use the minimum speed
    v *= m_maxSpeed;

    v.normalize();

    return v;
}

/*
====================
ParticleEmitter::GetAcceleration

Returns the random acceleration based on random angle, and random speed
====================
*/
Vector ParticleEmitter::GetAcceleration()
{
    float  var;
    float  angle;
    Vector v;

    //
    // Calculate the direction from random angle
    //
    angle = RandomizeAngle(m_accAngle, m_accAngleVar);
    v     = CalculateDirection(angle);

    //
    // Calculate the velocity from random speed
    //
    var = RandomizeRange(m_minSpeed, m_maxSpeed);
    v += m_gravityEffectVector * var;

    return v;
}

/*
====================
ParticleEmitter::GetAccelerationDirection

Returns the direction of the random acceleration
====================
*/
Vector ParticleEmitter::GetAccelerationDirection()
{
    float  var;
    float  angle;
    Vector v;

    //
    // Calculate the direction from random angle
    //
    angle = RandomizeAngle(m_accAngle, m_accAngleVar);
    v     = CalculateDirection(angle);

    //
    // Calculate the velocity from random speed
    //
    var = RandomizeRange(m_minSpeed, m_maxSpeed);
    v += m_gravityEffectVector * var;

    v.normalize();

    return v;
}

/*
====================
ParticleEmitter::GetAccelerationDirectionMin

Returns the random acceleration direction based on random angle, and minimum speed
====================
*/
Vector ParticleEmitter::GetAccelerationDirectionMin()
{
    float  angle;
    Vector v;

    //
    // Calculate the direction from random angle
    //
    angle = RandomizeAngle(m_accAngle, m_accAngleVar);
    v     = CalculateDirection(angle);

    // Use the minimum speed
    v += m_gravityEffectVector * m_minSpeed;

    v.normalize();

    return v;
}

/*
====================
ParticleEmitter::GetAccelerationDirectionMax

Returns the random acceleration direction based on random angle, and maximum speed
====================
*/
Vector ParticleEmitter::GetAccelerationDirectionMax()
{
    float  angle;
    Vector v;

    //
    // Calculate the direction from random angle
    //
    angle = RandomizeAngle(m_accAngle, m_accAngleVar);
    v     = CalculateDirection(angle);

    // Use the maximum speed
    v += m_gravityEffectVector * m_minSpeed;

    v.normalize();

    return v;
}

/*
====================
ParticleEmitter::GetSrcColor

Returns the source color
====================
*/
int ParticleEmitter::GetSrcColor()
{
    return m_srcColor;
}

/*
====================
ParticleEmitter::GetDstColor

Returns the destination color
====================
*/
int ParticleEmitter::GetDstColor()
{
    return m_dstColor;
}

/*
====================
ParticleEmitter::GetGravityWellStrength

Returns the strength of the gravity
====================
*/
float ParticleEmitter::GetGravityWellStrength()
{
    return m_gravityWellStrength;
}

/*
====================
ParticleEmitter::GetRate

Returns a random rate between minimum and maximum
====================
*/
float ParticleEmitter::GetRate()
{
    return RandomizeRange(m_minRate, m_maxRate);
}

/*
====================
ParticleEmitter::GetColorRate

Returns a random color between minimum and maximum
====================
*/
float ParticleEmitter::GetColorRate()
{
    return RandomizeRange(m_minColorRate, m_maxColorRate);
}

/*
====================
ParticleEmitter::GetWavyDistance

Returns a random wavy distance between minimum and maximum
====================
*/
float ParticleEmitter::GetWavyDistance()
{
    return RandomizeRange(m_minWavyDist, m_maxWavyDist);
}

/*
====================
ParticleEmitter::GetDieTime

Returns a random time at which the particle will stop, between minimum and maximum
====================
*/
float ParticleEmitter::GetDieTime(float currentTime)
{
    return currentTime + RandomizeRange(m_minLife, m_maxLife);
}

/*
====================
ParticleEmitter::GetBallLightningRadius

Returns a random lightning radius between minimum and maximum
====================
*/
int ParticleEmitter::GetBallLightningRadius()
{
    return RandomizeRange(m_minBallLightningRadius, m_maxBallLightningRadius);
}

/*
====================
ParticleEmitter::GetLightningVar

Returns the maximum lightning value
====================
*/
float ParticleEmitter::GetLightningVar()
{
    return m_lightningVar;
}

/*
====================
ParticleEmitter::GetLightningSubdivisions

Returns the number of lightning subdivisions
====================
*/
int ParticleEmitter::GetLightningSubdivisions()
{
    return m_lightningSubdivisions;
}

/*
====================
ParticleEmitter::IsWavy

Returns whether or not this is a wavy particle
====================
*/
qboolean ParticleEmitter::IsWavy()
{
    return m_wavy;
}

/*
====================
ParticleEmitter::IsGravityWell

Returns whether or not this particle has gravity
====================
*/
qboolean ParticleEmitter::IsGravityWell()
{
    return m_gravityWell;
}

/*
====================
ParticleEmitter::IsParticles

Returns whether or not this contains particles
====================
*/
qboolean ParticleEmitter::IsParticles()
{
    return m_particles;
}

/*
====================
ParticleEmitter::IsBallLightning

Returns whether or not ball lightning can be applied
====================
*/
qboolean ParticleEmitter::IsBallLightning()
{
    return m_ballLightning;
}

/*
====================
ParticleEmitter::IsSwarm

Returns whether or not this is a swarm of particles
====================
*/
qboolean ParticleEmitter::IsSwarm()
{
    return m_swarm;
}

/*
====================
ParticleEmitter::SetMinSpeed

Sets the minimum speed
====================
*/
void ParticleEmitter::SetMinSpeed(float value)
{
    m_minSpeed = value;
}

/*
====================
ParticleEmitter::SetMaxSpeed

Sets the maximum speed
====================
*/
void ParticleEmitter::SetMaxSpeed(float value)
{
    m_maxSpeed = value;
}

/*
====================
ParticleEmitter::SetAngle

Sets the angle/minimum angle
====================
*/
void ParticleEmitter::SetAngle(float value)
{
    m_angle = value;
}

/*
====================
ParticleEmitter::SetAngleVar

Sets the maximum speed
====================
*/
void ParticleEmitter::SetAngleVar(float value)
{
    m_angleVar = value;
}

/*
====================
ParticleEmitter::SetMinAccSpeed

Sets the minimum acceleration speed
====================
*/
void ParticleEmitter::SetMinAccSpeed(float value)
{
    m_minAccSpeed = value;
}

/*
====================
ParticleEmitter::SetMaxAccSpeed

Sets the maximum acceleration speed
====================
*/
void ParticleEmitter::SetMaxAccSpeed(float value)
{
    m_maxAccSpeed = value;
}

/*
====================
ParticleEmitter::SetAccAngle

Sets the acceleration angle / minimum acceleration angle
====================
*/
void ParticleEmitter::SetAccAngle(float value)
{
    m_accAngle = value;
}

/*
====================
ParticleEmitter::SetAccAngleVar

Sets the maximum acceleration speed
====================
*/
void ParticleEmitter::SetAccAngleVar(float value)
{
    m_accAngleVar = value;
}

/*
====================
ParticleEmitter::SetMinRate

Sets the minimum rate
====================
*/
void ParticleEmitter::SetMinRate(float value)
{
    m_minRate = value;
}

/*
====================
ParticleEmitter::SetMaxRate

Sets the maximum rate
====================
*/
void ParticleEmitter::SetMaxRate(float value)
{
    m_maxRate = value;
}

/*
====================
ParticleEmitter::SetSrcColor

Sets the source color
====================
*/
void ParticleEmitter::SetSrcColor(int value)
{
    m_srcColor = value;
}

/*
====================
ParticleEmitter::SetDstColor

Sets the destination color
====================
*/
void ParticleEmitter::SetDstColor(int value)
{
    m_dstColor = value;
}

/*
====================
ParticleEmitter::SetMinColorRate

Sets the minimum color rate
====================
*/
void ParticleEmitter::SetMinColorRate(float value)
{
    m_minColorRate = value;
}

/*
====================
ParticleEmitter::SetMaxColorRate

Sets the maximum color rate
====================
*/
void ParticleEmitter::SetMaxColorRate(float value)
{
    m_maxColorRate = value;
}

/*
====================
ParticleEmitter::SetWavy

Sets whether or not this is a wavy particle
====================
*/
void ParticleEmitter::SetWavy(int value)
{
    m_wavy = value;
}

/*
====================
ParticleEmitter::SetParticles

Sets whether or not this contains particles
====================
*/
void ParticleEmitter::SetParticles(int value)
{
    m_particles = value;
}

/*
====================
ParticleEmitter::SetGravityWell

Sets whether or not this has gravity
====================
*/
void ParticleEmitter::SetGravityWell(int value)
{
    m_gravityWell = value;
}

/*
====================
ParticleEmitter::SetMinWavyDist

Sets the minimum wavy distance
====================
*/
void ParticleEmitter::SetMinWavyDist(float value)
{
    m_minWavyDist = value;
}

/*
====================
ParticleEmitter::SetMaxWavyDist

Sets the maximum wavy distance
====================
*/
void ParticleEmitter::SetMaxWavyDist(float value)
{
    m_maxWavyDist = value;
}

/*
====================
ParticleEmitter::SetMinLife

Sets the minimum life
====================
*/
void ParticleEmitter::SetMinLife(float value)
{
    m_minLife = value;
}

/*
====================
ParticleEmitter::SetMaxLife

Sets the maximum life
====================
*/
void ParticleEmitter::SetMaxLife(float value)
{
    m_maxLife = value;
}

/*
====================
ParticleEmitter::SetGravityWellStrength

Sets the strength of the gravity
====================
*/
void ParticleEmitter::SetGravityWellStrength(float value)
{
    m_gravityWellStrength = value;
}

/*
====================
ParticleEmitter::SetGravityEffectVector

Sets the direction at which gravity is applied
====================
*/
void ParticleEmitter::SetGravityEffectVector(Vector value)
{
    m_gravityEffectVector = value;
}

/*
====================
ParticleEmitter::SetBallLightning

Sets whether or not it has ball lightning
====================
*/
void ParticleEmitter::SetBallLightning(int value)
{
    m_ballLightning = value;
}

/*
====================
ParticleEmitter::SetMinBallLightningRadius

Sets the minimum ball lightning radius
====================
*/
void ParticleEmitter::SetMinBallLightningRadius(int value)
{
    m_minBallLightningRadius = value;
}

/*
====================
ParticleEmitter::SetMaxBallLightningRadius

Sets the maximum ball lightning radius
====================
*/
void ParticleEmitter::SetMaxBallLightningRadius(int value)
{
    m_maxBallLightningRadius = value;
}

/*
====================
ParticleEmitter::SetLightningVar

Sets the maximum lightning value
====================
*/
void ParticleEmitter::SetLightningVar(float value)
{
    m_lightningVar = value;
}

/*
====================
ParticleEmitter::SetLightningSubdivisions

Sets the number of subdivisions for the lightning
====================
*/
void ParticleEmitter::SetLightningSubdivisions(int value)
{
    m_lightningSubdivisions = value;
}

/*
====================
ParticleEmitter::RandomizeRange

Returns a random value between the range [min, max]
====================
*/
float ParticleEmitter::RandomizeRange(float min, float max)
{
    return min + (max - min) * (rand() / (float)RAND_MAX);
}

/*
====================
ParticleEmitter::RandomizeAngle

Returns a random angle between the range [min, max]
====================
*/
float ParticleEmitter::RandomizeAngle(float min, float max)
{
    return min + ((rand() - (RAND_MAX / 2)) / (float)RAND_MAX) * max;
}

/*
====================
ParticleEmitter::CalculateDirection

Returns a random Vector direction based on the angle value
====================
*/
Vector ParticleEmitter::CalculateDirection(float value)
{
    float deg = DEG2RAD(value);

    return Vector(cos(deg), -sin(deg), 0);
}

/*
====================
ParticleEmitter::Load

Loads the specified particle emitter from the string buffer
====================
*/
void ParticleEmitter::Load(char **buf_p)
{
    m_position[0] = atoi(COM_ParseExt(buf_p, qtrue));
    m_position[1] = atoi(COM_ParseExt(buf_p, qtrue));
    m_position[2] = atoi(COM_ParseExt(buf_p, qtrue));

    m_srcColor     = atoi(COM_ParseExt(buf_p, qtrue));
    m_dstColor     = atoi(COM_ParseExt(buf_p, qtrue));
    m_minColorRate = atoi(COM_ParseExt(buf_p, qtrue));
    m_maxColorRate = atoi(COM_ParseExt(buf_p, qtrue));

    m_minRate = atof(COM_ParseExt(buf_p, qtrue));
    m_maxRate = atof(COM_ParseExt(buf_p, qtrue));
    m_minLife = atoi(COM_ParseExt(buf_p, qtrue));
    m_maxLife = atoi(COM_ParseExt(buf_p, qtrue));

    m_angle       = atoi(COM_ParseExt(buf_p, qtrue));
    m_angleVar    = atoi(COM_ParseExt(buf_p, qtrue));
    m_accAngle    = atoi(COM_ParseExt(buf_p, qtrue));
    m_accAngleVar = atoi(COM_ParseExt(buf_p, qtrue));

    m_minSpeed    = atof(COM_ParseExt(buf_p, qtrue));
    m_maxSpeed    = atof(COM_ParseExt(buf_p, qtrue));
    m_minAccSpeed = atof(COM_ParseExt(buf_p, qtrue));
    m_maxAccSpeed = atof(COM_ParseExt(buf_p, qtrue));

    m_minWavyDist = atof(COM_ParseExt(buf_p, qtrue));
    m_maxWavyDist = atof(COM_ParseExt(buf_p, qtrue));
    m_wavy        = atoi(COM_ParseExt(buf_p, qtrue));

    m_particles = atoi(COM_ParseExt(buf_p, qtrue));

    m_gravityWell         = atoi(COM_ParseExt(buf_p, qtrue));
    m_gravityWellStrength = atof(COM_ParseExt(buf_p, qtrue));

    m_ballLightning          = atoi(COM_ParseExt(buf_p, qtrue));
    m_minBallLightningRadius = atof(COM_ParseExt(buf_p, qtrue));
    m_maxBallLightningRadius = atof(COM_ParseExt(buf_p, qtrue));
    m_lightningVar           = atof(COM_ParseExt(buf_p, qtrue));
    m_lightningSubdivisions  = atof(COM_ParseExt(buf_p, qtrue));

    m_swarm          = atoi(COM_ParseExt(buf_p, qtrue));
    m_swarmfrequency = atoi(COM_ParseExt(buf_p, qtrue));
    m_swarmdelta     = atoi(COM_ParseExt(buf_p, qtrue));
}

/*
====================
GhostTexture::GhostTexture
====================
*/
GhostTexture::GhostTexture()
{
    m_image   = NULL;
    m_texture = NULL;
}

/*
====================
GhostTexture::SetTexel

Sets color at the specified xy texel location
====================
*/
void GhostTexture::SetTexel(int x, int y, unsigned int color)
{
    m_texture[x + m_width * y] = color;
}

/*
====================
GhostTexture::Burn

Applies fade/burn effect to the ghost texture
====================
*/
void GhostTexture::Burn()
{
    int     x, y;
    int     r, g, b;
    GLint   pixel;
    GLuint *line;

    if (m_isfade) {
        for (y = 1; y < m_height - 1; y++) {
            line = &m_texture[m_width * y];

            for (x = 1; x < m_width - 1; x++) {
                r = (line[x] & 0xff) - m_faderate;
                g = ((line[x] & 0xff00) >> 8) - m_faderate;
                b = ((line[x] & 0xff0000) >> 16) - m_faderate;

                if (r < 0) {
                    r = 0;
                }
                if (g < 0) {
                    g = 0;
                }
                if (b < 0) {
                    b = 0;
                }

                line[x] = (r) | (g << 8) | (b << 16);
            }
        }
    } else if (m_isburn) {
        for (y = 1; y < m_height - 1; y++) {
            line = &m_texture[m_width * y];

            for (x = 1; x < m_width - 1; x++) {
                r = (((line[m_width + x] & 0xff) + (line[x + 1] & 0xff) + (line[x - 1] & 0xff) + (line[x] & 0xff)) >> 2)
                  - m_burnrate;
                g = ((((line[m_width + x] & 0xFF00) >> 8) + ((line[x + 1] & 0xFF00) >> 8)
                      + ((line[x - 1] & 0xFF00) >> 8) + ((line[x] & 0xFF00) >> 8))
                     >> 2)
                  - m_burnrate;
                b = ((((line[m_width + x] & 0xFF0000) >> 16) + ((line[x + 1] & 0xFF0000) >> 16)
                      + ((line[x - 1] & 0xFF0000) >> 16) + ((line[x] & 0xFF0000) >> 16))
                     >> 2)
                  - m_burnrate;

                if (r < 0) {
                    r = 0;
                }
                if (g < 0) {
                    g = 0;
                }
                if (b < 0) {
                    b = 0;
                }

                pixel = (r) | (g << 8) | (b << 16);
                if (pixel > 0) {
                    line[x] = pixel;
                }
            }
        }
    }

    line = m_texture;

    for (x = 0; x < m_width; x++) {
        line[x] = 0;
    }

    for (y = 0; y < m_height; ++y) {
        line[m_width * y]               = 0;
        line[m_width - 1 + m_width * y] = 0;
    }

    line = &m_texture[m_width * (m_height - 1)];

    for (x = 0; x < m_width; x++) {
        line[x] = 0;
    }
}

/*
====================
GhostTexture::ComputeOutCode

Returns a flag value indicating if x/y variable reached xmin/xmax
====================
*/
outcode GhostTexture::ComputeOutCode(int x, int y, int xmin, int xmax, int ymin, int ymax)
{
    outcode code = 0;

    if (y > ymax) {
        code = 1;
    } else if (y < ymin) {
        code = 2;
    }

    if (x > xmax) {
        code |= 4;
    } else if (x < xmin) {
        code |= 8;
    }

    return code;
}

/*
====================
GhostTexture::ClipAndDrawLine

Draws a line from p0 to p1 with clipping
====================
*/
void GhostTexture::ClipAndDrawLine(Vector p0, Vector p1, int color)
{
    int     x0, x1;
    int     y0, y1;
    int     d;
    int     x, y;
    int     ax, ay;
    int     sx, sy;
    int     dx, dy;
    int     xmax, ymax;
    bool    accept;
    outcode outcode0;
    outcode outcode1;

    x0   = p0.x;
    y0   = p0.y;
    x1   = p1.x;
    y1   = p1.y;
    xmax = m_width - 1;
    ymax = m_height - 1;

    accept = false;

    outcode0 = ComputeOutCode(x0, y0, 0, xmax, 0, ymax);
    outcode1 = ComputeOutCode(x1, y1, 0, xmax, 0, ymax);

    while (outcode1 | outcode0) {
        outcode outCodeOut;
        if (outcode0 & outcode1) {
            break;
        }

        outCodeOut = outcode0 ? outcode0 : outcode1;

        if (outCodeOut & 1) {
            x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
            y = ymax;
        } else if (outCodeOut & 2) {
            x = x0 - y0 * (x1 - x0) / (y1 - y0);
            y = 0;
        } else if (outCodeOut & 4) {
            x = xmax;
            y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
        } else if (outCodeOut * 8) {
            x = 0;
            y = y0 - x0 * (y1 - y0) / (x1 - x0);
        } else {
            x = x0;
            y = y0;
        }
        if (outCodeOut == outcode0) {
            y0       = y;
            x0       = x;
            outcode0 = ComputeOutCode(x, y, 0, xmax, 0, ymax);
        } else {
            x1       = x;
            y1       = y;
            outcode1 = ComputeOutCode(x, y, 0, xmax, 0, ymax);
        }
    }

    accept = true;
    if (!accept) {
        return;
    }

    dx = x1 - x0;
    if (dx < 0) {
        dx = x0 - x1;
    }
    ax = dx * 2;
    sx = dx >= 0 ? 1 : -1;

    dy = y1 - y0;
    if (dy < 0) {
        dy = y0 - y1;
    }
    ay = dy * 2;
    sy = dy >= 0 ? 1 : -1;

    x = x0;
    y = y0;

    if (ax > ay) {
        for (d = ay - (ax >> 1);; d += ay, x += sx) {
            SetTexel(x, y, color);
            if (x == x1) {
                break;
            }

            if (d >= 0) {
                y += sy;
                d -= ax;
            }
        }
    } else {
        for (d = ax - (ay >> 1);; d += ax, y += sy) {
            SetTexel(x, y, color);
            if (y == y1) {
                break;
            }

            if (d >= 0) {
                x += sx;
                d -= ay;
            }
        }
    }
}

/*
====================
GhostTexture::RotateVector

Rotates the vector from angle
====================
*/
Vector GhostTexture::RotateVector(Vector v, float angle)
{
    Vector vec;
    float  rad;

    rad   = DEG2RAD(angle);
    vec.x = cos(rad) * v.x - sin(rad) * v.y;
    vec.y = cos(rad) * v.y + sin(rad) * v.x;
    vec.z = 0;

    return vec;
}

/*
====================
GhostTexture::GenerateLightning

Generates a lightning effect
====================
*/
void GhostTexture::GenerateLightning(
    Vector p1, Vector p2, int color, float angleVar, int numSubdivisions, int maxSubdivisions
)
{
    Vector mid;
    Vector delta;
    float  length;

    if (numSubdivisions == maxSubdivisions) {
        p1[0] += 0.5;
        p1[1] += 0.5;
        p2[0] += 0.5;
        p2[1] += 0.5;
        ClipAndDrawLine(p1, p2, color);

        return;
    }

    delta  = p2 - p1;
    length = delta.length() * 0.5;
    delta.normalize();

    mid = RotateVector(delta, RandomizeRange(-angleVar, angleVar));
    mid = p1 + mid * length;

    GenerateLightning(p1, mid, color, angleVar, numSubdivisions + 1, maxSubdivisions);
    GenerateLightning(mid, p2, color, angleVar, numSubdivisions + 1, maxSubdivisions);
}

/*
====================
GhostTexture::Update

Updates the ghost texture
====================
*/
void GhostTexture::Update()
{
    Vector pos;
    int    i, j;
    int    numEmitters;
    float  currentTime;

    currentTime = tr.refdef.time / 1000.0;

    numEmitters = m_emitterList.NumObjects();

    for (i = 1; i <= numEmitters; i++) {
        ParticleEmitter *pe = m_emitterList.ObjectAt(i);

        if (pe->IsGravityWell()) {
            break;
        }

        if (pe->IsBallLightning()) {
            Vector p1, p2;

            if (pe->GetBallLightningRadius()) {
                p1 = pe->GetPosition();
                p2 = pe->GetVelocityDirection();

                p2 = p1 + p2 * pe->GetBallLightningRadius();
                GenerateLightning(p1, p2, pe->GetSrcColor(), pe->GetLightningVar(), 0, pe->GetLightningSubdivisions());
            }
        } else if (pe->IsParticles()) {
            pe->Emit(currentTime);
        } else {
            Vector p = pe->GetPosition();

            SetTexel(p.x, p.y, pe->GetSrcColor());
        }

        for (j = pe->particleList.NumObjects(); j > 0; j--) {
            Particle *p = pe->particleList.ObjectAt(j);

            p->Update(currentTime);
            if (p->m_position.x < m_width && p->m_position.x >= 0 && p->m_position.y < m_height && p->m_position.y >= 0
                && currentTime <= p->GetDieTime()) {
                SetTexel(p->m_position.x, p->m_position.y, p->m_color);
            } else {
                delete p;
                pe->particleList.RemoveObjectAt(j);
            }
        }
    }
}

/*
====================
R_UpdateGhostTextures

Updates all ghost textures
====================
*/
void R_UpdateGhostTextures()
{
    int i;
    int numTextures;

    frameTime = tr.refdef.time - lastTime;
    lastTime  = tr.refdef.time;

    numTextures = ghostManager.m_textureList.NumObjects();
    for (i = 1; i <= numTextures; i++) {
        GhostTexture *gt = ghostManager.m_textureList.ObjectAt(i);

        gt->Update();
        gt->Burn();

        GL_Bind(gt->m_image);
        qglTexImage2D(GL_TEXTURE_2D, 0, 3, gt->m_width, gt->m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, gt->m_texture);
    }
}

/*
====================
R_SetGhostImage

Applies an image to the specified named ghost texture
====================
*/
void R_SetGhostImage(const char *name, image_t *image)
{
    int i;
    int numTextures;

    numTextures = ghostManager.m_textureList.NumObjects();
    for (i = 1; i <= numTextures; i++) {
        GhostTexture *gt = ghostManager.m_textureList.ObjectAt(i);

        if (gt->m_name == name) {
            gt->m_image = image;
        }
    }
}

/*
====================
LoadGHOST

Loads a GHOST image file
====================
*/
void LoadGHOST(const char *name, byte **pic, int *width, int *height)
{
    char         *buf_p;
    char         *buffer;
    int           numPixels;
    int           numParticles;
    int           i;
    GhostTexture *gt;

    if (ri.FS_ReadFile(name, (void **)&buffer) == -1) {
        return;
    }

    buf_p = buffer;
    COM_ParseExt(&buf_p, qtrue);

    *width  = atoi(COM_ParseExt(&buf_p, qtrue));
    *height = atoi(COM_ParseExt(&buf_p, qtrue));

    numParticles = atoi(COM_ParseExt(&buf_p, qtrue));
    numPixels    = *height * *width;
    *pic         = new byte[numPixels * 4];

    gt            = new GhostTexture();
    gt->m_width   = *width;
    gt->m_height  = *height;
    gt->m_texture = new unsigned int[numPixels];

    gt->m_isburn   = atoi(COM_ParseExt(&buf_p, qtrue));
    gt->m_burnrate = atof(COM_ParseExt(&buf_p, qtrue));

    gt->m_isfade   = atoi(COM_ParseExt(&buf_p, qtrue));
    gt->m_faderate = atof(COM_ParseExt(&buf_p, qtrue));

    ghostManager.m_textureList.AddObject(gt);

    for (i = 0; i < numParticles; i++) {
        ParticleEmitter *pe = new ParticleEmitter();
        gt->m_emitterList.AddObject(pe);
        pe->Load(&buf_p);
    }
}
