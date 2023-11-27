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

// spline.h: Spline paths.
//

#pragma once

#include "g_local.h"
#include <class.h>

template<unsigned int cGrids, unsigned int cPoints>
class cSpline : public Class
{
public:
    int   m_iPoints;
    float m_vPoints[cPoints][cGrids];
    int   m_iPointFlags[cPoints];

    cSpline();

    void   Reset(void);
    int    Add(float *fAdd, int flags);
    void   Modify(int, float *, int);
    void   UniformAdd(float *pos);
    void   Remove(class cSpline<cGrids, cPoints> *, int);
    void   RemoveRange(int, int);
    int    Right(float x);
    float *Get(float x, int *flags);
    float *GetByNode(float x, int *flags);
    int    Append(cSpline<cGrids, cPoints> *pNew);

    void Archive(Archiver& arc);
};

template<unsigned int cGrids, unsigned int cPoints>
cSpline<cGrids, cPoints>::cSpline()
{
    m_iPoints = 0;
}

template<unsigned int cGrids, unsigned int cPoints>
void cSpline<cGrids, cPoints>::Archive(Archiver& arc)
{
    arc.ArchiveInteger(&m_iPoints);

    for (int i = 0; i < cPoints; i++) {
        for (int j = 0; j < cGrids; j++) {
            arc.ArchiveFloat(&m_vPoints[i][j]);
        }

        arc.ArchiveInteger(&m_iPointFlags[i]);
    }
}

template<unsigned int cGrids, unsigned int cPoints>
void cSpline<cGrids, cPoints>::Reset(void)
{
    m_iPoints = 0;
}

template<unsigned int cGrids, unsigned int cPoints>
int cSpline<cGrids, cPoints>::Add(float *fAdd, int flags)
{
    int i;
    int ii;
    int insertIndex;

    if (m_iPoints + 1 > 512) {
        return -1;
    }

    insertIndex = Right(*fAdd);

    for (i = m_iPoints; i > insertIndex; i--) {
        for (ii = 0; ii < cGrids; ii++) {
            m_vPoints[i][ii] = m_vPoints[i - 1][ii];
        }

        m_iPointFlags[i] = m_iPointFlags[i - 1];
    }

    for (i = 0; i < cGrids; i++) {
        m_vPoints[insertIndex][i] = fAdd[i];
    }

    m_iPointFlags[insertIndex] = flags;
    m_iPoints++;

    return insertIndex;
}

template<unsigned int cGrids, unsigned int cPoints>
void cSpline<cGrids, cPoints>::UniformAdd(float *pos)
{
    int i;
    int ii;

    for (i = m_iPoints; i > 0; i--) {
        for (ii = 0; ii <= cGrids; ii++) {
            m_vPoints[i][ii] += pos[ii];
        }
    }
}

template<unsigned int cGrids, unsigned int cPoints>
int cSpline<cGrids, cPoints>::Right(float x)
{
    int i;

    for (i = 0; i < m_iPoints; i++) {
        if (m_vPoints[i][0] > x) {
            break;
        }
    }

    return i;
}

template<unsigned int cGrids, unsigned int cPoints>
float *cSpline<cGrids, cPoints>::Get(float x, int *flags)
{
    if (!m_iPoints) {
        return NULL;
    }

    int          rp;
    int          i;
    static float r[cGrids];
    double       delta[cGrids];

    rp = Right(x);

    if (rp) {
        if (rp == m_iPoints) {
            if (flags) {
                *flags = m_iPointFlags[rp - 1];
            }

            for (i = 0; i < cGrids; i++) {
                r[i] = m_vPoints[rp - 1][i];
            }
        } else {
            if (flags) {
                *flags = m_iPointFlags[rp - 1];
            }

            for (i = 0; i < cGrids; i++) {
                delta[i] = m_vPoints[rp][i] - m_vPoints[rp - 1][i];
            }

            for (i = 0; i < cGrids; i++) {
                r[i] = (x - m_vPoints[rp - 1][0]) / delta[0] * delta[i] + m_vPoints[rp - 1][i];
            }
        }
    } else {
        if (flags) {
            *flags = m_iPointFlags[0];
        }

        for (i = 0; i < cGrids; i++) {
            r[i] = m_vPoints[0][i];
        }
    }

    return r;
}

template<unsigned int cGrids, unsigned int cPoints>
float *cSpline<cGrids, cPoints>::GetByNode(float x, int *flags)
{
    if (!m_iPoints) {
        return NULL;
    }

    int          rp;
    int          i;
    static float r[cGrids];
    double       delta[cGrids];

    rp = (int)(floor(x) + 1.0f);

    if (rp <= 0) {
        if (flags) {
            *flags = m_iPointFlags[0];
        }

        for (i = 0; i < cGrids; i++) {
            r[i] = m_vPoints[0][i];
        }
    } else if (rp < m_iPoints) {
        if (flags) {
            *flags = m_iPointFlags[rp - 1];
        }

        for (i = 0; i < cGrids; i++) {
            delta[i] = m_vPoints[rp][i] - m_vPoints[rp - 1][i];
        }

        for (i = 0; i < cGrids; i++) {
            r[i] = (x - (rp - 1)) * delta[i] + m_vPoints[rp - 1][i];
        }
    } else {
        if (flags) {
            *flags = m_iPointFlags[rp - 1];
        }

        for (i = 0; i < cGrids; i++) {
            r[i] = m_vPoints[rp - 1][i];
        }
    }

    return r;
}

template<unsigned int cGrids, unsigned int cPoints>
int cSpline<cGrids, cPoints>::Append(cSpline<cGrids, cPoints> *pNew)
{
    float *i_fTmp;
    float  o_fTmp[4];
    float  fIndexAdd;
    int    i;
    int    ii;
    int    iFlags;

    if (!pNew || pNew->m_iPoints == 0) {
        return -1;
    }

    if (m_iPoints) {
        fIndexAdd = *GetByNode(m_iPoints, NULL);

        for (i = 0; i < pNew->m_iPoints; i++) {
            i_fTmp = pNew->GetByNode(i, &iFlags);

            for (ii = 0; ii < cGrids; ii++) {
                o_fTmp[ii] = i_fTmp[ii];
            }

            o_fTmp[0] += fIndexAdd;
            Add(o_fTmp, iFlags);
        }

        return m_iPoints;
    } else {
        for (i = 0; i < pNew->m_iPoints; i++) {
            i_fTmp = pNew->GetByNode(i, &iFlags);

            for (ii = 0; ii < cGrids; ii++) {
                o_fTmp[ii] = i_fTmp[ii];
            }

            Add(o_fTmp, iFlags);
        }

        return 0;
    }
}
