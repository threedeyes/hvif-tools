/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef ICON_ADAPTER_H
#define ICON_ADAPTER_H

#include "HaikuIcon.h"
#include "HVIFStructures.h"
#include "IOMStructures.h"

namespace adapter {

class HVIFAdapter {
public:
	static haiku::Icon			FromHVIF(const hvif::HVIFIcon& hvif);
	static hvif::HVIFIcon		ToHVIF(const haiku::Icon& icon);

private:
	static haiku::Color			ConvertColor(const hvif::Color& c);
	static hvif::Color			ConvertColorToHVIF(const haiku::Color& c);
	static haiku::Gradient		ConvertGradient(const hvif::Gradient& g);
	static hvif::Gradient		ConvertGradientToHVIF(const haiku::Gradient& g);
	static haiku::PathPoint		ConvertNode(float x, float y, float x_in, float y_in, float x_out, float y_out);
	static haiku::Transformer	ConvertTransformer(const hvif::Transformer& t);
	static hvif::Transformer	ConvertTransformerToHVIF(const haiku::Transformer& t);
};

class IOMAdapter {
public:
	static haiku::Icon			FromIOM(const iom::Icon& iom);
	static iom::Icon			ToIOM(const haiku::Icon& icon);

private:
	static haiku::Color			ConvertColor(uint32_t iomColor);
	static uint32_t				ConvertColorToIOM(const haiku::Color& c);
	static haiku::Gradient		ConvertGradient(const iom::Gradient& g);
	static iom::Gradient		ConvertGradientToIOM(const haiku::Gradient& g);
	static haiku::PathPoint		ConvertPoint(const iom::ControlPoint& cp);
	static iom::ControlPoint	ConvertPointToIOM(const haiku::PathPoint& p);
	static haiku::Transformer	ConvertTransformer(const iom::Transformer& t);
	static iom::Transformer		ConvertTransformerToIOM(const haiku::Transformer& t);
};

}

#endif
