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

// tr_font.cpp -- font rendering

#include "tr_local.h"

#define MAX_LOADED_FONTS 255

static fontheader_t s_loadedFonts[MAX_LOADED_FONTS];
static int s_numLoadedFonts = 0;
static float s_fontHeightScale = 1.0;
static float s_fontGeneralScale = 1.0;
static float s_fontZ = 0.0;;

void R_SetFontHeightScale(float scale)
{
    s_fontHeightScale = scale;
}

void R_SetFontScale(float scale)
{
    s_fontGeneralScale = scale;
}

void R_SetFontZ(float zed)
{
    s_fontZ = zed;
}

fontheader_t* R_LoadFont(const char* name)
{
    int i;
    char* theFile;
    fontheader_t* header;
    char* ref;
    const char* token;
    qboolean error;

    error = qfalse;

    for (i = 0; i < s_numLoadedFonts; i++)
    {
        header = &s_loadedFonts[i];
        if (!Q_stricmp(name, header->name)) {
            return header;
        }
    }

    if (s_numLoadedFonts >= MAX_LOADED_FONTS)
    {
        ri.Printf(PRINT_WARNING, "LoadFont: Too many fonts loaded!  Couldn't load %s\n", name);
        return NULL;
    }

    va("fonts/%s.RitualFont", name);
    if (ri.FS_ReadFile(va("fonts/%s.RitualFont", name), (void**)&theFile) == -1)
    {
        ri.Printf(PRINT_WARNING, "LoadFont: Couldn't load font %s\n", name);
        return 0;
    }

    header = &s_loadedFonts[s_numLoadedFonts];
    header->height = 0.0;
    header->aspectRatio = 0.0;
    Q_strncpyz(header->name, name, sizeof(header->name));

    ref = theFile;
    while (ref && !error)
    {
        token = COM_Parse(&ref);
        if (!Q_stricmp(token, "RitFont"))
        {
            // ignore this token
            continue;
        }

        if (!Q_stricmp(token, "indirections"))
        {
            token = COM_Parse(&ref);
            if (Q_stricmp(token, "{"))
            {
                error = qtrue;
                break;
            }

            for (i = 0; i < 256; i++)
            {
                token = COM_Parse(&ref);
                if (!token[0]) {
                    error = qtrue;
                    break;
                }

                header->indirection[i] = atoi(token);
            }

            if (error) {
                break;
            }

            token = COM_Parse(&ref);
            if (Q_stricmp(token, "}"))
            {
                error = qtrue;
                break;
            }
        }
        else if (!Q_stricmp(token, "locations"))
        {
            token = COM_Parse(&ref);
            if (Q_stricmp(token, "{"))
            {
                error = qtrue;
                break;
            }

            for (i = 0; i < 256; i++)
            {
                token = COM_Parse(&ref);
                if (Q_stricmp(token, "{")) {
                    error = qtrue;
                    break;
                }

                if (header->aspectRatio == 0.0)
                {
                    ri.Printf(PRINT_WARNING, "WARNING: aspect decl must be before locations in font '%s'", name);
                    break;
                }

                header->locations[i].pos[0] = atof(COM_Parse(&ref)) / 256.0;
                header->locations[i].pos[1] = atof(COM_Parse(&ref)) * header->aspectRatio / 256.0;
                header->locations[i].size[0] = atof(COM_Parse(&ref)) / 256.0;
                header->locations[i].size[1] = atof(COM_Parse(&ref)) * header->aspectRatio / 256.0;
            
                token = COM_Parse(&ref);
                if (Q_stricmp(token, "}"))
                {
                    error = qtrue;
                    break;
                }
            }

            if (error) {
                break;
            }

            token = COM_Parse(&ref);
            if (Q_stricmp(token, "}"))
            {
                error = qtrue;
                break;
            }
        }
        else if (!Q_stricmp(token, "height"))
        {
            // parse the header height
            token = COM_Parse(&ref);
            header->height = atof(token);
        }
        else if (!Q_stricmp(token, "aspect"))
        {
            // parse the aspect ratio
            token = COM_Parse(&ref);
            header->aspectRatio = atof(token);
        }
        else
        {
            // unknown token
            break;
        }
    }

    if (token[0])
    {
        ri.Printf(PRINT_WARNING, "WARNING: Unknown token '%s' parsing font '%s'\n", token, name);
        error = qtrue;
    }

    R_LoadFontShader(header);
    if (!header->height || !header->aspectRatio) {
        // invalid height or aspect ratio
        error = qtrue;
    }

    ri.FS_FreeFile(theFile);
    if (error)
    {
        ri.Printf(3, "WARNING: Error parsing font %s.\n", name);
        return NULL;
    }
    else
    {
        s_numLoadedFonts++;
        return header;
    }

    return NULL;
}

void R_LoadFontShader(fontheader_t* font)
{
    int i;
    int save;
    char filename[64];
    shader_t* fontshader;

    save = r_sequencenumber;
    r_sequencenumber = -1;
    Com_sprintf(filename, sizeof(filename), "gfx/fonts/%s", font->name);
    font->shader = R_FindShader(filename, -1, qfalse, qfalse, qfalse, qfalse);
    r_sequencenumber = save;

    if (!font->shader) {
        ri.Error(ERR_DROP, "Could not load font shader for %s\n", filename);
    }

    fontshader = (shader_t*)font->shader;
    if (fontshader->numUnfoggedPasses > 0)
    {
        for (i = 0; i < fontshader->numUnfoggedPasses; i++)
        {
            if (fontshader->unfoggedStages[0] != NULL && fontshader->unfoggedStages[0]->active)
            {
                fontshader->unfoggedStages[0]->rgbGen = CGEN_GLOBAL_COLOR;
                fontshader->unfoggedStages[0]->alphaGen = AGEN_GLOBAL_ALPHA;
            }
        }

        font->trhandle = r_sequencenumber;
    }
    else
    {
        font->trhandle = r_sequencenumber;
    }
}

void R_DrawString(const fontheader_t* font, const char* text, float x, float y, int maxlen, qboolean bVirtualScreen) {
    // FIXME: unimplemented
}

void R_DrawFloatingString(const fontheader_t* font, const char* text, const vec3_t org, const vec4_t color, float scale, int maxlen) {
    // FIXME: unimplemented
}

float R_GetFontHeight(const fontheader_t* font)
{
    if (!font) {
        return 0.0;
    }

    return font->height * s_fontGeneralScale * s_fontHeightScale;
}

float R_GetFontStringWidth(const fontheader_t* font, const char* s)
{
    float widths;
    int i;

    widths = 0.0;

    if (!font) {
        return 0.0;
    }

    for (i = 0; s[i]; i++)
    {
        int indirected;
        char c = *s;

        if (c == 9)
        {
            indirected = font->indirection[32];
            if (indirected != -1) {
                widths += font->locations[indirected].size[0] * 3.0;
            } else {
                Com_Printf("R_GetFontStringWidth: no space-character in font!\n");
            }
        }
        else
        {
            indirected = font->indirection[c];
            if (indirected != -1) {
                widths += font->locations[indirected].size[0];
            } else {
                Com_Printf("R_GetFontStringWidth: no 0x%02x-character in font!\n", c);
            }
        }
    }

    return widths * s_fontGeneralScale * 256.0;
}
