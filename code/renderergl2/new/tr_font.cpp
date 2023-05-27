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

#include "../tr_local.h"

#define MAX_LOADED_FONTS 255

static fontheader_t s_loadedFonts[MAX_LOADED_FONTS];
static int s_numLoadedFonts = 0;
static float s_fontHeightScale = 1.0;
static float s_fontGeneralScale = 1.0;
static float s_fontZ = 0.0;

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
    font->shader = R_FindShader(filename, -1, qfalse);
    r_sequencenumber = save;

    if (!font->shader) {
        ri.Error(ERR_DROP, "Could not load font shader for %s\n", filename);
    }

    fontshader = (shader_t*)font->shader;
    if (fontshader->numUnfoggedPasses > 0)
    {
        for (i = 0; i < fontshader->numUnfoggedPasses; i++)
        {
            if (fontshader->stages[0] != NULL && fontshader->stages[0]->active)
            {
                fontshader->stages[0]->rgbGen = CGEN_GLOBAL_COLOR;
                fontshader->stages[0]->alphaGen = AGEN_GLOBAL_ALPHA;
            }
        }

        font->trhandle = r_sequencenumber;
    }
    else
    {
        font->trhandle = r_sequencenumber;
    }
}

void R_DrawString(fontheader_t* font, const char* text, float x, float y, int maxlen, qboolean bVirtualScreen) {
    float charHeight;
    float startx, starty;
    int i;
    float fWidthScale, fHeightScale;

    i = 0;
    startx = x;
    starty = y;
    fWidthScale = (double)glConfig.vidWidth / 640.0;
    fHeightScale = (double)glConfig.vidHeight / 480.0;

    if (!font) {
        return;
    }

    R_SyncRenderThread();

    if (font->trhandle != r_sequencenumber) {
        font->shader = NULL;
    }

    if (!font->shader) {
        R_LoadFontShader(font);
    }

    charHeight = s_fontHeightScale * font->height * s_fontGeneralScale;
    RB_BeginSurface((shader_t*)font->shader, 0, 0);

    for (i = 0; text[i]; i++) {
        unsigned char c;
        int indirected;
        letterloc_t* loc;

        c = text[i];

        if (maxlen != -1 && i >= maxlen) {
            break;
        }

        switch (c)
        {
        case '\t':
            indirected = font->indirection[32];
            if (indirected == -1) {
                Com_DPrintf("R_DrawString: no space-character in font!\n");
            }
            else {
                x = s_fontGeneralScale * font->locations[indirected].size[0] * 256.0 * 3.0 + x;
            }
            break;

        case '\n':
            starty = charHeight + starty;
            x = startx;
            y = starty;
            break;

        case '\r':
            x = startx;
            break;

        default:
            indirected = font->indirection[c];
            if (indirected == -1)
            {
                Com_DPrintf("R_DrawString: no 0x%02x-character in font!\n", c);
                indirected = font->indirection['?'];
                if (indirected == -1) {
                    Com_DPrintf("R_DrawString: no '?' character in font!\n");
                    break;
                }
                // set the indirection for the next time
                font->indirection[c] = indirected;
            }

            if (tess.numVertexes + 4 >= SHADER_MAX_VERTEXES || tess.numIndexes + 6 >= SHADER_MAX_INDEXES) {
                RB_CheckOverflow(4, 6);
            }

            loc = &font->locations[indirected];

            // texture coordinates
            tess.texCoords[tess.numVertexes][0] = loc->pos[0];
            tess.texCoords[tess.numVertexes][1] = loc->pos[1];
            tess.texCoords[tess.numVertexes + 1][0] = loc->size[0] + loc->pos[0];
            tess.texCoords[tess.numVertexes + 1][1] = loc->pos[1];
            tess.texCoords[tess.numVertexes + 2][0] = loc->pos[0];
            tess.texCoords[tess.numVertexes + 2][1] = loc->size[1] + loc->pos[1];
            tess.texCoords[tess.numVertexes + 3][0] = loc->size[0] + loc->pos[0];
            tess.texCoords[tess.numVertexes + 3][1] = loc->size[1] + loc->pos[1];

            // vertices position
            tess.xyz[tess.numVertexes][0] = x;
            tess.xyz[tess.numVertexes][1] = y;
            tess.xyz[tess.numVertexes][2] = s_fontZ;
            tess.xyz[tess.numVertexes + 1][0] = x + s_fontGeneralScale * loc->size[0] * 256.0;
            tess.xyz[tess.numVertexes + 1][1] = y;
            tess.xyz[tess.numVertexes + 1][2] = s_fontZ;
            tess.xyz[tess.numVertexes + 2][0] = x;
            tess.xyz[tess.numVertexes + 2][1] = y + charHeight;
            tess.xyz[tess.numVertexes + 2][2] = s_fontZ;
            tess.xyz[tess.numVertexes + 3][0] = x + s_fontGeneralScale * loc->size[0] * 256.0;
            tess.xyz[tess.numVertexes + 3][1] = y + charHeight;
            tess.xyz[tess.numVertexes + 3][2] = s_fontZ;

            // indices
            tess.indexes[tess.numIndexes] = tess.numVertexes;
            tess.indexes[tess.numIndexes + 1] = tess.numVertexes + 1;
            tess.indexes[tess.numIndexes + 2] = tess.numVertexes + 2;
            tess.indexes[tess.numIndexes + 3] = tess.numVertexes + 1;
            tess.indexes[tess.numIndexes + 4] = tess.numVertexes + 3;
            tess.indexes[tess.numIndexes + 5] = tess.numVertexes + 2;

            if (bVirtualScreen)
            {
                // scale the string properly if virtual screen
                tess.xyz[tess.numVertexes][0] *= fWidthScale;
                tess.xyz[tess.numVertexes][1] *= fHeightScale;
                tess.xyz[tess.numVertexes + 1][0] *= fWidthScale;
                tess.xyz[tess.numVertexes + 1][1] *= fHeightScale;
                tess.xyz[tess.numVertexes + 2][0] *= fWidthScale;
                tess.xyz[tess.numVertexes + 2][1] *= fHeightScale;
                tess.xyz[tess.numVertexes + 3][0] *= fWidthScale;
                tess.xyz[tess.numVertexes + 3][1] *= fHeightScale;
            }

            x += s_fontGeneralScale * loc->size[0] * 256.0;
            tess.numVertexes += 4;
            tess.numIndexes += 6;
            break;
        }
    }

    RB_EndSurface();
}

void R_DrawFloatingString(fontheader_t* font, const char* text, const vec3_t org, const vec4_t color, float scale, int maxlen) {
    shader_t* fontshader;
    qhandle_t fsh;
    float charWidth, charHeight;
    int i;
    vec3_t pos;
    polyVert_t verts[4];

    if (!font) {
        return;
    }


    R_SyncRenderThread();
    if (font->trhandle != r_sequencenumber) {
        font->shader = NULL;
    }

    if (!font->shader) {
        R_LoadFontShader(font);
    }

    i = 0;
    fontshader = (shader_t*)font->shader;
    fsh = 0;

    for (i = 0; i < tr.numShaders; i++)
    {
        if (fontshader == tr.shaders[i])
        {
            fsh = i;
            break;
        }
    }

    i = 0;
    charHeight = font->height * s_fontHeightScale * s_fontGeneralScale * scale;
    VectorCopy(org, pos);

    for (i = 0; text[i]; i++) {
        unsigned char c;
        int indirected;
        letterloc_t* loc;

        c = text[i];
        indirected = font->indirection[c];
        if (indirected == -1)
        {
            Com_Printf("R_DrawFloatingString: no 0x%02x-character in font!\n", c);
            continue;
        }

        loc = &font->locations[indirected];

        // vertices color
        verts[0].modulate[0] = (int)(color[0] * 255.0);
        verts[0].modulate[1] = (int)(color[1] * 255.0);
        verts[0].modulate[2] = (int)(color[2] * 255.0);
        verts[0].modulate[3] = (int)(color[3] * 255.0);
        verts[1].modulate[0] = verts[0].modulate[0];
        verts[1].modulate[1] = verts[0].modulate[1];
        verts[1].modulate[2] = verts[0].modulate[2];
        verts[1].modulate[3] = verts[0].modulate[3];
        verts[2].modulate[0] = verts[0].modulate[0];
        verts[2].modulate[1] = verts[0].modulate[1];
        verts[2].modulate[2] = verts[0].modulate[2];
        verts[2].modulate[3] = verts[0].modulate[3];
        verts[3].modulate[0] = verts[0].modulate[0];
        verts[3].modulate[1] = verts[0].modulate[1];
        verts[3].modulate[2] = verts[0].modulate[2];
        verts[3].modulate[3] = verts[0].modulate[3];

        // texture coordinates
        verts[0].st[0] = loc->pos[0];
        verts[0].st[1] = loc->pos[1];
        verts[1].st[0] = loc->pos[0] + font->locations[indirected].size[0];
        verts[1].st[1] = loc->pos[1];
        verts[2].st[0] = verts[1].st[0];
        verts[2].st[1] = loc->pos[1] + loc->size[1];
        verts[3].st[0] = loc->pos[0];
        verts[3].st[1] = verts[2].st[1];
        VectorCopy(pos, verts[3].xyz);

        charWidth = font->locations[indirected].size[0] * 256.0 * s_fontGeneralScale * scale;
        verts[2].xyz[0] = pos[0] + tr.refdef.viewaxis[1][0] * -charWidth;
        verts[2].xyz[1] = pos[1] + tr.refdef.viewaxis[1][1] * -charWidth;
        verts[2].xyz[2] = pos[2] + tr.refdef.viewaxis[1][2] * -charWidth;
        verts[1].xyz[0] = verts[2].xyz[0] + charHeight * tr.refdef.viewaxis[2][0];
        verts[1].xyz[1] = verts[2].xyz[1] + charHeight * tr.refdef.viewaxis[2][1];
        verts[1].xyz[2] = verts[2].xyz[2] + charHeight * tr.refdef.viewaxis[2][2];
        verts[0].xyz[1] = verts[1].xyz[1] + tr.refdef.viewaxis[1][1] * charWidth;
        verts[0].xyz[2] = verts[1].xyz[2] + tr.refdef.viewaxis[1][2] * charWidth;
        verts[0].xyz[0] = verts[1].xyz[0] + tr.refdef.viewaxis[1][0] * charWidth;

        RE_AddPolyToScene(fsh, 4, verts, 1);

        pos[0] = verts[2].xyz[0];
        pos[1] = verts[2].xyz[1];
        pos[2] = verts[2].xyz[2];
    }
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
            }
            else {
                Com_Printf("R_GetFontStringWidth: no space-character in font!\n");
            }
        }
        else
        {
            indirected = font->indirection[c];
            if (indirected != -1) {
                widths += font->locations[indirected].size[0];
            }
            else {
                Com_Printf("R_GetFontStringWidth: no 0x%02x-character in font!\n", c);
            }
        }
    }

    return widths * s_fontGeneralScale * 256.0;
}
