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

// localization.cpp : Localization for different languages

#include "win_localization.h"
#include <tiki.h>

#define MAX_BUFFERS             6
#define MAX_LOCALIZATION_LENGTH 1024

cLocalization *g_localization;
static char    global_buf[MAX_BUFFERS][MAX_LOCALIZATION_LENGTH];
static char    szTemp[100];
static size_t  buf_index = 0;

void Sys_InitLocalization()
{
    g_localization = new cLocalization;
}

void Sys_ShutLocalization()
{
    delete g_localization;
    g_localization = NULL;
}

const char *Sys_LV_ConvertString(const char *var)
{
    //
    // Since 2.0, don't localize messages from fgame.
    // The client already localize messages it receives
    //
    return var;
}

const char *Sys_LV_CL_ConvertString(const char *var)
{
    if (!g_localization) {
        return var;
    }

    return g_localization->ConvertString(var);
}

typedef char *charstar;

int compare_strings(const void *pe1, const void *pe2)
{
    const char *psz1 = (const char *)pe1;
    const char *psz2 = (const char *)pe2;
    return Q_stricmp(psz1, psz2);
}

cLocalization::cLocalization()
{
    int    i;
    int    iFileCount;
    char **ppszFiles;
    char   szFilename[MAX_QPATH];
    size_t iBasePos;

    ppszFiles = FS_ListFilteredFiles("global", "txt", "localization*.txt", qfalse, &iFileCount, qtrue);
    Q_strncpyz(szFilename, "global/", sizeof(szFilename));
    memset(szFilename + 8, 0, sizeof(szFilename) - 8);
    iBasePos = strlen(szFilename);

    Com_Printf("-- Localization: I see %d localization files\n", iFileCount);
    qsort(ppszFiles, iFileCount, sizeof(char *), compare_strings);
    for (i = 0; i < iFileCount; i++) {
        Q_strncpyz(szFilename + iBasePos, ppszFiles[i], sizeof(szFilename) - iBasePos);
        Com_Printf("--- Localization: reading file %s\n", szFilename);
        LoadFile(szFilename);
    }

    FS_FreeFileList(ppszFiles);
}

void cLocalization::LoadFile(const char *name)
{
    TikiScript  tiki;
    const char *token;
    loc_entry_t new_entry;
    const char *p, *p2;
    size_t      rl;
    size_t      ll;
    size_t      i;

    Com_Printf("Loading Localization File %s\n", name);

    if (!tiki.LoadFile(name, qtrue)) {
        Com_Printf("--- Error: File Not Found\n");
        return;
    }

    while (tiki.TokenAvailable(qtrue)) {
        token = tiki.GetToken(qtrue);

        if (!Q_stricmp(token, "{")) {
            if (!tiki.TokenAvailable(qtrue)) {
                break;
            }

            token               = tiki.GetToken(qtrue);
            new_entry.m_refName = token;

            rl = strlen(token);
            p  = strstr(token, "&&&");

            if (p) {
                new_entry.m_r1_rep = p - token;
                p2                 = p + 3;
                p                  = strstr(p2, "&&&");

                if (p) {
                    new_entry.m_r2_rep = p - p2;
                    new_entry.m_r3_rep = strlen(p + 3);
                } else {
                    new_entry.m_r2_rep = strlen(p2);
                    new_entry.m_r3_rep = -1;
                }
            } else {
                new_entry.m_r1_rep = rl;
                new_entry.m_r2_rep = -1;
                new_entry.m_r3_rep = -1;
                new_entry.m_l1_rep = -1;
                new_entry.m_l2_rep = -1;
                new_entry.m_l3_rep = -1;
            }

            if (!tiki.TokenAvailable(qtrue)) {
                Com_Printf(
                    "--- Error: EOF Found without LocName. Last RefName was %s. (Misspelling/Unused)\n",
                    new_entry.m_refName.c_str()
                );
                break;
            }

            token               = tiki.GetToken(qtrue);
            new_entry.m_locName = token;

            ll = strlen(token);
            p  = strstr(token, "&&&");

            if (p) {
                new_entry.m_l1_rep = p - token;
                p2                 = p + 3;
                p                  = strstr(p2, "&&&");

                if (p) {
                    new_entry.m_l2_rep = p - p2;
                    new_entry.m_l3_rep = strlen(p + 3);
                } else {
                    new_entry.m_l2_rep = strlen(p2);
                    new_entry.m_l3_rep = -1;
                }
            } else {
                new_entry.m_r1_rep = rl;
                new_entry.m_r2_rep = -1;
                new_entry.m_r3_rep = -1;
                new_entry.m_l1_rep = ll;
                new_entry.m_l2_rep = -1;
                new_entry.m_l3_rep = -1;
            }

            for (i = 1; i <= m_entries.NumObjects(); i++) {
                if (new_entry.m_refName == m_entries.ObjectAt(i).m_refName) {
                    break;
                }
            }

            if (i == m_entries.NumObjects() + 1) {
                m_entries.AddObject(new_entry);
            }

            if (!tiki.TokenAvailable(qtrue)) {
                break;
            }

            token = tiki.GetToken(qtrue);

            if (Q_stricmp(token, "}")) {
                Com_Printf(
                    "--- Error: Invalid Entry... RefName: %s\t\tLocName: %s  Attempting to Recover.\n",
                    new_entry.m_refName.c_str(),
                    new_entry.m_locName.c_str()
                );
                do {
                    if (!tiki.TokenAvailable(qtrue)) {
                        Com_Printf("--- Error: EOF Found. Unable to Recover from Error. \n");
                        break;
                    }

                    token = tiki.GetToken(qtrue);
                } while (Q_stricmp(token, "}"));

                Com_Printf("--- Warning: Recovered From Error, Data May have been lost.\n");
            }
        }
    }

    Com_Printf("Loaded %d localization entries\n", m_entries.NumObjects());
}

static int bjb_strnscmp(const char *a, const char *b, size_t n)
{
    int i;

    while (n > 0 && *a && *b) {
        if (*a == *b) {
            a++;
            b++;
            n--;
            continue;
        }

        if (!isspace((unsigned char)*a) || !isspace((unsigned char)*b)) {
            return *a - *b;
        }

        while (n > 0 && *a && isspace(*a)) {
            a++;
            n--;
        }

        while (*b && isspace(*b)) {
            b++;
        }
    }

    while (n > 0 && *a && isspace(*a)) {
        a++;
        n--;
    }

    while (*b && isspace(*b)) {
        b++;
    }

    if (n <= 0) {
        return 0;
    }

    return *a - *b;
}

static void bjb_rebreak(const char *var, char *buf, size_t max)
{
#if 0
    const char *nl;
    char       *rb, *rb1, *rb2;

    nl = strchr(var, '\n');
    if (!nl || nl >= &var[max]) {
        return;
    }

    while (1) {
        rb  = &buf[nl - var];
        rb1 = &buf[nl - var];
        rb2 = &buf[nl - var];

        if (strlen(buf) < nl - var) {
            return;
        }

        while (rb1 > var && !isspace(*rb1)) {
            rb1--;
        }
        while (*rb2 && rb2 < &var[max] && !isspace(*rb2)) {
            rb2++;
        }

        if (rb1 < var) {
            if (!*rb2 || !isspace(*rb2)) {
                return;
            }

            goto _set_new_line;
        }

        if (!*rb2 || !isspace(*rb1) || !isspace(*rb2)) {
            if (rb1 >= var && isspace(*rb1)) {
                *rb1 = '\n';
                goto _new_line_set;
            } else if (!*rb2 || !isspace(*rb2)) {
                return;
            }

            goto _set_new_line;
        }

        if (rb - rb1 - 4 <= rb2 - rb) {
            *rb1 = '\n';
            goto _new_line_set;
        }

    _set_new_line:
        *rb2 = '\n';
    _new_line_set:
        nl = strchr(nl + 1, '\n');
        if (!nl || nl >= &var[max]) {
            return;
        }
    }
#endif
}

const char *cLocalization::ConvertString(const char *var)
{
    size_t       l;
    intptr_t     i, m, n, o;
    intptr_t     nArg1;
    const char  *pArg1, *pArg2;
    const char  *pRef;
    const char  *pVar;
    const char  *pAfter;
    const char  *pSrc;
    intptr_t     nAfter, nBefore;
    intptr_t     n1, n2;
    loc_entry_t *entry;
    str          s1, s2;

    if (!*var) {
        return var;
    }

    l = strlen(var);
    o = l;

    while (o && var[o - 1] < ' ') {
        o--;
    }

    pRef = &var[o];
    pVar = var;
    m    = l - o;
    n    = 0;

    while (o && (unsigned char)*pVar < ' ') {
        o--;
        pVar++;
        n++;
    }

    char *buf = global_buf[(buf_index++) % MAX_BUFFERS];
    for (i = 1; i <= m_entries.NumObjects(); i++) {
        entry = &m_entries.ObjectAt(i);

        if (entry->m_r2_rep < 0) {
            if (entry->m_r1_rep != o || bjb_strnscmp(pVar, entry->m_refName.c_str(), o)) {
                continue;
            }

            if (!(m + n)) {
                strncpy(buf, entry->m_locName, MAX_LOCALIZATION_LENGTH);
                bjb_rebreak(var, buf, MAX_LOCALIZATION_LENGTH);
                return buf;
            }

            if (entry->m_l1_rep + m + n < MAX_LOCALIZATION_LENGTH) {
                memcpy(buf, var, n);
                memcpy(&buf[n], entry->m_locName.c_str(), entry->m_l1_rep);
                memcpy(&buf[entry->m_l1_rep + n], pRef, m);
                buf[entry->m_l1_rep + m + n] = 0;
                bjb_rebreak(var, buf, entry->m_l1_rep);
                return buf;
            } else {
                return var;
            }
        }

        nArg1 = entry->m_r1_rep + entry->m_r2_rep;
        if (entry->m_r3_rep >= 0) {
            nArg1 += entry->m_r3_rep;
        }

        if (nArg1 > o) {
            continue;
        }

        if (bjb_strnscmp(pVar, entry->m_refName.c_str(), entry->m_r1_rep)) {
            continue;
        }

        pSrc = &pVar[entry->m_r1_rep];

        if (entry->m_r3_rep >= 0) {
            pAfter = &entry->m_refName[entry->m_r1_rep + 3];

            if (entry->m_r2_rep > 100) {
                continue;
            }

            strncpy(szTemp, pAfter, entry->m_r2_rep);
            szTemp[entry->m_r2_rep] = 0;

            pArg1 = strstr(pSrc, szTemp);

            if (!pArg1) {
                continue;
            }

            nAfter = pArg1 - pSrc;
            pArg2  = &pArg1[entry->m_r2_rep];

            if (entry->m_r3_rep > 0) {
                if (strlen(pArg2) < entry->m_r3_rep) {
                    continue;
                }

                if (bjb_strnscmp(&pVar[o - entry->m_r3_rep], &pAfter[entry->m_r2_rep + 3], entry->m_r3_rep)) {
                    continue;
                }

                nBefore = &pVar[o - entry->m_r3_rep] - pArg2;
            } else {
                nBefore = o - (pArg2 - pVar);
            }
        } else {
            if (entry->m_r2_rep > 0) {
                if (bjb_strnscmp(&pVar[o - entry->m_r2_rep], &entry->m_refName[entry->m_r1_rep + 3], entry->m_r2_rep)) {
                    continue;
                }

                nAfter = &pVar[o - entry->m_r2_rep] - pSrc;
            } else {
                nAfter = o - entry->m_r1_rep;
            }

            pArg2   = NULL;
            nBefore = -1;
        }

        n1 = 0;
        n2 = 0;
        if (n > 0) {
            if (n >= 1024) {
                continue;
            }

            memcpy(buf, var, n);
            n2 = n;
        }

        if (entry->m_l1_rep >= 0) {
            if (entry->m_l1_rep + n2 >= 1024) {
                continue;
            }

            if (entry->m_l1_rep > 0) {
                memcpy(&buf[n2], &entry->m_locName[0], entry->m_l1_rep);
            }

            n2 += entry->m_l1_rep;
            n1 = entry->m_l1_rep + 3;
        }

        if (nAfter >= 0) {
            if (nAfter + n2 >= 1024) {
                continue;
            }

            if (nAfter > 0) {
                memcpy(&buf[n2], pSrc, nAfter);
            }

            n2 += nAfter;
        }

        if (entry->m_l2_rep >= 0) {
            if (entry->m_l2_rep + n2 >= 1024) {
                continue;
            }

            if (entry->m_l2_rep > 0) {
                memcpy(&buf[n2], &entry->m_locName[n1], entry->m_l2_rep);
            }

            n2 += entry->m_l2_rep;
            n1 += entry->m_l2_rep + 3;
        }

        if (nBefore >= 0) {
            if (nBefore + n2 >= 1024) {
                continue;
            }

            if (nBefore > 0) {
                memcpy(&buf[n2], pArg2, nBefore);
            }

            n2 += nBefore;
        }

        if (entry->m_l3_rep >= 0) {
            if (entry->m_l3_rep + n2 >= 1024) {
                continue;
            }

            if (entry->m_l3_rep > 0) {
                memcpy(&buf[n2], &entry->m_locName[n1], entry->m_l3_rep);
            }

            n2 += entry->m_l3_rep;
        }

        if (m <= 0 || m + n2 < 1024) {
            if (m + n2 < 1024) {
                memcpy(&buf[n2], pRef, m);
                n2 += m;
            }

            buf[n2] = 0;
            bjb_rebreak(var, buf, n2);
            return buf;
        }
    }

    if (!developer->integer || !var[0] || !var[1]) {
        return var;
    }

    i = 0;

    while (var[i] < 0 || !isspace(var[i])) {
        i++;
        if (!var[i]) {
            return var;
        }
    }

    if (!missing.ObjectInList(var)) {
        missing.AddUniqueObject(var);
        Com_DPrintf("LOCALIZATION ERROR: '%s' does not have a localization entry\n", var);
    }

    return var;
}
