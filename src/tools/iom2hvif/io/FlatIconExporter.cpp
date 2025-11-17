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

#include "FlatIconExporter.h"
#include "../core/Icon.h"
#include "../core/Shape.h"
#include "../core/Style.h"
#include "../core/VectorPath.h"
#include "../format/FlatIconFormat.h"
#include "../format/LittleEndianBuffer.h"
#include "../format/PathCommandQueue.h"
#include "../transform/Transformers.h"
#include <fs_attr.h>
#include <DataIO.h>
#include <Node.h>
#include <new>

using std::nothrow;

_USING_ICON_NAMESPACE

FlatIconExporter::FlatIconExporter()
{
}

FlatIconExporter::~FlatIconExporter()
{
}

status_t
FlatIconExporter::Export(const Icon* icon, BPositionIO* stream)
{
    LittleEndianBuffer buffer;

    status_t ret = _Export(buffer, icon);
    if (ret < B_OK)
        return ret;

    ssize_t written = stream->Write(buffer.Buffer(), buffer.SizeUsed());
    if (written != (ssize_t)buffer.SizeUsed()) {
        if (written < 0)
            return (status_t)written;
        return B_ERROR;
    }

    return B_OK;
}

status_t
FlatIconExporter::ExportToAttribute(const Icon* icon, BNode* node,
                                    const char* attrName)
{
    if (!icon || !node || !attrName)
        return B_BAD_VALUE;

    LittleEndianBuffer buffer;

    status_t ret = _Export(buffer, icon);
    if (ret < B_OK)
        return ret;

    ssize_t written = node->WriteAttr(attrName, B_VECTOR_ICON_TYPE, 0,
                                      buffer.Buffer(), buffer.SizeUsed());
    if (written != (ssize_t)buffer.SizeUsed()) {
        if (written < 0)
            return (status_t)written;
        return B_ERROR;
    }

    return B_OK;
}

status_t
FlatIconExporter::_Export(LittleEndianBuffer& buffer, const Icon* icon)
{
    if (!buffer.Write(FLAT_ICON_MAGIC))
        return B_NO_MEMORY;

    const Container<Style>* styles = icon->Styles();
    status_t ret = _WriteStyles(buffer, styles);
    if (ret < B_OK)
        return ret;

    const Container<VectorPath>* paths = icon->Paths();
    ret = _WritePaths(buffer, paths);
    if (ret < B_OK)
        return ret;

    ret = _WriteShapes(buffer, styles, paths, icon->Shapes());
    if (ret < B_OK)
        return ret;

    return B_OK;
}

static bool
_WriteTransformable(LittleEndianBuffer& buffer, const Transformable* transformable)
{
    int32 matrixSize = Transformable::matrix_size;
    double matrix[matrixSize];
    transformable->StoreTo(matrix);
    for (int32 i = 0; i < matrixSize; i++) {
        if (!write_float_24(buffer, (float)matrix[i]))
            return false;
    }
    return true;
}

static bool
_WriteTranslation(LittleEndianBuffer& buffer, const Transformable* transformable)
{
    BPoint t(B_ORIGIN);
    transformable->Transform(&t);
    return write_coord(buffer, t.x) && write_coord(buffer, t.y);
}

status_t
FlatIconExporter::_WriteStyles(LittleEndianBuffer& buffer,
                               const Container<Style>* styles)
{
    if (styles->CountItems() > 255)
        return B_ERROR;
    uint8 styleCount = min_c(255, styles->CountItems());
    if (!buffer.Write(styleCount))
        return B_NO_MEMORY;

    for (int32 i = 0; i < styleCount; i++) {
        Style* style = styles->ItemAtFast(i);

        uint8 styleType;
        const Gradient* gradient = style->GradientObject();
        
        if (gradient) {
            styleType = STYLE_TYPE_GRADIENT;
        } else {
            rgb_color color = style->Color();
            if (color.red == color.green && color.red == color.blue) {
                if (style->Color().alpha == 255)
                    styleType = STYLE_TYPE_SOLID_GRAY_NO_ALPHA;
                else
                    styleType = STYLE_TYPE_SOLID_GRAY;
            } else {
                if (style->Color().alpha == 255)
                    styleType = STYLE_TYPE_SOLID_COLOR_NO_ALPHA;
                else
                    styleType = STYLE_TYPE_SOLID_COLOR;
            }
        }

        if (!buffer.Write(styleType))
            return B_NO_MEMORY;

        if (styleType == STYLE_TYPE_SOLID_COLOR) {
            rgb_color color = style->Color();
            if (!buffer.Write(*(uint32*)&color))
                return B_NO_MEMORY;
        } else if (styleType == STYLE_TYPE_SOLID_COLOR_NO_ALPHA) {
            rgb_color color = style->Color();
            if (!buffer.Write(color.red)
                || !buffer.Write(color.green)
                || !buffer.Write(color.blue))
                return B_NO_MEMORY;
        } else if (styleType == STYLE_TYPE_SOLID_GRAY) {
            rgb_color color = style->Color();
            if (!buffer.Write(color.red)
                || !buffer.Write(color.alpha))
                return B_NO_MEMORY;
        } else if (styleType == STYLE_TYPE_SOLID_GRAY_NO_ALPHA) {
            if (!buffer.Write(style->Color().red))
                return B_NO_MEMORY;
        } else if (styleType == STYLE_TYPE_GRADIENT) {
            if (!_WriteGradient(buffer, gradient))
                return B_NO_MEMORY;
        }
    }

    return B_OK;
}

bool
FlatIconExporter::_AnalysePath(VectorPath* path, uint8 pointCount,
    int32& straightCount, int32& lineCount, int32& curveCount)
{
    straightCount = 0;
    lineCount = 0;
    curveCount = 0;

    BPoint lastPoint(B_ORIGIN);
    for (uint32 p = 0; p < pointCount; p++) {
        BPoint point, pointIn, pointOut;
        if (!path->GetPointsAt(p, point, pointIn, pointOut))
            return false;
        
        if (point == pointIn && point == pointOut) {
            if (point.x == lastPoint.x || point.y == lastPoint.y)
                straightCount++;
            else
                lineCount++;
        } else {
            curveCount++;
        }
        lastPoint = point;
    }

    return true;
}

static bool
write_path_no_curves(LittleEndianBuffer& buffer, VectorPath* path, uint8 pointCount)
{
    for (uint32 p = 0; p < pointCount; p++) {
        BPoint point;
        if (!path->GetPointAt(p, point))
            return false;
        if (!write_coord(buffer, point.x) || !write_coord(buffer, point.y))
            return false;
    }
    return true;
}

static bool
write_path_curves(LittleEndianBuffer& buffer, VectorPath* path, uint8 pointCount)
{
    for (uint32 p = 0; p < pointCount; p++) {
        BPoint point, pointIn, pointOut;
        if (!path->GetPointsAt(p, point, pointIn, pointOut))
            return false;
        if (!write_coord(buffer, point.x)
            || !write_coord(buffer, point.y))
            return false;
        if (!write_coord(buffer, pointIn.x)
            || !write_coord(buffer, pointIn.y))
            return false;
        if (!write_coord(buffer, pointOut.x)
            || !write_coord(buffer, pointOut.y))
            return false;
    }
    return true;
}

static bool
write_path_with_commands(LittleEndianBuffer& buffer, VectorPath* path, uint8 pointCount)
{
    PathCommandQueue queue;
    return queue.Write(buffer, path, pointCount);
}

status_t
FlatIconExporter::_WritePaths(LittleEndianBuffer& buffer,
                              const Container<VectorPath>* paths)
{
    if (paths->CountItems() > 255)
        return B_ERROR;
    uint8 pathCount = min_c(255, paths->CountItems());
    if (!buffer.Write(pathCount))
        return B_NO_MEMORY;

    for (uint32 i = 0; i < pathCount; i++) {
        VectorPath* path = paths->ItemAtFast(i);
        uint8 pathFlags = 0;
        if (path->IsClosed())
            pathFlags |= PATH_FLAG_CLOSED;

        if (path->CountPoints() > 255)
            return B_ERROR;
        uint8 pointCount = min_c(255, path->CountPoints());

        int32 straightCount, lineCount, curveCount;
        if (!_AnalysePath(path, pointCount,
                straightCount, lineCount, curveCount))
            return B_ERROR;

        int32 commandPathLength
            = pointCount + straightCount * 2 + lineCount * 4 + curveCount * 12;
        int32 plainPathLength
            = pointCount * 12;

        if (commandPathLength < plainPathLength) {
            if (curveCount == 0)
                pathFlags |= PATH_FLAG_NO_CURVES;
            else
                pathFlags |= PATH_FLAG_USES_COMMANDS;
        }

        if (!buffer.Write(pathFlags) || !buffer.Write(pointCount))
            return B_NO_MEMORY;

        if (pathFlags & PATH_FLAG_NO_CURVES) {
            if (!write_path_no_curves(buffer, path, pointCount))
                return B_ERROR;
        } else if (pathFlags & PATH_FLAG_USES_COMMANDS) {
            if (!write_path_with_commands(buffer, path, pointCount))
                return B_ERROR;
        } else {
            if (!write_path_curves(buffer, path, pointCount))
                return B_ERROR;
        }
    }

    return B_OK;
}

static bool
_WriteTransformer(LittleEndianBuffer& buffer, Transformer* t)
{
    if (AffineTransformer* affine = dynamic_cast<AffineTransformer*>(t)) {
        if (!buffer.Write((uint8)TRANSFORMER_TYPE_AFFINE))
            return false;
        double matrix[6];
        affine->store_to(matrix);
        for (int32 i = 0; i < 6; i++) {
            if (!write_float_24(buffer, (float)matrix[i]))
                return false;
        }
    } else if (ContourTransformer* contour = dynamic_cast<ContourTransformer*>(t)) {
        if (!buffer.Write((uint8)TRANSFORMER_TYPE_CONTOUR))
            return false;
        uint8 width = (uint8)((int8)contour->width() + 128);
        uint8 lineJoin = (uint8)contour->line_join();
        uint8 miterLimit = (uint8)contour->miter_limit();
        if (!buffer.Write(width)
            || !buffer.Write(lineJoin)
            || !buffer.Write(miterLimit))
            return false;
    } else if (PerspectiveTransformer* perspective = dynamic_cast<PerspectiveTransformer*>(t)) {
        if (!buffer.Write((uint8)TRANSFORMER_TYPE_PERSPECTIVE))
            return false;
        double matrix[9];
        perspective->store_to(matrix);
        for (int32 i = 0; i < 9; i++) {
            if (!write_float_24(buffer, (float)matrix[i]))
                return false;
        }
    } else if (StrokeTransformer* stroke = dynamic_cast<StrokeTransformer*>(t)) {
        if (!buffer.Write((uint8)TRANSFORMER_TYPE_STROKE))
            return false;
        uint8 width = (uint8)((int8)stroke->width() + 128);
        uint8 lineOptions = (uint8)stroke->line_join();
        lineOptions |= ((uint8)stroke->line_cap()) << 4;
        uint8 miterLimit = (uint8)stroke->miter_limit();

        if (!buffer.Write(width)
            || !buffer.Write(lineOptions)
            || !buffer.Write(miterLimit))
            return false;
    }

    return true;
}

static bool
_WritePathSourceShape(LittleEndianBuffer& buffer, Shape* shape,
    const Container<Style>* styles, const Container<VectorPath>* paths)
{
    Style* style = shape->GetStyle();
    if (!style)
        return false;

    int32 styleIndex = styles->IndexOf(style);
    if (styleIndex < 0 || styleIndex > 255)
        return false;

    if (shape->Paths()->CountItems() > 255)
        return false;
    uint8 pathCount = min_c(255, shape->Paths()->CountItems());

    if (!buffer.Write((uint8)SHAPE_TYPE_PATH_SOURCE)
        || !buffer.Write((uint8)styleIndex)
        || !buffer.Write(pathCount))
        return false;

    for (uint32 i = 0; i < pathCount; i++) {
        VectorPath* path = shape->Paths()->ItemAtFast(i);
        int32 pathIndex = paths->IndexOf(path);
        if (pathIndex < 0 || pathIndex > 255)
            return false;

        if (!buffer.Write((uint8)pathIndex))
            return false;
    }

    if (shape->Transformers()->CountItems() > 255)
        return false;
    uint8 transformerCount = min_c(255, shape->Transformers()->CountItems());

    uint8 shapeFlags = 0;
    if (!shape->IsIdentity()) {
        if (shape->IsTranslationOnly())
            shapeFlags |= SHAPE_FLAG_TRANSLATION;
        else
            shapeFlags |= SHAPE_FLAG_TRANSFORM;
    }
    if (shape->Hinting())
        shapeFlags |= SHAPE_FLAG_HINTING;
    if (shape->MinVisibilityScale() != 0.0
        || shape->MaxVisibilityScale() != 4.0)
        shapeFlags |= SHAPE_FLAG_LOD_SCALE;
    if (transformerCount > 0)
        shapeFlags |= SHAPE_FLAG_HAS_TRANSFORMERS;

    if (!buffer.Write((uint8)shapeFlags))
        return false;

    if (shapeFlags & SHAPE_FLAG_TRANSFORM) {
        if (!_WriteTransformable(buffer, shape))
            return false;
    } else if (shapeFlags & SHAPE_FLAG_TRANSLATION) {
        if (!_WriteTranslation(buffer, shape))
            return false;
    }

    if (shapeFlags & SHAPE_FLAG_LOD_SCALE) {
        if (!buffer.Write((uint8)(shape->MinVisibilityScale() * 63.75 + 0.5))
            || !buffer.Write((uint8)(shape->MaxVisibilityScale() * 63.75 + 0.5))) {
            return false;
        }
    }

    if (shapeFlags & SHAPE_FLAG_HAS_TRANSFORMERS) {
        if (!buffer.Write(transformerCount))
            return false;

        for (uint32 i = 0; i < transformerCount; i++) {
            Transformer* transformer = shape->Transformers()->ItemAtFast(i);
            if (!_WriteTransformer(buffer, transformer))
                return false;
        }
    }

    return true;
}

status_t
FlatIconExporter::_WriteShapes(LittleEndianBuffer& buffer,
                               const Container<Style>* styles,
                               const Container<VectorPath>* paths,
                               const Container<Shape>* shapes)
{
    uint32 shapeCount = shapes->CountItems();

    if (shapeCount > 255)
        return B_ERROR;
    if (!buffer.Write((uint8)shapeCount))
        return B_NO_MEMORY;

    for (uint32 i = 0; i < shapeCount; i++) {
        Shape* shape = shapes->ItemAtFast(i);
        if (!_WritePathSourceShape(buffer, shape, styles, paths))
            return B_ERROR;
    }

    return B_OK;
}

bool
FlatIconExporter::_WriteGradient(LittleEndianBuffer& buffer,
                                 const Gradient* gradient)
{
    uint8 gradientType = (uint8)gradient->Type();
    uint8 gradientFlags = 0;
    uint8 gradientStopCount = (uint8)gradient->CountColors();

    if (!gradient->IsIdentity())
        gradientFlags |= GRADIENT_FLAG_TRANSFORM;

    bool alpha = false;
    bool gray = true;
    for (int32 i = 0; i < gradientStopCount; i++) {
        BGradient::ColorStop* step = gradient->ColorAtFast(i);
        if (step->color.alpha < 255)
            alpha = true;
        if (step->color.red != step->color.green
            || step->color.red != step->color.blue)
            gray = false;
    }
    if (!alpha)
        gradientFlags |= GRADIENT_FLAG_NO_ALPHA;
    if (gray)
        gradientFlags |= GRADIENT_FLAG_GRAYS;

    if (!buffer.Write(gradientType)
        || !buffer.Write(gradientFlags)
        || !buffer.Write(gradientStopCount))
        return false;

    if (gradientFlags & GRADIENT_FLAG_TRANSFORM) {
        if (!_WriteTransformable(buffer, gradient))
            return false;
    }

    for (int32 i = 0; i < gradientStopCount; i++) {
        BGradient::ColorStop* step = gradient->ColorAtFast(i);
        uint8 stopOffset = (uint8)(step->offset * 255.0);
        uint32 color = (uint32&)step->color;
        if (!buffer.Write(stopOffset))
            return false;
        if (alpha) {
            if (gray) {
                if (!buffer.Write(step->color.red)
                    || !buffer.Write(step->color.alpha))
                    return false;
            } else {
                if (!buffer.Write(color))
                    return false;
            }
        } else {
            if (gray) {
                if (!buffer.Write(step->color.red))
                    return false;
            } else {
                if (!buffer.Write(step->color.red)
                    || !buffer.Write(step->color.green)
                    || !buffer.Write(step->color.blue))
                    return false;
            }
        }
    }

    return true;
}
