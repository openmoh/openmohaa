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

#include "listener.h"
#include "g_public.h"

typedef enum voteoptiontype_e {
    /** No input. */
    VOTE_NO_CHOICES,
    /** List of choices. */
    VOTE_OPTION_LIST,
    /** Text input type command. */
    VOTE_OPTION_TEXT,
    /** Accepts an integer as an input. */
    VOTE_OPTION_INTEGER,
    /** Accepts a float number as input. */
    VOTE_OPTION_FLOAT,
    /** A list of clients, one to choose. */
    VOTE_OPTION_CLIENT,
    /** A list of clients (excluding self), one to choose. */
    VOTE_OPTION_CLIENT_NOT_SELF,
} voteoptiontype_t;

class VoteOptionListItem
{
    str                       m_sItemName;
    str                       m_sCommand;
    class VoteOptionListItem *m_pNext;
};

class SingleVoteOption
{
    str                     m_sVoteName;
    str                     m_sCommand;
    voteoptiontype_t        m_optionType;
    VoteOptionListItem     *m_pListItem;
    class SingleVoteOption *m_pNext;
};

class VoteOptions : public Class
{
public:
    CLASS_PROTOTYPE(VoteOptions);

public:
    str               m_sFileName;
    str               m_sBuffer;
    SingleVoteOption *m_pHeadOption;
    int               field_5;
    float             field_6;
    float             field_7;
    float             field_8;
    float             field_9;
    float             field_10;
};
