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

#include "Shape.h"
#include "Style.h"
#include "VectorPath.h"
#include "../transform/Transformers.h"
#include <Message.h>
#include <new>

using std::nothrow;

_USING_ICON_NAMESPACE

Shape::Shape(::Style* style)
    : Transformable()
    , fPaths(new (nothrow) Container<VectorPath>(false))
    , fStyle(NULL)
    , fTransformers(true)
    , fHinting(false)
    , fMinVisibilityScale(0.0)
    , fMaxVisibilityScale(4.0)
{
    SetStyle(style);
}

Shape::Shape(const Shape& other)
    : Transformable(other)
    , fPaths(new (nothrow) Container<VectorPath>(false))
    , fStyle(NULL)
    , fTransformers(true)
    , fHinting(other.fHinting)
    , fMinVisibilityScale(other.fMinVisibilityScale)
    , fMaxVisibilityScale(other.fMaxVisibilityScale)
{
    SetStyle(other.fStyle);

    if (fPaths && other.fPaths) {
        int32 count = other.fPaths->CountItems();
        for (int32 i = 0; i < count; i++) {
            fPaths->AddItem(other.fPaths->ItemAtFast(i));
        }
    }

    int32 count = other.Transformers()->CountItems();
    for (int32 i = 0; i < count; i++) {
        Transformer* original = other.Transformers()->ItemAtFast(i);
        Transformer* cloned = original->Clone();
        if (cloned)
            fTransformers.AddItem(cloned);
    }
}

Shape::~Shape()
{
    if (fPaths) {
        fPaths->MakeEmpty();
        delete fPaths;
    }
    fTransformers.MakeEmpty();
}

status_t
Shape::Archive(BMessage* into, bool deep) const
{
    status_t ret = B_OK;

    if (ret == B_OK)
        ret = into->AddBool("hinting", fHinting);

    if (ret == B_OK) {
        int32 count = fTransformers.CountItems();
        for (int32 i = 0; i < count; i++) {
            Transformer* transformer = fTransformers.ItemAtFast(i);
            BMessage transformerArchive;
            ret = transformer->Archive(&transformerArchive);
            if (ret == B_OK)
                ret = into->AddMessage("transformer", &transformerArchive);
            if (ret < B_OK)
                break;
        }
    }

    if (ret == B_OK) {
        int32 size = Transformable::matrix_size;
        double matrix[size];
        StoreTo(matrix);
        ret = into->AddData("transformation", B_DOUBLE_TYPE,
            matrix, size * sizeof(double));
    }

    if (ret == B_OK)
        ret = into->AddFloat("min visibility scale", fMinVisibilityScale);
    if (ret == B_OK)
        ret = into->AddFloat("max visibility scale", fMaxVisibilityScale);

    return ret;
}

status_t
Shape::Unarchive(BMessage* archive)
{
    if (archive->FindBool("hinting", &fHinting) < B_OK)
        fHinting = false;

    BMessage transformerArchive;
    for (int32 i = 0;
         archive->FindMessage("transformer", i, &transformerArchive) == B_OK;
         i++) {
        Transformer* transformer = TransformerFactory::TransformerFor(&transformerArchive);
        if (transformer)
            fTransformers.AddItem(transformer);
    }

    int32 size = Transformable::matrix_size;
    const void* matrix;
    ssize_t dataSize = size * sizeof(double);
    status_t ret = archive->FindData("transformation", B_DOUBLE_TYPE,
        &matrix, &dataSize);
    if (ret == B_OK && dataSize == (ssize_t)(size * sizeof(double)))
        LoadFrom((const double*)matrix);

    if (archive->FindFloat("min visibility scale", &fMinVisibilityScale) < B_OK)
        fMinVisibilityScale = 0.0;
    if (archive->FindFloat("max visibility scale", &fMaxVisibilityScale) < B_OK)
        fMaxVisibilityScale = 4.0;

    return B_OK;
}

Shape*
Shape::Clone() const
{
    return new (nothrow) Shape(*this);
}

void
Shape::SetStyle(::Style* style)
{
    fStyle = style;
}

void
Shape::SetHinting(bool hinting)
{
    fHinting = hinting;
}

void
Shape::SetMinVisibilityScale(float scale)
{
    fMinVisibilityScale = scale;
}

void
Shape::SetMaxVisibilityScale(float scale)
{
    fMaxVisibilityScale = scale;
}
