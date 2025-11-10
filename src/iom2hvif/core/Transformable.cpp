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

#include "Transformable.h"
#include <Point.h>
#include <string.h>

_USING_ICON_NAMESPACE

Transformable::Transformable()
    : agg::trans_affine()
{
}

Transformable::Transformable(const Transformable& other)
    : agg::trans_affine(other)
{
}

Transformable::~Transformable()
{
}

void
Transformable::StoreTo(double matrix[matrix_size]) const
{
    store_to(matrix);
}

void
Transformable::LoadFrom(const double matrix[matrix_size])
{
    Transformable t;
    t.load_from(matrix);
    if (*this != t) {
        load_from(matrix);
        TransformationChanged();
    }
}

void
Transformable::SetTransform(const Transformable& other)
{
    if (*this != other) {
        *this = other;
        TransformationChanged();
    }
}

Transformable&
Transformable::operator=(const Transformable& other)
{
    if (other != *this) {
        reset();
        multiply(other);
        TransformationChanged();
    }
    return *this;
}

Transformable&
Transformable::Multiply(const Transformable& other)
{
    if (!other.IsIdentity()) {
        multiply(other);
        TransformationChanged();
    }
    return *this;
}

void
Transformable::Reset()
{
    if (!IsIdentity()) {
        reset();
        TransformationChanged();
    }
}

bool
Transformable::IsIdentity() const
{
    double m[matrix_size];
    store_to(m);
    return (m[0] == 1.0 && m[1] == 0.0 && m[2] == 0.0 &&
            m[3] == 1.0 && m[4] == 0.0 && m[5] == 0.0);
}

bool
Transformable::IsTranslationOnly() const
{
    double m[matrix_size];
    store_to(m);
    return (m[0] == 1.0 && m[1] == 0.0 && m[2] == 0.0 && m[3] == 1.0);
}

bool
Transformable::operator==(const Transformable& other) const
{
    double m1[matrix_size];
    other.store_to(m1);
    double m2[matrix_size];
    store_to(m2);
    return memcmp(m1, m2, sizeof(m1)) == 0;
}

bool
Transformable::operator!=(const Transformable& other) const
{
    return !(*this == other);
}

void
Transformable::Transform(double* x, double* y) const
{
    transform(x, y);
}

void
Transformable::Transform(BPoint* point) const
{
    if (point) {
        double x = point->x;
        double y = point->y;
        transform(&x, &y);
        point->x = x;
        point->y = y;
    }
}

void
Transformable::TransformationChanged()
{
}
