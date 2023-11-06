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

#ifdef __cplusplus
extern "C" {
#endif

    const char *TIKI_FileExtension(const char *in);
    void        TIKI_ParseFrameCommands(dloaddef_t *ld, dloadframecmd_t **cmdlist, int maxcmds, int *numcmds);
    void        TIKI_ParseAnimationCommands(dloaddef_t *ld, dloadanim_t *anim);
    void        TIKI_ParseAnimationFlags(dloaddef_t *ld, dloadanim_t *anim);
    void        TIKI_ParseAnimationsFail(dloaddef_t *ld);
    qboolean    TIKI_ParseIncludes(dloaddef_t *ld);
    void        TIKI_ParseAnimations(dloaddef_t *ld);
    int         TIKI_ParseSurfaceFlag(const char *token);
    void        TIKI_InitSetup(dloaddef_t *ld);

#ifdef __cplusplus
    qboolean TIKI_LoadSetupCaseHeader(
        dtiki_t           *tiki,
        const char        *filename,
        dloadsurface_t    *loadsurfaces,
        int               *numSurfacesSetUp,
        msg_t             *msg,
        qboolean           skip,
        con_map<str, str> *keyValues
    );
    qboolean TIKI_LoadSetupCase(
        dtiki_t           *tiki,
        const char        *filename,
        dloadsurface_t    *loadsurfaces,
        int               *numSurfacesSetUp,
        msg_t             *msg,
        qboolean           skip,
        con_map<str, str> *keyValues
    );
    qboolean TIKI_LoadSetup(
        dtiki_t           *tiki,
        const char        *filename,
        dloadsurface_t    *loadsurfaces,
        int               *numSurfacesSetUp,
        byte              *modelData,
        size_t             modelDataSize,
        con_map<str, str> *keyValues
    );
    bool TIKI_strstr(const char *s, const char *substring);
#endif

    qboolean TIKI_ParseSetup(dloaddef_t *ld);
    void     TIKI_ParseInitCommands(dloaddef_t *ld, dloadinitcmd_t **cmdlist, int maxcmds, int *numcmds);
    void     TIKI_ParseInit(dloaddef_t *ld);
    qboolean TIKI_ParseCase(dloaddef_t *ld);

#ifdef __cplusplus
}
#endif
