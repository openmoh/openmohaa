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

// actorenemy.h

#ifndef __ACTORENEMY_H__
#define __ACTORENEMY_H__

#include "g_local.h"

class Actor;
class Sentient;

class ActorEnemy : public Class {
public:
	int m_iAddTime;
	float m_fLastLookTime;
	float m_fVisibility;
	float m_fTotalVisibility;
	int m_iThreat;
	SafePtr<Sentient> m_pEnemy;
	float m_fCurrentRangeSquared;
	Vector m_vLastKnownPos;
	int m_iLastSightChangeTime;
	bool m_bVisible;

	ActorEnemy();
	virtual ~ActorEnemy();
	float UpdateVisibility( Actor *pSelf, bool *pbInFovAndRange, bool *pbVisible );
	int UpdateThreat( Actor *pSelf );
	Sentient *GetEnemy( void ) const;
	float GetVisibility( void ) const;
	int GetThreat( void ) const;
	float GetRangeSquared( void ) const;

	void Archive( Archiver & ) override;

protected:
	float UpdateLMRF( Actor *pSelf, bool *pbInFovAndRange, bool *pbVisible );
};

inline void ActorEnemy::Archive
	(
	Archiver& arc
	)

{
	Class::Archive( arc );

	arc.ArchiveInteger( &m_iAddTime );
	arc.ArchiveFloat( &m_fLastLookTime );
	arc.ArchiveFloat( &m_fVisibility );
	arc.ArchiveFloat( &m_fTotalVisibility );
	arc.ArchiveInteger( &m_iThreat );
	arc.ArchiveSafePointer( &m_pEnemy );
	arc.ArchiveFloat( &m_fCurrentRangeSquared );
	arc.ArchiveVector( &m_vLastKnownPos );
	arc.ArchiveInteger( &m_iLastSightChangeTime );
	arc.ArchiveBool( &m_bVisible );
}

class ActorEnemySet : public Class {
protected:
	Container<ActorEnemy> m_Enemies;
	int m_iCheckCount;
	SafePtr<Sentient> m_pCurrentEnemy;
	float m_fCurrentVisibility;
	int m_iCurrentThreat;
public:
	ActorEnemySet();
	virtual ~ActorEnemySet();

	ActorEnemy *AddPotentialEnemy( Sentient *pEnemy );
	void FlagBadEnemy( Sentient *pEnemy );
	void CheckEnemies( Actor *pSelf );
	Sentient *GetCurrentEnemy( void ) const;
	float GetCurrentVisibility( void ) const;
	int GetCurrentThreat( void ) const;
	qboolean IsEnemyConfirmed( void ) const;
	bool HasAlternateEnemy( void ) const;
	void RemoveAll( void );
	void ConfirmEnemy( Actor *pSelf, Sentient *pEnemy );
	void ConfirmEnemyIfCanSeeSharerOrEnemy( Actor *pSelf, Actor *pSharer, Sentient *pEnemy );
	bool CaresAboutPerfectInfo( Sentient *pEnemy );

	void Archive( Archiver& arc ) override;
};

inline void ActorEnemySet::Archive
	(
	Archiver& arc
	)

{
	Class::Archive( arc );

	m_Enemies.Archive( arc, ArchiveClass< ActorEnemy > );
	arc.ArchiveInteger( &m_iCheckCount );
	arc.ArchiveSafePointer( &m_pCurrentEnemy );
	arc.ArchiveFloat( &m_fCurrentVisibility );
	arc.ArchiveInteger( &m_iCurrentThreat );
}

#endif /* actorenemy.h */