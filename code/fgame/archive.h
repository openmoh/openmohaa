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

// archive.h: OpenMoHAA Archiver

#ifndef __ARCHIVE_H__
#define __ARCHIVE_H__

#include "g_local.h"
#include "class.h"
#include "str.h"
#include "vector.h"

#define ARCHIVE_NULL_POINTER             (-654321)
#define ARCHIVE_POINTER_VALID            (0)
#define ARCHIVE_POINTER_NULL             (ARCHIVE_NULL_POINTER)
#define ARCHIVE_POINTER_SELF_REFERENTIAL (-123456)
#define ARCHIVE_USE_TYPES

typedef enum {
    ARCHIVE_NONE,
    ARCHIVE_WRITE,
    ARCHIVE_READ
} archivemode_e;

enum {
    pointer_fixup_ptr,
    pointer_fixup_normal,
    pointer_fixup_safe
};

typedef struct {
    void **ptr;
    int    index;
    int    type;
} pointer_fixup_t;

using fileSize_t = uint32_t;

class ArchiveFile
{
protected:
    str    filename;
    size_t length;
    byte  *buffer;
    byte  *pos;
    size_t bufferlength;
    bool   writing;
    bool   opened;

public:
    ArchiveFile();
    ~ArchiveFile();
    void        Close();
    const char *Filename(void);
    qboolean    Compress();
    size_t      Length(void);
    size_t      Pos(void);
    size_t      Tell(void);
    qboolean    Seek(size_t newpos);
    qboolean    OpenRead(const char *name);
    qboolean    OpenWrite(const char *name);
    qboolean    Read(void *dest, size_t size);
    qboolean    Write(const void *source, size_t size);
};

class Archiver
{
private:
    Container<LightClass *>      classpointerList;
    Container<pointer_fixup_t *> fixupList;

protected:
    str         filename;
    qboolean    fileerror;
    ArchiveFile archivefile;
    int         archivemode;
    int         numclassespos;
    qboolean    harderror;
    size_t      m_iNumBytesIO;
    qboolean    silent;

    void       CheckRead(void);
    void       CheckType(int type);
    int        ReadType(void);
    fileSize_t ReadSize(void);
    void       CheckSize(int type, fileSize_t size);
    void       ArchiveData(int type, void *data, size_t size);

    void CheckWrite(void);
    void WriteType(int type);
    void WriteSize(fileSize_t size);

public:
    Archiver();
    ~Archiver();
    void FileError(const char *fmt, ...);
    void Close(void);

    qboolean Read(str& name, qboolean harderror = qtrue);
    qboolean Read(const char *name, qboolean harderror = qtrue);
    Class   *ReadObject(void);

    qboolean Create(str& name, qboolean harderror = qtrue);
    qboolean Create(const char *name, qboolean harderror = qtrue);

    qboolean Loading(void);
    qboolean Saving(void);
    qboolean NoErrors(void);

    void ArchiveVector(Vector *vec);
    void ArchiveQuat(Quat *quat);
    void ArchiveInteger(int *num);
    void ArchiveUnsigned(unsigned *unum);
    void ArchiveSize(long *unum);
    void ArchiveByte(byte *num);
    void ArchiveChar(char *ch);
    void ArchiveShort(short *num);
    void ArchiveUnsignedShort(unsigned short *num);
    void ArchiveFloat(float *num);
    void ArchiveDouble(double *num);
    void ArchiveBoolean(qboolean *boolean);
    void ArchiveString(str *string);
    void ArchiveObjectPointer(LightClass **ptr);
    void ArchiveObjectPointer(Class **ptr);
    void ArchiveObjectPosition(LightClass *obj);
    void ArchiveSafePointer(SafePtrBase *ptr);
    void ArchiveEventPointer(Event **ev);
    void ArchiveBool(bool *boolean);
    void ArchivePosition(int *pos);
    void ArchiveSvsTime(int *time);
    void ArchiveVec2(vec2_t vec);
    void ArchiveVec3(vec3_t vec);
    void ArchiveVec4(vec4_t vec);

    void ArchiveRaw(void *data, size_t size);
    void ArchiveObject(Class *obj);

    qboolean ObjectPositionExists(void *obj);

    void   Reset();
    size_t Counter() const;

    void ArchiveConfigString(int cs);
    void SetSilent(bool bSilent);
};

template<class Type>
inline void Container<Type>::Archive(Archiver& arc, void (*ArchiveFunc)(Archiver& arc, Type *obj))
{
    int num;
    int i;

    if (arc.Loading()) {
        arc.ArchiveInteger(&num);
        Resize(num);
    } else {
        num = numobjects;
        arc.ArchiveInteger(&num);
    }

    for (i = 1; i <= num; i++) {
        if (num > numobjects) {
            numobjects = num;
        }

        ArchiveFunc(arc, &objlist[i - 1]);
    }
}

template<>
inline void Container<str>::Archive(Archiver& arc)
{
    int i, num;

    if (arc.Loading()) {
        ClearObjectList();
        arc.ArchiveInteger(&num);
        Resize(num);
    } else {
        num = numobjects;
        arc.ArchiveInteger(&num);
    }
    for (i = 1; i <= num; i++) {
        arc.ArchiveString(AddressOfObjectAt(i));
    }
}

template<>
inline void Container<Vector>::Archive(Archiver& arc)
{
    int i, num;

    if (arc.Loading()) {
        ClearObjectList();
        arc.ArchiveInteger(&num);
        Resize(num);
    } else {
        num = numobjects;
        arc.ArchiveInteger(&num);
    }
    for (i = 1; i <= num; i++) {
        arc.ArchiveVector(AddressOfObjectAt(i));
    }
}

template<>
inline void Container<int>::Archive(Archiver& arc)
{
    int i, num;

    if (arc.Loading()) {
        ClearObjectList();
        arc.ArchiveInteger(&num);
        Resize(num);
    } else {
        num = numobjects;
        arc.ArchiveInteger(&num);
    }
    for (i = 1; i <= num; i++) {
        arc.ArchiveInteger(AddressOfObjectAt(i));
    }
}

template<>
inline void Container<float>::Archive(Archiver& arc)
{
    int i, num;

    if (arc.Loading()) {
        ClearObjectList();
        arc.ArchiveInteger(&num);
        Resize(num);
    } else {
        num = numobjects;
        arc.ArchiveInteger(&num);
    }
    for (i = 1; i <= num; i++) {
        arc.ArchiveFloat(AddressOfObjectAt(i));
    }
}

template<typename c>
inline void ArchiveClass(Archiver& arc, c *obj)
{
    arc.ArchiveObject(obj);
}

template<typename Type>
void Container<Type>::Archive(Archiver& arc)
{
    Archive(arc, ArchiveClass<Type>);
}

#ifndef NO_ARCHIVE

template<typename key, typename value>
void con_set<key, value>::Archive(Archiver& arc)
{
    Entry *e;
    int    hash;
    int    i;
    int    total;

    arc.ArchiveUnsigned(&tableLength);
    arc.ArchiveUnsigned(&threshold);
    arc.ArchiveUnsigned(&count);
    arc.ArchiveUnsignedShort(&tableLengthIndex);

    if (arc.Loading()) {
        if (tableLength != 1) {
            table = new Entry *[tableLength]();
            memset(table, 0, tableLength * sizeof(Entry *));
        }

        for (i = 0; i < count; i++) {
            e = new Entry;
            e->Archive(arc);

            hash = HashCode<key>(e->key) % tableLength;

            e->next     = table[hash];
            table[hash] = e;
        }
    } else {
        total = 0;

        for (i = 0; i < tableLength; i++) {
            for (e = table[i]; e != NULL; e = e->next) {
                e->Archive(arc);
                total++;
            }
        }
        assert(total == count);
    }
}

template<typename key, typename value>
void con_map<key, value>::Archive(Archiver& arc)
{
    m_con_set.Archive(arc);
}

#endif

#define ArchiveEnum(thing, type)      \
    {                                 \
        int tempInt;                  \
                                      \
        tempInt = (int)(thing);       \
        arc.ArchiveInteger(&tempInt); \
        (thing) = (type)tempInt;      \
    }

#endif // __ARCHIVE_H__
