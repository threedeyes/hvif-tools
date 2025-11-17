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

#include "PathCommandQueue.h"
#include "../core/VectorPath.h"
#include "FlatIconFormat.h"
#include <Point.h>

_USING_ICON_NAMESPACE

PathCommandQueue::PathCommandQueue()
    : fCommandBuffer()
    , fPointBuffer()
    , fCommandByte(0)
    , fCommandPos(0)
    , fCommandCount(0)
{
}

PathCommandQueue::~PathCommandQueue()
{
}

bool
PathCommandQueue::Write(LittleEndianBuffer& buffer, const VectorPath* path,
                        uint8 pointCount)
{
    fCommandCount = 0;
    fCommandByte = 0;
    fCommandPos = 0;
    fCommandBuffer.Reset();
    fPointBuffer.Reset();

    BPoint last(B_ORIGIN);

    for (uint32 p = 0; p < pointCount; p++) {
        BPoint point, pointIn, pointOut;
        if (!path->GetPointsAt(p, point, pointIn, pointOut))
            return false;

        if (point == pointIn && point == pointOut) {
            if (point.x == last.x) {
                if (!_AppendVLine(point.y))
                    return false;
            } else if (point.y == last.y) {
                if (!_AppendHLine(point.x))
                    return false;
            } else {
                if (!_AppendLine(point))
                    return false;
            }
        } else {
            if (!_AppendCurve(point, pointIn, pointOut))
                return false;
        }

        last = point;
    }

    if (fCommandPos > 0) {
        if (!fCommandBuffer.Write(fCommandByte))
            return false;
    }

    return buffer.Write(fCommandBuffer) && buffer.Write(fPointBuffer);
}

bool
PathCommandQueue::Read(LittleEndianBuffer& buffer, VectorPath* path,
                       uint8 pointCount)
{
    fCommandCount = 0;
    fCommandByte = 0;
    fCommandPos = 0;
    fCommandBuffer.Reset();

    uint8 commandBufferSize = (pointCount + 3) / 4;
    if (!buffer.Read(fCommandBuffer, commandBufferSize))
        return false;

    BPoint last(B_ORIGIN);
    for (uint32 p = 0; p < pointCount; p++) {
        uint8 command;
        if (!_ReadCommand(command))
            return false;

        BPoint point, pointIn, pointOut;

        switch (command) {
            case PATH_COMMAND_H_LINE:
                if (!read_coord(buffer, point.x))
                    return false;
                point.y = last.y;
                pointIn = point;
                pointOut = point;
                break;
            case PATH_COMMAND_V_LINE:
                if (!read_coord(buffer, point.y))
                    return false;
                point.x = last.x;
                pointIn = point;
                pointOut = point;
                break;
            case PATH_COMMAND_LINE:
                if (!read_coord(buffer, point.x)
                    || !read_coord(buffer, point.y))
                    return false;
                pointIn = point;
                pointOut = point;
                break;
            case PATH_COMMAND_CURVE:
                if (!read_coord(buffer, point.x)
                    || !read_coord(buffer, point.y)
                    || !read_coord(buffer, pointIn.x)
                    || !read_coord(buffer, pointIn.y)
                    || !read_coord(buffer, pointOut.x)
                    || !read_coord(buffer, pointOut.y))
                    return false;
                break;
        }

        if (!path->AddPoint(point, pointIn, pointOut, false))
            return false;

        last = point;
    }

    return true;
}

bool
PathCommandQueue::_AppendHLine(float x)
{
    return _AppendCommand(PATH_COMMAND_H_LINE)
        && write_coord(fPointBuffer, x);
}

bool
PathCommandQueue::_AppendVLine(float y)
{
    return _AppendCommand(PATH_COMMAND_V_LINE)
        && write_coord(fPointBuffer, y);
}

bool
PathCommandQueue::_AppendLine(const BPoint& point)
{
    return _AppendCommand(PATH_COMMAND_LINE)
        && write_coord(fPointBuffer, point.x)
        && write_coord(fPointBuffer, point.y);
}

bool
PathCommandQueue::_AppendCurve(const BPoint& point, const BPoint& pointIn,
                                const BPoint& pointOut)
{
    return _AppendCommand(PATH_COMMAND_CURVE)
        && write_coord(fPointBuffer, point.x)
        && write_coord(fPointBuffer, point.y)
        && write_coord(fPointBuffer, pointIn.x)
        && write_coord(fPointBuffer, pointIn.y)
        && write_coord(fPointBuffer, pointOut.x)
        && write_coord(fPointBuffer, pointOut.y);
}

bool
PathCommandQueue::_AppendCommand(uint8 command)
{
    if (fCommandCount == 255)
        return false;

    fCommandByte |= command << fCommandPos;
    fCommandPos += 2;
    fCommandCount++;

    if (fCommandPos == 8) {
        uint8 commandByte = fCommandByte;
        fCommandByte = 0;
        fCommandPos = 0;
        return fCommandBuffer.Write(commandByte);
    }

    return true;
}

bool
PathCommandQueue::_ReadCommand(uint8& command)
{
    if (fCommandCount == 255)
        return false;

    if (fCommandPos == 0) {
        if (!fCommandBuffer.Read(fCommandByte))
            return false;
    }

    command = (fCommandByte >> fCommandPos) & 0x03;
    fCommandPos += 2;
    fCommandCount++;

    if (fCommandPos == 8)
        fCommandPos = 0;

    return true;
}
