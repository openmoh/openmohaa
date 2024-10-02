/*
===========================================================================
Copyright (C) 2024 the OpenMoHAA team

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

#include "snd_local.h"
#include "../qcommon/tiki.h"

void load_sfx_info()
{
    TikiScript  tiki;
    const char *token;
    int         current_sound_file;
    char        file_name[MAX_QPATH];

    sfx_infos[0].name[0]            = 0;
    sfx_infos[0].max_factor         = -1.0;
    sfx_infos[0].loop_start         = -1;
    sfx_infos[0].loop_end           = -1;
    sfx_infos[0].max_number_playing = DEFAULT_SFX_NUMBER_PLAYING;
    number_of_sfx_infos             = 1;

    for (current_sound_file = 0; current_sound_file < 10; current_sound_file++) {
        Com_sprintf(file_name, sizeof(file_name), "global/sound%d.txt", current_sound_file);

        if (!tiki.LoadFile(file_name, qtrue)) {
            continue;
        }

        Com_Printf("Loading %s\n", file_name);

        while (tiki.TokenAvailable(qtrue)) {
            token = tiki.GetToken(qtrue);

            if (!Q_stricmp(token, "sound")) {
                if (tiki.TokenAvailable(qtrue)) {
                    token = tiki.GetToken(qtrue);

                    if (number_of_sfx_infos == 1000) {
                        Com_DPrintf("Too many sound infos specified\n");
                        break;
                    }

                    Q_strncpyz(sfx_infos[number_of_sfx_infos].name, token, sizeof(sfx_infos[number_of_sfx_infos].name));
                    sfx_infos[number_of_sfx_infos].max_factor         = -1.f;
                    sfx_infos[number_of_sfx_infos].loop_start         = -1;
                    sfx_infos[number_of_sfx_infos].loop_end           = -1;
                    sfx_infos[number_of_sfx_infos].max_number_playing = DEFAULT_SFX_NUMBER_PLAYING;
                    number_of_sfx_infos++;
                }
            } else if (!Q_stricmp(token, "loopstart")) {
                if (!tiki.TokenAvailable(qtrue)) {
                    token                                         = tiki.GetToken(qtrue);
                    sfx_infos[number_of_sfx_infos - 1].loop_start = atoi(token);
                }
            } else if (!Q_stricmp(token, "loopend")) {
                if (!tiki.TokenAvailable(qtrue)) {
                    token                                       = tiki.GetToken(qtrue);
                    sfx_infos[number_of_sfx_infos - 1].loop_end = atoi(token);
                }
            } else if (!Q_stricmp(token, "maxnumber")) {
                if (!tiki.TokenAvailable(qtrue)) {
                    token                                                 = tiki.GetToken(qtrue);
                    sfx_infos[number_of_sfx_infos - 1].max_number_playing = atoi(token);
                }
            } else if (!Q_stricmp(token, "maxfactor")) {
                if (!tiki.TokenAvailable(qtrue)) {
                    token                                         = tiki.GetToken(qtrue);
                    sfx_infos[number_of_sfx_infos - 1].max_factor = atoi(token);
                }
            }
        }
    }

    tiki.Close();

    sfx_infos[0].name[0] = 0;
}
