/*
* ScriptSlave.cpp
*
* DESCRIPTION : Client-side script slave
*/

#include "scriptslave.h"

Event EV_ScriptSlave_GetSpeed
(
	"speed",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the speed.",
	EV_GETTER
);

Event EV_ScriptSlave_GetTime
(
	"time",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the travel time.",
	EV_GETTER
);

Event EV_ScriptSlave_MoveTo
(
	"moveto",
	EV_DEFAULT,
	"s",
	"vector_or_entity",
	"Move to the specified vector or entity.",
	EV_NORMAL
);

Event EV_ScriptSlave_Move
(
	"move",
	EV_DEFAULT,
	NULL,
	NULL,
	"Move the script slave.",
	EV_NORMAL
);

Event EV_ScriptSlave_SetSpeed
(
	"speed",
	EV_DEFAULT,
	"f",
	"speed",
	"Sets the speed.",
	EV_SETTER
);

Event EV_ScriptSlave_SetTime
(
	"time",
	EV_DEFAULT,
	"f",
	"travel_time",
	"Sets the travel time.",
	EV_SETTER
);

Event EV_ScriptSlave_StopMove
(
	"stopmove",
	EV_DEFAULT,
	NULL,
	NULL,
	"Make the script slave stop moving.",
	EV_NORMAL
);

Event EV_ScriptSlave_WaitMove
(
	"waitmove",
	EV_DEFAULT,
	NULL,
	NULL,
	"Move the script slave and wait until finished.",
	EV_NORMAL
);

void ScriptSlave::Archive( Archiver &arc )
{

}

ScriptSlave::ScriptSlave()
{
	moveflags = 0;
	slavetype = MOVE_UNSET;

	target.origin = NULL;

	movespeed = 200.0f;
	movetime = 1.0f;
	moving = false;
	currentTime = 0.f;

	turnThinkOn();
}

ScriptSlave::~ScriptSlave()
{
	ClearMove();
}

void ScriptSlave::Think()
{
	if( moving )
	{
		float length;
		Vector targetOrigin;

		// Calculate the length
		if( slavetype == MOVE_ENTITY )
		{
			Entity *ent = ( Entity* )target.entity->Pointer();

			if( ent ) {
				targetOrigin = ent->origin;
			} else {
				targetOrigin = vec_zero;
			}
		}
		else
		{
			targetOrigin = target.origin;
		}

		if( movespeed > 0.0f && !movetimedone )
		{
			length = Vector::Distance( origin, targetOrigin );

			movetime = ( length / movespeed ) * 1000.0f;
			movetimedone = true;
		}

		float ratio = currentTime / movetime;
		currentTime += cg->frametime;

		if( ratio >= 1.0f )
		{
			ratio = 1.0f;
			ClearMove();
		}

		setOrigin( startOrigin + ( targetOrigin - startOrigin ) * ratio );
	}

	Animate::Think();
}

void ScriptSlave::ClearMove()
{
	if( slavetype == MOVE_ENTITY )
	{
		if( target.entity ) {
			delete target.entity;
		}
	}
	else if( slavetype == MOVE_ORIGIN )
	{
		if( target.origin ) {
			delete ( Vector * ) target.origin;
		}
	}

	moving = false;
	currentTime = 0.f;
	slavetype = MOVE_UNSET;
}

float ScriptSlave::GetSpeed()
{
	return movespeed;
}

float ScriptSlave::GetTime()
{
	return movetime;
}

void ScriptSlave::MoveTo( Entity *entity )
{
	ClearMove();

	setSlaveType( MOVE_ENTITY );

	target.entity = new SafePtr < Entity > ;

	target.entity->InitSafePtr( entity );
}

void ScriptSlave::MoveTo( Vector origin )
{
	ClearMove();

	setSlaveType( MOVE_ORIGIN );

	target.origin = ( float * ) new Vector( origin );
}

void ScriptSlave::Move()
{
	if( slavetype == MOVE_UNSET ) {
		ScriptError( "No target set for the ScriptSlave !\n" );
	}

	moving = true;
	movetimedone = false;

	startOrigin = origin;
}

void ScriptSlave::SetSpeed( float speed )
{
	if( speed >= 0.f )
	{
		movespeed = speed;
		movetime = 0.0f;
	}
}

void ScriptSlave::SetTime( float time )
{
	if( time >= 0.f )
	{
		movespeed = 0.0f;
		movetime = time * 1000.0f;
	}
}

void ScriptSlave::StopMove()
{
	ClearMove();
}

s_movetype ScriptSlave::getSlaveType()
{
	return slavetype;
}

void ScriptSlave::setSlaveType( s_movetype type )
{
	slavetype = type;
}

//=== SCRIPT ===\\

void ScriptSlave::EventGetSpeed( Event *ev )
{
	ev->AddFloat( GetSpeed() );
}

void ScriptSlave::EventGetTime( Event *ev )
{
	ev->AddFloat( GetTime() );
}

void ScriptSlave::EventMove( Event *ev )
{
	Move();
}

void ScriptSlave::EventMoveTo( Event *ev )
{
	if( ev->IsStringAt( 1 ) || ev->IsVectorAt( 1 ) )
	{
		Vector pos = ev->GetVector( 1 );

		MoveTo( pos );
	}
	else if( ev->IsEntityAt( 1 ) )
	{
		Entity *ent = ev->GetEntity( 1 );

		MoveTo( ent );
	}
}

void ScriptSlave::EventSetSpeed( Event *ev )
{
	SetSpeed( ev->GetFloat( 1 ) );
}

void ScriptSlave::EventSetTime( Event *ev )
{
	SetTime( ev->GetFloat( 1 ) );
}

void ScriptSlave::EventStopMove( Event *ev )
{
	StopMove();
}

void ScriptSlave::EventWaitMove( Event *ev )
{
	Move();
}

CLASS_DECLARATION( CAnimate, ScriptSlave, "client_script_object" )
{
	{ &EV_ScriptSlave_GetSpeed,				&ScriptSlave::EventGetSpeed },
	{ &EV_ScriptSlave_GetTime,				&ScriptSlave::EventGetTime },
	{ &EV_ScriptSlave_Move,					&ScriptSlave::EventMove },
	{ &EV_ScriptSlave_MoveTo,				&ScriptSlave::EventMoveTo },
	{ &EV_ScriptSlave_SetSpeed,				&ScriptSlave::EventSetSpeed },
	{ &EV_ScriptSlave_SetTime,				&ScriptSlave::EventSetTime },
	{ &EV_ScriptSlave_StopMove,				&ScriptSlave::EventStopMove },
	{ &EV_ScriptSlave_WaitMove,				&ScriptSlave::EventWaitMove },
	{ NULL, NULL }
};
