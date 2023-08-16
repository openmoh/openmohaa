/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int iFlags;
	char szName[64];
} soundSfx_t;

typedef struct {
	qboolean bPlaying;
	int iStatus;
	soundSfx_t sfx;
	int iEntNum;
	int iEntChannel;
	float vOrigin[3];
	float fVolume;
	int iBaseRate;
	float fNewPitchMult;
	float fMinDist;
	float fMaxDist;
	int iStartTime;
	int iTime;
	int iNextCheckObstructionTime;
	int iEndTime;
	int iFlags;
	int iOffset;
	int iLoopCount;
} channelbasesavegame_t;

typedef struct {
	channelbasesavegame_t Channels[MAX_CHANNELS];
} soundsystemsavegame_t;

void S_ChannelFree_Callback(channel_t* v);
void S_LoadData(soundsystemsavegame_t* pSave);

#ifdef __cplusplus
}
#endif
