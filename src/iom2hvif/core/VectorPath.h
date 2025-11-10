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

#ifndef VECTOR_PATH_H
#define VECTOR_PATH_H

#include <Rect.h>
#include "IconBuild.h"

class BMessage;

_BEGIN_ICON_NAMESPACE

struct control_point {
    BPoint point;
    BPoint point_in;
    BPoint point_out;
    bool connected;
};

class VectorPath {
public:
    VectorPath();
    VectorPath(const VectorPath& from);
    VectorPath(BMessage* archive);
    virtual ~VectorPath();

    status_t Archive(BMessage* into, bool deep = true) const;

    VectorPath& operator=(const VectorPath& from);
    bool operator==(const VectorPath& from) const;

    void MakeEmpty();

    bool AddPoint(BPoint point);
    bool AddPoint(const BPoint& point, const BPoint& pointIn,
                  const BPoint& pointOut, bool connected);
    bool AddPoint(BPoint point, int32 index);
    bool RemovePoint(int32 index);

    bool SetPoint(int32 index, BPoint point);
    bool SetPoint(int32 index, BPoint point, BPoint pointIn,
                  BPoint pointOut, bool connected);

    bool GetPointAt(int32 index, BPoint& point) const;
    bool GetPointsAt(int32 index, BPoint& point, BPoint& pointIn,
                     BPoint& pointOut, bool* connected = NULL) const;

    int32 CountPoints() const;

    void SetClosed(bool closed);
    bool IsClosed() const { return fClosed; }

    const control_point* Points() const { return fPath; }

private:
    void _SetPoint(int32 index, BPoint point);
    void _SetPoint(int32 index, const BPoint& point, const BPoint& pointIn,
                   const BPoint& pointOut, bool connected);
    bool _SetPointCount(int32 count);

    control_point* fPath;
    bool fClosed;
    int32 fPointCount;
    int32 fAllocCount;
};

_END_ICON_NAMESPACE

#endif
