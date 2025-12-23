/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef EXPORT_SVG_WRITER_H
#define EXPORT_SVG_WRITER_H

#include <string>
#include <vector>
#include "HaikuIcon.h"

namespace haiku {

struct SVGWriterOptions {
	int				width;
	int				height;
	bool			includeNames;
	std::string		viewBox;
	float			coordinateScale;

	SVGWriterOptions()
		: width(64)
		, height(64)
		, includeNames(false)
		, viewBox("0 0 6528 6528")
		, coordinateScale(102.0f)
	{}
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

	std::string		_FormatCoord(double value);
	std::string		_FormatMatrix(double value);
	std::string		_ColorToCSS(const Color& color);
	float			_GetColorAlpha(const Color& color);

	std::string		_GradientToSVG(const Gradient& grad, const std::string& id,
						const std::string& styleName, const Shape& shape);
	std::string		_PathToSVG(const Path& path);
	std::string		_PathToSVGTransformed(const Path& path, const Shape& shape);
	std::string		_ShapeToSVG(const Shape& shape, const Icon& icon, int shapeIndex);
	std::string		_GenerateID();

	bool			_HasGeometricTransform(const Shape& shape);
	void			_TransformPoint(double& x, double& y, const Shape& shape);
	double			_GetTransformScale(const Shape& shape);
	std::vector<double>	_CombineGradientMatrix(const Gradient& grad, const Shape& shape);
};

}

#endif
