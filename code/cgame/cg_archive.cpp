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

#include "cg_local.h"
#include "cg_archive.h"
#include "cg_commands.h"
#include "memarchiver.h"
#include "../qcommon/tiki.h"

void CG_ArchiveStuff(MemArchiver& archiver, int svsTime)
{
	archiver.SetBaseTime(svsTime);
	commandManager.ArchiveToMemory(archiver);
	CG_ArchiveVSSGlobals(archiver);
}

size_t CG_SaveStateToBuffer(void** out, int svsTime)
{
	MemArchiver archiver;
	size_t size;

	archiver.SetupForWriting(0x200000);
	CG_ArchiveStuff(archiver, svsTime);
	size = archiver.BufferSize();
	if (size && out) {
		*out = archiver.ConfiscateBuffer();
	}

	return size;
}

qboolean CG_LoadStateToBuffer(void* state, size_t size, int svsTime)
{
	MemArchiver archiver;

	archiver.SetupForReading((byte*)state, size);
	CG_ArchiveStuff(archiver, svsTime);

	return archiver.FinishedReading();
}

void CG_ArchiveStringContainer(MemArchiver& archiver, Container<str>* container)
{
    int num;
    int i;

	if (archiver.IsReading()) {
		str tmp;

		archiver.ArchiveInteger(&num);
		container->ClearObjectList();

		for (i = 0; i < num; i++) {
			archiver.ArchiveString(&tmp);
			container->AddObject(tmp);
		}
	} else {
        num = container->NumObjects();
        archiver.ArchiveInteger(&num);

        for (i = 0; i < num; i++) {
			str& tmp = container->ObjectAt(i);
			archiver.ArchiveString(&tmp);
		}
	}
}

void CG_ArchiveTikiPointer(MemArchiver& archiver, dtiki_t** pTiki)
{
    str tmp;

	if (archiver.IsReading()) {
		archiver.ArchiveString(&tmp);
		if (tmp.length()) {
			*pTiki = cgi.R_Model_GetHandle(cgi.R_RegisterModel(tmp.c_str()));
		} else {
			*pTiki = NULL;
		}
	} else {
		if (*pTiki) {
			tmp = (*pTiki)->name;
		}

		archiver.ArchiveString(&tmp);
	}
}

void CG_ArchiveModelHandle(MemArchiver& archiver, qhandle_t* handle)
{
    str tmp;

	if (archiver.IsReading()) {
		archiver.ArchiveString(&tmp);
		if (tmp.length()) {
			*handle = cgi.R_RegisterModel(tmp.c_str());
		} else {
			*handle = NULL;
		}
	} else {
		if (*handle) {
			tmp = cgi.R_GetModelName(*handle);
		} else {
			tmp = "";
		}

		archiver.ArchiveString(&tmp);
	}
}

void CG_ArchiveShaderHandle(MemArchiver& archiver, qhandle_t* handle)
{
    str tmp;

	if (archiver.IsReading()) {
		archiver.ArchiveString(&tmp);
		if (tmp.length()) {
			*handle = cgi.R_RegisterShader(tmp.c_str());
		} else {
			*handle = NULL;
		}
	} else {
		if (*handle) {
			tmp = cgi.R_GetShaderName(*handle);
		} else {
			tmp = "";
		}

		archiver.ArchiveString(&tmp);
	}
}

void CG_ArchiveRefEntity(MemArchiver& archiver, refEntity_t* ref)
{
    archiver.ArchiveRaw(&ref->reType, sizeof(ref->reType));
    archiver.ArchiveInteger(&ref->renderfx);

    CG_ArchiveModelHandle(archiver, &ref->hModel);
    CG_ArchiveModelHandle(archiver, &ref->hOldModel);

    archiver.ArchiveVec3(ref->lightingOrigin);
    archiver.ArchiveInteger(&ref->parentEntity);
    archiver.ArchiveVec3(ref->axis[0]);
    archiver.ArchiveVec3(ref->axis[1]);
    archiver.ArchiveVec3(ref->axis[2]);
    archiver.ArchiveBoolean(&ref->nonNormalizedAxes);
    archiver.ArchiveVec3(ref->origin);
    archiver.ArchiveRaw(ref->frameInfo, 192);
    archiver.ArchiveFloat(&ref->actionWeight);
    archiver.ArchiveShort(&ref->wasframe);
    archiver.ArchiveFloat(&ref->scale);
    archiver.ArchiveVec3(ref->oldorigin);
    archiver.ArchiveInteger(&ref->skinNum);

    CG_ArchiveShaderHandle(archiver, &ref->customShader);
    archiver.ArchiveRaw(ref->shaderRGBA, 4);
    archiver.ArchiveFloat(ref->shaderTexCoord);
    archiver.ArchiveFloat(&ref->shaderTexCoord[1]);
    archiver.ArchiveFloat(&ref->shaderTime);

    archiver.ArchiveInteger(&ref->entityNumber);
    archiver.ArchiveRaw(ref->surfaces, 32);
    archiver.ArchiveFloat(ref->shader_data);
    archiver.ArchiveFloat(&ref->shader_data[1]);

    ref->bone_tag = NULL;
    ref->bone_quat = NULL;
    ref->of = NULL;
    ref->nf = NULL;

    CG_ArchiveTikiPointer(archiver, &ref->tiki);

    archiver.ArchiveInteger(&ref->bonestart);
    archiver.ArchiveInteger(&ref->morphstart);
    archiver.ArchiveBoolean(&ref->hasMorph);
    archiver.ArchiveFloat(&ref->radius);
    archiver.ArchiveFloat(&ref->rotation);
}
