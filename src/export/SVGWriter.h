/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef EXPORT_SVG_WRITER_H
#define EXPORT_SVG_WRITER_H

#include <string>
#include "HaikuIcon.h"

namespace haiku {

struct SVGWriterOptions {
	int				width;
	int				height;
	bool			includeNames;
	std::string		viewBox;
	float			coordinateScale;

	SVGWriterOptions() : width(64), height(64), includeNames(false),
		viewBox("0 0 64 64"), coordinateScale(1.0f) {}
};

class SVGWriter {
public:
					SVGWriter();
					~SVGWriter() {};

	std::string		Write(const Icon& icon, const SVGWriterOptions& opts);
	std::string		Write(const Icon& icon);

private:
	int				fIdCounter;
	bool			fIncludeNames;
	float			fCoordinateScale;

	std::string		_ColorToCSS(const Color& color);
	float			_GetColorAlpha(const Color& color);
	std::string		_GradientToSVG(const Gradient& grad, const std::string& id, const std::string& styleName);
	std::string		_PathToSVG(const Path& path);
	std::string		_ShapeToSVG(const Shape& shape, const Icon& icon, int shapeIndex);
	std::string		_TransformToSVG(const std::vector<double>& matrix);
	std::string		_GenerateID();
};

}

#endif
