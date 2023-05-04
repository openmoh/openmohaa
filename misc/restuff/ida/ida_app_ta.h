#pragma once

#include "ida_app.h"
#include "ida_common_ta.h"

typedef struct clSnapshot_s
{
  qboolean valid;
  int snapFlags;
  int serverTime;
  int serverTimeResidual;
  int messageNum;
  int deltaNum;
  int ping;
  unsigned char areamask[32];
  int cmdNum;
  playerState_t ps;
  int numEntities;
  int parseEntitiesNum;
  int serverCommandNum;
  int number_of_sounds;
  server_sound_t sounds[64];

} clSnapshot_t;

typedef struct clientActive_s
{
  int timeoutcount;
  clSnapshot_t snap;
  int serverTime;
  int oldServerTime;
  int serverStartTime;
  int oldFrameServerTime;
  int serverTimeDelta;
  qboolean extrapolatedSnapshot;
  qboolean newSnapshots;
  gameState_t gameState;
  char mapname[64];
  int parseEntitiesNum;
  int mouseDx[2];
  int mouseDy[2];
  int mouseIndex;
  int mousex;
  int mousey;
  unsigned int mouseButtons;
  int joystickAxis[6];
  usercmd_t cmds[128];
  int cmdNumber;
  outPacket_t outPackets[32];
  vec3_t viewangles;
  int serverId;
  clSnapshot_t snapshots[32];
  entityState_t entityBaselines[1024];
  entityState_t parseEntities[2048];

} clientActive_t;

typedef struct clientConnection_s
{
	int clientNum;
	int lastPacketSentTime;
	int lastPacketTime;
	netAdr_t serverAddress;
    int connectTime;
    int connectStartTime;
    int connectPacketCount;
    char serverMessage[1024];
    int challenge;
    int checksumFeed;
    int reliableSequence;
    int reliableAcknowledge;
    char reliableCommands[512][2048];
    int serverMessageSequence;
    char serverCommands[512][2048];
    int serverCommandSequence;
    int download;
    char downloadTempName[256];
    char downloadName[256];
    int downloadNumber;
    int downloadBlock;
    int downloadCount;
    int downloadSize;
    char downloadList[1350];
    int downloadRestart;
    char demoName[64];
    int demorecording;
    int demoplaying;
    int demowaiting;
    int firstDemoFrameSkipped;
    int demofile;
    int timeDemoFrames;
    int timeDemoStart;
    int timeDemoBaseTime;

	netChan_t netChan;

} clientConnection_t;

typedef struct clientSnapshot_s
{
  int areabytes;
  unsigned char areabits[32];
  playerState_t ps;
  int num_entities;
  int first_entity;
  int messageSent;
  int messageAcked;
  int messageSize;

} clientSnapshot_t;


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
	clientSnapshot_t frames[PACKET_BACKUP];
	int ping;
	int rate;
	int snapshotMsec;
	netChan_t netchan;
    int unknown1_0;
    int unknown1_1;
    int netprofile;
    int unknown2_0[417];
	server_sound_t sounds[64];
	int numberOfSounds;
	qboolean locprint;
	int locprintX;
	int locprintY;
	char stringToPrint[256];
    int radarInfo;
    int lastRadarTime[128];
    int gcdNum;
    int unknown3_0[15];
    int unknown4_0;
    qboolean isPure;

} client_t;

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
	stopWatch_t stopwatch;
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

typedef struct server_s
{
	serverState_t state;
	qboolean restarting;
	int serverId;
	int restartedServerId;
	int checksumFeed;
	int snapshotCounter;
	int timeResidual;
	int nextFrameTime;
	float frameTime;
	cStaticModel_t *models[1024];
	char *configstrings[2736];
	svEntity_t svEntities[1024];
	int farplane;
	qboolean skyportal;
	char *entityParsePoint;
	gentity_t *gentities;
	int gentitySize;
	int num_entities;
	playerState_t *gameClients;
	int gameClientSize;

} server_t;

typedef struct qr_implementation_s {
	int querysock;
	int hbsock;
	char gamename[64];
	char secret_key[128];
	qr_querycallback_t qr_basic_callback;
	qr_querycallback_t qr_info_callback;
	qr_querycallback_t qr_rules_callback;
	qr_querycallback_t qr_players_callback;
	long unsigned int lastheartbeat;
	int queryid;
	int packetnumber;
	int qport;
	char no_query;
	int unk1;
	int unk2;
	int unk3;
	int unk4;
	int unk5;
	void* udata;
} qr_implementation_t, *qr_t;
