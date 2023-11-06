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

// SkelVec4.h : Skeletor Vec4

#pragma once

#ifdef __cplusplus

class SkelVec4
{
public:
    union {
        float val[4];

        struct {
            float x;
            float y;
            float z;
            float w;
        } xyzw;
    };

protected:
    void copy(const SkelVec4&);

public:
    SkelVec4(float x, float y, float z, float w);
    SkelVec4(const SkelVec3& vec3, float w);
    SkelVec4(const float *vec4);

    operator float *();
    operator float *() const;

    void set(float x, float y, float z, float w);
};

#else

typedef struct {
    union {
        float val[4];

        struct {
            float x;
            float y;
            float z;
            float w;
        };
    };
} SkelVec4;

#endif
