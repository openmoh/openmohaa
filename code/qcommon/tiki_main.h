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

// tiki_main.h : TIKI main

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    extern int         cache_numskel;
    extern int         cache_maxskel;
    extern skelcache_t skelcache[];

    void TIKI_AddPointToBounds(float *v, float *mins, float *maxs);
    void TIKI_Message(const char *fmt, ...);
    void TIKI_Warning(const char *fmt, ...);
    void TIKI_Error(const char *fmt, ...);
    void
    TIKI_SetupIndividualSurface(const char *filename, dtikisurface_t *surf, const char *name, dloadsurface_t *loadsurf);
    size_t       TIKI_CalcAnimDefSize(dloaddef_t *ld);
    dtikianim_t *TIKI_FillTIKIStructureSkel(dloaddef_t *ld);

#ifdef __cplusplus
}
#endif
