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

// archive.cpp: OpenMoHAA Archiver

#include "glb_local.h"
#include "archive.h"
#include "level.h"
#include <lz77.h>

#ifdef GAME_DLL
#    include "../fgame/entity.h"
#endif

enum {
    ARC_NULL,
    ARC_Vector,
    ARC_Vec2,
    ARC_Vec3,
    ARC_Vec4,
    ARC_Integer,
    ARC_Unsigned,
    ARC_Byte,
    ARC_Char,
    ARC_Short,
    ARC_UnsignedShort,
    ARC_Float,
    ARC_Double,
    ARC_Boolean,
    ARC_String,
    ARC_Raw,
    ARC_Object,
    ARC_ObjectPointer,
    ARC_SafePointer,
    ARC_EventPointer,
    ARC_Quat,
    ARC_Entity,
    ARC_Bool,
    ARC_Position,
    ARC_Size,
    ARC_NUMTYPES
};

static const char *typenames[] = {
    "NULL",   "vector",        "vec2",           "vec3",         "vec4",       "int",      "unsigned", "byte",
    "char",   "short",         "unsigned short", "float",        "double",     "qboolean", "string",   "raw data",
    "object", "objectpointer", "safepointer",    "eventpointer", "quaternion", "entity",   "bool",     "position",
    "size"};

#define ArchiveHeader  (*(int *)"MHAA")
#define ArchiveVersion 14                             // This must be changed any time the format changes!
#define ArchiveInfo    "OPENMOHAA Archive Version 14" // This must be changed any time the format changes!

ArchiveFile::ArchiveFile()
{
    length       = 0;
    buffer       = 0;
    pos          = 0;
    bufferlength = 0;
    writing      = 0;
    opened       = 0;
}

ArchiveFile::~ArchiveFile()
{
    Close();
}

void ArchiveFile::Close()
{
    if (writing) {
        gi.FS_WriteFile(filename.c_str(), buffer, length);
    }

    if (buffer) {
        gi.Free((void *)buffer);
        buffer = NULL;
    }

    writing  = false;
    filename = "";
    length   = 0;
    pos      = 0;
}

const char *ArchiveFile::Filename(void)
{
    return filename.c_str();
}

qboolean ArchiveFile::Compress()
{
    byte  *tempbuf;
    size_t out_len;
    size_t tempbuf_len;

    tempbuf_len = (length >> 6) + length + 27;
    tempbuf     = (byte *)gi.Malloc(tempbuf_len);

    // Set the signature
    tempbuf[0]               = 'C';
    tempbuf[1]               = 'S';
    tempbuf[2]               = 'V';
    tempbuf[3]               = 'G';
    *(size_t *)(tempbuf + 4) = length;

    // Compress the data
    if (g_lz77.Compress(buffer, length, tempbuf + 8, &out_len)) {
        gi.Error(ERR_DROP, "Compression of SaveGame Failed!\n");
        return false;
    }

    gi.Free(buffer);
    buffer       = tempbuf;
    length       = out_len + 8;
    bufferlength = tempbuf_len;

    return true;
}

size_t ArchiveFile::Length(void)
{
    return length;
}

size_t ArchiveFile::Pos(void)
{
    return pos - buffer;
}

size_t ArchiveFile::Tell(void)
{
    return pos - buffer;
}

qboolean ArchiveFile::Seek(size_t newpos)
{
    if (!buffer) {
        return false;
    }

    if (newpos > length) {
        return false;
    }

    pos = buffer + newpos;

    return true;
}

qboolean ArchiveFile::OpenRead(const char *name)
{
    byte *tempbuf;
    assert(name);

    assert(!buffer);
    Close();

    if (!name) {
        return false;
    }

    length = gi.FS_ReadFile(name, (void **)&tempbuf, qtrue);
    if (length == (size_t)(-1) || length == 0) {
        return false;
    }
    // create our own space
    buffer       = (byte *)gi.Malloc(length);
    bufferlength = length;
    // copy the file over to our space
    memcpy(buffer, tempbuf, length);
    // free the file
    gi.FS_FreeFile(tempbuf);
    // set the file name
    filename = name;

    pos     = buffer;
    writing = false;
    opened  = true;

    char FileHeader[4];
    Read(FileHeader, sizeof(FileHeader));

    if (FileHeader[0] != 'C' || FileHeader[1] != 'S' || FileHeader[2] != 'V' || FileHeader[3] != 'G') {
        pos = buffer;
    } else {
        uint32_t new_len;
        size_t   iCSVGLength;

        new_len = 0;
        Read(&new_len, sizeof(uint32_t));
        tempbuf = (byte *)gi.Malloc(new_len);

        if (g_lz77.Decompress(pos, length - 8, tempbuf, &iCSVGLength) || iCSVGLength != new_len) {
            gi.Error(ERR_DROP, "Decompression of save game failed\n");
            return false;
        }

        gi.Free(buffer);

        buffer       = tempbuf;
        length       = iCSVGLength;
        bufferlength = length;
        pos          = buffer;
    }

    return true;
}

qboolean ArchiveFile::OpenWrite(const char *name)
{
    this->length = 0;
    // 4 MiB buffer
    this->bufferlength = 4 * 1024 * 1024;
    this->buffer       = (byte *)gi.Malloc(bufferlength);
    this->filename     = name;
    this->pos          = buffer;
    this->writing      = true;
    this->opened       = true;

    return true;
}

qboolean ArchiveFile::Read(void *dest, size_t size)
{
    if (!size) {
        return false;
    }

    if ((pos + size) > (buffer + length)) {
        return false;
    }

    memcpy(dest, pos, size);
    pos += size;

    return true;
}

qboolean ArchiveFile::Write(const void *source, size_t size)
{
    if ((pos + size) > (buffer + bufferlength)) {
        byte *oldbuf;

        do {
            bufferlength *= 2;
        } while ((pos + size) > (buffer + bufferlength));

        oldbuf = buffer;
        // reallocate a bigger buffer
        buffer = (byte *)gi.Malloc(bufferlength);
        memcpy(buffer, oldbuf, length);
        // free the old buffer
        gi.Free(oldbuf);
        // set the position with the new buffer
        pos = buffer + (pos - oldbuf);
    }

    memcpy(pos, source, size);
    pos += size;

    if (length < (pos - buffer)) {
        length = (pos - buffer);
    }

    return true;
}

Archiver::Archiver()
{
    archivemode = ARCHIVE_WRITE;
    fileerror   = false;
    harderror   = true;
    Reset();
    silent = false;

    assert((sizeof(typenames) / sizeof(typenames[0])) == ARC_NUMTYPES);
}

Archiver::~Archiver()
{
    if (archivemode != ARCHIVE_NONE) {
        Close();
    }
}

void Archiver::FileError(const char *fmt, ...)
{
    va_list argptr;
    char    text[1024];

    va_start(argptr, fmt);
    vsprintf(text, fmt, argptr);
    va_end(argptr);

    fileerror = true;
    Close();
    if (archivemode == ARCHIVE_READ) {
        if (harderror) {
            gi.Error(ERR_DROP, "Error while loading %s : %s\n", filename.c_str(), text);
        } else if (!silent) {
            gi.Printf("Error while loading %s : %s\n", filename.c_str(), text);
        }
    } else if (archivemode == ARCHIVE_WRITE) {
        if (harderror) {
            gi.Error(ERR_DROP, "Error while writing to %s : %s\n", filename.c_str(), text);
        } else if (!silent) {
            gi.Printf("Error while writing to %s : %s\n", filename.c_str(), text);
        }
    } else {
        if (harderror) {
            gi.Error(ERR_DROP, "Error while neither reading nor writing: %s\n", text);
        } else {
            gi.Printf("Error while neither reading nor writing: %s\n", text);
        }
    }
}

void Archiver::Close(void)
{
    if (archivemode == ARCHIVE_NONE) {
        // nothing to process
        return;
    }

    if (archivemode == ARCHIVE_WRITE) {
        int    numobjects;
        size_t pos;

        // write out the number of classpointers
        pos = archivefile.Tell();
        archivefile.Seek(numclassespos);
        numobjects = classpointerList.NumObjects();
        ArchiveInteger(&numobjects);
        // compress the file
        archivefile.Seek(pos);
        archivefile.Compress();
    }

    archivefile.Close();

    if (archivemode == ARCHIVE_READ) {
        int              i, num;
        pointer_fixup_t *fixup;

        num = fixupList.NumObjects();
        for (i = 1; i <= num; i++) {
            fixup = fixupList.ObjectAt(i);
            if (fixup->type == pointer_fixup_ptr) {
                LightClass **fixupptr;
                fixupptr  = (LightClass **)fixup->ptr;
                *fixupptr = classpointerList.ObjectAt(fixup->index);
            } else if (fixup->type == pointer_fixup_normal) {
                Class **fixupptr;
                fixupptr  = (Class **)fixup->ptr;
                *fixupptr = static_cast<Class *>(classpointerList.ObjectAt(fixup->index));
            } else if (fixup->type == pointer_fixup_safe) {
                SafePtrBase *fixupptr;
                fixupptr = (SafePtrBase *)fixup->ptr;
                fixupptr->InitSafePtr(static_cast<Class *>(classpointerList.ObjectAt(fixup->index)));
            }
            delete fixup;
        }
        fixupList.FreeObjectList();
        classpointerList.FreeObjectList();
    }

    archivemode = ARCHIVE_NONE;
}

/****************************************************************************************

File Read/Write functions

*****************************************************************************************/

qboolean Archiver::Read(const char *name, qboolean harderror)
{
    unsigned header;
    unsigned version;
    str      info;
    int      num;
    int      i;
    Class   *null;

    this->harderror   = harderror;
    this->fileerror   = false;
    this->archivemode = ARCHIVE_READ;
    this->filename    = name;

    if (!archivefile.OpenRead(filename.c_str())) {
        if (harderror) {
            FileError("Couldn't open file.");
        }
        return false;
    }

    ArchiveUnsigned(&header);
    if (header != ArchiveHeader) {
        archivefile.Close();
        FileError("Not a valid MOHAA archive.");
        return false;
    }

    ArchiveUnsigned(&version);
    if (version > ArchiveVersion) {
        archivefile.Close();
        FileError("Archive is from version %u.  Check http://www.x-null.net for an update.", version);
        return false;
    }

    if (version < ArchiveVersion) {
        archivefile.Close();
        FileError("Archive is out of date.");
        return false;
    }

    ArchiveString(&info);
    gi.DPrintf("%s\n", info.c_str());

    // setup out class pointers
    ArchiveInteger(&num);
    classpointerList.Resize(num);
    null = NULL;
    for (i = 1; i <= num; i++) {
        classpointerList.AddObject(null);
    }

    return true;
}

qboolean Archiver::Create(const char *name, qboolean harderror)
{
    unsigned header;
    unsigned version;
    str      info;
    int      numZero = 0;

    this->harderror   = harderror;
    this->fileerror   = false;
    this->archivemode = ARCHIVE_WRITE;
    this->filename    = name;

    if (!archivefile.OpenWrite(filename.c_str())) {
        FileError("Couldn't open file.");
        return false;
    }

    header = ArchiveHeader;
    ArchiveUnsigned(&header);
    version = ArchiveVersion;
    ArchiveUnsigned(&version);
    info = ArchiveInfo;
    ArchiveString(&info);

    numclassespos = archivefile.Tell();
    ArchiveInteger(&numZero);
    Reset();

    return true;
}

/****************************************************************************************

File Archive functions

*****************************************************************************************/

//#define ARCHIVE_USE_TYPES 1

#define ARCHIVE(func, type)                       \
    void Archiver::Archive##func(type *v)         \
                                                  \
    {                                             \
        ArchiveData(ARC_##func, v, sizeof(type)); \
    }

ARCHIVE(Vector, Vector);
ARCHIVE(Integer, int);
ARCHIVE(Unsigned, unsigned);
ARCHIVE(Byte, byte);
ARCHIVE(Char, char);
ARCHIVE(Short, short);
ARCHIVE(UnsignedShort, unsigned short);
ARCHIVE(Float, float);
ARCHIVE(Double, double);
ARCHIVE(Boolean, qboolean);
ARCHIVE(Quat, Quat);
ARCHIVE(Bool, bool);
ARCHIVE(Position, int);
ARCHIVE(Size, long);

void Archiver::ArchiveSvsTime(int *time)
{
#ifdef GAME_DLL
    if (archivemode == ARCHIVE_READ) {
        ArchiveInteger(time);
        gi.AddSvsTimeFixup(time);
    } else {
        *time -= level.svsTime;
        ArchiveInteger(time);
        *time += level.svsTime;
    }
#endif
}

void Archiver::ArchiveVec2(vec2_t vec)
{
    ArchiveData(ARC_Vec2, vec, sizeof(vec2_t));
}

void Archiver::ArchiveVec3(vec3_t vec)
{
    ArchiveData(ARC_Vec3, vec, sizeof(vec3_t));
}

void Archiver::ArchiveVec4(vec4_t vec)
{
    ArchiveData(ARC_Vec4, vec, sizeof(vec4_t));
}

void Archiver::ArchiveObjectPointer(LightClass **ptr)
{
    int index = 0;

    if (archivemode == ARCHIVE_READ) {
        pointer_fixup_t *fixup;
        ArchiveData(ARC_ObjectPointer, &index, sizeof(index));

        //
        // see if the variable was NULL
        //
        if (index == ARCHIVE_NULL_POINTER) {
            *ptr = NULL;
        } else {
            // init the pointer with NULL until we can fix it
            *ptr = NULL;

            fixup        = new pointer_fixup_t;
            fixup->ptr   = (void **)ptr;
            fixup->index = index;
            fixup->type  = pointer_fixup_ptr;
            fixupList.AddObject(fixup);
        }
    } else {
        if (*ptr) {
            index = classpointerList.AddUniqueObject(*ptr);
        } else {
            index = ARCHIVE_NULL_POINTER;
        }
        ArchiveData(ARC_ObjectPointer, &index, sizeof(index));
    }
}

void Archiver::ArchiveObjectPointer(Class **ptr)
{
    int index = 0;

    if (archivemode == ARCHIVE_READ) {
        pointer_fixup_t *fixup;
        ArchiveData(ARC_ObjectPointer, &index, sizeof(index));

        //
        // see if the variable was NULL
        //
        if (index == ARCHIVE_NULL_POINTER) {
            *ptr = NULL;
        } else {
            // init the pointer with NULL until we can fix it
            *ptr = NULL;

            fixup        = new pointer_fixup_t;
            fixup->ptr   = (void **)ptr;
            fixup->index = index;
            fixup->type  = pointer_fixup_normal;
            fixupList.AddObject(fixup);
        }
    } else {
        if (*ptr) {
            index = classpointerList.AddUniqueObject(*ptr);
        } else {
            index = ARCHIVE_NULL_POINTER;
        }
        ArchiveData(ARC_ObjectPointer, &index, sizeof(index));
    }
}

void Archiver::ArchiveSafePointer(SafePtrBase *ptr)
{
    int index = 0;

    if (archivemode == ARCHIVE_READ) {
        pointer_fixup_t *fixup;

        ArchiveData(ARC_SafePointer, &index, sizeof(index));

        //
        // see if the variable was NULL
        //
        if (index == ARCHIVE_NULL_POINTER) {
            ptr->InitSafePtr(NULL);
        } else {
            // init the pointer with NULL until we can fix it
            ptr->InitSafePtr(NULL);

            // Add new fixup
            fixup        = new pointer_fixup_t;
            fixup->ptr   = (void **)ptr;
            fixup->index = index;
            fixup->type  = pointer_fixup_safe;
            fixupList.AddObject(fixup);
        }
    } else {
        if (ptr->Pointer()) {
            Class *obj;

            obj   = ptr->Pointer();
            index = classpointerList.AddUniqueObject(obj);
        } else {
            index = ARCHIVE_NULL_POINTER;
        }
        ArchiveData(ARC_SafePointer, &index, sizeof(index));
    }
}

void Archiver::ArchiveEventPointer(Event **ev)
{
    int index;

    if (archivemode == ARCHIVE_READ) {
#ifndef NDEBUG
        CheckRead();
#endif
#ifdef ARCHIVE_USE_TYPES
        CheckType(ARC_EventPointer);
#endif
        ArchiveInteger(&index);

        if (!fileerror) {
            if (index == ARCHIVE_POINTER_VALID) {
                *ev = new Event;
                (*ev)->Archive(*this);
            } else {
                (*ev) = NULL;
            }
        }
    } else {
#ifndef NDEBUG
        CheckWrite();
#endif
        if (*ev) {
            index = ARCHIVE_POINTER_VALID;
        } else {
            index = ARCHIVE_NULL_POINTER;
        }

#ifdef ARCHIVE_USE_TYPES
        WriteType(ARC_EventPointer);
#endif

        ArchiveInteger(&index);
        if (*ev) {
            (*ev)->Archive(*this);
        }
    }
}

void Archiver::ArchiveRaw(void *data, size_t size)
{
    ArchiveData(ARC_Raw, data, size);
}

void Archiver::ArchiveString(str *string)
{
    if (archivemode == ARCHIVE_READ) {
        fileSize_t s;
        char      *data;

#ifndef NDEBUG
        CheckRead();
#endif
#ifdef ARCHIVE_USE_TYPES
        CheckType(ARC_String);
#endif

        if (!fileerror) {
            s = ReadSize();
            if (!fileerror) {
                data = new char[s + 1];
                if (data) {
                    if (s) {
                        archivefile.Read(data, s);
                    }
                    data[s] = 0;

                    *string = data;

                    delete[] data;
                }
            }
        }
    } else {
#ifndef NDEBUG
        CheckWrite();
#endif
#ifdef ARCHIVE_USE_TYPES
        WriteType(ARC_String);
#endif
        WriteSize((fileSize_t)string->length());
        archivefile.Write(string->c_str(), string->length());
    }
}

Class *Archiver::ReadObject(void)
{
    ClassDef *cls;
    Class    *obj;
    str       classname;
    size_t    objstart;
    size_t    endpos;
    int       index;
    size_t    size;
    qboolean  isent;
    int       type;

    CheckRead();

    type = ReadType();
    if ((type != ARC_Object) && (type != ARC_Entity)) {
        FileError("Expecting %s or %s", typenames[ARC_Object], typenames[ARC_Entity]);
    }

    size = ReadSize();
    ArchiveString(&classname);

    cls = getClass(classname.c_str());
    if (!cls) {
        FileError("Invalid class %s.", classname.c_str());
    }

#if defined(GAME_DLL)
    isent = checkInheritance(&Entity::ClassInfo, cls);
    if (type == ARC_Entity) {
        if (!isent) {
            FileError("Non-Entity class object '%s' saved as an Entity based object.", classname.c_str());
        }

        ArchiveInteger(&level.spawn_entnum);
        //
        // make sure to setup spawnflags properly
        //
        ArchiveInteger(&level.spawnflags);
    } else if (isent) {
        FileError("Entity class object '%s' saved as non-Entity based object.", classname.c_str());
    }
#else
    isent = false;
#endif

    ArchiveInteger(&index);
    objstart = archivefile.Pos();

    obj = (Class *)cls->newInstance();
    if (!obj) {
        FileError("Failed to on new instance of class %s.", classname.c_str());
    } else {
        obj->Archive(*this);
    }

    if (!fileerror) {
        endpos = archivefile.Pos();
        if ((endpos - objstart) > size) {
            FileError("Object read past end of object's data");
        } else if ((endpos - objstart) < size) {
            FileError("Object didn't read entire data from file");
        }
    }

    //
    // register this pointer with our list
    //
    classpointerList.AddObjectAt(index, obj);

    return obj;
}

void Archiver::ArchiveObject(Class *obj)
{
    str        classname;
    int        index;
    fileSize_t size;
    qboolean   isent;

    if (archivemode == ARCHIVE_READ) {
        ClassDef *cls;
        size_t    objstart;
        size_t    endpos;
        int       type;

        CheckRead();
        type = ReadType();
        if ((type != ARC_Object) && (type != ARC_Entity)) {
            FileError("Expecting %s or %s", typenames[ARC_Object], typenames[ARC_Entity]);
        }

        size = ReadSize();
        ArchiveString(&classname);

        cls = getClass(classname.c_str());
        if (!cls) {
            FileError("Invalid class %s.", classname.c_str());
        }

        if (obj->classinfo() != cls) {
            FileError(
                "Archive has a '%s' object, but was expecting a '%s' object.", classname.c_str(), obj->getClassname()
            );
        }

#if defined(GAME_DLL)
        isent = obj->isSubclassOf(Entity);
        if (type == ARC_Entity) {
            int entnum;
            if (!isent) {
                FileError("Non-Entity class object '%s' saved as an Entity based object.", classname.c_str());
            }

            ArchiveInteger(&entnum);
            ((Entity *)obj)->entnum = entnum;
            //
            // make sure to setup spawnflags properly
            //
            ArchiveInteger(&level.spawnflags);
        } else if (isent) {
            FileError("Entity class object '%s' saved as non-Entity based object.", classname.c_str());
        }
#else
        isent = false;
#endif

        ArchiveInteger(&index);
        objstart = archivefile.Pos();

        obj->Archive(*this);

        if (!fileerror) {
            endpos = archivefile.Pos();
            if ((endpos - objstart) > size) {
                FileError("Object read past end of object's data");
            } else if ((endpos - objstart) < size) {
                FileError("Object didn't read entire data from file");
            }
        }

        //
        // register this pointer with our list
        //
        classpointerList.AddObjectAt(index, obj);
    } else {
        long sizepos;
        long objstart = 0;
        long endpos;

        assert(obj);
        if (!obj) {
            FileError("NULL object in WriteObject");
        }

#if defined(GAME_DLL)
        isent = obj->isSubclassOf(Entity);
#else
        isent = false;
#endif

        CheckWrite();
        if (isent) {
            WriteType(ARC_Entity);
        } else {
            WriteType(ARC_Object);
        }

        sizepos = archivefile.Tell();
        size    = 0;
        WriteSize(size);

        classname = obj->getClassname();
        ArchiveString(&classname);

#if defined(GAME_DLL)
        if (isent) {
            // Write out the entity number
            ArchiveInteger(&((Entity *)obj)->entnum);
            //
            // make sure to setup spawnflags properly
            //
            ArchiveInteger(&((Entity *)obj)->spawnflags);
        }
#endif

        // write out pointer index for this class pointer
        index = classpointerList.AddUniqueObject(obj);
        ArchiveInteger(&index);

        if (!fileerror) {
            objstart = archivefile.Tell();
            obj->Archive(*this);
        }

        if (!fileerror) {
            endpos = archivefile.Tell();
            size   = endpos - objstart;
            archivefile.Seek(sizepos);
            WriteSize(size);

            if (!fileerror) {
                archivefile.Seek(archivefile.Length());
            }
        }
    }
}

void Archiver::ArchiveObjectPosition(LightClass *obj)
{
    int index = 0;

    if (archivemode == ARCHIVE_READ) {
        ArchivePosition(&index);
        classpointerList.AddObjectAt(index, (Class *)obj);
    } else {
        index = classpointerList.AddUniqueObject((Class *)obj);
        ArchivePosition(&index);
    }
}

qboolean Archiver::ObjectPositionExists(void *obj)
{
    return classpointerList.IndexOfObject((Class *)obj) != 0;
}

void Archiver::CheckRead(void)
{
    assert(archivemode == ARCHIVE_READ);
    if (!fileerror && (archivemode != ARCHIVE_READ)) {
        FileError("File read during a write operation.");
    }
}

void Archiver::CheckWrite(void)
{
    assert(archivemode == ARCHIVE_WRITE);
    if (!fileerror && (archivemode != ARCHIVE_WRITE)) {
        FileError("File write during a read operation.");
    }
}

qboolean Archiver::Read(str& name, qboolean harderror)
{
    return Read(name.c_str(), harderror);
}

qboolean Archiver::Create(str& name, qboolean harderror)
{
    return Create(name.c_str(), harderror);
}

qboolean Archiver::Loading(void)
{
    return (archivemode == ARCHIVE_READ) ? qtrue : qfalse;
}

qboolean Archiver::Saving(void)
{
    return (archivemode == ARCHIVE_WRITE) ? qtrue : qfalse;
}

qboolean Archiver::NoErrors(void)
{
    return fileerror ? qfalse : qtrue;
}

size_t Archiver::Counter() const
{
    return m_iNumBytesIO;
}

void Archiver::Reset()
{
    m_iNumBytesIO = 0;
}

fileSize_t Archiver::ReadSize(void)
{
    fileSize_t s;

    s = 0;
    if (!fileerror) {
        archivefile.Read(&s, sizeof(s));
    }

    return s;
}

void Archiver::CheckSize(int type, fileSize_t size)
{
    fileSize_t s;

    if (!fileerror) {
        s = ReadSize();

        if (size != s) {
            FileError("Invalid data size of %d on %s.", s, typenames[type]);
        }
    }
}

void Archiver::WriteSize(fileSize_t size)
{
    archivefile.Write(&size, sizeof(fileSize_t));
}

int Archiver::ReadType(void)
{
    int t;

    if (!fileerror) {
        archivefile.Read(&t, sizeof(t));

        return t;
    }

    return ARC_NULL;
}

void Archiver::WriteType(int type)
{
    archivefile.Write(&type, sizeof(type));
}

void Archiver::CheckType(int type)
{
    int t;

    assert((type >= 0) && (type < ARC_NUMTYPES));

    if (!fileerror) {
        t = ReadType();
        if (t != type) {
            if (t < ARC_NUMTYPES) {
                FileError("Expecting %s, Should be %s", typenames[type], typenames[t]);
                assert(0);
            } else {
                FileError("Expecting %s, Should be %i (Unknown type)", typenames[type], t);
            }
        }
    }
}

void Archiver::ArchiveData(int type, void *data, size_t size)
{
    if (archivemode == ARCHIVE_READ) {
#ifndef NDEBUG
        CheckRead();
#endif
#ifdef ARCHIVE_USE_TYPES
        CheckType(type);
#endif

        if (!fileerror && size) {
            m_iNumBytesIO += size;
            archivefile.Read(data, size);
        }
    } else {
#ifndef NDEBUG
        CheckWrite();
#endif
#ifdef ARCHIVE_USE_TYPES
        WriteType(type);
#endif

        if (!fileerror && size) {
            m_iNumBytesIO += size;
            archivefile.Write(data, size);
        }
    }
}

void Archiver::ArchiveConfigString(int cs)
{
    str s;

    if (archivemode == ARCHIVE_READ) {
        ArchiveString(&s);
        gi.setConfigstring(cs, s.c_str());
    } else {
        s = gi.getConfigstring(cs);
        ArchiveString(&s);
    }
}

void Archiver::SetSilent(bool bSilent)
{
    silent = bSilent;
}
