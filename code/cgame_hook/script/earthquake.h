#ifndef __EARTHQUAKE_H__
#define __EARTHQUAKE_H__

#include "simpleentity.h"

class Earthquake : public SimpleEntity
{
private:
	float			_duration;
	float			_magnitude;
	qboolean		_no_rampup;
	qboolean		_no_rampdown;
	Vector			_location;
	float			_radius;

	float			_currentMagnitude;
	float			_currentTime;
	float			_startTime;

	qboolean		active;

public:
	CLASS_PROTOTYPE( Earthquake );

	virtual void Archive( Archiver &arc );

	Earthquake();
	Earthquake( float duration, float magnitude, qboolean no_rampup, qboolean no_rampdown, Vector location = Vector( 0, 0, 0 ), float radius = 1.0f );
	virtual ~Earthquake();

	void			Enable( void );
	void			Disable( void );
	qboolean		isActive( void );

	float			getMagnitude( void );
	Vector			getLocation( void );
	float			getRadius( void );

	qboolean		hasLocation( void );

	void			SetDuration( float duration );
	void			SetMagnitude( float magnitude );
	void			SetNoRampup( qboolean no_rampup );
	void			SetNoRampdown( qboolean no_rampdown );
	void			SetLocation( Vector origin );
	void			SetRadius( float radius );

	void			Think( Event *ev );
};

extern Container< Earthquake * > earthquakes;

#endif
