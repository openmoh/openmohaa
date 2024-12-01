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

// tr_sun_flare.cpp: Sun flares

#include "tr_local.h"

#define MAX_FLARES 20

class flare_s
{
public:
    float     size;
    float     where;
    shader_t *shader;
    float     alphascale;
};

class lens_flare
{
public:
    static int max_flares;

    float dot_min;
    float fullscale;

    // List of flares
    int     num_flares;
    flare_s flares[MAX_FLARES];

    shader_t *fullscreen;
    int       fullfade;
    qboolean  blend_active;

    // Flare location
    vec3_t trace_v;
    vec3_t v;

    // Color and alpha
    vec3_t color;
    float  alpha;

    // Screen projection
    vec2_t screen;
    float  dot;

    int   lasttime;
    float lastbright;

    // Whether or not it's inside a portal sky
    bool inportalsky;

    qboolean initted;

public:
    lens_flare()
    {
        color[0] = 1.0;
        color[1] = 1.0;
        color[2] = 1.0;
        alpha    = 1.0;
        initted  = false;
    }

    bool         CheckRange();
    bool         CheckRay();
    bool         ScreenCalc();
    virtual void Try();
    virtual void Init(const char *which);
    void         ScreenBlend();
    void         InPortalSky();
    void         NotInPortalSky();
    void         SetVect(const float *vect);

    void SetColor(const float *c) { VectorCopy(c, color); }

    void SetAlpha(float a) { alpha = a; }
};

class dlight_lens_flare : public lens_flare
{
public:
    shader_t *corona_shader;

public:
    dlight_lens_flare() { corona_shader = NULL; }

    void Try() override;

    void Init(const char *which) override
    {
        lens_flare::Init(which);
        corona_shader = R_FindShader("textures/sprites/corona", -1, qfalse, qfalse, qfalse, qfalse);
    }
};

class sun_flare_class : public lens_flare
{
public:
    bool SunCheckRay();
    void SunScreenCalc();
    void SunTry();
};

lens_flare        torches;
dlight_lens_flare dlights;
sun_flare_class   sunFlare;
int               lens_flare::max_flares;

void lens_flare::SetVect(const float *vect)
{
    if (inportalsky) {
        vec3_t offset;
        vec3_t rot_offset;

        VectorSubtract(vect, tr.refdef.sky_origin, offset);
        VectorRotate(offset, tr.refdef.sky_axis, rot_offset);
        VectorAdd(tr.refdef.vieworg, rot_offset, v);

        VectorNormalize(offset);
        VectorMA(tr.refdef.vieworg, 16384, offset, trace_v);
    } else {
        VectorCopy(vect, v);
        VectorCopy(vect, trace_v);
    }
}

void lens_flare::InPortalSky()
{
    inportalsky = true;
}

void lens_flare::NotInPortalSky()
{
    inportalsky = false;
}

bool lens_flare::CheckRange()
{
    vec3_t diff;

    VectorSubtract(v, backEnd.viewParms.ori.origin, diff);
    VectorNormalizeFast(diff);
    dot = DotProduct(backEnd.viewParms.ori.axis[0], diff);

    return dot > dot_min;
}

bool lens_flare::CheckRay()
{
    trace_t trace;

    ri.CM_BoxTrace(&trace, backEnd.viewParms.ori.origin, trace_v, vec3_origin, vec3_origin, 0, CONTENTS_SOLID, qfalse);
    if (inportalsky) {
        return (trace.surfaceFlags & 4) != 0;
    }

    return trace.fraction == 1.0;
}

bool lens_flare::ScreenCalc()
{
    vec4_t eye;
    vec4_t clip;
    vec4_t point;
    bool   retval;
    int    i, j;

    retval = true;

    VectorCopy(v, point);
    point[3] = 1.0;

    VectorClear4(eye);
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            eye[i] += point[j] * tr.ori.modelMatrix[i + j * 4];
        }
    }

    VectorClear4(clip);
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            clip[i] += eye[j] * tr.viewParms.projectionMatrix[i + j * 4];
        }
    }

    for (i = 0; i < 3; i++) {
        if (clip[i] >= clip[3] || clip[i] <= -clip[3]) {
            retval = false;
        }
    }

    for (i = 0; i < 2; i++) {
        screen[i] = clip[i] / clip[3];
    }

    return retval;
}

void lens_flare::Try()
{
    vec2_t diff;
    vec2_t point;
    float  alpha;
    int    i;

    if (!ScreenCalc()) {
        return;
    }
    if (!CheckRange()) {
        return;
    }
    if (!CheckRay()) {
        return;
    }

    VectorScale2D(screen, -2.0, diff);
    alpha = (dot - dot_min) / (1.0 - dot_min);

    for (i = 0; i < num_flares; i++) {
        flare_s *flare = &flares[i];

        VectorMA2D(screen, flare->where, diff, point);

        RB_Color4f(color[0], color[1], color[2], alpha * this->alpha * flare->alphascale);
        RB_StreamBegin(flare->shader);
        RB_Texcoord2f(0.0, 0.0);

        RB_Vertex2f(point[0] - flare->size, point[1] + flare->size);
        RB_Texcoord2f(1.0, 0.0);

        RB_Vertex2f(point[0] + flare->size, point[1] + flare->size);
        RB_Texcoord2f(0.0, 1.0);

        RB_Vertex2f(point[0] - flare->size, point[1] - flare->size);
        RB_Texcoord2f(1.0, 1.0);

        RB_Vertex2f(point[0] + flare->size, point[1] - flare->size);
        RB_StreamEnd();
    }

    lasttime     = backEnd.refdef.time;
    blend_active = true;
    lastbright   = Square(alpha) * Square(alpha) * this->fullscale;
}

void dlight_lens_flare::Try()
{
    float size;

    size = r_lightcoronasize->value;
    lens_flare::Try();
    lens_flare::CheckRange();

    if (dot <= 0) {
        return;
    }

    if (!CheckRay()) {
        return;
    }

    RB_Color4f(color[0], color[1], color[2], alpha);
    RB_StreamBegin(corona_shader);
    RB_Texcoord2f(0.0, 0.0);

    RB_Vertex2f(screen[0] - size, screen[1] + size);
    RB_Texcoord2f(1.0, 0.0);

    RB_Vertex2f(screen[0] + size, screen[1] + size);
    RB_Texcoord2f(0.0, 1.0);

    RB_Vertex2f(screen[0] - size, screen[1] - size);
    RB_Texcoord2f(1.0, 1.0);

    RB_Vertex2f(screen[0] + size, screen[1] - size);
    RB_StreamEnd();
}

void lens_flare::ScreenBlend()
{
    float alpha;
    int   timediff;

    if (!fullscreen || !blend_active) {
        return;
    }

    timediff = backEnd.refdef.time - lasttime;
    if (timediff > fullfade) {
        blend_active = false;
        return;
    }

    alpha = (float)(fullfade - timediff) / fullfade * lastbright;
    if (alpha <= 0.0001) {
        blend_active = false;
        return;
    }

    RB_Color4f(1.0, 1.0, 1.0, alpha);

    RB_StreamBegin(fullscreen);
    RB_Vertex2f(-1.0, -1.0);
    RB_Vertex2f(1.0, -1.0);
    RB_Vertex2f(-1.0, 1.0);
    RB_Vertex2f(1.0, 1.0);

    RB_StreamEnd();
}

class tmpRiFile
{
private:
    bool  file_exists;
    void *buf;

public:
    tmpRiFile()
    {
        file_exists = false;
        buf         = NULL;
    }

    ~tmpRiFile() { close(); }

    int open(const char *which, void **buffer)
    {
        close();

        if (ri.FS_ReadFile(which, buffer) != -1) {
            file_exists = true;
            buf         = *buffer;
        }

        return file_exists;
    }

    bool exists() const { return file_exists; }

    void close()
    {
        if (file_exists) {
            ri.FS_FreeFile(buf);
            file_exists = false;
        }
    }
};

void lens_flare::Init(const char *which)
{
    char       *file;
    const char *token;
    tmpRiFile   theFile;

    initted = false;
    theFile.open("global/lensflaredefs.txt", (void **)&file);

    if (!theFile.exists()) {
        ri.Printf(PRINT_WARNING, "WARNING: Could not open lens flare info file!\n");
        return;
    }

    while ((token = COM_ParseExt(&file, qtrue))) {
        if (!file) {
            ri.Printf(PRINT_WARNING, "WARNING: could not find begin for section '%s' in lensflares\n", which);
            return;
        }

        if (!Q_stricmp(token, "begin")) {
            token = COM_ParseExt(&file, qfalse);
            if (!Q_stricmp(token, which)) {
                break;
            }
        }
    }

    dot_min      = 0.8f;
    fullscale    = 0.7f;
    num_flares   = 0;
    fullscreen   = NULL;
    initted      = true;
    fullfade     = 0;
    blend_active = false;

    while ((token = COM_ParseExt(&file, qtrue))) {
        if (!file) {
            ri.Printf(PRINT_WARNING, "WARNING: could not find end for section '%s' in lensflares\n", which);
            break;
        }

        if (!Q_stricmp(token, "dot_min")) {
            token = COM_ParseExt(&file, qfalse);
            if (!token[0]) {
                ri.Printf(PRINT_WARNING, "WARNING: invalid dot_min in lensflares\n");
                continue;
            }
            dot_min = atof(token);
        } else if (!Q_stricmp(token, "flare")) {
            assert(num_flares < MAX_FLARES);

            flares[num_flares].alphascale = 1.0;

            token = COM_ParseExt(&file, qfalse);
            if (!token[0]) {
                ri.Printf(PRINT_WARNING, "WARNING: no flare arg 1 for flare '%d' in lensflares\n", num_flares);
                continue;
            }
            flares[num_flares].size = atof(token);

            token = COM_ParseExt(&file, qfalse);
            if (!token[0]) {
                ri.Printf(PRINT_WARNING, "WARNING: no flare arg 2 for flare '%d' in lensflares\n", num_flares);
                continue;
            }
            flares[num_flares].where = atof(token);

            token = COM_ParseExt(&file, qfalse);
            if (!token[0]) {
                ri.Printf(PRINT_WARNING, "WARNING: no flare arg 3 for flare '%d' in lensflares\n", num_flares);
                continue;
            }
            flares[num_flares].shader = R_FindShader(token, -1, qfalse, qfalse, qfalse, qfalse);

            token = COM_ParseExt(&file, qfalse);
            if (token[0]) {
                flares[num_flares].alphascale = atof(token);
            }

            num_flares++;
        } else if (!Q_stricmp(token, "fullscale")) {
            token = COM_ParseExt(&file, qfalse);
            if (!token[0]) {
                ri.Printf(PRINT_WARNING, "WARNING: no arg for fullscale, assuming default (in lensflares)\n");
                continue;
            }
            fullscale = atof(token);
        } else if (!Q_stricmp(token, "fullscreen")) {
            token = COM_ParseExt(&file, qfalse);
            if (!token[0]) {
                ri.Printf(PRINT_WARNING, "WARNING: no arg for fullsreen in lensflares\n");
                continue;
            }
            fullscreen = R_FindShader(token, -1, qfalse, qfalse, qfalse, qfalse);
        } else if (!Q_stricmp(token, "fullfade")) {
            token = COM_ParseExt(&file, qfalse);
            if (!token[0]) {
                ri.Printf(PRINT_WARNING, "WARNING: no arg for fullfade in lensflares\n");
                continue;
            }
            fullfade = atof(token);
        } else if (!Q_stricmp(token, "end")) {
            break;
        }
    }

    if (!num_flares) {
        ri.Printf(PRINT_WARNING, "WARNING: no lensflares defined!\n");
    }
    lasttime = backEnd.refdef.time - fullfade;
}

static void R_DrawSunFlare()
{
    if (!s_sun.exists) {
        return;
    }
    if (!sunFlare.initted) {
        if (!s_sun.szFlareName[0]) {
            return;
        }
        if (Q_stricmp(s_sun.szFlareName, "none")) {
            sunFlare.Init(s_sun.szFlareName);
        }
    }

    if (!sunFlare.initted) {
        s_sun.szFlareName[0] = 0;
        return;
    }

    if (!sunFlare.num_flares) {
        return;
    }

    VectorMA(backEnd.viewParms.ori.origin, 16384, s_sun.flaredirection, sunFlare.trace_v);
    VectorMA(backEnd.viewParms.ori.origin, 128, s_sun.flaredirection, sunFlare.v);

    sunFlare.SunTry();
}

static void R_DrawSunFlareBlend()
{
    if (!s_sun.exists) {
        return;
    }
    if (!sunFlare.initted) {
        return;
    }

    sunFlare.ScreenBlend();
}

void R_DrawLensFlares()
{
    int i;

    R_RotateForViewer();

    qglPushMatrix();
    qglLoadIdentity();
    qglMatrixMode(GL_PROJECTION);
    qglPushMatrix();
    qglLoadIdentity();
    qglOrtho(-1.0, 1.0, -1.0, 1.0, -99999.0, 99999.0);

    tess.no_global_fog = true;
    R_DrawSunFlare();

    for (i = 0; i < backEnd.refdef.num_entities; i++) {
        if ((backEnd.viewParms.isPortalSky && !(backEnd.refdef.entities[i].e.renderfx & RF_SKYENTITY))
            || (!backEnd.viewParms.isPortalSky && (backEnd.refdef.entities[i].e.renderfx & RF_SKYENTITY))) {
            continue;
        }

        if (backEnd.refdef.entities[i].e.renderfx & RF_LENSFLARE) {
            vec3_t rgb;

            if (backEnd.refdef.entities[i].e.renderfx & RF_SKYENTITY) {
                dlights.InPortalSky();
            } else {
                dlights.NotInPortalSky();
            }

            dlights.SetVect(backEnd.refdef.entities[i].e.origin);

            //
            // Set the dlight color from entity
            //

            rgb[0] = backEnd.refdef.entities[i].e.shaderRGBA[0] / 255.0;
            rgb[1] = backEnd.refdef.entities[i].e.shaderRGBA[1] / 255.0;
            rgb[2] = backEnd.refdef.entities[i].e.shaderRGBA[2] / 255.0;
            dlights.SetColor(rgb);

            dlights.Try();
        }

        if (backEnd.refdef.entities[i].e.renderfx & RF_VIEWLENSFLARE) {
            vec3_t rgb;

            if (backEnd.refdef.entities[i].e.renderfx & RF_SKYENTITY) {
                torches.InPortalSky();
            } else {
                torches.NotInPortalSky();
            }

            torches.SetVect(backEnd.refdef.entities[i].e.origin);

            rgb[0] = backEnd.refdef.entities[i].e.shaderRGBA[0] / 255.0;
            rgb[1] = backEnd.refdef.entities[i].e.shaderRGBA[1] / 255.0;
            rgb[2] = backEnd.refdef.entities[i].e.shaderRGBA[2] / 255.0;
            torches.SetColor(rgb);

            torches.Try();
        }
    }

    for (i = 0; i < backEnd.refdef.num_dlights; i++) {
        if ((backEnd.refdef.dlights[i].type & dlighttype_t::lensflare)
            || (backEnd.refdef.dlights[i].type & dlighttype_t::additive)) {
            dlights.NotInPortalSky();
            dlights.SetVect(backEnd.refdef.dlights[i].origin);
            dlights.SetColor(backEnd.refdef.dlights[i].color);

            dlights.Try();
        }
    }

    R_DrawSunFlareBlend();
    tess.no_global_fog = qfalse;

    qglPopMatrix();
    qglMatrixMode(GL_MODELVIEW);
    qglPopMatrix();
}

void R_InitLensFlare()
{
    torches.Init("entity");
    dlights.Init("dlight");
    sunFlare.initted = false;
}

bool sun_flare_class::SunCheckRay()
{
    mnode_t *pViewLeaf;
    trace_t  trace;

    pViewLeaf = R_PointInLeaf(tr.refdef.vieworg);

    if (pViewLeaf->area == -1 || !tr.world->vis || tr.sSunLight.leaf != (mnode_s *)-1
        || pViewLeaf->numlights && pViewLeaf->lights[0] == &tr.sSunLight) {
        ri.CM_BoxTrace(
            &trace, backEnd.viewParms.ori.origin, trace_v, vec3_origin, vec3_origin, 0, CONTENTS_SOLID, qfalse
        );

        if (trace.surfaceFlags & SURF_SKY) {
            return true;
        }
    }

    return false;
}

void sun_flare_class::SunScreenCalc()
{
    vec4_t eye;
    vec4_t clip;
    vec4_t point;
    int    i, j;

    VectorCopy(v, point);
    point[3] = 1.0;

    VectorClear4(eye);
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            eye[i] += point[j] * tr.ori.modelMatrix[i + j * 4];
        }
    }

    VectorClear4(clip);
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            clip[i] += eye[j] * tr.viewParms.projectionMatrix[i + j * 4];
        }
    }

    for (i = 0; i < 2; i++) {
        screen[i] = clip[i] / clip[3];
    }
}

void sun_flare_class::SunTry()
{
    vec2_t   diff;
    vec2_t   point;
    float    alpha;
    int      i;
    qboolean bDrawingFade = qfalse;

    if (!CheckRange()) {
        return;
    }

    if (SunCheckRay()) {
        alpha = (dot - dot_min) / (1.0 - dot_min);
    } else {
        int timediff;

        if (!fullscreen) {
            return;
        }
        if (!blend_active) {
            return;
        }

        timediff = backEnd.refdef.time - lasttime;
        alpha    = (float)(fullfade - timediff) / fullfade * lastbright;

        if (alpha <= 0.0001) {
            return;
        }

        bDrawingFade = qtrue;
    }

    SunScreenCalc();
    VectorScale2D(screen, -2.0, diff);

    for (i = 0; i < num_flares; i++) {
        flare_s *flare = &flares[i];

        VectorMA2D(screen, flare->where, diff, point);

        RB_Color4f(color[0], color[1], color[2], alpha * this->alpha * flare->alphascale);
        RB_StreamBegin(flare->shader);
        RB_Texcoord2f(0.0, 0.0);

        RB_Vertex2f(point[0] - flare->size, point[1] + flare->size);
        RB_Texcoord2f(1.0, 0.0);

        RB_Vertex2f(point[0] + flare->size, point[1] + flare->size);
        RB_Texcoord2f(0.0, 1.0);

        RB_Vertex2f(point[0] - flare->size, point[1] - flare->size);
        RB_Texcoord2f(1.0, 1.0);

        RB_Vertex2f(point[0] + flare->size, point[1] - flare->size);
        RB_StreamEnd();
    }

    if (!bDrawingFade) {
        lasttime     = backEnd.refdef.time;
        blend_active = true;
        lastbright   = Square(alpha) * Square(alpha) * fullscale;
    }
}
