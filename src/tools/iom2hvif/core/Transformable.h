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

#ifndef TRANSFORMABLE_H
#define TRANSFORMABLE_H

#include <Point.h>
#include <agg_trans_affine.h>
#include "IconBuild.h"

_BEGIN_ICON_NAMESPACE

class Transformable : public agg::trans_affine {
public:
    enum {
        matrix_size = 6,
    };

    Transformable();
    Transformable(const Transformable& other);
    virtual ~Transformable();

    void StoreTo(double matrix[matrix_size]) const;
    void LoadFrom(const double matrix[matrix_size]);

    void SetTransform(const Transformable& other);
    Transformable& operator=(const Transformable& other);
    Transformable& Multiply(const Transformable& other);
    void Reset();

    bool IsIdentity() const;
    bool IsTranslationOnly() const;

    bool operator==(const Transformable& other) const;
    bool operator!=(const Transformable& other) const;

    void Transform(double* x, double* y) const;
    void Transform(::BPoint* point) const;

    virtual void TransformationChanged();
};

_END_ICON_NAMESPACE

#endif
