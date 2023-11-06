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

// tokenizer.h : Tokenizer

#pragma once

class Tokenizer
{
    char *pBuffer;
    char *pEnd;
    int   iLength;
    char *pCurrent;
    int   iLine;
    int   iError;
    bool  fTokenReady;
    char  szToken[256];

public:
    Tokenizer();
    Tokenizer(const char *pInputBuffer, int iBufferLength);

    void   SetBuffer(const char *pInputBuffer, int iBufferLength);
    void   Reset();
    int    GetLineNumber();
    bool   HasError();
    int    GetError();
    bool   SkipToEOL();
    bool   CheckOverflow();
    bool   SkipWhiteSpace(bool fCrossLine);
    bool   AtComment();
    void   SkipNonToken(bool fCrossLine);
    bool   TokenAvailable(bool fCrossLine);
    bool   CommentAvailable(bool fCrossLine);
    void   UnGetToken();
    bool   AtString(bool fCrossLine);
    char  *GetToken(bool fCrossLine);
    char  *GetLine(bool fCrossLine);
    char  *GetRaw();
    char  *GetString(bool fCrossLine);
    bool   GetSpecific(const char *szString);
    int    GetInteger(bool fCrossLine);
    double GetDouble(bool fCrossLine);
    float  GetFloat(bool fCrossLine);
    int    LinesInFile();
    char  *Token();
    char  *GetCurrentPointer();
};
