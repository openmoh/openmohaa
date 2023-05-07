/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// cl_cgame.c  -- client system interaction with client game

#include "client.h"
#include "cl_ui.h"
#include "tiki.h"
#include <localization.h>

extern qboolean loadCamera(const char *name);
extern void startCamera(int time);
extern qboolean getCameraInfo(int time, vec3_t *origin, vec3_t *angles);

/*
====================
CL_GetGameState
====================
*/
void CL_GetGameState( gameState_t *gs ) {
	*gs = cl.gameState;
}

/*
====================
CL_GetGlconfig
====================
*/
void CL_GetGlconfig( glconfig_t *glconfig ) {
	strncpy( glconfig->renderer_string, cls.glconfig.renderer_string, sizeof( glconfig->renderer_string ) );
	strncpy( glconfig->vendor_string, cls.glconfig.vendor_string, sizeof( glconfig->vendor_string ) );
	strncpy( glconfig->version_string, cls.glconfig.version_string, sizeof( glconfig->version_string ) );
	strncpy( glconfig->extensions_string, cls.glconfig.extensions_string, sizeof( glconfig->extensions_string ) );
	glconfig->maxTextureSize = cls.glconfig.maxTextureSize;
	glconfig->colorBits = cls.glconfig.colorBits;
	glconfig->depthBits = cls.glconfig.depthBits;
	glconfig->stencilBits = cls.glconfig.stencilBits;
	glconfig->driverType = cls.glconfig.driverType;
	glconfig->hardwareType = cls.glconfig.hardwareType;
	glconfig->deviceSupportsGamma = cls.glconfig.deviceSupportsGamma;
	glconfig->textureCompression = cls.glconfig.textureCompression;
	glconfig->textureEnvAddAvailable = cls.glconfig.textureEnvAddAvailable;
	glconfig->vidWidth = cls.glconfig.vidWidth;
	glconfig->vidHeight = cls.glconfig.vidHeight;
	glconfig->windowAspect = cls.glconfig.windowAspect;
	glconfig->displayFrequency = cls.glconfig.displayFrequency;
	glconfig->isFullscreen = cls.glconfig.isFullscreen;
	glconfig->stereoEnabled = cls.glconfig.stereoEnabled;
	glconfig->smpActive = cls.glconfig.smpActive;
	glconfig->registerCombinerAvailable = cls.glconfig.registerCombinerAvailable;
	glconfig->secondaryColorAvailable = cls.glconfig.secondaryColorAvailable;
	glconfig->VAR = cls.glconfig.VAR;
	glconfig->fence = cls.glconfig.fence;
}


/*
====================
CL_GetUserCmd
====================
*/
qboolean CL_GetUserCmd( int cmdNumber, usercmd_t *ucmd ) {
	// cmds[cmdNumber] is the last properly generated command

	// can't return anything that we haven't created yet
	if ( cmdNumber > cl.cmdNumber ) {
		Com_Error( ERR_DROP, "CL_GetUserCmd: %i >= %i", cmdNumber, cl.cmdNumber );
	}

	// the usercmd has been overwritten in the wrapping
	// buffer because it is too far out of date
	if ( cmdNumber <= cl.cmdNumber - CMD_BACKUP ) {
		return qfalse;
	}

	*ucmd = cl.cmds[ cmdNumber & CMD_MASK ];

	return qtrue;
}

int CL_GetCurrentCmdNumber( void ) {
	return cl.cmdNumber;
}


/*
====================
CL_GetParseEntityState
====================
*/
qboolean	CL_GetParseEntityState( int parseEntityNumber, entityState_t *state ) {
	// can't return anything that hasn't been parsed yet
	if ( parseEntityNumber >= cl.parseEntitiesNum ) {
		Com_Error( ERR_DROP, "CL_GetParseEntityState: %i >= %i",
			parseEntityNumber, cl.parseEntitiesNum );
	}

	// can't return anything that has been overwritten in the circular buffer
	if ( parseEntityNumber <= cl.parseEntitiesNum - MAX_PARSE_ENTITIES ) {
		return qfalse;
	}

	*state = cl.parseEntities[ parseEntityNumber & ( MAX_PARSE_ENTITIES - 1 ) ];
	return qtrue;
}

/*
====================
CL_GetCurrentSnapshotNumber
====================
*/
void	CL_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime ) {
	*snapshotNumber = cl.snap.messageNum;
	*serverTime = cl.snap.serverTime;
}

/*
====================
CL_GetServerStartTime
====================
*/
int CL_GetServerStartTime( void ) {
	return cl.serverStartTime;
}

/*
====================
CL_SetTime
====================
*/
void CL_SetTime( int time ) {
	cl.serverTime = time;
}

/*
====================
CL_GetSnapshot
====================
*/
qboolean	CL_GetSnapshot( int snapshotNumber, snapshot_t *snapshot ) {
	clSnapshot_t	*clSnap;
	int				i, count;

	if ( snapshotNumber > cl.snap.messageNum ) {
		Com_Error( ERR_DROP, "CL_GetSnapshot: snapshotNumber > cl.snapshot.messageNum" );
	}

	// if the frame has fallen out of the circular buffer, we can't return it
	if ( cl.snap.messageNum - snapshotNumber >= PACKET_BACKUP ) {
		return qfalse;
	}

	// if the frame is not valid, we can't return it
	clSnap = &cl.snapshots[snapshotNumber & PACKET_MASK];
	if ( !clSnap->valid ) {
		return qfalse;
	}

	// if the entities in the frame have fallen out of their
	// circular buffer, we can't return it
	if ( cl.parseEntitiesNum - clSnap->parseEntitiesNum >= MAX_PARSE_ENTITIES ) {
		return qfalse;
	}

	// write the snapshot
	snapshot->snapFlags = clSnap->snapFlags;
	snapshot->serverCommandSequence = clSnap->serverCommandNum;
	snapshot->ping = clSnap->ping;
	snapshot->serverTime = clSnap->serverTime;
	Com_Memcpy( snapshot->areamask, clSnap->areamask, sizeof( snapshot->areamask ) );
	snapshot->ps = clSnap->ps;
	count = clSnap->numEntities;
	if ( count > MAX_ENTITIES_IN_SNAPSHOT ) {
		Com_DPrintf( "CL_GetSnapshot: truncated %i visible entities to %i\n", count, MAX_ENTITIES_IN_SNAPSHOT );
		count = MAX_ENTITIES_IN_SNAPSHOT;
	}
	snapshot->numEntities = count;
	for ( i = 0 ; i < count ; i++ ) {
		snapshot->entities[i] =
			cl.parseEntities[ ( clSnap->parseEntitiesNum + i ) & (MAX_PARSE_ENTITIES-1) ];
	}

	// wombat: sounds
	count = clSnap->number_of_sounds;
	if ( snapshot->number_of_sounds > MAX_SERVER_SOUNDS) {
		Com_DPrintf( "CL_GetSnapshot: truncated %i sounds to %i\n", count, MAX_SERVER_SOUNDS);
		count = MAX_SERVER_SOUNDS;
	}
	snapshot->number_of_sounds = count;
	Com_Memcpy( snapshot->sounds, clSnap->sounds, sizeof(snapshot->sounds[0])*count );

	// FIXME: configstring changes and server commands!!!

	return qtrue;
}

/*
=====================
CL_SetUserCmdValue
=====================
*/
void CL_SetUserCmdValue( int userCmdValue, float sensitivityScale ) {
	cl.cgameUserCmdValue = userCmdValue;
	cl.cgameSensitivity = sensitivityScale;
}

/*
=====================
CL_AddCgameCommand
=====================
*/
void CL_AddCgameCommand( const char *cmdName ) {
	Cmd_AddCommand( cmdName, NULL );
}

/*
=====================
CL_CgameError
=====================
*/
void CL_CgameError( const char *string ) {
	Com_Error( ERR_DROP, "%s", string );
}


/*
=====================
CL_ConfigstringModified
=====================
*/
void CL_ConfigstringModified( void ) {
	char		*old, *s;
	int			i, index;
	char		*dup;
	gameState_t	oldGs;
	int			len;

	index = atoi( Cmd_Argv(1) );
	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		Com_Error( ERR_DROP, "configstring > MAX_CONFIGSTRINGS" );
	}
	// get everything after "cs <num>"
	s = Cmd_ArgsFrom(2);

	old = cl.gameState.stringData + cl.gameState.stringOffsets[ index ];
	if ( !strcmp( old, s ) ) {
		return;		// unchanged
	}

	// build the new gameState_t
	oldGs = cl.gameState;

	Com_Memset( &cl.gameState, 0, sizeof( cl.gameState ) );

	// leave the first 0 for uninitialized strings
	cl.gameState.dataCount = 1;

	for ( i = 0 ; i < MAX_CONFIGSTRINGS ; i++ ) {
		if ( i == index ) {
			dup = s;
		} else {
			dup = oldGs.stringData + oldGs.stringOffsets[ i ];
		}
		if ( !dup[0] ) {
			continue;		// leave with the default empty string
		}

		len = strlen( dup );

		if ( len + 1 + cl.gameState.dataCount > MAX_GAMESTATE_CHARS ) {
			Com_Error( ERR_DROP, "MAX_GAMESTATE_CHARS exceeded" );
		}

		// append it to the gameState string buffer
		cl.gameState.stringOffsets[ i ] = cl.gameState.dataCount;
		Com_Memcpy( cl.gameState.stringData + cl.gameState.dataCount, dup, len + 1 );
		cl.gameState.dataCount += len + 1;
	}

	if ( index == CS_SYSTEMINFO ) {
		// parse serverId and other cvars
		CL_SystemInfoChanged();
	}

}


/*
===================
CL_GetServerCommand

Set up argc/argv for the given command
===================
*/
qboolean CL_GetServerCommand( int serverCommandNumber, qboolean differentServer ) {
	char	*s;
	char	*cmd;
	static char bigConfigString[BIG_INFO_STRING];
	int argc;

	// if we have irretrievably lost a reliable command, drop the connection
	if ( serverCommandNumber <= clc.serverCommandSequence - MAX_RELIABLE_COMMANDS ) {
		// when a demo record was started after the client got a whole bunch of
		// reliable commands then the client never got those first reliable commands
		if ( clc.demoplaying )
			return qfalse;
		Com_Error( ERR_DROP, "CL_GetServerCommand: a reliable command was cycled out" );
		return qfalse;
	}

	if ( serverCommandNumber > clc.serverCommandSequence ) {
		Com_Error( ERR_DROP, "CL_GetServerCommand: requested a command not received" );
		return qfalse;
	}

	s = clc.serverCommands[ serverCommandNumber & ( MAX_RELIABLE_COMMANDS - 1 ) ];
	clc.lastExecutedServerCommand = serverCommandNumber;

	if( developer->integer == 2 ) {
		Com_DPrintf( "serverCommand: %i : %s\n", serverCommandNumber, s );
	}

rescan:
	Cmd_TokenizeString( s );
	cmd = Cmd_Argv(0);
	argc = Cmd_Argc();

	if ( !strcmp( cmd, "disconnect" ) ) {
		// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=552
		// allow server to indicate why they were disconnected
		if ( argc >= 2 )
			Com_Error( ERR_SERVERDISCONNECT, "Server disconnected - %s", Cmd_Argv( 1 ) );
		else
			Com_Error( ERR_SERVERDISCONNECT, "Server disconnected\n" );
	}

	if ( !strcmp( cmd, "cs" ) ) {
		CL_ConfigstringModified();
		// reparse the string, because CL_ConfigstringModified may have done another Cmd_TokenizeString()
		Cmd_TokenizeString( s );
		return qtrue;
	}

	// we may want to put a "connect to other server" command here

	// cgame can now act on the command
	return qtrue;
}


/*
====================
CL_CM_LoadMap

Just adds default parameters that cgame doesn't need to know about
====================
*/
void CL_CM_LoadMap( const char *mapname ) {
	int		checksum;

	CM_LoadMap( mapname, qtrue, &checksum );
}

/*
====================
CL_CG_Malloc
====================
*/
void *CL_CG_Malloc(int size )
{
	return Z_TagMalloc( size, TAG_CGAME );
}

/*
====================
CL_CG_Free
====================
*/
void CL_CG_Free( void *ptr )
{
	Z_Free( ptr );
}

/*
====================
CL_R_AddPolyToScene
====================
*/
void CL_R_AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts )
{
	re.AddPolyToScene( hShader, numVerts, verts, 1 );
}

/*
====================
CL_GetShaderPointer
====================
*/
baseshader_t *CL_GetShaderPointer( int iShaderNum )
{
	return CM_ShaderPointer( iShaderNum );
}

/*
====================
CL_ShutdownCGame

====================
*/
void CL_ShutdownCGame( void ) {
	Key_SetCatcher( Key_GetCatcher( ) & ~KEYCATCH_CGAME );
	cls.cgameStarted = qfalse;

	if( cge )
	{
		cge->CG_Shutdown();
		cge = NULL;
	}

	Sys_UnloadCGame();

	if( re.FreeModels ) {
		re.FreeModels();
	}

	Z_FreeTags( TAG_CGAME );
}

static int	FloatAsInt( float f ) {
	int		temp;

	*(float *)&temp = f;

	return temp;
}

/*
====================
CL_RegisterSound
====================
*/
sfxHandle_t CL_RegisterSound( const char *sample, qboolean streamed ) {
	return S_RegisterSound( sample, streamed, qfalse );
}

/*
====================
CL_StartLocalSound
====================
*/
void CL_StartLocalSound(const char* soundName, qboolean forceLoad) {
	S_StartLocalSoundByName(soundName, qfalse );
}

/*
====================
CL_InitCGameDLL
====================
*/
void CL_InitCGameDLL( clientGameImport_t *cgi, clientGameExport_t **cge ) {

	*cge = ( clientGameExport_t * )Sys_GetCGameAPI( cgi );
	if( !*cge ) {
		Com_Error( ERR_DROP, "CL_InitCGameImport failed" );
	}

	memset( cgi, 0, sizeof( clientGameImport_t ) );

	cgi->Printf							= Com_Printf;
	cgi->DPrintf						= Com_DPrintf;

	cgi->Malloc							= CL_CG_Malloc;
	cgi->Free							= CL_CG_Free;

	cgi->Error							= Com_Error;
	cgi->Milliseconds					= Sys_Milliseconds;

	cgi->LV_ConvertString				= Sys_LV_CL_ConvertString;

	cgi->Cvar_Get						= Cvar_Get;
	cgi->Cvar_Set						= Cvar_Set;

	cgi->Argc							= Cmd_Argc;
	cgi->Args							= Cmd_Args;
	cgi->Argv							= Cmd_Argv;

	cgi->AddCommand						= CL_AddCgameCommand;
	cgi->Cmd_Stuff						= Cbuf_AddText;
	cgi->Cmd_Execute					= Cbuf_ExecuteText;
	cgi->Cmd_TokenizeString				= Cmd_TokenizeString;

	cgi->FS_ReadFile					= FS_ReadFileEx;
	cgi->FS_FreeFile					= FS_FreeFile;
	cgi->FS_WriteFile					= FS_WriteFile;
	cgi->FS_WriteTextFile				= FS_WriteTextFile;

	cgi->SendConsoleCommand				= Cbuf_AddText;

	cgi->MSG_ReadBits					= CL_MSG_ReadBits;
	cgi->MSG_ReadChar					= CL_MSG_ReadChar;
	cgi->MSG_ReadByte					= CL_MSG_ReadByte;
	cgi->MSG_ReadSVC					= CL_MSG_ReadSVC;
	cgi->MSG_ReadShort					= CL_MSG_ReadShort;
	cgi->MSG_ReadLong					= CL_MSG_ReadLong;
	cgi->MSG_ReadFloat					= CL_MSG_ReadFloat;
	cgi->MSG_ReadString					= CL_MSG_ReadString;
	cgi->MSG_ReadStringLine				= CL_MSG_ReadStringLine;
	cgi->MSG_ReadAngle8					= CL_MSG_ReadAngle8;
	cgi->MSG_ReadAngle16				= CL_MSG_ReadAngle16;
	cgi->MSG_ReadData					= CL_MSG_ReadData;
	cgi->MSG_ReadCoord					= CL_MSG_ReadCoord;
	cgi->MSG_ReadDir					= CL_MSG_ReadDir;

	cgi->SendClientCommand				= CL_AddReliableCommand;

	cgi->CM_LoadMap						= CL_CM_LoadMap;
	cgi->CM_NumInlineModels				= CM_NumInlineModels;
	cgi->CM_PointContents				= CM_PointContents;
	cgi->CM_TransformedPointContents	= CM_TransformedPointContents;
	cgi->CM_BoxTrace					= CM_BoxTrace;
	cgi->CM_TransformedBoxTrace			= CM_TransformedBoxTrace;
	cgi->CM_TempBoxModel				= CM_TempBoxModel;
	cgi->CM_InlineModel					= CM_InlineModel;
	cgi->CM_PrintBSPFileSizes			= CM_PrintBSPFileSizes;
	cgi->CM_PointLeafnum				= CM_PointLeafnum;
	cgi->CM_LeafInPVS					= CM_LeafInPVS;

	cgi->R_MarkFragments				= re.MarkFragments;
	cgi->R_MarkFragmentsForInlineModel	= re.MarkFragmentsForInlineModel;
	cgi->R_GetInlineModelBounds			= re.GetInlineModelBounds;
	cgi->R_GetLightingForDecal			= re.GetLightingForDecal;
	cgi->R_GetLightingForSmoke			= re.GetLightingForSmoke;
	cgi->R_GatherLightSources			= re.R_GatherLightSources;

	cgi->S_StartSound					= S_StartSound;
	cgi->S_StartLocalSound				= CL_StartLocalSound;
	cgi->S_StopSound					= S_StopSound;
	cgi->S_AddLoopingSound				= S_AddLoopingSound;
	cgi->S_ClearLoopingSounds			= S_ClearLoopingSoundsNoParam;
	cgi->S_Respatialize					= S_RespatializeOld;
	cgi->S_BeginRegistration			= S_BeginRegistration;
	cgi->S_EndRegistration				= S_EndRegistration;
	cgi->S_UpdateEntity					= S_UpdateEntity;
	cgi->S_SetReverb					= S_SetReverb;
	cgi->S_SetGlobalAmbientVolumeLevel	= S_SetGlobalAmbientVolumeLevel;
	cgi->S_RegisterSound				= CL_RegisterSound;
	cgi->S_GetSoundTime					= S_GetSoundTime;
	cgi->S_ChannelNameToNum				= S_ChannelNameToNum;
	cgi->S_ChannelNumToName				= S_ChannelNumToName;
	cgi->S_IsSoundPlaying				= S_IsSoundPlaying;

	cgi->MUSIC_NewSoundtrack		= MUSIC_NewSoundtrack;
	cgi->MUSIC_UpdateMood			= MUSIC_UpdateMood;
	cgi->MUSIC_UpdateVolume			= MUSIC_UpdateVolume;

	cgi->get_camera_offset			= get_camera_offset;
	
	cgi->R_ClearScene				= re.ClearScene;
	cgi->R_RenderScene				= re.RenderScene;

	cgi->R_LoadWorldMap				= re.LoadWorld;
	cgi->R_PrintBSPFileSizes		= re.PrintBSPFileSizes;
	cgi->R_MapVersion				= re.MapVersion;
	cgi->R_RegisterModel			= re.RegisterModel;
	cgi->R_SpawnEffectModel			= re.SpawnEffectModel;
	cgi->R_RegisterServerModel		= re.RegisterServerModel;
	cgi->R_UnregisterServerModel	= re.UnregisterServerModel;
	cgi->R_RegisterShader			= re.RegisterShader;
	cgi->R_RegisterShaderNoMip		= re.RegisterShaderNoMip;

	cgi->R_AddRefEntityToScene		= re.AddRefEntityToScene;
	cgi->R_AddRefSpriteToScene		= re.AddRefSpriteToScene;
	
	cgi->R_AddLightToScene			= re.AddLightToScene;
	cgi->R_AddPolyToScene			= re.AddPolyToScene;
	cgi->R_AddTerrainMarkToScene	= re.AddTerrainMarkToScene;
	cgi->R_SetColor					= re.SetColor;
	cgi->R_DrawStretchPic			= re.DrawStretchPic;
	cgi->R_LoadFont					= re.LoadFont;
	cgi->R_DrawString				= re.DrawString;
	cgi->R_ModelBounds				= re.ModelBounds;
	cgi->R_ModelRadius				= re.ModelRadius;
	cgi->R_GetRenderEntity			= re.GetRenderEntity;
	cgi->R_Noise					= re.Noise;
	cgi->R_DebugLine				= re.DebugLine;

	cgi->GetShader					= CL_GetShaderPointer;
	cgi->R_GetShaderWidth			= re.GetShaderWidth;
	cgi->R_GetShaderHeight			= re.GetShaderHeight;

	cgi->R_DrawBox					= re.DrawBox;
	cgi->R_SwipeBegin				= re.SwipeBegin;
	cgi->R_SwipePoint				= re.SwipePoint;
	cgi->R_SwipeEnd					= re.SwipeEnd;

	cgi->GetGameState				= CL_GetGameState;
	cgi->GetSnapshot				= CL_GetSnapshot;
	cgi->GetServerStartTime			= CL_GetServerStartTime;
	cgi->SetTime					= CL_SetTime;
	cgi->GetCurrentSnapshotNumber	= CL_GetCurrentSnapshotNumber;

	cgi->GetGlconfig				= CL_GetGlconfig;

	cgi->GetParseEntityState		= CL_GetParseEntityState;
	cgi->GetCurrentCmdNumber		= CL_GetCurrentCmdNumber;
	cgi->GetUserCmd					= CL_GetUserCmd;
	cgi->GetServerCommand			= CL_GetServerCommand;

	cgi->Alias_Add					= Alias_Add;
	cgi->Alias_ListAdd				= Alias_ListAdd;
	cgi->Alias_FindRandom			= Alias_FindRandom;
	cgi->Alias_ListFindRandom		= Alias_ListFindRandom;
	cgi->Alias_Dump					= Alias_Dump;
	cgi->Alias_Clear				= Alias_Clear;
	cgi->AliasList_New				= AliasList_New;
	cgi->Alias_ListFindRandomRange	= Alias_ListFindRandomRange;
	cgi->Alias_GetGlobalList		= Alias_GetGlobalList;

	cgi->UI_ShowMenu				= UI_ShowMenu;
	cgi->UI_HideMenu				= UI_HideMenu;
	cgi->UI_FontStringWidth			= uie.FontStringWidth;
	cgi->Key_StringToKeynum			= Key_StringToKeynum;
	cgi->Key_KeynumToBindString		= Key_KeynumToBindString;
	cgi->Key_GetKeysForCommand		= Key_GetKeysForCommand;

	cgi->R_Model_GetHandle			= re.R_Model_GetHandle;

	cgi->TIKI_NumAnims				= TIKI_NumAnims;
	cgi->TIKI_CalculateBounds		= TIKI_CalculateBounds;
	cgi->TIKI_Name					= TIKI_Name;
	cgi->TIKI_GetSkeletor			= TIKI_GetSkeletor;
	cgi->TIKI_SetEyeTargetPos		= TIKI_SetEyeTargetPos;

	cgi->Anim_NameForNum			= TIKI_Anim_NameForNum;
	cgi->Anim_NumForName			= TIKI_Anim_NumForName;
	cgi->Anim_Random				= TIKI_Anim_Random;
	cgi->Anim_NumFrames				= TIKI_Anim_NumFrames;
	cgi->Anim_Time					= TIKI_Anim_Time;
	cgi->Anim_Frametime				= TIKI_Anim_Frametime;
	cgi->Anim_Flags					= TIKI_Anim_Flags;
	cgi->Anim_FlagsSkel				= TIKI_Anim_FlagsSkel;
	cgi->Anim_CrossblendTime		= TIKI_Anim_CrossblendTime;
	cgi->Anim_HasCommands			= TIKI_Anim_HasClientCommands;
	cgi->Frame_Commands				= TIKI_Frame_Commands_Client;
	cgi->Frame_CommandsTime			= TIKI_Frame_Commands_ClientTime;

	cgi->Surface_NameToNum			= TIKI_Surface_NameToNum;
	cgi->Tag_NumForName				= TIKI_Tag_NameToNum;
	cgi->Tag_NameForNum				= TIKI_Tag_NumToName;

	cgi->ForceUpdatePose			= re.ForceUpdatePose;
	cgi->TIKI_Orientation			= re.TIKI_Orientation;
	cgi->TIKI_IsOnGround			= re.TIKI_IsOnGround;

	cgi->UI_ShowScoreBoard			= UI_ShowScoreboard_f;
	cgi->UI_HideScoreBoard			= UI_HideScoreboard_f;
	cgi->UI_SetScoreBoardItem		= UI_SetScoreBoardItem;
	cgi->UI_DeleteScoreBoardItems	= UI_DeleteScoreBoardItems;
	cgi->UI_ToggleDMMessageConsole	= UI_ToggleDMConsole;

	cgi->TIKI_FindTiki				= TIKI_FindTiki;

	cgi->LoadResource				= UI_LoadResource;
	cgi->FS_CanonicalFilename		= FS_CanonicalFilename;

	cgi->fsDebug					= fs_debug;
	cgi->HudDrawElements			= cls.HudDrawElements;
	cgi->anim						= &cls.anim;
	cgi->stopWatch					= &cls.stopwatch;
	// FIXME
	//cgi->pUnknownVar				= NULL;

	cls.cgameStarted = qtrue;
}

/*
====================
CL_ViewModelAnimation_Init
====================
*/
static void CL_ViewModelAnimation_Init() {
	int i;

	for( i = 0; i < MAX_FRAMEINFOS; i++ ) {
		cls.anim.g_VMFrameInfo[ i ].index = 0;
		cls.anim.g_VMFrameInfo[ i ].time = 0.0;
		cls.anim.g_VMFrameInfo[ i ].weight = 0.0;
	}

	VectorClear( cls.anim.g_vCurrentVMPosOffset );
	cls.anim.g_iLastVMAnim = -1;
	cls.anim.g_iLastVMAnimChanged = -1;
	cls.anim.g_iCurrentVMAnimSlot = 0;
	cls.anim.g_iCurrentVMDuration = 9999;
	cls.anim.g_bCrossblending = qfalse;
	cls.anim.g_iLastEquippedWeaponStat = -1;
	cls.anim.g_szLastActiveItem[ 0 ] = 0;
	cls.anim.g_iLastAnimPrefixIndex = 0;
}

/*
====================
CL_InitializeHudDrawElements
====================
*/
static void CL_InitializeHudDrawElements() {
	int i;

	for( i = 0; i < MAX_HUDDRAW_ELEMENTS; i++ ) {
		cls.HudDrawElements[ i ].shaderName[ 0 ] = 0;
		VectorSet4( cls.HudDrawElements[ i ].vColor, 1, 1, 1, 0 );
		cls.HudDrawElements[ i ].hShader = 0;
		cls.HudDrawElements[ i ].iX = 0;
		cls.HudDrawElements[ i ].iY = 0;
		cls.HudDrawElements[ i ].iWidth = 0;
		cls.HudDrawElements[ i ].iHeight = 0;
		cls.HudDrawElements[ i ].iHorizontalAlign = 1;
		cls.HudDrawElements[ i ].iVerticalAlign = 1;
		cls.HudDrawElements[ i ].bVirtualScreen = qfalse;
		cls.HudDrawElements[ i ].string[ 0 ] = 0;
		cls.HudDrawElements[ i ].fontName[ 0 ] = 0;
		cls.HudDrawElements[ i ].pFont = NULL;
	}
}

/*
====================
CL_InitializeStopwatch
====================
*/
static void CL_InitializeStopwatch() {
	cls.stopwatch.iStartTime = 0;
	cls.stopwatch.iEndTime = 0;
}

/*
====================
CL_InitClientSavedData
====================
*/
void CL_InitClientSavedData( void ) {
	CL_ViewModelAnimation_Init();
	CL_InitializeHudDrawElements();
	CL_InitializeStopwatch();
}

/*
====================
CL_InitCGame

Should only be called by CL_StartHunkUsers
====================
*/
void CL_InitCGame( void ) {
	const char			*info;
	const char			*mapname;
	int					t1, t2;
	clientGameImport_t	cgi;

	t1 = Sys_Milliseconds();

	// find the current mapname
	info = cl.gameState.stringData + cl.gameState.stringOffsets[ CS_SERVERINFO ];
	mapname = Info_ValueForKey( info, "mapname" );
	Com_sprintf( cl.mapname, sizeof( cl.mapname ), "maps/%s.bsp", mapname );

	S_BeginRegistration();
	CL_ShutdownCGame();
	CL_InitCGameDLL( &cgi, &cge );
	cls.state = CA_LOADING;

	if( !com_sv_running->integer && !cls.vid_restart )
	{
		TIKI_End();
		TIKI_Begin();
		TIKI_FreeAll();
		CL_InitClientSavedData();
	}

	// init for this gamestate
	// use the lastExecutedServerCommand instead of the serverCommandSequence
	// otherwise server commands sent just before a gamestate are dropped
	cge->CG_Init( &cgi, clc.serverMessageSequence, clc.lastExecutedServerCommand, clc.clientNum );

	ClearNewConfigFlag();
	TIKI_FinishLoad();

	// reset any CVAR_CHEAT cvars registered by cgame
	if( !clc.demoplaying && !cl_connectedToCheatServer )
		Cvar_SetCheatState();

	// we will send a usercmd this frame, which
	// will cause the server to send us the first snapshot
	cls.state = CA_PRIMED;

	CL_EndRegistration();

	t2 = Sys_Milliseconds();

	Com_Printf( "CL_InitCGame: %5.2f seconds\n", ( t2 - t1 ) / 1000.0 );
}


/*
====================
CL_GameCommand

See if the current console command is claimed by the cgame
====================
*/
qboolean CL_GameCommand( void ) {
	if ( !cge ) {
		return qfalse;
	}

	return cge->CG_ConsoleCommand();
}



/*
=====================
CL_CGameRendering
=====================
*/
void CL_CGameRendering( stereoFrame_t stereo ) {
	if( cl.oldServerTime < cl.serverStartTime ) {
		cl.serverTime = cl.serverStartTime;
		cl.oldServerTime = cl.serverStartTime;
	}

	cge->CG_DrawActiveFrame( cl.serverTime, cl.serverTime - cl.oldServerTime, stereo, clc.demoplaying );

	cl.oldServerTime = cl.serverTime;
}

/*
=====================
CL_CGame2D
=====================
*/
void CL_CGame2D( stereoFrame_t stereo ) {
	cge->CG_Draw2D();
}


/*
=================
CL_AdjustTimeDelta

Adjust the clients view of server time.

We attempt to have cl.serverTime exactly equal the server's view
of time plus the timeNudge, but with variable latencies over
the internet it will often need to drift a bit to match conditions.

Our ideal time would be to have the adjusted time approach, but not pass,
the very latest snapshot.

Adjustments are only made when a new snapshot arrives with a rational
latency, which keeps the adjustment process framerate independent and
prevents massive overadjustment during times of significant packet loss
or bursted delayed packets.
=================
*/

#define	RESET_TIME	500

void CL_AdjustTimeDelta( void ) {
	int		resetTime;
	int		newDelta;
	int		deltaDelta;

	cl.newSnapshots = qfalse;

	// the delta never drifts when replaying a demo
	if ( clc.demoplaying ) {
		return;
	}

	// if the current time is WAY off, just correct to the current value
	if ( com_sv_running->integer ) {
		resetTime = 100;
	} else {
		resetTime = RESET_TIME;
	}

	newDelta = cl.snap.serverTime - cls.realtime;
	deltaDelta = abs( newDelta - cl.serverTimeDelta );

	if ( deltaDelta > RESET_TIME ) {
		cl.serverTimeDelta = newDelta;
		cl.oldServerTime = cl.snap.serverTime;	// FIXME: is this a problem for cgame?
		cl.serverTime = cl.snap.serverTime;
		if ( cl_showTimeDelta->integer ) {
			Com_Printf( "<RESET> " );
		}
	} else if ( deltaDelta > 100 ) {
		// fast adjust, cut the difference in half
		if ( cl_showTimeDelta->integer ) {
			Com_Printf( "<FAST> " );
		}
		cl.serverTimeDelta = ( cl.serverTimeDelta + newDelta ) >> 1;
	} else {
		// slow drift adjust, only move 1 or 2 msec

		// if any of the frames between this and the previous snapshot
		// had to be extrapolated, nudge our sense of time back a little
		// the granularity of +1 / -2 is too high for timescale modified frametimes
		if ( !cls.timeScaled ) {
			if ( cl.extrapolatedSnapshot ) {
				cl.extrapolatedSnapshot = qfalse;
				cl.serverTimeDelta -= 2;
			} else {
				// otherwise, move our sense of time forward to minimize total latency
				cl.serverTimeDelta++;
			}
		}
	}

	if ( cl_showTimeDelta->integer ) {
		Com_Printf( "%i ", cl.serverTimeDelta );
	}
}


/*
==================
CL_FirstSnapshot
==================
*/
void CL_FirstSnapshot( void ) {
	// ignore snapshots that don't have entities
	if ( cl.snap.snapFlags & SNAPFLAG_NOT_ACTIVE ) {
		return;
	}
	cls.state = CA_ACTIVE;

	// notify the UI
	UI_ServerLoaded();

	// set the timedelta so we are exactly on this first frame
	cl.serverStartTime = cl.snap.serverTime;
	cl.serverTimeDelta = cl.snap.serverTime - cls.realtime;

	clc.timeDemoBaseTime = cl.snap.serverTime;

	// if this is the first frame of active play,
	// execute the contents of activeAction now
	// this is to allow scripting a timedemo to start right
	// after loading
	if ( cl_activeAction->string[0] ) {
		Cbuf_AddText( cl_activeAction->string );
		Cvar_Set( "activeAction", "" );
	}
}

static int lastSnapFlags;
/*
==================
CL_UpdateSnapFlags
==================
*/
void CL_UpdateSnapFlags( void ) {
	lastSnapFlags = cl.snap.snapFlags;
}

/*
==================
CL_SetCGameTime
==================
*/
void CL_SetCGameTime( void ) {
	// getting a valid frame message ends the connection process
	if ( cls.state != CA_ACTIVE ) {
		if ( cls.state != CA_PRIMED ) {
			return;
		}
		if ( clc.demoplaying ) {
			// we shouldn't get the first snapshot on the same frame
			// as the gamestate, because it causes a bad time skip
			if ( !clc.firstDemoFrameSkipped ) {
				clc.firstDemoFrameSkipped = qtrue;
				return;
			}
			CL_ReadDemoMessage();
		}
		if ( cl.newSnapshots ) {
			cl.newSnapshots = qfalse;
			CL_FirstSnapshot();
		}
		if ( cls.state != CA_ACTIVE ) {
			return;
		}
	}

	// if we have gotten to this point, cl.snap is guaranteed to be valid
	if ( !cl.snap.valid ) {
		assert( 0 );
		Com_Error( ERR_DROP, "CL_SetCGameTime: !cl.snap.valid" );
	}

	// allow pause in single player
	if ( paused->integer && com_sv_running->integer ) {
		// paused
		CL_AdjustTimeDelta();
		return;
	}

	// FIXME
	//if( ( cl.snap.snapFlags ^ lastSnapFlags ) & SNAPFLAG_SERVERCOUNT ) {
	//	CL_ServerRestarted();
	//} else {

		if( cl.snap.serverTime < cl.oldFrameServerTime ) {
			assert( 0 );
			Com_Error( ERR_DROP, "cl.snap.serverTime < cl.oldFrameServerTime" );
		}
	//}

	cl.oldFrameServerTime = cl.snap.serverTime;


	// get our current view of time

	if ( clc.demoplaying && cl_freezeDemo->integer ) {
		// cl_freezeDemo is used to lock a demo in place for single frame advances

	} else {
		// cl_timeNudge is a user adjustable cvar that allows more
		// or less latency to be added in the interest of better
		// smoothness or better responsiveness.
		int tn;

		tn = cl_timeNudge->integer;
		if (tn<-30) {
			tn = -30;
		} else if (tn>30) {
			tn = 30;
		}

		cl.serverTime = cls.realtime + cl.serverTimeDelta - tn;

		// guarantee that time will never flow backwards, even if
		// serverTimeDelta made an adjustment or cl_timeNudge was changed
		if ( cl.serverTime < cl.oldServerTime ) {
			cl.serverTime = cl.oldServerTime;
		}

		// note if we are almost past the latest frame (without timeNudge),
		// so we will try and adjust back a bit when the next snapshot arrives
		if ( cls.realtime + cl.serverTimeDelta >= cl.snap.serverTime - 5 ) {
			cl.extrapolatedSnapshot = qtrue;
		}
	}

	// if we have gotten new snapshots, drift serverTimeDelta
	// don't do this every frame, or a period of packet loss would
	// make a huge adjustment
	if ( cl.newSnapshots ) {
		CL_AdjustTimeDelta();
	}

	if ( !clc.demoplaying ) {
		return;
	}

	// if we are playing a demo back, we can just keep reading
	// messages from the demo file until the cgame definately
	// has valid snapshots to interpolate between

	// a timedemo will always use a deterministic set of time samples
	// no matter what speed machine it is run on,
	// while a normal demo may have different time samples
	// each time it is played back
	if ( cl_timedemo->integer ) {
		int now = Sys_Milliseconds( );
		int frameDuration;

		if (!clc.timeDemoStart) {
			clc.timeDemoStart = clc.timeDemoLastFrame = now;
			clc.timeDemoMinDuration = INT_MAX;
			clc.timeDemoMaxDuration = 0;
		}

		frameDuration = now - clc.timeDemoLastFrame;
		clc.timeDemoLastFrame = now;

		// Ignore the first measurement as it'll always be 0
		if( clc.timeDemoFrames > 0 )
		{
			if( frameDuration > clc.timeDemoMaxDuration )
				clc.timeDemoMaxDuration = frameDuration;

			if( frameDuration < clc.timeDemoMinDuration )
				clc.timeDemoMinDuration = frameDuration;

			// 255 ms = about 4fps
			if( frameDuration > UCHAR_MAX )
				frameDuration = UCHAR_MAX;

			clc.timeDemoDurations[ ( clc.timeDemoFrames - 1 ) %
				MAX_TIMEDEMO_DURATIONS ] = frameDuration;
		}

		clc.timeDemoFrames++;
		cl.serverTime = clc.timeDemoBaseTime + clc.timeDemoFrames * 50;
	}

	while ( cl.serverTime >= cl.snap.serverTime ) {
		// feed another messag, which should change
		// the contents of cl.snap
		CL_ReadDemoMessage();
		if ( cls.state != CA_ACTIVE ) {
			return;		// end of demo
		}
	}

}



