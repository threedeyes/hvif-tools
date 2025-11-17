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

#include "VectorPath.h"
#include <Message.h>
#include <TypeConstants.h>
#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using std::nothrow;

_USING_ICON_NAMESPACE

#define obj_new(type, n)        ((type *)malloc ((n) * sizeof(type)))
#define obj_renew(p, type, n)   ((type *)realloc ((void *)p, (n) * sizeof(type)))
#define obj_free                free

#define ALLOC_CHUNKS 20

VectorPath::VectorPath()
    : fPath(NULL)
    , fClosed(false)
    , fPointCount(0)
    , fAllocCount(0)
{
}

VectorPath::VectorPath(const VectorPath& from)
    : fPath(NULL)
    , fClosed(false)
    , fPointCount(0)
    , fAllocCount(0)
{
    *this = from;
}

VectorPath::VectorPath(BMessage* archive)
    : fPath(NULL)
    , fClosed(false)
    , fPointCount(0)
    , fAllocCount(0)
{
    if (!archive)
        return;

    type_code typeFound;
    int32 countFound;
    if (archive->GetInfo("point", &typeFound, &countFound) >= B_OK
        && typeFound == B_POINT_TYPE
        && _SetPointCount(countFound)) {
        
        memset((void*)fPath, 0, fAllocCount * sizeof(control_point));

        BPoint point, pointIn, pointOut;
        bool connected;
        for (int32 i = 0; i < fPointCount
                && archive->FindPoint("point", i, &point) >= B_OK
                && archive->FindPoint("point in", i, &pointIn) >= B_OK
                && archive->FindPoint("point out", i, &pointOut) >= B_OK
                && archive->FindBool("connected", i, &connected) >= B_OK; i++) {
            fPath[i].point = point;
            fPath[i].point_in = pointIn;
            fPath[i].point_out = pointOut;
            fPath[i].connected = connected;
        }
    }
    if (archive->FindBool("path closed", &fClosed) < B_OK)
        fClosed = false;
}

VectorPath::~VectorPath()
{
    if (fPath)
        obj_free(fPath);
}

status_t
VectorPath::Archive(BMessage* into, bool deep) const
{
    status_t ret = B_OK;

    if (fPointCount > 0) {
        ret = into->AddData("point", B_POINT_TYPE, &fPath[0].point,
            sizeof(BPoint), true, fPointCount);
        if (ret >= B_OK) {
            ret = into->AddData("point in", B_POINT_TYPE, &fPath[0].point_in,
                sizeof(BPoint), true, fPointCount);
        }
        if (ret >= B_OK) {
            ret = into->AddData("point out", B_POINT_TYPE, &fPath[0].point_out,
                sizeof(BPoint), true, fPointCount);
        }
        if (ret >= B_OK) {
            ret = into->AddData("connected", B_BOOL_TYPE, &fPath[0].connected,
                sizeof(bool), true, fPointCount);
        }

        for (int32 i = 1; i < fPointCount && ret >= B_OK; i++) {
            ret = into->AddData("point", B_POINT_TYPE, &fPath[i].point,
                sizeof(BPoint));
            if (ret >= B_OK) {
                ret = into->AddData("point in", B_POINT_TYPE, &fPath[i].point_in,
                    sizeof(BPoint));
            }
            if (ret >= B_OK) {
                ret = into->AddData("point out", B_POINT_TYPE,
                    &fPath[i].point_out, sizeof(BPoint));
            }
            if (ret >= B_OK) {
                ret = into->AddData("connected", B_BOOL_TYPE,
                    &fPath[i].connected, sizeof(bool));
            }
        }
    }

    if (ret >= B_OK)
        ret = into->AddBool("path closed", fClosed);

    return ret;
}

VectorPath&
VectorPath::operator=(const VectorPath& from)
{
    _SetPointCount(from.fPointCount);
    fClosed = from.fClosed;
    if (fPath) {
        memcpy((void*)fPath, from.fPath, fPointCount * sizeof(control_point));
    }
    return *this;
}

bool
VectorPath::operator==(const VectorPath& other) const
{
    if (fClosed != other.fClosed)
        return false;
    if (fPointCount != other.fPointCount)
        return false;
    if (fPath == NULL && other.fPath == NULL)
        return true;
    if (fPath == NULL || other.fPath == NULL)
        return false;

    for (int32 i = 0; i < fPointCount; i++) {
        if (fPath[i].point != other.fPath[i].point
            || fPath[i].point_in != other.fPath[i].point_in
            || fPath[i].point_out != other.fPath[i].point_out
            || fPath[i].connected != other.fPath[i].connected) {
            return false;
        }
    }
    return true;
}

void
VectorPath::MakeEmpty()
{
    _SetPointCount(0);
}

bool
VectorPath::AddPoint(BPoint point)
{
    int32 index = fPointCount;
    if (_SetPointCount(fPointCount + 1)) {
        _SetPoint(index, point);
        return true;
    }
    return false;
}

bool
VectorPath::AddPoint(const BPoint& point, const BPoint& pointIn,
    const BPoint& pointOut, bool connected)
{
    int32 index = fPointCount;
    if (_SetPointCount(fPointCount + 1)) {
        _SetPoint(index, point, pointIn, pointOut, connected);
        return true;
    }
    return false;
}

bool
VectorPath::AddPoint(BPoint point, int32 index)
{
    if (index < 0)
        index = 0;
    if (index > fPointCount)
        index = fPointCount;

    if (_SetPointCount(fPointCount + 1)) {
        if (index < fPointCount - 1) {
            for (int32 i = fPointCount; i > index; i--) {
                fPath[i].point = fPath[i - 1].point;
                fPath[i].point_in = fPath[i - 1].point_in;
                fPath[i].point_out = fPath[i - 1].point_out;
                fPath[i].connected = fPath[i - 1].connected;
            }
        }
        _SetPoint(index, point);
        return true;
    }
    return false;
}

bool
VectorPath::RemovePoint(int32 index)
{
    if (index >= 0 && index < fPointCount) {
        if (index < fPointCount - 1) {
            for (int32 i = index; i < fPointCount - 1; i++) {
                fPath[i].point = fPath[i + 1].point;
                fPath[i].point_in = fPath[i + 1].point_in;
                fPath[i].point_out = fPath[i + 1].point_out;
                fPath[i].connected = fPath[i + 1].connected;
            }
        }
        fPointCount -= 1;
        return true;
    }
    return false;
}

bool
VectorPath::SetPoint(int32 index, BPoint point)
{
    if (index == fPointCount)
        index = 0;
    if (index >= 0 && index < fPointCount) {
        BPoint offset = point - fPath[index].point;
        fPath[index].point = point;
        fPath[index].point_in += offset;
        fPath[index].point_out += offset;
        return true;
    }
    return false;
}

bool
VectorPath::SetPoint(int32 index, BPoint point, BPoint pointIn, BPoint pointOut,
    bool connected)
{
    if (index == fPointCount)
        index = 0;
    if (index >= 0 && index < fPointCount) {
        fPath[index].point = point;
        fPath[index].point_in = pointIn;
        fPath[index].point_out = pointOut;
        fPath[index].connected = connected;
        return true;
    }
    return false;
}

bool
VectorPath::GetPointAt(int32 index, BPoint& point) const
{
    if (index == fPointCount)
        index = 0;
    if (index >= 0 && index < fPointCount) {
        point = fPath[index].point;
        return true;
    }
    return false;
}

bool
VectorPath::GetPointsAt(int32 index, BPoint& point, BPoint& pointIn,
    BPoint& pointOut, bool* connected) const
{
    if (index >= 0 && index < fPointCount) {
        point = fPath[index].point;
        pointIn = fPath[index].point_in;
        pointOut = fPath[index].point_out;
        if (connected)
            *connected = fPath[index].connected;
        return true;
    }
    return false;
}

int32
VectorPath::CountPoints() const
{
    return fPointCount;
}

void
VectorPath::SetClosed(bool closed)
{
    fClosed = closed;
}

void
VectorPath::_SetPoint(int32 index, BPoint point)
{
    fPath[index].point = point;
    fPath[index].point_in = point;
    fPath[index].point_out = point;
    fPath[index].connected = true;
}

void
VectorPath::_SetPoint(int32 index, const BPoint& point, const BPoint& pointIn,
    const BPoint& pointOut, bool connected)
{
    fPath[index].point = point;
    fPath[index].point_in = pointIn;
    fPath[index].point_out = pointOut;
    fPath[index].connected = connected;
}

bool
VectorPath::_SetPointCount(int32 count)
{
    if (count >= fAllocCount) {
        fAllocCount = ((count) / ALLOC_CHUNKS + 1) * ALLOC_CHUNKS;
        if (fPath)
            fPath = obj_renew(fPath, control_point, fAllocCount);
        else
            fPath = obj_new(control_point, fAllocCount);

        if (fPath != NULL) {
            memset((void*)(fPath + fPointCount), 0,
                (fAllocCount - fPointCount) * sizeof(control_point));
        }
    }

    if (fPath) {
        fPointCount = count;
    } else {
        fPointCount = 0;
        fAllocCount = 0;
    }

    return fPath != NULL;
}
