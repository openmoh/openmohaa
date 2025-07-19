/*
===========================================================================
Copyright (C) 2023-2024 the OpenMoHAA team

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

typedef enum te_mode_e {
    TE_MODE_EMITTER,
    TE_MODE_MODEL_ANIM,
    TE_MODE_MODEL_INIT,
    TE_MODE_SFX,
} te_mode_t;

cvar_t *cg_te_alpha;
cvar_t *cg_te_dietouch;
cvar_t *cg_te_bouncefactor;
cvar_t *cg_te_scale;
cvar_t *cg_te_scalemin;
cvar_t *cg_te_scalemax;
cvar_t *cg_te_model;
cvar_t *cg_te_life;
cvar_t *cg_te_color_r;
cvar_t *cg_te_color_g;
cvar_t *cg_te_color_b;
cvar_t *cg_te_accel_x;
cvar_t *cg_te_accel_y;
cvar_t *cg_te_accel_z;
cvar_t *cg_te_count;
cvar_t *cg_te_fade;
cvar_t *cg_te_fadedelay;
cvar_t *cg_te_fadein;
cvar_t *cg_te_spawnrate;
cvar_t *cg_te_offsbase_x;
cvar_t *cg_te_offsbase_y;
cvar_t *cg_te_offsbase_z;
cvar_t *cg_te_offsamp_x;
cvar_t *cg_te_offsamp_y;
cvar_t *cg_te_offsamp_z;
cvar_t *cg_te_scalerate;
cvar_t *cg_te_circle;
cvar_t *cg_te_sphere;
cvar_t *cg_te_insphere;
cvar_t *cg_te_radius;
cvar_t *cg_te_align;
cvar_t *cg_te_flickeralpha;
cvar_t *cg_te_collision;
cvar_t *cg_te_randomroll;
cvar_t *cg_te_anglesbase_p;
cvar_t *cg_te_anglesbase_y;
cvar_t *cg_te_anglesbase_r;
cvar_t *cg_te_anglesamp_p;
cvar_t *cg_te_anglesamp_y;
cvar_t *cg_te_anglesamp_r;
cvar_t *cg_te_forwardvel;
cvar_t *cg_te_randvelbase_x;
cvar_t *cg_te_randvelbase_y;
cvar_t *cg_te_randvelbase_z;
cvar_t *cg_te_randvelamp_x;
cvar_t *cg_te_randvelamp_y;
cvar_t *cg_te_randvelamp_z;
cvar_t *cg_te_clampvelmin_x;
cvar_t *cg_te_clampvelmax_x;
cvar_t *cg_te_clampvelmin_y;
cvar_t *cg_te_clampvelmax_y;
cvar_t *cg_te_clampvelmin_z;
cvar_t *cg_te_clampvelmax_z;
cvar_t *cg_te_clampvelaxis;
cvar_t *cg_te_volumetric;
cvar_t *cg_te_randaxis;
cvar_t *cg_te_axisoffsbase_x;
cvar_t *cg_te_axisoffsbase_y;
cvar_t *cg_te_axisoffsbase_z;
cvar_t *cg_te_axisoffsamp_x;
cvar_t *cg_te_axisoffsamp_y;
cvar_t *cg_te_axisoffsamp_z;
cvar_t *cg_te_swarm_freq;
cvar_t *cg_te_swarm_maxspeed;
cvar_t *cg_te_swarm_delta;
cvar_t *cg_te_avelbase_p;
cvar_t *cg_te_avelbase_y;
cvar_t *cg_te_avelbase_r;
cvar_t *cg_te_avelamp_p;
cvar_t *cg_te_avelamp_y;
cvar_t *cg_te_avelamp_r;
cvar_t *cg_te_radial_scale;
cvar_t *cg_te_radial_min;
cvar_t *cg_te_radial_max;
cvar_t *cg_te_friction;
cvar_t *cg_te_spin; // Added in 2.0
cvar_t *cg_te_varycolor;
cvar_t *cg_te_spritegridlighting;
cvar_t *cg_te_cone_height;
cvar_t *cg_te_alignstretch_scale;
cvar_t *cg_te_spawnrange_a;
cvar_t *cg_te_spawnrange_b;
cvar_t *cg_te_command_time;
cvar_t *cg_te_singlelinecommand;
cvar_t *cg_te_tag;
cvar_t *cg_te_xangles;
cvar_t *cg_te_yangles;
cvar_t *cg_te_zangles;
cvar_t *cg_te_emittermodel;
cvar_t *cg_te_mode;
cvar_t *cg_te_mode_name;
cvar_t *cg_te_currCommand;
cvar_t *cg_te_numCommands;

refEntity_t te_refEnt;
float       te_vEmitterOrigin[3];
int         te_iNumCommands;
int         te_iCurrCommand;

specialeffect_t *pTesteffect;

specialeffectcommand_t *pCurrCommand;
spawnthing_t           *pCurrSpawnthing;

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
str cg_te_spinG[MAX_TESTEMITTERS_SAVE]; // Added in 2.0
str cg_te_varycolorG[MAX_TESTEMITTERS_SAVE];
str cg_te_spritegridlightingG[MAX_TESTEMITTERS_SAVE];
str cg_te_spawnrange_aG[MAX_TESTEMITTERS_SAVE];
str cg_te_spawnrange_bG[MAX_TESTEMITTERS_SAVE];
str cg_te_cone_heightG[MAX_TESTEMITTERS_SAVE];
str cg_te_alignstretch_scaleG[MAX_TESTEMITTERS_SAVE];
str cg_te_command_timeG[MAX_TESTEMITTERS_SAVE];
str cg_te_singlelinecommandG[MAX_TESTEMITTERS_SAVE];
str cg_te_tagG[MAX_TESTEMITTERS_SAVE];

spawnthing_t *testspawnthing = NULL;

void CG_SaveEffectCommandCvars(int iCommand)
{
    cg_te_alphaG[iCommand]              = cg_te_alpha->string;
    cg_te_dietouchG[iCommand]           = cg_te_dietouch->string;
    cg_te_bouncefactorG[iCommand]       = cg_te_bouncefactor->string;
    cg_te_scaleG[iCommand]              = cg_te_scale->string;
    cg_te_scaleminG[iCommand]           = cg_te_scalemin->string;
    cg_te_scalemaxG[iCommand]           = cg_te_scalemax->string;
    cg_te_modelG[iCommand]              = cg_te_model->string;
    cg_te_lifeG[iCommand]               = cg_te_life->string;
    cg_te_color_rG[iCommand]            = cg_te_color_r->string;
    cg_te_color_gG[iCommand]            = cg_te_color_g->string;
    cg_te_color_bG[iCommand]            = cg_te_color_b->string;
    cg_te_accel_xG[iCommand]            = cg_te_accel_x->string;
    cg_te_accel_yG[iCommand]            = cg_te_accel_y->string;
    cg_te_accel_zG[iCommand]            = cg_te_accel_z->string;
    cg_te_countG[iCommand]              = cg_te_count->string;
    cg_te_fadeG[iCommand]               = cg_te_fade->string;
    cg_te_fadedelayG[iCommand]          = cg_te_fadedelay->string;
    cg_te_fadeinG[iCommand]             = cg_te_fadein->string;
    cg_te_spawnrateG[iCommand]          = cg_te_spawnrate->string;
    cg_te_offsbase_xG[iCommand]         = cg_te_offsbase_x->string;
    cg_te_offsbase_yG[iCommand]         = cg_te_offsbase_y->string;
    cg_te_offsbase_zG[iCommand]         = cg_te_offsbase_z->string;
    cg_te_offsamp_xG[iCommand]          = cg_te_offsamp_x->string;
    cg_te_offsamp_yG[iCommand]          = cg_te_offsamp_y->string;
    cg_te_offsamp_zG[iCommand]          = cg_te_offsamp_z->string;
    cg_te_scalerateG[iCommand]          = cg_te_scalerate->string;
    cg_te_circleG[iCommand]             = cg_te_circle->string;
    cg_te_sphereG[iCommand]             = cg_te_sphere->string;
    cg_te_insphereG[iCommand]           = cg_te_insphere->string;
    cg_te_radiusG[iCommand]             = cg_te_radius->string;
    cg_te_alignG[iCommand]              = cg_te_align->string;
    cg_te_flickeralphaG[iCommand]       = cg_te_flickeralpha->string;
    cg_te_collisionG[iCommand]          = cg_te_collision->string;
    cg_te_randomrollG[iCommand]         = cg_te_randomroll->string;
    cg_te_anglesbase_pG[iCommand]       = cg_te_anglesbase_p->string;
    cg_te_anglesbase_yG[iCommand]       = cg_te_anglesbase_y->string;
    cg_te_anglesbase_rG[iCommand]       = cg_te_anglesbase_r->string;
    cg_te_anglesamp_pG[iCommand]        = cg_te_anglesamp_p->string;
    cg_te_anglesamp_yG[iCommand]        = cg_te_anglesamp_y->string;
    cg_te_anglesamp_rG[iCommand]        = cg_te_anglesamp_r->string;
    cg_te_forwardvelG[iCommand]         = cg_te_forwardvel->string;
    cg_te_randvelbase_xG[iCommand]      = cg_te_randvelbase_x->string;
    cg_te_randvelbase_yG[iCommand]      = cg_te_randvelbase_y->string;
    cg_te_randvelbase_zG[iCommand]      = cg_te_randvelbase_z->string;
    cg_te_randvelamp_xG[iCommand]       = cg_te_randvelamp_x->string;
    cg_te_randvelamp_yG[iCommand]       = cg_te_randvelamp_y->string;
    cg_te_randvelamp_zG[iCommand]       = cg_te_randvelamp_z->string;
    cg_te_clampvelmin_xG[iCommand]      = cg_te_clampvelmin_x->string;
    cg_te_clampvelmax_xG[iCommand]      = cg_te_clampvelmax_x->string;
    cg_te_clampvelmin_yG[iCommand]      = cg_te_clampvelmin_y->string;
    cg_te_clampvelmax_yG[iCommand]      = cg_te_clampvelmax_y->string;
    cg_te_clampvelmin_zG[iCommand]      = cg_te_clampvelmin_z->string;
    cg_te_clampvelmax_zG[iCommand]      = cg_te_clampvelmax_z->string;
    cg_te_clampvelaxisG[iCommand]       = cg_te_clampvelaxis->string;
    cg_te_volumetricG[iCommand]         = cg_te_volumetric->string;
    cg_te_randaxisG[iCommand]           = cg_te_randaxis->string;
    cg_te_axisoffsbase_xG[iCommand]     = cg_te_axisoffsbase_x->string;
    cg_te_axisoffsbase_yG[iCommand]     = cg_te_axisoffsbase_y->string;
    cg_te_axisoffsbase_zG[iCommand]     = cg_te_axisoffsbase_z->string;
    cg_te_axisoffsamp_xG[iCommand]      = cg_te_axisoffsamp_x->string;
    cg_te_axisoffsamp_yG[iCommand]      = cg_te_axisoffsamp_y->string;
    cg_te_axisoffsamp_zG[iCommand]      = cg_te_axisoffsamp_z->string;
    cg_te_swarm_freqG[iCommand]         = cg_te_swarm_freq->string;
    cg_te_swarm_maxspeedG[iCommand]     = cg_te_swarm_maxspeed->string;
    cg_te_swarm_deltaG[iCommand]        = cg_te_swarm_delta->string;
    cg_te_avelbase_pG[iCommand]         = cg_te_avelbase_p->string;
    cg_te_avelbase_yG[iCommand]         = cg_te_avelbase_y->string;
    cg_te_avelbase_rG[iCommand]         = cg_te_avelbase_r->string;
    cg_te_avelamp_pG[iCommand]          = cg_te_avelamp_p->string;
    cg_te_avelamp_yG[iCommand]          = cg_te_avelamp_y->string;
    cg_te_avelamp_rG[iCommand]          = cg_te_avelamp_r->string;
    cg_te_radial_scaleG[iCommand]       = cg_te_radial_scale->string;
    cg_te_radial_minG[iCommand]         = cg_te_radial_min->string;
    cg_te_radial_maxG[iCommand]         = cg_te_radial_max->string;
    cg_te_frictionG[iCommand]           = cg_te_friction->string;
    cg_te_spinG[iCommand]               = cg_te_spin->string; // Added in 2.0
    cg_te_varycolorG[iCommand]          = cg_te_varycolor->string;
    cg_te_spritegridlightingG[iCommand] = cg_te_spritegridlighting->string;
    cg_te_spawnrange_aG[iCommand]       = cg_te_spawnrange_a->string;
    cg_te_spawnrange_bG[iCommand]       = cg_te_spawnrange_b->string;
    cg_te_cone_heightG[iCommand]        = cg_te_cone_height->string;
    cg_te_alignstretch_scaleG[iCommand] = cg_te_alignstretch_scale->string;
    cg_te_command_timeG[iCommand]       = cg_te_command_time->string;
    cg_te_singlelinecommandG[iCommand]  = cg_te_singlelinecommand->string;
    cg_te_tagG[iCommand]                = cg_te_tag->string;
}

void CG_GetEffectCommandCvars(int iCommand)
{
    cgi.Cvar_Set("cg_te_alpha", cg_te_alphaG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_dietouch", cg_te_dietouchG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_bouncefactor", cg_te_bouncefactorG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_scale", cg_te_scaleG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_scalemin", cg_te_scaleminG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_scalemax", cg_te_scalemaxG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_model", cg_te_modelG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_life", cg_te_lifeG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_color_r", cg_te_color_rG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_color_g", cg_te_color_gG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_color_b", cg_te_color_bG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_accel_x", cg_te_accel_xG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_accel_y", cg_te_accel_yG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_accel_z", cg_te_accel_zG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_count", cg_te_countG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_fade", cg_te_fadeG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_fadedelay", cg_te_fadedelayG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_fadein", cg_te_fadeinG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_spawnrate", cg_te_spawnrateG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_offsbase_x", cg_te_offsbase_xG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_offsbase_y", cg_te_offsbase_yG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_offsbase_z", cg_te_offsbase_zG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_offsamp_x", cg_te_offsamp_xG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_offsamp_y", cg_te_offsamp_yG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_offsamp_z", cg_te_offsamp_zG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_scalerate", cg_te_scalerateG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_circle", cg_te_circleG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_sphere", cg_te_sphereG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_insphere", cg_te_insphereG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_radius", cg_te_radiusG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_align", cg_te_alignG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_flickeralpha", cg_te_flickeralphaG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_collision", cg_te_collisionG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_randomroll", cg_te_randomrollG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_anglesbase_p", cg_te_anglesbase_pG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_anglesbase_y", cg_te_anglesbase_yG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_anglesbase_r", cg_te_anglesbase_rG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_anglesamp_p", cg_te_anglesamp_pG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_anglesamp_y", cg_te_anglesamp_yG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_anglesamp_r", cg_te_anglesamp_rG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_forwardvel", cg_te_forwardvelG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_randvelbase_x", cg_te_randvelbase_xG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_randvelbase_y", cg_te_randvelbase_yG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_randvelbase_z", cg_te_randvelbase_zG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_randvelamp_x", cg_te_randvelamp_xG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_randvelamp_y", cg_te_randvelamp_yG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_randvelamp_z", cg_te_randvelamp_zG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_clampvelmin_x", cg_te_clampvelmin_xG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_clampvelmax_x", cg_te_clampvelmax_xG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_clampvelmin_y", cg_te_clampvelmin_yG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_clampvelmax_y", cg_te_clampvelmax_yG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_clampvelmin_z", cg_te_clampvelmin_zG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_clampvelmax_z", cg_te_clampvelmax_zG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_clampvelaxis", cg_te_clampvelaxisG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_volumetric", cg_te_volumetricG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_randaxis", cg_te_randaxisG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_axisoffsbase_x", cg_te_axisoffsbase_xG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_axisoffsbase_y", cg_te_axisoffsbase_yG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_axisoffsbase_z", cg_te_axisoffsbase_zG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_axisoffsamp_x", cg_te_axisoffsamp_xG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_axisoffsamp_y", cg_te_axisoffsamp_yG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_axisoffsamp_z", cg_te_axisoffsamp_zG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_swarm_freq", cg_te_swarm_freqG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_swarm_maxspeed", cg_te_swarm_maxspeedG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_swarm_delta", cg_te_swarm_deltaG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_avelbase_p", cg_te_avelbase_pG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_avelbase_y", cg_te_avelbase_yG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_avelbase_r", cg_te_avelbase_rG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_avelamp_p", cg_te_avelamp_pG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_avelamp_y", cg_te_avelamp_yG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_avelamp_r", cg_te_avelamp_rG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_radial_scale", cg_te_radial_scaleG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_radial_min", cg_te_radial_minG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_radial_max", cg_te_radial_maxG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_friction", cg_te_frictionG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_spin", cg_te_spinG[iCommand].c_str()); // Added in 2.0
    cgi.Cvar_Set("cg_te_varycolor", cg_te_varycolorG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_spritegridlighting", cg_te_spritegridlightingG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_spawnrange_a", cg_te_spawnrange_aG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_spawnrnage_b", cg_te_spawnrange_bG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_cone_height", cg_te_cone_heightG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_alignstretch_scale", cg_te_alignstretch_scaleG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_command_time", cg_te_command_timeG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_singlelinecommand", cg_te_singlelinecommandG[iCommand].c_str());
    cgi.Cvar_Set("cg_te_tag", cg_te_tagG[iCommand].c_str());
}

void CG_ResetEffectCommandCvars()
{
    cgi.Cvar_Set("cg_te_alpha", "1");
    cgi.Cvar_Set("cg_te_dietouch", "0");
    cgi.Cvar_Set("cg_te_bouncefactor", "0.3");
    cgi.Cvar_Set("cg_te_scale", "1");
    cgi.Cvar_Set("cg_te_scalemin", "0");
    cgi.Cvar_Set("cg_te_scalemax", "0");
    cgi.Cvar_Set("cg_te_model", "none");
    cgi.Cvar_Set("cg_te_life", "1");
    cgi.Cvar_Set("cg_te_color_r", "1");
    cgi.Cvar_Set("cg_te_color_g", "1");
    cgi.Cvar_Set("cg_te_color_b", "1");
    cgi.Cvar_Set("cg_te_accel_x", "0");
    cgi.Cvar_Set("cg_te_accel_y", "0");
    cgi.Cvar_Set("cg_te_accel_z", "0");
    cgi.Cvar_Set("cg_te_count", "1");
    cgi.Cvar_Set("cg_te_fade", "0");
    cgi.Cvar_Set("cg_te_fadedelay", "0");
    cgi.Cvar_Set("cg_te_fadein", "0");
    cgi.Cvar_Set("cg_te_spawnrate", "1");
    cgi.Cvar_Set("cg_te_offsbase_x", "0");
    cgi.Cvar_Set("cg_te_offsbase_y", "0");
    cgi.Cvar_Set("cg_te_offsbase_z", "0");
    cgi.Cvar_Set("cg_te_offsamp_x", "0");
    cgi.Cvar_Set("cg_te_offsamp_y", "0");
    cgi.Cvar_Set("cg_te_offsamp_z", "0");
    cgi.Cvar_Set("cg_te_scalerate", "0");
    cgi.Cvar_Set("cg_te_circle", "0");
    cgi.Cvar_Set("cg_te_sphere", "0");
    cgi.Cvar_Set("cg_te_insphere", "0");
    cgi.Cvar_Set("cg_te_radius", "0");
    cgi.Cvar_Set("cg_te_align", "0");
    cgi.Cvar_Set("cg_te_flickeralpha", "0");
    cgi.Cvar_Set("cg_te_collision", "0");
    cgi.Cvar_Set("cg_te_randomroll", "0");
    cgi.Cvar_Set("cg_te_anglesbase_p", "0");
    cgi.Cvar_Set("cg_te_anglesbase_y", "0");
    cgi.Cvar_Set("cg_te_anglesbase_r", "0");
    cgi.Cvar_Set("cg_te_anglesamp_p", "0");
    cgi.Cvar_Set("cg_te_anglesamp_y", "0");
    cgi.Cvar_Set("cg_te_anglesamp_r", "0");
    cgi.Cvar_Set("cg_te_forwardvel", "0");
    cgi.Cvar_Set("cg_te_randvelbase_x", "0");
    cgi.Cvar_Set("cg_te_randvelbase_y", "0");
    cgi.Cvar_Set("cg_te_randvelbase_z", "0");
    cgi.Cvar_Set("cg_te_randvelamp_x", "0");
    cgi.Cvar_Set("cg_te_randvelamp_y", "0");
    cgi.Cvar_Set("cg_te_randvelamp_z", "0");
    cgi.Cvar_Set("cg_te_clampvelmin_x", "-9999");
    cgi.Cvar_Set("cg_te_clampvelmax_x", "9999");
    cgi.Cvar_Set("cg_te_clampvelmin_y", "-9999");
    cgi.Cvar_Set("cg_te_clampvelmax_y", "9999");
    cgi.Cvar_Set("cg_te_clampvelmin_z", "-9999");
    cgi.Cvar_Set("cg_te_clampvelmax_z", "9999");
    cgi.Cvar_Set("cg_te_clampvelaxis", "0");
    cgi.Cvar_Set("cg_te_volumetric", "0");
    cgi.Cvar_Set("cg_te_randaxis", "0");
    cgi.Cvar_Set("cg_te_axisoffsbase_x", "0");
    cgi.Cvar_Set("cg_te_axisoffsbase_y", "0");
    cgi.Cvar_Set("cg_te_axisoffsbase_z", "0");
    cgi.Cvar_Set("cg_te_axisoffsamp_x", "0");
    cgi.Cvar_Set("cg_te_axisoffsamp_y", "0");
    cgi.Cvar_Set("cg_te_axisoffsamp_z", "0");
    cgi.Cvar_Set("cg_te_swarm_freq", "0");
    cgi.Cvar_Set("cg_te_swarm_maxspeed", "0");
    cgi.Cvar_Set("cg_te_swarm_delta", "0");
    cgi.Cvar_Set("cg_te_avelbase_p", "0");
    cgi.Cvar_Set("cg_te_avelbase_y", "0");
    cgi.Cvar_Set("cg_te_avelbase_r", "0");
    cgi.Cvar_Set("cg_te_avelamp_p", "0");
    cgi.Cvar_Set("cg_te_avelamp_y", "0");
    cgi.Cvar_Set("cg_te_avelamp_r", "0");
    cgi.Cvar_Set("cg_te_radial_scale", "0");
    cgi.Cvar_Set("cg_te_radial_min", "0");
    cgi.Cvar_Set("cg_te_radial_max", "0");
    cgi.Cvar_Set("cg_te_friction", "0");
    cgi.Cvar_Set("cg_te_spin", "0"); // Added in 2.0
    cgi.Cvar_Set("cg_te_varycolor", "0");
    cgi.Cvar_Set("cg_te_spritegridlighting", "0");
    cgi.Cvar_Set("cg_te_spawnrange_a", "0");
    cgi.Cvar_Set("cg_te_spawnrange_b", "0");
    cgi.Cvar_Set("cg_te_cone_height", "0");
    cgi.Cvar_Set("cg_te_alignstretch_scale", "0");
    cgi.Cvar_Set("cg_te_command_time", "0");
    cgi.Cvar_Set("cg_te_singlelinecommand", "");
    cgi.Cvar_Set("cg_te_tag", "");
}

void CG_SetTestEmitterValues()
{
    int    i;
    int    iTagNum;
    vec3_t vAngles;
    vec3_t vOrigin;
    vec3_t axis[3];

    pCurrSpawnthing->cgd.flags2 |= T2_ACCEL | T2_AMOVE | T2_MOVE;

    if (te_refEnt.scale > 0 && Q_stricmp(cg_te_tag->string, "") && cg_te_mode->integer != TE_MODE_SFX) {
        iTagNum = cgi.Tag_NumForName(te_refEnt.tiki, cg_te_tag->string);
    } else {
        iTagNum = -1;
    }

    VectorCopy(te_vEmitterOrigin, vOrigin);

    if (iTagNum != -1) {
        orientation_t oTag;

        oTag = cgi.TIKI_Orientation(&te_refEnt, iTagNum);
        for (i = 0; i < 3; ++i) {
            VectorMA(vOrigin, oTag.origin[i], te_refEnt.axis[i], vOrigin);
        }

        R_ConcatRotations(oTag.axis, te_refEnt.axis, axis);
    } else {
        vAngles[0] = cg_te_xangles->value;
        vAngles[1] = cg_te_yangles->value;
        vAngles[2] = cg_te_zangles->value;
        AnglesToAxis(vAngles, axis);
    }

    VectorCopy(vOrigin, pCurrSpawnthing->cgd.origin);
    pCurrSpawnthing->SetModel(cg_te_model->string);

    AxisCopy(axis, pCurrSpawnthing->axis);

    pCurrSpawnthing->cgd.alpha        = cg_te_alpha->value;
    pCurrSpawnthing->cgd.color[3]     = cg_te_alpha->value;
    pCurrSpawnthing->cgd.bouncefactor = cg_te_bouncefactor->value;
    pCurrSpawnthing->cgd.scale        = cg_te_scale->value;
    pCurrSpawnthing->cgd.life         = cg_te_life->value * 1000.0;
    pCurrSpawnthing->cgd.color[0]     = cg_te_color_r->value;
    pCurrSpawnthing->cgd.color[1]     = cg_te_color_g->value;
    pCurrSpawnthing->cgd.color[2]     = cg_te_color_b->value;

    if (cg_te_varycolor->integer) {
        pCurrSpawnthing->cgd.flags2 |= T2_VARYCOLOR;
    } else {
        pCurrSpawnthing->cgd.flags2 &= ~T2_VARYCOLOR;
    }

    if (cg_te_spritegridlighting->integer) {
        pCurrSpawnthing->cgd.flags2 |= T2_SPRITEGRIDLIGHTING;
    } else {
        pCurrSpawnthing->cgd.flags2 &= ~T2_SPRITEGRIDLIGHTING;
    }

    pCurrSpawnthing->cgd.accel[0] = cg_te_accel_x->value;
    pCurrSpawnthing->cgd.accel[1] = cg_te_accel_y->value;
    pCurrSpawnthing->cgd.accel[2] = cg_te_accel_z->value;

    pCurrSpawnthing->count     = cg_te_count->value;
    pCurrSpawnthing->spawnRate = 1.0 / cg_te_spawnrate->value * 1000.0;

    if (cg_te_spawnrange_a->value || cg_te_spawnrange_b->value) {
        vAngles[0] = Square(cg_te_spawnrange_a->value);
        vAngles[1] = Square(cg_te_spawnrange_b->value);

        if (vAngles[0] > vAngles[1]) {
            pCurrSpawnthing->fMaxRangeSquared = vAngles[0];
            pCurrSpawnthing->fMinRangeSquared = vAngles[1];
        } else {
            pCurrSpawnthing->fMinRangeSquared = vAngles[0];
            pCurrSpawnthing->fMaxRangeSquared = vAngles[2];
        }
    } else {
        pCurrSpawnthing->fMinRangeSquared = 0.0;
        pCurrSpawnthing->fMaxRangeSquared = 9.9999997e37f;
    }

    pCurrSpawnthing->cgd.scaleRate = cg_te_scalerate->value;
    pCurrSpawnthing->sphereRadius  = cg_te_radius->value;

    if (cg_te_cone_height->value) {
        pCurrSpawnthing->cgd.flags2 |= T2_CONE;
        pCurrSpawnthing->coneHeight = cg_te_cone_height->value;
    } else {
        pCurrSpawnthing->cgd.flags2 &= ~T2_CONE;
    }

    pCurrSpawnthing->forwardVelocity = cg_te_forwardvel->value;
    if (cg_te_friction->value) {
        pCurrSpawnthing->cgd.flags2 |= T2_FRICTION;
    } else {
        pCurrSpawnthing->cgd.flags2 &= ~T2_FRICTION;
    }

    //
    // Added in 2.0
    //
    if (cg_te_spin->value) {
        pCurrSpawnthing->cgd.flags2 |= T2_SPIN;
    } else {
        pCurrSpawnthing->cgd.flags2 &= ~T2_SPIN;
    }

    pCurrSpawnthing->origin_offset_base[0]      = cg_te_offsbase_x->value;
    pCurrSpawnthing->origin_offset_base[1]      = cg_te_offsbase_y->value;
    pCurrSpawnthing->origin_offset_base[2]      = cg_te_offsbase_z->value;
    pCurrSpawnthing->origin_offset_amplitude[0] = cg_te_offsamp_x->value;
    pCurrSpawnthing->origin_offset_amplitude[1] = cg_te_offsamp_y->value;
    pCurrSpawnthing->origin_offset_amplitude[2] = cg_te_offsamp_z->value;

    pCurrSpawnthing->axis_offset_base[0]      = cg_te_axisoffsbase_x->value;
    pCurrSpawnthing->axis_offset_base[1]      = cg_te_axisoffsbase_y->value;
    pCurrSpawnthing->axis_offset_base[2]      = cg_te_axisoffsbase_z->value;
    pCurrSpawnthing->axis_offset_amplitude[0] = cg_te_axisoffsamp_x->value;
    pCurrSpawnthing->axis_offset_amplitude[1] = cg_te_axisoffsamp_y->value;
    pCurrSpawnthing->axis_offset_amplitude[2] = cg_te_axisoffsamp_z->value;

    pCurrSpawnthing->cgd.angles[0]       = cg_te_anglesbase_p->value;
    pCurrSpawnthing->cgd.angles[1]       = cg_te_anglesbase_y->value;
    pCurrSpawnthing->cgd.angles[2]       = cg_te_anglesbase_r->value;
    pCurrSpawnthing->angles_amplitude[0] = cg_te_anglesamp_p->value;
    pCurrSpawnthing->angles_amplitude[1] = cg_te_anglesamp_y->value;
    pCurrSpawnthing->angles_amplitude[2] = cg_te_anglesamp_r->value;

    if (!VectorCompare(pCurrSpawnthing->cgd.angles, vec_zero)
        || !VectorCompare(pCurrSpawnthing->angles_amplitude, vec_zero)) {
        pCurrSpawnthing->cgd.flags |= T_ANGLES;
    } else {
        pCurrSpawnthing->cgd.flags &= ~T_ANGLES;
    }

    pCurrSpawnthing->avelocity_base[0]      = cg_te_avelbase_p->value;
    pCurrSpawnthing->avelocity_base[1]      = cg_te_avelbase_y->value;
    pCurrSpawnthing->avelocity_base[2]      = cg_te_avelbase_r->value;
    pCurrSpawnthing->avelocity_amplitude[0] = cg_te_avelamp_p->value;
    pCurrSpawnthing->avelocity_amplitude[1] = cg_te_avelamp_y->value;
    pCurrSpawnthing->avelocity_amplitude[2] = cg_te_avelamp_r->value;

    pCurrSpawnthing->randvel_base[0]      = cg_te_randvelbase_x->value;
    pCurrSpawnthing->randvel_base[1]      = cg_te_randvelbase_y->value;
    pCurrSpawnthing->randvel_base[2]      = cg_te_randvelbase_z->value;
    pCurrSpawnthing->randvel_amplitude[0] = cg_te_randvelamp_x->value;
    pCurrSpawnthing->randvel_amplitude[1] = cg_te_randvelamp_y->value;
    pCurrSpawnthing->randvel_amplitude[2] = cg_te_randvelamp_z->value;

    if (cg_te_randaxis->integer) {
        pCurrSpawnthing->cgd.flags |= T_RANDVELAXIS;
    } else {
        pCurrSpawnthing->cgd.flags &= ~T_RANDVELAXIS;
    }

    if (cg_te_radial_scale->value || cg_te_radial_min->value || cg_te_radial_max->value) {
        pCurrSpawnthing->cgd.velocity[0] = cg_te_radial_scale->value;
        pCurrSpawnthing->cgd.velocity[1] = cg_te_radial_min->value;
        pCurrSpawnthing->cgd.velocity[2] = cg_te_radial_max->value;
        pCurrSpawnthing->cgd.velocity[2] -= pCurrSpawnthing->cgd.velocity[1];

        pCurrSpawnthing->cgd.flags2 |= T2_RADIALVELOCITY;
    } else {
        pCurrSpawnthing->cgd.flags2 &= ~T2_RADIALVELOCITY;
    }

    pCurrSpawnthing->cgd.minVel[0] = cg_te_clampvelmin_x->value;
    pCurrSpawnthing->cgd.maxVel[0] = cg_te_clampvelmax_x->value;
    pCurrSpawnthing->cgd.minVel[1] = cg_te_clampvelmin_y->value;
    pCurrSpawnthing->cgd.maxVel[1] = cg_te_clampvelmax_y->value;
    pCurrSpawnthing->cgd.minVel[2] = cg_te_clampvelmin_z->value;
    pCurrSpawnthing->cgd.maxVel[2] = cg_te_clampvelmax_z->value;
    pCurrSpawnthing->cgd.flags &= ~(T_GLOBALFADEIN | T_GLOBALFADEOUT);

    if (pCurrSpawnthing->cgd.minVel[0] > -9999 || pCurrSpawnthing->cgd.minVel[1] > -9999
        || pCurrSpawnthing->cgd.minVel[2] > -9999 || pCurrSpawnthing->cgd.maxVel[0] < 9999
        || pCurrSpawnthing->cgd.maxVel[1] < 9999 || pCurrSpawnthing->cgd.maxVel[2] < 9999) {
        if (cg_te_clampvelaxis->value) {
            pCurrSpawnthing->cgd.flags |= T_GLOBALFADEOUT;
        } else {
            pCurrSpawnthing->cgd.flags |= T_GLOBALFADEIN;
        }
    }

    pCurrSpawnthing->cgd.scalemin = cg_te_scalemin->value;
    pCurrSpawnthing->cgd.scalemax = cg_te_scalemax->value;
    if (pCurrSpawnthing->cgd.scalemin || pCurrSpawnthing->cgd.scalemax) {
        pCurrSpawnthing->cgd.flags |= T_RANDSCALE;
    } else {
        pCurrSpawnthing->cgd.flags &= ~T_RANDSCALE;
    }

    if (cg_te_dietouch->integer) {
        pCurrSpawnthing->cgd.flags |= T_DIETOUCH;
    } else {
        pCurrSpawnthing->cgd.flags &= ~T_DIETOUCH;
    }

    if (cg_te_fade->integer) {
        pCurrSpawnthing->cgd.flags |= T_FADE;
    } else {
        pCurrSpawnthing->cgd.flags &= ~T_FADE;
    }

    if (cg_te_fadedelay->integer > 0) {
        pCurrSpawnthing->cgd.flags |= T_FADE;
        pCurrSpawnthing->cgd.fadedelay = cg_te_fadedelay->value * 1000.0;
    }

    if (cg_te_fadein->integer > 0) {
        pCurrSpawnthing->cgd.flags |= T_FADEIN;
        pCurrSpawnthing->cgd.fadeintime = cg_te_fadein->value * 1000.0;
    } else {
        pCurrSpawnthing->cgd.flags &= ~T_FADEIN;
    }

    if (cg_te_circle->integer) {
        pCurrSpawnthing->cgd.flags |= T_CIRCLE;
    } else {
        pCurrSpawnthing->cgd.flags &= ~T_CIRCLE;
    }

    if (cg_te_sphere->integer) {
        pCurrSpawnthing->cgd.flags |= T_SPHERE;
    } else {
        pCurrSpawnthing->cgd.flags &= ~T_SPHERE;
    }

    if (cg_te_insphere->integer) {
        pCurrSpawnthing->cgd.flags |= T_INWARDSPHERE;
    } else {
        pCurrSpawnthing->cgd.flags &= ~T_INWARDSPHERE;
    }

    if (cg_te_align->integer) {
        pCurrSpawnthing->cgd.flags |= T_ALIGN;
    } else {
        pCurrSpawnthing->cgd.flags &= ~T_ALIGN;
    }

    if (cg_te_alignstretch_scale->integer) {
        pCurrSpawnthing->cgd.flags |= T_ALIGN;
        pCurrSpawnthing->cgd.flags2 |= T2_ALIGNSTRETCH;
        pCurrSpawnthing->cgd.scale2 = cg_te_alignstretch_scale->value;
    } else {
        pCurrSpawnthing->cgd.flags2 &= ~T2_ALIGNSTRETCH;
    }

    if (cg_te_flickeralpha->integer) {
        pCurrSpawnthing->cgd.flags |= T_FLICKERALPHA;
    } else {
        pCurrSpawnthing->cgd.flags &= ~T_FLICKERALPHA;
    }

    if (cg_te_randomroll->integer) {
        pCurrSpawnthing->cgd.flags |= T_RANDOMROLL;
    } else {
        pCurrSpawnthing->cgd.flags &= ~T_RANDOMROLL;
    }

    if (cg_te_volumetric->integer) {
        pCurrSpawnthing->cgd.flags2 |= T2_VOLUMETRIC;
    } else {
        pCurrSpawnthing->cgd.flags2 &= ~T2_VOLUMETRIC;
    }

    if (cg_te_collision->integer == 2) {
        pCurrSpawnthing->cgd.flags |= T_COLLISION;
        pCurrSpawnthing->cgd.collisionmask = CONTENTS_TRIGGER | CONTENTS_FENCE | CONTENTS_WATER | CONTENTS_SOLID;
    } else if (cg_te_collision->integer == 1 || (pCurrSpawnthing->cgd.flags & T_DIETOUCH)) {
        pCurrSpawnthing->cgd.flags &= ~T_COLLISION;
        pCurrSpawnthing->cgd.collisionmask = CONTENTS_TRIGGER | CONTENTS_FENCE | CONTENTS_SOLID;
    } else {
        pCurrSpawnthing->cgd.flags &= ~T_COLLISION;
        pCurrSpawnthing->cgd.collisionmask = 0;
    }

    if (cg_te_swarm_freq->integer > 0 && cg_te_swarm_maxspeed->value > 0 && cg_te_swarm_delta->value > 0) {
        pCurrSpawnthing->cgd.swarmfreq     = cg_te_swarm_freq->integer;
        pCurrSpawnthing->cgd.swarmmaxspeed = cg_te_swarm_maxspeed->value;
        pCurrSpawnthing->cgd.swarmdelta    = cg_te_swarm_delta->value;
        pCurrSpawnthing->cgd.flags |= T_SWARM;
        pCurrSpawnthing->cgd.flags &= ~T_SPHERE;
    }

    if (pCurrCommand) {
        Event *pEvent;
        int    argcount;

        pCurrCommand->fCommandTime = cg_te_command_time->value;
        pCurrCommand->endfcn       = &ClientGameCommandManager::TestEffectEndFunc;

        if (pCurrCommand->pEvent) {
            delete pCurrCommand->pEvent;
            pCurrCommand->pEvent = NULL;
        }

        if (Q_stricmp(cg_te_singlelinecommand->string, "")) {
            cgi.Cmd_TokenizeString(cg_te_singlelinecommand->string);

            pEvent = new Event(cgi.Argv(0));

            argcount = cgi.Argc();
            for (i = 1; i < argcount; i++) {
                pEvent->AddToken(cgi.Argv(i));
            }

            pCurrCommand->pEvent = pEvent;
        }
    }
}

void CG_UpdateTestEmitter(void)
{
    int            count;
    Vector         vAng;
    Vector         delta;
    Vector         end, end2;
    Vector         axis[3];
    emittertime_t *et = NULL;

    if (VectorCompare(te_vEmitterOrigin, vec_zero) || te_iNumCommands < 1) {
        if (Q_stricmp(cg_te_mode_name->string, "Temp Emitter Off")) {
            cgi.Cvar_Set("cg_te_mode_name", "Temp Emitter Off");
        }
        return;
    }

    cgi.Cvar_Set("cg_te_currCommand", va("%i", te_iCurrCommand + 1));
    cgi.Cvar_Set("cg_te_numCommands", va("%i", te_iNumCommands));

    switch (cg_te_mode->integer) {
    case TE_MODE_MODEL_ANIM:
        cgi.Cvar_Set("cg_te_mode_name", "Model Anim Mode");
        break;
    case TE_MODE_MODEL_INIT:
        cgi.Cvar_Set("cg_te_mode_name", "Model Init Mode");
        break;
    case TE_MODE_SFX:
        cgi.Cvar_Set("cg_te_mode_name", "SFX Mode");
        break;
    default:
        cgi.Cvar_Set("cg_te_mode_name", "Emitter Mode");
        break;
    }

    vAng[0] = cg_te_xangles->value;
    vAng[1] = cg_te_yangles->value;
    vAng[2] = cg_te_zangles->value;
    AnglesToAxis(vAng, (vec3_t *)axis);

    end = te_vEmitterOrigin + axis[0] * 50;
    cgi.R_DebugLine(te_vEmitterOrigin, end, 1.0, 0.0, 0.0, 1.0);

    end2 = end + axis[1] * 16 + axis[0] * -16;
    cgi.R_DebugLine(end, end2, 1.0, 0.0, 0.0, 1.0);

    end2 = end + axis[1] * -16 + axis[0] * -16;
    cgi.R_DebugLine(end, end2, 1.0, 0.0, 0.0, 1.0);

    if (cg_te_emittermodel->string[0]) {
        //
        // Initialize the entity
        //

        memset(&te_refEnt, 0, sizeof(te_refEnt));
        te_refEnt.parentEntity  = ENTITYNUM_NONE;
        te_refEnt.scale         = 1.0;
        te_refEnt.shaderRGBA[0] = -1;
        te_refEnt.shaderRGBA[1] = -1;
        te_refEnt.shaderRGBA[2] = -1;
        te_refEnt.shaderRGBA[3] = -1;

        //
        // Setup transforms
        //

        AxisCopy((const vec3_t *)axis, te_refEnt.axis);
        VectorCopy(te_vEmitterOrigin, te_refEnt.origin);
        VectorCopy(te_vEmitterOrigin, te_refEnt.lightingOrigin);

        //
        // Setup the model
        //

        te_refEnt.radius = 4.0;
        te_refEnt.hModel = cgi.R_RegisterModel(cg_te_emittermodel->string);
        te_refEnt.tiki   = cgi.R_Model_GetHandle(te_refEnt.hModel);

        //
        // Setup the shader
        //

        if (!Q_stricmp(cg_te_emittermodel->string, "*beam")) {
            te_refEnt.reType       = RT_BEAM;
            te_refEnt.customShader = cgi.R_RegisterShader("beamshader");
        } else if (strstr(cg_te_emittermodel->string, ".spr")) {
            te_refEnt.reType = RT_SPRITE;
        } else {
            te_refEnt.reType = RT_MODEL;
        }

        //
        // Setup the animation
        //

        te_refEnt.frameInfo[0].index = cgi.Anim_NumForName(te_refEnt.tiki, "idle");
        ;
        if (te_refEnt.frameInfo[0].index < 0) {
            te_refEnt.frameInfo[0].index = 0;
        }
        te_refEnt.frameInfo[0].weight = 1.0;
        te_refEnt.frameInfo[0].time   = 0.0;
        te_refEnt.actionWeight        = 1.0;
        te_refEnt.entityNumber        = ENTITYNUM_NONE;

        if (te_refEnt.reType == RT_SPRITE) {
            cgi.R_AddRefSpriteToScene(&te_refEnt);
        } else {
            cgi.R_AddRefEntityToScene(&te_refEnt, ENTITYNUM_NONE);
        }
    } else {
        te_refEnt.scale = 0.0;
    }

    cg_te_currCommand = cgi.Cvar_Get("cg_te_currCommand", "0", 0);
    cg_te_numCommands = cgi.Cvar_Get("cg_te_numCommands", "0", 0);

    if (cg_te_mode->integer != TE_MODE_EMITTER) {
        return;
    }

    pCurrCommand = pTesteffect->m_commands[te_iCurrCommand];
    if (!pCurrCommand) {
        return;
    }

    pCurrSpawnthing = pCurrCommand->emitter;
    if (!pCurrSpawnthing) {
        return;
    }

    CG_SetTestEmitterValues();

    et = pCurrSpawnthing->GetEmitTime(1);
    if (!et->active) {
        return;
    }

    pCurrSpawnthing->cgd.createTime = cg.time;

    if (et->last_emit_time > 0 && pCurrSpawnthing->spawnRate != 0) {
        float dtime;
        float lerp;
        float lerpfrac;

        dtime = cg.time - et->last_emit_time;
        count = dtime / pCurrSpawnthing->spawnRate;
        if (!count) {
            return;
        }

        if (count == 1) {
            commandManager.SpawnEffect(1, pCurrSpawnthing);
            et->last_emit_time = cg.time;
        } else {
            lerpfrac = 1.0 / count;
            for (lerp = 0.0;; lerp = lerp + lerpfrac) {
                if (dtime <= pCurrSpawnthing->spawnRate) {
                    break;
                }

                et->last_emit_time = cg.time;
                dtime              = dtime - pCurrSpawnthing->spawnRate;
                if (et->lerp_emitter) {
                    pCurrSpawnthing->cgd.origin = et->oldorigin + delta * lerp;
                }

                commandManager.SpawnEffect(1, pCurrSpawnthing);
            }
        }
    } else {
        et->last_emit_time = cg.time;
    }

    pCurrCommand    = NULL;
    pCurrSpawnthing = NULL;
}

void CG_SortEffectCommands()
{
    int j, k;

    for (j = 1; j < pTesteffect->m_iCommandCount; j++) {
        for (k = 0; k < j; k++) {
            if (pTesteffect->m_commands[k] > pTesteffect->m_commands[k + 1]) {
                specialeffectcommand_t *command = pTesteffect->m_commands[k];

                pTesteffect->m_commands[k]     = pTesteffect->m_commands[k + 1];
                pTesteffect->m_commands[k + 1] = command;
            }
        }
    }
}

void CG_TriggerTestEmitter_f(void)
{
    int    i;
    vec3_t axis[3];

    if (!cg_te_mode->integer) {
        return;
    }

    if (!te_iNumCommands) {
        return;
    }

    CG_SaveEffectCommandCvars(te_iCurrCommand);

    for (i = 0; i < te_iNumCommands; ++i) {
        pCurrCommand = pTesteffect->m_commands[i];
        if (!pCurrCommand) {
            return;
        }

        pCurrSpawnthing = pCurrCommand->emitter;
        if (!pCurrSpawnthing) {
            return;
        }

        CG_GetEffectCommandCvars(i);
        CG_SetTestEmitterValues();
    }

    CG_GetEffectCommandCvars(te_iCurrCommand);
    CG_SortEffectCommands();

    pCurrCommand    = pTesteffect->m_commands[0];
    pCurrSpawnthing = pCurrCommand->emitter;
    AxisCopy(pCurrSpawnthing->axis, axis);

    // Spawn the effect
    sfxManager.MakeEffect_Axis(SFX_TEST_EFFECT, pCurrSpawnthing->cgd.origin, axis);

    pCurrCommand    = NULL;
    pCurrSpawnthing = NULL;
}

void CG_DumpBaseAndAmplitude(str *buff, const char *prefix, Vector *base, Vector *amplitude)
{
    int i;

    if ((*amplitude)[0] || (*amplitude)[1] || (*amplitude)[2]) {
        *buff += prefix;

        for (i = 0; i < 3; i++) {
            if (!(*amplitude)[i]) {
                *buff += va(" %g", (*base)[i]);
            } else if (!(*base)[i]) {
                *buff += va(" random %g", (*amplitude)[i]);
            } else if (-(*base)[i] == (*base)[i] + (*amplitude)[i]) {
                *buff += va(" crandom %g", (*amplitude)[i] * 0.5);
            } else {
                *buff += va(" range %g %g", (*base)[i], (*amplitude)[i]);
            }
        }

        *buff += "\n";
    } else if ((*base)[0] || (*base)[1] || (*base)[2]) {
        *buff += prefix;
        *buff += va(" %g %g %g\n", (*base)[0], (*base)[1], (*base)[2]);
    }
}

void CG_DumpEmitter_f(void)
{
    cvar_t *filename;
    str     buff;
    str     sTabbing;
    str     old_buff;
    int     iCurrCommand;
    int     iTagNum;

    if (te_iNumCommands < 1) {
        return;
    }

    filename = cgi.Cvar_Get("cg_te_filename", "dump/emitter.txt", 0);
    switch (cg_te_mode->integer) {
    case TE_MODE_MODEL_ANIM:
        sTabbing = "\t\t\t";
        break;
    case TE_MODE_MODEL_INIT:
        sTabbing = "\t\t";
        break;
    case TE_MODE_SFX:
        sTabbing = "\t\t";
        break;
    default:
        sTabbing = "\t\t";
        break;
    }
    CG_SaveEffectCommandCvars(te_iCurrCommand);

    if (cg_te_mode->integer == TE_MODE_EMITTER) {
        iCurrCommand = te_iCurrCommand;
    } else {
        iCurrCommand = 0;
    }

    do {
        pCurrCommand = pTesteffect->m_commands[iCurrCommand];
        if (!pCurrCommand) {
            return;
        }

        pCurrSpawnthing = pCurrCommand->emitter;
        if (!pCurrSpawnthing) {
            return;
        }

        CG_GetEffectCommandCvars(iCurrCommand);
        CG_SetTestEmitterValues();

        if (te_refEnt.scale > 0 && Q_stricmp(cg_te_tag->string, "") && cg_te_mode->integer != TE_MODE_SFX) {
            iTagNum = cgi.Tag_NumForName(te_refEnt.tiki, cg_te_tag->string);
        } else {
            iTagNum = -1;
        }

        switch (cg_te_mode->integer) {
        case TE_MODE_MODEL_ANIM:
            buff += va("%s0", sTabbing.c_str());

            if (pCurrCommand->fCommandTime > 0) {
                buff += va("commanddelay %g ", pCurrCommand->fCommandTime);
            }

            if (iTagNum != -1) {
                buff += va("tagspawn %s\n", cg_te_tag->string);
            } else {
                buff += "originspawn\n";
            }
            break;
        case TE_MODE_MODEL_INIT:
            buff += sTabbing;

            if (pCurrCommand->fCommandTime > 0) {
                buff += va("commanddelay %g ", pCurrCommand->fCommandTime);
            }

            if (iTagNum != -1) {
                buff += va("tagspawn %s\n", cg_te_tag->string);
            } else {
                buff += "originspawn\n";
            }
            break;
        case TE_MODE_SFX:
            buff += sTabbing;

            if (pCurrCommand->fCommandTime > 0) {
                buff += va("delayedsfx %.3g originspawn\n", pCurrCommand->fCommandTime);
            } else {
                buff += "sfx originspawn\n";
            }
            break;
        default:
            if (iTagNum != -1) {
                buff += va("%stagemitter %s name%i\n", sTabbing.c_str(), cg_te_tag->string, iCurrCommand + 1);
            } else {
                buff += va("%soriginemitter name%i\n", sTabbing.c_str(), iCurrCommand + 1);
            }
            break;
        }

        buff += sTabbing + "(\n";

        if (pCurrSpawnthing->fMinRangeSquared > 0) {
            buff +=
                va("%s\tspawnrange %i %i\n",
                   sTabbing.c_str(),
                   (int)sqrt(pCurrSpawnthing->fMinRangeSquared),
                   (int)sqrt(pCurrSpawnthing->fMaxRangeSquared));
        } else if (pCurrSpawnthing->fMaxRangeSquared > 0) {
            buff += va("%s\tspawnrange %i\n", sTabbing.c_str(), (int)sqrt(pCurrSpawnthing->fMaxRangeSquared));
        }

        if (pCurrSpawnthing->cgd.flags2 & T2_VOLUMETRIC) {
            buff += sTabbing + "\tvolumetric\n";
        }

        buff += va("%s\tspawnrate %g\n", sTabbing.c_str(), cg_te_spawnrate->value);
        buff += va("%s\tmodel %s\n", sTabbing.c_str(), pCurrSpawnthing->m_modellist.ObjectAt(1).c_str());

        if (!(pCurrSpawnthing->cgd.flags2 & T2_VOLUMETRIC) && pCurrSpawnthing->count > 1) {
            buff += va("%s\tcount %i\n", sTabbing.c_str(), pCurrSpawnthing->count);
        }

        if (pCurrSpawnthing->cgd.alpha != 1.0) {
            buff += va("%s\talpha %g\n", sTabbing.c_str(), pCurrSpawnthing->cgd.alpha);
        }

        buff +=
            va("%s\tcolor %g %g %g\n",
               sTabbing.c_str(),
               pCurrSpawnthing->cgd.color[0],
               pCurrSpawnthing->cgd.color[1],
               pCurrSpawnthing->cgd.color[2]);

        if (!(pCurrSpawnthing->cgd.flags2 & T2_VOLUMETRIC)) {
            if (pCurrSpawnthing->cgd.flags2 & T2_VARYCOLOR) {
                buff += sTabbing + "\tvarycolor\n";
            }
            if (pCurrSpawnthing->cgd.flags2 & T2_SPRITEGRIDLIGHTING) {
                buff += sTabbing + "\tspritegridlighting\n";
            }
        }

        if (pCurrSpawnthing->cgd.flags & T_COLLISION) {
            if (pCurrSpawnthing->cgd.collisionmask & CONTENTS_WATER) {
                buff += sTabbing + "\tcollision water\n";
            } else {
                buff += sTabbing + "\tcollision\n";
            }
        }

        if (!(pCurrSpawnthing->cgd.flags2 & T2_VOLUMETRIC) && (pCurrSpawnthing->cgd.flags & T_COLLISION)) {
            buff += va("%s\tbouncefactor %g\n", sTabbing.c_str(), pCurrSpawnthing->cgd.bouncefactor);
        }

        if (pCurrSpawnthing->cgd.scale != 1.0) {
            buff += va("%s\tscale %g\n", sTabbing.c_str(), pCurrSpawnthing->cgd.scale);
        }

        buff += va("%s\tlife %g\n", sTabbing.c_str(), cg_te_life->value);

        if (!(pCurrSpawnthing->cgd.flags2 & T2_VOLUMETRIC) && pCurrSpawnthing->cgd.scaleRate) {
            buff + va("%s\tscalerate %g\n", sTabbing.c_str(), pCurrSpawnthing->cgd.scaleRate);
        }

        if (pCurrSpawnthing->sphereRadius) {
            if (cg_te_cone_height->value) {
                buff += va(
                    "%s\tcone %g %g\n", sTabbing.c_str(), pCurrSpawnthing->coneHeight, pCurrSpawnthing->sphereRadius
                );
            } else {
                buff += va("%s\tradius %g\n", sTabbing.c_str(), pCurrSpawnthing->sphereRadius);
            }
        }

        if (pCurrSpawnthing->forwardVelocity) {
            buff += va("%s\tvelocity %g\n", sTabbing.c_str(), pCurrSpawnthing->forwardVelocity);
        }

        if (pCurrSpawnthing->cgd.flags2 & T2_RADIALVELOCITY) {
            buff +=
                va("%s\tradialvelocity %g %g %g\n",
                   sTabbing.c_str(),
                   pCurrSpawnthing->cgd.velocity[0],
                   pCurrSpawnthing->cgd.velocity[1],
                   pCurrSpawnthing->cgd.velocity[2]);
        }

        if (pCurrSpawnthing->cgd.flags & T_RANDVELAXIS) {
            CG_DumpBaseAndAmplitude(
                &buff,
                va("%s\trandvelaxis", sTabbing.c_str()),
                &pCurrSpawnthing->randvel_base,
                &pCurrSpawnthing->randvel_amplitude
            );
        } else {
            CG_DumpBaseAndAmplitude(
                &buff,
                va("%s\trandvel", sTabbing.c_str()),
                &pCurrSpawnthing->randvel_base,
                &pCurrSpawnthing->randvel_amplitude
            );
        }

        if (pCurrSpawnthing->cgd.flags2 & T2_CLAMP_VEL) {
            buff +=
                va("%s\tclampvel %g %g %g %g %g %g\n",
                   sTabbing.c_str(),
                   pCurrSpawnthing->cgd.minVel[0],
                   pCurrSpawnthing->cgd.maxVel[0],
                   pCurrSpawnthing->cgd.minVel[1],
                   pCurrSpawnthing->cgd.maxVel[1],
                   pCurrSpawnthing->cgd.minVel[2],
                   pCurrSpawnthing->cgd.maxVel[2]);
        } else if (pCurrSpawnthing->cgd.flags2 & T2_CLAMP_VEL_AXIS) {
            buff +=
                va("%s\tclampvelaxis %g %g %g %g %g %g\n",
                   sTabbing.c_str(),
                   pCurrSpawnthing->cgd.minVel[0],
                   pCurrSpawnthing->cgd.maxVel[0],
                   pCurrSpawnthing->cgd.minVel[1],
                   pCurrSpawnthing->cgd.maxVel[1],
                   pCurrSpawnthing->cgd.minVel[2],
                   pCurrSpawnthing->cgd.maxVel[2]);
        }

        if (pCurrSpawnthing->cgd.flags2 & T2_VOLUMETRIC) {
            if (pCurrSpawnthing->cgd.accel[0] || pCurrSpawnthing->cgd.accel[1] || pCurrSpawnthing->cgd.accel[2]) {
                buff +=
                    va("%s\tsmokeparms %g %g %g\n",
                       sTabbing.c_str(),
                       pCurrSpawnthing->cgd.accel[0],
                       pCurrSpawnthing->cgd.accel[1],
                       pCurrSpawnthing->cgd.accel[2]);
            }
        } else {
            if (pCurrSpawnthing->cgd.accel[0] || pCurrSpawnthing->cgd.accel[1] || pCurrSpawnthing->cgd.accel[2]) {
                buff +=
                    va("%s\taccel %g %g %g\n",
                       sTabbing.c_str(),
                       pCurrSpawnthing->cgd.accel[0],
                       pCurrSpawnthing->cgd.accel[1],
                       pCurrSpawnthing->cgd.accel[2]);
            }
        }

        if (pCurrSpawnthing->cgd.flags2 & T2_FRICTION) {
            buff += va("%s\tfriction %g\n", sTabbing.c_str(), pCurrSpawnthing->cgd.friction);
        }

        if (pCurrSpawnthing->cgd.flags2 & T2_SPIN) {
            buff += va("%s\tfriction %g\n", sTabbing.c_str(), pCurrSpawnthing->cgd.spin_rotation);
        }

        CG_DumpBaseAndAmplitude(
            &buff,
            va("%s\toffset", sTabbing.c_str()),
            &pCurrSpawnthing->origin_offset_base,
            &pCurrSpawnthing->origin_offset_amplitude
        );
        CG_DumpBaseAndAmplitude(
            &buff,
            va("%s\toffsetalongaxis", sTabbing.c_str()),
            &pCurrSpawnthing->axis_offset_base,
            &pCurrSpawnthing->axis_offset_amplitude
        );

        if (!(pCurrSpawnthing->cgd.flags2 & T2_VOLUMETRIC) && (pCurrSpawnthing->cgd.flags & T_ANGLES)) {
            CG_DumpBaseAndAmplitude(
                &buff,
                va("%s\tangles", sTabbing.c_str()),
                &pCurrSpawnthing->cgd.angles,
                &pCurrSpawnthing->angles_amplitude
            );
        }

        if (pCurrSpawnthing->cgd.flags2 & T2_AMOVE) {
            CG_DumpBaseAndAmplitude(
                &buff,
                va("%s\tavelocity", sTabbing.c_str()),
                &pCurrSpawnthing->avelocity_base,
                &pCurrSpawnthing->avelocity_amplitude
            );
        }

        if (pCurrSpawnthing->cgd.scalemin) {
            buff += va("%s\tscalemin %g\n", sTabbing.c_str(), pCurrSpawnthing->cgd.scalemin);
        }
        if (pCurrSpawnthing->cgd.scalemax) {
            buff += va("%s\tscalemax %g\n", sTabbing.c_str(), pCurrSpawnthing->cgd.scalemax);
        }

        if (pCurrSpawnthing->cgd.flags & T_DIETOUCH) {
            buff += sTabbing + "\tdietouch\n";
        }

        if (!(pCurrSpawnthing->cgd.flags2 & T2_VOLUMETRIC)) {
            if (pCurrSpawnthing->cgd.fadedelay > 0) {
                buff += va("%s\tfadedelay %g\n", sTabbing.c_str(), pCurrSpawnthing->cgd.fadedelay / 1000.0);
            } else if (pCurrSpawnthing->cgd.flags & T_FADE) {
                buff += sTabbing + "\tfade\n";
            }

            if (pCurrSpawnthing->cgd.flags & T_FADEIN) {
                buff += va("%s\tfadein %g\n", sTabbing.c_str(), pCurrSpawnthing->cgd.fadeintime / 1000.0);
            }
        }

        if (pCurrSpawnthing->cgd.flags & T_CIRCLE) {
            buff += sTabbing + "\tcircle\n";
        }
        if (pCurrSpawnthing->cgd.flags & T_SPHERE) {
            buff += sTabbing + "\tsphere\n";
        }
        if (pCurrSpawnthing->cgd.flags & T_INWARDSPHERE) {
            buff += sTabbing + "\tinwardsphere\n";
        }

        if (!(pCurrSpawnthing->cgd.flags2 & T2_VOLUMETRIC)) {
            if (pCurrSpawnthing->cgd.flags2 & T2_ALIGNSTRETCH) {
                buff += va("%s\talignstretch %g", sTabbing.c_str(), pCurrSpawnthing->cgd.scale2);
            } else if (pCurrSpawnthing->cgd.flags & T_ALIGN) {
                buff += sTabbing + "\talign\n";
            }

            if (pCurrSpawnthing->cgd.flags & T_FLICKERALPHA) {
                buff += sTabbing + "\tflickeralpha\n";
            }
        }

        if (pCurrSpawnthing->cgd.flags & T_RANDOMROLL) {
            buff += sTabbing + "\trandomroll\n";
        }

        if (!(pCurrSpawnthing->cgd.flags2 & T2_VOLUMETRIC) && (pCurrSpawnthing->cgd.flags & T_SWARM)) {
            buff +=
                va("%s\tswarm %i %g %g\n",
                   sTabbing.c_str(),
                   pCurrSpawnthing->cgd.swarmfreq,
                   pCurrSpawnthing->cgd.swarmmaxspeed,
                   pCurrSpawnthing->cgd.swarmdelta);
        }

        buff += sTabbing + ")\n\n";

        iCurrCommand++;
    } while (iCurrCommand < te_iNumCommands && cg_te_mode->integer != TE_MODE_EMITTER);

    char *buffer;
    str   szTmp;
    int   iLen;

    iLen = cgi.FS_ReadFile(filename->string, (void **)&buffer, qfalse);
    if (iLen != -1) {
        char FBuf[512];

        COM_StripExtension(filename->string, FBuf, sizeof(FBuf));
        szTmp = FBuf;
        szTmp += ".old";

        cgi.FS_WriteFile(szTmp.c_str(), buffer, iLen);
    }

    cgi.FS_WriteTextFile(filename->string, buff.c_str(), buff.length());

    pCurrSpawnthing = NULL;

    CG_GetEffectCommandCvars(te_iCurrCommand);
}

void CG_LoadBaseAndAmplitude(
    char      **pBufer,
    const char *szCvarX,
    const char *szCvarY,
    const char *szCvarZ,
    const char *szCvarXAmp,
    const char *szCvarYAmp,
    const char *szCvarZAmp
)
{
    char com_token[MAX_TOKEN_CHARS];

    const char *szCvarList[]    = {szCvarX, szCvarY, szCvarZ};
    const char *szCvarAmpList[] = {szCvarXAmp, szCvarYAmp, szCvarZAmp};
    int         i;

    // Fixed in OPM
    //  Use a loop rather than a copy-paste

    for (i = 0; i < 3; i++) {
        Q_strncpyz(com_token, COM_ParseExt(pBufer, qfalse), sizeof(com_token));
        if (!com_token[0]) {
            return;
        }

        if (!Q_stricmp(com_token, "random")) {
            Q_strncpyz(com_token, COM_ParseExt(pBufer, qfalse), sizeof(com_token));
            if (!com_token[0]) {
                return;
            }

            cgi.Cvar_Set(szCvarAmpList[i], com_token);
        } else if (!Q_stricmp(com_token, "crandom")) {
            float fTmp;

            Q_strncpyz(com_token, COM_ParseExt(pBufer, qfalse), sizeof(com_token));
            if (!com_token[0]) {
                return;
            }

            fTmp = atof(com_token);
            cgi.Cvar_Set(szCvarList[i], va("%g", -fTmp));
            cgi.Cvar_Set(szCvarAmpList[i], va("%g", fTmp * 2.0));
        } else if (!Q_stricmp(com_token, "range")) {
            Q_strncpyz(com_token, COM_ParseExt(pBufer, qfalse), sizeof(com_token));
            if (!com_token[0]) {
                return;
            }
            cgi.Cvar_Set(szCvarList[i], com_token);

            Q_strncpyz(com_token, COM_ParseExt(pBufer, qfalse), sizeof(com_token));
            if (!com_token[0]) {
                return;
            }
            cgi.Cvar_Set(szCvarAmpList[i], com_token);
        }
    }
}

void CG_LoadEmitter_f(void)
{
    int     iCommandCount;
    char   *buffer;
    char   *bufstart;
    char    com_token[MAX_TOKEN_CHARS];
    cvar_t *filename;

    if (te_iNumCommands <= 0) {
        CG_TestEmitter_f();
    } else {
        CG_SaveEffectCommandCvars(te_iCurrCommand);
    }
    filename = cgi.Cvar_Get("cg_te_filename", "dump/emitter.txt", 0);

    if (cgi.FS_ReadFile(filename->string, (void **)&buffer, qfalse) == -1) {
        return;
    }

    Com_Printf("Loading emitter dump '%s'\n", filename->string);
    bufstart      = buffer;
    iCommandCount = 0;

    while ((Q_strncpyz(com_token, COM_ParseExt(&buffer, qtrue), sizeof(com_token))), com_token[0]) {
        iCommandCount++;
        if (te_iNumCommands < iCommandCount) {
            CG_NewEmitterCommand_f();
        }

        te_iCurrCommand = iCommandCount - 1;
        pCurrCommand    = pTesteffect->m_commands[iCommandCount - 1];
        if (!pCurrCommand) {
            Com_Printf("Error loading effect dump\n");
            break;
        }

        pCurrSpawnthing = pCurrCommand->emitter;
        commandManager.InitializeSpawnthing(pCurrSpawnthing);
        pCurrCommand->fCommandTime = 0.0;
        CG_ResetEffectCommandCvars();

        if (com_token[0] == '0') {
            cgi.Cvar_Set("cg_te_mode", va("%i", 1));
            Q_strncpyz(com_token, COM_ParseExt(&buffer, qtrue), sizeof(com_token));
            if (!com_token[0]) {
                Com_Printf("Error loading effect dump\n");
                break;
            }
        } else if (!Q_stricmp(com_token, "delayedsfx") || !Q_stricmp(com_token, "sfx")) {
            cgi.Cvar_Set("cg_te_mode", va("%i", 3));
            if (!Q_stricmp(com_token, "delayedsfx")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (!com_token[0]) {
                    Com_Printf("Error loading effect dump\n");
                    break;
                }
                cgi.Cvar_Set("cg_te_command_time", com_token);

                Q_strncpyz(com_token, COM_ParseExt(&buffer, qtrue), sizeof(com_token));
                if (!com_token[0]) {
                    Com_Printf("Error loading effect dump\n");
                    break;
                }
            }
        } else if (!Q_stricmp(com_token, "tagemitter") || !Q_stricmp(com_token, "originemitter")) {
            cgi.Cvar_Set("cg_te_mode", va("%i", 0));
        } else {
            cgi.Cvar_Set("cg_te_mode", va("%i", 2));
        }

        if (!Q_stricmp(com_token, "commanddelay")) {
            Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
            if (!com_token[0]) {
                Com_Printf("Error loading effect dump\n");
                break;
            }
            cgi.Cvar_Set("cg_te_command_time", com_token);

            Q_strncpyz(com_token, COM_ParseExt(&buffer, qtrue), sizeof(com_token));
            if (!com_token[0]) {
                Com_Printf("Error loading effect dump\n");
                break;
            }
        }

        if (!Q_stricmpn(com_token, "tag", 3)) {
            Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
            if (!com_token[0]) {
                Com_Printf("Error loading effect dump\n");
                break;
            }
            cgi.Cvar_Set("cg_te_tag", com_token);
        }

        while (COM_ParseExt(&buffer, qfalse)) {}

        Q_strncpyz(com_token, COM_ParseExt(&buffer, qtrue), sizeof(com_token));
        if (Q_stricmp(com_token, "(")) {
            Com_Printf("Error loading effect dump\n");
            break;
        }

        Q_strncpyz(com_token, COM_ParseExt(&buffer, qtrue), sizeof(com_token));
        if (!com_token[0]) {
            Com_Printf("Error loading effect dump\n");
            break;
        }

        while (Q_stricmp(com_token, ")")) {
            if (!Q_stricmp(com_token, "spawnrange")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_spawnrange_a", com_token);

                    Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                    if (com_token[0]) {
                        cgi.Cvar_Set("cg_te_spawnrange_b", com_token);
                    }
                }
            } else if (!Q_stricmp(com_token, "volumetric")) {
                cgi.Cvar_Set("cg_te_volumetric", "1");
            } else if (!Q_stricmp(com_token, "spawnrate")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_spawnrate", com_token);
                }
            } else if (!Q_stricmp(com_token, "model")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_model", com_token);
                }
            } else if (!Q_stricmp(com_token, "count")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_count", com_token);
                }
            } else if (!Q_stricmp(com_token, "alpha")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_alpha", com_token);
                }
            } else if (!Q_stricmp(com_token, "color")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_color_r", com_token);

                    Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                    if (com_token[0]) {
                        cgi.Cvar_Set("cg_te_color_g", com_token);

                        Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                        if (com_token[0]) {
                            cgi.Cvar_Set("cg_te_color_b", com_token);
                        }
                    }
                }
            } else if (!Q_stricmp(com_token, "varycolor")) {
                cgi.Cvar_Set("cg_te_varycolor", "1");
            } else if (!Q_stricmp(com_token, "spritegridlighting")) {
                cgi.Cvar_Set("cg_te_spritegridlighting", "1");
            } else if (!Q_stricmp(com_token, "collision")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_collision", "1");
                } else {
                    cgi.Cvar_Set("cg_te_collision", "1");
                }
            } else if (!Q_stricmp(com_token, "bouncefactor")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_alpha", com_token);
                }
            } else if (!Q_stricmp(com_token, "scale")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_alpha", com_token);
                }
            } else if (!Q_stricmp(com_token, "life")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_life", com_token);
                }
            } else if (!Q_stricmp(com_token, "scalerate")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_scalerate", com_token);
                }
            } else if (!Q_stricmp(com_token, "radius")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_radius", com_token);
                }
            } else if (!Q_stricmp(com_token, "cone")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_cone_height", com_token);

                    Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                    if (com_token[0]) {
                        cgi.Cvar_Set("cg_te_radius", com_token);
                    }
                }
            } else if (!Q_stricmp(com_token, "velocity")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_forwardvel", com_token);
                }
            } else if (!Q_stricmp(com_token, "radialvelocity")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_radial_scale", com_token);

                    Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                    if (com_token[0]) {
                        cgi.Cvar_Set("cg_te_radial_min", com_token);

                        Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                        if (com_token[0]) {
                            cgi.Cvar_Set("cg_te_radial_max", com_token);
                        }
                    }
                }
            } else if (!Q_stricmp(com_token, "randvelaxis") || !Q_stricmp(com_token, "randvel")) {
                if (!Q_stricmp(com_token, "randvelaxis")) {
                    cgi.Cvar_Set("cg_te_randaxis", "1");

                    CG_LoadBaseAndAmplitude(
                        &buffer,
                        "cg_te_randvelbase_x",
                        "cg_te_randvelbase_y",
                        "cg_te_randvelbase_z",
                        "cg_te_randvelamp_x",
                        "cg_te_randvelamp_y",
                        "cg_te_randvelamp_z"
                    );
                }
            } else if (!Q_stricmp(com_token, "clampvel") || !Q_stricmp(com_token, "clampvelaxis")) {
                if (!Q_stricmp(com_token, "clampvelaxis")) {
                    cgi.Cvar_Set("cg_te_clampvelaxis", "1");
                }

                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_clampvelmin_x", com_token);

                    Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                    if (com_token[0]) {
                        cgi.Cvar_Set("cg_te_clampvelmax_x", com_token);

                        Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                        if (com_token[0]) {
                            cgi.Cvar_Set("cg_te_clampvelmin_y", com_token);

                            Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                            if (com_token[0]) {
                                cgi.Cvar_Set("cg_te_clampvelmax_y", com_token);

                                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                                if (com_token[0]) {
                                    cgi.Cvar_Set("cg_te_clampvelmin_z", com_token);

                                    Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                                    if (com_token[0]) {
                                        cgi.Cvar_Set("cg_te_clampvelmax_z", com_token);
                                    }
                                }
                            }
                        }
                    }
                }
            } else if (!Q_stricmp(com_token, "accel") || !Q_stricmp(com_token, "smokeparms")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_accel_x", com_token);

                    Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                    if (com_token[0]) {
                        cgi.Cvar_Set("cg_te_accel_y", com_token);

                        Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                        if (com_token[0]) {
                            cgi.Cvar_Set("cg_te_accel_z", com_token);
                        }
                    }
                }
            } else if (!Q_stricmp(com_token, "friction")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_friction", com_token);
                }
                // Added in 2.0
                //  spin
            } else if (!Q_stricmp(com_token, "spin")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_spin", com_token);
                }
            } else if (!Q_stricmp(com_token, "offset")) {
                CG_LoadBaseAndAmplitude(
                    &buffer,
                    "cg_te_offsbase_x",
                    "cg_te_offsbase_y",
                    "cg_te_offsbase_z",
                    "cg_te_offsamp_x",
                    "cg_te_offsamp_y",
                    "cg_te_offsamp_z"
                );
            } else if (!Q_stricmp(com_token, "offsetalongaxis")) {
                CG_LoadBaseAndAmplitude(
                    &buffer,
                    "cg_te_axisoffsbase_x",
                    "cg_te_axisoffsbase_y",
                    "cg_te_axisoffsbase_z",
                    "cg_te_axisoffsamp_x",
                    "cg_te_axisoffsamp_y",
                    "cg_te_axisoffsamp_z"
                );
            } else if (!Q_stricmp(com_token, "angles")) {
                CG_LoadBaseAndAmplitude(
                    &buffer,
                    "cg_te_anglesbase_p",
                    "cg_te_anglesbase_y",
                    "cg_te_anglesbase_r",
                    "cg_te_anglesamp_p",
                    "cg_te_anglesamp_y",
                    "cg_te_anglesamp_r"
                );
            } else if (!Q_stricmp(com_token, "avelocity")) {
                CG_LoadBaseAndAmplitude(
                    &buffer,
                    "cg_te_avelbase_p",
                    "cg_te_avelbase_y",
                    "cg_te_avelbase_r",
                    "cg_te_avelamp_p",
                    "cg_te_avelamp_y",
                    "cg_te_avelamp_r"
                );
            } else if (!Q_stricmp(com_token, "scalemin")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_scalemin", com_token);
                }
            } else if (!Q_stricmp(com_token, "scalemax")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_scalemax", com_token);
                }
            } else if (!Q_stricmp(com_token, "dietouch")) {
                cgi.Cvar_Set("cg_te_dietouch", "1");
            } else if (!Q_stricmp(com_token, "fade")) {
                cgi.Cvar_Set("cg_te_fade", "1");
            } else if (!Q_stricmp(com_token, "fadedelay")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_fadedelay", com_token);
                }
            } else if (!Q_stricmp(com_token, "fadein")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_fadein", com_token);
                }
            } else if (!Q_stricmp(com_token, "circle")) {
                cgi.Cvar_Set("cg_te_circle", "1");
            } else if (!Q_stricmp(com_token, "sphere")) {
                cgi.Cvar_Set("cg_te_sphere", "1");
            } else if (!Q_stricmp(com_token, "inwardsphere")) {
                cgi.Cvar_Set("cg_te_insphere", "1");
            } else if (!Q_stricmp(com_token, "align")) {
                cgi.Cvar_Set("cg_te_align", "1");
            } else if (!Q_stricmp(com_token, "alignstretch")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_alignstretch_scale", com_token);
                }
            } else if (!Q_stricmp(com_token, "flickeralpha")) {
                cgi.Cvar_Set("cg_te_flickeralpha", "1");
            } else if (!Q_stricmp(com_token, "randomroll")) {
                cgi.Cvar_Set("cg_te_randomroll", "1");
            } else if (!Q_stricmp(com_token, "swarm")) {
                Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                if (com_token[0]) {
                    cgi.Cvar_Set("cg_te_swarm_freq", com_token);

                    Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                    if (com_token[0]) {
                        cgi.Cvar_Set("cg_te_swarm_maxspeed", com_token);

                        Q_strncpyz(com_token, COM_ParseExt(&buffer, qfalse), sizeof(com_token));
                        if (com_token[0]) {
                            cgi.Cvar_Set("cg_te_swarm_delta", com_token);
                        }
                    }
                }
            }
        }

        CG_SaveEffectCommandCvars(te_iCurrCommand);
    }

    cgi.FS_FreeFile(bufstart);
    return;
}

void CG_TestEmitter_f(void)
{
    vec3_t angles;

    if (!pTesteffect->m_iCommandCount) {
        pCurrCommand = pTesteffect->AddNewCommand();
        if (!pCurrCommand) {
            return;
        }

        pCurrSpawnthing       = new spawnthing_t();
        pCurrCommand->emitter = pCurrSpawnthing;
        commandManager.InitializeSpawnthing(pCurrSpawnthing);

        te_iNumCommands++;
    }

    assert(pCurrSpawnthing);

    VectorMA(cg.refdef.vieworg, 100.0, cg.refdef.viewaxis[0], te_vEmitterOrigin);
    VectorSet(angles, 0, cg.refdefViewAngles[1], 0);
    AnglesToAxis(angles, pCurrSpawnthing->axis);

    pCurrSpawnthing->cgd.tiki = NULL;
    CG_SetTestEmitterValues();

    pCurrCommand    = NULL;
    pCurrSpawnthing = NULL;
}

void CG_PrevEmitterCommand_f(void)
{
    CG_SaveEffectCommandCvars(te_iCurrCommand);

    te_iCurrCommand--;
    if (te_iCurrCommand < 0) {
        te_iCurrCommand = te_iNumCommands - 1;
    }

    CG_GetEffectCommandCvars(te_iCurrCommand);
}

void CG_NextEmitterCommand_f(void)
{
    CG_SaveEffectCommandCvars(te_iCurrCommand);

    te_iCurrCommand++;
    if (te_iCurrCommand >= te_iNumCommands) {
        te_iCurrCommand = 0;
    }

    CG_GetEffectCommandCvars(te_iCurrCommand);
}

void CG_NewEmitterCommand_f(void)
{
    if (te_iNumCommands >= MAX_TESTEMITTERS_SAVE) {
        Com_Printf("Test effect can not have more than %i effect commands\n", MAX_TESTEMITTERS_SAVE);
        return;
    }

    pCurrCommand = pTesteffect->AddNewCommand();
    if (!pCurrCommand) {
        return;
    }

    pCurrSpawnthing       = new spawnthing_t();
    pCurrCommand->emitter = pCurrSpawnthing;
    commandManager.InitializeSpawnthing(pCurrSpawnthing);

    te_iNumCommands++;
    CG_SaveEffectCommandCvars(te_iCurrCommand);

    te_iCurrCommand = te_iNumCommands - 1;
    CG_GetEffectCommandCvars(te_iCurrCommand);

    Com_Printf("Test effect now has %i effect commands\n", te_iNumCommands);
}

void CG_DeleteEmitterCommand_f(void) {}

void CG_InitTestEmitter(void)
{
    cg_te_life               = cgi.Cvar_Get("cg_te_life", "1", 0);
    cg_te_alpha              = cgi.Cvar_Get("cg_te_alpha", "1", 0);
    cg_te_dietouch           = cgi.Cvar_Get("cg_te_dietouch", "0", 0);
    cg_te_bouncefactor       = cgi.Cvar_Get("cg_te_bouncefactor", "0.3", 0);
    cg_te_scale              = cgi.Cvar_Get("cg_te_scale", "1", 0);
    cg_te_scalemin           = cgi.Cvar_Get("cg_te_scalemin", "0", 0);
    cg_te_scalemax           = cgi.Cvar_Get("cg_te_scalemax", "0", 0);
    cg_te_model              = cgi.Cvar_Get("cg_te_model", "none", 0);
    cg_te_life               = cgi.Cvar_Get("cg_te_life", "1", 0);
    cg_te_color_r            = cgi.Cvar_Get("cg_te_color_r", "1", 0);
    cg_te_color_g            = cgi.Cvar_Get("cg_te_color_g", "1", 0);
    cg_te_color_b            = cgi.Cvar_Get("cg_te_color_b", "1", 0);
    cg_te_accel_x            = cgi.Cvar_Get("cg_te_accel_x", "0", 0);
    cg_te_accel_y            = cgi.Cvar_Get("cg_te_accel_y", "0", 0);
    cg_te_accel_z            = cgi.Cvar_Get("cg_te_accel_z", "0", 0);
    cg_te_count              = cgi.Cvar_Get("cg_te_count", "1", 0);
    cg_te_fade               = cgi.Cvar_Get("cg_te_fade", "0", 0);
    cg_te_fadedelay          = cgi.Cvar_Get("cg_te_fadedelay", "0", 0);
    cg_te_fadein             = cgi.Cvar_Get("cg_te_fadein", "0", 0);
    cg_te_spawnrate          = cgi.Cvar_Get("cg_te_spawnrate", "1", 0);
    cg_te_scalerate          = cgi.Cvar_Get("cg_te_scalerate", "0", 0);
    cg_te_circle             = cgi.Cvar_Get("cg_te_circle", "0", 0);
    cg_te_sphere             = cgi.Cvar_Get("cg_te_sphere", "0", 0);
    cg_te_insphere           = cgi.Cvar_Get("cg_te_insphere", "0", 0);
    cg_te_radius             = cgi.Cvar_Get("cg_te_radius", "0", 0);
    cg_te_align              = cgi.Cvar_Get("cg_te_align", "0", 0);
    cg_te_flickeralpha       = cgi.Cvar_Get("cg_te_flickeralpha", "0", 0);
    cg_te_collision          = cgi.Cvar_Get("cg_te_collision", "0", 0);
    cg_te_randomroll         = cgi.Cvar_Get("cg_te_randomroll", "0", 0);
    cg_te_offsbase_x         = cgi.Cvar_Get("cg_te_offsbase_x", "0", 0);
    cg_te_offsbase_y         = cgi.Cvar_Get("cg_te_offsbase_y", "0", 0);
    cg_te_offsbase_z         = cgi.Cvar_Get("cg_te_offsbase_z", "0", 0);
    cg_te_offsamp_x          = cgi.Cvar_Get("cg_te_offsamp_x", "0", 0);
    cg_te_offsamp_y          = cgi.Cvar_Get("cg_te_offsamp_y", "0", 0);
    cg_te_offsamp_z          = cgi.Cvar_Get("cg_te_offsamp_z", "0", 0);
    cg_te_anglesbase_p       = cgi.Cvar_Get("cg_te_anglesbase_p", "0", 0);
    cg_te_anglesbase_y       = cgi.Cvar_Get("cg_te_anglesbase_y", "0", 0);
    cg_te_anglesbase_r       = cgi.Cvar_Get("cg_te_anglesbase_r", "0", 0);
    cg_te_anglesamp_p        = cgi.Cvar_Get("cg_te_anglesamp_p", "0", 0);
    cg_te_anglesamp_y        = cgi.Cvar_Get("cg_te_anglesamp_y", "0", 0);
    cg_te_anglesamp_r        = cgi.Cvar_Get("cg_te_anglesamp_r", "0", 0);
    cg_te_randvelbase_x      = cgi.Cvar_Get("cg_te_randvelbase_x", "0", 0);
    cg_te_randvelbase_y      = cgi.Cvar_Get("cg_te_randvelbase_y", "0", 0);
    cg_te_randvelbase_z      = cgi.Cvar_Get("cg_te_randvelbase_z", "0", 0);
    cg_te_randvelamp_x       = cgi.Cvar_Get("cg_te_randvelamp_x", "0", 0);
    cg_te_randvelamp_y       = cgi.Cvar_Get("cg_te_randvelamp_y", "0", 0);
    cg_te_randvelamp_z       = cgi.Cvar_Get("cg_te_randvelamp_z", "0", 0);
    cg_te_clampvelmin_x      = cgi.Cvar_Get("cg_te_clampvelmin_x", "-9999", 0);
    cg_te_clampvelmax_x      = cgi.Cvar_Get("cg_te_clampvelmax_x", "9999", 0);
    cg_te_clampvelmin_y      = cgi.Cvar_Get("cg_te_clampvelmin_y", "-9999", 0);
    cg_te_clampvelmax_y      = cgi.Cvar_Get("cg_te_clampvelmax_y", "9999", 0);
    cg_te_clampvelmin_z      = cgi.Cvar_Get("cg_te_clampvelmin_z", "-9999", 0);
    cg_te_clampvelmax_z      = cgi.Cvar_Get("cg_te_clampvelmax_z", "9999", 0);
    cg_te_clampvelaxis       = cgi.Cvar_Get("cg_te_clampvelaxis", "0", 0);
    cg_te_forwardvel         = cgi.Cvar_Get("cg_te_forwardvel", "0", 0);
    cg_te_volumetric         = cgi.Cvar_Get("cg_te_volumetric", "0", 0);
    cg_te_randaxis           = cgi.Cvar_Get("cg_te_randaxis", "0", 0);
    cg_te_axisoffsbase_x     = cgi.Cvar_Get("cg_te_axisoffsbase_x", "0", 0);
    cg_te_axisoffsbase_y     = cgi.Cvar_Get("cg_te_axisoffsbase_y", "0", 0);
    cg_te_axisoffsbase_z     = cgi.Cvar_Get("cg_te_axisoffsbase_z", "0", 0);
    cg_te_axisoffsamp_x      = cgi.Cvar_Get("cg_te_axisoffsamp_x", "0", 0);
    cg_te_axisoffsamp_y      = cgi.Cvar_Get("cg_te_axisoffsamp_y", "0", 0);
    cg_te_axisoffsamp_z      = cgi.Cvar_Get("cg_te_axisoffsamp_z", "0", 0);
    cg_te_swarm_freq         = cgi.Cvar_Get("cg_te_swarm_freq", "0", 0);
    cg_te_swarm_maxspeed     = cgi.Cvar_Get("cg_te_swarm_maxspeed", "0", 0);
    cg_te_swarm_delta        = cgi.Cvar_Get("cg_te_swarm_delta", "0", 0);
    cg_te_avelbase_p         = cgi.Cvar_Get("cg_te_avelbase_p", "0", 0);
    cg_te_avelbase_y         = cgi.Cvar_Get("cg_te_avelbase_y", "0", 0);
    cg_te_avelbase_r         = cgi.Cvar_Get("cg_te_avelbase_r", "0", 0);
    cg_te_avelamp_p          = cgi.Cvar_Get("cg_te_avelamp_p", "0", 0);
    cg_te_avelamp_y          = cgi.Cvar_Get("cg_te_avelamp_y", "0", 0);
    cg_te_avelamp_r          = cgi.Cvar_Get("cg_te_avelamp_r", "0", 0);
    cg_te_radial_scale       = cgi.Cvar_Get("cg_te_radial_scale", "0", 0);
    cg_te_radial_min         = cgi.Cvar_Get("cg_te_radial_min", "0", 0);
    cg_te_radial_max         = cgi.Cvar_Get("cg_te_radial_max", "0", 0);
    cg_te_friction           = cgi.Cvar_Get("cg_te_friction", "0", 0);
    cg_te_spin               = cgi.Cvar_Get("cg_te_spin", "0", 0); // Added in 2.0
    cg_te_varycolor          = cgi.Cvar_Get("cg_te_varycolor", "0", 0);
    cg_te_spritegridlighting = cgi.Cvar_Get("cg_te_spritegridlighting", "0", 0);
    cg_te_spawnrange_a       = cgi.Cvar_Get("cg_te_spawnrange_a", "0", 0);
    cg_te_spawnrange_b       = cgi.Cvar_Get("cg_te_spawnrange_b", "0", 0);
    cg_te_cone_height        = cgi.Cvar_Get("cg_te_cone_height", "0", 0);
    cg_te_alignstretch_scale = cgi.Cvar_Get("cg_te_alignstretch_scale", "0", 0);
    cg_te_command_time       = cgi.Cvar_Get("cg_te_command_time", "0", 0);
    cg_te_singlelinecommand  = cgi.Cvar_Get("cg_te_singlelinecommand", "", 0);
    cg_te_tag                = cgi.Cvar_Get("cg_te_tag", "", 0);

    for (int i = 0; i < MAX_TESTEMITTERS_SAVE; ++i) {
        CG_SaveEffectCommandCvars(i);
    }

    cg_te_xangles      = cgi.Cvar_Get("cg_te_xangles", "0", 0);
    cg_te_yangles      = cgi.Cvar_Get("cg_te_yangles", "0", 0);
    cg_te_zangles      = cgi.Cvar_Get("cg_te_zangles", "0", 0);
    cg_te_emittermodel = cgi.Cvar_Get("cg_te_emittermodel", "", 0);
    cg_te_mode         = cgi.Cvar_Get("cg_te_mode", "0", 0);
    cg_te_mode_name    = cgi.Cvar_Get("cg_te_mode_name", "Emitter Mode", 0);
    cg_te_currCommand  = cgi.Cvar_Get("cg_te_currCommand", "0", 0);
    cg_te_numCommands  = cgi.Cvar_Get("cg_te_numCommands", "0", 0);

    VectorClear(te_vEmitterOrigin);
    te_refEnt.scale = 0.0;
    te_iNumCommands = 0;
    te_iCurrCommand = 0;

    pTesteffect = sfxManager.GetTestEffectPointer();
}
