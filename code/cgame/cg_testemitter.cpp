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
// testemitter functions. Used for designing emitters in game

#include "cg_commands.h"
#include "cg_specialfx.h"

cvar_t* cg_te_alpha;
cvar_t* cg_te_dietouch;
cvar_t* cg_te_bouncefactor;
cvar_t* cg_te_scale;
cvar_t* cg_te_scalemin;
cvar_t* cg_te_scalemax;
cvar_t* cg_te_model;
cvar_t* cg_te_life;
cvar_t* cg_te_color_r;
cvar_t* cg_te_color_g;
cvar_t* cg_te_color_b;
cvar_t* cg_te_accel_x;
cvar_t* cg_te_accel_y;
cvar_t* cg_te_accel_z;
cvar_t* cg_te_count;
cvar_t* cg_te_fade;
cvar_t* cg_te_fadedelay;
cvar_t* cg_te_fadein;
cvar_t* cg_te_spawnrate;
cvar_t* cg_te_offsbase_x;
cvar_t* cg_te_offsbase_y;
cvar_t* cg_te_offsbase_z;
cvar_t* cg_te_offsamp_x;
cvar_t* cg_te_offsamp_y;
cvar_t* cg_te_offsamp_z;
cvar_t* cg_te_scalerate;
cvar_t* cg_te_circle;
cvar_t* cg_te_sphere;
cvar_t* cg_te_insphere;
cvar_t* cg_te_radius;
cvar_t* cg_te_align;
cvar_t* cg_te_flickeralpha;
cvar_t* cg_te_collision;
cvar_t* cg_te_randomroll;
cvar_t* cg_te_anglesbase_p;
cvar_t* cg_te_anglesbase_y;
cvar_t* cg_te_anglesbase_r;
cvar_t* cg_te_anglesamp_p;
cvar_t* cg_te_anglesamp_y;
cvar_t* cg_te_anglesamp_r;
cvar_t* cg_te_forwardvel;
cvar_t* cg_te_randvelbase_x;
cvar_t* cg_te_randvelbase_y;
cvar_t* cg_te_randvelbase_z;
cvar_t* cg_te_randvelamp_x;
cvar_t* cg_te_randvelamp_y;
cvar_t* cg_te_randvelamp_z;
cvar_t* cg_te_clampvelmin_x;
cvar_t* cg_te_clampvelmax_x;
cvar_t* cg_te_clampvelmin_y;
cvar_t* cg_te_clampvelmax_y;
cvar_t* cg_te_clampvelmin_z;
cvar_t* cg_te_clampvelmax_z;
cvar_t* cg_te_clampvelaxis;
cvar_t* cg_te_volumetric;
cvar_t* cg_te_randaxis;
cvar_t* cg_te_axisoffsbase_x;
cvar_t* cg_te_axisoffsbase_y;
cvar_t* cg_te_axisoffsbase_z;
cvar_t* cg_te_axisoffsamp_x;
cvar_t* cg_te_axisoffsamp_y;
cvar_t* cg_te_axisoffsamp_z;
cvar_t* cg_te_swarm_freq;
cvar_t* cg_te_swarm_maxspeed;
cvar_t* cg_te_swarm_delta;
cvar_t* cg_te_avelbase_p;
cvar_t* cg_te_avelbase_y;
cvar_t* cg_te_avelbase_r;
cvar_t* cg_te_avelamp_p;
cvar_t* cg_te_avelamp_y;
cvar_t* cg_te_avelamp_r;
cvar_t* cg_te_radial_scale;
cvar_t* cg_te_radial_min;
cvar_t* cg_te_radial_max;
cvar_t* cg_te_friction;
cvar_t* cg_te_varycolor;
cvar_t* cg_te_spritegridlighting;
cvar_t* cg_te_cone_height;
cvar_t* cg_te_alignstretch_scale;
cvar_t* cg_te_spawnrange_a;
cvar_t* cg_te_spawnrange_b;
cvar_t* cg_te_command_time;
cvar_t* cg_te_singlelinecommand;
cvar_t* cg_te_tag;
cvar_t* cg_te_xangles;
cvar_t* cg_te_yangles;
cvar_t* cg_te_zangles;
class specialeffect_t;
refEntity_t te_refEnt;
cvar_t* cg_te_emittermodel;
cvar_t* cg_te_mode;
cvar_t* cg_te_mode_name;
cvar_t* cg_te_currCommand;
cvar_t* cg_te_numCommands;
float te_vEmitterOrigin[3];
int te_iNumCommands;
int te_iCurrCommand;
specialeffect_t *pTesteffect;

#define MAX_TESTEMITTERS_SAVE 32

str cg_te_alphaG[MAX_TESTEMITTERS_SAVE];
str cg_te_dietouchG[MAX_TESTEMITTERS_SAVE];
str cg_te_bouncefactorG[MAX_TESTEMITTERS_SAVE];
str cg_te_scaleG[MAX_TESTEMITTERS_SAVE];
str cg_te_scaleminG[MAX_TESTEMITTERS_SAVE];
str cg_te_scalemaxG[MAX_TESTEMITTERS_SAVE];
str cg_te_modelG[MAX_TESTEMITTERS_SAVE];
str cg_te_lifeG[MAX_TESTEMITTERS_SAVE];
str cg_te_color_rG[MAX_TESTEMITTERS_SAVE];
str cg_te_color_gG[MAX_TESTEMITTERS_SAVE];
str cg_te_color_bG[MAX_TESTEMITTERS_SAVE];
str cg_te_accel_xG[MAX_TESTEMITTERS_SAVE];
str cg_te_accel_yG[MAX_TESTEMITTERS_SAVE];
str cg_te_accel_zG[MAX_TESTEMITTERS_SAVE];
str cg_te_countG[MAX_TESTEMITTERS_SAVE];
str cg_te_fadeG[MAX_TESTEMITTERS_SAVE];
str cg_te_fadedelayG[MAX_TESTEMITTERS_SAVE];
str cg_te_fadeinG[MAX_TESTEMITTERS_SAVE];
str cg_te_spawnrateG[MAX_TESTEMITTERS_SAVE];
str cg_te_offsbase_xG[MAX_TESTEMITTERS_SAVE];
str cg_te_offsbase_yG[MAX_TESTEMITTERS_SAVE];
str cg_te_offsbase_zG[MAX_TESTEMITTERS_SAVE];
str cg_te_offsamp_xG[MAX_TESTEMITTERS_SAVE];
str cg_te_offsamp_yG[MAX_TESTEMITTERS_SAVE];
str cg_te_offsamp_zG[MAX_TESTEMITTERS_SAVE];
str cg_te_scalerateG[MAX_TESTEMITTERS_SAVE];
str cg_te_circleG[MAX_TESTEMITTERS_SAVE];
str cg_te_sphereG[MAX_TESTEMITTERS_SAVE];
str cg_te_insphereG[MAX_TESTEMITTERS_SAVE];
str cg_te_radiusG[MAX_TESTEMITTERS_SAVE];
str cg_te_alignG[MAX_TESTEMITTERS_SAVE];
str cg_te_flickeralphaG[MAX_TESTEMITTERS_SAVE];
str cg_te_collisionG[MAX_TESTEMITTERS_SAVE];
str cg_te_randomrollG[MAX_TESTEMITTERS_SAVE];
str cg_te_anglesbase_pG[MAX_TESTEMITTERS_SAVE];
str cg_te_anglesbase_yG[MAX_TESTEMITTERS_SAVE];
str cg_te_anglesbase_rG[MAX_TESTEMITTERS_SAVE];
str cg_te_anglesamp_pG[MAX_TESTEMITTERS_SAVE];
str cg_te_anglesamp_yG[MAX_TESTEMITTERS_SAVE];
str cg_te_anglesamp_rG[MAX_TESTEMITTERS_SAVE];
str cg_te_forwardvelG[MAX_TESTEMITTERS_SAVE];
str cg_te_randvelbase_xG[MAX_TESTEMITTERS_SAVE];
str cg_te_randvelbase_yG[MAX_TESTEMITTERS_SAVE];
str cg_te_randvelbase_zG[MAX_TESTEMITTERS_SAVE];
str cg_te_randvelamp_xG[MAX_TESTEMITTERS_SAVE];
str cg_te_randvelamp_yG[MAX_TESTEMITTERS_SAVE];
str cg_te_randvelamp_zG[MAX_TESTEMITTERS_SAVE];
str cg_te_clampvelmin_xG[MAX_TESTEMITTERS_SAVE];
str cg_te_clampvelmax_xG[MAX_TESTEMITTERS_SAVE];
str cg_te_clampvelmin_yG[MAX_TESTEMITTERS_SAVE];
str cg_te_clampvelmax_yG[MAX_TESTEMITTERS_SAVE];
str cg_te_clampvelmin_zG[MAX_TESTEMITTERS_SAVE];
str cg_te_clampvelmax_zG[MAX_TESTEMITTERS_SAVE];
str cg_te_clampvelaxisG[MAX_TESTEMITTERS_SAVE];
str cg_te_volumetricG[MAX_TESTEMITTERS_SAVE];
str cg_te_randaxisG[MAX_TESTEMITTERS_SAVE];
str cg_te_axisoffsbase_xG[MAX_TESTEMITTERS_SAVE];
str cg_te_axisoffsbase_yG[MAX_TESTEMITTERS_SAVE];
str cg_te_axisoffsbase_zG[MAX_TESTEMITTERS_SAVE];
str cg_te_axisoffsamp_xG[MAX_TESTEMITTERS_SAVE];
str cg_te_axisoffsamp_yG[MAX_TESTEMITTERS_SAVE];
str cg_te_axisoffsamp_zG[MAX_TESTEMITTERS_SAVE];
str cg_te_swarm_freqG[MAX_TESTEMITTERS_SAVE];
str cg_te_swarm_maxspeedG[MAX_TESTEMITTERS_SAVE];
str cg_te_swarm_deltaG[MAX_TESTEMITTERS_SAVE];
str cg_te_avelbase_pG[MAX_TESTEMITTERS_SAVE];
str cg_te_avelbase_yG[MAX_TESTEMITTERS_SAVE];
str cg_te_avelbase_rG[MAX_TESTEMITTERS_SAVE];
str cg_te_avelamp_pG[MAX_TESTEMITTERS_SAVE];
str cg_te_avelamp_yG[MAX_TESTEMITTERS_SAVE];
str cg_te_avelamp_rG[MAX_TESTEMITTERS_SAVE];
str cg_te_radial_scaleG[MAX_TESTEMITTERS_SAVE];
str cg_te_radial_minG[MAX_TESTEMITTERS_SAVE];
str cg_te_radial_maxG[MAX_TESTEMITTERS_SAVE];
str cg_te_frictionG[MAX_TESTEMITTERS_SAVE];
str cg_te_varycolorG[MAX_TESTEMITTERS_SAVE];
str cg_te_spritegridlightingG[MAX_TESTEMITTERS_SAVE];
str cg_te_spawnrange_aG[MAX_TESTEMITTERS_SAVE];
str cg_te_spawnrange_bG[MAX_TESTEMITTERS_SAVE];
str cg_te_cone_heightG[MAX_TESTEMITTERS_SAVE];
str cg_te_alignstretch_scaleG[MAX_TESTEMITTERS_SAVE];
str cg_te_command_timeG[MAX_TESTEMITTERS_SAVE];
str cg_te_singlelinecommandG[MAX_TESTEMITTERS_SAVE];
str cg_te_tagG[MAX_TESTEMITTERS_SAVE];

spawnthing_t* testspawnthing = NULL;

void CG_SaveEffectCommandCvars(int iCommand)
{
    cg_te_alphaG[iCommand] = cg_te_alpha->string;
    cg_te_dietouchG[iCommand] = cg_te_dietouch->string;
    cg_te_bouncefactorG[iCommand] = cg_te_bouncefactor->string;
    cg_te_scaleG[iCommand] = cg_te_scale->string;
    cg_te_scaleminG[iCommand] = cg_te_scalemin->string;
    cg_te_scalemaxG[iCommand] = cg_te_scalemax->string;
    cg_te_modelG[iCommand] = cg_te_model->string;
    cg_te_lifeG[iCommand] = cg_te_life->string;
    cg_te_color_rG[iCommand] = cg_te_color_r->string;
    cg_te_color_gG[iCommand] = cg_te_color_g->string;
    cg_te_color_bG[iCommand] = cg_te_color_b->string;
    cg_te_accel_xG[iCommand] = cg_te_accel_x->string;
    cg_te_accel_yG[iCommand] = cg_te_accel_y->string;
    cg_te_accel_zG[iCommand] = cg_te_accel_z->string;
    cg_te_countG[iCommand] = cg_te_count->string;
    cg_te_fadeG[iCommand] = cg_te_fade->string;
    cg_te_fadedelayG[iCommand] = cg_te_fadedelay->string;
    cg_te_fadeinG[iCommand] = cg_te_fadein->string;
    cg_te_spawnrateG[iCommand] = cg_te_spawnrate->string;
    cg_te_offsbase_xG[iCommand] = cg_te_offsbase_x->string;
    cg_te_offsbase_yG[iCommand] = cg_te_offsbase_y->string;
    cg_te_offsbase_zG[iCommand] = cg_te_offsbase_z->string;
    cg_te_offsamp_xG[iCommand] = cg_te_offsamp_x->string;
    cg_te_offsamp_yG[iCommand] = cg_te_offsamp_y->string;
    cg_te_offsamp_zG[iCommand] = cg_te_offsamp_z->string;
    cg_te_scalerateG[iCommand] = cg_te_scalerate->string;
    cg_te_circleG[iCommand] = cg_te_circle->string;
    cg_te_sphereG[iCommand] = cg_te_sphere->string;
    cg_te_insphereG[iCommand] = cg_te_insphere->string;
    cg_te_radiusG[iCommand] = cg_te_radius->string;
    cg_te_alignG[iCommand] = cg_te_align->string;
    cg_te_flickeralphaG[iCommand] = cg_te_flickeralpha->string;
    cg_te_collisionG[iCommand] = cg_te_collision->string;
    cg_te_randomrollG[iCommand] = cg_te_randomroll->string;
    cg_te_anglesbase_pG[iCommand] = cg_te_anglesbase_p->string;
    cg_te_anglesbase_yG[iCommand] = cg_te_anglesbase_y->string;
    cg_te_anglesbase_rG[iCommand] = cg_te_anglesbase_r->string;
    cg_te_anglesamp_pG[iCommand] = cg_te_anglesamp_p->string;
    cg_te_anglesamp_yG[iCommand] = cg_te_anglesamp_y->string;
    cg_te_anglesamp_rG[iCommand] = cg_te_anglesamp_r->string;
    cg_te_forwardvelG[iCommand] = cg_te_forwardvel->string;
    cg_te_randvelbase_xG[iCommand] = cg_te_randvelbase_x->string;
    cg_te_randvelbase_yG[iCommand] = cg_te_randvelbase_y->string;
    cg_te_randvelbase_zG[iCommand] = cg_te_randvelbase_z->string;
    cg_te_randvelamp_xG[iCommand] = cg_te_randvelamp_x->string;
    cg_te_randvelamp_yG[iCommand] = cg_te_randvelamp_y->string;
    cg_te_randvelamp_zG[iCommand] = cg_te_randvelamp_z->string;
    cg_te_clampvelmin_xG[iCommand] = cg_te_clampvelmin_x->string;
    cg_te_clampvelmax_xG[iCommand] = cg_te_clampvelmax_x->string;
    cg_te_clampvelmin_yG[iCommand] = cg_te_clampvelmin_y->string;
    cg_te_clampvelmax_yG[iCommand] = cg_te_clampvelmax_y->string;
    cg_te_clampvelmin_zG[iCommand] = cg_te_clampvelmin_z->string;
    cg_te_clampvelmax_zG[iCommand] = cg_te_clampvelmax_z->string;
    cg_te_clampvelaxisG[iCommand] = cg_te_clampvelaxis->string;
    cg_te_volumetricG[iCommand] = cg_te_volumetric->string;
    cg_te_randaxisG[iCommand] = cg_te_randaxis->string;
    cg_te_axisoffsbase_xG[iCommand] = cg_te_axisoffsbase_x->string;
    cg_te_axisoffsbase_yG[iCommand] = cg_te_axisoffsbase_y->string;
    cg_te_axisoffsbase_zG[iCommand] = cg_te_axisoffsbase_z->string;
    cg_te_axisoffsamp_xG[iCommand] = cg_te_axisoffsamp_x->string;
    cg_te_axisoffsamp_yG[iCommand] = cg_te_axisoffsamp_y->string;
    cg_te_axisoffsamp_zG[iCommand] = cg_te_axisoffsamp_z->string;
    cg_te_swarm_freqG[iCommand] = cg_te_swarm_freq->string;
    cg_te_swarm_maxspeedG[iCommand] = cg_te_swarm_maxspeed->string;
    cg_te_swarm_deltaG[iCommand] = cg_te_swarm_delta->string;
    cg_te_avelbase_pG[iCommand] = cg_te_avelbase_p->string;
    cg_te_avelbase_yG[iCommand] = cg_te_avelbase_y->string;
    cg_te_avelbase_rG[iCommand] = cg_te_avelbase_r->string;
    cg_te_avelamp_pG[iCommand] = cg_te_avelamp_p->string;
    cg_te_avelamp_yG[iCommand] = cg_te_avelamp_y->string;
    cg_te_avelamp_rG[iCommand] = cg_te_avelamp_r->string;
    cg_te_radial_scaleG[iCommand] = cg_te_radial_scale->string;
    cg_te_radial_minG[iCommand] = cg_te_radial_min->string;
    cg_te_radial_maxG[iCommand] = cg_te_radial_max->string;
    cg_te_frictionG[iCommand] = cg_te_friction->string;
    cg_te_varycolorG[iCommand] = cg_te_varycolor->string;
    cg_te_spritegridlightingG[iCommand] = cg_te_spritegridlighting->string;
    cg_te_spawnrange_aG[iCommand] = cg_te_spawnrange_a->string;
    cg_te_spawnrange_bG[iCommand] = cg_te_spawnrange_b->string;
    cg_te_cone_heightG[iCommand] = cg_te_cone_height->string;
    cg_te_alignstretch_scaleG[iCommand] = cg_te_alignstretch_scale->string;
    cg_te_command_timeG[iCommand] = cg_te_command_time->string;
    cg_te_singlelinecommandG[iCommand] = cg_te_singlelinecommand->string;
    cg_te_tagG[iCommand] = cg_te_tag->string;
}

void CG_ResetEffectCommandCvars()
{
    // FIXME: unimplemented
}

void CG_SetTestEmitterValues()
{
    // FIXME: unimplemented
}

void CG_UpdateTestEmitter(void)
{
    // FIXME: unimplemented
}

void CG_SortEffectCommands()
{
    // FIXME: unimplemented
}

void CG_TriggerTestEmitter_f(void)
{
    // FIXME: unimplemented
}

void CG_DumpBaseAndAmplitude(str* buff, char* prefix, Vector* base, Vector* amplitude)
{
    // FIXME: unimplemented
}

void CG_DumpEmitter_f(void)
{
    // FIXME: unimplemented
}

void CG_LoadBaseAndAmplitude(char** pBufer, char* szCvarX, char* szCvarY, char* szCvarZ, char* szCvarXAmp, char* szCvarYAmp, char* szCvarZAmp)
{
    // FIXME: unimplemented
}

void CG_LoadEmitter_f(void)
{
    // FIXME: unimplemented
}

void CG_TestEmitter_f(void)
{
    // FIXME: unimplemented
}

void CG_PrevEmitterCommand_f(void)
{
    // FIXME: unimplemented
}

void CG_NextEmitterCommand_f(void)
{
    // FIXME: unimplemented
}

void CG_NewEmitterCommand_f(void)
{
    // FIXME: unimplemented
}

void CG_DeleteEmitterCommand_f(void)
{
    // FIXME: unimplemented
}

void CG_InitTestEmitter(void)
{
    cg_te_life = cgi.Cvar_Get("cg_te_life", "1", 0);
    cg_te_alpha = cgi.Cvar_Get("cg_te_alpha", "1", 0);
    cg_te_dietouch = cgi.Cvar_Get("cg_te_dietouch", "0", 0);
    cg_te_bouncefactor = cgi.Cvar_Get("cg_te_bouncefactor", "0.3", 0);
    cg_te_scale = cgi.Cvar_Get("cg_te_scale", "1", 0);
    cg_te_scalemin = cgi.Cvar_Get("cg_te_scalemin", "0", 0);
    cg_te_scalemax = cgi.Cvar_Get("cg_te_scalemax", "0", 0);
    cg_te_model = cgi.Cvar_Get("cg_te_model", "none", 0);
    cg_te_life = cgi.Cvar_Get("cg_te_life", "1", 0);
    cg_te_color_r = cgi.Cvar_Get("cg_te_color_r", "1", 0);
    cg_te_color_g = cgi.Cvar_Get("cg_te_color_g", "1", 0);
    cg_te_color_b = cgi.Cvar_Get("cg_te_color_b", "1", 0);
    cg_te_accel_x = cgi.Cvar_Get("cg_te_accel_x", "0", 0);
    cg_te_accel_y = cgi.Cvar_Get("cg_te_accel_y", "0", 0);
    cg_te_accel_z = cgi.Cvar_Get("cg_te_accel_z", "0", 0);
    cg_te_count = cgi.Cvar_Get("cg_te_count", "1", 0);
    cg_te_fade = cgi.Cvar_Get("cg_te_fade", "0", 0);
    cg_te_fadedelay = cgi.Cvar_Get("cg_te_fadedelay", "0", 0);
    cg_te_fadein = cgi.Cvar_Get("cg_te_fadein", "0", 0);
    cg_te_spawnrate = cgi.Cvar_Get("cg_te_spawnrate", "1", 0);
    cg_te_scalerate = cgi.Cvar_Get("cg_te_scalerate", "0", 0);
    cg_te_circle = cgi.Cvar_Get("cg_te_circle", "0", 0);
    cg_te_sphere = cgi.Cvar_Get("cg_te_sphere", "0", 0);
    cg_te_insphere = cgi.Cvar_Get("cg_te_insphere", "0", 0);
    cg_te_radius = cgi.Cvar_Get("cg_te_radius", "0", 0);
    cg_te_align = cgi.Cvar_Get("cg_te_align", "0", 0);
    cg_te_flickeralpha = cgi.Cvar_Get("cg_te_flickeralpha", "0", 0);
    cg_te_collision = cgi.Cvar_Get("cg_te_collision", "0", 0);
    cg_te_randomroll = cgi.Cvar_Get("cg_te_randomroll", "0", 0);
    cg_te_offsbase_x = cgi.Cvar_Get("cg_te_offsbase_x", "0", 0);
    cg_te_offsbase_y = cgi.Cvar_Get("cg_te_offsbase_y", "0", 0);
    cg_te_offsbase_z = cgi.Cvar_Get("cg_te_offsbase_z", "0", 0);
    cg_te_offsamp_x = cgi.Cvar_Get("cg_te_offsamp_x", "0", 0);
    cg_te_offsamp_y = cgi.Cvar_Get("cg_te_offsamp_y", "0", 0);
    cg_te_offsamp_z = cgi.Cvar_Get("cg_te_offsamp_z", "0", 0);
    cg_te_anglesbase_p = cgi.Cvar_Get("cg_te_anglesbase_p", "0", 0);
    cg_te_anglesbase_y = cgi.Cvar_Get("cg_te_anglesbase_y", "0", 0);
    cg_te_anglesbase_r = cgi.Cvar_Get("cg_te_anglesbase_r", "0", 0);
    cg_te_anglesamp_p = cgi.Cvar_Get("cg_te_anglesamp_p", "0", 0);
    cg_te_anglesamp_y = cgi.Cvar_Get("cg_te_anglesamp_y", "0", 0);
    cg_te_anglesamp_r = cgi.Cvar_Get("cg_te_anglesamp_r", "0", 0);
    cg_te_randvelbase_x = cgi.Cvar_Get("cg_te_randvelbase_x", "0", 0);
    cg_te_randvelbase_y = cgi.Cvar_Get("cg_te_randvelbase_y", "0", 0);
    cg_te_randvelbase_z = cgi.Cvar_Get("cg_te_randvelbase_z", "0", 0);
    cg_te_randvelamp_x = cgi.Cvar_Get("cg_te_randvelamp_x", "0", 0);
    cg_te_randvelamp_y = cgi.Cvar_Get("cg_te_randvelamp_y", "0", 0);
    cg_te_randvelamp_z = cgi.Cvar_Get("cg_te_randvelamp_z", "0", 0);
    cg_te_clampvelmin_x = cgi.Cvar_Get("cg_te_clampvelmin_x", "-9999", 0);
    cg_te_clampvelmax_x = cgi.Cvar_Get("cg_te_clampvelmax_x", "9999", 0);
    cg_te_clampvelmin_y = cgi.Cvar_Get("cg_te_clampvelmin_y", "-9999", 0);
    cg_te_clampvelmax_y = cgi.Cvar_Get("cg_te_clampvelmax_y", "9999", 0);
    cg_te_clampvelmin_z = cgi.Cvar_Get("cg_te_clampvelmin_z", "-9999", 0);
    cg_te_clampvelmax_z = cgi.Cvar_Get("cg_te_clampvelmax_z", "9999", 0);
    cg_te_clampvelaxis = cgi.Cvar_Get("cg_te_clampvelaxis", "0", 0);
    cg_te_forwardvel = cgi.Cvar_Get("cg_te_forwardvel", "0", 0);
    cg_te_volumetric = cgi.Cvar_Get("cg_te_volumetric", "0", 0);
    cg_te_randaxis = cgi.Cvar_Get("cg_te_randaxis", "0", 0);
    cg_te_axisoffsbase_x = cgi.Cvar_Get("cg_te_axisoffsbase_x", "0", 0);
    cg_te_axisoffsbase_y = cgi.Cvar_Get("cg_te_axisoffsbase_y", "0", 0);
    cg_te_axisoffsbase_z = cgi.Cvar_Get("cg_te_axisoffsbase_z", "0", 0);
    cg_te_axisoffsamp_x = cgi.Cvar_Get("cg_te_axisoffsamp_x", "0", 0);
    cg_te_axisoffsamp_y = cgi.Cvar_Get("cg_te_axisoffsamp_y", "0", 0);
    cg_te_axisoffsamp_z = cgi.Cvar_Get("cg_te_axisoffsamp_z", "0", 0);
    cg_te_swarm_freq = cgi.Cvar_Get("cg_te_swarm_freq", "0", 0);
    cg_te_swarm_maxspeed = cgi.Cvar_Get("cg_te_swarm_maxspeed", "0", 0);
    cg_te_swarm_delta = cgi.Cvar_Get("cg_te_swarm_delta", "0", 0);
    cg_te_avelbase_p = cgi.Cvar_Get("cg_te_avelbase_p", "0", 0);
    cg_te_avelbase_y = cgi.Cvar_Get("cg_te_avelbase_y", "0", 0);
    cg_te_avelbase_r = cgi.Cvar_Get("cg_te_avelbase_r", "0", 0);
    cg_te_avelamp_p = cgi.Cvar_Get("cg_te_avelamp_p", "0", 0);
    cg_te_avelamp_y = cgi.Cvar_Get("cg_te_avelamp_y", "0", 0);
    cg_te_avelamp_r = cgi.Cvar_Get("cg_te_avelamp_r", "0", 0);
    cg_te_radial_scale = cgi.Cvar_Get("cg_te_radial_scale", "0", 0);
    cg_te_radial_min = cgi.Cvar_Get("cg_te_radial_min", "0", 0);
    cg_te_radial_max = cgi.Cvar_Get("cg_te_radial_max", "0", 0);
    cg_te_friction = cgi.Cvar_Get("cg_te_friction", "0", 0);
    cg_te_varycolor = cgi.Cvar_Get("cg_te_varycolor", "0", 0);
    cg_te_spritegridlighting = cgi.Cvar_Get("cg_te_spritegridlighting", "0", 0);
    cg_te_spawnrange_a = cgi.Cvar_Get("cg_te_spawnrange_a", "0", 0);
    cg_te_spawnrange_b = cgi.Cvar_Get("cg_te_spawnrange_b", "0", 0);
    cg_te_cone_height = cgi.Cvar_Get("cg_te_cone_height", "0", 0);
    cg_te_alignstretch_scale = cgi.Cvar_Get("cg_te_alignstretch_scale", "0", 0);
    cg_te_command_time = cgi.Cvar_Get("cg_te_command_time", "0", 0);
    cg_te_singlelinecommand = cgi.Cvar_Get("cg_te_singlelinecommand", "", 0);
    cg_te_tag = cgi.Cvar_Get("cg_te_tag", "", 0);

    for (int i = 0; i < MAX_TESTEMITTERS_SAVE; ++i) {
        CG_SaveEffectCommandCvars(i);
    }

    cg_te_xangles = cgi.Cvar_Get("cg_te_xangles", "0", 0);
    cg_te_yangles = cgi.Cvar_Get("cg_te_yangles", "0", 0);
    cg_te_zangles = cgi.Cvar_Get("cg_te_zangles", "0", 0);
    cg_te_emittermodel = cgi.Cvar_Get("cg_te_emittermodel", "", 0);
    cg_te_mode = cgi.Cvar_Get("cg_te_mode", "0", 0);
    cg_te_mode_name = cgi.Cvar_Get("cg_te_mode_name", "Emitter Mode", 0);
    cg_te_currCommand = cgi.Cvar_Get("cg_te_currCommand", "0", 0);

    te_vEmitterOrigin[2] = 0.0;
    te_vEmitterOrigin[1] = 0.0;
    te_vEmitterOrigin[0] = 0.0;
    te_refEnt.scale = 0.0;
    cg_te_numCommands = cgi.Cvar_Get("cg_te_numCommands", "0", 0);
    te_iNumCommands = 0;
    te_iCurrCommand = 0;

    pTesteffect = sfxManager.GetTestEffectPointer();
}
