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

// lodthing.cpp : Level of detail manager

#include "lodthing.h"
#include "player.h"
#include "scriptexception.h"

Event EV_LODTool_Spawn
(
	"lod_spawn",
	EV_CONSOLE,
	"s",
	"model",
	"Edit LOD on the specified model"
);
Event EV_LODTool_Think
(
	"lod_think",
	EV_DEFAULT,
	NULL,
	NULL,
	"Called every frame to process the view thing."
);
Event EV_LODTool_SetModel
(
	"lod_model",
	EV_DEFAULT,
	NULL,
	NULL,
	"Set the model of the current viewthing"
);
Event EV_LODTool_SetYaw
(
	"lod_yaw",
	EV_CONSOLE,
	"f",
	"yaw",
	"Set the yaw of the model"
);
Event EV_LODTool_SetRoll
(
	"lod_roll",
	EV_CONSOLE,
	"f",
	"roll",
	"Set the roll of the model"
);
Event EV_LODTool_SetAngles
(
	"lod_angles",
	EV_CONSOLE,
	"f[0,360]f[0,360]f[0,360]",
	"pitch yaw roll",
	"Set the angles of the model"
);
Event EV_LODTool_Discard
(
	"lod_discard",
	EV_CONSOLE,
	NULL,
	NULL,
	"Discard LOD changes"
);

LODMaster LODModel;

CLASS_DECLARATION( Listener, LODMaster, "lodmaster" )
{
	{ &EV_LODTool_Spawn,			&LODMaster::Spawn },
	{ &EV_LODTool_SetYaw,			&LODMaster::PassEvent },
	{ &EV_LODTool_SetRoll,			&LODMaster::PassEvent },
	{ &EV_LODTool_SetAngles,		&LODMaster::PassEvent },
	{ &EV_LODTool_Discard,			&LODMaster::PassEvent },
	{ NULL, NULL }
};

void LODMaster::Init
	(
	void
	)
{
	gi.AddCommand( "lod_spawn", NULL );
	gi.AddCommand( "lod_yaw", NULL );
	gi.AddCommand( "lod_pitch", NULL );
	gi.AddCommand( "lod_roll", NULL );
	gi.AddCommand( "lod_angles", NULL );
	gi.AddCommand( "lod_discard", NULL );
}

void LODMaster::Spawn
	(
	Event *ev
	)
{
	const char *mdl;
	Vector forward, up, delta;
	Entity *ent;
	LODSlave *m_lodmodel;

	mdl = ev->GetString( 1 );

	if( !mdl || !*mdl )
	{
		ScriptError( "Must specify a model name" );
	}

	gi.cvar_set( "lod_tikiname", mdl );
	gi.cvar_set( "lod_tool", "1" );

	ent = g_entities->entity;

	if( ent->IsSubclassOfPlayer() )
	{
		( ( Player* )ent )->SetFov( 80 );
		ent->setOriginEvent( Vector( 2000, 0, 0 ) );
	}

	if( m_current )
	{
		m_current->PostEvent( EV_Remove, 0 );
	}

	m_lodmodel = new LODSlave;
	m_current = m_lodmodel;

	ent->angles.AngleVectorsLeft( &forward, NULL, &up );

	m_lodmodel->m_baseorigin = Vector( -1600, 0, 0 );
	m_lodmodel->setOrigin( m_lodmodel->m_baseorigin );
	m_lodmodel->m_baseorigin = m_lodmodel->origin;

	delta = Vector( "1 0 0" );
	delta.toAngles();

	m_lodmodel->setAngles( delta );
	m_lodmodel->setScale( 1.0f );

	Event *event = new Event( EV_LODTool_SetModel );

	event->AddString( mdl );
	m_lodmodel->ProcessEvent( event );

	if( !m_lodmodel->edict->tiki )
	{
		m_current = NULL;

		ScriptError( "Error in model '%s', LOD model not spawned.", mdl );
	}

	for( int i = 0; i < MAX_FRAMEINFOS; i++ )
	{
		m_lodmodel->Pause( i );
	}

	gi.SendConsoleCommand( "cg_drawvidemodel 0\n" );
	gi.SendConsoleCommand( "cg_shadows 0\n" );
	gi.SendConsoleCommand( "ui_compass 0\n" );
	gi.SendConsoleCommand( "r_fastsky 1\n" );
	gi.SendConsoleCommand( "r_drawworld 0\n" );
}

void LODMaster::PassEvent
	(
	Event *ev
	)
{
	if( !m_current )
	{
		ScriptError( "No LOD Model" );
	}

	m_current->ProcessEvent(*ev);
}

CLASS_DECLARATION( Animate, LODSlave, "lodslave" )
{
	{ &EV_LODTool_Think,		&LODSlave::ThinkEvent },
	{ &EV_LODTool_SetModel,		&LODSlave::SetModelEvent },
	{ &EV_LODTool_Discard,		&LODSlave::Delete },
	{ NULL, NULL }
};

void LODSlave::UpdateCvars
	(
	qboolean quiet,
	qboolean updateFrame
	)
{

}

LODSlave::LODSlave
	(
	void
	)
{
	m_baseorigin = origin;
	LODModel.m_current = this;

	edict->s.eType = ET_MODELANIM_SKEL;
	edict->s.renderfx |= RF_SHADOW;
	edict->s.renderfx |= RF_ADDITIVE_DLIGHT;

	PostEvent( EV_LODTool_Think, level.frametime );
}

static float lod_yaw = 0.0f;

void LODSlave::ThinkEvent
	(
	Event *ev
	)
{
	cvar_t *cvar;
	Entity *ent;
	Vector position;

	PostEvent( EV_LODTool_Think, level.frametime );

	lod_yaw += 2.0f;
	if( lod_yaw > 180.0f ) {
		lod_yaw = lod_yaw - 360.0f;
	}

	cvar = gi.Cvar_Get( "lod_pitch_val", "", 0 );

	angles[ 0 ] = cvar->value;
	angles[ 1 ] = lod_yaw;

	setAngles( angles );

	ent = g_entities->entity;
	if( ent->IsSubclassOfPlayer() )
	{
		( ( Player * )ent )->SetFov( 80.0 );
		ent->setOrigin( Vector( -4000, 0, 0 ) );
	}

	cvar = gi.Cvar_Get( "lod_position", "", 0 );
	sscanf( cvar->string, "%f %f %f", &position[ 0 ], &position[ 1 ], &position[ 2 ] );

	cvar = gi.Cvar_Get( "lod_zee_val", "", 0 );
	position[ 2 ] -= cvar->value;

	setOrigin( position );
}

void LODSlave::SetModelEvent
	(
	Event *ev
	)
{
	str modelname = ev->GetString( 1 );

	setModel( modelname );

	if( edict->tiki )
	{
		NewAnim( "idle" );
		SetFrame();
	}
}

void LODSlave::Delete
	(
	Event *ev
	)
{
	ScriptError( "DELETE" );
}
