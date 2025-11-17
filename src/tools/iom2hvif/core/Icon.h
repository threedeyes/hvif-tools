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

#ifndef ICON_H
#define ICON_H

#include "Container.h"
#include "IconBuild.h"

_BEGIN_ICON_NAMESPACE

class Shape;
class Style;
class VectorPath;

class Icon {
public:
    Icon();
    Icon(const Icon& other);
    virtual ~Icon();

    const Container<Style>* Styles() const { return &fStyles; }
    Container<Style>* Styles() { return &fStyles; }
    
    const Container<VectorPath>* Paths() const { return &fPaths; }
    Container<VectorPath>* Paths() { return &fPaths; }
    
    const Container<Shape>* Shapes() const { return &fShapes; }
    Container<Shape>* Shapes() { return &fShapes; }

    Icon* Clone() const;
    void MakeEmpty();

private:
    Container<Style> fStyles;
    Container<VectorPath> fPaths;
    Container<Shape> fShapes;
};

_END_ICON_NAMESPACE

#endif
