#ifndef __CPLAYER_H__
#define __CPLAYER_H__

#include "canimate.h"
#include "scripttimer.h"
#include "scriptvariable.h"

// EQ prefix = earthquake

#define ANGLES_EQ			0
#define MAX_DELTAANGLES		4

class ReplicationInfo : public Listener
{
private:
	ScriptVariableList		replicatedvars;

public:
	CLASS_PROTOTYPE( ReplicationInfo );

	void		ReplicateVariable( ScriptVariable& variable );

	void		EventGetVariable( Event *ev );
};

class CPlayer : public Animate
{
private:
	Vector			_targetDeltaAngles[ MAX_DELTAANGLES ];
	Vector			_targetDeltaStart[ MAX_DELTAANGLES ];
	Vector			_targetDeltaTime[ MAX_DELTAANGLES ];
	qboolean		_targetActive[ MAX_DELTAANGLES ][ 3 ];
	qboolean		_godown[ MAX_DELTAANGLES ][ 3 ];

	Vector			_currentTarget[ MAX_DELTAANGLES ];
	Vector			_currentDeltaTime[ MAX_DELTAANGLES ];
	Vector			_glideRatio[ MAX_DELTAANGLES ];
	Vector			_glideTime[ MAX_DELTAANGLES ];
	qboolean		_glidingDown[ MAX_DELTAANGLES ][ 3 ];
	qboolean		_glideUse[ MAX_DELTAANGLES ][ 3 ];

	qboolean		debugCamera;
	qboolean		debugCameraEnd;
	Vector			debugPoint;
	Vector			debugAngles;
	Vector			debugPlayerAngles;

public:
	float			deltaAnglesOld[MAX_DELTAANGLES][3];
	float			deltaAngles[MAX_DELTAANGLES][3];
	Vector			eyesDelta;
	Vector			eyesDeltaOld;
	Vector			vAngles;

	Vector			oldAngles;
	Vector			avel;

	ReplicationInfo	replication;
	float			m_fMoveSpeedScale;

	SafePtr< Entity >	m_ExposureTrigger;
	float				m_fExposure;
	float				m_fExposurePrevious;
	float				m_fExposureStart;
	float				m_fExposureTarget;
	ScriptTimer			m_ExposureTimer;

	playerState_t	*ps;

private:
	void			SmoothProcessDeltaAngles( void );
	void			SmoothProcessDeltaAngle( int port, int index );

public:
	CLASS_PROTOTYPE( CPlayer );

	virtual void Archive( Archiver &arc );

	CPlayer();

	virtual void	Think( void );

	void			DebugCamera( void );
	void			ShakeCamera( void );
	void			SmoothSetDeltaAngles( int port, float time, vec3_t newDeltaAngles, qboolean bMoveVM = false );
	void			SmoothSetDeltaAngle( int port, int index, float time, float angle );

	void			EventGetMovement( Event *ev );
	void			EventGetReplicationInfo( Event *ev );
	void			EventGetSpeed( Event *ev );
	void			EventSetSpeed( Event *ev );
	void			EventViewModelAnim( Event *ev );
};

extern CPlayer player;

#endif
