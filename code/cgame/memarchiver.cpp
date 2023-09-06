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

#include "cg_local.h"
#include "memarchiver.h"
#include "../qcommon/vector.h"
#include "../qcommon/str.h"

MemArchiver::MemArchiver()
{
    state = MEMARC_WRITING;
    buffer = 0;
    allocatedSize = 0;
    bufferSize = 0;
}

MemArchiver::~MemArchiver()
{
    if (state == MEMARC_WRITING && buffer) {
        cgi.Free(buffer);
    }
}

void MemArchiver::SetupForWriting(size_t initialSize)
{
    state = MEMARC_WRITING;
    buffer = (byte*)cgi.Malloc(initialSize);
    allocatedSize = initialSize;
    bufferSize = 0;
}

void MemArchiver::SetupForReading(byte* buffer, size_t size)
{
    state = MEMARC_READING;
    buffer = buffer;
    allocatedSize = size;
    bufferSize = 0;
}

void MemArchiver::SetBaseTime(unsigned int time)
{
    svsTime = time;
}

size_t MemArchiver::BufferSize() const
{
    return bufferSize;
}

byte* MemArchiver::ConfiscateBuffer()
{
    byte* confiscatedBuffer = buffer;

    buffer = NULL;
    allocatedSize = 0;
    bufferSize = 0;

    return confiscatedBuffer;
}

bool MemArchiver::IsReading() const
{
    return !IsWriting();
}

bool MemArchiver::IsWriting() const
{
    return state == MEMARC_WRITING;
}

bool MemArchiver::FinishedReading() const
{
    return bufferSize == allocatedSize;
}

void MemArchiver::ArchiveByte(byte* value)
{
    ArchiveRaw(value, sizeof(byte));
}

void MemArchiver::ArchiveBoolean(qboolean* value)
{
    ArchiveRaw(value, sizeof(qboolean));
}

void MemArchiver::ArchiveChar(char* value)
{
    ArchiveRaw(value, sizeof(char));
}

void MemArchiver::ArchiveUChar(unsigned char* value)
{
    ArchiveRaw(value, sizeof(unsigned char));
}

void MemArchiver::ArchiveShort(short* value)
{
    ArchiveRaw(value, sizeof(short));
}

void MemArchiver::ArchiveInteger(int* value)
{
    ArchiveRaw(value, sizeof(int));
}

void MemArchiver::ArchiveSize(size_t* value)
{
    ArchiveRaw(value, sizeof(size_t));
}

void MemArchiver::ArchiveFloat(float* value)
{
    ArchiveRaw(value, sizeof(float));
}

void MemArchiver::ArchiveVec3(vec3_t value)
{
    ArchiveRaw(value, sizeof(vec3_t));
}

void MemArchiver::ArchiveVector(Vector* value)
{
    ArchiveFloat(&value->x);
    ArchiveFloat(&value->y);
    ArchiveFloat(&value->z);
}

void MemArchiver::ArchiveString(str* value)
{
    char tmp[4096];
    int current = 0;

    if (IsReading()) {
        for (current = 0; current < 4096; current++) {
            ArchiveReadRaw(&tmp[current], sizeof(char));
            if (!tmp[current]) {
                break;
            }
        }

        if (current == 4096) {
            tmp[current - 1] = 0;
        }

        *value = tmp;
    } else {
        ArchiveWriteRaw(value->c_str(), value->length() + 1);
    }
}

void MemArchiver::ArchiveTime(int* value)
{
    int time;

    if (IsReading()) {
        ArchiveReadRaw(&time, sizeof(time));

        if (time) {
            time += svsTime;
        }

        *value = time;
    } else {
        time = *value;
        if (time) {
            time -= svsTime;
        }

        ArchiveWriteRaw(&time, sizeof(time));
    }
}

void MemArchiver::ArchiveRaw(void* buffer, size_t size)
{
    if (IsReading()) {
        ArchiveReadRaw(buffer, size);
    } else {
        ArchiveWriteRaw(buffer, size);
    }
}

void MemArchiver::ArchiveReadRaw(void* buffer, size_t size)
{
    if (!size) {
        return;
    }

    if (size + this->bufferSize > this->allocatedSize) {
        Com_Error(ERR_DROP, "MemArchiver: tried to read past end of buffer");
    }

    memcpy(buffer, this->buffer + this->bufferSize, size);
    this->bufferSize += size;
}

void MemArchiver::ArchiveWriteRaw(const void* buffer, size_t size)
{
    if (!size) {
        return;
    }

    if (size + this->bufferSize > this->allocatedSize) {
        if (!this->allocatedSize) {
            this->allocatedSize = 1024;
            this->buffer = (byte*)cgi.Malloc(this->allocatedSize);
        } else {
            byte* newBuffer;

            this->allocatedSize = 1024;
            this->buffer = (byte*)cgi.Malloc(this->allocatedSize);

            do {
                this->allocatedSize *= 2;
            } while (size + this->bufferSize > this->allocatedSize);

            newBuffer = (byte*)cgi.Malloc(this->allocatedSize);
            memcpy(newBuffer, this->buffer, this->bufferSize);
            cgi.Free(this->buffer);
            buffer = newBuffer;
        }

        memcpy(this->buffer + this->bufferSize, buffer, size);
        this->bufferSize += size;
    }
}
