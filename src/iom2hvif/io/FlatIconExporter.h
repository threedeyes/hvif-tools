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

#ifndef FLAT_ICON_EXPORTER_H
#define FLAT_ICON_EXPORTER_H

#include <SupportDefs.h>
#include "../core/IconBuild.h"

class BNode;
class BPositionIO;

_BEGIN_ICON_NAMESPACE

template <class Type> class Container;
class Gradient;
class Icon;
class LittleEndianBuffer;
class Shape;
class Style;
class VectorPath;

class FlatIconExporter {
public:
    FlatIconExporter();
    virtual ~FlatIconExporter();

    status_t Export(const Icon* icon, BPositionIO* stream);
    status_t ExportToAttribute(const Icon* icon, BNode* node,
                                const char* attrName);

private:
    status_t _Export(LittleEndianBuffer& buffer, const Icon* icon);
    status_t _WriteStyles(LittleEndianBuffer& buffer,
                          const Container<Style>* styles);
    status_t _WritePaths(LittleEndianBuffer& buffer,
                         const Container<VectorPath>* paths);
    status_t _WriteShapes(LittleEndianBuffer& buffer,
                          const Container<Style>* styles,
                          const Container<VectorPath>* paths,
                          const Container<Shape>* shapes);
    bool _WriteGradient(LittleEndianBuffer& buffer,
                        const Gradient* gradient);
    bool _AnalysePath(VectorPath* path, uint8 pointCount,
                      int32& straightCount, int32& lineCount,
                      int32& curveCount);
};

_END_ICON_NAMESPACE

#endif
