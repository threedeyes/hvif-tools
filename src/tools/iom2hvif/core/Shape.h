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

#ifndef SHAPE_H
#define SHAPE_H

#include "Container.h"
#include "IconBuild.h"
#include "Transformable.h"

class BMessage;

_BEGIN_ICON_NAMESPACE

class Style;
class VectorPath;
class Transformer;

class Shape : public Transformable {
public:
    Shape(::Style* style);
    Shape(const Shape& other);
    virtual ~Shape();

    status_t Archive(BMessage* into, bool deep = true) const;
    status_t Unarchive(BMessage* archive);

    virtual Shape* Clone() const;

    Container<VectorPath>* Paths() const { return fPaths; }
    const Container<Transformer>* Transformers() const { return &fTransformers; }
    Container<Transformer>* Transformers() { return &fTransformers; }

    void SetStyle(::Style* style);
    ::Style* GetStyle() const { return fStyle; }

    void SetHinting(bool hinting);
    bool Hinting() const { return fHinting; }

    void SetMinVisibilityScale(float scale);
    float MinVisibilityScale() const { return fMinVisibilityScale; }

    void SetMaxVisibilityScale(float scale);
    float MaxVisibilityScale() const { return fMaxVisibilityScale; }

private:
    Container<VectorPath>* fPaths;
    ::Style* fStyle;
    Container<Transformer> fTransformers;
    bool fHinting;
    float fMinVisibilityScale;
    float fMaxVisibilityScale;
};

_END_ICON_NAMESPACE

#endif
