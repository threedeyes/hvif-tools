/*
 * Copyright 2006-2025, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Based on Haiku Icon-O-Matic implementation by:
 *   Stephan Aßmus <superstippi@gmx.de>
 *   Zardshard
 *
 * Adapted as standalone converter without GUI dependencies.
 */

#include "LittleEndianBuffer.h"
#include <ByteOrder.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

_USING_ICON_NAMESPACE

#define CHUNK_SIZE 256

LittleEndianBuffer::LittleEndianBuffer()
    : fBuffer((uint8*)malloc(CHUNK_SIZE))
    , fHandle(fBuffer)
    , fBufferEnd(fBuffer + CHUNK_SIZE)
    , fSize(CHUNK_SIZE)
    , fOwnsBuffer(true)
{
}

LittleEndianBuffer::LittleEndianBuffer(size_t size)
    : fBuffer((uint8*)malloc(size))
    , fHandle(fBuffer)
    , fBufferEnd(fBuffer + size)
    , fSize(size)
    , fOwnsBuffer(true)
{
}

LittleEndianBuffer::LittleEndianBuffer(uint8* buffer, size_t size)
    : fBuffer(buffer)
    , fHandle(fBuffer)
    , fBufferEnd(fBuffer + size)
    , fSize(size)
    , fOwnsBuffer(false)
{
}

LittleEndianBuffer::~LittleEndianBuffer()
{
    if (fOwnsBuffer)
        free(fBuffer);
}

bool
LittleEndianBuffer::Write(uint8 value)
{
    if (fHandle == fBufferEnd)
        _SetSize(fSize + CHUNK_SIZE);

    if (!fBuffer)
        return false;

    *fHandle = value;
    fHandle++;
    return true;
}

bool
LittleEndianBuffer::Write(uint16 value)
{
    if ((fHandle + 1) >= fBufferEnd)
        _SetSize(fSize + CHUNK_SIZE);

    if (!fBuffer)
        return false;

    *(uint16*)fHandle = B_HOST_TO_LENDIAN_INT16(value);
    fHandle += 2;
    return true;
}

bool
LittleEndianBuffer::Write(uint32 value)
{
    if ((fHandle + 3) >= fBufferEnd)
        _SetSize(fSize + CHUNK_SIZE);

    if (!fBuffer)
        return false;

    *(uint32*)fHandle = B_HOST_TO_LENDIAN_INT32(value);
    fHandle += 4;
    return true;
}

bool
LittleEndianBuffer::Write(float value)
{
    if ((fHandle + sizeof(float) - 1) >= fBufferEnd)
        _SetSize(fSize + CHUNK_SIZE);

    if (!fBuffer)
        return false;

    *(float*)fHandle = B_HOST_TO_LENDIAN_FLOAT(value);
    fHandle += sizeof(float);
    return true;
}

bool
LittleEndianBuffer::Write(double value)
{
    if ((fHandle + sizeof(double) - 1) >= fBufferEnd)
        _SetSize(fSize + CHUNK_SIZE);

    if (!fBuffer)
        return false;

    *(double*)fHandle = B_HOST_TO_LENDIAN_DOUBLE(value);
    fHandle += sizeof(double);
    return true;
}

bool
LittleEndianBuffer::Write(const LittleEndianBuffer& other)
{
    return Write(other.Buffer(), other.SizeUsed());
}

bool
LittleEndianBuffer::Write(const uint8* buffer, size_t bytes)
{
    if (bytes == 0)
        return true;

    size_t neededSize = SizeUsed() + bytes;
    size_t newSize = fSize;
    while (newSize < neededSize)
        newSize += CHUNK_SIZE;

    if (newSize > fSize)
        _SetSize(newSize);

    if (!fBuffer)
        return false;

    memcpy(fHandle, buffer, bytes);
    fHandle += bytes;
    return true;
}

bool
LittleEndianBuffer::Read(uint8& value)
{
    if (fHandle >= fBufferEnd)
        return false;

    value = *fHandle++;
    return true;
}

bool
LittleEndianBuffer::Read(uint16& value)
{
    if ((fHandle + 1) >= fBufferEnd)
        return false;

    value = B_LENDIAN_TO_HOST_INT16(*(uint16*)fHandle);
    fHandle += 2;
    return true;
}

bool
LittleEndianBuffer::Read(uint32& value)
{
    if ((fHandle + 3) >= fBufferEnd)
        return false;

    value = B_LENDIAN_TO_HOST_INT32(*(uint32*)fHandle);
    fHandle += 4;
    return true;
}

bool
LittleEndianBuffer::Read(float& value)
{
    if ((fHandle + sizeof(float) - 1) >= fBufferEnd)
        return false;

    value = B_LENDIAN_TO_HOST_FLOAT(*(float*)fHandle);
    fHandle += sizeof(float);
    return true;
}

bool
LittleEndianBuffer::Read(double& value)
{
    if ((fHandle + sizeof(double) - 1) >= fBufferEnd)
        return false;

    value = B_LENDIAN_TO_HOST_DOUBLE(*(double*)fHandle);
    fHandle += sizeof(double);
    return true;
}

bool
LittleEndianBuffer::Read(LittleEndianBuffer& other, size_t bytes)
{
    if ((fHandle + bytes - 1) >= fBufferEnd)
        return false;

    if (other.Write(fHandle, bytes)) {
        other.fHandle -= bytes;
        fHandle += bytes;
        return true;
    }
    return false;
}

void
LittleEndianBuffer::Skip(size_t bytes)
{
    fHandle += bytes;
}

void
LittleEndianBuffer::Reset()
{
    fHandle = fBuffer;
}

void
LittleEndianBuffer::_SetSize(size_t size)
{
    if (!fOwnsBuffer) {
        fBuffer = NULL;
        return;
    }

    int32 pos = fHandle - fBuffer;
    fBuffer = (uint8*)realloc((void*)fBuffer, size);
    fHandle = fBuffer + pos;
    fBufferEnd = fBuffer + size;
    fSize = size;
}
