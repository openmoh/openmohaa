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

// DESCRIPTION:
// Init functions for the cgame

#include "cg_local.h"
#include "cg_parsemsg.h"
#include "cg_archive.h"
#include "cg_radar.h"

#ifdef _WIN32
#    include <windows.h>
#endif

clientGameImport_t        cgi;
static clientGameExport_t cge;

cvar_t       *paused;
cvar_t       *developer;
cg_t          cg;
cgs_t         cgs;
target_game_e cg_target_game = TG_INVALID;
int           cg_protocol;
centity_t     cg_entities[MAX_GENTITIES];

cvar_t *cg_animSpeed;
cvar_t *cg_debugAnim;
cvar_t *cg_debugAnimWatch;
cvar_t *cg_errorDecay;
cvar_t *cg_nopredict;
cvar_t *cg_showmiss;
cvar_t *cg_addMarks;
cvar_t *cg_maxMarks;
cvar_t *cg_viewsize;
cvar_t *cg_3rd_person;
cvar_t *cg_drawviewmodel;
cvar_t *cg_synchronousClients;
cvar_t *cg_stereoSeparation;
cvar_t *cg_stats;
cvar_t *cg_lagometer;
cvar_t *r_lerpmodels;
cvar_t *cg_cameraheight;
cvar_t *cg_cameradist;
cvar_t *cg_cameraverticaldisplacement;
cvar_t *cg_camerascale;
cvar_t *cg_shadows;
cvar_t *cg_hidetempmodels;
cvar_t *cg_traceinfo;
cvar_t *cg_debugFootsteps;
cvar_t *cg_smoothClients;
cvar_t *cg_smoothClientsTime;
cvar_t *pmove_fixed;
cvar_t *pmove_msec;
cvar_t *cg_pmove_msec;
cvar_t *dm_playermodel;
cvar_t *dm_playergermanmodel;
cvar_t *cg_forceModel;
cvar_t *cg_animationviewmodel;
cvar_t *cg_hitmessages;
cvar_t *cg_acidtrip;
cvar_t *cg_hud;
cvar_t *cg_huddraw_force;
cvar_t *cg_drawsvlag;
cvar_t *cg_crosshair;
cvar_t *cg_crosshair_friend;
cvar_t *ui_crosshair;
cvar_t *vm_offset_max;
cvar_t *vm_offset_speed;
cvar_t *vm_sway_front;
cvar_t *vm_sway_side;
cvar_t *vm_sway_up;
cvar_t *vm_offset_air_front;
cvar_t *vm_offset_air_side;
cvar_t *vm_offset_air_up;
cvar_t *vm_offset_crouch_front;
cvar_t *vm_offset_crouch_side;
cvar_t *vm_offset_crouch_up;
cvar_t *vm_offset_rocketcrouch_front;
cvar_t *vm_offset_rocketcrouch_side;
cvar_t *vm_offset_rocketcrouch_up;
cvar_t *vm_offset_shotguncrouch_front;
cvar_t *vm_offset_shotguncrouch_side;
cvar_t *vm_offset_shotguncrouch_up;
cvar_t *vm_offset_vel_base;
cvar_t *vm_offset_vel_front;
cvar_t *vm_offset_vel_side;
cvar_t *vm_offset_vel_up;
cvar_t *vm_offset_upvel;
cvar_t *vm_lean_lower;
cvar_t *voiceChat;
cvar_t *cg_shadowscount;
cvar_t *cg_shadowdebug;
cvar_t *ui_timemessage;

//
// Added in OPM
//
cvar_t *cg_fov;
cvar_t *cg_cheats;

/*
=================
CG_RegisterCvars
=================
*/
void CG_RegisterCvars(void)
{
    cvar_t *temp;

    cgi.Cvar_Get("g_subtitle", "0", CVAR_ARCHIVE);
    cg_viewsize                   = cgi.Cvar_Get("viewsize", "100", CVAR_ARCHIVE);
    cg_addMarks                   = cgi.Cvar_Get("cg_marks_add", "1", CVAR_ARCHIVE);
    cg_maxMarks                   = cgi.Cvar_Get("cg_marks_max", "256", CVAR_ARCHIVE | CVAR_LATCH);
    cg_animSpeed                  = cgi.Cvar_Get("cg_animspeed", "1", CVAR_CHEAT);
    cg_debugAnim                  = cgi.Cvar_Get("cg_debuganim", "0", CVAR_CHEAT);
    cg_debugAnimWatch             = cgi.Cvar_Get("cg_debuganimwatch", "0", CVAR_CHEAT);
    cg_errorDecay                 = cgi.Cvar_Get("cg_errordecay", "100", 0);
    cg_nopredict                  = cgi.Cvar_Get("cg_nopredict", "0", 0);
    cg_showmiss                   = cgi.Cvar_Get("cg_showmiss", "0", 0);
    cg_stats                      = cgi.Cvar_Get("cg_stats", "0", 0);
    cg_hidetempmodels             = cgi.Cvar_Get("cg_hidetempmodels", "0", 0);
    cg_synchronousClients         = cgi.Cvar_Get("g_synchronousClients", "0", 0);
    cg_stereoSeparation           = cgi.Cvar_Get("cg_stereosep", "0.4", CVAR_ARCHIVE);
    cg_lagometer                  = cgi.Cvar_Get("cg_lagometer", "0", 0);
    paused                        = cgi.Cvar_Get("paused", "0", 0);
    r_lerpmodels                  = cgi.Cvar_Get("r_lerpmodels", "1", 0);
    cg_3rd_person                 = cgi.Cvar_Get("cg_3rd_person", "0", CVAR_CHEAT);
    cg_drawviewmodel              = cgi.Cvar_Get("cg_drawviewmodel", "2", CVAR_ARCHIVE);
    cg_cameraheight               = cgi.Cvar_Get("cg_cameraheight", "18", CVAR_ARCHIVE);
    cg_cameradist                 = cgi.Cvar_Get("cg_cameradist", "120", CVAR_ARCHIVE);
    cg_cameraverticaldisplacement = cgi.Cvar_Get("cg_cameraverticaldisplacement", "-2", CVAR_ARCHIVE);
    cg_camerascale                = cgi.Cvar_Get("cg_camerascale", "0.3", CVAR_ARCHIVE);
    cg_traceinfo                  = cgi.Cvar_Get("cg_traceinfo", "0", CVAR_ARCHIVE);
    cg_debugFootsteps             = cgi.Cvar_Get("cg_debugfootsteps", "0", CVAR_CHEAT);
    cg_smoothClients              = cgi.Cvar_Get("cg_smoothClients", "1", CVAR_ARCHIVE);
    cg_smoothClientsTime          = cgi.Cvar_Get("cg_smoothClientsTime", "100", CVAR_ARCHIVE);
    pmove_fixed                   = cgi.Cvar_Get("pmove_fixed", "0", 0);
    pmove_msec                    = cgi.Cvar_Get("pmove_msec", "8", 0);
    cg_pmove_msec                 = cgi.Cvar_Get("cg_pmove_msec", "8", 0);
    cg_shadows                    = cgi.Cvar_Get("cg_shadows", "0", CVAR_ARCHIVE);
    cg_shadowscount               = cgi.Cvar_Get("cg_shadowscount", "8", 0);
    cg_shadowdebug                = cgi.Cvar_Get("cg_shadowdebug", "0", 0);
    developer                     = cgi.Cvar_Get("developer", "0", 0);
    dm_playermodel                = cgi.Cvar_Get("dm_playermodel", "american_army", 3);
    dm_playergermanmodel          = cgi.Cvar_Get("dm_playergermanmodel", "german_wehrmacht_soldier", CVAR_ARCHIVE | CVAR_USERINFO);
    cg_forceModel                 = cgi.Cvar_Get("cg_forceModel", "0", CVAR_ARCHIVE);
    cg_animationviewmodel         = cgi.Cvar_Get("cg_animationviewmodel", "0", CVAR_SYSTEMINFO);
    cg_hitmessages                = cgi.Cvar_Get("cg_hitmessages", "1", CVAR_ARCHIVE);
    cg_acidtrip                   = cgi.Cvar_Get("cg_acidtrip", "0", CVAR_CHEAT);
    cg_hud                        = cgi.Cvar_Get("cg_hud", "0", 0);
    cg_huddraw_force              = cgi.Cvar_Get("cg_huddraw_force", "0", CVAR_SAVEGAME);
    cg_drawsvlag                  = cgi.Cvar_Get("cg_drawsvlag", "1", CVAR_ARCHIVE);
    cg_crosshair                  = cgi.Cvar_Get("cg_crosshair", "textures/hud/crosshair", CVAR_ARCHIVE);

    // 1.11 and below will fallback to cg_crosshair if the texture doesn't exist
    // as it doesn't have crosshair_friend texture
    cg_crosshair_friend = cgi.Cvar_Get("cg_crosshair_friend", "textures/hud/crosshair_friend", CVAR_ARCHIVE);
    ui_crosshair                  = cgi.Cvar_Get("ui_crosshair", "1", CVAR_ARCHIVE);
    vm_offset_max                 = cgi.Cvar_Get("vm_offset_max", "8.0", 0);
    vm_offset_speed               = cgi.Cvar_Get("vm_offset_speed", "8.0", 0);
    vm_sway_front                 = cgi.Cvar_Get("vm_sway_front", "0.1", 0);
    vm_sway_side                  = cgi.Cvar_Get("vm_sway_side", "0.005", 0);
    vm_sway_up                    = cgi.Cvar_Get("vm_sway_up", "0.003", 0);
    vm_offset_air_front           = cgi.Cvar_Get("vm_offset_air_front", "-3.0", 0);
    vm_offset_air_side            = cgi.Cvar_Get("vm_offset_air_side", "1.5", 0);
    vm_offset_air_up              = cgi.Cvar_Get("vm_offset_air_up", "-6.0", 0);
    vm_offset_crouch_front        = cgi.Cvar_Get("vm_offset_crouch_front", "-0.5", 0);
    vm_offset_crouch_side         = cgi.Cvar_Get("vm_offset_crouch_side", "2.25", 0);
    vm_offset_crouch_up           = cgi.Cvar_Get("vm_offset_crouch_up", "0.2", 0);
    vm_offset_rocketcrouch_front  = cgi.Cvar_Get("vm_offset_rocketcrouch_front", "0", 0);
    vm_offset_rocketcrouch_side   = cgi.Cvar_Get("vm_offset_rocketcrouch_side", "0", 0);
    vm_offset_rocketcrouch_up     = cgi.Cvar_Get("vm_offset_rocketcrouch_up", "0", 0);
    vm_offset_shotguncrouch_front = cgi.Cvar_Get("vm_offset_shotguncrouch_front", "-1", 0);
    vm_offset_shotguncrouch_side  = cgi.Cvar_Get("vm_offset_shotguncrouch_side", "2.5", 0);
    vm_offset_shotguncrouch_up    = cgi.Cvar_Get("vm_offset_shotguncrouch_up", "-1.1", 0);
    vm_offset_vel_base            = cgi.Cvar_Get("vm_offset_vel_base", "100", 0);
    vm_offset_vel_front           = cgi.Cvar_Get("vm_offset_vel_front", "-2.0", 0);
    vm_offset_vel_side            = cgi.Cvar_Get("vm_offset_vel_side", "1.5", 0);
    vm_offset_vel_up              = cgi.Cvar_Get("vm_offset_vel_up", "-4.0", 0);
    vm_offset_upvel               = cgi.Cvar_Get("vm_offset_upvel", "0.0025", 0);
    vm_lean_lower                 = cgi.Cvar_Get("vm_lean_lower", "0.1", 0);
    voiceChat                     = cgi.Cvar_Get("cg_voicechat", "1", 0);

    ui_timemessage = cgi.Cvar_Get("ui_timemessage", "", 0);

    // see if we are also running the server on this machine
    temp            = cgi.Cvar_Get("sv_running", "0", 0);
    cgs.localServer = temp->integer;

    //
    // Added in OPM
    //

    cg_fov = cgi.Cvar_Get("cg_fov", "80", CVAR_ARCHIVE);
    cg_cheats = cgi.Cvar_Get("cheats", "0", CVAR_USERINFO | CVAR_SERVERINFO | CVAR_LATCH);
}
/*
===============
CG_UseLargeLightmaps

Added in 2.0
Returns true if the standard BSP file should be used, false if the smaller lightmap BSP file should be used
===============
*/
qboolean CG_UseLargeLightmaps(const char* mapName) {
	char buffer[MAX_QPATH];

	Com_sprintf(buffer, sizeof(buffer), "maps/%s_sml.bsp", mapName);

	if (cgi.FS_ReadFile(buffer, NULL, qtrue) == -1) {
		return qtrue;
	}

	return cgi.Cvar_Get("r_largemap", "0", 0)->integer;
}

/*
================
CG_RegisterSoundsForFile

Register the specified ubersound source file
================
*/
void CG_RegisterSoundsForFile(const char *name)
{
    int startTime;
    int endTime;

    Com_Printf("\n\n-----------PARSING '%s'------------\n", name);
    Com_Printf(
        "Any SetCurrentTiki errors means that tiki wasn't prefetched and tiki-specific sounds for it won't work. To "
        "fix prefetch the tiki. Ignore if you don't use that tiki on this level.\n"
    );

    startTime = cgi.Milliseconds();
    CG_Command_ProcessFile(name, qfalse, NULL);
    endTime = cgi.Milliseconds();

    Com_Printf("Parse/Load time: %f seconds.\n", (float)(endTime - startTime) / 1000.0);
    Com_Printf("-------------PARSING '%s' DONE---------------\n\n", name);
}

/*
=================
qsort_compare_strings

perform case-insensitive sorting
=================
*/
int qsort_compare_strings(const void *s1, const void *s2)
{
    return Q_stricmp(*(const char **)s1, *(const char **)s2);
}

/*
=================
CG_RegisterSounds

Called during a precache command
=================
*/
void CG_RegisterSounds(void)
{
    char **fileList;
    int    numFiles;
    int    i;

    fileList = cgi.FS_ListFilteredFiles("ubersound/", "scr", "*.scr", qfalse, &numFiles, qtrue);
    if (cg_target_game >= TG_MOHTA) {
        // Fixed in 2.0
        //  The behavior has changed, all aliases get cleared
        if (cgs.gametype != GT_SINGLE_PLAYER) {
            cgi.Alias_Clear();
        }
    } else {
        if (!cgs.localServer) {
            cgi.Alias_Clear();
        }
    }
    
    qsort(fileList, numFiles, sizeof(char *), &qsort_compare_strings);

    for (i = 0; i < numFiles; i++) {
        // Added in 2.0
        //  Since this version, all files in the ubersound folder
        //  are parsed
        CG_RegisterSoundsForFile(va("ubersound/%s", fileList[i]));
    }

    cgi.FS_FreeFileList(fileList);
}

/*
================
CG_IsHandleUnique

Check if the model handle is unique
================
*/
static qboolean CG_IsHandleUnique(qhandle_t handle) {
    int i;
    int numRef;

    numRef = 0;
    for (i = 0; i < MAX_MODELS; i++) {
        if (cgs.model_draw[i] == handle) {
            numRef++;
            if (numRef >= 2) {
                return qfalse;
            }
        }
    }

    return qtrue;
}

/*
================
CG_ProcessConfigString
================
*/
void CG_ProcessConfigString(int num, qboolean modelOnly)
{
    const char *str;
    size_t      index;
    int         i;

    str = CG_ConfigString(num);

    if (num >= CS_MODELS && num < CS_MODELS + MAX_MODELS) {
        qhandle_t hOldModel;

        index     = num - CS_MODELS;
        hOldModel = cgs.model_draw[index];

        if (str && str[0] && !modelOnly) {
            qhandle_t hModel = cgi.R_RegisterServerModel(str);
            dtiki_t  *tiki;

            if (hModel != hOldModel) {
                if (hOldModel) {
                    assert(CG_IsHandleUnique(hOldModel));
                    cgi.R_UnregisterServerModel(hOldModel);
                }

                cgs.model_draw[index] = hModel;
                assert(CG_IsHandleUnique(hModel));
            }
            tiki = cgi.R_Model_GetHandle(hModel);
            if (tiki) {
                CG_ProcessCacheInitCommands(tiki);
            }

            CG_ServerModelLoaded(str, hModel);
        } else {
            // clear out the model
            if (hOldModel) {
                assert(CG_IsHandleUnique(hOldModel));
                cgi.R_UnregisterServerModel(hOldModel);
            }

            cgs.model_draw[index] = 0;

            if (!str || !str[0]) {
                CG_ServerModelUnloaded(hOldModel);
            }
        }
    }

    if (!modelOnly) {
        switch (num) {
        case CS_RAIN_DENSITY:
            cg.rain.density = atof(str);
            return;
        case CS_RAIN_SPEED:
            cg.rain.speed = atof(str);
            return;
        case CS_RAIN_SPEEDVARY:
            cg.rain.speed_vary = atoi(str);
            return;
        case CS_RAIN_SLANT:
            cg.rain.slant = atoi(str);
            return;
        case CS_RAIN_LENGTH:
            cg.rain.length = atof(str);
            return;
        case CS_RAIN_MINDIST:
            cg.rain.min_dist = atof(str);
            return;
        case CS_RAIN_WIDTH:
            cg.rain.width = atof(str);
            return;
        case CS_RAIN_SHADER:
            Q_strncpyz(cg.rain.currentShader, str, sizeof(cg.rain.currentShader));
            if (cg.rain.numshaders) {
                // Fixed in OPM
                //  not sure why some maps set a digit at the end...
                size_t len = strlen(cg.rain.currentShader);
                if (isdigit(cg.rain.currentShader[len - 1])) {
                    cg.rain.currentShader[len - 1] = 0;
                }
            }
            for (i = 0; i < cg.rain.numshaders; ++i) {
                Com_sprintf(cg.rain.shader[i], sizeof(cg.rain.shader[i]), "%s%i", cg.rain.currentShader, i);
            }
            if (!cg.rain.numshaders) {
                Q_strncpyz(cg.rain.shader[0], cg.rain.currentShader, sizeof(cg.rain.shader[0]));
            }
            return;
        case CS_RAIN_NUMSHADERS:
            cg.rain.numshaders = atoi(str);
            if (cg.rain.numshaders) {
                for (i = 0; i < cg.rain.numshaders; i++) {
                    Com_sprintf(cg.rain.shader[i], sizeof(cg.rain.shader[i]), "%s%i", cg.rain.currentShader, i);
                }
            }
            return;
        case CS_CURRENT_OBJECTIVE:
            cg.ObjectivesCurrentIndex = atoi(str);
            return;
        }

        if (num >= CS_OBJECTIVES && num < CS_OBJECTIVES + MAX_OBJECTIVES) {
            cobjective_t *objective = &cg.Objectives[num - CS_OBJECTIVES];

            objective->flags = atoi(Info_ValueForKey(str, "flags"));
            Q_strncpyz(objective->text, Info_ValueForKey(str, "text"), sizeof(objective->text));
        }

        switch (num) {
        case CS_MUSIC:
            cgi.MUSIC_NewSoundtrack(str);
            return;
        case CS_WARMUP:
            cg.matchStartTime = atoi(str);
            return;
        case CS_FOGINFO:
            cg.farclipOverride          = 0;
            cg.farplaneColorOverride[0] = -1;
            cg.farplaneColorOverride[1] = -1;
            cg.farplaneColorOverride[2] = -1;
            CG_ParseFogInfo(str);
            return;
        case CS_SKYINFO:
            sscanf(str, "%f %d", &cg.sky_alpha, &cg.sky_portal);
            return;
        case CS_SERVERINFO:
            CG_ParseServerinfo();
            return;
        case CS_LEVEL_START_TIME:
            cgs.levelStartTime = atoi(str);
            return;
        case CS_VOTE_TIME:
            cgs.voteTime      = atoi(str);
            cgs.voteRefreshed = qtrue;
            break;
        case CS_VOTE_STRING:
            Q_strncpyz(cgs.voteString, str, sizeof(cgs.voteString));
            break;
        case CS_VOTE_YES:
            cgs.numVotesYes   = atoi(str);
            cgs.voteRefreshed = qtrue;
            break;
        case CS_VOTE_NO:
            cgs.numVotesNo    = atoi(str);
            cgs.voteRefreshed = qtrue;
            break;
        case CS_VOTE_UNDECIDED:
            cgs.numUndecidedVotes = atoi(str);
            cgs.voteRefreshed     = qtrue;
            break;
        case CS_MATCHEND:
            cgs.matchEndTime = atoi(str);
            return;
        }

        if (num >= CS_SOUNDS && num < CS_SOUNDS + MAX_SOUNDS) {
            size_t len = strlen(str);
            if (len) {
                qboolean streamed;
                char     buf[1024];
                Q_strncpyz(buf, str, sizeof(buf));

                streamed     = buf[len - 1] != '0';
                buf[len - 1] = 0;
                if (buf[0] != '*') {
                    cgs.sound_precache[num - CS_SOUNDS] = cgi.S_RegisterSound(buf, streamed);
                }
            }
        } else if (num >= CS_LIGHTSTYLES && num < CS_LIGHTSTYLES + MAX_LIGHTSTYLES) {
            CG_SetLightStyle(num - CS_LIGHTSTYLES, str);
        } else if (num >= CS_PLAYERS && num < CS_PLAYERS + MAX_CLIENTS) {
            const char *value;

            index = num - CS_PLAYERS;

            // Get the name
            value = Info_ValueForKey(str, "name");
            if (!value) {
                value = "UnnamedSoldier";
            }
            Q_strncpyz(cg.clientinfo[index].name, value, sizeof(cg.clientinfo[index].name));

            value = Info_ValueForKey(str, "team");
            if (value) {
                cg.clientinfo[index].team = atoi(value);
            } else {
                cg.clientinfo[index].team = TEAM_NONE;
            }
        }
    }
}

//===================================================================================

/*
=================
CG_PrepRefresh

Call before entering a new level, or after changing renderers
This function may execute for a couple of minutes with a slow disk.
=================
*/
void CG_PrepRefresh(void)
{
    int i;

    memset(&cg.refdef, 0, sizeof(cg.refdef));

    cgi.R_LoadWorldMap(cgs.mapname);

    // register the inline models
    cgs.numInlineModels = cgi.CM_NumInlineModels();

    for (i = 1; i < cgs.numInlineModels; i++) {
        char   name[10];
        vec3_t mins, maxs;
        int    j;

        Com_sprintf(name, sizeof(name), "*%i", i);
        cgs.inlineDrawModel[i] = cgi.R_RegisterModel(name);
        cgi.R_ModelBounds(cgs.inlineDrawModel[i], mins, maxs);

        for (j = 0; j < 3; j++) {
            cgs.inlineModelMidpoints[i][j] = mins[j] + 0.5 * (maxs[j] - mins[j]);
        }
    }
    // register media shaders
    cgs.media.shadowMarkShader         = cgi.R_RegisterShader("markShadow");
    cgs.media.footShadowMarkShader     = cgi.R_RegisterShader("footShadow");
    cgs.media.wakeMarkShader           = cgi.R_RegisterShader("ripple.spr");
    cgs.media.lagometerShader          = cgi.R_RegisterShaderNoMip("gfx/2d/blank");
    cgs.media.levelExitShader          = cgi.R_RegisterShaderNoMip("textures/menu/exit");
    cgs.media.pausedShader             = cgi.R_RegisterShaderNoMip("textures/menu/paused");
    cgs.media.backTileShader           = cgi.R_RegisterShader("gfx/2d/backtile");
    cgs.media.zoomOverlayShader        = cgi.R_RegisterShaderNoMip("textures/hud/zoomoverlay");
    cgs.media.kar98TopOverlayShader    = cgi.R_RegisterShaderNoMip("textures/hud/kartop.tga");
    cgs.media.kar98BottomOverlayShader = cgi.R_RegisterShaderNoMip("textures/hud/karbottom.tga");
    cgs.media.binocularsOverlayShader  = cgi.R_RegisterShaderNoMip("textures/hud/binocularsoverlay");
    cgs.media.hudDrawFont              = cgi.R_LoadFont("verdana-14");
    cgs.media.attackerFont             = cgi.R_LoadFont("facfont-20");
    cgs.media.objectiveFont            = cgi.R_LoadFont("facfont-20"); // was courier-16 before 2.0
    cgs.media.objectivesBackShader     = cgi.R_RegisterShaderNoMip("textures/hud/objectives_backdrop");
    cgs.media.checkedBoxShader         = cgi.R_RegisterShaderNoMip("textures/objectives/filledbox");
    cgs.media.uncheckedBoxShader       = cgi.R_RegisterShaderNoMip("textures/objectives/emptybox");

    // go through all the configstrings and process them
    for (i = CS_SYSTEMINFO + 1; i < MAX_CONFIGSTRINGS; i++) {
        CG_ProcessConfigString(i, qfalse);
    }
}

//===========================================================================

/*
=================
CG_ConfigString
=================
*/
const char *CG_ConfigString(int index)
{
    if (index < 0 || index >= MAX_CONFIGSTRINGS) {
        cgi.Error(ERR_DROP, "CG_ConfigString: bad index: %i", index);
    }
    return cgs.gameState.stringData + cgs.gameState.stringOffsets[index];
}

//==================================================================

void CG_GetRendererConfig(void)
{
    // get the rendering configuration from the client system
    cgi.GetGlconfig(&cgs.glconfig);
    cgs.screenXScale = cgs.glconfig.vidWidth / 640.0;
    cgs.screenYScale = cgs.glconfig.vidHeight / 480.0;
    cgi.UI_GetHighResolutionScale(cgs.uiHiResScale);
}

/*
======================
CG_GameStateReceived

Displays the info screen while loading media
======================
*/
void CG_GameStateReceived(void)
{
    const char *s;
    int checksum;

    // clear everything
    memset(&cg, 0, sizeof(cg));
    memset(cg_entities, 0, sizeof(cg_entities));

    // clear the light styles
    CG_ClearLightStyles();

    // get the rendering configuration from the client system
    CG_GetRendererConfig();

    // get the gamestate from the client system
    cgi.GetGameState(&cgs.gameState);

    // check version
    s = CG_ConfigString(CS_GAME_VERSION);
    if (strcmp(s, GAME_VERSION)) {
        cgi.Error(ERR_DROP, "Client/Server game mismatch: %s/%s", GAME_VERSION, s);
    }

    s                  = CG_ConfigString(CS_LEVEL_START_TIME);
    cgs.levelStartTime = atoi(s);

    CG_ParseServerinfo();

    // load the new map
    cgi.CM_LoadMap(cgs.mapname, &checksum);
    if (cgs.useMapChecksum && checksum != cgs.mapChecksum && cgs.gametype != GT_SINGLE_PLAYER) {
        cgi.Error(ERR_DROP, "Client/Server map checksum mismatch: %x/%x", checksum, cgs.mapChecksum);
    }

    CG_InitMarks();

    CG_RegisterSounds();

    CG_PrepRefresh();

    CG_InitializeSpecialEffectsManager();

    CG_InitializeObjectives();
}

/*
======================
CG_ServerRestarted

The server has beeen restarted, adjust our cgame data accordingly
======================
*/
void CG_ServerRestarted(void)
{
    const char *s;

    s                  = CG_ConfigString(CS_LEVEL_START_TIME);
    cgs.levelStartTime = atoi(s);

    CG_ParseServerinfo();

    cg.thisFrameTeleport = qtrue;
    // free up any temp models currently spawned
    CG_RestartCommandManager();
    // get rid of left over decals from the last game
    CG_InitMarks();
    // clear all the swipes
    CG_ClearSwipes();
    // Reset tempmodels
    CG_ResetTempModels();
    // Reset resources
    CG_ResetVSSSources();
    // Reset objectives
    CG_InitializeObjectives();
}

/*
=================
CG_Init

Called after every level change or subsystem restart
=================
*/
void CG_Init(clientGameImport_t *imported, int serverMessageNum, int serverCommandSequence, int clientNum)
{
    cgi = *imported;

    cg_protocol = cgi.Cvar_Get("com_protocol", "", 0)->integer;
    cg_target_game = (target_game_e)cgi.Cvar_Get("com_target_game", "0", 0)->integer;

    CG_InitCGMessageAPI(&cge);
    CG_InitScoresAPI(&cge);

    memset(&cg, 0, sizeof(cg));
    memset(&cgs, 0, sizeof(cgs));
    // clear fog values
    cg.farclipOverride = 0;
    cg.farplaneColorOverride[0] = -1;
    cg.farplaneColorOverride[1] = -1;
    cg.farplaneColorOverride[2] = -1;

    cg.clientNum              = clientNum;
    cgs.processedSnapshotNum  = serverMessageNum;
    cgs.serverCommandSequence = serverCommandSequence;

    CG_RegisterCvars();

    L_InitEvents();

    // init swapping for endian conversion
    Swap_Init();

    CG_InitializeCommandManager();

    CG_GameStateReceived();

    CG_InitConsoleCommands();

    cg.vEyeOffsetMax[0]         = 40.0f;
    cg.vEyeOffsetMax[1]         = 45.0f;
    cg.vEyeOffsetMax[2]         = 60.0f;
    cg.fEyeOffsetFrac           = 0.1f;
    cg.fCurrentViewHeight       = 0.0f;
    cg.fCurrentViewBobPhase     = 0.0f;
    cg.fCurrentViewBobAmp       = 0.0f;
    cg.pLastPlayerWorldModel    = NULL;
    cg.pPlayerFPSModel          = NULL;
    cg.hPlayerFPSModelHandle    = 0;
    cg.pAlliedPlayerModel       = NULL;
    cg.hAlliedPlayerModelHandle = 0;
    cg.pAxisPlayerModel         = NULL;
    cg.hAxisPlayerModelHandle   = 0;
    cg.bFPSOnGround             = qtrue;

    // Pop the stats UI screen menu
    cgi.UI_HideMenu("StatsScreen", 1);

    // Scoreboard setup
    CG_PrepScoreBoardInfo();
    cgi.UI_HideScoreBoard();

    // HUD setup
    CG_RefreshHudDrawElements();
    cgi.Cmd_Execute(EXEC_NOW, "ui_hud 1\n");
}

/*
=================
CG_Shutdown

Called before every level change or subsystem restart
=================
*/
void CG_Shutdown(void)
{
    L_ShutdownEvents();
    // Shutdown radar
    cgi.CL_InitRadar(NULL, NULL, -1);

    // some mods may need to do cleanup work here,
    // like closing files or archiving session data

    // hide the stats screen
    cgi.UI_HideMenu("StatsScreen", qtrue);

    // reset the scoreboard
    CG_PrepScoreBoardInfo();
    cgi.UI_HideScoreBoard();
}

int CG_GetParent(int entnum)
{
    return cg_entities[entnum].currentState.parent;
}

float CG_GetObjectiveAlpha()
{
    return cg.ObjectivesCurrentAlpha;
}

/*
================
GetCGameAPI

The only exported function from this module
================
*/
clientGameExport_t *GetCGameAPI(void)
{
    cge.CG_Init                     = CG_Init;
    cge.CG_DrawActiveFrame          = CG_DrawActiveFrame;
    cge.CG_Shutdown                 = CG_Shutdown;
    cge.CG_ConsoleCommand           = CG_ConsoleCommand;
    cge.CG_GetRendererConfig        = CG_GetRendererConfig;
    cge.CG_Draw2D                   = CG_Draw2D;
    cge.CG_EyePosition              = CG_EyePosition;
    cge.CG_EyeOffset                = CG_EyeOffset;
    cge.CG_EyeAngles                = CG_EyeAngles;
    cge.CG_SensitivityScale         = CG_SensitivityScale;
    cge.CG_RefreshHudDrawElements   = CG_RefreshHudDrawElements;
    cge.CG_HudDrawShader            = CG_HudDrawShader;
    cge.CG_HudDrawFont              = CG_HudDrawFont;
    cge.CG_PermanentMark            = CG_PermanentMark;
    cge.CG_PermanentTreadMarkDecal  = CG_PermanentTreadMarkDecal;
    cge.CG_PermanentUpdateTreadMark = CG_PermanentUpdateTreadMark;
    cge.CG_Command_ProcessFile      = CG_Command_ProcessFile;
    cge.CG_ProcessInitCommands      = CG_ProcessInitCommands;
    cge.CG_EndTiki                  = CG_EndTiki;
    cge.CG_GetParent                = CG_GetParent;
    cge.CG_GetObjectiveAlpha        = CG_GetObjectiveAlpha;
    cge.CG_WeaponCommandButtonBits  = CG_WeaponCommandButtonBits;
    cge.CG_CheckCaptureKey          = CG_CheckCaptureKey;
    cge.CG_ReadNonPVSClient         = CG_ReadNonPVSClient;
    cge.CG_UpdateRadar              = CG_UpdateRadar;
    cge.CG_SaveStateToBuffer        = CG_SaveStateToBuffer;
    cge.CG_LoadStateToBuffer        = CG_LoadStateToBuffer;
    cge.CG_CleanUpTempModels        = CG_CleanUpTempModels;

    // FIXME
    //cge.profStruct = NULL;

    return &cge;
}

/*
=====================
CG_DrawActive

Perform all drawing needed to completely fill the screen
=====================
*/
void CG_DrawActive(stereoFrame_t stereoView)
{
    float  separation;
    vec3_t baseOrg;

    switch (stereoView) {
    case STEREO_CENTER:
        separation = 0;
        break;
    case STEREO_LEFT:
        separation = -cg_stereoSeparation->value / 2;
        break;
    case STEREO_RIGHT:
        separation = cg_stereoSeparation->value / 2;
        break;
    default:
        separation = 0;
        cgi.Error(ERR_DROP, "CG_DrawActive: Undefined stereoView");
    }

    // clear around the rendered view if sized down
    CG_TileClear();

    // offset vieworg appropriately if we're doing stereo separation
    VectorCopy(cg.refdef.vieworg, baseOrg);
    if (separation != 0) {
        VectorMA(cg.refdef.vieworg, -separation, cg.refdef.viewaxis[1], cg.refdef.vieworg);
    }

    // draw 3D view
    cgi.R_RenderScene(&cg.refdef);

    // restore original viewpoint if running stereo
    if (separation != 0) {
        VectorCopy(baseOrg, cg.refdef.vieworg);
    }
}

#ifndef CGAME_HARD_LINKED
// this is only here so the functions in q_shared.c and bg_*.c can link (FIXME)

void Com_Error(int level, const char *error, ...)
{
    va_list argptr;
    char    text[1024];

    va_start(argptr, error);
    Q_vsnprintf(text, sizeof(text), error, argptr);
    va_end(argptr);

    cgi.Error(level, "%s", text);
}

void Com_Printf(const char *msg, ...)
{
    va_list argptr;
    char    text[1024];

    va_start(argptr, msg);
    Q_vsnprintf(text, sizeof(text), msg, argptr);
    va_end(argptr);

    cgi.Printf("%s", text);
}

#endif

void CG_ParseFogInfo_ver_15(const char *str)
{
    sscanf(
        str,
        "%d %f %f %f %f %f %f %f %d %f %f %f %f",
        &cg.farplane_cull,
        &cg.farplane_distance,
        &cg.farplane_bias,
        &cg.skyboxFarplane,
        &cg.skyboxSpeed,
        &cg.farplane_color[0],
        &cg.farplane_color[1],
        &cg.farplane_color[2],
        &cg.renderTerrain,
        &cg.farclipOverride,
        &cg.farplaneColorOverride[0],
        &cg.farplaneColorOverride[1],
        &cg.farplaneColorOverride[2]
    );
}

void CG_ParseFogInfo_ver_6(const char *str)
{
    //
    // clear all unsupported fields in protocol below version 15
    //

    // don't set the farplane_bias 0, otherwise the renderer will set a minimum value
    cg.farplane_bias            = 0.001f;
    cg.skyboxFarplane           = 0;
    cg.skyboxSpeed              = 0;
    cg.renderTerrain            = qtrue;
    cg.farclipOverride          = -1.0;
    cg.farplaneColorOverride[0] = -1.0;
    cg.farplaneColorOverride[1] = -1.0;
    cg.farplaneColorOverride[2] = -1.0;

    sscanf(
        str,
        "%d %f %f %f %f",
        &cg.farplane_cull,
        &cg.farplane_distance,
        &cg.farplane_color[0],
        &cg.farplane_color[1],
        &cg.farplane_color[2]
    );
}

void CG_ParseFogInfo(const char *str)
{
    if (cg_protocol >= PROTOCOL_MOHTA_MIN) {
        CG_ParseFogInfo_ver_15(str);
    } else {
        CG_ParseFogInfo_ver_6(str);
    }
}
