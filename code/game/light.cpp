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

// light.cpp: Classes for creating and controlling lights.
// 

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "light.h"
#include "scriptmaster.h"

/*****************************************************************************/
/*QUAKED light (0.75 0.5 0) (-8 -8 -8) (8 8 8) LINEAR NO_ENTITIES ENTITY_TRACE

Non-displayed light. If it targets another entity it will become a spot light
if "LINEAR" is set, it will be a linear light
if "NO_ENTITIES" is set, this light will only effect the world, not entities
if "ENTITY_TRACE" is set, a trace is done betwee the light and the entity.\
The light is only added if the trace is clear

"no_entity_light" - this light will not effect entities, just the world
"light" - the intensity of the light, default 300       
"color" - the color of the light
"falloff" - if linear, specify the linear falloff (defaults to 1)
"radius" - make this a spot light of the given radius
"angles" - make this a spot light centered on angles
"spot_angle" - if this is a spot light, what angle to use (default 45)
"entity_trace" - trace between the entity and the light

******************************************************************************/

Event EV_Light_SetLight
	( 
	"light",
	EV_DEFAULT,
   NULL,
   NULL,
   "Set the intensity of the light",
   EV_NORMAL
	);

Event EV_Light_SetColor
	( 
	"color",
	EV_DEFAULT,
   NULL,
   NULL,
   "",
   EV_NORMAL
	);

Event EV_Light_SetFalloff
	( 
	"falloff",
	EV_CODEONLY,
   NULL,
   NULL,
   "",
   EV_NORMAL
	);

Event EV_Light_SetRadius
	( 
	"falloff",
	EV_CODEONLY,
   NULL,
   NULL,
   "",
   EV_NORMAL
	);

Event EV_Light_SpotDir
	( 
	"spot_dir",
	EV_CODEONLY,
   NULL,
   NULL,
   "",
   EV_NORMAL
	);

Event EV_Light_SpotRadiusByDistance
	( 
	"spot_radiusbydistance",
	EV_CODEONLY,
   NULL,
   NULL,
   "",
   EV_NORMAL
	);

Event EV_Light_NoEntityLight
	( 
	"no_entity_light",
	EV_CODEONLY,
   NULL,
   NULL,
   "",
   EV_NORMAL
	);

Event EV_Light_EntityTrace
	( 
	"entity_trace",
	EV_CODEONLY,
   NULL,
   NULL,
   "",
   EV_NORMAL
	);

Event EV_Light_SpotAngle
	( 
	"spot_angle",
	EV_CODEONLY,
   NULL,
   NULL,
   "",
   EV_NORMAL
	);

CLASS_DECLARATION( Entity, Light, "light" )
	{
      { &EV_Light_SetLight, NULL },
		{ NULL, NULL }
	};

Light::Light()
	{
   PostEvent( EV_Remove, 0 );
	}

