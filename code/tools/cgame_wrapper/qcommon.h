/*
===========================================================================
Copyright (C) 2012 Michael Rieder

This file is part of OpenMohaa source code.

OpenMohaa source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMohaa source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMohaa source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
#include <string.h>

typedef unsigned char byte;
typedef enum {
	qfalse,
	qtrue
} qboolean;

typedef float vec_t;
typedef float vec2_t[2];
typedef float vec3_t[3];
typedef float vec4_t[4];
typedef float vec5_t[5];
typedef vec3_t SkelVec3;
typedef vec_t matrix_t[16];

#define DotProduct(x,y)			((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define VectorSubtract(a,b,c)	((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2])
#define VectorAdd(a,b,c)		((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2])
#define VectorCopy(a,b)			((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
#define	VectorScale(v, s, o)	((o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s))
#define	VectorMA(v, s, b, o)	((o)[0]=(v)[0]+(b)[0]*(s),(o)[1]=(v)[1]+(b)[1]*(s),(o)[2]=(v)[2]+(b)[2]*(s))
#define VectorSet(v, x, y, z)	((v)[0]=(x), (v)[1]=(y), (v)[2]=(z))

typedef struct SkelMat4 { /* size 48 id 162 */
  float val[4][3]; /* bitsize 384, bitpos 0 */
} SkelMat4_t;

typedef struct SkelQuat_t {
	float val[4];
} SkelQuat;

typedef int qhandle_t;
typedef int sfxHandle_t;
typedef int fileHandle_t;
typedef int clipHandle_t;

typedef enum {
	STEREO_CENTER,
	STEREO_LEFT,
	STEREO_RIGHT
} stereoFrame_t;

typedef struct cvar_s { /* size 44 id 30 */
  char *name; /* bitsize 32, bitpos 0 */
  char *string; /* bitsize 32, bitpos 32 */
  char *resetString; /* bitsize 32, bitpos 64 */
  char *latchedString; /* bitsize 32, bitpos 96 */
  int flags; /* bitsize 32, bitpos 128 */
  qboolean modified; /* bitsize 32, bitpos 160 */
  int modificationCount; /* bitsize 32, bitpos 192 */
  float value; /* bitsize 32, bitpos 224 */
  int integer; /* bitsize 32, bitpos 256 */
  struct cvar_s /* id 30 */ *next; /* bitsize 32, bitpos 288 */
  struct cvar_s /* id 30 */ *hashNext; /* bitsize 32, bitpos 320 */
} cvar_t;

typedef struct letterloc_s { /* size 16 */
  float pos[2]; /* bitsize 64, bitpos 0 */
  float size[2]; /* bitsize 64, bitpos 64 */
} letterloc_t;

typedef struct fontheader_s { /* size 5200 */
  int indirection[256]; /* bitsize 8192, bitpos 0 */
  letterloc_t locations[256]; /* bitsize 32768, bitpos 8192 */
  char name[64]; /* bitsize 512, bitpos 40960 */
  float height; /* bitsize 32, bitpos 41472 */
  float aspectRatio; /* bitsize 32, bitpos 41504 */
  void *shader; /* bitsize 32, bitpos 41536 */
  int trhandle; /* bitsize 32, bitpos 41568 */
} fontheader_t;

typedef struct hdelement_s { /* size 2228 id 44 */
  qhandle_t hShader; /* bitsize 32, bitpos 0 */
  char shaderName[64]; /* bitsize 512, bitpos 32 */
  int iX; /* bitsize 32, bitpos 544 */
  int iY; /* bitsize 32, bitpos 576 */
  int iWidth; /* bitsize 32, bitpos 608 */
  int iHeight; /* bitsize 32, bitpos 640 */
  float vColor[4]; /* bitsize 128, bitpos 672 */
  int iHorizontalAlign; /* bitsize 32, bitpos 800 */
  int iVerticalAlign; /* bitsize 32, bitpos 832 */
  qboolean bVirtualScreen; /* bitsize 32, bitpos 864 */
  char string[2048]; /* bitsize 16384, bitpos 896 */
  char fontName[64]; /* bitsize 512, bitpos 17280 */
  fontheader_t *pFont; /* bitsize 32, bitpos 17792 */
} hdelement_t;

typedef struct frameInfo_s { /* size 12 id 21 */
  int index; /* bitsize 32, bitpos 0 */
  float time; /* bitsize 32, bitpos 32 */
  float weight; /* bitsize 32, bitpos 64 */
} frameInfo_t;

typedef struct clientAnim_s { /* size 312 */
  frameInfo_t g_VMFrameInfo[16]; /* bitsize 1536, bitpos 0 */
  int g_iLastVMAnim; /* bitsize 32, bitpos 1536 */
  int g_iLastVMAnimChanged; /* bitsize 32, bitpos 1568 */
  int g_iCurrentVMAnimSlot; /* bitsize 32, bitpos 1600 */
  int g_iCurrentVMDuration; /* bitsize 32, bitpos 1632 */
  qboolean g_bCrossblending; /* bitsize 32, bitpos 1664 */
  int g_iLastEquippedWeaponStat; /* bitsize 32, bitpos 1696 */
  char g_szLastActiveItem[80]; /* bitsize 640, bitpos 1728 */
  int g_iLastAnimPrefixIndex; /* bitsize 32, bitpos 2368 */
  float g_vCurrentVMPosOffset[3]; /* bitsize 96, bitpos 2400 */
} clientAnim_t;

typedef struct baseshader_s { /* size 72 */
  char shader[64]; /* bitsize 512, bitpos 0 */
  int surfaceFlags; /* bitsize 32, bitpos 512 */
  int contentFlags; /* bitsize 32, bitpos 544 */
} baseshader_t;

typedef struct stopwatch_s { /* size 8 id 112 */
  int iStartTime; /* bitsize 32, bitpos 0 */
  int iEndTime; /* bitsize 32, bitpos 32 */
} stopwatch_t;

typedef enum {
	RT_MODEL,
	RT_SPRITE,
	RT_BEAM,
	RT_PORTALSURFACE,
	RT_MAX_REF_ENTITY_TYPE
} refEntityType_t;

typedef struct refEntity_s { /* size 408 */
  refEntityType_t reType; /* bitsize 32, bitpos 0 */
  int renderfx; /* bitsize 32, bitpos 32 */
  qhandle_t hModel; /* bitsize 32, bitpos 64 */
  qhandle_t hOldModel; /* bitsize 32, bitpos 96 */
  float lightingOrigin[3]; /* bitsize 96, bitpos 128 */
  int parentEntity; /* bitsize 32, bitpos 224 */
  float axis[3][3]; /* bitsize 288, bitpos 256 */
  qboolean nonNormalizedAxes; /* bitsize 32, bitpos 544 */
  float origin[3]; /* bitsize 96, bitpos 576 */
  frameInfo_t frameInfo[16]; /* bitsize 1536, bitpos 672 */
  float actionWeight; /* bitsize 32, bitpos 2208 */
  short int wasframe; /* bitsize 16, bitpos 2240 */
  float scale; /* bitsize 32, bitpos 2272 */
  float oldorigin[3]; /* bitsize 96, bitpos 2304 */
  int skinNum; /* bitsize 32, bitpos 2400 */
  qhandle_t customShader; /* bitsize 32, bitpos 2432 */
  unsigned char shaderRGBA[4]; /* bitsize 32, bitpos 2464 */
  float shaderTexCoord[2]; /* bitsize 64, bitpos 2496 */
  float shaderTime; /* bitsize 32, bitpos 2560 */
  int entityNumber; /* bitsize 32, bitpos 2592 */
  unsigned char surfaces[32]; /* bitsize 256, bitpos 2624 */
  float shader_data[2]; /* bitsize 64, bitpos 2880 */
  int *bone_tag; /* bitsize 32, bitpos 2944 */
  float (*bone_quat)[4]; /* bitsize 32, bitpos 2976 */
  struct tikiFrame_s /* id 22 */ *of; /* bitsize 32, bitpos 3008 */
  struct tikiFrame_s /* id 22 */ *nf; /* bitsize 32, bitpos 3040 */
  struct dtiki_s /* id 19 */ *tiki; /* bitsize 32, bitpos 3072 */
  int bonestart; /* bitsize 32, bitpos 3104 */
  int morphstart; /* bitsize 32, bitpos 3136 */
  qboolean hasMorph; /* bitsize 32, bitpos 3168 */
  float radius; /* bitsize 32, bitpos 3200 */
  float rotation; /* bitsize 32, bitpos 3232 */
} refEntity_t;

typedef struct orientation_s { /* size 48 id 34 */
  float origin[3]; /* bitsize 96, bitpos 0 */
  float axis[3][3]; /* bitsize 288, bitpos 96 */
} orientation_t;

typedef struct dtikianim_s { /* size 80 id 127 */
  char *name; /* bitsize 32, bitpos 0 */
  int num_anims; /* bitsize 32, bitpos 32 */
  void *alias_list; /* bitsize 32, bitpos 64 */
  int num_client_initcmds; /* bitsize 32, bitpos 96 */
  struct dtikicmd_s /* id 128 */ *client_initcmds; /* bitsize 32, bitpos 128 */
  int num_server_initcmds; /* bitsize 32, bitpos 160 */
  struct dtikicmd_s /* id 128 */ *server_initcmds; /* bitsize 32, bitpos 192 */
  byte *modelData; /* bitsize 32, bitpos 224 */
  int modelDataSize; /* bitsize 32, bitpos 256 */
  float mins[3]; /* bitsize 96, bitpos 288 */
  float maxs[3]; /* bitsize 96, bitpos 384 */
  short int *m_aliases; /* bitsize 32, bitpos 480 */
  char *headmodels; /* bitsize 32, bitpos 512 */
  char *headskins; /* bitsize 32, bitpos 544 */
  qboolean bIsCharacter; /* bitsize 32, bitpos 576 */
  struct dtikianimdef_s /* id 129 */ *animdefs[/*1*/512]; /* bitsize 32, bitpos 608 */
} dtikianim_t;

typedef struct tiki_singlecmd_s { /* size 8 */
  int num_args; /* bitsize 32, bitpos 0 */
  char **args; /* bitsize 32, bitpos 32 */
} tiki_singlecmd_t;

typedef struct tiki_cmd_s { /* size 1028 */
  int num_cmds; /* bitsize 32, bitpos 0 */
  tiki_singlecmd_t cmds[128]; /* bitsize 8192, bitpos 32 */
} tiki_cmd_t;

typedef struct skelChannelList_s { /* size 408 id 126 */
// public:
  short int m_numChannels; /* bitsize 16, bitpos 0 */
// private:
  short int m_numLocalFromGlobal; /* bitsize 16, bitpos 16 */
  short int *m_chanLocalFromGlobal; /* bitsize 32, bitpos 32 */
// public:
  short int m_chanGlobalFromLocal[200]; /* bitsize 3200, bitpos 64 */
} skelChannelList_c;

typedef struct skelAnimGameFrame_s { /* size 48 */
    SkelVec3 bounds[2]; /* bitsize 192, bitpos 0 */
    float radius; /* bitsize 32, bitpos 192 */
    SkelVec3 delta; /* bitsize 96, bitpos 224 */
    float angleDelta; /* bitsize 32, bitpos 320 */
    float (*pChannels)[4]; /* bitsize 32, bitpos 352 */
} skelAnimGameFrame_t;

typedef struct skanGameFrame_s { /* size 20 */
  short int nFrameNum; /* bitsize 16, bitpos 0 */
  short int nPrevFrameIndex; /* bitsize 16, bitpos 16 */
  float pChannelData[4]; /* bitsize 128, bitpos 32 */
} skanGameFrame;

typedef struct skanChannelHdr_s { /* size 8 */
  short int nFramesInChannel; /* bitsize 16, bitpos 0 */
  skanGameFrame *ary_frames; /* bitsize 32, bitpos 32 */
} skanChannelHdr;

typedef struct skelAnimDataGameHeader_s { /* size 484 id 1242 */
  int flags; /* bitsize 32, bitpos 0 */
  int nBytesUsed; /* bitsize 32, bitpos 32 */
  byte bHasDelta; /* bitsize 8, bitpos 64 */
  byte bHasMorph; /* bitsize 8, bitpos 72 */
  byte bHasUpper; /* bitsize 8, bitpos 80 */
  int numFrames; /* bitsize 32, bitpos 96 */
  SkelVec3 totalDelta; /* bitsize 96, bitpos 128 */
  float totalAngleDelta; /* bitsize 32, bitpos 224 */
  float frameTime; /* bitsize 32, bitpos 256 */
  skelChannelList_c /* id 146 */ channelList; /* bitsize 3264, bitpos 288 */
  SkelVec3 bounds[2]; /* bitsize 192, bitpos 3552 */
  skelAnimGameFrame_t *m_frame; /* bitsize 32, bitpos 3744 */
  short int nTotalChannels; /* bitsize 16, bitpos 3776 */
  skanChannelHdr ary_channels[1]; /* bitsize 64, bitpos 3808 */
} skelAnimDataGameHeader_t;

typedef struct skanBlendInfo_s { /* size 12 */
  float weight; /* bitsize 32, bitpos 0 */
  struct skelAnimDataGameHeader_s /* id 848 */ *pAnimationData; /* bitsize 32, bitpos 32 */
  int frame; /* bitsize 32, bitpos 64 */
} skanBlendInfo;

typedef struct skelAnimStoreFrameList_s { /* size 776 id 851 */
  short int numMovementFrames; /* bitsize 16, bitpos 0 */
  short int numActionFrames; /* bitsize 16, bitpos 16 */
  float actionWeight; /* bitsize 32, bitpos 32 */
  skanBlendInfo m_blendInfo[64]; /* bitsize 6144, bitpos 64 */
} skelAnimStoreFrameList_c;

// su44: these are indexes in skelBone_c class vtable
// they might be different in different builds of MOH
typedef enum skelBoneVTBLFuncType_e {
	BONEFUNC_GETDIRTYTRANFORM,
	BONEFUNC_SETBASEVAL,
	BONEFUNC_GETCHANNELINDEX,
	BONEFUNC_GETBONEREF,
} skelBoneVTBLFuncType_t;

typedef struct {
	int funcs[16];
} skelBoneVTable_t;

typedef struct skelBone_Base { /* size 64 vtable self  id 855 */
 skelBoneVTable_t *vptr; // su44: IMHO vtable is at the beginning of the struct, not at the end
// public:
  qboolean m_isDirty; /* bitsize 32, bitpos 0 */
// protected:
  struct skelBone_Base /* id 855 */ *m_parent; /* bitsize 32, bitpos 32 */
  struct SkelMat4 /* id 183 */ m_cachedValue; /* bitsize 384, bitpos 64 */
// public:
  float *m_controller; /* bitsize 32, bitpos 448 */
// private:
//  __vtbl_ptr_type *_vptr$; /* bitpos 480 */
} skelBone_Base_c;

typedef struct skelBone_HoseRot /*: public skelBone_Base*/ { /* size 108 vtable skelBone_Base  id 1229 */
	// inherited from base
 skelBoneVTable_t *vptr; // su44: IMHO vtable is at the beginning of the struct, not at the end
  qboolean m_isDirty; /* bitsize 32, bitpos 0 */
  struct skelBone_Base /* id 855 */ *m_parent; /* bitsize 32, bitpos 32 */
  struct SkelMat4 /* id 183 */ m_cachedValue; /* bitsize 384, bitpos 64 */
  float *m_controller; /* bitsize 32, bitpos 448 */
	// hoserot specific
  SkelVec3 /* id 1221 */ m_basePos; /* bitsize 96, bitpos 512 */
  SkelQuat /* id 1226 */ m_cachedQuat; /* bitsize 128, bitpos 608 */
  skelBone_Base_c /* id 855 */ *m_target; /* bitsize 32, bitpos 736 */
  float m_bendRatio; /* bitsize 32, bitpos 768 */
  float m_bendMax; /* bitsize 32, bitpos 800 */
  float m_spinRatio; /* bitsize 32, bitpos 832 */
} skelBone_HoseRot_c;

typedef struct skelBone_IKshoulder /*: public skelBone_Base*/ { /* size 120 vtable skelBone_Base  id 1224 */
// public:
	// inherited from base
 skelBoneVTable_t *vptr; // su44: IMHO vtable is at the beginning of the struct, not at the end
  qboolean m_isDirty; /* bitsize 32, bitpos 0 */
  struct skelBone_Base /* id 855 */ *m_parent; /* bitsize 32, bitpos 32 */
  struct SkelMat4 /* id 183 */ m_cachedValue; /* bitsize 384, bitpos 64 */
  float *m_controller; /* bitsize 32, bitpos 448 */
	// shoulder specific
  struct skelBone_IKwrist /* id 1225 */ *m_wrist; /* bitsize 32, bitpos 512 */
  float m_upperLength; /* bitsize 32, bitpos 544 */
  float m_lowerLength; /* bitsize 32, bitpos 576 */
  SkelVec3 /* id 1221 */ m_baseValue; /* bitsize 96, bitpos 608 */
  float m_cosElbowAngle; /* bitsize 32, bitpos 704 */
  SkelVec3 /* id 1221 */ m_wristPos; /* bitsize 96, bitpos 736 */
  SkelQuat /* id 1226 */ m_wristAngle; /* bitsize 128, bitpos 832 */
  //class skelBone_IKshoulder /* id 1224 */ &class skelBone_IKshoulder /* id 1224 */::__as (class skelBone_IKshoulder /* id 1224 */ const &) /* __as__19skelBone_IKshoulderRC19skelBone_IKshoulder  */;
  //class skelBone_IKshoulder /* id 1224 */ *class skelBone_IKshoulder /* id 1224 */::skelBone_IKshoulder (class skelBone_IKshoulder /* id 1224 */ const &) /* __19skelBone_IKshoulderRC19skelBone_IKshoulder  */;
  //class skelBone_IKshoulder /* id 1224 */ *class skelBone_IKshoulder /* id 1224 */::skelBone_IKshoulder () /* __19skelBone_IKshoulder  */;
  //class SkelMat4 /* id 183 */ *class skelBone_IKshoulder /* id 1224 */::GetDirtyTransform (class skelAnimStoreFrameList_c /* id 1218 */ const *) /* GetDirtyTransform__19skelBone_IKshoulderPC24skelAnimStoreFrameList_c context skelBone_Base voffset 2 */;
  //void class skelBone_IKshoulder /* id 1224 */::SetBaseValue (class boneData_s /* id 158 */ *) /* SetBaseValue__19skelBone_IKshoulderP10boneData_s context skelBone_Base voffset 3 */;
  //int class skelBone_IKshoulder /* id 1224 */::GetChannelIndex (int) /* GetChannelIndex__19skelBone_IKshoulderi context skelBone_Base voffset 4 */;
  //class skelBone_Base /* id 855 */ *class skelBone_IKshoulder /* id 1224 */::GetBoneRef (int) /* GetBoneRef__19skelBone_IKshoulderi context skelBone_Base voffset 5 */;
  //void class skelBone_IKshoulder /* id 1224 */::SetElbowValue (float) /* SetElbowValue__19skelBone_IKshoulderf  */;
  //void class skelBone_IKshoulder /* id 1224 */::SetWristValue (float) /* SetWristValue__19skelBone_IKshoulderf  */;
  //void class skelBone_IKshoulder /* id 1224 */::SetWristBone (class skelBone_IKwrist /* id 1225 */ *) /* SetWristBone__19skelBone_IKshoulderP16skelBone_IKwrist  */;
  //float class skelBone_IKshoulder /* id 1224 */::GetUpperLength () /* GetUpperLength__19skelBone_IKshoulder  */;
  //float class skelBone_IKshoulder /* id 1224 */::GetLowerLength () /* GetLowerLength__19skelBone_IKshoulder  */;
} skelBone_IKshoulder_c;

typedef struct skelBone_IKelbow/* : public skelBone_Base*/ { /* size 68 vtable skelBone_Base  id 1227 */
	// inherited from base
 skelBoneVTable_t *vptr; // su44: IMHO vtable is at the beginning of the struct, not at the end
  qboolean m_isDirty; /* bitsize 32, bitpos 0 */
  struct skelBone_Base /* id 855 */ *m_parent; /* bitsize 32, bitpos 32 */
  struct SkelMat4 /* id 183 */ m_cachedValue; /* bitsize 384, bitpos 64 */
  float *m_controller; /* bitsize 32, bitpos 448 */
	// elbow specific
  skelBone_IKshoulder_c *m_shoulder; /* bitsize 32, bitpos 512 */
  //class skelBone_IKelbow /* id 1227 */ &class skelBone_IKelbow /* id 1227 */::__as (class skelBone_IKelbow /* id 1227 */ const &) /* __as__16skelBone_IKelbowRC16skelBone_IKelbow  */;
  //class skelBone_IKelbow /* id 1227 */ *class skelBone_IKelbow /* id 1227 */::skelBone_IKelbow (class skelBone_IKelbow /* id 1227 */ const &) /* __16skelBone_IKelbowRC16skelBone_IKelbow  */;
  //class skelBone_IKelbow /* id 1227 */ *class skelBone_IKelbow /* id 1227 */::skelBone_IKelbow () /* __16skelBone_IKelbow  */;
  //class SkelMat4 /* id 183 */ *class skelBone_IKelbow /* id 1227 */::GetDirtyTransform (class skelAnimStoreFrameList_c /* id 1218 */ const *) /* GetDirtyTransform__16skelBone_IKelbowPC24skelAnimStoreFrameList_c context skelBone_Base voffset 2 */;
  //void class skelBone_IKelbow /* id 1227 */::SetBoneRefs (skelBone_IKshoulder *) /* SetBoneRefs__16skelBone_IKelbowP19skelBone_IKshoulder  */;
  //void class skelBone_IKelbow /* id 1227 */::SetBaseValue (class boneData_s /* id 158 */ *) /* SetBaseValue__16skelBone_IKelbowP10boneData_s context skelBone_Base voffset 3 */;
  //int class skelBone_IKelbow /* id 1227 */::GetChannelIndex (int) /* GetChannelIndex__16skelBone_IKelbowi context skelBone_Base voffset 4 */;
  //class skelBone_Base /* id 855 */ *class skelBone_IKelbow /* id 1227 */::GetBoneRef (int) /* GetBoneRef__16skelBone_IKelbowi context skelBone_Base voffset 5 */;
} skelBone_IKelbow_c;
typedef struct skelBone_IKwrist /* : public skelBone_Base*/ { /* size 76 vtable skelBone_Base  id 1225 */
	// inherited from base
 skelBoneVTable_t *vptr; // su44: IMHO vtable is at the beginning of the struct, not at the end
  qboolean m_isDirty; /* bitsize 32, bitpos 0 */
  struct skelBone_Base /* id 855 */ *m_parent; /* bitsize 32, bitpos 32 */
  struct SkelMat4 /* id 183 */ m_cachedValue; /* bitsize 384, bitpos 64 */
  float *m_controller; /* bitsize 32, bitpos 448 */
	// wrist specific
  skelBone_IKshoulder_c *m_shoulder; /* bitsize 32, bitpos 512 */
  int m_quatChannel; /* bitsize 32, bitpos 544 */
  int m_offsetChannel; /* bitsize 32, bitpos 576 */
  //class skelBone_IKwrist /* id 1225 */ &class skelBone_IKwrist /* id 1225 */::__as (class skelBone_IKwrist /* id 1225 */ const &) /* __as__16skelBone_IKwristRC16skelBone_IKwrist  */;
  //class skelBone_IKwrist /* id 1225 */ *class skelBone_IKwrist /* id 1225 */::skelBone_IKwrist (class skelBone_IKwrist /* id 1225 */ const &) /* __16skelBone_IKwristRC16skelBone_IKwrist  */;
  //class skelBone_IKwrist /* id 1225 */ *class skelBone_IKwrist /* id 1225 */::skelBone_IKwrist () /* __16skelBone_IKwrist  */;
  //class SkelMat4 /* id 183 */ *class skelBone_IKwrist /* id 1225 */::GetDirtyTransform (class skelAnimStoreFrameList_c /* id 1218 */ const *) /* GetDirtyTransform__16skelBone_IKwristPC24skelAnimStoreFrameList_c context skelBone_Base voffset 2 */;
  //void class skelBone_IKwrist /* id 1225 */::SetChannels (int, int) /* SetChannels__16skelBone_IKwristii  */;
  //void class skelBone_IKwrist /* id 1225 */::SetBoneRefs (skelBone_IKshoulder *) /* SetBoneRefs__16skelBone_IKwristP19skelBone_IKshoulder  */;
  //void class skelBone_IKwrist /* id 1225 */::SetBaseValue (class boneData_s /* id 158 */ *) /* SetBaseValue__16skelBone_IKwristP10boneData_s context skelBone_Base voffset 3 */;
  //int class skelBone_IKwrist /* id 1225 */::GetChannelIndex (int) /* GetChannelIndex__16skelBone_IKwristi context skelBone_Base voffset 4 */;
  //class skelBone_Base /* id 855 */ *class skelBone_IKwrist /* id 1225 */::GetBoneRef (int) /* GetBoneRef__16skelBone_IKwristi context skelBone_Base voffset 5 */;
} skelBone_IKwrist_c;
typedef struct boneData_s { /* size 56 id 158 */
  short int channel; /* bitsize 16, bitpos 0 */
  int type; /* bitsize 32, bitpos 32 */
  short int parent; /* bitsize 16, bitpos 64 */
  short int numChannels; /* bitsize 16, bitpos 80 */
  short int numRefs; /* bitsize 16, bitpos 96 */
  short int channelIndex[2]; /* bitsize 32, bitpos 112 */
  short int refIndex[2]; /* bitsize 32, bitpos 144 */
  float offset[3]; /* bitsize 96, bitpos 192 */
  float length; /* bitsize 32, bitpos 288 */
  float weight; /* bitsize 32, bitpos 320 */
  float bendRatio; /* bitsize 32, bitpos 352 */
  float bendMax; /* bitsize 32, bitpos 384 */
  float spinRatio; /* bitsize 32, bitpos 416 */
} boneData_t;
typedef struct skeletor_s { /* size 1268 id 2016 */
// public:
  struct dtiki_s /* id 104 */ *m_Tiki; /* bitsize 32, bitpos 0 */

// private:
  SkelVec3 m_frameBounds[2]; /* bitsize 192, bitpos 32 */
  float m_frameRadius; /* bitsize 32, bitpos 224 */
  skelAnimStoreFrameList_c m_frameList; /* bitsize 6208, bitpos 256 */
  short int m_targetLookLeft; /* bitsize 16, bitpos 6464 */
  short int m_targetLookRight; /* bitsize 16, bitpos 6480 */
  short int m_targetLookUp; /* bitsize 16, bitpos 6496 */
  short int m_targetLookDown; /* bitsize 16, bitpos 6512 */
  short int m_targetLookCrossed; /* bitsize 16, bitpos 6528 */
  short int m_targetBlink; /* bitsize 16, bitpos 6544 */
  short int m_timeNextBlink; /* bitsize 16, bitpos 6560 */
  short int m_headBoneIndex; /* bitsize 16, bitpos 6576 */
  float m_eyeTargetPos[3]; /* bitsize 96, bitpos 6592 */
  float m_eyePrevTargetPos[3]; /* bitsize 96, bitpos 6688 */
  struct skelBone_Base /* id 855 */ *m_leftFoot; /* bitsize 32, bitpos 6784 */
  struct skelBone_Base /* id 855 */ *m_rightFoot; /* bitsize 32, bitpos 6816 */
  skelChannelList_c m_morphTargetList; /* bitsize 3264, bitpos 6848 */
  struct skelBone_Base /* id 855 */ **m_bone; /* bitsize 32, bitpos 10112 */
} skeletor_c;

typedef struct dtikisurface_s { /* size 348 id 151 */
  char name[64]; /* bitsize 512, bitpos 0 */
  char shader[4][64]; /* bitsize 2048, bitpos 512 */
  int hShader[4]; /* bitsize 128, bitpos 2560 */
  int numskins; /* bitsize 32, bitpos 2688 */
  int flags; /* bitsize 32, bitpos 2720 */
  float damage_multiplier; /* bitsize 32, bitpos 2752 */
} dtikisurface_t;

typedef struct dtiki_s { /* size 476 id 19 */
  char *name; /* bitsize 32, bitpos 0 */
  dtikianim_t *a; /* bitsize 32, bitpos 32 */
  skeletor_c *skeletor; /* bitsize 32, bitpos 64 */
  int num_surfaces; /* bitsize 32, bitpos 96 */
  struct dtikisurface_s /* id 130 */ *surfaces; /* bitsize 32, bitpos 128 */
  float load_scale; /* bitsize 32, bitpos 160 */
  float lod_scale; /* bitsize 32, bitpos 192 */
  float lod_bias; /* bitsize 32, bitpos 224 */
  float light_offset[3]; /* bitsize 96, bitpos 256 */
  float load_origin[3]; /* bitsize 96, bitpos 352 */
  float radius; /* bitsize 32, bitpos 448 */
  skelChannelList_c m_boneList; /* bitsize 3264, bitpos 480 */
  int numMeshes; /* bitsize 32, bitpos 3744 */
  short int mesh[/*1*/16]; /* bitsize 16, bitpos 3776 */

//short int dummy;
} dtiki_t;

typedef struct trajectory_s { /* size 16 */
  int trTime; /* bitsize 32, bitpos 0 */
  float trDelta[3]; /* bitsize 96, bitpos 32 */
} trajectory_t;

typedef struct entityState_s { /* size 612 id 7 */
  int number; /* bitsize 32, bitpos 0 */
  int eType; /* bitsize 32, bitpos 32 */
  int eFlags; /* bitsize 32, bitpos 64 */
  trajectory_t pos; /* bitsize 128, bitpos 96 */
  float netorigin[3]; /* bitsize 96, bitpos 224 */
  float origin[3]; /* bitsize 96, bitpos 320 */
  float origin2[3]; /* bitsize 96, bitpos 416 */
  float netangles[3]; /* bitsize 96, bitpos 512 */
  float angles[3]; /* bitsize 96, bitpos 608 */
  int constantLight; /* bitsize 32, bitpos 704 */
  int loopSound; /* bitsize 32, bitpos 736 */
  float loopSoundVolume; /* bitsize 32, bitpos 768 */
  float loopSoundMinDist; /* bitsize 32, bitpos 800 */
  float loopSoundMaxDist; /* bitsize 32, bitpos 832 */
  float loopSoundPitch; /* bitsize 32, bitpos 864 */
  int loopSoundFlags; /* bitsize 32, bitpos 896 */
  int parent; /* bitsize 32, bitpos 928 */
  int tag_num; /* bitsize 32, bitpos 960 */
  qboolean attach_use_angles; /* bitsize 32, bitpos 992 */
  float attach_offset[3]; /* bitsize 96, bitpos 1024 */
  int beam_entnum; /* bitsize 32, bitpos 1120 */
  int modelindex; /* bitsize 32, bitpos 1152 */
  int usageIndex; /* bitsize 32, bitpos 1184 */
  int skinNum; /* bitsize 32, bitpos 1216 */
  int wasframe; /* bitsize 32, bitpos 1248 */
  frameInfo_t frameInfo[16]; /* bitsize 1536, bitpos 1280 */
  float actionWeight; /* bitsize 32, bitpos 2816 */
  int bone_tag[5]; /* bitsize 160, bitpos 2848 */
  float bone_angles[5][3]; /* bitsize 480, bitpos 3008 */
  float bone_quat[5][4]; /* bitsize 640, bitpos 3488 */
  unsigned char surfaces[32]; /* bitsize 256, bitpos 4128 */
  int clientNum; /* bitsize 32, bitpos 4384 */
  int groundEntityNum; /* bitsize 32, bitpos 4416 */
  int solid; /* bitsize 32, bitpos 4448 */
  float scale; /* bitsize 32, bitpos 4480 */
  float alpha; /* bitsize 32, bitpos 4512 */
  int renderfx; /* bitsize 32, bitpos 4544 */
  float shader_data[2]; /* bitsize 64, bitpos 4576 */
  float shader_time; /* bitsize 32, bitpos 4640 */
  float quat[4]; /* bitsize 128, bitpos 4672 */
  float eyeVector[3]; /* bitsize 96, bitpos 4800 */
} entityState_t;

typedef struct centity_s { /* size 1632 id 6 */
  entityState_t /* id 7 */ currentState; /* bitsize 4896, bitpos 0 */
  entityState_t /* id 7 */ nextState; /* bitsize 4896, bitpos 4896 */
  int teleported; /* bitsize 32, bitpos 9792 */
  int interpolate; /* bitsize 32, bitpos 9824 */
  int currentValid; /* bitsize 32, bitpos 9856 */
  int miscTime; /* bitsize 32, bitpos 9888 */
  int snapShotTime; /* bitsize 32, bitpos 9920 */
  int errorTime; /* bitsize 32, bitpos 9952 */
  float errorOrigin[3]; /* bitsize 96, bitpos 9984 */
  float errorAngles[3]; /* bitsize 96, bitpos 10080 */
  int extrapolated; /* bitsize 32, bitpos 10176 */
  float rawOrigin[3]; /* bitsize 96, bitpos 10208 */
  float rawAngles[3]; /* bitsize 96, bitpos 10304 */
  float beamEnd[3]; /* bitsize 96, bitpos 10400 */
  float lerpOrigin[3]; /* bitsize 96, bitpos 10496 */
  float lerpAngles[3]; /* bitsize 96, bitpos 10592 */
  int tikiLoopSound; /* bitsize 32, bitpos 10688 */
  float tikiLoopSoundVolume; /* bitsize 32, bitpos 10720 */
  float tikiLoopSoundMinDist; /* bitsize 32, bitpos 10752 */
  float tikiLoopSoundMaxDist; /* bitsize 32, bitpos 10784 */
  float tikiLoopSoundPitch; /* bitsize 32, bitpos 10816 */
  int tikiLoopSoundFlags; /* bitsize 32, bitpos 10848 */
  float color[4]; /* bitsize 128, bitpos 10880 */
  float client_color[4]; /* bitsize 128, bitpos 11008 */
  int clientFlags; /* bitsize 32, bitpos 11136 */
  int splash_last_spawn_time; /* bitsize 32, bitpos 11168 */
  int splash_still_count; /* bitsize 32, bitpos 11200 */
  float bone_quat[5][4]; /* bitsize 640, bitpos 11232 */
  float animLastTimes[16]; /* bitsize 512, bitpos 11872 */
  int animLast[16]; /* bitsize 512, bitpos 12384 */
  int animLastWeight; /* bitsize 32, bitpos 12896 */
  int usageIndexLast; /* bitsize 32, bitpos 12928 */
  int bFootOnGround_Right; /* bitsize 32, bitpos 12960 */
  int bFootOnGround_Left; /* bitsize 32, bitpos 12992 */
  int iNextLandTime; /* bitsize 32, bitpos 13024 */
} centity_t;

typedef struct gameState_s { /* size 42948 */
  int stringOffsets[2736]; /* bitsize 87552, bitpos 0 */
  char stringData[32000]; /* bitsize 256000, bitpos 87552 */
  int dataCount; /* bitsize 32, bitpos 343552 */
} gameState_t;

typedef struct server_sound_s {
	float origin[3];
	int entity_number;
	int channel;
	short int sound_index;
	float volume;
	float min_dist;
	float maxDist;
	float pitch;
	qboolean stop_flag;
	qboolean streamed;
} server_sound_t;

typedef struct cplane_s { /* size 20 id 32 */
  float normal[3]; /* bitsize 96, bitpos 0 */
  float dist; /* bitsize 32, bitpos 96 */
  byte type; /* bitsize 8, bitpos 128 */
  byte signbits; /* bitsize 8, bitpos 136 */
  unsigned char pad[2]; /* bitsize 16, bitpos 144 */
} cplane_t;

typedef struct trace_s { /* size 68 */
  qboolean allsolid; /* bitsize 32, bitpos 0 */
  qboolean startsolid; /* bitsize 32, bitpos 32 */
  float fraction; /* bitsize 32, bitpos 64 */
  float endpos[3]; /* bitsize 96, bitpos 96 */
  cplane_t plane; /* bitsize 160, bitpos 192 */
  int surfaceFlags; /* bitsize 32, bitpos 352 */
  int shaderNum; /* bitsize 32, bitpos 384 */
  int contents; /* bitsize 32, bitpos 416 */
  int entityNum; /* bitsize 32, bitpos 448 */
  int location; /* bitsize 32, bitpos 480 */
  struct gentity_s /* id 0 */ *ent; /* bitsize 32, bitpos 512 */
} trace_t;

typedef struct playerState_s { /* size 672 id 16 */
  int commandTime; /* bitsize 32, bitpos 0 */
  int pm_type; /* bitsize 32, bitpos 32 */
  int bobCycle; /* bitsize 32, bitpos 64 */
  int pm_flags; /* bitsize 32, bitpos 96 */
  int pm_runtime; /* bitsize 32, bitpos 128 */
  float origin[3]; /* bitsize 96, bitpos 160 */
  float velocity[3]; /* bitsize 96, bitpos 256 */
  int gravity; /* bitsize 32, bitpos 352 */
  int speed; /* bitsize 32, bitpos 384 */
  int delta_angles[3]; /* bitsize 96, bitpos 416 */
  int groundEntityNum; /* bitsize 32, bitpos 512 */
  qboolean walking; /* bitsize 32, bitpos 544 */
  qboolean groundPlane; /* bitsize 32, bitpos 576 */
  int feetfalling; /* bitsize 32, bitpos 608 */
  float falldir[3]; /* bitsize 96, bitpos 640 */
  trace_t groundTrace; /* bitsize 544, bitpos 736 */
  int clientNum; /* bitsize 32, bitpos 1280 */
  float viewangles[3]; /* bitsize 96, bitpos 1312 */
  int viewheight; /* bitsize 32, bitpos 1408 */
  float fLeanAngle; /* bitsize 32, bitpos 1440 */
  int iViewModelAnim; /* bitsize 32, bitpos 1472 */
  int iViewModelAnimChanged; /* bitsize 32, bitpos 1504 */
  int stats[32]; /* bitsize 1024, bitpos 1536 */
  int activeItems[8]; /* bitsize 256, bitpos 2560 */
  int ammo_name_index[16]; /* bitsize 512, bitpos 2816 */
  int ammo_amount[16]; /* bitsize 512, bitpos 3328 */
  int max_ammo_amount[16]; /* bitsize 512, bitpos 3840 */
  int current_music_mood; /* bitsize 32, bitpos 4352 */
  int fallback_music_mood; /* bitsize 32, bitpos 4384 */
  float music_volume; /* bitsize 32, bitpos 4416 */
  float music_volume_fade_time; /* bitsize 32, bitpos 4448 */
  int reverb_type; /* bitsize 32, bitpos 4480 */
  float reverb_level; /* bitsize 32, bitpos 4512 */
  float blend[4]; /* bitsize 128, bitpos 4544 */
  float fov; /* bitsize 32, bitpos 4672 */
  float camera_origin[3]; /* bitsize 96, bitpos 4704 */
  float camera_angles[3]; /* bitsize 96, bitpos 4800 */
  float camera_time; /* bitsize 32, bitpos 4896 */
  float camera_offset[3]; /* bitsize 96, bitpos 4928 */
  float camera_posofs[3]; /* bitsize 96, bitpos 5024 */
  int camera_flags; /* bitsize 32, bitpos 5120 */
  float damage_angles[3]; /* bitsize 96, bitpos 5152 */
  int ping; /* bitsize 32, bitpos 5248 */
  float vEyePos[3]; /* bitsize 96, bitpos 5280 */
} playerState_t;

typedef struct snapshot_s { /* size 630492 */
  int snapFlags; /* bitsize 32, bitpos 0 */
  int ping; /* bitsize 32, bitpos 32 */
  int serverTime; /* bitsize 32, bitpos 64 */
  unsigned char areamask[32]; /* bitsize 256, bitpos 96 */
  playerState_t ps; /* bitsize 5376, bitpos 352 */
  int numEntities; /* bitsize 32, bitpos 5728 */
  entityState_t entities[1024]; /* bitsize 5013504, bitpos 5760 */
  int numServerCommands; /* bitsize 32, bitpos 5019264 */
  int serverCommandSequence; /* bitsize 32, bitpos 5019296 */
  int number_of_sounds; /* bitsize 32, bitpos 5019328 */
  server_sound_t sounds[64]; /* bitsize 24576, bitpos 5019360 */
} snapshot_t;

typedef struct clientGameImport_s { /* size 684 */
  int apiversion; /* bitsize 32, bitpos 0 */
  void (*Printf) ( const char *fmt, ... );
  void (*DPrintf) (/* unknown */); /* bitsize 32, bitpos 64 */
  void (*DebugPrintf) (/* unknown */); /* bitsize 32, bitpos 96 */
  void *(*Malloc) (/* unknown */); /* bitsize 32, bitpos 128 */
  void (*Free) (/* unknown */); /* bitsize 32, bitpos 160 */
  void (*Error) ( int level, const char *fmt, ... ); /* bitsize 32, bitpos 192 */
  int (*Milliseconds) (/* unknown */); /* bitsize 32, bitpos 224 */
  char *(*LV_ConvertString) (/* unknown */); /* bitsize 32, bitpos 256 */
  cvar_t *(*Cvar_Get) ( const char *var_name, const char *value, int flags ); /* bitsize 32, bitpos 288 */
  void (*Cvar_Set) (/* unknown */); /* bitsize 32, bitpos 320 */
  int (*Argc) ( );
  char *(*Argv) ( int arg );
  char *(*Args) ( );
  void (*AddCommand) ( char *cmdName );
  void (*Cmd_Stuff) (/* unknown */); /* bitsize 32, bitpos 480 */
  void (*Cmd_Execute) (/* unknown */); /* bitsize 32, bitpos 512 */
  void (*Cmd_TokenizeString) (/* unknown */); /* bitsize 32, bitpos 544 */
  int (*FS_ReadFile) (/* unknown */); /* bitsize 32, bitpos 576 */
  void (*FS_FreeFile) (/* unknown */); /* bitsize 32, bitpos 608 */
  int (*FS_WriteFile) (/* unknown */); /* bitsize 32, bitpos 640 */
  void (*FS_WriteTextFile) (/* unknown */); /* bitsize 32, bitpos 672 */
  void (*SendConsoleCommand) (/* unknown */); /* bitsize 32, bitpos 704 */
  int (*MSG_ReadBits) (/* unknown */); /* bitsize 32, bitpos 736 */
  int (*MSG_ReadChar) (/* unknown */); /* bitsize 32, bitpos 768 */
  int (*MSG_ReadByte) (/* unknown */); /* bitsize 32, bitpos 800 */
  int (*MSG_ReadSVC) (/* unknown */); /* bitsize 32, bitpos 832 */
  int (*MSG_ReadShort) (/* unknown */); /* bitsize 32, bitpos 864 */
  int (*MSG_ReadLong) (/* unknown */); /* bitsize 32, bitpos 896 */
  float (*MSG_ReadFloat) (/* unknown */); /* bitsize 32, bitpos 928 */
  char *(*MSG_ReadString) (/* unknown */); /* bitsize 32, bitpos 960 */
  char *(*MSG_ReadStringLine) (/* unknown */); /* bitsize 32, bitpos 992 */
  float (*MSG_ReadAngle8) (/* unknown */); /* bitsize 32, bitpos 1024 */
  float (*MSG_ReadAngle16) (/* unknown */); /* bitsize 32, bitpos 1056 */
  void (*MSG_ReadData) (/* unknown */); /* bitsize 32, bitpos 1088 */
  float (*MSG_ReadCoord) (/* unknown */); /* bitsize 32, bitpos 1120 */
  void (*MSG_ReadDir) (/* unknown */); /* bitsize 32, bitpos 1152 */
  void (*SendClientCommand) (/* unknown */); /* bitsize 32, bitpos 1184 */
  void (*CM_LoadMap) (/* unknown */); /* bitsize 32, bitpos 1216 */
  clipHandle_t (*CM_InlineModel) (/* unknown */); /* bitsize 32, bitpos 1248 */
  int (*CM_NumInlineModels) (/* unknown */); /* bitsize 32, bitpos 1280 */
  int (*CM_PointContents) (/* unknown */); /* bitsize 32, bitpos 1312 */
  int (*CM_TransformedPointContents) (/* unknown */); /* bitsize 32, bitpos 1344 */
  void (*CM_BoxTrace) (/* unknown */); /* bitsize 32, bitpos 1376 */
  void (*CM_TransformedBoxTrace) (/* unknown */); /* bitsize 32, bitpos 1408 */
  clipHandle_t (*CM_TempBoxModel) (/* unknown */); /* bitsize 32, bitpos 1440 */
  void (*CM_PrintBSPFileSizes) (/* unknown */); /* bitsize 32, bitpos 1472 */
  qboolean (*CM_LeafInPVS) (/* unknown */); /* bitsize 32, bitpos 1504 */
  int (*CM_PointLeafnum) (/* unknown */); /* bitsize 32, bitpos 1536 */
  int (*R_MarkFragments) (/* unknown */); /* bitsize 32, bitpos 1568 */
  int (*R_MarkFragmentsForInlineModel) (/* unknown */); /* bitsize 32, bitpos 1600 */
  void (*R_GetInlineModelBounds) (/* unknown */); /* bitsize 32, bitpos 1632 */
  void (*R_GetLightingForDecal) (/* unknown */); /* bitsize 32, bitpos 1664 */
  void (*R_GetLightingForSmoke) (/* unknown */); /* bitsize 32, bitpos 1696 */
  int (*R_GatherLightSources) (/* unknown */); /* bitsize 32, bitpos 1728 */
  void (*S_StartSound) (/* unknown */); /* bitsize 32, bitpos 1760 */
  void (*S_StartLocalSound) (/* unknown */); /* bitsize 32, bitpos 1792 */
  void (*S_StopSound) (/* unknown */); /* bitsize 32, bitpos 1824 */
  void (*S_ClearLoopingSounds) (/* unknown */); /* bitsize 32, bitpos 1856 */
  void (*S_AddLoopingSound) (/* unknown */); /* bitsize 32, bitpos 1888 */
  void (*S_Respatialize) (/* unknown */); /* bitsize 32, bitpos 1920 */
  void (*S_BeginRegistration) (/* unknown */); /* bitsize 32, bitpos 1952 */
  sfxHandle_t (*S_RegisterSound) (/* unknown */); /* bitsize 32, bitpos 1984 */
  void (*S_EndRegistration) (/* unknown */); /* bitsize 32, bitpos 2016 */
  void (*S_UpdateEntity) (/* unknown */); /* bitsize 32, bitpos 2048 */
  void (*S_SetReverb) (/* unknown */); /* bitsize 32, bitpos 2080 */
  void (*S_SetGlobalAmbientVolumeLevel) (/* unknown */); /* bitsize 32, bitpos 2112 */
  float (*S_GetSoundTime) (/* unknown */); /* bitsize 32, bitpos 2144 */
  int (*S_ChannelNameToNum) (/* unknown */); /* bitsize 32, bitpos 2176 */
  char *(*S_ChannelNumToName) (/* unknown */); /* bitsize 32, bitpos 2208 */
  int (*S_IsSoundPlaying) (/* unknown */); /* bitsize 32, bitpos 2240 */
  void (*MUSIC_NewSoundtrack) (/* unknown */); /* bitsize 32, bitpos 2272 */
  void (*MUSIC_UpdateMood) (/* unknown */); /* bitsize 32, bitpos 2304 */
  void (*MUSIC_UpdateVolume) (/* unknown */); /* bitsize 32, bitpos 2336 */
  float *(*get_camera_offset) (/* unknown */); /* bitsize 32, bitpos 2368 */
  void (*R_ClearScene) (/* unknown */); /* bitsize 32, bitpos 2400 */
  void (*R_RenderScene) (/* unknown */); /* bitsize 32, bitpos 2432 */
  void (*R_LoadWorldMap) (/* unknown */); /* bitsize 32, bitpos 2464 */
  void (*R_PrintBSPFileSizes) (/* unknown */); /* bitsize 32, bitpos 2496 */
  int (*MapVersion) (/* unknown */); /* bitsize 32, bitpos 2528 */
  int (*R_MapVersion) (/* unknown */); /* bitsize 32, bitpos 2560 */
  qhandle_t (*R_RegisterModel) ( char *name );
  qhandle_t (*R_SpawnEffectModel) (/* unknown */); /* bitsize 32, bitpos 2624 */
  qhandle_t (*R_RegisterServerModel) (/* unknown */); /* bitsize 32, bitpos 2656 */
  void (*R_UnregisterServerModel) (/* unknown */); /* bitsize 32, bitpos 2688 */
  qhandle_t (*R_RegisterSkin) (/* unknown */); /* bitsize 32, bitpos 2720 */
  qhandle_t (*R_RegisterShader) (/* unknown */); /* bitsize 32, bitpos 2752 */
  qhandle_t (*R_RegisterShaderNoMip) (/* unknown */); /* bitsize 32, bitpos 2784 */
  void (*R_AddRefEntityToScene) ( refEntity_t *r );
  void (*R_AddRefSpriteToScene) (/* unknown */); /* bitsize 32, bitpos 2848 */
  void (*R_AddLightToScene) (/* unknown */); /* bitsize 32, bitpos 2880 */
  qboolean (*R_AddPolyToScene) (/* unknown */); /* bitsize 32, bitpos 2912 */
  void (*R_AddTerrainMarkToScene) (/* unknown */); /* bitsize 32, bitpos 2944 */
  void (*R_SetColor) ( vec_t *rgba );
  void (*R_DrawStretchPic) (/* unknown */); /* bitsize 32, bitpos 3008 */
  fontheader_t *(*R_LoadFont) ( char *name );
  void (*R_DrawString) (fontheader_t *font, char *text, float x, float y, int maxlen, qboolean bVirtualScreen );
  refEntity_t *(*R_GetRenderEntity) (/* unknown */); /* bitsize 32, bitpos 3104 */
  void (*R_ModelBounds) (/* unknown */); /* bitsize 32, bitpos 3136 */
  float (*R_ModelRadius) (/* unknown */); /* bitsize 32, bitpos 3168 */
  float (*R_Noise) (/* unknown */); /* bitsize 32, bitpos 3200 */
  void (*R_DebugLine) (vec_t *start, vec_t *end, float r, float g, float b, float alpha );
  baseshader_t *(*GetShader) (/* unknown */); /* bitsize 32, bitpos 3264 */
  void (*R_SwipeBegin) (/* unknown */); /* bitsize 32, bitpos 3296 */
  void (*R_SwipePoint) (/* unknown */); /* bitsize 32, bitpos 3328 */
  void (*R_SwipeEnd) (/* unknown */); /* bitsize 32, bitpos 3360 */
  int (*R_GetShaderWidth) (/* unknown */); /* bitsize 32, bitpos 3392 */
  int (*R_GetShaderHeight) (/* unknown */); /* bitsize 32, bitpos 3424 */
  void (*R_DrawBox) (/* unknown */); /* bitsize 32, bitpos 3456 */
  void (*GetGameState) ( gameState_t *gs );
  int (*GetSnapshot) ( int snapshotNumber, snapshot_t *snap );
  int (*GetServerStartTime) (/* unknown */); /* bitsize 32, bitpos 3552 */
  void (*SetTime) (/* unknown */); /* bitsize 32, bitpos 3584 */
  void (*GetCurrentSnapshotNumber) (/* unknown */); /* bitsize 32, bitpos 3616 */
  void (*GetGlconfig) (/* unknown */); /* bitsize 32, bitpos 3648 */
  qboolean (*GetParseEntityState) (/* unknown */); /* bitsize 32, bitpos 3680 */
  int (*GetCurrentCmdNumber) (/* unknown */); /* bitsize 32, bitpos 3712 */
  qboolean (*GetUserCmd) (/* unknown */); /* bitsize 32, bitpos 3744 */
  qboolean (*GetServerCommand) (/* unknown */); /* bitsize 32, bitpos 3776 */
  qboolean (*Alias_Add) (/* unknown */); /* bitsize 32, bitpos 3808 */
  qboolean (*Alias_ListAdd) (/* unknown */); /* bitsize 32, bitpos 3840 */
  char *(*Alias_FindRandom) (/* unknown */); /* bitsize 32, bitpos 3872 */
  char *(*Alias_ListFindRandom) (/* unknown */); /* bitsize 32, bitpos 3904 */
  void (*Alias_Dump) (/* unknown */); /* bitsize 32, bitpos 3936 */
  void (*Alias_Clear) (/* unknown */); /* bitsize 32, bitpos 3968 */
  struct AliasList_s /* id 114 */ *(*AliasList_New) (/* unknown */); /* bitsize 32, bitpos 4000 */
  void (*Alias_ListFindRandomRange) (/* unknown */); /* bitsize 32, bitpos 4032 */
  struct AliasList_s /* id 114 */ *(*Alias_GetGlobalList) (/* unknown */); /* bitsize 32, bitpos 4064 */
  void (*UI_ShowMenu) (/* unknown */); /* bitsize 32, bitpos 4096 */
  void (*UI_HideMenu) (/* unknown */); /* bitsize 32, bitpos 4128 */
  int (*UI_FontStringWidth) (/* unknown */); /* bitsize 32, bitpos 4160 */
  int (*Key_StringToKeynum) (/* unknown */); /* bitsize 32, bitpos 4192 */
  char *(*Key_KeynumToBindString) (/* unknown */); /* bitsize 32, bitpos 4224 */
  void (*Key_GetKeysForCommand) (/* unknown */); /* bitsize 32, bitpos 4256 */

  // TIKI stuff
  struct dtiki_s *(*R_Model_GetHandle) ( qhandle_t handle );
  int (*TIKI_NumAnims) ( dtiki_t *pmdl );
  void (*TIKI_CalculateBounds) ( dtiki_t *pmdl, float scale, float *mins, float *maxs );
  char *(*TIKI_Name) ( dtiki_t *pmdl );
  skeletor_c *(*TIKI_GetSkeletor) ( dtiki_t *tiki, int entnum );
  void (*TIKI_SetEyeTargetPos) ( dtiki_t *tiki, int entnum, float *pos );
  char *(*Anim_NameForNum) ( dtiki_t *pmdl, int animnum );
  int (*Anim_NumForName) ( dtiki_t *pmdl, char *name );
  int (*Anim_Random) ( dtiki_t *pmdl, char *name );
  int (*Anim_NumFrames) ( dtiki_t *pmdl, int animnum );
  float (*Anim_Time) ( dtiki_t *pmdl, int animnum );
  float (*Anim_Frametime) ( dtiki_t *pmdl, int animnum );
  // WARNING: Anim_Delta might be NULL pointer in MOHAA
  void (*Anim_Delta) ( dtiki_t *pmdl, int animnum, float *delta );
  int (*Anim_Flags) ( dtiki_t *pmdl, int animnum );
  int (*Anim_FlagsSkel) ( dtiki_t *pmdl, int animnum );
  float (*Anim_CrossblendTime) ( dtiki_t *pmdl, int animnum );
  qboolean (*Anim_HasCommands) ( dtiki_t *pmdl, int animnum );
  qboolean (*Frame_Commands) ( dtiki_t *pmdl, int animnum, int framenum, tiki_cmd_t *tiki_cmd );
  qboolean (*Frame_CommandsTime) ( dtiki_t *pmdl, int animnum, float start, float end, tiki_cmd_t *tiki_cmd );
  int (*Surface_NameToNum) ( dtiki_t *pmdl, char *name );
  int (*Tag_NumForName) ( dtiki_t *pmdl, char *name );
  char *(*Tag_NameForNum) ( dtiki_t *pmdl, int iTagNum );
  void (*ForceUpdatePose) ( refEntity_t *model );
  orientation_t (*TIKI_Orientation) ( refEntity_t *model, int tagnum );
  qboolean (*TIKI_IsOnGround) ( refEntity_t *model, int tagnum, float threshold );

  void (*UI_ShowScoreBoard) ();
  void (*UI_HideScoreBoard) ();
  void (*UI_SetScoreBoardItem) ();
  void (*UI_DeleteScoreBoardItems) ();
  void (*UI_ToggleDMMessageConsole) ();

  struct dtiki_s *(*TIKI_FindTiki) ( const char *path );
  // end of TIKI stuff

  void (*LoadResource) (/* unknown */); /* bitsize 32, bitpos 5280 */
  void (*FS_CanonicalFilename) (/* unknown */); /* bitsize 32, bitpos 5312 */
  cvar_t *fs_debug; /* bitsize 32, bitpos 5344 */
  hdelement_t *HudDrawElements; /* bitsize 32, bitpos 5376 */
  clientAnim_t *anim; /* bitsize 32, bitpos 5408 */
  stopwatch_t *stopwatch; /* bitsize 32, bitpos 5440 */
} clientGameImport_t;

typedef struct clientGameExport_s { /* size 120 */
  void (*CG_Init) ( clientGameImport_t *imported, int serverMessageNum, int serverCommandSequence, int clientNum );
  void (*CG_Shutdown) ( );
  void (*CG_DrawActiveFrame) ( int serverTime, int frametime, stereoFrame_t stereoView, qboolean demoPlayback );
  qboolean (*CG_ConsoleCommand) ( );
  void (*CG_GetRendererConfig) (/* unknown */); /* bitsize 32, bitpos 128 */
  void (*CG_Draw2D) ( );
  void (*CG_EyePosition) (vec3_t *o_vPos); /* bitsize 32, bitpos 192 */
  void (*CG_EyeOffset) (vec3_t *o_vOfs); /* bitsize 32, bitpos 224 */
  void (*CG_EyeAngles) (vec3_t *o_vAngles); /* bitsize 32, bitpos 256 */
  float (*CG_SensitivityScale) (/* unknown */); /* bitsize 32, bitpos 288 */
  void (*CG_ParseCGMessage) (/* unknown */); /* bitsize 32, bitpos 320 */
  void (*CG_RefreshHudDrawElements) (/* unknown */); /* bitsize 32, bitpos 352 */
  void (*CG_HudDrawShader) (/* unknown */); /* bitsize 32, bitpos 384 */
  void (*CG_HudDrawFont) (/* unknown */); /* bitsize 32, bitpos 416 */
  int (*CG_GetParent) (/* unknown */); /* bitsize 32, bitpos 448 */
  float (*CG_GetObjectiveAlpha) (/* unknown */); /* bitsize 32, bitpos 480 */
  int (*CG_PermanentMark) (/* unknown */); /* bitsize 32, bitpos 512 */
  int (*CG_PermanentTreadMarkDecal) (/* unknown */); /* bitsize 32, bitpos 544 */
  int (*CG_PermanentUpdateTreadMark) (/* unknown */); /* bitsize 32, bitpos 576 */
  void (*CG_ProcessInitCommands) (dtiki_t *tiki, refEntity_t *ent); /* bitsize 32, bitpos 608 */
  void (*CG_EndTiki) (dtiki_t *tiki); /* bitsize 32, bitpos 640 */
  char *(*CG_GetColumnName) (/* unknown */); /* bitsize 32, bitpos 672 */
  void (*CG_GetScoreBoardColor) (/* unknown */); /* bitsize 32, bitpos 704 */
  void (*CG_GetScoreBoardFontColor) (/* unknown */); /* bitsize 32, bitpos 736 */
  int (*CG_GetScoreBoardDrawHeader) (/* unknown */); /* bitsize 32, bitpos 768 */
  void (*CG_GetScoreBoardPosition) (/* unknown */); /* bitsize 32, bitpos 800 */
  int (*CG_WeaponCommandButtonBits) (/* unknown */); /* bitsize 32, bitpos 832 */
  int (*CG_CheckCaptureKey) (/* unknown */); /* bitsize 32, bitpos 864 */
  struct prof_cgame_s /* id 116 */ *prof_struct; /* bitsize 32, bitpos 896 */
  qboolean (*CG_Command_ProcessFile) (/* unknown */); /* bitsize 32, bitpos 928 */
} clientGameExport_t;

// cg_consolecmds.c
qboolean CG_ConsoleCommand( void );

// cg_draw.c
extern fontheader_t *facfont;

void CG_MakeCross( vec_t *point );
void CG_DrawAxisAtPoint(vec3_t p, vec3_t axis[3]);
void CG_Draw2D();
void CG_DrawActiveFrame ( int serverTime, int frametime, stereoFrame_t stereoView, qboolean demoPlayback );
qhandle_t R_RegisterModel ( char *name );
void R_AddRefEntityToScene(refEntity_t *ent);
int GetSnapshot ( int snapshotNumber, snapshot_t *snap );

// cg_main.c
extern clientGameExport_t cge;
extern clientGameExport_t cge_out;
extern clientGameImport_t cgi;
extern clientGameImport_t cgi_out;
extern snapshot_t	*snapshot;

// cg_tiki.c
struct dtiki_s *R_Model_GetHandle ( qhandle_t handle );
struct dtiki_s *TIKI_FindTiki ( char *path );
int TIKI_NumAnims( dtiki_t *pmdl );
void TIKI_CalculateBounds( dtiki_t *pmdl, float scale, float *mins, float *maxs );
char *TIKI_Name( dtiki_t *pmdl );
skeletor_c *TIKI_GetSkeletor( dtiki_t *tiki, int entnum );
void TIKI_SetEyeTargetPos( dtiki_t *tiki, int entnum, float *pos );
char *Anim_NameForNum( dtiki_t *pmdl, int animnum );
int Anim_NumForName( dtiki_t *pmdl, char *name );
int Anim_Random( dtiki_t *pmdl, char *name );
int Anim_NumFrames( dtiki_t *pmdl, int animnum );
float Anim_Time( dtiki_t *pmdl, int animnum );
float Anim_Frametime( dtiki_t *pmdl, int animnum );
// WARNING: Anim_Delta might be NULL pointer in MOHAA
void Anim_Delta( dtiki_t *pmdl, int animnum, float *delta );
int Anim_Flags( dtiki_t *pmdl, int animnum );
int Anim_FlagsSkel( dtiki_t *pmdl, int animnum );
float Anim_CrossblendTime( dtiki_t *pmdl, int animnum );
qboolean Anim_HasCommands( dtiki_t *pmdl, int animnum );
qboolean Frame_Commands( dtiki_t *pmdl, int animnum, int framenum, tiki_cmd_t *tiki_cmd );
qboolean Frame_CommandsTime( dtiki_t *pmdl, int animnum, float start, float end, tiki_cmd_t *tiki_cmd );
int Surface_NameToNum( dtiki_t *pmdl, char *name );
int Tag_NumForName( dtiki_t *pmdl, char *name );
char *Tag_NameForNum( dtiki_t *pmdl, int iTagNum );
void ForceUpdatePose( refEntity_t *model );
orientation_t TIKI_Orientation( refEntity_t *model, int tagnum );
qboolean TIKI_IsOnGround( refEntity_t *model, int tagnum, float threshold );

// cg_skeletor.c
void CG_InitSkeletorCvarsAndCmds();


// this is only here so the functions in q_shared.c and bg_*.c can link
void	Com_Error( int level, const char *error, ... );
void	Com_Printf( const char *msg, ... );
void	Com_DPrintf( const char *msg, ... );

void            Matrix4x4Multiply(const matrix_t a, const matrix_t b, matrix_t out);
void            MatrixSetupTransformFromVectorsFLU(matrix_t m, const vec3_t forward, const vec3_t left, const vec3_t up, const vec3_t origin);
void            MatrixToVectorsFLU(const matrix_t m, vec3_t forward, vec3_t left, vec3_t up);



