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

// SkelVec3.h : Skeletor

#pragma once

typedef enum {
    svX,
    svY,
    svZ,
    svW
} SkelVec_Axis;

typedef enum {
    Vec3YAW,
    Vec3PITCH,
    Vec3ROLL
} YPR_Axes;

#ifdef __cplusplus

class SkelVec3
{
public:
    union {
        float val[3];

        struct {
            float x;
            float y;
            float z;
        };
    };

protected:
    void copy(const SkelVec3& skel);

public:
    SkelVec3(float x, float y, float z);
    SkelVec3(vec3_t vec);
    SkelVec3();

    operator float *();
    operator float *() const;

    float& operator[](int index);
    float  operator[](int index) const;

    const SkelVec3& operator+=(const SkelVec3& a);
    const SkelVec3& operator+=(vec3_t a);

    bool IsZero() const;
    bool IsUnit() const;
    void set(float x, float y, float z);

    float Normalize();
    void  NormalizeFast();

    void SetZero();
    void SetXAxis();
    void SetYAxis();
    void SetZAxis();
    void RotateYaw(float yaw, float deg);
};

inline SkelVec3::SkelVec3(float x, float y, float z)
{
    set(x, y, z);
}

inline SkelVec3::SkelVec3(vec3_t vec)
{
    this->x = vec[0];
    this->y = vec[1];
    this->z = vec[2];
}

inline SkelVec3::SkelVec3()
{
    SetZero();
}

inline SkelVec3::operator float *()
{
    return val;
}

inline SkelVec3::operator float *() const
{
    return (float *)val;
}

inline bool SkelVec3::IsZero() const
{
    return (x == 0.0f) && (y == 0.0f) && (z == 0.0f);
}

inline bool SkelVec3::IsUnit() const
{
    // FIXME: stub
    return false;
}

inline void SkelVec3::set(float x, float y, float z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

inline float SkelVec3::Normalize()
{
    return VectorNormalize(val);
}

inline void SkelVec3::NormalizeFast()
{
    VectorNormalizeFast(val);
}

inline void SkelVec3::SetZero()
{
    this->x = 0.0f;
    this->y = 0.0f;
    this->z = 0.0f;
}

inline void SkelVec3::SetXAxis()
{
    x = 0.0f;
}

inline void SkelVec3::SetYAxis()
{
    y = 0.0f;
}

inline void SkelVec3::SetZAxis()
{
    z = 0.0f;
}

inline void SkelVec3::RotateYaw(float yaw, float deg)
{
    // FIXME: stub
}

inline float& SkelVec3::operator[](int index)
{
    return val[index];
}

inline float SkelVec3::operator[](int index) const
{
    return val[index];
}

inline const SkelVec3& SkelVec3::operator+=(const SkelVec3& a)
{
    x += a.x;
    y += a.y;
    z += a.z;

    return *this;
}

inline const SkelVec3& SkelVec3::operator+=(vec3_t a)
{
    x += a[0];
    y += a[1];
    z += a[2];

    return *this;
}

#else

typedef struct {
    union {
        float val[3];

        struct {
            float x;
            float y;
            float z;
        };
    };
} SkelVec3;

#endif
