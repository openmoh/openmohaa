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

// tiki_parse.cpp : TIKI Parser

#include "q_shared.h"
#include "qcommon.h"
#include "../server/server.h"
#include "../skeletor/skeletor.h"
#include <tiki.h>

typedef enum tiki_setupcase_e {
    SETUP_END_MESSAGE = -1,
    SETUP_LOAD_SCALE,
    SETUP_LOD_SCALE,
    SETUP_LOD_BIAS,
    SETUP_SKELMODEL,
    SETUP_LOAD_ORIGIN,
    SETUP_LIGHT_OFFSET,
    SETUP_RADIUS,
    SETUP_SURFACE,
    SETUP_FLAGS,
    SETUP_DAMAGE,
    SETUP_SHADER,
    SETUP_CASE_KEY,
    SETUP_CASE_VALUE,
    SETUP_BEGIN_CASE,
    SETUP_BEGIN_CASE_BODY,
    SETUP_END_CASE
} tiki_setupcase_t;

/*
===============
TIKI_FileExtension
===============
*/
const char *TIKI_FileExtension(const char *in)
{
    static char exten[8];
    int         i;

    for (i = 0; in[i] != 0; i++) {
        if (in[i] == '.') {
            i++;
            break;
        }
    }

    if (!in[i]) {
        return "";
    }

    strncpy(exten, &in[i], sizeof(exten));
    return exten;
}

/*
===============
TIKI_ParseFrameCommands
===============
*/
void TIKI_ParseFrameCommands(dloaddef_t *ld, dloadframecmd_t **cmdlist, int maxcmds, int *numcmds)
{
    qboolean         usecurrentframe = false;
    const char      *token;
    dloadframecmd_t *cmds;
    int              framenum = 0;
    int              i;
    char            *pszArgs[256];

    ld->tikiFile.GetToken(true);

    while (ld->tikiFile.TokenAvailable(true)) {
        token = ld->tikiFile.GetToken(true);
        if (!Q_stricmp(token, "}")) {
            break;
        }

        if (*numcmds < maxcmds) {
            cmds              = (dloadframecmd_t *)TIKI_AllocateLoadData(sizeof(dloadframecmd_t));
            cmdlist[*numcmds] = cmds;
            if (!cmds) {
                TIKI_Error(
                    "TIKI_ParseFrameCommands: could not allocate storage for dloadframecmd_t in %s on line %d.\n",
                    ld->tikiFile.Filename(),
                    ld->tikiFile.GetLineNumber()
                );
                continue;
            }

            cmds->num_args = 0;
            (*numcmds)++;

            if (!Q_stricmp(token, "start") || !Q_stricmp(token, "first")) {
                framenum = TIKI_FRAME_FIRST;
            } else if (!Q_stricmp(token, "end")) {
                framenum = TIKI_FRAME_END;
            } else if (!Q_stricmp(token, "last")) {
                framenum = TIKI_FRAME_LAST;
            } else if (!Q_stricmp(token, "every")) {
                framenum = TIKI_FRAME_EVERY;
            } else if (!Q_stricmp(token, "exit")) {
                framenum = TIKI_FRAME_EXIT;
            } else if (!Q_stricmp(token, "entry") || !Q_stricmp(token, "enter")) {
                framenum = TIKI_FRAME_ENTRY;
            } else if (!Q_stricmp(token, "(")) {
                usecurrentframe = true;
                ld->tikiFile.UnGetToken();
            } else if (!Q_stricmp(token, ")")) {
                usecurrentframe = false;
                ld->tikiFile.UnGetToken();
            } else if (!usecurrentframe) {
                framenum = atoi(token);
            } else {
                ld->tikiFile.UnGetToken();
            }

            if (framenum < TIKI_FRAME_LAST) {
                TIKI_Error(
                    "TIKI_ParseFrameCommands: illegal frame number %d on line %d in %s\n",
                    framenum,
                    ld->tikiFile.GetLineNumber(),
                    ld->tikiFile.Filename()
                );
                while (ld->tikiFile.TokenAvailable(false)) {
                    ld->tikiFile.GetToken(false);
                }
                (*numcmds)--;
                continue;
            }

            cmds->frame_num = framenum;
            if (ld->tikiFile.currentScript) {
                Com_sprintf(cmds->location, sizeof(cmds->location), "%s, line: %d", ld->tikiFile.Filename(), ld->tikiFile.GetLineNumber());
            }

            while (ld->tikiFile.TokenAvailable(false)) {
                token = ld->tikiFile.GetToken(false);

                if (cmds->num_args > 255) {
                    TIKI_Error(
                        "TIKI_ParseFrameCommands: too many args in anim commands in %s.\n", ld->tikiFile.Filename()
                    );
                    continue;
                }

                pszArgs[cmds->num_args] = TIKI_CopyString(token);
                cmds->num_args++;
            }

            cmds->args = (char **)TIKI_AllocateLoadData(cmds->num_args * sizeof(char *));
            for (i = 0; i < cmds->num_args; i++) {
                cmds->args[i] = pszArgs[i];
            }
        } else {
            TIKI_Error(
                "TIKI_ParseFrameCommands: too many anim commands in %s starting on line %i.\n",
                ld->tikiFile.Filename(),
                ld->tikiFile.GetLineNumber()
            );
            while (ld->tikiFile.TokenAvailable(false)) {
                ld->tikiFile.GetToken(false);
            }
        }
    }
}

/*
===============
TIKI_ParseAnimationCommands
===============
*/
void TIKI_ParseAnimationCommands(dloaddef_t *ld, dloadanim_t *anim)
{
    const char *token;

    while (ld->tikiFile.TokenAvailable(true)) {
        token = ld->tikiFile.GetToken(true);
        if (!Q_stricmp(token, "client")) {
            TIKI_ParseFrameCommands(
                ld,
                anim->loadclientcmds,
                sizeof(anim->loadclientcmds) / sizeof(anim->loadclientcmds[0]),
                &anim->num_client_cmds
            );
        } else if (!Q_stricmp(token, "server")) {
            TIKI_ParseFrameCommands(
                ld,
                anim->loadservercmds,
                sizeof(anim->loadservercmds) / sizeof(anim->loadservercmds[0]),
                &anim->num_server_cmds
            );
        } else if (!Q_stricmp(token, "}")) {
            break;
        } else {
            TIKI_Error(
                "TIKI_ParseAnimationCommands: unknown anim command '%s' in '%s' on line %d, skipping line.\n",
                token,
                ld->tikiFile.Filename(),
                ld->tikiFile.GetLineNumber()
            );
            while (ld->tikiFile.TokenAvailable(false)) {
                token = ld->tikiFile.GetToken(false);
            }
        }
    }
}

/*
===============
TIKI_ParseAnimationFlags
===============
*/
void TIKI_ParseAnimationFlags(dloaddef_t *ld, dloadanim_t *anim)
{
    const char *token;

    anim->weight    = 1.0f;
    anim->blendtime = 0.2f;
    anim->flags     = 0;

    while (ld->tikiFile.TokenAvailable(false)) {
        token = ld->tikiFile.GetToken(false);

        if (!Q_stricmp(token, "weight")) {
            anim->weight = ld->tikiFile.GetFloat(false);
            anim->flags |= TAF_RANDOM;
        } else if (!Q_stricmp(token, "deltadriven")) {
            anim->flags |= TAF_DELTADRIVEN;
        } else if (!Q_stricmp(token, "default_angles")) {
            anim->flags |= TAF_DEFAULT_ANGLES;
        } else if (!Q_stricmp(token, "notimecheck")) {
            anim->flags |= TAF_NOTIMECHECK;
        } else if (!Q_stricmp(token, "crossblend")) {
            anim->blendtime = ld->tikiFile.GetFloat(false);
        } else if (!Q_stricmp(token, "dontrepeate")) {
            anim->flags |= TAF_NOREPEAT;
        } else if (!Q_stricmp(token, "random")) {
            anim->flags |= TAF_RANDOM;
        } else if (!Q_stricmp(token, "autosteps_run")) {
            anim->flags |= TAF_AUTOSTEPS | TAF_AUTOSTEPS_RUNNING | TAF_AUTOSTEPS_EQUIPMENT;
        } else if (!Q_stricmp(token, "autosteps_walk")) {
            anim->flags |= TAF_AUTOSTEPS | TAF_AUTOSTEPS_EQUIPMENT;
        } else if (!Q_stricmp(token, "autosteps_dog")) {
            anim->flags |= TAF_AUTOSTEPS;
        } else {
            TIKI_Error(
                "Unknown Animation flag %s for anim '%s' in %s\n",
                token,
                anim->alias,
                TikiScript::currentScript->Filename()
            );
        }
    }
}

/*
===============
TIKI_ParseAnimationsFail
===============
*/
void TIKI_ParseAnimationsFail(dloaddef_t *ld)
{
    int         nestcount = 0;
    const char *token;

    while (ld->tikiFile.TokenAvailable(false)) {
        ld->tikiFile.GetToken(false);
    }

    if (!ld->tikiFile.TokenAvailable(true)) {
        return;
    }

    token = ld->tikiFile.GetToken(true);
    if (Q_stricmp(token, "{")) {
        ld->tikiFile.UnGetToken();
        return;
    }

    ld->tikiFile.UnGetToken();

    while (ld->tikiFile.TokenAvailable(true)) {
        token = ld->tikiFile.GetToken(true);
        if (!Q_stricmp(token, "{")) {
            nestcount++;
        } else if (!Q_stricmp(token, "}")) {
            nestcount--;
            if (!nestcount) {
                break;
            }
        }
    }
}

/*
===============
TIKI_ParseIncludes
===============
*/
qboolean TIKI_ParseIncludes(dloaddef_t *ld)
{
    const char *token;
    qboolean    b_incl = false;
    const char *mapname;
    const char *servertype;
    int         depth = 0;

    static cvar_t *pServerType = Cvar_Get("g_servertype", "2", 0);
    static cvar_t *pGameType   = Cvar_Get("cg_gametype", "0", CVAR_SERVERINFO | CVAR_LATCH);

    if (pGameType->integer != GT_SINGLE_PLAYER && pServerType->integer == 1) {
        servertype = "spearheadserver";
    } else {
        servertype = "breakthroughserver";
    }

    token = ld->tikiFile.GetToken(true);
    if (sv_mapname) {
        mapname = sv_mapname->string;
    } else {
        mapname = "utils";
    }

    while (1) {
        if (!Q_stricmpn(token, mapname, strlen(token)) || !Q_stricmpn(token, servertype, strlen(token))) {
            b_incl = true;
        } else if (!Q_stricmp(token, "{") || !ld->tikiFile.TokenAvailable(true)) {
            break;
        }

        token = ld->tikiFile.GetToken(true);
    }

    if (b_incl) {
        return true;
    }

    while (ld->tikiFile.TokenAvailable(true)) {
        token = ld->tikiFile.GetAndIgnoreLine(false);
        if (strstr(token, "{")) {
            depth++;
        }

        if (strstr(token, "}")) {
            if (!depth) {
                break;
            }

            depth--;
        }
    }

    return false;
}

dloadanim_t *TIKI_AllocAnim(dloaddef_t *ld)
{
    dloadanim_t *panim;

    panim                         = static_cast<dloadanim_t *>(TIKI_AllocateLoadData(sizeof(dloadanim_t)));
    panim->weight                 = 1.f;
    panim->location[0]            = 0;
    panim->num_client_cmds        = 0;
    panim->num_server_cmds        = 0;
    panim->flags                  = 0;
    panim->blendtime              = 0.2f;
    ld->loadanims[ld->numanims++] = panim;

    return panim;
}

/*
===============
TIKI_ParseAnimations
===============
*/
void TIKI_ParseAnimations(dloaddef_t *ld)
{
    const char  *token;
    dloadanim_t *anim;
    qboolean     b_mapspec = false;
    const char  *mapname;
    size_t       depth = 0;

    ld->tikiFile.GetToken(true);

    while (ld->tikiFile.TokenAvailable(true)) {
        token = ld->tikiFile.GetToken(true);
        if (!Q_stricmp(token, "}")) {
            if (!b_mapspec) {
                // Return when there is no mapspec
                return;
            }
            b_mapspec = false;
        } else if (!Q_stricmp(token, "$mapspec")) {
            token = ld->tikiFile.GetToken(true);
            if (sv_mapname) {
                mapname = sv_mapname->string;
            } else {
                mapname = "utils";
            }

            while (ld->tikiFile.TokenAvailable(true)) {
                if (!Q_stricmpn(token, mapname, strlen(token))) {
                    b_mapspec = true;
                } else if (!Q_stricmp(token, "{")) {
                    break;
                }

                token = ld->tikiFile.GetToken(true);
            }

            if (!b_mapspec) {
                while (ld->tikiFile.TokenAvailable(true)) {
                    token = ld->tikiFile.GetToken(true);
                    if (strstr(token, "{")) {
                        depth++;
                    }

                    if (strstr(token, "}")) {
                        if (!depth) {
                            break;
                        }

                        depth--;
                    }
                }
            }
        } else {
            if (ld->numanims >= MAX_TIKI_LOAD_ANIMS) {
                TIKI_Error("TIKI_ParseAnimations: Too many animations in '%s'.\n", ld->path);
                continue;
            }

            anim                        = (dloadanim_t *)TIKI_AllocateLoadData(sizeof(dloadanim_t));
            ld->loadanims[ld->numanims] = anim;
            if (!anim) {
                TIKI_Error(
                    "TIKI_ParseAnimations: Could not allocate storage for anim alias name %s in %s.\n",
                    token,
                    ld->tikiFile.Filename()
                );
                TIKI_ParseAnimationsFail(ld);
                continue;
            }

            if (strlen(token) < 48) {
                anim->alias = (char *)TIKI_CopyString(token);

                token = ld->tikiFile.GetToken(false);
                Q_strncpyz(anim->name, TikiScript::currentScript->path, sizeof(anim->name));
                Q_strcat(anim->name, sizeof(anim->name), token);

                anim->location[0] = 0;
                if (ld->tikiFile.currentScript) {
                    Com_sprintf(
                        anim->location,
                        sizeof(anim->location),
                        "%s, line: %d",
                        ld->tikiFile.currentScript->Filename(),
                        ld->tikiFile.currentScript->GetLineNumber()
                    );
                }

                anim->num_client_cmds = 0;
                anim->num_server_cmds = 0;
                TIKI_ParseAnimationFlags(ld, anim);
                ld->numanims++;

                if (ld->tikiFile.TokenAvailable(true)) {
                    token = ld->tikiFile.GetToken(true);
                    if (!Q_stricmp(token, "{")) {
                        TIKI_ParseAnimationCommands(ld, anim);
                    } else {
                        ld->tikiFile.UnGetToken();
                    }
                }
            } else {
                TIKI_Error(
                    "TIKI_ParseAnimations: Anim alias name %s is too long in %s.\n", token, ld->tikiFile.Filename()
                );
                TIKI_ParseAnimationsFail(ld);
            }
        }
    }
}

/*
===============
TIKI_ParseSurfaceFlag
===============
*/
int TIKI_ParseSurfaceFlag(const char *token)
{
    int flags = 0;

    if (!Q_stricmp(token, "skin1")) {
        flags = TIKI_SURF_SKIN1;
    } else if (!Q_stricmp(token, "skin2")) {
        flags = TIKI_SURF_SKIN2;
    } else if (!Q_stricmp(token, "skin3")) {
        flags = TIKI_SURF_SKIN3;
    } else if (!Q_stricmp(token, "nodraw")) {
        flags = TIKI_SURF_NODRAW;
    } else if (!Q_stricmp(token, "nodamage")) {
        flags = TIKI_SURF_NODAMAGE;
    } else if (!Q_stricmp(token, "crossfade")) {
        flags = TIKI_SURF_CROSSFADE;
    } else if (!Q_stricmp(token, "nomipmaps")) {
        flags = TIKI_SURF_NOMIPMAPS;
    } else if (!Q_stricmp(token, "nopicmip")) {
        flags = TIKI_SURF_NOPICMIP;
    } else {
        TIKI_Error("Unknown surface flag %s\n", token);
    }

    return flags;
}

/*
===============
WriteScale
===============
*/
static void WriteScale(dloaddef_t *ld, float scale)
{
    MSG_WriteByte(ld->modelBuf, SETUP_LOAD_SCALE);
    MSG_WriteFloat(ld->modelBuf, scale);
}

/*
===============
WriteLoadScale
===============
*/
static void WriteLoadScale(dloaddef_t *ld, float lod_scale)
{
    MSG_WriteByte(ld->modelBuf, SETUP_LOD_SCALE);
    MSG_WriteFloat(ld->modelBuf, lod_scale);
}

/*
===============
WriteLodBias
===============
*/
static void WriteLodBias(dloaddef_t *ld, float lod_bias)
{
    MSG_WriteByte(ld->modelBuf, SETUP_LOD_BIAS);
    MSG_WriteFloat(ld->modelBuf, lod_bias);
}

/*
===============
WriteSkelmodel
===============
*/
void WriteSkelmodel(dloaddef_t *ld, const char *name)
{
    MSG_WriteByte(ld->modelBuf, SETUP_SKELMODEL);
    MSG_WriteString(ld->modelBuf, name);

    Q_strncpyz(ld->idleSkel, name, sizeof(ld->idleSkel));
    ld->numskels++;
}

/*
===============
WriteOrigin
===============
*/
static void WriteOrigin(dloaddef_t *ld, float origin_x, float origin_y, float origin_z)
{
    MSG_WriteByte(ld->modelBuf, SETUP_LOAD_ORIGIN);
    MSG_WriteFloat(ld->modelBuf, origin_x);
    MSG_WriteFloat(ld->modelBuf, origin_y);
    MSG_WriteFloat(ld->modelBuf, origin_z);
}

/*
===============
WriteLightOffset
===============
*/
static void WriteLightOffset(dloaddef_t *ld, float light_offset_x, float light_offset_y, float light_offset_z)
{
    MSG_WriteByte(ld->modelBuf, SETUP_LIGHT_OFFSET);
    MSG_WriteFloat(ld->modelBuf, light_offset_x);
    MSG_WriteFloat(ld->modelBuf, light_offset_y);
    MSG_WriteFloat(ld->modelBuf, light_offset_z);
}

/*
===============
WriteRadius
===============
*/
static void WriteRadius(dloaddef_t *ld, float radius)
{
    MSG_WriteByte(ld->modelBuf, SETUP_RADIUS);
    MSG_WriteFloat(ld->modelBuf, radius);
}

/*
===============
WriteSurface
===============
*/
static void WriteSurface(dloaddef_t *ld, const char *surface)
{
    MSG_WriteByte(ld->modelBuf, SETUP_SURFACE);
    MSG_WriteString(ld->modelBuf, surface);
}

/*
===============
WriteFlags
===============
*/
static void WriteFlags(dloaddef_t *ld, int flags)
{
    MSG_WriteByte(ld->modelBuf, SETUP_FLAGS);
    MSG_WriteLong(ld->modelBuf, flags);
}

/*
===============
WriteDamage
===============
*/
static void WriteDamage(dloaddef_t *ld, float damage)
{
    MSG_WriteByte(ld->modelBuf, SETUP_DAMAGE);
    MSG_WriteFloat(ld->modelBuf, damage);
}

/*
===============
WriteShader
===============
*/
static void WriteShader(dloaddef_t *ld, const char *shader)
{
    MSG_WriteByte(ld->modelBuf, SETUP_SHADER);
    MSG_WriteString(ld->modelBuf, shader);
}

/*
===============
WriteBeginCase
===============
*/
static void WriteBeginCase(dloaddef_t *ld)
{
    MSG_WriteByte(ld->modelBuf, SETUP_BEGIN_CASE);
}

/*
===============
WriteCaseKey
===============
*/
static void WriteCaseKey(dloaddef_t *ld, const char *key)
{
    MSG_WriteByte(ld->modelBuf, SETUP_CASE_KEY);
    MSG_WriteString(ld->modelBuf, key);
}

/*
===============
WriteCaseValue
===============
*/
static void WriteCaseValue(dloaddef_t *ld, const char *value)
{
    MSG_WriteByte(ld->modelBuf, SETUP_CASE_VALUE);
    MSG_WriteString(ld->modelBuf, value);
}

/*
===============
WriteBeginCaseBody
===============
*/
static void WriteBeginCaseBody(dloaddef_t *ld)
{
    MSG_WriteByte(ld->modelBuf, SETUP_BEGIN_CASE_BODY);
}

/*
===============
WriteEndCase
===============
*/
static void WriteEndCase(dloaddef_t *ld)
{
    MSG_WriteByte(ld->modelBuf, SETUP_END_CASE);
}

/*
===============
TIKI_InitSetup
===============
*/
void TIKI_InitSetup(dloaddef_t *ld)
{
    MSG_Init(ld->modelBuf, ld->modelData, 8192);
}

/*
===============
TIKI_LoadSetupCaseHeader
===============
*/
qboolean TIKI_LoadSetupCaseHeader(
    dtiki_t           *tiki,
    const char        *filename,
    dloadsurface_t    *loadsurfaces,
    int               *numSurfacesSetUp,
    msg_t             *msg,
    qboolean           skip,
    con_map<str, str> *keyValues
)
{
    int         c = 0;
    str         key;
    str        *val;
    const char *value;
    qboolean    match = false;

    while (c != SETUP_BEGIN_CASE_BODY) {
        c = MSG_ReadByte(msg);
        switch (c) {
        case SETUP_CASE_KEY:
            value = MSG_ReadString(msg);
            key   = value;
            break;
        case SETUP_CASE_VALUE:
            value = MSG_ReadString(msg);
            if (skip || match || !keyValues) {
                break;
            }

            val = keyValues->find(key);
            if (val && !Q_stricmp(val->c_str(), value)) {
                match = true;
            }
            break;
        default:
            break;
        }
    }

    return TIKI_LoadSetupCase(tiki, filename, loadsurfaces, numSurfacesSetUp, msg, skip || !match, keyValues);
}

/*
===============
TIKI_LoadSetupCase
===============
*/
qboolean TIKI_LoadSetupCase(
    dtiki_t           *tiki,
    const char        *filename,
    dloadsurface_t    *loadsurfaces,
    int               *numSurfacesSetUp,
    msg_t             *msg,
    qboolean           skip,
    con_map<str, str> *keyValues
)
{
    int               c;
    const char       *name;
    int               currentSurface = -1;
    int               mesh;
    skelHeaderGame_t *skelmodel;
    float             load_scale;
    float             lod_scale;
    float             lod_bias;
    float             load_origin[3];
    float             light_offset[3];
    float             radius;
    int               flags;
    float             damage_multiplier;

    while (1) {
        c = MSG_ReadByte(msg);
        switch (c) {
        default:
            break;
        case SETUP_LOAD_SCALE:
            load_scale = MSG_ReadFloat(msg);
            if (skip) {
                break;
            }
            tiki->load_scale = load_scale;
            break;
        case SETUP_LOD_SCALE:
            lod_scale = MSG_ReadFloat(msg);
            if (skip) {
                break;
            }
            tiki->lod_scale = lod_scale;
            break;
        case SETUP_LOD_BIAS:
            lod_bias = MSG_ReadFloat(msg);
            if (skip) {
                break;
            }
            tiki->lod_bias = lod_bias;
            break;
        case SETUP_SKELMODEL:
            name = MSG_ReadString(msg);
            if (skip) {
                break;
            }

            if (tiki->numMeshes >= MAX_SKELMODELS) {
                TIKI_Error("^~^~^ TIKI_LoadSetup: too many skelmodels in %s.\n", filename);
                return false;
            }

            mesh = TIKI_RegisterSkel(name, tiki);
            if (mesh < 0) {
                return false;
            }

            tiki->mesh[tiki->numMeshes] = mesh;
            skelmodel                   = TIKI_GetSkel(mesh);
            tiki->num_surfaces += skelmodel->numSurfaces;
            tiki->numMeshes++;
            break;
        case SETUP_LOAD_ORIGIN:
            load_origin[0] = MSG_ReadFloat(msg);
            load_origin[1] = MSG_ReadFloat(msg);
            load_origin[2] = MSG_ReadFloat(msg);
            if (skip) {
                break;
            }
            VectorCopy(load_origin, tiki->load_origin);
            break;
        case SETUP_LIGHT_OFFSET:
            light_offset[0] = MSG_ReadFloat(msg);
            light_offset[1] = MSG_ReadFloat(msg);
            light_offset[2] = MSG_ReadFloat(msg);
            if (skip) {
                break;
            }
            VectorCopy(light_offset, tiki->load_origin);
            break;
        case SETUP_RADIUS:
            radius = MSG_ReadFloat(msg);
            if (skip) {
                break;
            }
            tiki->radius = radius;
            break;
        case SETUP_SURFACE:
            name = MSG_ReadString(msg);
            if (skip) {
                break;
            }

            for (currentSurface = 0; currentSurface < *numSurfacesSetUp; currentSurface++) {
                if (!Q_stricmp(loadsurfaces[currentSurface].name, name)) {
                    break;
                }
            }

            if (currentSurface == *numSurfacesSetUp) {
                loadsurfaces[currentSurface].flags = 0;
                (*numSurfacesSetUp)++;
            }
            Q_strncpyz(loadsurfaces[currentSurface].name, name, sizeof(loadsurfaces[currentSurface].name));
            break;
        case SETUP_FLAGS:
            flags = MSG_ReadLong(msg);
            if (skip) {
                break;
            }
            loadsurfaces[currentSurface].flags |= flags;
            break;
        case SETUP_DAMAGE:
            damage_multiplier = MSG_ReadFloat(msg);
            if (skip) {
                break;
            }
            loadsurfaces[currentSurface].damage_multiplier = damage_multiplier;
            break;
        case SETUP_SHADER:
            name = MSG_ReadString(msg);
            if (skip) {
                break;
            }

            if (loadsurfaces[currentSurface].numskins > 3) {
                TIKI_Error(
                    "TIKI_ParseSetup: Too many skins defined for surface %s in %s.\n",
                    loadsurfaces[currentSurface].name,
                    filename
                );
                break;
            }

            strncpy(
                loadsurfaces[currentSurface].shader[loadsurfaces[currentSurface].numskins],
                name,
                sizeof(loadsurfaces[currentSurface].shader[loadsurfaces[currentSurface].numskins])
            );
            loadsurfaces[currentSurface].numskins++;
            break;
        case SETUP_BEGIN_CASE:
            if (!TIKI_LoadSetupCaseHeader(tiki, filename, loadsurfaces, numSurfacesSetUp, msg, skip, keyValues)) {
                return false;
            }
            break;
        case SETUP_END_MESSAGE:
        case SETUP_END_CASE:
            return true;
        }
    }
}

/*
===============
TIKI_LoadSetup
===============
*/
qboolean TIKI_LoadSetup(
    dtiki_t           *tiki,
    const char        *filename,
    dloadsurface_t    *loadsurfaces,
    int               *numSurfacesSetUp,
    byte              *modelData,
    size_t             modelDataSize,
    con_map<str, str> *keyValues
)
{
    msg_t msg;

    MSG_Init(&msg, modelData, modelDataSize);
    msg.cursize = modelDataSize;
    MSG_BeginReading(&msg);

    memset(tiki, 0, sizeof(dtiki_t));
    memset(loadsurfaces, 0, sizeof(dloadsurface_t) * 24);
    *numSurfacesSetUp = 0;

    VectorCopy(vec3_origin, tiki->load_origin);
    VectorCopy(vec3_origin, tiki->light_offset);
    tiki->load_scale = 1.0f;
    tiki->lod_scale  = 1.0f;
    tiki->lod_bias   = 0.0f;
    tiki->numMeshes  = 0;

    return TIKI_LoadSetupCase(tiki, filename, loadsurfaces, numSurfacesSetUp, &msg, false, keyValues);
}

/*
===============
TIKI_strstr
===============
*/
bool TIKI_strstr(const char *s, const char *substring)
{
    const char *t = strstr(s, substring);
    const char *w;

    if (!t || (t != s && *(t - 1) != '\n')) {
        return false;
    }

    w = strstr(t, "\n");
    if (w != (t + strlen(substring))) {
        return false;
    }

    return true;
}

/*
===============
TIKI_ParseSetup
===============
*/
qboolean TIKI_ParseSetup(dloaddef_t *ld)
{
    const char *token;
    float       load_scale;
    float       lod_scale;
    float       lod_bias;
    float       tmpFloat;
    float       tmpVec[3];
    int         tmpInt;
    size_t      length;
    char        name[128];

    // Skip the setup token
    ld->tikiFile.GetToken(true);

    while (ld->tikiFile.TokenAvailable(true)) {
        token = ld->tikiFile.GetToken(true);

        if (!Q_stricmp(token, "scale")) {
            load_scale = ld->tikiFile.GetFloat(false);
            WriteScale(ld, load_scale);
        } else if (!Q_stricmp(token, "lod_scale")) {
            lod_scale = ld->tikiFile.GetFloat(false) / 5.0f;
            WriteLoadScale(ld, lod_scale);
        } else if (!Q_stricmp(token, "lod_bias")) {
            lod_bias = ld->tikiFile.GetFloat(false);
            WriteLodBias(ld, lod_bias);
        } else if (!Q_stricmp(token, "skelmodel")) {
            token = ld->tikiFile.GetToken(false);
            Q_strncpyz(name, ld->tikiFile.currentScript->path, sizeof(name));
            Q_strcat(name, sizeof(name), token);
            WriteSkelmodel(ld, name);
        } else if (!Q_stricmp(token, "path")) {
            token = ld->tikiFile.GetToken(false);
            Q_strncpyz(ld->tikiFile.currentScript->path, token, sizeof(ld->tikiFile.currentScript->path));
            length = strlen(ld->tikiFile.currentScript->path);
            token  = ld->tikiFile.currentScript->path + length - 1;

            if (*token != '/' && *token != '\\') {
                Q_strcat(ld->tikiFile.currentScript->path, sizeof(ld->tikiFile.currentScript->path), "/");
            }
        } else if (!Q_stricmp(token, "origin")) {
            tmpVec[0] = ld->tikiFile.GetFloat(false);
            tmpVec[1] = ld->tikiFile.GetFloat(false);
            tmpVec[2] = ld->tikiFile.GetFloat(false);
            WriteOrigin(ld, tmpVec[0], tmpVec[1], tmpVec[2]);
        } else if (!Q_stricmp(token, "lightoffset")) {
            tmpVec[0] = ld->tikiFile.GetFloat(false);
            tmpVec[1] = ld->tikiFile.GetFloat(false);
            tmpVec[2] = ld->tikiFile.GetFloat(false);
            WriteLightOffset(ld, tmpVec[0], tmpVec[1], tmpVec[2]);
        } else if (!Q_stricmp(token, "radius")) {
            tmpFloat = ld->tikiFile.GetFloat(false);
            WriteRadius(ld, tmpFloat);
        } else if (!Q_stricmp(token, "surface")) {
            token = ld->tikiFile.GetToken(false);
            WriteSurface(ld, token);
            while (ld->tikiFile.TokenAvailable(false)) {
                token = ld->tikiFile.GetToken(false);
                if (!Q_stricmp(token, "flags")) {
                    token  = ld->tikiFile.GetToken(false);
                    tmpInt = TIKI_ParseSurfaceFlag(token);
                    WriteFlags(ld, tmpInt);
                } else if (!Q_stricmp(token, "damage")) {
                    tmpFloat = ld->tikiFile.GetFloat(false);
                    WriteDamage(ld, tmpFloat);
                } else if (!Q_stricmp(token, "shader")) {
                    token = ld->tikiFile.GetToken(false);
                    if (strstr(token, ".")) {
                        Q_strncpyz(name, ld->tikiFile.currentScript->path, sizeof(name));
                        Q_strcat(name, sizeof(name), token);
                        WriteShader(ld, name);
                    } else {
                        WriteShader(ld, token);
                    }
                }
            }
        } else if (!Q_stricmp(token, "ischaracter")) {
            ld->bIsCharacter = true;
        } else if (!Q_stricmp(token, "case")) {
            if (!TIKI_ParseCase(ld)) {
                return false;
            }
        } else if (!Q_stricmp(token, "}")) {
            return true;
        } else {
            TIKI_Error(
                "TIKI_ParseSetup: unknown setup command '%s' in '%s' on line %d, skipping line.\n",
                token,
                ld->tikiFile.Filename(),
                ld->tikiFile.GetLineNumber()
            );
            while (ld->tikiFile.TokenAvailable(false)) {
                ld->tikiFile.GetToken(false);
            }
        }
    }

    return true;
}

/*
===============
TIKI_ParseInitCommands
===============
*/
void TIKI_ParseInitCommands(dloaddef_t *ld, dloadinitcmd_t **cmdlist, int maxcmds, int *numcmds)
{
    int             i;
    const char     *token;
    char           *pszArgs[256];
    dloadinitcmd_t *cmd;

    // Skip current token
    ld->tikiFile.GetToken(true);

    while (ld->tikiFile.TokenAvailable(true)) {
        token = ld->tikiFile.GetToken(true);

        if (!Q_stricmp(token, "}")) {
            break;
        }

        if (*numcmds < maxcmds) {
            cmd               = (dloadinitcmd_t *)TIKI_AllocateLoadData(sizeof(dloadinitcmd_t));
            cmdlist[*numcmds] = cmd;
            if (cmd) {
                (*numcmds)++;

                cmd->num_args = 0;
                cmd->args     = NULL;

                ld->tikiFile.UnGetToken();
                while (ld->tikiFile.TokenAvailable(false)) {
                    token = ld->tikiFile.GetToken(false);
                    if (cmd->num_args < 255) {
                        pszArgs[cmd->num_args] = TIKI_CopyString(token);
                        cmd->num_args++;
                    } else {
                        TIKI_Error(
                            "TIKI_ParseInitCommands: too many args in anim commands in %s.\n", ld->tikiFile.Filename()
                        );
                    }
                }

                cmd->args = (char **)TIKI_AllocateLoadData(cmd->num_args * sizeof(char *));
                for (i = 0; i < cmd->num_args; i++) {
                    cmd->args[i] = pszArgs[i];
                }
            } else {
                TIKI_Error(
                    "TIKI_ParseInitCommands: could not allocate storage for dloadinitcmd_t in %s on line %d.\n",
                    ld->tikiFile.Filename(),
                    ld->tikiFile.GetLineNumber()
                );
            }
        }
    }
}

/*
===============
TIKI_ParseInit
===============
*/
void TIKI_ParseInit(dloaddef_t *ld)
{
    const char *token;

    // Skip the init token
    ld->tikiFile.GetToken(true);

    while (ld->tikiFile.TokenAvailable(true)) {
        token = ld->tikiFile.GetToken(true);
        if (!Q_stricmp(token, "client")) {
            TIKI_ParseInitCommands(ld, ld->loadclientinitcmds, 160, &ld->numclientinitcmds);
        } else if (!Q_stricmp(token, "server")) {
            TIKI_ParseInitCommands(ld, ld->loadserverinitcmds, 160, &ld->numserverinitcmds);
        } else if (!Q_stricmp(token, "}")) {
            break;
        } else {
            TIKI_Error(
                "TIKI_ParseInit: unknown init command %s in %s on line %d, skipping line.\n",
                token,
                ld->tikiFile.Filename(),
                ld->tikiFile.GetLineNumber()
            );

            // Skip the current line
            while (ld->tikiFile.TokenAvailable(false)) {
                ld->tikiFile.GetToken(false);
            }
        }
    }
}

/*
===============
TIKI_ParseCase
===============
*/
qboolean TIKI_ParseCase(dloaddef_t *ld)
{
    const char *token;
    bool        isheadmodel;
    bool        isheadskin;

    WriteBeginCase(ld);

__newcase:
    token = ld->tikiFile.GetToken(false);
    WriteCaseKey(ld, token);
    isheadmodel = !Q_stricmp(token, "headmodel");
    isheadskin  = !Q_stricmp(token, "headskin");

    while (1) {
        if (!ld->tikiFile.TokenAvailable(true)) {
            TIKI_Error(
                "TIKI_ParseSetup: unexpected end of file while parsing 'case' switch in %s on line %d.\n",
                ld->tikiFile.Filename(),
                ld->tikiFile.GetLineNumber()
            );
            return 0;
        }

        token = ld->tikiFile.GetToken(true);
        if (!Q_stricmp(token, "case")) {
            goto __newcase;
        } else if (!Q_stricmp(token, "{")) {
            break;
        }

        WriteCaseValue(ld, token);

        if (isheadmodel && !TIKI_strstr(ld->headmodels, token)) {
            strcat(ld->headmodels, token);
            strcat(ld->headmodels, "\n");
        }

        if (isheadskin && !TIKI_strstr(ld->headskins, token)) {
            strcat(ld->headskins, token);
            strcat(ld->headskins, "\n");
        }
    }

    WriteBeginCaseBody(ld);
    ld->tikiFile.UnGetToken();
    if (!TIKI_ParseSetup(ld)) {
        return false;
    }

    WriteEndCase(ld);
    return true;
}
