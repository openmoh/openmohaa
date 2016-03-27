/*
 * CPlayer.cpp
 *
 * DESCRIPTION : Client-side player
 */

#include "cplayer.h"
#include "earthquake.h"
#include "cgame/cg_viewmodelanim.h"
#include "archive.h"

CPlayer player;

Event EV_ReplicationInfo_GetVariable
(
	"getvar",
	EV_DEFAULT,
	"s",
	"name",
	"Gets a replicated variable",
	EV_RETURN
);

Event EV_Player_GetReplicationInfo
(
	"replicationinfo",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the player's replication info",
	EV_GETTER
);

Event EV_Player_GetMovement
(
	"getmovement",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the player's movement state",
	EV_RETURN
);

Event EV_Player_GetSpeed
(
	"movespeedscale",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the player's speed scale",
	EV_GETTER
);

Event EV_Player_SetSpeed
(
	"movespeedscale",
	EV_DEFAULT,
	"f",
	"speed",
	"Sets the player's speed scale until the next snapshot",
	EV_SETTER
);

Event EV_Player_ViewModelAnim
(
	"viewmodelanim",
	EV_DEFAULT,
	"si",
	"anim force_restart",
	"Sets the player's view model animation",
	EV_NORMAL
);

void ReplicationInfo::ReplicateVariable( ScriptVariable& var )
{
	replicatedvars.SetVariable( var.GetKey(), var );
}

void ReplicationInfo::EventGetVariable( Event *ev )
{
	ScriptVariable *pVar = replicatedvars.GetVariable( ev->GetString( 1 ) );

	if( pVar )
	{
		ev->AddValue( *pVar );
	}
	else
	{
		ev->AddNil();
	}
}

CLASS_DECLARATION( Listener, ReplicationInfo, NULL )
{
	{ &EV_ReplicationInfo_GetVariable,			&ReplicationInfo::EventGetVariable },
	{ NULL, NULL }
};

CPlayer::CPlayer()
{
	for( int i = 0; i < MAX_DELTAANGLES; i++ )
	{
		_targetActive[ i ][ 0 ] = false;
		_targetActive[ i ][ 1 ] = false;
		_targetActive[ i ][ 2 ] = false;

		VectorSet( _currentTarget[ i ], 0, 0, 0 );
		VectorSet( deltaAngles[ i ], 0, 0, 0 );
		VectorSet( deltaAnglesOld[ i ], 0, 0, 0 );
	}

	VectorSet( eyesDelta, 0, 0, 0 );
	VectorSet( eyesDeltaOld, 0, 0, 0 );

	debugCamera = false;
	debugCameraEnd = false;
	debugPoint = Vector( 0, 0, 0 );

	mins = Vector( -16, -16, 0 );
	maxs = Vector( 16, 16, 96 );

	m_fMoveSpeedScale = 1.0f;
	m_fExposure = 0.0f;
	m_fExposurePrevious = 0.0f;

	m_ExposureTimer.setType( TIMER_GLIDE );

	renderFx = RF_INVISIBLE;

	turnThinkOn();
}

void CPlayer::Archive( Archiver &arc )
{
	str s;

	if( this != &player )
	{
		player.Archive( arc );

		delete this;
		return;
	}

	Animate::Archive( arc );

	for( int i = 0; i < MAX_DELTAANGLES; i++ )
	{
		arc.ArchiveVector( &_targetDeltaAngles[ i ] );
		arc.ArchiveVector( &_targetDeltaStart[ i ] );
		arc.ArchiveVector( &_targetDeltaTime[ i ] );

		for( int x = 0; x < 3; x++ )
		{
			arc.ArchiveBoolean( &_targetActive[ i ][ x ] );
			arc.ArchiveBoolean( &_godown[ i ][ x ] );
			arc.ArchiveBoolean( &_glidingDown[ i ][ x ] );
			arc.ArchiveBoolean( &_glideUse[ i ][ x ] );
		}

		arc.ArchiveVector( &_currentTarget[ i ] );
		arc.ArchiveVector( &_currentDeltaTime[ i ] );
		arc.ArchiveVector( &_glideRatio[ i ] );
	}

	arc.ArchiveBoolean( &debugCamera );
	arc.ArchiveBoolean( &debugCameraEnd );

	arc.ArchiveVector( &debugPoint );
	arc.ArchiveVector( &debugAngles );
	arc.ArchiveVector( &debugPlayerAngles );

	int num_prefixes = 19;

	if( !arc.Loading() )
	{
		for( int i = 19; i < 255; i++ )
		{
			if( itemName[i] != NULL && itemPrefix[i] != NULL ) {
				num_prefixes++;
			}
		}
	}

	arc.ArchiveInteger( &num_prefixes );

	if( arc.Saving() )
	{
		for( int i = 19; i < 255; i++ )
		{
			if( itemName[ i ] != NULL && itemPrefix[ i ] != NULL )
			{
				s = itemName[ i ];
				arc.ArchiveString( &s );

				s = itemPrefix[ i ];
				arc.ArchiveString( &s );
			}
		}
	}
	else
	{
		for( int i = 19; i < num_prefixes; i++ )
		{
			arc.ArchiveString( &s );

			itemName[ i ] = ( char * )malloc( s.length() + 1 );
			strcpy( ( char * )itemName[ i ], s.c_str() );

			arc.ArchiveString( &s );

			itemPrefix[ i ] = ( char * )malloc( s.length() + 1 );
			strcpy( ( char * )itemPrefix[ i ], s.c_str() );
		}
	}

	arc.ArchiveVector( ( Vector * )&viewAngles );
	arc.ArchiveVector( &avel );
	arc.ArchiveVector( &oldAngles );
}

void CPlayer::Think()
{
	Vector a;
	Entity *ent;

	cent = &cg_entities[ 0 ];

	if( cg->playerFPSModel != NULL )
	{
		model = cg->playerFPSModel->name;
		modelhandle = cg->playerFPSModelHandle;

		tiki = cg->playerFPSModel;
	}
	else
	{
		model = "";
		modelhandle = 0;
		tiki = NULL;
	}

	MatrixToEulerAngles( cg->refdef.viewAxis, a );

	setOrigin( cg->refdef.viewOrg );

	ShakeCamera();
	SmoothProcessDeltaAngles();

	a[ 0 ] = abs( a[ 0 ] );
	a[ 1 ] = abs( a[ 1 ] );
	a[ 2 ] = abs( a[ 2 ] );

	avel = a - oldAngles;

	oldAngles = a;

	Container< SafePtr< Entity > > entities = s_entities;

	for( int i = entities.NumObjects(); i > 0; i-- )
	{
		ent = entities.ObjectAt( i );

		if( ent && IsTouching( ent ) )
		{
			Event *event = new Event( EV_Touch );
			event->AddEntity( this );

			ent->ProcessEvent( event );
		}
	}

	/*if( m_ExposureTrigger != NULL && !IsTouching( m_ExposureTrigger ) )
	{
		TriggerExposure *trigger = ( TriggerExposure * )m_ExposureTrigger.Pointer();

		m_fExposureStart = m_fExposure;
		m_fExposureTarget = 0.0f;

		m_ExposureTimer.SetTime( trigger->m_fExposureTime );
		m_ExposureTimer.Enable();

		m_ExposureTrigger = NULL;
	}*/
	
	m_fExposure = m_ExposureTimer.LerpValue( m_fExposureStart, m_fExposureTarget );

	Animate::Think();
}

void CPlayer::ShakeCamera()
{
	// Shake the current player's camera
	float earthquakeMagnitude = 0.0f;
	Earthquake *eq;
	float magnitude;

	for( int i = earthquakes.NumObjects(); i > 0; i-- )
	{
		eq = earthquakes.ObjectAt( i );

		if( !eq->isActive() ) {
			continue;
		}

		magnitude = eq->getMagnitude();

		if( eq->hasLocation() )
		{
			Vector location = eq->getLocation();
			float radius = eq->getRadius();

			if( inRadius( location, radius ) )
			{
				Vector org = origin;
				Vector dir = org - location;
				float dist = dir.length();

				magnitude = magnitude - ( magnitude * ( dist / radius ) );
			}
			else
			{
				magnitude = 0.0f;
			}
		}

		earthquakeMagnitude += magnitude;
	}

	if( earthquakeMagnitude <= 0.0f )
	{
		VectorSet( deltaAngles[ ANGLES_EQ ], 0, 0, 0 );
		return;
	}

	magnitude = earthquakeMagnitude / 2.0f;

	if( !_targetActive[ ANGLES_EQ ][ PITCH ] )
	{
		float angle;

		if( !_godown[ ANGLES_EQ ][ PITCH ] )
		{
			angle = 1.85f * magnitude;
			_godown[ ANGLES_EQ ][ PITCH ] = true;
		}
		else
		{
			angle = -1.85f * magnitude;
			_godown[ ANGLES_EQ ][ PITCH ] = false;
		}

		SmoothSetDeltaAngle( ANGLES_EQ, PITCH, 0.105f, angle );
	}

	if( !_targetActive[ ANGLES_EQ ][ YAW ] )
	{
		float angle;

		if( !_godown[ ANGLES_EQ ][ YAW ] )
		{
			angle = 1.75f * magnitude;
			_godown[ ANGLES_EQ ][ YAW ] = true;
		}
		else
		{
			angle = -1.75f * magnitude;
			_godown[ ANGLES_EQ ][ YAW ] = false;
		}

		SmoothSetDeltaAngle( ANGLES_EQ, YAW, 0.060f, angle );
	}

	if( !_targetActive[ ANGLES_EQ ][ ROLL ] )
	{
		float angle;

		if( !_godown[ ANGLES_EQ ][ ROLL ] )
		{
			angle = 1.35f * magnitude;
			_godown[ ANGLES_EQ ][ ROLL ] = true;
		}
		else
		{
			angle = -1.35f * magnitude;
			_godown[ ANGLES_EQ ][ ROLL ] = false;
		}

		SmoothSetDeltaAngle( ANGLES_EQ, ROLL, 0.082f, angle );
	}
}

void CPlayer::SmoothSetDeltaAngles( int port, float time, vec3_t newDeltaAngles, qboolean bMoveVM )
{
	SmoothSetDeltaAngle( port, 0, time, newDeltaAngles[ 0 ] );
	SmoothSetDeltaAngle( port, 1, time, newDeltaAngles[ 1 ] );
	SmoothSetDeltaAngle( port, 2, time, newDeltaAngles[ 2 ] );
}

void CPlayer::SmoothSetDeltaAngle( int port, int index, float time, float angle )
{
	if( index > 2 ) {
		return;
	}

	float realtime = time * 1000.0f + 0.5f;

	_currentDeltaTime[ port ][ index ]	= 0.0f;

	deltaAngles[ port ][ index ]		= _currentTarget[ port ][ index ];

	_glideRatio[ port ][ index ]		= 0.0f;
	_glidingDown[ port ][ index ]		= false;
	_glideTime[ port ][ index ]			= 0.0f;
	_glideUse[ port ][ index ]			= ( ( realtime / 2 ) > cg->frametime ) ? true : false;

	_targetDeltaAngles[ port ][ index ]	= angle;
	_targetDeltaStart[ port ][ index ]	= _currentTarget[ port ][ index ];
	_targetDeltaTime[ port ][ index ]	= realtime;

	_targetActive[ port ][ index ]		= true;
}

void CPlayer::SmoothProcessDeltaAngle( int port, int index )
{
	if( index > 2 || port >= MAX_DELTAANGLES ) {
		return;
	}

	// Process each angle

	if( !_targetActive[ port ][ index ] ) {
		return;
	}

	_glideTime[ port ][ index ] += cg->frametime;

	// Glide the angle by time
	if( _glideUse[ port ][ index ] )
	{
		if( _glidingDown[ port ][ index ] )
		{
			//_glideRatio[ port ][ index ] -= 2.0f/( _targetDeltaTime[ port ][ index ]/2 ) * cg->frametime;
			_glideRatio[ port ][ index ] = 2.0f - 2.0f * _glideTime[ port ][ index ] / ( _targetDeltaTime[ port ][ index ] / 2.0f );

			if( _glideRatio[ port ][ index ] < 0.0f )
			{
				_glideRatio[ port ][ index ] = 0.0f;
				_glidingDown[ port ][ index ] = false;

				_glideTime[ port ][ index ] = 0.0f;
			}
		}
		else
		{
			//_glideRatio[ port ][ index ] += 2.0f/( _targetDeltaTime[ port ][ index ]/2 ) * cg->frametime;
			_glideRatio[ port ][ index ] = 2.0f * _glideTime[ port ][ index ] / ( _targetDeltaTime[ port ][ index ] / 2.0f );

			if( _glideRatio[ port ][ index ] > 2.0f )
			{
				_glideRatio[ port ][ index ] = 2.0f;
				_glidingDown[ port ][ index ] = true;

				_glideTime[ port ][ index ] = 0.0f;
			}
		}

		_currentDeltaTime[ port ][ index ] += cg->frametime * _glideRatio[ port ][ index ];
	} else {
		_currentDeltaTime[ port ][ index ] += cg->frametime;
	}

	float ratio = _currentDeltaTime[ port ][ index ] / _targetDeltaTime[ port ][ index ];

	if( ratio >= 1.0f )
	{
		deltaAngles[ port ][ index ] = _targetDeltaAngles[ port ][ index ];
		_currentTarget[ port ][ index ] = _targetDeltaAngles[ port ][ index ];
		_targetActive[ port ][ index ] = false;

		return;
	}

	deltaAngles[ port ][ index ] = _targetDeltaStart[ port ][ index ] + ( _targetDeltaAngles[ port ][ index ] - _targetDeltaStart[ port ][ index ] ) * ratio;

	_currentTarget[ port ][ index ] = deltaAngles[ port ][ index ];
}

void CPlayer::SmoothProcessDeltaAngles()
{
	for( int i = 0; i < MAX_DELTAANGLES; i++ )
	{
		SmoothProcessDeltaAngle( i, 0 );
		SmoothProcessDeltaAngle( i, 1 );
		SmoothProcessDeltaAngle( i, 2 );
	}
}

void CPlayer::DebugCamera()
{
	if( !cg_debugview->integer )
	{
		//debugCamera = false;

		if( !debugCameraEnd )
		{
			// Restore the player's view angles
			VectorCopy( debugPlayerAngles, viewAngles );

			debugCameraEnd = true;
		}

		return;
	}

	// Initialize debug point
	if( !debugCamera )
	{
		debugCamera = true;

		debugAngles = cg->refdefViewAngles;
		debugPoint = cg->refdef.viewOrg;
		debugPoint.z += cg->predictedPlayerState.viewheight;
	}

	if( debugCameraEnd )
	{
		// Save the player's view angles
		debugPlayerAngles = viewAngles;

		// Restore the debug camera's view angles
		VectorCopy( debugAngles, viewAngles );

		debugCameraEnd = false;
	}

	// Save the camera's view angles
	debugAngles = viewAngles;

	cg->predictedPlayerState.camera_flags |= 8;

	kbutton_t *forward = ( kbutton_t * )0x12F6FE0;
	kbutton_t *backward = ( kbutton_t * )0x12F70C0;
	kbutton_t *moveleft = ( kbutton_t * )0x12F7040;
	kbutton_t *moveright = ( kbutton_t * )0x12F70A0;
	kbutton_t *movedown = ( kbutton_t * )0x012F6FC0;
	kbutton_t *moveup = ( kbutton_t* )0x012F6FA0;

	Vector newAngles = cg->refdefViewAngles;

	//newAngles[ 0 ] = 0.f;

	if( forward->active )
	{
		Vector fwdvec;

		AngleVectors( newAngles, fwdvec, NULL, NULL );

		debugPoint = debugPoint + fwdvec * ( ( float )cg->frametime * 2 );
	}

	if( backward->active )
	{
		Vector backvec;

		AngleVectors( newAngles, backvec, NULL, NULL );

		debugPoint = debugPoint - backvec * ( ( float )cg->frametime * 2 );
	}

	if( moveleft->active )
	{
		Vector leftvec;

		AngleVectors( newAngles, NULL, leftvec, NULL );

		debugPoint = debugPoint + leftvec * ( ( float )cg->frametime * 2 );
	}

	if( moveright->active )
	{
		Vector rightvec;

		AngleVectors( newAngles, NULL, rightvec, NULL );

		debugPoint = debugPoint - rightvec * ( ( float )cg->frametime * 2 );
	}

	if( moveup->active )
	{
		Vector upvec;

		newAngles.x = 0.f;

		AngleVectors( newAngles, NULL, NULL, upvec );

		debugPoint = debugPoint + upvec * ( ( float )cg->frametime );
	}

	if( movedown->active )
	{
		Vector downvec;

		newAngles.x = 0.f;

		AngleVectors( newAngles, NULL, NULL, downvec );

		debugPoint = debugPoint - downvec * ( ( float )cg->frametime );
	}

	cg->refdef.viewOrg[ 0 ] = debugPoint[ 0 ];
	cg->refdef.viewOrg[ 1 ] = debugPoint[ 1 ];
	cg->refdef.viewOrg[ 2 ] = debugPoint[ 2 ];

	VectorCopy( cg->refdef.viewOrg, cg->currentViewPos );
	VectorCopy( cg->refdef.viewOrg, cg->playerHeadPos );
	VectorCopy( cg->refdef.viewOrg, cg->cameraOrigin );
	VectorCopy( cg->refdef.viewOrg, cg->soundOrg );

	VectorCopy( cg->refdef.viewOrg, cg->predictedPlayerState.camera_origin );

	//VectorCopy( cg->refdef.viewOrg, cg->predictedPlayerState.origin );
	//cg->predictedPlayerState.origin[ 0 ] -= cg->predictedPlayerState.viewHeight;
}

// Script

void CPlayer::EventGetMovement( Event *ev )
{
	kbutton_t *forward = ( kbutton_t * )0x12F6FE0;
	kbutton_t *movedown = ( kbutton_t* )0x012F6FC0;
	kbutton_t *moveup = ( kbutton_t* )0x012F6FA0;

	if( forward->active && !moveup->active && !movedown->active )
	{
		ev->AddString( "forward" );
	}
	else if( movedown->active )
	{
		ev->AddString( "crouch" );
	}
	else if( moveup->active )
	{
		ev->AddString( "jump" );
	}
	else
	{
		ev->AddString( "idle" );
	}
}

void CPlayer::EventGetReplicationInfo( Event *ev )
{
	ev->AddListener( &replication );
}

void CPlayer::EventGetSpeed( Event *ev )
{
	ev->AddFloat( m_fMoveSpeedScale );
}

void CPlayer::EventSetSpeed( Event *ev )
{
	m_fMoveSpeedScale = ev->GetFloat( 1 );
}

void CPlayer::EventViewModelAnim( Event *ev )
{
	if( cg->snap != NULL )
	{
		cg->snap->ps.iViewModelAnim = VMA_CUSTOMANIMATION;
	}

	cgi.anim->lastVMAnim = VMA_CUSTOMANIMATION;
	cgi.anim->lastVMAnimChanged = ( cg->snap->ps.iViewModelAnimChanged + 1 ) & 3;

	CG_ViewModelAnim( ( char * )ev->GetString( 1 ).c_str(), false );
}

CLASS_DECLARATION( CAnimate, CPlayer, NULL )
{
	{ &EV_Player_GetMovement,					&CPlayer::EventGetMovement },
	{ &EV_Player_GetReplicationInfo,			&CPlayer::EventGetReplicationInfo },
	{ &EV_Player_GetSpeed,						&CPlayer::EventGetSpeed },
	{ &EV_Player_SetSpeed,						&CPlayer::EventSetSpeed },
	{ &EV_Player_ViewModelAnim,					&CPlayer::EventViewModelAnim },
	{ NULL, NULL }
};
