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

#include "Transformers.h"
#include "../format/FlatIconFormat.h"
#include <Message.h>
#include <new>

using std::nothrow;

_USING_ICON_NAMESPACE

AffineTransformer::AffineTransformer()
{
    fMatrix[0] = 1.0; fMatrix[1] = 0.0;
    fMatrix[2] = 0.0; fMatrix[3] = 1.0;
    fMatrix[4] = 0.0; fMatrix[5] = 0.0;
}

AffineTransformer::AffineTransformer(BMessage* archive)
{
    if (!archive) {
        AffineTransformer();
        return;
    }

    int32 size = 6;
    const void* matrix;
    ssize_t dataSize = size * sizeof(double);
    if (archive->FindData("matrix", B_DOUBLE_TYPE,
                          &matrix, &dataSize) == B_OK
        && dataSize == (ssize_t)(size * sizeof(double))) {
        load_from((const double*)matrix);
    } else {
        AffineTransformer();
    }
}

AffineTransformer::~AffineTransformer()
{
}

status_t
AffineTransformer::Archive(BMessage* into) const
{
    into->what = archive_code;
    double matrix[6];
    store_to(matrix);
    return into->AddData("matrix", B_DOUBLE_TYPE, matrix, 6 * sizeof(double));
}

Transformer*
AffineTransformer::Clone() const
{
    AffineTransformer* clone = new (nothrow) AffineTransformer();
    if (clone) {
        double matrix[6];
        store_to(matrix);
        clone->load_from(matrix);
    }
    return clone;
}

void
AffineTransformer::store_to(double* matrix) const
{
    for (int i = 0; i < 6; i++)
        matrix[i] = fMatrix[i];
}

void
AffineTransformer::load_from(const double* matrix)
{
    for (int i = 0; i < 6; i++)
        fMatrix[i] = matrix[i];
}

ContourTransformer::ContourTransformer()
    : fWidth(1.0)
    , fLineJoin(agg::miter_join)
    , fMiterLimit(4.0)
{
}

ContourTransformer::ContourTransformer(BMessage* archive)
    : fWidth(1.0)
    , fLineJoin(agg::miter_join)
    , fMiterLimit(4.0)
{
    if (!archive)
        return;

    int32 mode;
    if (archive->FindInt32("line join", &mode) == B_OK)
        fLineJoin = (agg::line_join_e)mode;

    double value;
    if (archive->FindDouble("width", &value) == B_OK)
        fWidth = value;

    if (archive->FindDouble("miter limit", &value) == B_OK)
        fMiterLimit = value;
}

ContourTransformer::~ContourTransformer()
{
}

status_t
ContourTransformer::Archive(BMessage* into) const
{
    into->what = archive_code;
    status_t ret = into->AddInt32("line join", fLineJoin);
    if (ret == B_OK)
        ret = into->AddDouble("width", fWidth);
    if (ret == B_OK)
        ret = into->AddDouble("miter limit", fMiterLimit);
    return ret;
}

Transformer*
ContourTransformer::Clone() const
{
    ContourTransformer* clone = new (nothrow) ContourTransformer();
    if (clone) {
        clone->fWidth = fWidth;
        clone->fLineJoin = fLineJoin;
        clone->fMiterLimit = fMiterLimit;
    }
    return clone;
}

PerspectiveTransformer::PerspectiveTransformer()
{
    fMatrix[0] = 1.0; fMatrix[1] = 0.0; fMatrix[2] = 0.0;
    fMatrix[3] = 0.0; fMatrix[4] = 1.0; fMatrix[5] = 0.0;
    fMatrix[6] = 0.0; fMatrix[7] = 0.0; fMatrix[8] = 1.0;
}

PerspectiveTransformer::PerspectiveTransformer(BMessage* archive)
{
    if (!archive) {
        PerspectiveTransformer();
        return;
    }

    double matrix[9];
    for (int i = 0; i < 9; i++) {
        if (archive->FindDouble("matrix", i, &matrix[i]) != B_OK)
            matrix[i] = (i == 0 || i == 4 || i == 8) ? 1.0 : 0.0;
    }
    load_from(matrix);
}

PerspectiveTransformer::~PerspectiveTransformer()
{
}

status_t
PerspectiveTransformer::Archive(BMessage* into) const
{
    into->what = archive_code;
    double matrix[9];
    store_to(matrix);
    status_t ret = B_OK;
    for (int i = 0; i < 9; i++) {
        ret = into->AddDouble("matrix", matrix[i]);
        if (ret != B_OK)
            break;
    }
    return ret;
}

Transformer*
PerspectiveTransformer::Clone() const
{
    PerspectiveTransformer* clone = new (nothrow) PerspectiveTransformer();
    if (clone) {
        double matrix[9];
        store_to(matrix);
        clone->load_from(matrix);
    }
    return clone;
}

void
PerspectiveTransformer::store_to(double* matrix) const
{
    for (int i = 0; i < 9; i++)
        matrix[i] = fMatrix[i];
}

void
PerspectiveTransformer::load_from(const double* matrix)
{
    for (int i = 0; i < 9; i++)
        fMatrix[i] = matrix[i];
}

StrokeTransformer::StrokeTransformer()
    : fWidth(1.0)
    , fLineCap(agg::butt_cap)
    , fLineJoin(agg::miter_join)
    , fMiterLimit(4.0)
{
}

StrokeTransformer::StrokeTransformer(BMessage* archive)
    : fWidth(1.0)
    , fLineCap(agg::butt_cap)
    , fLineJoin(agg::miter_join)
    , fMiterLimit(4.0)
{
    if (!archive)
        return;

    int32 mode;
    if (archive->FindInt32("line cap", &mode) == B_OK)
        fLineCap = (agg::line_cap_e)mode;

    if (archive->FindInt32("line join", &mode) == B_OK)
        fLineJoin = (agg::line_join_e)mode;

    double value;
    if (archive->FindDouble("width", &value) == B_OK)
        fWidth = value;

    if (archive->FindDouble("miter limit", &value) == B_OK)
        fMiterLimit = value;
}

StrokeTransformer::~StrokeTransformer()
{
}

status_t
StrokeTransformer::Archive(BMessage* into) const
{
    into->what = archive_code;
    status_t ret = into->AddInt32("line cap", fLineCap);
    if (ret == B_OK)
        ret = into->AddInt32("line join", fLineJoin);
    if (ret == B_OK)
        ret = into->AddDouble("width", fWidth);
    if (ret == B_OK)
        ret = into->AddDouble("miter limit", fMiterLimit);
    return ret;
}

Transformer*
StrokeTransformer::Clone() const
{
    StrokeTransformer* clone = new (nothrow) StrokeTransformer();
    if (clone) {
        clone->fWidth = fWidth;
        clone->fLineCap = fLineCap;
        clone->fLineJoin = fLineJoin;
        clone->fMiterLimit = fMiterLimit;
    }
    return clone;
}

Transformer*
TransformerFactory::TransformerFor(BMessage* archive)
{
    if (!archive)
        return NULL;

    switch (archive->what) {
        case AffineTransformer::archive_code:
            return new (nothrow) AffineTransformer(archive);
        case PerspectiveTransformer::archive_code:
            return new (nothrow) PerspectiveTransformer(archive);
        case ContourTransformer::archive_code:
            return new (nothrow) ContourTransformer(archive);
        case StrokeTransformer::archive_code:
            return new (nothrow) StrokeTransformer(archive);
    }

    return NULL;
}
