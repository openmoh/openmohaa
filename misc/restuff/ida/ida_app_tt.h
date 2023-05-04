#pragma once

#include "ida_app.h"
#include "ida_common_tt.h"

typedef struct clientActive_s
{
  int timeoutcount;
  clSnapshot_sh_t snap;
  int serverTime;
  int oldServerTime;
  int serverStartTime;
  int oldFrameServerTime;
  int serverTimeDelta;
  qboolean extrapolatedSnapshot;
  qboolean newSnapshots;
  gameState_sh_t gameState;
  char mapname[64];
  int parseEntitiesNum;
  int mouseDx[2];
  int mouseDy[2];
  int mouseIndex;
  int mousex;
  int mousey;
  unsigned int mouseButtons;
  int joystickAxis[6];
  userCmd_t cmds[128];
  int cmdNumber;
  outPacket_t outPackets[32];
  vec3_t viewangles;
  int serverId;
  clSnapshot_sh_t snapshots[32];
  entityState_t entityBaselines[1024];
  entityState_t parseEntities[2048];

} clientActive_t;

typedef struct client_s
{
	clientState_t state;
	char userinfo[MAX_INFOSTRING];
	int reliableSequence;
	int reliableAcknowledge;
	char reliableCommands[MAX_RELIABLECOMMANDS][MAX_STRING_CHARS];
	int reliableSent;
	int messageAcknowledge;
	int gamestateMessageNum;
	int challenge;
	int unknown1;
	struct usercmd_s lastUsercmd;
	struct userEyes_s lastEyeinfo;
	int lastMessageNum;
	int lastClientCommand;
	char  lastClientCommandString[MAX_STRING_CHARS];
	struct gentity_s *gentity;
	char name[MAX_NAMELENGTH];
	char downloadName[MAX_QPATH];
	fileHandle_t download;
	int downloadSize;
	int downloadCount;
	int downloadClientBlock;
	int downloadCurrentBlock;
	int downloadXmitBlock;
	unsigned char *downloadBlocks[MAX_DOWNLOADWINDOW];
	int downloadBlockSize[MAX_DOWNLOADWINDOW];
	qboolean downloadEOF;
	int downloadSendTime;
	int deltaMessage;
	int nextReliableTime;
	int lastPacketTime;
	int lastConnectTime;
	int nextSnapshotTime;
	qboolean rateDelayed;
	int timeoutCount;
	clientSnapshot_sh_t frames[PACKET_BACKUP];
	int ping;
	int rate;
	int snapshotMsec;
	netChan_t netchan;
    int unknown2;
    int unknown3;
    int netprofile;
    int unknown4[417];
	server_sound_t sounds[64];
	int numberOfSounds;
	qboolean locprint;
	int locprintX;
	int locprintY;
	char stringToPrint[256];
    int radarInfo;
    int lastRadarTime[128];
    int gcdNum;
    qboolean isPure;

} client_t;

typedef struct {
	netAdr_t	adr;
	char	  	hostName[ 32 ];
	char	  	mapName[ 32 ];
	char	  	game[ 32 ];
	char		gameTypeString[ 32 ];
	int			netType;
	int			gameType;
	int		  	clients;
	int		  	maxClients;
	int			minPing;
	int			maxPing;
	int			ping;
	qboolean	visible;
    qboolean    unknown0_0;
} serverInfo_t;

typedef struct clientStatic_s {
	int startStage;
	connstate_t	state;
	int loading;
	int keyCatchers;
	qboolean vid_restart;
	qboolean cddialog;
	qboolean no_menus;
	char servername[256];
	qboolean rendererRegistered;
	qboolean cgameStarted;
	qboolean uiStarted;
	qboolean timeScaled;
	int framecount;
	int frametime;
    float serverFrameTime;
	int realtime;
	int realFrametime;
	int numlocalservers;
	serverInfo_t localServers[128];
	qboolean bNewLocalServerInfo;
	int numglobalservers;
	serverInfo_t  globalServers[2048];
	int numGlobalServerAddresses;
	serverAddress_t globalServerAddresses[2048];
	int numfavoriteservers;
	serverInfo_t favoriteServers[128];
	int nummplayerservers;
	serverInfo_t mplayerServers[128];
	int pingUpdateSource;
	int masterNum;
	netAdr_t updateServer;
	char updateChallenge[2048];
	char updateInfoString[1350];
	netAdr_t authorizeServer;
    int unknown1_0[1024];
	glconfig_t glconfig;
	int total_tris;
	int total_verts;
	int total_texels;
	int world_tris;
	int world_verts;
	int character_lights;
    int unknown2_0;
	hdelement_t HudDrawElements[256];
	clientAnim_t anim;
	stopWatch_sh_t stopwatch;
    void* savedCgameState;
    int savedCgameStateSize;
    char gcdResponse[73];
    int unknown3_0;
    int currentRate;
} clientStatic_t;

typedef struct serverStatic_s
{
	qboolean initialized;
	int snapFlagServerBit;
	int time;
	int startTime;
	int lastTime;
	int serverLagTime;
	qboolean autosave;
	int mapTime;
	client_t *clients;
	int iNumClients;
	int numSnapshotEntities;
	int nextSnapshotEntities;
	entityState_t *snapshotEntities;
	int nextHeartbeatTime;
	challenge_t challenges[1024];
	netAdr_t redirectAddress;
	netAdr_t authorizeAddress;
	char gameName[64];
	char mapName[64];
	char rawServerName[64];
	int areaBitsWarningTime;
	qboolean soundsNeedLoad;
	char tmFileName[64];
	int tmMoopcount;
	int tmOffset;
	sgSoundSystem_t soundSystem;

} serverStatic_t;
