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
#  include "listener.h"
#endif

#include "g_public.h"

#ifdef __cplusplus

static const unsigned long MAX_VOTEOPTIONS_UPLOAD_BUFFER_LENGTH = 2024;
static const unsigned long MAX_VOTEOPTIONS_BUFFER_LENGTH        = 0x100000;

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
public:
    str                       m_sItemName;
    str                       m_sCommand;
    class VoteOptionListItem *m_pNext;

public:
    VoteOptionListItem();
};

class SingleVoteOption
{
public:
    str                 m_sOptionName;
    str                 m_sCommand;
    voteoptiontype_t    m_optionType;
    VoteOptionListItem *m_pListItem;
    SingleVoteOption   *m_pNext;

public:
    SingleVoteOption();
    ~SingleVoteOption();
};

class VoteOptions : public Class
{
public:
    CLASS_PROTOTYPE(VoteOptions);

private:
    str               m_sFileName;
    str               m_sBuffer;
    SingleVoteOption *m_pHeadOption;

public:
    VoteOptions();
    ~VoteOptions();

    bool IsSetup() const;

    void        ClearOptions();
    void        SetupVoteOptions(const char *configFileName);
    void        SetupVoteOptions(const char *configFileName, int length, const char *buffer);
    void        ParseVoteOptions();
    const char *GetVoteOptionsFile(int *outLen) const;
    bool        GetVoteOptionsMain(int index, str *outOptionCommand, voteoptiontype_t *outOptionType) const;
    bool        GetVoteOptionSub(int index, int listIndex, str *outCommand) const;
    bool        GetVoteOptionMainName(int index, str *outVoteName) const;
    bool        GetVoteOptionSubName(int index, int listIndex, str *outName) const;

    void SetupMainOptionsList();
    void SetupSubOptionsList(int index);
};

inline bool VoteOptions::IsSetup() const
{
    return m_pHeadOption != NULL;
}

#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CGAME_DLL)
    void CG_VoteOptions_StartReadFromServer(const char *string);
    void CG_VoteOptions_ContinueReadFromServer(const char *string);
    void CG_VoteOptions_FinishReadFromServer(const char *string);
    void CG_PushCallVote_f();
    void CG_PushCallVoteSubList_f();
    void CG_PushCallVoteSubText_f();
    void CG_PushCallVoteSubInteger_f();
    void CG_PushCallVoteSubFloat_f();
    void CG_PushCallVoteSubClient_f();
    void CG_PushVote_f();
    void CG_CallEntryVote_f();
#endif

#ifdef __cplusplus
}
#endif
