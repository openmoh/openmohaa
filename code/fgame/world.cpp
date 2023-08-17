/*
===========================================================================
Copyright (C) 2015 the OpenMoHAA team

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

// world.cpp : Holds the target list, and general info (fog and such).

#include "g_phys.h"
#include "world.h"
#include "level.h"
#include "scriptmaster.h"
#include "scriptexception.h"
#include "player.h"

#ifdef GAME_DLL
#include "../fgame/soundman.h"
#endif

WorldPtr world;

Event EV_World_MapTime
(
	"map_time",
	EV_DEFAULT,
	"i",
	"version",
	"Internal usage."
);
Event EV_World_SetSoundtrack
(
	"soundtrack",
	EV_DEFAULT,
	"s",
	"MusicFile",
	"Set music soundtrack for this level."
);
Event EV_World_SetGravity
(
	"gravity",
	EV_DEFAULT,
	"f",
	"worldGravity",
	"Set the gravity for the whole world."
);
Event EV_World_SetNextMap
(
	"nextmap",
	EV_DEFAULT,
	"s",
	"nextMap",
	"Set the next map to change to"
);
Event EV_World_SetMessage
(
	"message",
	EV_DEFAULT,
	"s",
	"worldMessage",
	"Set a message for the world"
);
Event EV_World_SetWaterColor
(
	"watercolor",
	EV_DEFAULT,
	"v",
	"waterColor",
	"Set the watercolor screen blend"
);
Event EV_World_SetWaterAlpha
(
	"wateralpha",
	EV_DEFAULT,
	"f",
	"waterAlpha",
	"Set the alpha of the water screen blend"
);
Event EV_World_SetLavaColor
(
	"lavacolor",
	EV_DEFAULT,
	"v",
	"lavaColor",
	"Set the color of lava screen blend"
);
Event EV_World_SetLavaAlpha
(
	"lavaalpha",
	EV_DEFAULT,
	"f",
	"lavaAlpha",
	"Set the alpha of lava screen blend"
);
Event EV_World_GetFarPlane_Color
(
	"farplane_color",
	EV_DEFAULT,
	NULL,
	NULL,
	"Get the color of the far clipping plane fog",
	EV_GETTER
);
Event EV_World_SetFarPlane_Color
(
	"farplane_color",
	EV_DEFAULT,
	"v",
	"farplaneColor",
	"Set the color of the far clipping plane fog",
	EV_SETTER
);
Event EV_World_SetFarPlane_Color2
(
	"farplane_color",
	EV_DEFAULT,
	"v",
	"farplaneColor",
	"Set the color of the far clipping plane fog",
	EV_NORMAL
);
Event EV_World_SetAnimatedFarPlaneColor
(
	"animated_farplane_color",
	EV_DEFAULT,
	"vvff",
	"colorStart colorEnd ZStart ZEnd",
	"Set the color of the far clipping plane based on a color range and a Z range",
	EV_NORMAL
);
Event EV_World_SetFarPlane_Cull
(
	"farplane_cull",
	EV_DEFAULT,
	"i",
	"farplaneCull",
	"Whether or not the far clipping plane should cull things out of the world\n"
	"0 - no cull\n"
	"1 - normal cull\n"
	"2 - cull but no bsp culling"
);
Event EV_World_GetFarPlane
(
	"farplane",
	EV_DEFAULT,
	NULL,
	NULL,
	"Get the distance of the far clipping plane",
	EV_GETTER
);
Event EV_World_SetFarPlane
(
	"farplane",
	EV_DEFAULT,
	"f",
	"farplaneDistance",
	"Set the distance of the far clipping plane",
	EV_SETTER
);
Event EV_World_SetFarClipOverride
(
	"farclipoverride",
	EV_DEFAULT,
	"f",
	"farclipoverride",
	"Override the min config default for the far clip distance",
	EV_NORMAL
);
Event EV_World_SetFarPlaneColorOverride
(
	"farplaneclipcolor",
	EV_DEFAULT,
	"v",
	"farplaneclipcolor",
	"Override the fog color for the min config",
	EV_NORMAL
);
Event EV_World_SetFarPlane2
(
	"farplane",
	EV_DEFAULT,
	"f",
	"farplaneDistance",
	"Set the distance of the far clipping plane",
	EV_NORMAL
);
Event EV_World_SetAnimatedFarPlane
(
	"animated_farplane",
	EV_DEFAULT,
	"ffff",
	"farplaneStart farplaneEnd ZStart ZEnd",
	"Set the distance of the far clipping plane based on a farplane range and a Z range",
	EV_NORMAL
);
Event EV_World_GetFarPlaneBias
(
	"farplane_bias",
	EV_DEFAULT,
	NULL,
	NULL,
	"Get the distance bias of the far clipping plane",
	EV_GETTER
);
Event EV_World_SetFarPlaneBias
(
	"farplane_bias",
	EV_DEFAULT,
	"f",
	"farplaneDistanceBias",
	"Set the distance bias of the far clipping plane",
	EV_SETTER
);
Event EV_World_SetFarPlaneBias2
(
	"farplane_bias",
	EV_DEFAULT,
	"f",
	"farplaneDistanceBias",
	"Set the distance bias of the far clipping plane",
	EV_NORMAL
);
Event EV_World_SetAnimatedFarPlaneBias
(
	"animated_farplane_bias",
	EV_DEFAULT,
	"ffff",
	"biasStart biasEnd ZStart ZEnd",
	"Set the bias of the far clipping plane based on a bias range and a Z range",
	EV_NORMAL
);
Event EV_World_UpdateAnimatedFarplane
(
	"_update_animated_farplane",
	EV_DEFAULT,
	NULL,
	NULL,
	"Updates the farplane if it is being animated.",
	EV_NORMAL
);
Event EV_World_GetSkyboxFarPlane
(
	"skybox_farplane",
	EV_DEFAULT,
	NULL,
	NULL,
	"Get the distance of the skybox far clipping plane",
	EV_GETTER
);
Event EV_World_SetSkyboxFarPlane
(
	"skybox_farplane",
	EV_DEFAULT,
	"f",
	"farplaneDistance",
	"Set the distance of the skybox far clipping plane",
	EV_SETTER
);
Event EV_World_SetSkyboxFarPlane2
(
	"skybox_farplane",
	EV_DEFAULT,
	"f",
	"farplaneDistance",
	"Set the distance of the skybox far clipping plane",
	EV_NORMAL
);
Event EV_World_GetSkyboxSpeed
(
	"skybox_speed",
	EV_DEFAULT,
	NULL,
	NULL,
	"Get the speed of the skybox",
	EV_GETTER
);
Event EV_World_SetSkyboxSpeed
(
	"skybox_speed",
	EV_DEFAULT,
	"f",
	"speed",
	"Set the speed of the skybox",
	EV_SETTER
);
Event EV_World_SetSkyboxSpeed2
(
	"skybox_speed",
	EV_DEFAULT,
	"f",
	"speed",
	"Set the speed of the skybox",
	EV_NORMAL
);
Event EV_World_SetRenderTerrain
(
	"render_terrain",
	EV_DEFAULT,
	"b",
	"render_terrain",
	"Set to draw or not draw terrain",
	EV_NORMAL
);
Event EV_World_GetRenderTerrain
(
	"get_render_terrain",
	EV_DEFAULT,
	NULL,
	NULL,
	"Get the render terrain switch",
	EV_GETTER
);
Event EV_World_SetAmbientLight
(
	"ambientlight",
	EV_DEFAULT,
	"b",
	"ambientLight",
	"Set whether or not ambient light should be used"
);
Event EV_World_SetAmbientIntensity
(
	"ambient",
	EV_DEFAULT,
	"f",
	"ambientIntensity",
	"Set the intensity of the ambient light"
);
Event EV_World_SetSunColor
(
	"suncolor",
	EV_DEFAULT,
	"v",
	"sunColor",
	"Set the color of the sun"
);
Event EV_World_SetSunLight
(
	"sunlight",
	EV_DEFAULT,
	"b",
	"sunlight",
	"Set whether or not there should be sunlight"
);
Event EV_World_SetSunDirection
(
	"sundirection",
	EV_DEFAULT,
	"v",
	"sunlightDirection",
	"Set the direction of the sunlight"
);
Event EV_World_LightmapDensity
(
	"lightmapdensity",
	EV_DEFAULT,
	"f",
	"density",
	"Set the default lightmap density for all world surfaces"
);
Event EV_World_SunFlareName
(
	"sunflarename",
	EV_DEFAULT,
	"s",
	"flarename",
	"Set the flare to use for the sunflare"
);
Event EV_World_SunFlareDirection
(
	"sunflaredirection",
	EV_DEFAULT,
	"v",
	"angles",
	"Set the direction of the sunflare"
);
Event EV_World_SetSkyAlpha
(
	"skyalpha",
	EV_DEFAULT,
	"f",
	"newAlphaForPortalSky",
	"Set the alpha on the sky"
);
Event EV_World_SetSkyPortal
(
	"skyportal",
	EV_DEFAULT,
	"b",
	"newSkyPortalState",
	"Whether or not to use the sky portal at all"
);
Event EV_World_SetNumArenas
(
	"numarenas",
	EV_DEFAULT,
	"i",
	"numarenas",
	"Set the number of arenas in the world",
	EV_NORMAL
);
Event EV_World_SetAIVisionDistance
(
	"ai_visiondistance",
	EV_DEFAULT,
	"f",
	"vision_distance",
	"Sets the default AI Vision Distance",
	EV_NORMAL
);
Event EV_World_SetNorthYaw
(
	"northyaw",
	EV_DEFAULT,
	"f",
	"yaw",
	"Sets the yaw direction that is considered to be north"
);
Event EV_World_SetSunDiffuse
(
	"sundiffuse",
	EV_DEFAULT,
	"f",
	"factor",
	"Sets the fraction of the sunlight to use for diffuse sun"
);

Event EV_World_SetSunDiffuseColor
(
	"sundiffusecolor",
	EV_DEFAULT,
	"v",
	"diffusecolor",
	"Sets an alternate sun color to use for clcing diffuse sun"
);
Event EV_World_Overbright
(
	"overbright",
	EV_DEFAULT,
	"i",
	"use",
	"When set to non-zero, removes the overbright cutoff of lightmaps",
	EV_NORMAL
);
Event EV_World_VisDerived
(
	"vis_derived",
	EV_DEFAULT,
	"i",
	"use",
	"whether or not the vis compiler derives additional vis info from the manual vis",
	EV_NORMAL
);

CLASS_DECLARATION( Entity, World, "worldspawn" )
{
	{ &EV_World_SetSoundtrack,				&World::SetSoundtrack },
	{ &EV_World_SetGravity,					&World::SetGravity },
	{ &EV_World_SetNextMap,					&World::SetNextMap },
	{ &EV_World_SetMessage,					&World::SetMessage },
	{ &EV_World_SetWaterColor,				&World::SetWaterColor },
	{ &EV_World_SetWaterAlpha,				&World::SetWaterAlpha },
	{ &EV_World_SetLavaColor,				&World::SetLavaColor },
	{ &EV_World_SetLavaAlpha,				&World::SetLavaAlpha },
	{ &EV_World_GetFarPlane_Color,			&World::GetFarPlane_Color },
	{ &EV_World_SetFarPlane_Color,			&World::SetFarPlane_Color },
	{ &EV_World_SetFarPlane_Color2,			&World::SetFarPlane_Color },
	{ &EV_World_SetFarPlane_Cull,			&World::SetFarPlane_Cull },
	{ &EV_World_GetFarPlane,				&World::GetFarPlane },
	{ &EV_World_SetFarPlane,				&World::SetFarPlane },
	{ &EV_World_SetFarClipOverride,			&World::SetFarClipOverride },
	{ &EV_World_SetFarPlaneColorOverride,	&World::SetFarPlaneColorOverride },
	{ &EV_World_SetFarPlane2,				&World::SetFarPlane },
	{ &EV_World_GetFarPlaneBias,			&World::GetFarPlaneBias },
	{ &EV_World_SetFarPlaneBias,			&World::SetFarPlaneBias },
	{ &EV_World_SetFarPlaneBias2,			&World::SetFarPlaneBias },
	{ &EV_World_GetSkyboxFarPlane,			&World::GetSkyboxFarplane },
	{ &EV_World_SetSkyboxFarPlane,			&World::SetSkyboxFarplane },
	{ &EV_World_SetSkyboxFarPlane2,			&World::SetSkyboxFarplane },
	{ &EV_World_GetSkyboxSpeed,				&World::GetSkyboxSpeed },
	{ &EV_World_SetSkyboxSpeed,				&World::SetSkyboxSpeed },
	{ &EV_World_SetSkyboxSpeed2,			&World::SetSkyboxSpeed },
	{ &EV_World_GetRenderTerrain,			&World::GetRenderTerrain },
	{ &EV_World_SetRenderTerrain,			&World::SetRenderTerrain },
	{ &EV_World_SetSkyAlpha,				&World::SetSkyAlpha },
	{ &EV_World_SetSkyPortal,				&World::SetSkyPortal },
	{ &EV_World_SetNorthYaw,				&World::SetNorthYaw },
	{ &EV_World_SetAmbientLight,			NULL },
	{ &EV_World_SetAmbientIntensity,		NULL },
	{ &EV_World_SetSunColor,				NULL },
	{ &EV_World_SetSunLight,				NULL },
	{ &EV_World_SetSunDirection,			NULL },
	{ &EV_World_LightmapDensity,			NULL },
	{ &EV_World_SunFlareName,				NULL },
	{ &EV_World_SunFlareDirection,			NULL },
	{ &EV_World_SetNumArenas,				NULL },
	{ &EV_World_SetSunDiffuse,				NULL },
	{ &EV_World_SetSunDiffuseColor,			NULL },
	{ &EV_World_Overbright,					NULL },
	{ &EV_World_VisDerived,					NULL },
	{ &EV_World_SetAIVisionDistance,		&World::SetAIVisionDistance },
	{ &EV_World_SetNorthYaw,				&World::SetNorthYaw },
	{ &EV_World_SetAnimatedFarPlaneColor,	&World::SetAnimatedFarplaneColor },
	{ &EV_World_SetAnimatedFarPlane,		&World::SetAnimatedFarplane },
	{ &EV_World_SetAnimatedFarPlaneBias,	&World::SetAnimatedFarplaneBias },
	{ &EV_World_UpdateAnimatedFarplane,		&World::UpdateAnimatedFarplane },
	{ NULL,									NULL }
};

World::World()
{
	world = this;
	world_dying = qfalse;

	// Anything that modifies configstrings, or spawns things is ignored when loading savegames
	if( LoadingSavegame )
	{
		return;
	}

	assert( entnum == ENTITYNUM_WORLD );

	setMoveType( MOVETYPE_NONE );
	setSolidType( SOLID_BSP );

	// world model is always index 1
	edict->s.modelindex = 1;
	model = "*1";

	UpdateConfigStrings();

	// clear out the soundtrack from the last level
	ChangeSoundtrack( "" );

	// set the default gravity
	gi.cvar_set( "sv_gravity", "800" );

	// set the default farplane parameters
	farplane_distance = 0;
	farplane_bias = 0.0;
	farplane_color = vec_zero;
	farplane_cull = qtrue;
	skybox_farplane = 0.0;
	render_terrain = qtrue;
	skybox_speed = 0.0;
	farclip_override = 0.f;
	farplane_color_override = Vector(-1, -1, -1);
	animated_farplane_start = 0.0;
	animated_farplane_end = 0.0;
	animated_farplane_start_z = 8192.0;
	animated_farplane_end_z = 0.0;
	animated_farplane_bias_start = 0.0;
	animated_farplane_bias_end = 0.0;
	animated_farplane_bias_start_z = 8192.0;
	animated_farplane_bias_end_z = 0.0;
	animated_farplane_color_start = vec_zero;
	animated_farplane_color_end = vec_zero;
	animated_farplane_color_start_z = 8192.0;
	animated_farplane_color_end_z = 0.0;

	UpdateFog();

	sky_alpha = 1.0f;
	sky_portal = qtrue;
	UpdateSky();

	m_fAIVisionDistance = 2048.0f;

	level.cinematic = spawnflags & WORLD_CINEMATIC;

	if( level.cinematic )
		gi.cvar_set( "sv_cinematic", "1" );
	else
		gi.cvar_set( "sv_cinematic", "0" );

	level.nextmap = "";
	level.level_name = level.mapname;

	SoundMan.Load();

	// Set the color for the blends.
	level.water_color = Vector( 0, 0, 1 );
	level.water_alpha = 0.1f;
	level.lava_color = Vector( 1.0f, 0.3f, 0 );
	level.lava_alpha = 0.6f;

	//
	// set the targetname of the world
	//
	SetTargetName( "world" );

	m_fNorth = 0;
}

void World::UpdateConfigStrings( void )
{
	//
	// make some data visible to connecting client
	//
	gi.setConfigstring( CS_GAME_VERSION, GAME_VERSION );
	gi.setConfigstring( CS_LEVEL_START_TIME, va( "%i", level.svsStartTime ) );

	// make some data visible to the server
	gi.setConfigstring( CS_MESSAGE, level.level_name.c_str() );
};

void World::UpdateFog(void)
{
	const char* fogInfoString;
	gi.SetFarPlane(farplane_distance);

	if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
		fogInfoString = va(
			"%d %.0f %.0f %.0f %.6f %.4f %.4f %.4f %d %.0f %.2f %.2f %.2f",
			farplane_cull,
			farplane_distance,
			farplane_bias,
			skybox_farplane,
			skybox_speed,
			farplane_color.x,
			farplane_color.y,
			farplane_color.z,
			render_terrain,
			farclip_override,
			farplane_color_override.x,
			farplane_color_override.y,
			farplane_color_override.z
		);
	} else {
		fogInfoString = va(
			"%d %.0f %.4f %.4f %.4f",
			farplane_cull,
			farplane_distance,
			farplane_color.x,
			farplane_color.y,
			farplane_color.z
		);
	}

	gi.setConfigstring(CS_FOGINFO, fogInfoString);
}

void World::SetFarClipOverride(Event* ev)
{
	farplane_color_override = ev->GetVector(1);
	UpdateFog();
}

void World::SetFarPlaneColorOverride(Event* ev)
{
	farclip_override = ev->GetFloat(1);
	UpdateFog();
}

void World::UpdateSky( void )
{
	gi.SetSkyPortal( sky_portal );
	gi.setConfigstring( CS_SKYINFO, va( "%.4f %d", sky_alpha, sky_portal ) );
}

void World::SetSoundtrack( Event *ev )
{
	const char *text;

	text = ev->GetString( 1 );
	ChangeSoundtrack( text );
}

void World::SetGravity( Event *ev )
{
	gi.cvar_set( "sv_gravity", ev->GetString( 1 ) );
}

void World::GetFarPlane(Event* ev)
{
	ev->AddFloat(farplane_distance);
}

void World::SetFarPlane( Event *ev )
{
	farplane_distance = ev->GetFloat( 1 );
	UpdateFog();
}

void World::GetFarPlaneBias(Event* ev)
{
	ev->AddFloat(farplane_bias);
}

void World::SetFarPlaneBias(Event* ev)
{
	farplane_bias = ev->GetFloat(1);
}

void World::GetFarPlane_Color(Event* ev)
{
	ev->AddVector(farplane_color);
}

void World::SetFarPlane_Color(Event* ev)
{
	farplane_color = ev->GetVector(1);
	UpdateFog();
}

void World::SetFarPlane_Cull( Event *ev )
{
	farplane_cull = ev->GetBoolean( 1 );
	UpdateFog();
}

void World::GetSkyboxFarplane(Event* ev)
{
	ev->AddFloat(skybox_farplane);
}

void World::SetSkyboxFarplane(Event* ev)
{
	skybox_farplane = ev->GetFloat(1);
}

void World::SetAnimatedFarplaneColor(Event* ev)
{

	animated_farplane_color_start = ev->GetVector(1);
	animated_farplane_color_end = ev->GetVector(2);
	animated_farplane_color_start_z = ev->GetFloat(3);
	animated_farplane_color_end_z = ev->GetFloat(4);

	if (animated_farplane_color_start_z > animated_farplane_color_end_z)
	{
		float end_z = animated_farplane_color_end_z;
		animated_farplane_color_end_z = animated_farplane_color_start_z;
		animated_farplane_color_start_z = end_z;

		Vector end = animated_farplane_color_end;
		animated_farplane_color_end = animated_farplane_color_start;
		animated_farplane_color_start = end;
	}

	PostEvent(EV_World_UpdateAnimatedFarplane, 0);
}

void World::SetAnimatedFarplane(Event* ev)
{
	animated_farplane_start = ev->GetFloat(1);
	animated_farplane_end = ev->GetFloat(2);
	animated_farplane_start_z = ev->GetFloat(3);
	animated_farplane_end_z = ev->GetFloat(4);

	if (animated_farplane_start_z > animated_farplane_end_z)
	{
		float end_z = animated_farplane_end_z;
		animated_farplane_end_z = animated_farplane_start_z;
		animated_farplane_start_z = end_z;

		float end = animated_farplane_end;
		animated_farplane_end = animated_farplane_start;
		animated_farplane_start = end;
	}

	PostEvent(EV_World_UpdateAnimatedFarplane, 0);
}

void World::SetAnimatedFarplaneBias(Event* ev)
{
	animated_farplane_bias_start = ev->GetFloat(1);
	animated_farplane_bias_end = ev->GetFloat(2);
	animated_farplane_bias_start_z = ev->GetFloat(3);
	animated_farplane_bias_end_z = ev->GetFloat(4);

	if (animated_farplane_bias_start_z > animated_farplane_bias_end_z)
	{
		float end_z = animated_farplane_bias_end_z;
		animated_farplane_bias_end_z = animated_farplane_bias_start_z;
		animated_farplane_bias_start_z = end_z;

		float end = animated_farplane_end;
		animated_farplane_bias_end = animated_farplane_bias_start;
		animated_farplane_bias_start = end;
	}

	PostEvent(EV_World_UpdateAnimatedFarplane, 0);
}

void World::UpdateAnimatedFarplane(Event* ev)
{
	Entity* player;

	CancelEventsOfType(EV_World_UpdateAnimatedFarplane);

	if (g_gametype->integer != GT_SINGLE_PLAYER) {
		return;
	}

	PostEvent(EV_World_UpdateAnimatedFarplane, level.frametime);

	player = g_entities[0].entity;
	if (!player) {
		return;
	}

	if (animated_farplane_start_z != 8192)
	{
		float alpha;

		if (player->origin.z < animated_farplane_start_z)
		{
			alpha = 0;
		} else if (player->origin.z > animated_farplane_end_z) {
			alpha = 1;
		} else {
			alpha = (player->origin.z - animated_farplane_start_z) / (animated_farplane_end_z - animated_farplane_start_z);
		}

		farplane_distance = alpha * (animated_farplane_end - animated_farplane_start) + animated_farplane_start;
	}

	if (animated_farplane_bias_start_z != 8192)
	{
		float alpha;

		if (player->origin.z < animated_farplane_bias_start_z)
		{
			alpha = 0;
		} else if (player->origin.z > animated_farplane_bias_end_z) {
			alpha = 1;
		} else {
			alpha = (player->origin.z - animated_farplane_bias_start_z) / (animated_farplane_bias_end_z - animated_farplane_bias_start_z);
		}

		farplane_bias = alpha * (animated_farplane_bias_end - animated_farplane_bias_start) + animated_farplane_bias_start;
	}

	if (animated_farplane_color_start_z != 8192)
	{
		float alpha, startAlpha, endAlpha;
		Vector start, end;
		float dot;

		if (player->origin.z < animated_farplane_color_start_z)
		{
			alpha = 0;
		} else if (player->origin.z > animated_farplane_color_end_z) {
			alpha = 1;
		} else {
			alpha = (player->origin.z - animated_farplane_color_start_z) / (animated_farplane_color_end_z - animated_farplane_color_start_z);
		}
		
		start = animated_farplane_color_start;
		end = animated_farplane_color_end;
		VectorNormalize(start);
		VectorNormalize(end);

		dot = Vector::Dot(start, end);
		if (1.f - dot > 0.000001)
		{
			startAlpha = sin((1.f - alpha) * acos(dot)) / sin(acos(dot));
			endAlpha = sin(acos(dot) * alpha) / sin(acos(dot));
		}
		else
		{
			startAlpha = 1.f - alpha;
			endAlpha = alpha;
		}

		farplane_color = end * endAlpha + start * startAlpha;
	}

	UpdateFog();
}

void World::GetRenderTerrain(Event* ev)
{
	ev->AddInteger(render_terrain);
}

void World::SetRenderTerrain(Event* ev)
{
	render_terrain = ev->GetInteger(1);
}

void World::GetSkyboxSpeed(Event* ev)
{
	ev->AddInteger(skybox_speed);
}

void World::SetSkyboxSpeed(Event* ev)
{
	skybox_speed = ev->GetInteger(1);
}

void World::SetSkyAlpha( Event *ev )
{
	sky_alpha = ev->GetFloat( 1 );
	UpdateSky();
}

void World::SetSkyPortal( Event *ev )
{
	sky_portal = ev->GetBoolean( 1 );
	UpdateSky();
}

void World::SetNextMap( Event *ev )
{
	level.nextmap = ev->GetString( 1 );
}

void World::SetMessage( Event *ev )
{
	const char *text;

	text = ev->GetString( 1 );
	level.level_name = text;
	gi.setConfigstring( CS_MESSAGE, text );
}

void World::SetWaterColor( Event *ev )
{
	level.water_color = ev->GetVector( 1 );
}

void World::SetWaterAlpha( Event *ev )
{
	level.water_alpha = ev->GetFloat( 1 );
}
void World::SetLavaColor( Event *ev )
{
	level.lava_color = ev->GetVector( 1 );
}

void World::SetLavaAlpha( Event *ev )
{
	level.lava_alpha = ev->GetFloat( 1 );
}

void World::SetNumArenas(Event* ev)
{
	level.m_numArenas = ev->GetInteger(1);
}

void World::SetAIVisionDistance(Event* ev)
{
	m_fAIVisionDistance = ev->GetFloat(1);
}

void World::SetNorthYaw( Event *ev )
{
	m_fNorth = anglemod( ev->GetFloat( 1 ) );
}

SimpleEntity *World::GetTarget( str targetname, bool quiet )
{
	return GetTarget( Director.AddString( targetname ), quiet );
}

SimpleEntity *World::GetTarget( const_str targetname, bool quiet )
{
	ConSimple* list = GetTargetList( targetname );

	if( list->NumObjects() == 1 )
	{
		return list->ObjectAt( 1 );
	}
	else if( list->NumObjects() > 1 )
	{
		if( !quiet ) {
			warning( "World::GetTarget", "There are %d entities with targetname '%s'. You are using a command that requires exactly one.", list->NumObjects(), Director.GetString( targetname ).c_str() );
		}
	}

	return NULL;
}

SimpleEntity *World::GetScriptTarget( str targetname )
{
	return GetScriptTarget( Director.AddString( targetname ) );
}

SimpleEntity *World::GetScriptTarget( const_str targetname )
{
	ConSimple* list = GetTargetList( targetname );

	if( list->NumObjects() == 1 )
	{
		return list->ObjectAt( 1 );
	}
	else if( list->NumObjects() > 1 )
	{
		ScriptError( "There are %d entities with targetname '%s'. You are using a command that requires exactly one.", list->NumObjects(), Director.GetString( targetname ).c_str() );
	}

	return NULL;
}

ConSimple *World::GetExistingTargetList( const str& targetname )
{
	return GetExistingTargetList( Director.AddString( targetname ) );
}

ConSimple *World::GetExistingTargetList( const_str targetname )
{
	return m_targetList.findKeyValue( targetname );
}

ConSimple *World::GetTargetList( str& targetname )
{
	return GetTargetList( Director.AddString( targetname ) );
}

ConSimple *World::GetTargetList( const_str targetname )
{
	return &m_targetList.addKeyValue( targetname );
}

void World::AddTargetEntity( SimpleEntity *ent )
{
	str targetname = ent->TargetName();

	if( !targetname.length() )
	{
		return;
	}

	ConSimple* list = GetTargetList( targetname );

	list->AddObject( ent );
}

void World::AddTargetEntityAt( SimpleEntity *ent, int index )
{
	str targetname = ent->TargetName();

	if( !targetname.length() )
	{
		return;
	}

	ConSimple* list = GetTargetList( targetname );

	list->AddObjectAt( index, ent );
}

int World::GetTargetnameIndex(SimpleEntity* ent)
{
	ConSimple* list = GetTargetList(ent->TargetName());

	return list->IndexOfObject(ent);
}

void World::RemoveTargetEntity( SimpleEntity *ent )
{
	if (world->world_dying) {
		return;
	}

	const str targetname = ent->TargetName();

	if( !targetname.length() )
	{
		return;
	}

	ConSimple* list = GetExistingTargetList( targetname );

	if( list )
	{
		list->RemoveObject( ent );

		if( list->NumObjects() <= 0 )
		{
			m_targetList.remove( Director.AddString( targetname ) );
		}
	}
}

SimpleEntity *World::GetNextEntity( str targetname, SimpleEntity *ent )
{
	return GetNextEntity( Director.AddString( targetname ), ent );
}

SimpleEntity *World::GetNextEntity( const_str targetname, SimpleEntity *ent )
{
	ConSimple* list = GetTargetList( targetname );
	int index;

	if( ent )
	{
		index = list->IndexOfObject( ent ) + 1;
	}
	else
	{
		index = 1;
	}

	if( list->NumObjects() >= index )
	{
		return list->ObjectAt( index );
	}
	else
	{
		return NULL;
	}
}

void World::FreeTargetList()
{
	m_targetList.clear();
}
