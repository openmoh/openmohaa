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

#include "q_shared.h"

#include "../tiki/tiki_shared.h"

#ifdef __cplusplus
class Archiver;
#    include "../qcommon/mem_blockalloc.h"
#    include "../qcommon/con_set.h"
#    include "../qcommon/str.h"
#endif

#define MAX_TIKI_LOAD_ANIMS                 4095
#define MAX_TIKI_LOAD_SKEL_INDEX            12
#define MAX_TIKI_LOAD_SERVER_INIT_COMMANDS  160
#define MAX_TIKI_LOAD_CLIENT_INIT_COMMANDS  180 // 2.30: Increased from 160 to 180
#define MAX_TIKI_LOAD_HEADMODELS_LENGTH     4096
#define MAX_TIKI_LOAD_HEADSKINS_LENGTH      4096
#define MAX_TIKI_LOAD_MODEL_BUFFER          8192

#define MAX_TIKI_LOAD_FRAME_SERVER_COMMANDS 32
#define MAX_TIKI_LOAD_FRAME_CLIENT_COMMANDS 128

#define MAX_TIKI_LOAD_SHADERS               4

typedef struct AliasList_s     AliasList_t;
typedef struct AliasListNode_s AliasListNode_t;
typedef struct msg_s           msg_t;

typedef struct {
    int indexes[3];
} tikiTriangle_t;

typedef struct {
    vec2_t st;
} tikiSt_t;

typedef struct {
    short unsigned int xyz[3];
    short int          normal;
} tikiXyzNormal_t;

typedef struct {
    vec3_t origin;
    vec3_t axis[3];
} tikiTagData_t;

typedef struct {
    char name[64];
} tikiTag_t;

typedef struct {
    qboolean valid;
    int      surface;
    vec3_t   position;
    vec3_t   normal;
    float    damage_multiplier;
} tikimdl_intersection_t;

typedef struct {
    int indexes[3];
} skelTriangle_t;

typedef struct dtikicmd_s {
    int    frame_num;
    int    num_args;
    char **args;
} dtikicmd_t;

typedef struct {
    int    frame_num;
    int    num_args;
    char **args;
    char   location[MAX_QPATH];
} dloadframecmd_t;

typedef struct {
    int    num_args;
    char **args;
} dloadinitcmd_t;

typedef struct {
    char  name[MAX_NAME_LENGTH];
    char  shader[MAX_TIKI_LOAD_SHADERS][MAX_RES_NAME];
    int   numskins;
    int   flags;
    float damage_multiplier;
} dloadsurface_t;

typedef struct {
    char            *alias;
    char             name[MAX_QPATH];
    char             location[MAX_QPATH];
    float            weight;
    float            blendtime;
    int              flags;
    int              num_client_cmds;
    int              num_server_cmds;
    dloadframecmd_t *loadservercmds[MAX_TIKI_LOAD_FRAME_SERVER_COMMANDS];
    dloadframecmd_t *loadclientcmds[MAX_TIKI_LOAD_FRAME_CLIENT_COMMANDS];
} dloadanim_t;

typedef struct dloaddef_s dloaddef_t;

#include "tiki_script.h"

#ifdef __cplusplus

typedef struct dloaddef_s {
    const char      *path;
    class TikiScript tikiFile;

    dloadanim_t    *loadanims[MAX_TIKI_LOAD_ANIMS];
    dloadinitcmd_t *loadserverinitcmds[MAX_TIKI_LOAD_SERVER_INIT_COMMANDS];
    dloadinitcmd_t *loadclientinitcmds[MAX_TIKI_LOAD_CLIENT_INIT_COMMANDS];

    int skelIndex_ld[MAX_TIKI_LOAD_SKEL_INDEX];
    int numanims;
    int numserverinitcmds;
    int numclientinitcmds;

    char     headmodels[MAX_TIKI_LOAD_HEADMODELS_LENGTH];
    char     headskins[MAX_TIKI_LOAD_HEADSKINS_LENGTH];
    qboolean bIsCharacter;

    struct msg_s *modelBuf;
    unsigned char modelData[MAX_TIKI_LOAD_MODEL_BUFFER];

    qboolean bInIncludesSection;

    // Added in 2.0
    //====
    char     idleSkel[MAX_QPATH];
    int      numskels;
    qboolean hasSkel;
    //====
} dloaddef_t;

#endif

#include "../skeletor/skeletor.h"

#include "../qcommon/tiki_main.h"
#include "../tiki/tiki_anim.h"
#include "../tiki/tiki_cache.h"
#include "../tiki/tiki_commands.h"
#include "../tiki/tiki_files.h"
#include "../tiki/tiki_imports.h"
#include "../tiki/tiki_parse.h"
#include "../tiki/tiki_skel.h"
#include "../tiki/tiki_tag.h"
#include "../tiki/tiki_utility.h"
#include "../tiki/tiki_frame.h"
#include "../tiki/tiki_surface.h"
#include "../tiki/tiki_mesh.h"
