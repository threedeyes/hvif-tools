/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef HVIF_SVG_RENDERER_H
#define HVIF_SVG_RENDERER_H

#include <string>
#include "HVIFStructures.h"

namespace hvif {

class SVGRenderer {
public:
	SVGRenderer();
	~SVGRenderer();

	std::string RenderIcon(const HVIFIcon& icon, int width, int height);

private:
	std::string _ColorToCSS(const Color& color);
	float _GetColorAlpha(const Color& color);
	std::string _GradientToSVG(const Gradient& grad, const std::string& id);
	std::string _PathToSVG(const std::vector<Path>& paths);
	std::string _ShapeToSVG(const Shape& shape, const HVIFIcon& icon, 
							const std::string& id, int shapeIndex);
	std::string _TransformToSVG(const std::vector<float>& transform, 
							const std::string& type);
	std::string _MatrixToSVG(const std::vector<float>& matrix);
	std::string _GenerateID();

	int fIdCounter;
};

}

#endif
