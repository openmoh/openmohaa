/*
===========================================================================
Copyright (C) 2008 Leszek Godlewski
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

#ifndef __TIKI_H__
#define __TIKI_H__

#include "q_shared.h"

#include "../tiki/tiki_shared.h"

#ifdef __cplusplus
class Archiver;
#include <mem_blockalloc.h>
#include <con_set.h>
#include <str.h>
#endif

typedef struct AliasList_s AliasList_t;
typedef struct AliasListNode_s AliasListNode_t;
typedef struct msg_s msg_t;

typedef struct {
	int indexes[ 3 ];
} tikiTriangle_t;

typedef struct {
	float st[ 2 ];
} tikiSt_t;

typedef struct {
	short unsigned int xyz[ 3 ];
	short int normal;
} tikiXyzNormal_t;

typedef struct {
	float origin[ 3 ];
	float axis[ 3 ][ 3 ];
} tikiTagData_t;

typedef struct {
	char name[ 64 ];
} tikiTag_t;

typedef struct {
	qboolean valid;
	int surface;
	vec3_t position;
	vec3_t normal;
	float damage_multiplier;
} tikimdl_intersection_t;

typedef struct {
	int indexes[ 3 ];
} skelTriangle_t;

typedef struct dtikicmd_s {
	int frame_num;
	int num_args;
	char **args;
} dtikicmd_t;

typedef struct {
	int frame_num;
	int num_args;
	char **args;
	char location[ 256 ];
} dloadframecmd_t;

typedef struct {
	int num_args;
	char **args;
} dloadinitcmd_t;

typedef struct {
	char name[ 32 ];
	char shader[ 4 ][ 64 ];
	int numskins;
	int flags;
	float damage_multiplier;
} dloadsurface_t;

typedef struct {
	char *alias;
	char name[ 128 ];
	char location[ 256 ];
	float weight;
	float blendtime;
	int flags;
	int num_client_cmds;
	int num_server_cmds;
	dloadframecmd_t *loadservercmds[ 32 ];
	dloadframecmd_t *loadclientcmds[ 128 ];
} dloadanim_t;

typedef struct dloaddef_s dloaddef_t;

#include <tiki_script.h>

#ifdef __cplusplus

typedef struct dloaddef_s {
	const char			*path;
	class TikiScript	tikiFile;

	dloadanim_t			*loadanims[ 4095 ];
	dloadinitcmd_t		*loadserverinitcmds[ 160 ];
	dloadinitcmd_t		*loadclientinitcmds[ 160 ];

	int		skelIndex_ld[ 12 ];
	int		numanims;
	int		numserverinitcmds;
	int		numclientinitcmds;

	char		headmodels[ 4096 ];
	char		headskins[ 4096 ];
	qboolean	bIsCharacter;

	struct msg_s	*modelBuf;
	unsigned char	modelData[ 8192 ];

	qboolean bInIncludesSection;
} dloaddef_t;

#endif

#include "../skeletor/skeletor.h"

#include "../tiki/tiki_anim.h"
#include "../tiki/tiki_cache.h"
#include "../tiki/tiki_commands.h"
#include "../tiki/tiki_files.h"
#include "../tiki/tiki_imports.h"
#include "../tiki/tiki_main.h"
#include "../tiki/tiki_parse.h"
#include "../tiki/tiki_skel.h"
#include "../tiki/tiki_tag.h"
#include "../tiki/tiki_utility.h"
#include "../tiki/tiki_frame.h"
#include "../tiki/tiki_surface.h"

#endif // __TIKI_H__
