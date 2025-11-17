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

#ifndef TRANSFORMERS_H
#define TRANSFORMERS_H

#include <SupportDefs.h>
#include <agg_basics.h>
#include <agg_math_stroke.h>
#include "../core/IconBuild.h"

class BMessage;

_BEGIN_ICON_NAMESPACE

class Transformer {
public:
    Transformer() {}
    virtual ~Transformer() {}
    
    virtual status_t Archive(BMessage* into) const = 0;
    virtual Transformer* Clone() const = 0;
};

class AffineTransformer : public Transformer {
public:
    enum { archive_code = 'affn' };
    
    AffineTransformer();
    AffineTransformer(BMessage* archive);
    virtual ~AffineTransformer();
    
    virtual status_t Archive(BMessage* into) const;
    virtual Transformer* Clone() const;
    
    void store_to(double* matrix) const;
    void load_from(const double* matrix);
    
private:
    double fMatrix[6];
};

class ContourTransformer : public Transformer {
public:
    enum { archive_code = 'cntr' };
    
    ContourTransformer();
    ContourTransformer(BMessage* archive);
    virtual ~ContourTransformer();
    
    virtual status_t Archive(BMessage* into) const;
    virtual Transformer* Clone() const;
    
    float width() const { return fWidth; }
    agg::line_join_e line_join() const { return fLineJoin; }
    float miter_limit() const { return fMiterLimit; }
    
private:
    float fWidth;
    agg::line_join_e fLineJoin;
    float fMiterLimit;
};

class PerspectiveTransformer : public Transformer {
public:
    enum { archive_code = 'prsp' };
    
    PerspectiveTransformer();
    PerspectiveTransformer(BMessage* archive);
    virtual ~PerspectiveTransformer();
    
    virtual status_t Archive(BMessage* into) const;
    virtual Transformer* Clone() const;
    
    void store_to(double* matrix) const;
    void load_from(const double* matrix);
    
private:
    double fMatrix[9];
};

class StrokeTransformer : public Transformer {
public:
    enum { archive_code = 'strk' };
    
    StrokeTransformer();
    StrokeTransformer(BMessage* archive);
    virtual ~StrokeTransformer();
    
    virtual status_t Archive(BMessage* into) const;
    virtual Transformer* Clone() const;
    
    float width() const { return fWidth; }
    agg::line_cap_e line_cap() const { return fLineCap; }
    agg::line_join_e line_join() const { return fLineJoin; }
    float miter_limit() const { return fMiterLimit; }
    
private:
    float fWidth;
    agg::line_cap_e fLineCap;
    agg::line_join_e fLineJoin;
    float fMiterLimit;
};

class TransformerFactory {
public:
    static Transformer* TransformerFor(BMessage* archive);
};

_END_ICON_NAMESPACE

#endif
