/*
===========================================================================
Copyright (C) 2025 the OpenMoHAA team

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

#include "clientvote.h"
#include "bg_voteoptions.h"
#include "level.h"
#include "game.h"

CLASS_DECLARATION(Listener, VoteUpload, NULL) {
    {NULL, NULL}
};

VoteUpload::VoteUpload()
    : clientNum(-1)
    , bufferToSend(NULL)
{}

VoteUpload::VoteUpload(int clientNum)
    : VoteUpload()
{
    this->clientNum = clientNum;
}

void VoteUpload::StartSending(const VoteOptions& options)
{
    bufferLength = 0;
    bufferToSend = options.GetVoteOptionsFile(&bufferLength);
    offset       = 0;
}

bool VoteUpload::ClientThink()
{
    const char *cmd;
    size_t      destLength;
    size_t      i;
    size_t      c;
    char        buffer[2068];

    if (bufferLength < MAX_VOTEOPTIONS_UPLOAD_BUFFER_LENGTH) {
        //
        // Small buffer
        //

        Q_strncpyz(buffer, bufferToSend, bufferLength + 1);

        for (i = 0; i < bufferLength; i++) {
            if (buffer[i] == '"') {
                buffer[i] = 1;
            }
        }

        gi.SendServerCommand(clientNum, "vo0 \"\"\n");
        gi.SendServerCommand(clientNum, "vo2 \"%s\"\n", buffer);

        return true;
    }

    for (c = 0; c < MAX_VOTEOPTIONS_UPLOAD_BURST; c++) {
        if (gi.Client_NumPendingCommands(clientNum) + 2 > gi.Client_MaxPendingCommands(clientNum)) {
            // Wait before sending new commands
            break;
        }

        if (offset == 0) {
            cmd        = "vo0";
            destLength = MAX_VOTEOPTIONS_UPLOAD_BUFFER_LENGTH;
        } else if ((bufferLength - offset) >= MAX_VOTEOPTIONS_UPLOAD_BUFFER_LENGTH) {
            cmd        = "vo1";
            destLength = MAX_VOTEOPTIONS_UPLOAD_BUFFER_LENGTH;
        } else {
            cmd        = "vo2";
            destLength = bufferLength - offset;
        }

        Q_strncpyz(buffer, &bufferToSend[offset], MAX_VOTEOPTIONS_UPLOAD_BUFFER_LENGTH);

        for (i = 0; i < destLength; i++) {
            if (buffer[i] == '"') {
                buffer[i] = 1;
            }
        }

        gi.SendServerCommand(clientNum, "%s \"%s\"\n", cmd, buffer);
        offset += MAX_VOTEOPTIONS_UPLOAD_BUFFER_LENGTH - 1;

        if (offset >= bufferLength) {
            //
            // Finished sending
            //
            return true;
        }
    }

    return false;
}
