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

// skeletor_model_file_format.h : Skeletor model file format

#pragma once

static const char *SKEL_BONENAME_WORLD = "worldbone";

typedef enum boneType_e {
    SKELBONE_ROTATION,
    SKELBONE_POSROT,
    SKELBONE_IKSHOULDER,
    SKELBONE_IKELBOW,
    SKELBONE_IKWRIST,
    SKELBONE_HOSEROT,
    SKELBONE_AVROT,
    SKELBONE_ZERO,
    SKELBONE_NUMBONETYPES,
    SKELBONE_WORLD,
    SKELBONE_HOSEROTBOTH,
    SKELBONE_HOSEROTPARENT
} boneType_t;

typedef enum {
    HRTYPE_PLAIN,
    HRTYPE_ROTATEPARENT180Y,
    HRTYPE_ROTATEBOTH180Y
} hoseRotType_t;

typedef struct boneFileData_s {
    char       name[32];
    char       parent[32];
    boneType_t boneType;
    int        ofsBaseData;
    int        ofsChannelNames;
    int        ofsBoneNames;
    int        ofsEnd;
} boneFileData_t;

typedef struct skelHitBox_s {
    int boneIndex;
} skelHitBox_t;

typedef struct {
    int  ident;
    int  version;
    char name[64];
    int  numSurfaces;
    int  numBones;
    int  ofsBones;
    int  ofsSurfaces;
    int  ofsEnd;
    int  lodIndex[10];
    int  numBoxes;
    int  ofsBoxes;
    int  numMorphTargets;
    int  ofsMorphTargets;
} skelBaseHeader_t;

#if __cplusplus

#    include "SkelVec3.h"
#    include "SkelQuat.h"

struct boneData_s;

int CreateRotationBoneFileData(
    const char *newBoneName, const char *newBoneParentName, SkelVec3 basePos, boneFileData_t *fileData
);
int  CreatePosRotBoneFileData(char *newBoneName, char *newBoneParentName, boneFileData_t *fileData);
void CreatePosRotBoneData(const char *newBoneName, const char *newBoneParentName, boneData_s *boneData);
int  CreateIKShoulderBoneFileData(
     const char *newBoneName, const char *newBoneParentName, SkelQuat baseOrient, SkelVec3 basePos, boneData_s *boneData
 );
int CreateIKElbowBoneFileData(
    const char *newBoneName, const char *newBoneParentName, SkelVec3 basePos, boneData_s *boneData
);
int CreateIKWristBoneFileData(
    const char     *newBoneName,
    const char     *newBoneParentName,
    const char     *shoulderBoneName,
    SkelVec3        basePos,
    boneFileData_t *fileData
);
int CreateHoseRotBoneFileData(
    char           *newBoneName,
    char           *newBoneParentName,
    char           *targetBoneName,
    float           bendRatio,
    float           bendMax,
    float           spinRatio,
    hoseRotType_t   hoseRotType,
    SkelVec3        basePos,
    boneFileData_t *fileData
);
int CreateAvRotBoneFileData(
    char           *newBoneName,
    char           *newBoneParentName,
    char           *baseBoneName,
    char           *targetBoneName,
    float           rotRatio,
    SkelVec3        basePos,
    boneFileData_t *fileData
);

#endif
