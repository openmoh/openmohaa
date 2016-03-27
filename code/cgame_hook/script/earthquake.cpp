/*
 * Earthquake.cpp
 *
 * DESCRIPTION : Client-side realistic earthquake and network optimizer
 */

#include "earthquake.h"
#include "archive.h"

Event EV_Earthquake_Think
(
	"eq_think",
	EV_CODEONLY,
	NULL,
	NULL,
	"Internal event",
	EV_NORMAL
);

Container< Earthquake * > earthquakes;

Earthquake::Earthquake()
{
	_duration = 1.0f;
	_magnitude = 0.7f;
	_no_rampup = false;
	_no_rampdown = false;

	_location = Vector( 0, 0, 0 );
	_radius = 1000.0f;

	active = false;

	earthquakes.AddObject( this );
}

Earthquake::Earthquake( float duration, float magnitude, qboolean no_rampup, qboolean no_rampdown, Vector location, float radius )
{
	SetDuration( duration );
	SetMagnitude( magnitude );
	SetNoRampup( no_rampup );
	SetNoRampdown( no_rampdown );
	SetLocation( location );
	SetRadius( radius );

	active = false;

	earthquakes.AddObject( this );
}

Earthquake::~Earthquake()
{
	Disable();

	earthquakes.RemoveObject( this );
}

void Earthquake::Archive( Archiver &arc )
{
	SimpleEntity::Archive( arc );

	arc.ArchiveFloat( &_duration );
	arc.ArchiveFloat( &_magnitude );

	arc.ArchiveBoolean( &_no_rampup );
	arc.ArchiveBoolean( &_no_rampdown );

	arc.ArchiveVector( &_location );
	arc.ArchiveFloat( &_radius );

	arc.ArchiveFloat( &_currentMagnitude );
	arc.ArchiveFloat( &_currentTime );

	arc.ArchiveBoolean( &active );

	if( arc.Loading() ) {
		_startTime = cg->time - _currentTime;
	}
}

float Earthquake::getMagnitude()
{
	return _currentMagnitude;
}

Vector Earthquake::getLocation()
{
	return _location;
}

float Earthquake::getRadius()
{
	return _radius;
}

qboolean Earthquake::hasLocation()
{
	return _location != Vector( 0, 0, 0 );
}

void Earthquake::SetDuration( float duration )
{
	_duration = duration * 1000.0f;
}

void Earthquake::SetMagnitude( float magnitude )
{
	_magnitude = magnitude;
}

void Earthquake::SetNoRampup( qboolean no_rampup )
{
	_no_rampup = no_rampup;
}

void Earthquake::SetNoRampdown( qboolean no_rampdown )
{
	_no_rampdown = no_rampdown;
}

void Earthquake::SetLocation( Vector origin )
{
	_location = origin;
}

void Earthquake::SetRadius( float radius )
{
	_radius = radius;
}

void Earthquake::Enable( void )
{
	if ( _duration > 0.0f )
	{
		_startTime = ( float )cg->time;

		_currentMagnitude = _magnitude;
		_currentTime = 0.0f;

		active = true;

		ProcessScriptEvent( EV_Earthquake_Think );
	}
}

void Earthquake::Disable( void )
{
	active = false;

	_startTime = 0.0f;
	_currentMagnitude = 0.0f;
	_currentTime = 0.0f;

	CancelEventsOfType( EV_Earthquake_Think );
}

qboolean Earthquake::isActive( void )
{
	return active;
}

void Earthquake::Think( Event *ev )
{
	float timeDelta;

	if( !active )
	{
		CancelEventsOfType( EV_Earthquake_Think );
		return;
	}

	// Disable the earthquake if the time reached the maximum duration
	if( _currentTime >= _duration )
	{
		delete this;
		return;
	}

	_currentTime += cg->frametime;

	timeDelta = cgi.Milliseconds() - _startTime;

	// we are in the first half of the earthquake
	if ( timeDelta < ( _duration * 0.5f ) )
	{
		if ( !_no_rampup )
		{
			float rampUpTime;

			rampUpTime = _startTime + ( _duration * 0.33f );

			if ( cg->time < rampUpTime )
			{
				float scale;

				scale = ( timeDelta ) / ( _duration * 0.33f );
				_currentMagnitude = _magnitude * scale;
			} else {
				_currentMagnitude = _magnitude;
			}
		}
	}
	// we are in the second half of the earthquake
	else
	{
		if ( !_no_rampdown )
		{
			float rampDownTime;

			rampDownTime = _startTime + ( _duration * 0.66f );

			if ( cg->time > rampDownTime )
			{
				float scale;

				scale = 1.0f - ( ( cg->time - rampDownTime ) / ( _duration * 0.33f ) );
				_currentMagnitude = _magnitude * scale;
			} else {
				_currentMagnitude = _magnitude;
			}
		}
	}

	CancelEventsOfType( EV_Earthquake_Think );
	PostEvent( EV_Earthquake_Think, level.frametime );
}

CLASS_DECLARATION( SimpleEntity, Earthquake, NULL )
{
	{ &EV_Earthquake_Think,		&Earthquake::Think },
	{ NULL, NULL },
};
