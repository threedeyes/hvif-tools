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

#include "MessageImporter.h"
#include "../core/Icon.h"
#include "../core/Shape.h"
#include "../core/Style.h"
#include "../core/VectorPath.h"
#include "../transform/Transformers.h"
#include <ByteOrder.h>
#include <DataIO.h>
#include <Message.h>
#include <new>

using std::nothrow;

_USING_ICON_NAMESPACE

MessageImporter::MessageImporter()
{
}

MessageImporter::~MessageImporter()
{
}

status_t
MessageImporter::Import(Icon* icon, BPositionIO* stream)
{
    if (!icon || !stream)
        return B_BAD_VALUE;

    status_t ret = B_OK;

    uint32 magic = 0;
    ssize_t size = sizeof(magic);
    off_t position = stream->Position();
    ssize_t read = stream->Read(&magic, size);
    if (read != size) {
        if (read < 0)
            ret = (status_t)read;
        else
            ret = B_IO_ERROR;
        return ret;
    }

    if (B_BENDIAN_TO_HOST_INT32(magic) != kNativeIconMagicNumber) {
        if (stream->Seek(position, SEEK_SET) != position)
            return B_IO_ERROR;
    }

    BMessage archive;
    ret = archive.Unflatten(stream);
    if (ret != B_OK)
        return ret;

    Container<VectorPath>* paths = icon->Paths();
    ret = _ImportPaths(&archive, paths);
    if (ret < B_OK)
        return ret;

    Container<Style>* styles = icon->Styles();
    ret = _ImportStyles(&archive, styles);
    if (ret < B_OK)
        return ret;

    ret = _ImportShapes(&archive, paths, styles, icon->Shapes());
    if (ret < B_OK)
        return ret;

    return B_OK;
}

status_t
MessageImporter::_ImportPaths(const BMessage* archive,
                              Container<VectorPath>* paths) const
{
    BMessage allPaths;
    status_t ret = archive->FindMessage("paths", &allPaths);
    if (ret < B_OK)
        return ret;

    BMessage pathArchive;
    for (int32 i = 0;
         allPaths.FindMessage("path", i, &pathArchive) == B_OK; i++) {
        VectorPath* path = new (nothrow) VectorPath(&pathArchive);
        if (!path || !paths->AddItem(path)) {
            delete path;
            ret = B_NO_MEMORY;
        }
        if (ret < B_OK)
            break;
    }

    return ret;
}

status_t
MessageImporter::_ImportStyles(const BMessage* archive,
                               Container<Style>* styles) const
{
    BMessage allStyles;
    status_t ret = archive->FindMessage("styles", &allStyles);
    if (ret < B_OK)
        return ret;

    BMessage styleArchive;
    for (int32 i = 0;
         allStyles.FindMessage("style", i, &styleArchive) == B_OK; i++) {
        Style* style = new (nothrow) Style(&styleArchive);
        if (!style || !styles->AddItem(style)) {
            delete style;
            ret = B_NO_MEMORY;
        }
        if (ret < B_OK)
            break;
    }

    return ret;
}

status_t
MessageImporter::_ImportShapes(const BMessage* archive,
                               Container<VectorPath>* paths,
                               Container<Style>* styles,
                               Container<Shape>* shapes) const
{
    BMessage allShapes;
    status_t ret = archive->FindMessage("shapes", &allShapes);
    if (ret < B_OK)
        return ret;

    BMessage shapeArchive;
    for (int32 i = 0;
         allShapes.FindMessage("shape", i, &shapeArchive) == B_OK; i++) {

        int32 styleIndex;
        if (shapeArchive.FindInt32("style ref", &styleIndex) < B_OK)
            continue;

        Style* style = styles->ItemAt(styleIndex);
        if (style == NULL)
            continue;

        Shape* shape = new (nothrow) Shape(style);
        if (shape == NULL || !shapes->AddItem(shape)) {
            delete shape;
            ret = B_NO_MEMORY;
        }
        if (ret < B_OK)
            break;

        int32 pathIndex;
        for (int32 j = 0;
             shapeArchive.FindInt32("path ref", j, &pathIndex) == B_OK;
             j++) {
            VectorPath* path = paths->ItemAt(pathIndex);
            if (path == NULL)
                continue;
            shape->Paths()->AddItem(path);
        }

        if (ret == B_OK)
            shape->Unarchive(&shapeArchive);
    }

    return ret;
}
