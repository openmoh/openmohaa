#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "listener.h"

class SimpleArchivedEntity;

class Level : public Listener
{
public:
	bool		m_LoopDrop;
	bool		m_LoopProtection;

	str	m_mapscript;
	str	current_map;

	// Level time
	int framenum;
	int inttime;
	int intframetime;

	float time;
	float frametime;

	int		spawnflags;

	// Server time
	int		svsTime;
	float	svsFloatTime;
	int		svsStartTime;
	int		svsEndTime;

	bool	m_bScriptSpawn;
	bool	m_bRejectSpawn;

	Container< SimpleArchivedEntity * > m_SimpleArchivedEntities;

	qboolean		prespawned;
	qboolean		spawned;

public:
	CLASS_PROTOTYPE( Level );

	void			setTime( int _svsTime_ );
	void			setFrametime( int frameTime );

	virtual void Archive( Archiver &arc );

	Level();
	~Level();

	void				TestEvent( Event *ev );
};


extern Level level;

#endif // __LEVEL_H__
