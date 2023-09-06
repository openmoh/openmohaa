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

// DESCRIPTION:
// Memory archiver

#pragma once

#ifdef __cplusplus

#include "../qcommon/container.h"
#include "../qcommon/str.h"

class MemArchiver;

extern "C" {
#endif

size_t CG_SaveStateToBuffer(void** out, int svsTime);
qboolean CG_LoadStateToBuffer(void* state, size_t size, int svsTime);

#ifdef __cplusplus
void CG_ArchiveStuff(MemArchiver& archiver, int svsTime);
void CG_ArchiveStringContainer(MemArchiver& archiver, Container<str>* container);
void CG_ArchiveTikiPointer(MemArchiver& archiver, dtiki_t** pTiki);
void CG_ArchiveModelHandle(MemArchiver& archiver, qhandle_t* handle);
void CG_ArchiveShaderHandle(MemArchiver& archiver, qhandle_t* handle);
void CG_ArchiveRefEntity(MemArchiver& archiver, refEntity_t* ref);
#endif


#ifdef __cplusplus
}
#endif