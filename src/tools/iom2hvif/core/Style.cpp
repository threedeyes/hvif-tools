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

#include "Style.h"
#include <Message.h>
#include <new>

using std::nothrow;

_USING_ICON_NAMESPACE

Gradient::Gradient(bool empty)
    : Transformable()
    , fColors(4)
    , fType(GRADIENT_LINEAR)
    , fInterpolation(INTERPOLATION_SMOOTH)
    , fInheritTransformation(true)
{
    if (!empty) {
        AddColor(BGradient::ColorStop(0, 0, 0, 255, 0.0), 0);
        AddColor(BGradient::ColorStop(255, 255, 255, 255, 1.0), 1);
    }
}

Gradient::Gradient(BMessage* archive)
    : Transformable()
    , fColors(4)
    , fType(GRADIENT_LINEAR)
    , fInterpolation(INTERPOLATION_SMOOTH)
    , fInheritTransformation(true)
{
    if (!archive)
        return;

    int32 size = Transformable::matrix_size;
    const void* matrix;
    ssize_t dataSize = size * sizeof(double);
    if (archive->FindData("transformation", B_DOUBLE_TYPE,
                          &matrix, &dataSize) == B_OK
        && dataSize == (ssize_t)(size * sizeof(double))) {
        LoadFrom((const double*)matrix);
    }

    BGradient::ColorStop step;
    for (int32 i = 0; archive->FindFloat("offset", i, &step.offset) >= B_OK; i++) {
        if (archive->FindInt32("color", i, (int32*)&step.color) >= B_OK) {
            AddColor(step.color, step.offset);
        } else
            break;
    }

    if (archive->FindInt32("type", (int32*)&fType) < B_OK)
        fType = GRADIENT_LINEAR;

    if (archive->FindInt32("interpolation", (int32*)&fInterpolation) < B_OK)
        fInterpolation = INTERPOLATION_SMOOTH;

    if (archive->FindBool("inherit transformation", &fInheritTransformation) < B_OK)
        fInheritTransformation = true;
}

Gradient::Gradient(const Gradient& other)
    : Transformable(other)
    , fColors(4)
    , fType(other.fType)
    , fInterpolation(other.fInterpolation)
    , fInheritTransformation(other.fInheritTransformation)
{
    for (int32 i = 0; BGradient::ColorStop* step = other.ColorAt(i); i++) {
        AddColor(*step, i);
    }
}

Gradient::~Gradient()
{
    _MakeEmpty();
}

status_t
Gradient::Archive(BMessage* into, bool deep) const
{
    status_t ret = B_OK;

    int32 size = Transformable::matrix_size;
    double matrix[size];
    StoreTo(matrix);
    ret = into->AddData("transformation", B_DOUBLE_TYPE,
                        matrix, size * sizeof(double));

    if (ret >= B_OK) {
        for (int32 i = 0; BGradient::ColorStop* step = ColorAt(i); i++) {
            ret = into->AddInt32("color", (const uint32&)step->color);
            if (ret < B_OK)
                break;
            ret = into->AddFloat("offset", step->offset);
            if (ret < B_OK)
                break;
        }
    }

    if (ret >= B_OK)
        ret = into->AddInt32("type", (int32)fType);
    if (ret >= B_OK)
        ret = into->AddInt32("interpolation", (int32)fInterpolation);
    if (ret >= B_OK)
        ret = into->AddBool("inherit transformation", fInheritTransformation);

    return ret;
}

Gradient&
Gradient::operator=(const Gradient& other)
{
    SetTransform(other);
    SetColors(other);
    SetType(other.fType);
    SetInterpolation(other.fInterpolation);
    SetInheritTransformation(other.fInheritTransformation);
    return *this;
}

bool
Gradient::operator==(const Gradient& other) const
{
    if (!Transformable::operator==(other))
        return false;

    int32 count = CountColors();
    if (count != other.CountColors())
        return false;
    if (fType != other.fType)
        return false;
    if (fInterpolation != other.fInterpolation)
        return false;
    if (fInheritTransformation != other.fInheritTransformation)
        return false;

    for (int32 i = 0; i < count; i++) {
        BGradient::ColorStop* ourStep = ColorAtFast(i);
        BGradient::ColorStop* otherStep = other.ColorAtFast(i);
        if (*ourStep != *otherStep)
            return false;
    }
    return true;
}

bool
Gradient::operator!=(const Gradient& other) const
{
    return !(*this == other);
}

void
Gradient::SetColors(const Gradient& other)
{
    _MakeEmpty();
    for (int32 i = 0; BGradient::ColorStop* step = other.ColorAt(i); i++)
        AddColor(*step, i);
}

int32
Gradient::AddColor(const rgb_color& color, float offset)
{
    BGradient::ColorStop* step = new BGradient::ColorStop(color, offset);
    int32 index = 0;
    int32 count = CountColors();
    for (; index < count; index++) {
        BGradient::ColorStop* s = ColorAtFast(index);
        if (s->offset > step->offset)
            break;
    }
    if (!fColors.AddItem((void*)step, index)) {
        delete step;
        return -1;
    }
    return index;
}

bool
Gradient::AddColor(const BGradient::ColorStop& color, int32 index)
{
    BGradient::ColorStop* step = new (nothrow) BGradient::ColorStop(color);
    if (!step || !fColors.AddItem((void*)step, index)) {
        delete step;
        return false;
    }
    return true;
}

bool
Gradient::RemoveColor(int32 index)
{
    BGradient::ColorStop* step = (BGradient::ColorStop*)fColors.RemoveItem(index);
    if (!step)
        return false;
    delete step;
    return true;
}

bool
Gradient::SetColor(int32 index, const BGradient::ColorStop& color)
{
    if (BGradient::ColorStop* step = ColorAt(index)) {
        if (*step != color) {
            step->color = color.color;
            step->offset = color.offset;
            return true;
        }
    }
    return false;
}

bool
Gradient::SetColor(int32 index, const rgb_color& color)
{
    if (BGradient::ColorStop* step = ColorAt(index)) {
        if ((uint32&)step->color != (uint32&)color) {
            step->color = color;
            return true;
        }
    }
    return false;
}

bool
Gradient::SetOffset(int32 index, float offset)
{
    BGradient::ColorStop* step = ColorAt(index);
    if (step && step->offset != offset) {
        step->offset = offset;
        return true;
    }
    return false;
}

int32
Gradient::CountColors() const
{
    return fColors.CountItems();
}

BGradient::ColorStop*
Gradient::ColorAt(int32 index) const
{
    return (BGradient::ColorStop*)fColors.ItemAt(index);
}

BGradient::ColorStop*
Gradient::ColorAtFast(int32 index) const
{
    return (BGradient::ColorStop*)fColors.ItemAtFast(index);
}

void
Gradient::SetType(gradients_type type)
{
    fType = type;
}

void
Gradient::SetInterpolation(interpolation_type type)
{
    fInterpolation = type;
}

void
Gradient::SetInheritTransformation(bool inherit)
{
    fInheritTransformation = inherit;
}

void
Gradient::_MakeEmpty()
{
    int32 count = CountColors();
    for (int32 i = 0; i < count; i++)
        delete ColorAtFast(i);
    fColors.MakeEmpty();
}

Style::Style()
    : fColor((rgb_color){ 255, 255, 255, 255 })
    , fGradient(NULL)
{
}

Style::Style(const rgb_color& color)
    : fColor(color)
    , fGradient(NULL)
{
}

Style::Style(const Style& other)
    : fColor(other.fColor)
    , fGradient(NULL)
{
    SetGradient(other.fGradient);
}

Style::Style(BMessage* archive)
    : fColor((rgb_color){ 255, 255, 255, 255 })
    , fGradient(NULL)
{
    if (!archive)
        return;

    if (archive->FindInt32("color", (int32*)&fColor) < B_OK)
        fColor = (rgb_color){ 255, 255, 255, 255 };

    BMessage gradientArchive;
    if (archive->FindMessage("gradient", &gradientArchive) == B_OK) {
        Gradient gradient(&gradientArchive);
        SetGradient(&gradient);
    }
}

Style::~Style()
{
    delete fGradient;
}

status_t
Style::Archive(BMessage* into, bool deep) const
{
    status_t ret = into->AddInt32("color", (uint32&)fColor);

    if (ret == B_OK && fGradient) {
        BMessage gradientArchive;
        ret = fGradient->Archive(&gradientArchive, deep);
        if (ret == B_OK)
            ret = into->AddMessage("gradient", &gradientArchive);
    }

    return ret;
}

void
Style::SetColor(const rgb_color& color)
{
    fColor = color;
}

void
Style::SetGradient(const Gradient* gradient)
{
    if (!fGradient && !gradient)
        return;

    if (gradient) {
        if (!fGradient) {
            fGradient = new (nothrow) Gradient(*gradient);
        } else {
            if (*fGradient != *gradient)
                *fGradient = *gradient;
        }
    } else {
        delete fGradient;
        fGradient = NULL;
    }
}
