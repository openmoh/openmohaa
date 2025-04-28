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

// gamescript.h: Subclass of script that preprocesses labels

#pragma once

#include "class.h"
#include "script.h"
#include "archive.h"

class Listener;
class ScriptThread;
class ScriptVariable;
class GameScript;

typedef struct {
    byte     *codepos;   // code position pointer
    const_str key;       // label name
    bool      isprivate; // new script engine implementation
} script_label_t;

struct sourceinfo_t {
    unsigned int sourcePos;
    unsigned int startLinePos;
    int          column;
    int          line;

    sourceinfo_t()
        : sourcePos(0)
        , column(0)
        , line(0)
    {}
};

class AbstractScript
{
public:
    // File variables
    const_str m_Filename;
    char     *m_SourceBuffer;
    size_t    m_SourceLength;

    // Developper variable
    con_set<const uchar *, sourceinfo_t> *m_ProgToSource;

    sourceinfo_t cachedInfo[16];
    size_t       cachedInfoIndex;

public:
    AbstractScript();

    str&      Filename(void);
    const_str ConstFilename(void);
    bool      GetSourceAt(size_t sourcePos, str *sourceLine, int& column, int& line);
    bool      GetSourceAt(const unsigned char *sourcePos, str *sourceLine, int& column, int& line);
    void      PrintSourcePos(sourceinfo_t *sourcePos, bool dev);
    void      PrintSourcePos(size_t sourcePos, bool dev);
    void      PrintSourcePos(unsigned char *m_pCodePos, bool dev);
    void      PrintSourcePos(str sourceLine, int column, int line, bool dev);
};

class StateScript : public Class
{
    friend class GameScript;

private:
    // Label list
    con_set<const_str, script_label_t> label_list;

public:
    // Parent gamescript
    GameScript *m_Parent;

public:
    StateScript();

    void Archive(Archiver& arc) override;

    bool           AddLabel(str label, unsigned char *pos, bool private_section = false);
    bool           AddLabel(const_str label, unsigned char *pos, bool private_section = false);
    unsigned char *FindLabel(str label);
    unsigned char *FindLabel(const_str label);
    const_str      NearestLabel(unsigned char *pos);
};

class CatchBlock
{
public:
    // program variable
    StateScript m_StateScript;

    // code position variables
    unsigned char *m_TryStartCodePos;
    unsigned char *m_TryEndCodePos;
};

class GameScript : public AbstractScript
{
protected:
    // try/throw variable
    Container<CatchBlock *>  m_CatchBlocks;
    Container<StateScript *> m_StateScripts;

public:
    // program variables
    StateScript    m_State;
    unsigned char *m_ProgBuffer;
    size_t         m_ProgLength;

    // compile variables
    bool successCompile;
    bool m_bPrecompiled;

    // stack variables
    unsigned int requiredStackSize;

public:
    GameScript();
    GameScript(const char *filename);
    ~GameScript();

    void        Archive(Archiver& arc);
    static void Archive(Archiver& arc, GameScript *& scr);
    void        ArchiveCodePos(Archiver& arc, unsigned char **codePos);

    void Close(void);
    void Load(const void *sourceBuffer, size_t sourceLength);

    bool GetCodePos(unsigned char *codePos, str& filename, int& pos);
    bool SetCodePos(unsigned char *& codePos, str& filename, int pos);

    unsigned int GetRequiredStackSize(void);

    qboolean labelExists(const char *name);

    StateScript *CreateCatchStateScript(unsigned char *try_begin_code_pos, unsigned char *try_end_code_pos);
    StateScript *CreateSwitchStateScript(void);

    StateScript *GetCatchStateScript(unsigned char *in, unsigned char *& out);

    bool ScriptCheck(void);
};

class ScriptThreadLabel
{
private:
    GameScript *m_Script;
    const_str   m_Label;

public:
    ScriptThreadLabel();

    ScriptThread *Create(Listener *listener) const;
    void          Execute(Listener *listener = NULL) const;
    void          Execute(Listener *listener, Event& ev) const;
    void          Execute(Listener *listener, Event *ev) const;
    void          Execute(Listener *pSelf, const SafePtr<Listener>& listener, const SafePtr<Listener>& param) const;

    void Clear();
    void Set(const char *label);
    void Set(const_str label);
    void SetScript(const ScriptVariable& label);
    void SetScript(const char *label);
    void SetScript(const_str label);
    void SetThread(const ScriptVariable& label);

    bool TrySet(const_str label);
    bool TrySet(const char *label);
    bool TrySetScript(const_str label);
    bool TrySetScript(const char *label);

    bool IsSet(void) const;
    bool IsFile(const_str filename) const;

    void GetScriptValue(ScriptVariable *var) const;

    void Archive(Archiver& arc);

    friend bool operator==(const ScriptThreadLabel& a, const ScriptThreadLabel& b);
};

inline bool operator==(const ScriptThreadLabel& a, const ScriptThreadLabel& b)
{
    return a.m_Label == b.m_Label && a.m_Script == b.m_Script;
}
