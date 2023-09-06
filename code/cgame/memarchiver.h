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

// DESCRIPTION:
// In-memory archiver

#include "cg_local.h"

class Vector;
class str;

typedef enum {
    MEMARC_WRITING,
    MEMARC_READING
} archiverState_e;

class MemArchiver {
public:
    MemArchiver();
    ~MemArchiver();

    void SetupForWriting(size_t initialSize);
    void SetupForReading(byte* buffer, size_t size);
    void SetBaseTime(unsigned int time);
    size_t BufferSize() const;
    byte* ConfiscateBuffer();
    bool IsReading() const;
    bool IsWriting() const;
    bool FinishedReading() const;

    void ArchiveByte(byte* value);
    void ArchiveBoolean(qboolean* value);
    void ArchiveChar(char* value);
    void ArchiveUChar(unsigned char* value);
    void ArchiveShort(short* value);
    void ArchiveInteger(int* value);
    void ArchiveSize(size_t* value);
    void ArchiveFloat(float* value);
    void ArchiveVec3(vec3_t value);
    void ArchiveVector(Vector* value);
    void ArchiveString(str* value);
    void ArchiveTime(int* value);
    void ArchiveRaw(void* buffer, size_t size);
    void ArchiveReadRaw(void* buffer, size_t size);
    void ArchiveWriteRaw(const void* buffer, size_t size);

private:
    archiverState_e state;
    byte* buffer;
    size_t bufferSize;
    size_t allocatedSize;
    int svsTime;
};
