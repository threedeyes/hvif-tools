/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef IOM_SVG_RENDERER_H
#define IOM_SVG_RENDERER_H

#include <string>

#include "IOMStructures.h"

namespace iom {

class SVGRenderer {
public:
	SVGRenderer(bool addNames = false);
	~SVGRenderer();

	std::string		RenderIcon(const Icon& icon, int width, int height);

private:
	std::string		_ColorToCSS(uint32_t color);
	float			_GetColorAlpha(uint32_t color);
	std::string		_GradientToSVG(const Gradient& grad, const std::string& id, const std::string& styleName = "");
	std::string		_PathToSVG(const Path& path);
	std::string		_ShapeToSVG(const Shape& shape, const Icon& icon, int shapeIndex);
	std::string		_TransformToSVG(const std::vector<double>& matrix);
	std::string		_GetLineJoinName(int32_t lineJoin);
	std::string		_GetLineCapName(int32_t lineCap);
	std::string		_GenerateID();

	int				fIdCounter;
	bool			fAddNames;
};

}

#endif
