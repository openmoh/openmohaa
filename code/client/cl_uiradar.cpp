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

// cl_uiradar.cpp: draw teammates
// since 2.0

#include "cl_uiradar.h"
#include "client.h"

radarClient_t* g_radarClients;
qhandle_t* g_radarShaders;
int g_radarClientNum;

void CL_InitRadar(radarClient_t* radars, qhandle_t* shaders, int clientNum)
{
	g_radarClients = radars;
	g_radarShaders = shaders;
	g_radarClientNum = clientNum;
}

CLASS_DECLARATION( UIWidget, UIRadar, NULL )
{
	{ NULL, NULL }
};

UIRadar::UIRadar()
{
	m_fRadius = 32.f;
	g_radarClients = NULL;
}

UIRadar::~UIRadar()
{
}

void UIRadar::Draw(void)
{
	radarClient_t* radar;
	float inv;
	float iconSize;
	int blinkTime, speakTime;
	float origin[2], axis[2];
	float halfScale;
	int i;

	if (!g_radarClients) {
		return;
	}

	if (!cl.snap.valid) {
		return;
	}

	if (g_radarClientNum < 0) {
		return;
	}

	cge->CG_UpdateRadar();

	radar = &g_radarClients[g_radarClientNum];
	if (!radar->time) {
		return;
	}

	if (com_radar_range && com_radar_range->value > 0) {
		inv = 1.f / com_radar_range->value;
	} else {
		inv = 0;
	}

	if (cl_radar_icon_size->value > 0) {
		iconSize = cl_radar_icon_size->value;
	} else {
		iconSize = 0;
	}

	axis[0] = radar->axis[0];
	axis[1] = radar->axis[1];
	origin[0] = radar->origin[0];
	origin[1] = radar->origin[1];
	blinkTime = cl_radar_blink_time->value * 1000;
	speakTime = cl_radar_speak_time->value * 1000;

	if (blinkTime < 100) blinkTime = 100;

	halfScale = iconSize * 0.5f;

	for (i = 0; i < MAX_CLIENTS; i++) {
		float delta[2];
		float newOrg[2];
		float screenOrg[2];
		float length;

		radar = &g_radarClients[i];
		if (i == g_radarClientNum) {
			continue;
		}

		if (!g_radarClients[i].time) {
			continue;
		}

		if (cl.snap.serverTime - radar->time >= 5000) {
			continue;
		}

		if (radar->lastSpeakTime) {
			int delta = cl.snap.serverTime - radar->lastSpeakTime;
			// make the icon blink
			if (delta >= 0 && delta < speakTime && ((delta / blinkTime) % 2) == 0) {
				continue;
			}
		}

		radar->lastSpeakTime = 0;
		delta[0] = radar->origin[0] - origin[0];
		delta[1] = radar->origin[1] - origin[1];
		newOrg[0] = -(delta[0] * axis[0] + delta[1] * axis[1]) * inv;
		newOrg[1] = (delta[0] * axis[1] + delta[1] * -axis[0]) * inv;
		length = VectorLength2D(newOrg);

		if (length > 1)
		{
			newOrg[0] = -(delta[0] * axis[0] + delta[1] * axis[1]) * inv * (1.f / length);
			newOrg[1] = (delta[0] * axis[1] + delta[1] * -axis[0]) * inv * (1.f / length);
		}

		screenOrg[0] = -((halfScale * (1.141f * newOrg[1] + m_vVirtualScale[0])) - ((1.f + newOrg[1]) * 0.5f * m_frame.size.width));
		screenOrg[1] = -((halfScale * (1.141f * newOrg[0] + m_vVirtualScale[1])) - ((1.f + newOrg[0]) * 0.5f * m_frame.size.height));

		uii.Rend_DrawPicStretched2(
			screenOrg[0],
			screenOrg[1],
			m_vVirtualScale[0] * iconSize,
			m_vVirtualScale[1] * iconSize,
			0,
			0,
			1,
			1,
			radar->axis[0] * axis[0] - (-radar->axis[1] * axis[1]),
			radar->axis[0] * axis[1] + (-radar->axis[1] * axis[0]),
			g_radarShaders[radar->teamShader]
		);
	}
}



