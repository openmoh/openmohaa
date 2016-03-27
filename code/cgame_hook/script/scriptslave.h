#ifndef __SCRIPTSLAVE_H__
#define __SCRIPTSLAVE_H__

#include "canimate.h"

#define MF_GLIDE					(1<<0)			// The script slave will glide when moving
#define MF_TIMEDMOVE				(1<<1)			// The script slave will move within the specified time

typedef enum
{
	MOVE_UNSET,
	MOVE_ENTITY,
	MOVE_ORIGIN
} s_movetype;

class ScriptSlave : public Animate
{
private:
	int						moveflags;
	s_movetype				slavetype;
	float					movespeed;
	float					movetime;
	qboolean				movetimedone;
	float					currentTime;
	qboolean				moving;

	union {
		SafePtr<Entity>			*entity;
		float					*origin;
	} target;

	Vector					startOrigin;

public:
	CLASS_PROTOTYPE( ScriptSlave );

	virtual void Archive( Archiver &arc );

	ScriptSlave();
	~ScriptSlave();

	virtual void	Think();

	void			ClearMove();

	float			GetSpeed();
	float			GetTime();

	void			MoveTo( Entity *entity );
	void			MoveTo( Vector origin );
	void			Move();

	void			SetSpeed( float speed );
	void			SetTime( float time );

	void			StopMove();

	s_movetype		getSlaveType();
	void			setSlaveType( s_movetype type );

	void			EventGetSpeed( Event *ev );
	void			EventGetTime( Event *ev );
	void			EventMove( Event *ev );
	void			EventMoveTo( Event *ev );
	void			EventSetSpeed( Event *ev );
	void			EventSetTime( Event *ev );
	void			EventStopMove( Event *ev );
	void			EventWaitMove( Event *ev );
};

#endif /* __SCRIPTSLAVE_H__ */
