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

// SkelQuat.h : Skeletor Quat

#pragma once

#ifdef __cplusplus

#    include "SkelVec3.h"
#    include "SkelVec4.h"
#    include "SkelMat3.h"
#    include "SkelMat4.h"

class SkelQuat
{
public:
    union {
        float val[4];

        struct {
            float x;
            float y;
            float z;
            float w;
        };
    };

    SkelQuat();
    SkelQuat(SkelVec3 vec);
    SkelQuat(SkelMat3 mat3);
    SkelQuat(SkelMat4 mat4);
    SkelQuat(float x, float y, float z, float w);
    SkelQuat(float *quat);
    SkelQuat(const float *quat);
    SkelQuat(float w, const SkelVec3 *vec);

    float& operator[](int index);
    float  operator[](int index) const;

    operator float *();
    operator float *() const;

    void Set(float x, float y, float z, float w);
    void Set(SkelVec4 vec4);
    void Set(float *quat);
    void SetAngle(float a);
    void SetAxis(SkelVec3 vec);

    void  Invert();
    float Length() const;
    float LengthSquared() const;
    void  Normalize();

    void GetMat3(SkelMat3& mat3) const;
    void GetMat4(SkelMat4& mat4) const;
    void GetEulerAngles(float *angles) const;
    bool IsUnit() const;
    void MakeIdentity();
    bool IsIdentity();
    bool IsValid() const;
};

inline SkelQuat::SkelQuat()
{
    QuatClear(val);
}

inline SkelQuat::SkelQuat(SkelVec3 vec)
{
    EulerToQuat(vec.val, val);
}

inline SkelQuat::SkelQuat(SkelMat3 mat3)
{
    MatToQuat(mat3.val, val);
}

inline SkelQuat::SkelQuat(SkelMat4 mat4)
{
    MatToQuat(mat4.val, val);
}

inline SkelQuat::SkelQuat(float x, float y, float z, float w)
{
    Set(x, y, z, w);
}

inline SkelQuat::SkelQuat(float *quat)
{
    val[0] = quat[0];
    val[1] = quat[1];
    val[2] = quat[2];
    val[3] = quat[3];
}

inline SkelQuat::SkelQuat(const float *quat)
{
    val[0] = quat[0];
    val[1] = quat[1];
    val[2] = quat[2];
    val[3] = quat[3];
}

inline SkelQuat::SkelQuat(float w, const SkelVec3 *vec)
{
    val[0] = vec->x;
    val[1] = vec->y;
    val[2] = vec->z;
    val[3] = w;
}

inline void SkelQuat::Set(float x, float y, float z, float w)
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
}

inline void SkelQuat::Set(SkelVec4 vec4)
{
    val[0] = vec4.val[0];
    val[1] = vec4.val[1];
    val[2] = vec4.val[2];
    val[3] = vec4.val[3];
}

inline void SkelQuat::Set(float *quat)
{
    val[0] = quat[0];
    val[1] = quat[1];
    val[2] = quat[2];
    val[3] = quat[3];
}

inline void SkelQuat::SetAngle(float a)
{
    SkelVec3 vec;
    vec.y = a;
    EulerToQuat(vec.val, val);
}

inline void SkelQuat::SetAxis(SkelVec3 vec)
{
    EulerToQuat(vec.val, val);
}

inline void SkelQuat::Invert()
{
    QuatInverse(val);
}

inline float SkelQuat::Length() const
{
    return (float)sqrt(val[0] * val[0] + val[1] * val[1] + val[2] * val[2] + val[3] * val[3]);
}

inline float SkelQuat::LengthSquared() const
{
    return val[0] * val[0] + val[1] * val[1] + val[2] * val[2] + val[3] * val[3];
}

inline void SkelQuat::Normalize()
{
    VectorNormalize(val);
}

inline void SkelQuat::GetMat3(SkelMat3& mat3) const
{
    QuatToMat(val, mat3.val);
}

inline void SkelQuat::GetMat4(SkelMat4& mat4) const
{
    QuatToMat(val, mat4.val);
}

inline void SkelQuat::GetEulerAngles(float *angles) const
{
    QuatToAngles(val, angles);
}

inline bool SkelQuat::IsUnit() const
{
    // FIXME: stub
    return false;
}

inline void SkelQuat::MakeIdentity()
{
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    w = 1.0f;
}

inline bool SkelQuat::IsIdentity()
{
    return (x == 0.0f) && (y == 0.0f) && (z == 0.0f) && (w == 1.0f);
}

inline bool SkelQuat::IsValid() const
{
    // FIXME: stub
    return true;
}

inline void Slerp(SkelQuat& from, SkelQuat& to, float t, SkelQuat *out)
{
    static_assert(sizeof(float) == sizeof(int), "Float must be the same size as Int");
    float f;
    float f2;

    f  = from.x * to.x + from.y * to.y + from.z * to.z + from.w * to.w;
    f2 = 1.0 - t;

    // little hack because I don't know how can this operation be converted to float...
    *(int *)&f = (*(int *)&f & 0x80000000) ^ *(int *)&f2;

    out->x = to.x * t + from.x * f;
    out->y = to.y * t + from.y * f;
    out->z = to.z * t + from.z * f;
    out->w = to.w * t + from.w * f;

    f      = 1.0 / out->Length();
    out->x = out->x * f;
    out->y = out->y * f;
    out->z = out->z * f;
    out->w = out->w * f;
}

inline SkelQuat::operator float *()
{
    return val;
}

inline SkelQuat::operator float *() const
{
    return (float *)val;
}

inline float& SkelQuat::operator[](int index)
{
    return val[index];
}

inline float SkelQuat::operator[](int index) const
{
    return val[index];
}

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
} SkelQuat;

#endif
