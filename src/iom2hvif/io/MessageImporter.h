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

#ifndef MESSAGE_IMPORTER_H
#define MESSAGE_IMPORTER_H

#include <SupportDefs.h>
#include "../core/IconBuild.h"

class BMessage;
class BPositionIO;

_BEGIN_ICON_NAMESPACE

template <class Type> class Container;
class Icon;
class Shape;
class Style;
class VectorPath;

const uint32 kNativeIconMagicNumber = 'IMSG';

class MessageImporter {
public:
    MessageImporter();
    virtual ~MessageImporter();

    status_t Import(Icon* icon, BPositionIO* stream);

private:
    status_t _ImportPaths(const BMessage* archive,
                          Container<VectorPath>* paths) const;
    status_t _ImportStyles(const BMessage* archive,
                           Container<Style>* styles) const;
    status_t _ImportShapes(const BMessage* archive,
                           Container<VectorPath>* paths,
                           Container<Style>* styles,
                           Container<Shape>* shapes) const;
};

_END_ICON_NAMESPACE

#endif
