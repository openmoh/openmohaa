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

// bg_compat.cpp

#include "./bg_compat.h"
#include "./qcommon.h"

static size_t CPT_NormalizeConfigstring_ver_6(size_t num) {
    if (num <= 5 || num >= 26) {
        return num;
    }

    return num - 2;
}

static size_t CPT_DenormalizeConfigstring_ver_6(size_t num) {
    if (num <= 5 || num >= 26) {
        return num;
    }

    return num + 2;
}

static size_t CPT_NormalizeConfigstring_ver_15(size_t num) {
    return num;
}

static size_t CPT_DenormalizeConfigstring_ver_15(size_t num) {
    return num;
}

static uint32_t CPT_NormalizePlayerStateFlags_ver_6(uint32_t flags) {
    uint32_t normalizedFlags = 0;

    // Convert AA PlayerMove flags to SH/BT flags
    normalizedFlags |= flags & (1 << 0);
    for (size_t i = 1; i < 32; ++i)
    {
        if (flags & (1 << (i + 2))) {
            normalizedFlags |= (1 << i);
        }
    }

    // So that flags are normalized across modules
    return normalizedFlags;
}

static uint32_t CPT_DenormalizePlayerStateFlags_ver_6(uint32_t flags) {
    uint32_t normalizedFlags = 0;

    // Convert AA PlayerMove flags to SH/BT flags
    normalizedFlags |= flags & (1 << 0);
    for (size_t i = 1; i < 32; ++i)
    {
		if (flags & (1 << i)) {
            normalizedFlags |= (1 << (i + 2));
		}
    }

    // So that flags are normalized across modules
    return normalizedFlags;
}

static int CPT_NormalizeViewModelAnim_ver_6(int iViewModelAnim) {
    return iViewModelAnim + 1;
}

static int CPT_DenormalizeViewModelAnim_ver_6(int iViewModelAnim) {
    return iViewModelAnim - 1;
}

static uint32_t CPT_NormalizePlayerStateFlags_ver_15(uint32_t flags) {
    return flags;
}

static uint32_t CPT_DenormalizePlayerStateFlags_ver_15(uint32_t flags) {
    return flags;
}

static int CPT_NormalizeViewModelAnim_ver_15(int iViewModelAnim) {
    return iViewModelAnim;
}

static int CPT_DenormalizeViewModelAnim_ver_15(int iViewModelAnim) {
    return iViewModelAnim;
}

size_t CPT_NormalizeConfigstring(size_t num)
{
    if (com_protocol->integer <= protocol_e::PROTOCOL_MOH)
    {
        return CPT_NormalizeConfigstring_ver_6(num);
    }
    else if (com_protocol->integer >= protocol_e::PROTOCOL_MOHTA_MIN)
    {
        return CPT_NormalizeConfigstring_ver_15(num);
    }
    else
    {
        Com_Error(ERR_DROP, "CPT Normalizer: unknown protocol %d", com_protocol->integer);
        return 0;
    }
}

size_t CPT_DenormalizeConfigstring(size_t num)
{
    if (com_protocol->integer <= protocol_e::PROTOCOL_MOH)
    {
        return CPT_DenormalizeConfigstring_ver_6(num);
    }
    else if (com_protocol->integer >= protocol_e::PROTOCOL_MOHTA_MIN)
    {
        return CPT_DenormalizeConfigstring_ver_15(num);
    }
    else
    {
        Com_Error(ERR_DROP, "CPT Normalizer: unknown protocol %d", com_protocol->integer);
        return 0;
    }
}

uint32_t CPT_NormalizePlayerStateFlags(uint32_t flags)
{
    if (com_protocol->integer <= protocol_e::PROTOCOL_MOH)
    {
        return CPT_NormalizePlayerStateFlags_ver_6(flags);
    }
    else if (com_protocol->integer >= protocol_e::PROTOCOL_MOHTA_MIN)
    {
        return CPT_NormalizePlayerStateFlags_ver_15(flags);
    }
    else
    {
        Com_Error(ERR_DROP, "CPT Normalizer: unknown protocol %d", com_protocol->integer);
        return 0;
    }
}

uint32_t CPT_DenormalizePlayerStateFlags(uint32_t flags)
{
    if (com_protocol->integer <= protocol_e::PROTOCOL_MOH)
    {
        return CPT_DenormalizePlayerStateFlags_ver_6(flags);
    }
    else if (com_protocol->integer >= protocol_e::PROTOCOL_MOHTA_MIN)
    {
        return CPT_DenormalizePlayerStateFlags_ver_15(flags);
    }
    else
    {
        Com_Error(ERR_DROP, "CPT Normalizer: unknown protocol %d", com_protocol->integer);
        return 0;
    }
}

int CPT_NormalizeViewModelAnim(int iViewModelAnim) {
    if (com_protocol->integer <= protocol_e::PROTOCOL_MOH)
    {
        return CPT_NormalizeViewModelAnim_ver_6(iViewModelAnim);
    }
    else if (com_protocol->integer >= protocol_e::PROTOCOL_MOHTA_MIN)
    {
        return CPT_NormalizeViewModelAnim_ver_15(iViewModelAnim);
    }
    else
    {
        Com_Error(ERR_DROP, "CPT Normalizer: unknown protocol %d", com_protocol->integer);
        return 0;
    }
}

int CPT_DenormalizeViewModelAnim(int iViewModelAnim) {
    if (com_protocol->integer <= protocol_e::PROTOCOL_MOH)
    {
        return CPT_DenormalizeViewModelAnim_ver_6(iViewModelAnim);
    }
    else if (com_protocol->integer >= protocol_e::PROTOCOL_MOHTA_MIN)
    {
        return CPT_DenormalizeViewModelAnim_ver_15(iViewModelAnim);
    }
    else
    {
        Com_Error(ERR_DROP, "CPT Normalizer: unknown protocol %d", com_protocol->integer);
        return 0;
    }
}
