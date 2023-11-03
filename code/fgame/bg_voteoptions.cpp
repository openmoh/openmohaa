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

#include "bg_voteoptions.h"
#include "../qcommon/script.h"

#if defined(GAME_DLL)
#    define VO_FS_FreeFile           gi.FS_FreeFile
#    define VO_FS_ReadFile(a, b)     gi.FS_ReadFile(a, b, true)
#    define VO_SendRemoteCommand(s)  // ?
#    define VO_ExecuteCommand(s)     gi.Cmd_Execute(EXEC_NOW, s)
#    define VO_Cvar_Set(name, value) gi.cvar_set(name, value)
#elif defined(CGAME_DLL)
#    define VO_FS_FreeFile           cgi.FS_FreeFile
#    define VO_FS_ReadFile(a, b)     cgi.FS_ReadFile(a, b, qtrue)
#    define VO_SendRemoteCommand(s)  cgi.SendClientCommand(s)
#    define VO_ExecuteCommand(s)     cgi.Cmd_Execute(EXEC_NOW, s)
#    define VO_Cvar_Set(name, value) cgi.Cvar_Set(name, value)
#else
#    pragma error("VoteOptions can must be used with fgame or cgame")
#endif

CLASS_DECLARATION(Class, VoteOptions, NULL) {
    {NULL, NULL}
};

VoteOptionListItem::VoteOptionListItem()
    : m_pNext(NULL)
{}

SingleVoteOption::SingleVoteOption()
    : m_optionType(VOTE_NO_CHOICES)
    , m_pListItem(NULL)
    , m_pNext(NULL)
{}

SingleVoteOption::~SingleVoteOption()
{
    VoteOptionListItem *item;
    VoteOptionListItem *next;

    for (item = m_pListItem; item; item = next) {
        next = item->m_pNext;
        delete item;
    }
}

VoteOptions::VoteOptions()
{
    m_pHeadOption = NULL;
}

VoteOptions::~VoteOptions()
{
    ClearOptions();
}

void VoteOptions::ClearOptions()
{
    SingleVoteOption *option;
    SingleVoteOption *next;

    for (option = m_pHeadOption; option; option = next) {
        next = option->m_pNext;
        delete option;
    }

    m_pHeadOption = NULL;
}

void VoteOptions::SetupVoteOptions(const char *configFileName)
{
    char *buffer;
    int   compressedLength;
    long  length;

    length = VO_FS_ReadFile(configFileName, (void **)&buffer, qtrue);
    if (length == -1 || !length) {
        Com_Printf("WARNING: Couldn't find voting options file: %s\n", configFileName);
        return;
    }

    // compress the buffer before setting up
    compressedLength = COM_Compress(buffer);
    SetupVoteOptions(configFileName, compressedLength, buffer);
    VO_FS_FreeFile(buffer);
}

void VoteOptions::SetupVoteOptions(const char *configFileName, int length, const char *buffer)
{
    if (length >= MAX_VOTEOPTIONS_BUFFER_LENGTH) {
        Com_Error(
            ERR_DROP,
            "VoteOptions: Options file '%s' is too big. Max size is %i bytes\n",
            configFileName,
            MAX_VOTEOPTIONS_BUFFER_LENGTH
        );
        return;
    }

    m_sFileName = configFileName;
    m_sBuffer   = buffer;

    ParseVoteOptions();
}

void VoteOptions::ParseVoteOptions()
{
    SingleVoteOption   *option;
    SingleVoteOption   *newOption;
    VoteOptionListItem *listItem;
    VoteOptionListItem *newListItem;
    str                 token;
    Script              script;

    ClearOptions();
    script.LoadFile(m_sFileName.c_str(), m_sBuffer.length(), m_sBuffer.c_str());

    option = m_pHeadOption;

    while (script.TokenAvailable(true)) {
        token = script.GetToken(true);

        if (!str::icmp(token, "{")) {
            Com_Error(
                ERR_DROP,
                "Vote Options %s: Found choices list without option header on line %d.\n",
                m_sFileName.c_str(),
                script.GetLineNumber()
            );
            return;
        }

        if (!str::icmp(token, "}")) {
            Com_Error(
                ERR_DROP,
                "Vote Options %s: Illegal end of choices list without list being started on line %d.\n",
                m_sFileName.c_str(),
                script.GetLineNumber()
            );
            return;
        }

        if (!token.length()) {
            Com_Error(
                ERR_DROP,
                "Vote Options %s: Empty option name on line %d.\n",
                m_sFileName.c_str(),
                script.GetLineNumber()
            );
            return;
        }

        newOption = new SingleVoteOption();

        if (option) {
            option->m_pNext = newOption;
        } else {
            m_pHeadOption = newOption;
        }
        option = newOption;

        newOption->m_sOptionName = token;

        if (!script.TokenAvailable(false)) {
            Com_Error(
                ERR_DROP,
                "Vote Options %s: Option without a command specified on line %d.\n",
                m_sFileName.c_str(),
                script.GetLineNumber()
            );
            return;
        }

        newOption->m_sCommand = script.GetToken(false);

        if (script.TokenAvailable(false)) {
            token = script.GetToken(false);

            if (!str::icmp(token, "nochoices")) {
                newOption->m_optionType = VOTE_NO_CHOICES;
            } else if (!str::icmp(token, "list")) {
                newOption->m_optionType = VOTE_OPTION_LIST;
            } else if (!str::icmp(token, "text")) {
                newOption->m_optionType = VOTE_OPTION_TEXT;
            } else if (!str::icmp(token, "integer")) {
                newOption->m_optionType = VOTE_OPTION_INTEGER;
            } else if (!str::icmp(token, "float")) {
                newOption->m_optionType = VOTE_OPTION_FLOAT;
            } else if (!str::icmp(token, "client")) {
                newOption->m_optionType = VOTE_OPTION_CLIENT;
            } else if (!str::icmp(token, "clientnotself")) {
                newOption->m_optionType = VOTE_OPTION_CLIENT_NOT_SELF;
            } else {
                Com_Error(
                    ERR_DROP,
                    "Vote Options %s: Illegal option type '%s' specified on line %d.\n"
                    " Valid types are nochoices, list, text, & number.\n",
                    m_sFileName.c_str(),
                    token.c_str(),
                    script.GetLineNumber()
                );
                return;
            }
        }

        if (newOption->m_optionType == VOTE_OPTION_LIST) {
            if (!script.TokenAvailable(true) || Q_stricmp(script.GetToken(true), "{")) {
                Com_Error(
                    ERR_DROP,
                    "Vote Options %s: Missing '{'. No choices list specified for list option on line %d.\n",
                    m_sFileName.c_str(),
                    script.GetLineNumber()
                );
                return;
            }

            listItem = NULL;

            while (script.TokenAvailable(true)) {
                token = script.GetToken(true);

                if (!str::icmp(token, "}")) {
                    break;
                }

                newListItem = new VoteOptionListItem();

                if (listItem) {
                    listItem->m_pNext = newListItem;
                } else {
                    newOption->m_pListItem = newListItem;
                }
                listItem = newListItem;

                if (!script.TokenAvailable(false)) {
                    Com_Error(
                        ERR_DROP,
                        "Vote Options %s: List choice without vote string specified on line %d.\n",
                        m_sFileName.c_str(),
                        script.GetLineNumber()
                    );
                    return;
                }

                newListItem->m_sCommand = script.GetToken(false);
            }
        } else if (script.TokenAvailable(true)) {
            token = script.GetToken(true);

            if (!str::icmp(token, "{")) {
                Com_Error(
                    ERR_DROP,
                    "Vote Options %s: Choices list specified for non-list option on line %d.\n",
                    m_sFileName.c_str(),
                    script.GetLineNumber()
                );
                return;
            }

            script.UnGetToken();
        }
    }
}

const char *VoteOptions::GetVoteOptionsFile(int *outLen) const
{
    if (outLen) {
        *outLen = m_sBuffer.length();
    }

    return m_sBuffer.c_str();
}

bool VoteOptions::GetVoteOptionsMain(int index, str *outOptionCommand, voteoptiontype_t *outOptionType) const
{
    SingleVoteOption *option;
    int               optionIndex;

    if (index < 1) {
        return false;
    }

    optionIndex = 1;
    for (option = m_pHeadOption; optionIndex < index && option != NULL; option = option->m_pNext) {
        optionIndex++;
    }

    if (!option) {
        return false;
    }

    *outOptionCommand = option->m_sCommand;
    *outOptionType    = option->m_optionType;

    return true;
}

bool VoteOptions::GetVoteOptionSub(int index, int listIndex, str *outCommand) const
{
    SingleVoteOption   *option;
    VoteOptionListItem *item;
    int                 optionIndex;
    int                 itemIndex;

    if (index < 1 || listIndex < 1) {
        return false;
    }

    optionIndex = 1;
    for (option = m_pHeadOption; optionIndex < index && option != NULL; option = option->m_pNext) {
        optionIndex++;
    }

    if (!option) {
        return false;
    }

    if (option->m_optionType != VOTE_OPTION_LIST) {
        return false;
    }

    itemIndex = 1;
    for (item = option->m_pListItem; itemIndex < listIndex && option != NULL; item = item->m_pNext) {
        itemIndex++;
    }

    if (!item) {
        return false;
    }

    *outCommand = item->m_sCommand;

    return true;
}

bool VoteOptions::GetVoteOptionMainName(int index, str *outVoteName) const
{
    SingleVoteOption *option;
    int               optionIndex;

    if (index < 1) {
        return false;
    }

    optionIndex = 1;
    for (option = m_pHeadOption; optionIndex < index && option != NULL; option = option->m_pNext) {
        optionIndex++;
    }

    if (!option) {
        return false;
    }

    *outVoteName = option->m_sOptionName;

    return true;
}

bool VoteOptions::GetVoteOptionSubName(int index, int listIndex, str *outName) const
{
    SingleVoteOption   *option;
    VoteOptionListItem *item;
    int                 optionIndex;
    int                 itemIndex;

    if (index < 1 || listIndex < 1) {
        return false;
    }

    optionIndex = 1;
    for (option = m_pHeadOption; optionIndex < index && option != NULL; option = option->m_pNext) {
        optionIndex++;
    }

    if (!option) {
        return false;
    }

    if (option->m_optionType != VOTE_OPTION_LIST) {
        return false;
    }

    itemIndex = 1;
    for (item = option->m_pListItem; itemIndex < listIndex && option != NULL; item = item->m_pNext) {
        itemIndex++;
    }

    if (!item) {
        return false;
    }

    *outName = item->m_sItemName;

    return true;
}

#if defined(CGAME_DLL)

static str         g_sVoteString;
static VoteOptions g_voteOptions;

void CG_VoteOptions_StartReadFromServer(const char *string)
{
    g_sVoteString = string;
}

void CG_VoteOptions_ContinueReadFromServer(const char *string)
{
    if (g_sVoteString.length() >= MAX_VOTEOPTIONS_BUFFER_LENGTH) {
        return;
    }

    g_sVoteString += string;
}

void CG_VoteOptions_FinishReadFromServer(const char *string)
{
    int i;

    if (g_sVoteString.length() >= MAX_VOTEOPTIONS_BUFFER_LENGTH) {
        return;
    }

    g_sVoteString += va("%s\n", string);
    if (!str::cmp(g_sVoteString, "\n")) {
        VO_SendRemoteCommand("wait 250;gvo\n");
        return;
    }

    for (i = 0; i < g_sVoteString.length(); i++) {
        if (g_sVoteString[i] == 1) {
            g_sVoteString[i] = '"';
        }
    }

    g_voteOptions.SetupVoteOptions("ServerVoteOptions", g_sVoteString.length(), g_sVoteString.c_str());
    g_sVoteString = "";
    g_voteOptions.SetupMainOptionsList();
}

void VoteOptions::SetupMainOptionsList()
{
    // FIXME: unimplemented
}

void VoteOptions::SetupSubOptionsList(int index)
{
    // FIXME: unimplemented
}
#endif
