#pragma once

#include "ida_common.h"

typedef struct
{
	int			blocNode;
	int			blocPtrs;
	node_t*		tree;
	node_t*		lhead;
	node_t*		ltail;
	node_t*		loc[HMAX+1];
	node_t**	freelist;
	node_t		nodeList[1024];
	node_t*		nodePtrs[1024];

} huff_t;

typedef struct
{
	huff_t		compressor;
	huff_t		decompressor;

} huffman_t;

typedef struct glconfig_s
{
	char rendererString[MAX_STRING_CHARS];
	char vendorString[MAX_STRING_CHARS];
	char versionString[MAX_STRING_CHARS];
	char unknownString1[MAX_STRING_CHARS];
	char unknownString2[MAX_STRING_CHARS];
	char extensionsString[MAX_EXTSTRINGCHARS];
	int maxTextureSize;
	int maxActiveTextures;
	int colorBits;
	int depthBits;
	int stencilBits;
	glDriverType_t driverType;
	glHardwareType_t hardwareType;
	qboolean deviceSupportsGamma;
	int textureCompression;
	qboolean textureEnvAddAvailable;
	int vidWidth;
	int vidHeight;
	float windowAspect;
	int displayFrequency;
	qboolean isFullscreen;
	qboolean stereoEnabled;
	qboolean smpActive;
	int registerCombinerAvailable;
	qboolean secondaryColorAvailable;
	qboolean var;
	qboolean fence;
	int unk1;

} glconfig_t;

typedef struct {
	char *path;
	TikiScript tikiFile;
	dloadanim_t *loadanims[ 4095 ];
	dloadinitcmd_t *loadserverinitcmds[ 160 ];
	dloadinitcmd_t *loadclientinitcmds[ 180 ];
	int skelIndex_ld[ 12 ];
	int numanims;
	int numserverinitcmds;
	int numclientinitcmds;
	char headmodels[ 4096 ];
	char headskins[ 4096 ];
	qboolean bIsCharacter;
	struct msg_s *modelBuf;
	unsigned char modelData[ 8192 ];
	qboolean bInIncludesSection;
	char idleSkel[128];
	int numskels;
	qboolean hasSkel;
} dloaddef_t;

typedef struct refDef_s
{
	int x;
	int y;
	int width;
	int height;
	float fovX;
	float fovY;
	vec3_t viewOrg;
	vec3_t viewAxis[3];
	int time;
	int rdFlags;
	byte areaMask[MAX_MAPAREABYTES];
	float farplaneDistance;
	int unk1;
	float farplaneColor[3];
	qboolean farplaneCull;
	qboolean skyPortal;
	int unk2;
	int unk3;
	int unk4;
	int unk5;
	int unk6;
	int unk7;
	float skyAlpha;
	vec3_t skyOrigin;
	vec3_t skyAxis[3];

} refDef_t;

typedef struct playerState_s
{
	int commandTime;
	pmType_t pmType;
	int bobCycle;
	int pmFlags;
	vec3_t origin;
	vec3_t velocity;
	int gravity;
	int speed;
	int deltaAngles[3];
	int groundEntityNum;
	qboolean walking;
	qboolean groundPlane;
	int feetFalling;
	vec3_t fallDir;
	trace_t groundTrace;
	int clientNum;
	vec3_t viewAngles;
	int viewHeight;
	float leanAngle;
	int viewModelAnim;
	int viewModelAnimChanged;
	playerStat_e stats[MAX_PLAYERSTATS];
	int activeItems[8];
	int ammoNameIndex[MAX_WEAPONS];
	int ammoAmount[MAX_WEAPONS];
	int maxAmmoAmount[MAX_WEAPONS];
	musicMood_t currentMusicMood;
	musicMood_t fallbackMusicMood;
	float musicVolume;
	float musicVolumeFadeTime;
	int reverbType;
	float reverbLevel;
	vec4_t blend;
	float fov;
	vec3_t cameraOrigin;
	vec3_t cameraAngles;
	float cameraTime;
	vec3_t cameraOffset;
	vec3_t cameraPosOfs;
	int cameraFlags;
	vec3_t damageAngles;
	int radarInfo;
	qboolean voted;
	int ping;
	vec3_t eyePos;

} playerState_t;

typedef struct usercmd_s
{
	int serverTime;
	short unsigned int buttons;
	short int angles[3];
	uint8_t msec;
	signed char forwardmove;
	signed char rightmove;
	signed char upmove;

} userCmd_t;

typedef struct snapshot_s
{
	int snapFlags;
	int ping;
	int serverTime;
	byte areaMask[MAX_MAPAREABYTES];
	playerState_t ps;
	int numEntities;
	entityState_t entities[MAX_ENTITIESINSNAPSHOT];
	int numServerCommands;
	int serverCommandSequence;
	int numberOfSounds;
	serverSound_t sounds[MAX_SERVERSOUNDS];

} snapshot_t;

typedef struct pmove_s
{
  playerState_t *ps;
  userCmd_t cmd;
  int tracemask;
  qboolean noFootsteps;
  qboolean canLean;
  int framecount;
  int numtouch;
  int touchents[32];
  int moveresult;
  qboolean stepped;
  int pmoveEvent;
  float mins[3];
  float maxs[3];
  int watertype;
  int waterlevel;
  int pmove_fixed;
  int pmove_msec;
  void (*trace) ();
  int (*pointcontents) ();

} pmove_t;

typedef struct gameState_s
{
	int stringOffsets[MAX_CONFIGSTRINGS];
	char stringData[MAX_GAMESTATECHARS_SH];
	int dataCount;

} gameState_t;

typedef struct stopWatch_s
{
	int startTime;
	int endTime;
    int unknown1_0;

} stopWatch_t;
