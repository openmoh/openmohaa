#include "q_shared.h"
#include "server.h"
#include "sv_gqueryreporting.h"

#include <gcdkey/gcdkeys.h>

static char gamemode[128];
static qboolean gcdInitialized = qfalse;

#if TARGET_GAME_TYPE == 1
static const char SECRET_GS_KEY[] = {'h', '2', 'P', '1', 'c', '9'};
static const unsigned int GCD_GAME_ID = 641;
#elif TARGET_GAME_TYPE == 2
static const char SECRET_GS_KEY[] = { 'y', '3', '2', 'F', 'D', 'c' };
static const unsigned int GCD_GAME_ID = 802;
#else
static const char SECRET_GS_KEY[] = { 'M', '5', 'F', 'd', 'w', 'c' };
static const unsigned int GCD_GAME_ID = 0;
#endif

void qr_send_statechanged(qr_t qrec);
void qr_shutdown(qr_t qrec);
void qr_process_queries(qr_t qrec);

int qr_init(
	qr_t* qrec,
	char* ip,
	int baseport,
	char* gamename,
	char* secret_key,
	qr_querycallback_t qr_basic_callback,
	qr_querycallback_t qr_info_callback,
	qr_querycallback_t qr_rules_callback,
	qr_querycallback_t qr_players_callback,
	void* userdata
);

static const char* ConvertMapFilename(const char* mapname)
{
	static char converted[1024];

	const char* name = strstr(mapname, "/");
	if (!name) {
		return mapname;
	}

	strcpy(converted, name + 1);
	return converted;
}

static void basic_callback(char* outbuf, int maxlen, void* userdata)
{
	Info_SetValueForKey(outbuf, "gamename", TARGET_GAME_NAME);
	Info_SetValueForKey(outbuf, "gamever", TARGET_GAME_VERSION);
	Info_SetValueForKey(outbuf, "location", va("%i", sv_location->integer));

	if (sv_debug_gamespy->integer) {
		Com_DPrintf("Basic callback, sent: %s\n\n", outbuf);
	}
}

static void info_callback(char* outbuf, int maxlen, void* userdata)
{
	char infostring[1024];
	qboolean allowlean = qfalse;

	infostring[0] = 0;
	Info_SetValueForKey(infostring, "hostname", sv_hostname->string);
	Info_SetValueForKey(infostring, "hostport", Cvar_Get("net_port", "12203", 32)->string);
	Info_SetValueForKey(infostring, "mapname", ConvertMapFilename(svs.mapName));
	Info_SetValueForKey(infostring, "gametype", g_gametypestring->string);
	Info_SetValueForKey(infostring, "numplayers", va("%i", SV_NumClients()));
	Info_SetValueForKey(infostring, "maxplayers", va("%i", svs.iNumClients - sv_privateClients->integer));
	Info_SetValueForKey(infostring, "gamemode", gamemode);
	Info_SetValueForKey(infostring, "gametype_i", va("%i", g_gametype->integer));
	Info_SetValueForKey(infostring, "dedicated", Cvar_Get("ui_dedicated", "0", 0)->string);
	Info_SetValueForKey(infostring, "sprinton", Cvar_Get("sv_sprinton", "1", 0)->string);
	Info_SetValueForKey(infostring, "realism", Cvar_Get("g_realismmode", "0", 0)->string);
	Info_SetValueForKey(infostring, "pure", va("%i", sv_pure->integer));
	if ((Cvar_VariableIntegerValue("dmflags") & 0x40000) != 0) {
		allowlean = 1;
	}

	Info_SetValueForKey(infostring, "allowlean", va("%i", allowlean));
	if (strlen(infostring) < maxlen) {
		strcpy(outbuf, infostring);
	}

	if (sv_debug_gamespy->integer) {
		Com_DPrintf("Info callback, sent: %s\n\n", outbuf);
	}
}

static void rules_callback(char* outbuf, int maxlen, void* userdata)
{
	char infostring[1024];

	infostring[0] = 0;

	Info_SetValueForKey(infostring, "timelimit", Cvar_VariableString("timelimit"));
	Info_SetValueForKey(infostring, "fraglimit", Cvar_VariableString("fraglimit"));
	Info_SetValueForKey(infostring, "rankedserver", Cvar_VariableString("g_rankedserver"));

	if (strlen(infostring) < maxlen) {
		strcpy(outbuf, infostring);
	}

	if (sv_debug_gamespy->integer) {
		Com_DPrintf("Rules callback, sent: %s\n\n", outbuf);
	}
}

static void players_callback(char* outbuf, int maxlen, void* userdata)
{
	int i;
	char infostring[128];
	size_t infolen;
	size_t currlen = 0;

	for (i = 0; i < svs.iNumClients; i++)
	{
		playerState_t* ps = SV_GameClientNum(i);

		Com_sprintf(
			infostring,
			128,
			"\\player_%d\\%s\\frags_%d\\%d\\deaths_%d\\%d\\ping_%d\\%d",
			i,
			svs.clients[i].name,
			i,
			ps->stats[STAT_KILLS],
			i,
			ps->stats[STAT_DEATHS],
			i,
			svs.clients[i].rate
		);

		infolen = strlen(infostring);
		if (currlen + infolen < maxlen)
		{
			strcat(outbuf, infostring);
			currlen += infolen;
		}
	}
}

void SV_GamespyHeartbeat()
{
	if (!g_gametype->integer || !sv_gamespy->integer) {
		return;
	}

	if (sv_debug_gamespy->integer) {
		Com_DPrintf("GameSpy Heartbeat\n");
	}

	qr_send_statechanged(NULL);
}

void SV_ProcessGamespyQueries()
{
	if (!g_gametype->integer || !sv_gamespy->integer) {
		return;
	}

	qr_process_queries(NULL);
	gcd_think();
}

void SV_ShutdownGamespy()
{
	if (!g_gametype->integer || !sv_gamespy->integer) {
		return;
	}

	strcpy(gamemode, "exiting");

	if (gcdInitialized)
	{
		gcd_shutdown();
		gcdInitialized = 0;
	}

	qr_send_statechanged(NULL);
	qr_shutdown(NULL);
}

qboolean SV_InitGamespy()
{
	cvar_t* net_ip;
	cvar_t* net_gamespy_port;
	char secret_key[9];

	sv_debug_gamespy = Cvar_Get("sv_debuggamespy", "0", 0);
	sv_location = Cvar_Get("sv_location", "1", CVAR_ARCHIVE);
	sv_gamespy = Cvar_Get("sv_gamespy", "1", CVAR_LATCH);

	if (!sv_gamespy->integer || !g_gametype->integer) {
		return qfalse;
	}

	strcpy(gamemode, "openplaying");
	strcpy(secret_key, SECRET_GS_KEY);

	net_ip = Cvar_Get("net_ip", "localhost", CVAR_LATCH);
	net_gamespy_port = Cvar_Get("net_gamespy_port", "12300", CVAR_LATCH);

	if (qr_init(
		NULL,
		net_ip->string,
		net_gamespy_port->integer,
		TARGET_GAME_NAME,
		secret_key,
		basic_callback,
		info_callback,
		rules_callback,
		players_callback,
		NULL))
	{
		Com_DPrintf("Error starting query sockets in SV_GamespyInit\n");
		return qfalse;
	}

	if (!sv_gamespy->integer)
	{
		strcpy(gamemode, "exiting");
		qr_send_statechanged(NULL);
	}

	if (!gcdInitialized)
	{
#if GCD_GAME_ID
		gcd_init(GCD_GAME_ID);
#endif
		gcdInitialized = 1;
	}

	return qtrue;
}

