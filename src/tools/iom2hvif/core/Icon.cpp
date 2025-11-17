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

#include "Icon.h"
#include "Shape.h"
#include "Style.h"
#include "VectorPath.h"
#include <new>

using std::nothrow;

_USING_ICON_NAMESPACE

Icon::Icon()
    : fStyles(true)
    , fPaths(true)
    , fShapes(true)
{
}

Icon::Icon(const Icon& other)
    : fStyles(true)
    , fPaths(true)
    , fShapes(true)
{
    int32 styleCount = other.fStyles.CountItems();
    for (int32 i = 0; i < styleCount; i++) {
        Style* style = other.fStyles.ItemAtFast(i);
        Style* clone = new (nothrow) Style(*style);
        if (clone)
            fStyles.AddItem(clone);
    }

    int32 pathCount = other.fPaths.CountItems();
    for (int32 i = 0; i < pathCount; i++) {
        VectorPath* path = other.fPaths.ItemAtFast(i);
        VectorPath* clone = new (nothrow) VectorPath(*path);
        if (clone)
            fPaths.AddItem(clone);
    }

    int32 shapeCount = other.fShapes.CountItems();
    for (int32 i = 0; i < shapeCount; i++) {
        Shape* shape = other.fShapes.ItemAtFast(i);
        Shape* clone = shape->Clone();
        if (!clone)
            continue;
        
        fShapes.AddItem(clone);

        int32 styleIndex = other.fStyles.IndexOf(shape->GetStyle());
        clone->SetStyle(fStyles.ItemAt(styleIndex));

        clone->Paths()->MakeEmpty();
        pathCount = shape->Paths()->CountItems();
        for (int32 j = 0; j < pathCount; j++) {
            VectorPath* remote = shape->Paths()->ItemAtFast(j);
            int32 index = other.fPaths.IndexOf(remote);
            VectorPath* local = fPaths.ItemAt(index);
            if (local)
                clone->Paths()->AddItem(local);
        }
    }
}

Icon::~Icon()
{
    MakeEmpty();
}

Icon*
Icon::Clone() const
{
    return new (nothrow) Icon(*this);
}

void
Icon::MakeEmpty()
{
    fShapes.MakeEmpty();
    fPaths.MakeEmpty();
    fStyles.MakeEmpty();
}
