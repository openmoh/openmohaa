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

// tiki_cache.h : TIKI Cache

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    dtikianim_t *TIKI_FindTikiAnim(const char *path);
    dtiki_t     *TIKI_FindTiki(const char *path);
    dtikianim_t *TIKI_RegisterTikiAnimFlags(const char *path, qboolean use);
    dtikianim_t *TIKI_RegisterTikiAnim(const char *path);
    dtiki_t     *TIKI_RegisterTikiFlags(const char *path, qboolean use);
    dtiki_t     *TIKI_RegisterTiki(const char *path);
    void         TIKI_FreeAll();
    void        *TIKI_GetSkeletor(dtiki_t *tiki, int entnum);
    static void  TIKI_DeleteSkeletor(int entnum);
    void         TIKI_Begin(void);
    void         TIKI_End(void);
    void         TIKI_FinishLoad(void);
    void         TIKI_FreeImages(void);
    void         TIKI_TikiAnimList_f(void);
    void         TIKI_TikiList_f(void);

#ifdef __cplusplus
}
#endif
