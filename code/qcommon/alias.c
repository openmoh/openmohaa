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

#include "q_shared.h"
#include "qcommon.h"
#include "alias.h"

static AliasList_t *Aliases;
//static AliasListNode_t* foundlist[64];
// Changed in 2.0
//static AliasListNode_t* foundlist[128];
// Changed in OPM
static AliasListNode_t *foundlist[256];

AliasListNode_t *AliasListNode_New()
{
    AliasListNode_t *node = Z_TagMalloc(sizeof(AliasListNode_t), TAG_TIKI);

    memset(node, 0, sizeof(AliasListNode_t));
    node->weight = 1.0f;
    node->next   = NULL;

    return node;
}

AliasList_t *AliasList_New(const char *name)
{
    AliasList_t *list = Z_TagMalloc(sizeof(AliasList_t), TAG_TIKI);

    list->num_in_list = 0;
    list->sorted_list = NULL;
    list->data_list   = NULL;
    list->dirty       = qfalse;

    if (name) {
        strncpy(list->name, name, sizeof(list->name));
    } else {
        list->name[0] = 0;
    }

    return list;
}

void Alias_ListClear(AliasList_t *list)
{
    AliasListNode_t *node;
    AliasListNode_t *next;

    if (list->sorted_list) {
        Z_Free(list->sorted_list);
    }

    for (node = list->data_list; node != NULL; node = next) {
        if (node->subtitle) {
            // Added in OPM
            Z_Free(node->subtitle);
        }

        next = node->next;
        Z_Free(node);
    }

    list->num_in_list = 0;
    list->dirty       = qfalse;
    list->sorted_list = NULL;
    list->data_list   = NULL;
}

void Alias_ListDelete(AliasList_t *list)
{
    Alias_ListClear(list);
    Z_Free(list);
}

float getTokenFloat(const char **parameters, char *parmName, AliasListNode_t *node)
{
    const char *token;
    float       temp_float = 0.0f;

    token = COM_GetToken((char **)parameters, 1);

    if (token && *token) {
        temp_float = (float)atof(token);
        if (temp_float < 0.0f) {
            Com_Printf("ERROR getTokenFloat: %s value out of range in %s alias\n", parmName, node->alias_name);
        }

        return temp_float;
    } else {
        Com_Printf("ERROR getTokenFloat: %s value is hosed or wasn't found for %s\n", parmName, node->alias_name);
    }

    return 0.0f;
}

int S_ChannelNameToNum(const char *pszName)
{
    if (!Q_stricmp(pszName, "auto")) {
        return CHAN_AUTO;
    } else if (!Q_stricmp(pszName, "local")) {
        return CHAN_LOCAL;
    } else if (!Q_stricmp(pszName, "weapon")) {
        return CHAN_WEAPON;
    } else if (!Q_stricmp(pszName, "voice")) {
        return CHAN_VOICE;
    } else if (!Q_stricmp(pszName, "item")) {
        return CHAN_ITEM;
    } else if (!Q_stricmp(pszName, "body")) {
        return CHAN_BODY;
    } else if (!Q_stricmp(pszName, "dialog")) {
        return CHAN_DIALOG;
    } else if (!Q_stricmp(pszName, "dialog_secondary")) {
        return CHAN_DIALOG_SECONDARY;
    } else if (!Q_stricmp(pszName, "weaponidle")) {
        return CHAN_WEAPONIDLE;
    } else if (!Q_stricmp(pszName, "menu")) {
        return CHAN_MENU;
    } else {
        return -1;
    }
}

const char *S_ChannelNumToName(int iChannel)
{
    switch (iChannel) {
    case CHAN_AUTO:
        return "auto";
    case CHAN_LOCAL:
        return "local";
    case CHAN_WEAPON:
        return "weapon";
    case CHAN_VOICE:
        return "voice";
    case CHAN_ITEM:
        return "item";
    case CHAN_BODY:
        return "body";
    case CHAN_DIALOG:
        return "dialog";
    case CHAN_DIALOG_SECONDARY:
        return "dialog_secondary";
    case CHAN_WEAPONIDLE:
        return "weaponidle";
    case CHAN_MENU:
        return "menu";
    default:
        return NULL;
    }
}

void Alias_ListAddParms(AliasListNode_t *node, const char *parameters)
{
    const char *token;
    float       temp_float;
    const char *nptr;

    if (!node || !parameters) {
        return;
    }

    for (;;) {
        token = nptr = COM_GetToken((char **)&parameters, qtrue);
        if (!token || !token[0]) {
            break;
        }

        if (!Q_stricmp(token, "stop")) {
            node->stop_flag = 1;
        } else if (!Q_stricmp(token, "soundparms")) {
            node->volume    = getTokenFloat(&parameters, "volume", node);
            node->volumeMod = getTokenFloat(&parameters, "volumeMod", node);
            node->pitch     = getTokenFloat(&parameters, "pitch", node);
            node->pitchMod  = getTokenFloat(&parameters, "pitchMod", node);
            node->dist      = getTokenFloat(&parameters, "dist", node);
            node->maxDist   = getTokenFloat(&parameters, "maxDist", node);

            token = COM_GetToken((char **)&parameters, qtrue);
            if (!token || !*token) {
                return;
            }

            node->channel = S_ChannelNameToNum(token);
            if (node->channel == -1) {
                Com_Printf("%s is not avalid channel on %s alias.\n", token, node->alias_name);
                node->channel = 0;
            }

            token = COM_GetToken((char **)&parameters, qtrue);
            if (!token || !*token) {
                Com_Printf("ERROR: defaulting %s to not streamed - please setup alias properly.\n", node->alias_name);
                return;
            }

            if (!Q_stricmp(token, "streamed")) {
                node->streamed = qtrue;
            } else if (!Q_stricmp(token, "loaded")) {
                node->streamed = qfalse;
            } else {
                Com_Printf(
                    "ERROR: Expecting streamed or loaded in ubersound and got %s on alias %s\n", token, node->alias_name
                );
                continue;
            }
        } else if (!Q_stricmp(token, "subtitle") || !Q_stricmp(token, "forcesubtitle")) {
            size_t size;

            if (!Q_stricmp(token, "subtitle")) {
                node->forcesubtitle = qfalse;
            } else {
                node->forcesubtitle = qtrue;
            }

            token = COM_GetToken((char **)&parameters, qtrue);
            if (!token || !*token) {
                Com_Printf("NULL subtitle on %s alias.\n", node->alias_name);
                return;
            }

            size           = strlen(token) + 1;
            node->subtitle = Z_TagMalloc(size, TAG_TIKI);
            memcpy(node->subtitle, token, size);
        } else if (!Q_stricmp(token, "pitch")) {
            token = COM_GetToken((char **)&parameters, qtrue);
            if (!token || !*token) {
                return;
            }

            temp_float = (float)atof(token);
            if (temp_float < 0.0f) {
                Com_Printf("Pitch value out of range in %s alias\n", node->alias_name);
                continue;
            }

            node->pitch = temp_float;
        } else if (!Q_stricmp(token, "pitchmod")) {
            token = COM_GetToken((char **)&parameters, qtrue);
            if (!token || !*token) {
                return;
            }

            temp_float = (float)atof(token);
            if (temp_float < 0.0f) {
                Com_Printf("PitchMod value out of range in %s alias\n", node->alias_name);
                continue;
            }

            node->pitchMod = temp_float;
        } else if (!Q_stricmp(token, "volumemod")) {
            token = COM_GetToken((char **)&parameters, qtrue);
            if (!token || !*token) {
                return;
            }

            temp_float = (float)atof(token);
            if (temp_float < 0.0f) {
                Com_Printf("volumeMod value out of range in %s alias\n", node->alias_name);
                continue;
            }

            node->volumeMod = temp_float;
        } else if (!Q_stricmp(token, "dist")) {
            token = COM_GetToken((char **)&parameters, qtrue);
            if (!token || !*token) {
                Com_Printf("\n\n BAD DISTANCE\n\n");
                return;
            }

            temp_float = (float)atof(token);
            if (temp_float < 0.0f) {
                Com_Printf("dist value out of range in %s alias\n", node->alias_name);
                continue;
            }

            node->dist = temp_float;
        } else if (!Q_stricmp(token, "channel")) {
            token = COM_GetToken((char **)&parameters, qtrue);
            if (!token || !*token) {
                Com_Printf("\n\n BAD DISTANCE\n\n");
                return;
            }

            node->channel = S_ChannelNameToNum(token);
            if (node->channel == -1) {
                Com_Printf("%s is not a valid channel on %s alias.\n", token, node->alias_name);
                node->channel = CHAN_AUTO;
            }
        } else {
            Com_Printf("Unknown parameter for %s alias\n", node->alias_name);
            continue;
        }
    }
}

qboolean Alias_ListAdd(AliasList_t *list, const char *alias, const char *name, const char *parameters)
{
    AliasListNode_t *ptr = NULL;
    int              i;
    AliasListNode_t *node;

    if (!list) {
        return qfalse;
    }

    if (list->dirty) {
        for (node = list->data_list; node != NULL; node = node->next) {
            if (!strcmp(node->alias_name, alias)) {
                Com_DPrintf("DUPLICATE ALIASES: %s and %s\n", node->alias_name, alias);
                return qtrue;
            }
        }
    } else {
        ptr = Alias_ListFindNode(list, alias);
    }

    if (!ptr) {
        node = AliasListNode_New();
        strncpy(node->alias_name, alias, sizeof(node->alias_name));

        for (i = 0; i < MAX_ALIASLIST_NAME_LENGTH; i++) {
            node->alias_name[i] = tolower(node->alias_name[i]);
        }

        strncpy(node->real_name, name, sizeof(node->real_name));
        node->stop_flag = 0;
        Alias_ListAddParms(node, parameters);
        list->dirty = qtrue;
        list->num_in_list++;
        node->next      = list->data_list;
        list->data_list = node;
    } else if (strcmp(name, ptr->real_name)) {
        Com_DPrintf("Duplicate Aliases for %s in list %s.\n", alias, name);
    }

    return qtrue;
}

static int Alias_ListCompareNodes(const void *a1, const void *a2)
{
    const AliasListNode_t *ptr1 = *(const AliasListNode_t **)a1;
    const AliasListNode_t *ptr2 = *(const AliasListNode_t **)a2;

    return strcmp(ptr1->alias_name, ptr2->alias_name);
}

void Alias_ListSort(AliasList_t *list)
{
    AliasListNode_t *ptr;
    int              num;
    int              i;

    if (!list->dirty) {
        return;
    }

    if (list->sorted_list) {
        Z_Free(list->sorted_list);
    }

    num = list->num_in_list;

    list->sorted_list = Z_TagMalloc(num * sizeof(AliasListNode_t *), TAG_TIKI);

    for (i = 0, ptr = list->data_list; i < num; i++, ptr = ptr->next) {
        list->sorted_list[i] = ptr;
    }

    qsort(list->sorted_list, num, sizeof(AliasListNode_t *), Alias_ListCompareNodes);
    list->dirty = 0;
}

const char *Alias_ListFind(AliasList_t *list, const char *alias)
{
    AliasListNode_t *node = Alias_ListFindNode(list, alias);

    if (node) {
        return node->real_name;
    } else {
        return NULL;
    }
}

void Alias_ListUpdateDialog(AliasList_t *list, const char *alias)
{
    int   min_index;
    int   max_index;
    float total_weight;

    Alias_ListFindRandomRange(list, alias, &min_index, &max_index, &total_weight);
}

AliasListNode_t *Alias_ListFindNode(AliasList_t *list, const char *alias)
{
    char   convalias[40];
    int    index;
    int    l;
    int    r;
    int    diff;
    size_t length;
    int    __res;

    if (list->dirty) {
        Alias_ListSort(list);
    }

    length = strlen(alias);

    for (index = 0; index < length; index++) {
        convalias[index] = tolower(alias[index]);
    }

    convalias[length] = 0;

    r = 0;
    l = list->num_in_list - 1;
    for (index = l; index >= 0; index--) {
        __res = (l + r) >> 1;
        diff  = strcmp(convalias, list->sorted_list[index]->alias_name);

        if (diff == 0) {
            return list->sorted_list[index];
        } else if (diff > 0) {
            r = __res + 1;
        } else {
            l = __res - 1;
        }
    }

    return NULL;
}

float randweight()
{
    return ((float)(rand() & 0x7fff)) / ((float)0x8000);
}

const char *Alias_ListFindRandom(AliasList_t *list, const char *alias, AliasListNode_t **ret)
{
    int   min_index;
    int   max_index;
    float total_weight;
    int   i;
    int   numfound;
    float weight;
    float num;

    if (!*alias) {
        return NULL;
    }

    Alias_ListFindRandomRange(list, alias, &min_index, &max_index, &total_weight);

    if (min_index == -1 || max_index == -1) {
        return NULL;
    }

    num      = 0.0f;
    numfound = max_index - min_index + 1;
    weight   = randweight() * total_weight;

    for (i = 0; i < numfound; i++) {
        num += list->sorted_list[i + min_index]->weight;
        if (num > weight) {
            if (ret) {
                *ret = list->sorted_list[i + min_index];
            }

            return list->sorted_list[i + min_index]->real_name;
        }
    }

    return NULL;
}

void Alias_ListFindRandomRange(
    AliasList_t *list, const char *alias, int *min_index, int *max_index, float *total_weight
)
{
    char              convalias[MAX_ALIASLIST_NAME_LENGTH];
    int               index;
    int               l;
    int               r;
    int               diff;
    int               i;
    size_t            length;
    int               numfound;
    float             totalfoundweight = 0.0f;
    AliasListNode_t **ptr;

    *min_index = -1;
    *max_index = -1;

    if (!*alias) {
        return;
    }

    if (list->dirty) {
        Alias_ListSort(list);
    }

    length = strlen(alias);

    if (length + 1 > MAX_ALIASLIST_NAME_LENGTH) {
        return;
    }

    for (i = 0; i < length; i++) {
        convalias[i] = tolower(alias[i]);
    }

    convalias[length] = 0;

    l = 0;
    r = list->num_in_list - 1;
    while (r >= l) {
        index = (l + r) >> 1;
        diff  = strncmp(convalias, list->sorted_list[index]->alias_name, length);

        if (diff == 0) {
            break;
        } else if (diff > 0) {
            l = index + 1;
        } else {
            r = index - 1;
        }
    }

    if (r < l) {
        return;
    }

    numfound         = 0;
    *min_index       = index;
    *max_index       = index;
    totalfoundweight = 0.f;

    ptr = &list->sorted_list[index];
    for (i = index + 1; i > 0; i--, ptr--) {
        if (strncmp(convalias, (*ptr)->alias_name, length)) {
            break;
        }

        if (numfound < ARRAY_LEN(foundlist) && (*ptr)->alias_name[length] != '_') {
            foundlist[numfound++] = *ptr;
            totalfoundweight += (*ptr)->weight;
            *min_index = i - 1;
        }
    }

    ptr = &list->sorted_list[index + 1];
    for (i = index + 1; i < list->num_in_list; i++, ptr++) {
        if (strncmp(convalias, (*ptr)->alias_name, length)) {
            break;
        }

        if (numfound < ARRAY_LEN(foundlist) && (*ptr)->alias_name[length] != '_') {
            foundlist[numfound++] = *ptr;
            totalfoundweight += (*ptr)->weight;
            *max_index = i;
        }
    }

    if (!numfound) {
        *min_index = -1;
        *max_index = -1;
        return;
    }

    *total_weight = totalfoundweight;
}

void Alias_ListDump(AliasList_t *list)
{
    AliasListNode_t *ptr;
    int              i;

    if (list->dirty) {
        Alias_ListSort(list);
    }

    Com_DPrintf("Alias List: %s\n", list->name);
    Com_DPrintf("Alias Name / Weight / Real Name\n");

    for (i = 0; i < list->num_in_list; ++i) {
        ptr = list->sorted_list[i];
        Com_DPrintf("%32s %3.2f %s\n", ptr->alias_name, ptr->weight, ptr->real_name);
    }

    Com_DPrintf("%d total aliases\n", list->num_in_list);
}

const char *Alias_Find(const char *alias)
{
    if (!Aliases) {
        Aliases = AliasList_New("Global");
    }
    return Alias_ListFind(Aliases, alias);
}

qboolean Alias_Add(const char *alias, const char *name, const char *parameters)
{
    if (!Aliases) {
        Aliases = AliasList_New("Global");
    }
    return Alias_ListAdd(Aliases, alias, name, parameters);
}

qboolean Alias_Delete(const char *alias)
{
    return qtrue;
}

const char *Alias_FindRandom(const char *alias, AliasListNode_t **ret)
{
    if (!Aliases) {
        Aliases = AliasList_New("Global");
    }
    return Alias_ListFindRandom(Aliases, alias, ret);
}

void Alias_Dump()
{
    if (!Aliases) {
        Aliases = AliasList_New("Global");
    }
    Alias_ListDump(Aliases);
}

void Alias_Clear()
{
    if (!Aliases) {
        Aliases = AliasList_New("Global");
    }
    Alias_ListClear(Aliases);
}

AliasList_t *Alias_GetGlobalList()
{
    return Aliases;
}
