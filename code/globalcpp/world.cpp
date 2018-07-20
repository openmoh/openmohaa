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

#include "world.h"
#include <scriptmaster.h>

#ifdef GAME_DLL
#include "../game/soundman.h"
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
Event EV_World_SetFarPlane_Color
(
	"farplane_color",
	EV_DEFAULT,
	"v",
	"farplaneColor",
	"Set the color of the far clipping plane fog"
);
Event EV_World_SetFarPlane_Cull
(
	"farplane_cull",
	EV_DEFAULT,
	"b",
	"farplaneCull",
	"Whether or not the far clipping plane should cull things out of the world"
);
Event EV_World_SetFarPlane
(
	"farplane",
	EV_DEFAULT,
	"f",
	"farplaneDistance",
	"Set the distance of the far clipping plane"
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
Event EV_World_SunFlare
(
	"sunflare",
	EV_DEFAULT,
	"v",
	"position_of_sun",
	"Set the position of the sun for the purposes of the sunflare"
);
Event EV_World_SunFlareInPortalSky
(
	"sunflare_inportalsky",
	EV_DEFAULT,
	NULL,
	NULL,
	"Let the renderer know that the sun is in the portal sky"
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

void World::RemoveTargetEntity( SimpleEntity *ent )
{
	str targetname = ent->TargetName();

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

void World::FreeTargetList()
{
	m_targetList.clear();
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

int World::GetTargetnameIndex( SimpleEntity *ent )
{
	ConSimple* list = GetTargetList( ent->TargetName() );

	return list->IndexOfObject( ent );
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


#ifdef GAME_DLL

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
	gi.Cvar_Set( "sv_gravity", "800" );

	// set the default farplane parameters
	farplane_distance = 0;
	farplane_color = "0 0 0";
	farplane_cull = qtrue;

	UpdateFog();

	sky_portal = qtrue;
	UpdateSky();

	m_fAIVisionDistance = 2048.0f;

	level.cinematic = spawnflags & WORLD_CINEMATIC;

	if( level.cinematic )
		gi.Cvar_Set( "sv_cinematic", "1" );
	else
		gi.Cvar_Set( "sv_cinematic", "0" );

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
	gi.SetConfigstring( CS_GAME_VERSION, GAME_VERSION );
	gi.SetConfigstring( CS_LEVEL_START_TIME, va( "%i", level.svsStartTime ) );

	// make some data visible to the server
	gi.SetConfigstring( CS_MESSAGE, level.level_name.c_str() );
};

void World::UpdateFog( void )
{
	gi.SetFarPlane( farplane_distance );
	gi.SetConfigstring( CS_FOGINFO, va( "%d %.0f %.4f %.4f %.4f", farplane_cull, farplane_distance, farplane_color.x, farplane_color.y, farplane_color.z ) );
}

void World::UpdateSky( void )
{
	gi.SetSkyPortal( sky_portal );
	gi.SetConfigstring( CS_SKYINFO, va( "%.4f %d", sky_alpha, sky_portal ) );
}


void World::SetSoundtrack( Event *ev )
{
	const char *text;

	text = ev->GetString( 1 );
	ChangeSoundtrack( text );
}

void World::SetGravity( Event *ev )
{
	gi.Cvar_Set( "sv_gravity", ev->GetString( 1 ) );
}

void World::SetFarPlane( Event *ev )
{
	farplane_distance = ev->GetFloat( 1 );
	UpdateFog();
}

void World::SetFarPlane_Color( Event *ev )
{
	farplane_color = ev->GetVector( 1 );
	UpdateFog();
}

void World::SetFarPlane_Cull( Event *ev )
{
	farplane_cull = ev->GetBoolean( 1 );
	UpdateFog();
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
	gi.SetConfigstring( CS_MESSAGE, text );
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

void World::SetNorthYaw( Event *ev )
{
	m_fNorth = anglemod( ev->GetFloat( 1 ) );
}

CLASS_DECLARATION( Entity, World, "worldspawn" )
{
	{ &EV_World_SetSoundtrack,			&World::SetSoundtrack },
	{ &EV_World_SetGravity,				&World::SetGravity },
	{ &EV_World_SetNextMap,				&World::SetNextMap },
	{ &EV_World_SetMessage,				&World::SetMessage },
	{ &EV_World_SetWaterColor,			&World::SetWaterColor },
	{ &EV_World_SetWaterAlpha,			&World::SetWaterAlpha },
	{ &EV_World_SetLavaColor,			&World::SetLavaColor },
	{ &EV_World_SetLavaAlpha,			&World::SetLavaAlpha },
	{ &EV_World_SetFarPlane_Color,		&World::SetFarPlane_Color },
	{ &EV_World_SetFarPlane_Cull,		&World::SetFarPlane_Cull },
	{ &EV_World_SetFarPlane,			&World::SetFarPlane },
	{ &EV_World_SetSkyAlpha,			&World::SetSkyAlpha },
	{ &EV_World_SetSkyPortal,			&World::SetSkyPortal },
	{ &EV_World_SetNorthYaw,			&World::SetNorthYaw },
	{ &EV_World_SetAmbientLight,		NULL },
	{ &EV_World_SetAmbientIntensity,	NULL },
	{ &EV_World_SetSunColor,			NULL },
	{ &EV_World_SetSunLight,			NULL },
	{ &EV_World_SetSunDirection,		NULL },
	{ &EV_World_LightmapDensity,		NULL },
	{ &EV_World_SunFlare,				NULL },
	{ &EV_World_SunFlareInPortalSky,	NULL },
	{ &EV_World_SetSunDiffuse,			NULL },
	{ &EV_World_SetSunDiffuseColor,		NULL },
	{ NULL,								NULL }
};

#else

World::World()
{
	world = this;
	world_dying = qfalse;
}

CLASS_DECLARATION( SimpleEntity, World, "worldspawn" )
{
	{ NULL, NULL }
};

#endif
