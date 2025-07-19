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

#pragma once

class UColor
{
public:
    float r;
    float g;
    float b;
    float a;

    UColor();
    UColor(float r, float g, float b, float a = 1.0);
    UColor(class UColorHSV hsv);
    UColor(const UColor& color);

    operator float *();
    operator float *() const;

    void ScaleColor(float scale);
    void ScaleAlpha(float scale);
    void set(float r, float g, float b, float a);
};

class UColorHSV
{
public:
    float h;
    float s;
    float v;
    float a;

    UColorHSV();
    UColorHSV(UColor rgb);
    UColorHSV(float h, float s, float v, float a);

    void set(float h, float s, float v, float a);
};

typedef enum {
    DARK,
    REALLYDARK,
    LIGHT,
    NORMAL
} colorType_t;

class UBorderColor
{
public:
    UColor dark;
    UColor reallydark;
    UColor light;
    UColor original;

    UBorderColor();
    UBorderColor(const UColor& dark, const UColor& reallydark, const UColor& light);
    UBorderColor(const UColor& color);

    void CreateSolidBorder(const UColor& color, colorType_t type);
};

extern const UColor UClear;
extern const UColor UWhite;
extern const UColor UBlack;
extern const UColor ULightGrey;
extern const UColor UGrey;
extern const UColor UDarkGrey;
extern const UColor ULightRed;
extern const UColor URed;
extern const UColor UGreen;
extern const UColor ULightGreen;
extern const UColor UBlue;
extern const UColor UYellow;
extern const UColor UHudColor;
