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

// earthquake.h: Earthquake trigger causes a localized earthquake when triggered.
// The earthquake effect is visible to the user as the shaking of his screen.
// 

#ifndef __EARTHQUAKE_H__
#define __EARTHQUAKE_H__

#include "g_local.h"
#include "trigger.h"

#define EARTHQUAKE_STRENGTH 50

class ViewJitter : public Trigger {
protected:
	float		m_fRadius;
	float		m_fEdgeEffect;
	Vector		m_vJitterStrength;
	float		m_fDuration;
	Vector		m_vTimeDecay;
	qboolean	m_bDoneDeath;
	float		m_fTimeRunning;

public:
	CLASS_PROTOTYPE( ViewJitter );

	ViewJitter();
	ViewJitter( Vector vOrigin, float fRadius, float fEdgeEffect, Vector vStrength, float fDuration, Vector vTimeDecay, float fStartDecay );

	void				EventActivateJitter( Event *ev );
	void				EventJitterThink( Event *ev );
	void				EventSetRadius( Event *ev );
	void				EventSetEdgeEffect( Event *ev );
	void				EventSetAmount( Event *ev );
	void				EventSetDuration( Event *ev );
	void				EventSetTimeDecay( Event *ev );
	void				EventSetDoneDeath( Event *ev );

	void Archive( Archiver& arc ) override;
};

inline void ViewJitter::Archive
	(
	Archiver &arc
	)

{
	Trigger::Archive( arc );

	arc.ArchiveFloat( &m_fRadius );
	arc.ArchiveFloat( &m_fEdgeEffect );
	arc.ArchiveVector( &m_vJitterStrength );
	arc.ArchiveFloat( &m_fDuration );
	arc.ArchiveVector( &m_vTimeDecay );
	arc.ArchiveBoolean( &m_bDoneDeath );
	arc.ArchiveFloat( &m_fTimeRunning );
}

#endif
