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

float RandomizeRange(float min, float max)
{
    return min + (max - min) * (rand() / (float)RAND_MAX);
}

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

void Particle::Update(float currentTime)
{
    int    r, g, b;
    float  ftime;
    float  time2;
    float  factor;
    Vector offset;

    // FIXME: unimplemented
}

float Particle::GetDieTime()
{
    return m_dieTime;
}

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

ParticleEmitter::ParticleEmitter()
{
    m_lasttime = tr.refdef.time / 1000.0;
}

ParticleEmitter::~ParticleEmitter() {}

void ParticleEmitter::Emit(float currentTime)
{
    Particle *p;
    float     rate;
    float     delta;

    // FIXME: unimplemented
}

void ParticleEmitter::UpdateValues()
{
    // FIXME: unimplemented
}

Vector ParticleEmitter::GetPosition()
{
    return m_position;
}

Vector ParticleEmitter::GetVelocity()
{
    // FIXME: unimplemented
    return Vector();
}

Vector ParticleEmitter::GetVelocityDirection()
{
    // FIXME: unimplemented
    return Vector();
}

Vector ParticleEmitter::GetVelocityDirectionMin()
{
    // FIXME: unimplemented
    return Vector();
}

Vector ParticleEmitter::GetVelocityDirectionMax()
{
    // FIXME: unimplemented
    return Vector();
}

Vector ParticleEmitter::GetAcceleration()
{
    // FIXME: unimplemented
    return Vector();
}

Vector ParticleEmitter::GetAccelerationDirection()
{
    // FIXME: unimplemented
    return Vector();
}

Vector ParticleEmitter::GetAccelerationDirectionMin()
{
    // FIXME: unimplemented
    return Vector();
}

Vector ParticleEmitter::GetAccelerationDirectionMax()
{
    // FIXME: unimplemented
    return Vector();
}

int ParticleEmitter::GetSrcColor()
{
    return m_srcColor;
}

int ParticleEmitter::GetDstColor()
{
    return m_dstColor;
}

float ParticleEmitter::GetGravityWellStrength()
{
    return m_gravityWellStrength;
}

float ParticleEmitter::GetRate()
{
    // FIXME: unimplemented
    return 0;
}

float ParticleEmitter::GetColorRate()
{
    // FIXME: unimplemented
    return 0;
}

float ParticleEmitter::GetWavyDistance()
{
    // FIXME: unimplemented
    return 0;
}

float ParticleEmitter::GetDieTime(float currentTime)
{
    // FIXME: unimplemented
    return 0;
}

int ParticleEmitter::GetBallLightningRadius()
{
    // FIXME: unimplemented
    return 0;
}

float ParticleEmitter::GetLightningVar()
{
    // FIXME: unimplemented
    return 0;
}

int ParticleEmitter::GetLightningSubdivisions()
{
    // FIXME: unimplemented
    return 0;
}

qboolean ParticleEmitter::IsWavy()
{
    return m_wavy;
}

qboolean ParticleEmitter::IsGravityWell()
{
    return m_gravityWell;
}

qboolean ParticleEmitter::IsParticles()
{
    return m_particles;
}

qboolean ParticleEmitter::IsBallLightning()
{
    return m_ballLightning;
}

qboolean ParticleEmitter::IsSwarm()
{
    return m_swarm;
}

void ParticleEmitter::SetMinSpeed(float value)
{
    m_minSpeed = value;
}

void ParticleEmitter::SetMaxSpeed(float value)
{
    m_maxSpeed = value;
}

void ParticleEmitter::SetAngle(float value)
{
    m_angle = value;
}

void ParticleEmitter::SetAngleVar(float value)
{
    m_angleVar = value;
}

void ParticleEmitter::SetMinAccSpeed(float value)
{
    m_minAccSpeed = value;
}

void ParticleEmitter::SetMaxAccSpeed(float value)
{
    m_maxAccSpeed = value;
}

void ParticleEmitter::SetAccAngle(float value)
{
    m_accAngle = value;
}

void ParticleEmitter::SetAccAngleVar(float value)
{
    m_accAngleVar = value;
}

void ParticleEmitter::SetMinRate(float value)
{
    m_minRate = value;
}

void ParticleEmitter::SetMaxRate(float value)
{
    m_maxRate = value;
}

void ParticleEmitter::SetSrcColor(int value)
{
    m_srcColor = value;
}

void ParticleEmitter::SetDstColor(int value)
{
    m_dstColor = value;
}

void ParticleEmitter::SetMinColorRate(float value)
{
    m_minColorRate = value;
}

void ParticleEmitter::SetMaxColorRate(float value)
{
    m_maxColorRate = value;
}

void ParticleEmitter::SetWavy(int value)
{
    m_wavy = value;
}

void ParticleEmitter::SetParticles(int value)
{
    m_particles = value;
}

void ParticleEmitter::SetGravityWell(int value)
{
    m_gravityWell = value;
}

void ParticleEmitter::SetMinWavyDist(float value)
{
    m_minWavyDist = value;
}

void ParticleEmitter::SetMaxWavyDist(float value)
{
    m_maxWavyDist = value;
}

void ParticleEmitter::SetMinLife(float value)
{
    m_minLife = value;
}

void ParticleEmitter::SetMaxLife(float value)
{
    m_maxLife = value;
}

void ParticleEmitter::SetGravityWellStrength(float value)
{
    m_gravityWellStrength = value;
}

void ParticleEmitter::SetGravityEffectVector(Vector value)
{
    m_gravityEffectVector = value;
}

void ParticleEmitter::SetBallLightning(int value)
{
    m_ballLightning = value;
}

void ParticleEmitter::SetMinBallLightningRadius(int value)
{
    m_minBallLightningRadius = value;
}

void ParticleEmitter::SetMaxBallLightningRadius(int value)
{
    m_maxBallLightningRadius = value;
}

void ParticleEmitter::SetLightningVar(float value)
{
    m_lightningVar = value;
}

void ParticleEmitter::SetLightningSubdivisions(int value)
{
    m_lightningSubdivisions = value;
}

float ParticleEmitter::RandomizeRange(float min, float max)
{
    return min + (max - min) * (rand() / (float)RAND_MAX);
}

float ParticleEmitter::RandomizeAngle(float min, float max)
{
    return min + (max - min) * (rand() / (float)RAND_MAX);
}

Vector ParticleEmitter::CalculateDirection(float value)
{
    // FIXME: unimplemented
    return Vector();
}

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

GhostTexture::GhostTexture()
{
    m_image   = NULL;
    m_texture = NULL;
}

void GhostTexture::SetTexel(int x, int y, unsigned int color)
{
    m_texture[x + m_width * y] = color;
}

void GhostTexture::Burn()
{
    int     x, y;
    int     r, g, b;
    GLint   pixel;
    GLuint *line;

    // FIXME: unimplemented
}

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

    x0     = p0.x;
    y0     = p0.y;
    x1     = p1.x;
    y1     = p1.y;
    accept = false;
    xmax   = this->m_width - 1;
    ymax   = this->m_height - 1;

    outcode0 = ComputeOutCode(x0, y0, 0, xmax, 0, ymax);
    outcode0 = ComputeOutCode(x1, y1, 0, xmax, 0, ymax);

    // FIXME: unimplemented
}

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

void GhostTexture::Update()
{
    Vector pos;
    int    i, j;
    int    numEmitters;
    float  currentTime;

    // FIXME: unimplemented
}

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
