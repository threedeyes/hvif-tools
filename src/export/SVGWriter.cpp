/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <sstream>
#include <iomanip>
#include <cmath>

#include "SVGWriter.h"
#include "Utils.h"

namespace haiku {

static const float HVIF_SCALE = 102.0f;

SVGWriter::SVGWriter()
	: fIdCounter(0), fIncludeNames(false), fCoordinateScale(HVIF_SCALE)
{
}

std::string
SVGWriter::Write(const Icon& icon)
{
	SVGWriterOptions opts;
	return Write(icon, opts);
}

std::string
SVGWriter::Write(const Icon& icon, const SVGWriterOptions& opts)
{
	fIncludeNames = opts.includeNames;
	fCoordinateScale = opts.coordinateScale;
	fIdCounter = 0;

	std::ostringstream svg;

	svg << "<svg width=\"" << opts.width << "\" height=\"" << opts.height 
		<< "\" viewBox=\"" << opts.viewBox << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";

	for (size_t i = 0; i < icon.shapes.size(); ++i) {
		if (icon.shapes[i].maxLOD < 3.99f)
			continue;
		svg << _ShapeToSVG(icon.shapes[i], icon, static_cast<int>(i));
	}

	svg << "</svg>";
	return svg.str();
}

std::string
SVGWriter::_FormatCoord(double value)
{
	long rounded = utils::RoundToLong(value);

	if (std::fabs(value - rounded) < 0.001)
		return utils::ToString(rounded);

	std::ostringstream oss;
	oss << std::fixed << std::setprecision(1) << value;
	std::string result = oss.str();

	size_t dotPos = result.find('.');
	if (dotPos != std::string::npos) {
		size_t lastNonZero = result.find_last_not_of('0');
		if (lastNonZero == dotPos) {
			result = result.substr(0, dotPos);
		} else if (lastNonZero != std::string::npos) {
			result = result.substr(0, lastNonZero + 1);
		}
	}

	return result;
}

std::string
SVGWriter::_ColorToCSS(const Color& color)
{
	std::ostringstream css;
	css << "#" << std::hex << std::setw(2) << std::setfill('0')
		<< static_cast<int>(color.Red())
		<< std::setw(2) << std::setfill('0')
		<< static_cast<int>(color.Green())
		<< std::setw(2) << std::setfill('0')
		<< static_cast<int>(color.Blue());
	return css.str();
}

float
SVGWriter::_GetColorAlpha(const Color& color)
{
	return static_cast<float>(color.Alpha()) / 255.0f;
}

std::string
SVGWriter::_GradientToSVG(const Gradient& grad, const std::string& id, const std::string& styleName)
{
	std::ostringstream svg;

	std::string tagName = (grad.type == GRADIENT_LINEAR || grad.type == GRADIENT_CONIC) 
		? "linearGradient" : "radialGradient";

	svg << "<" << tagName << " id=\"" << id << "\"";

	if (fIncludeNames && !styleName.empty()) {
		svg << " data-name=\"" << styleName << "\"";
	}

	svg << " gradientUnits=\"userSpaceOnUse\"";

	if (grad.hasTransform && grad.transform.size() == 6) {
		svg << " gradientTransform=\"" << _TransformToSVG(grad.transform) << "\"";
	}

	float baseCoord = 64.0f * fCoordinateScale;
	std::string baseCoordStr = _FormatCoord(baseCoord);
	std::string negBaseCoordStr = _FormatCoord(-baseCoord);

	if (grad.type == GRADIENT_LINEAR) {
		svg << " x1=\"" << negBaseCoordStr << "\" x2=\"" << baseCoordStr
		    << "\" y1=\"" << negBaseCoordStr << "\" y2=\"" << negBaseCoordStr << "\"";
	} else if (grad.type == GRADIENT_CONIC) {
		svg << " x1=\"" << baseCoordStr << "\" x2=\"" << negBaseCoordStr
		    << "\" y1=\"" << negBaseCoordStr << "\" y2=\"" << negBaseCoordStr << "\"";
	} else {
		svg << " cx=\"0\" cy=\"0\" r=\"" << baseCoordStr << "\"";
	}

	svg << ">\n";

	for (size_t i = 0; i < grad.stops.size(); ++i) {
		const ColorStop& stop = grad.stops[i];
		std::string stopColor = _ColorToCSS(stop.color);
		float alpha = _GetColorAlpha(stop.color);

		svg << "<stop offset=\"" << _FormatCoord(stop.offset * 100.0)
			<< "%\" stop-color=\"" << stopColor << "\"";

		if (alpha < 1.0f) {
			svg << " stop-opacity=\"" << utils::FormatFixed(alpha, 2) << "\"";
		}

		svg << " />\n";
	}

	svg << "</" << tagName << ">\n";

	return svg.str();
}

std::string
SVGWriter::_PathToSVG(const Path& path)
{
	std::ostringstream pathData;

	if (path.points.empty())
		return "";

	const PathPoint& first = path.points[0];
	pathData << "M " << _FormatCoord(first.x * fCoordinateScale)
	         << " " << _FormatCoord(first.y * fCoordinateScale);

	for (size_t i = 1; i < path.points.size(); ++i) {
		const PathPoint& prev = path.points[i - 1];
		const PathPoint& curr = path.points[i];

		pathData << " C "
			<< _FormatCoord(prev.x_out * fCoordinateScale) << " "
			<< _FormatCoord(prev.y_out * fCoordinateScale) << " "
			<< _FormatCoord(curr.x_in * fCoordinateScale) << " "
			<< _FormatCoord(curr.y_in * fCoordinateScale) << " "
			<< _FormatCoord(curr.x * fCoordinateScale) << " "
			<< _FormatCoord(curr.y * fCoordinateScale);
	}

	if (path.closed && path.points.size() > 1) {
		const PathPoint& last = path.points[path.points.size() - 1];
		pathData << " C "
			<< _FormatCoord(last.x_out * fCoordinateScale) << " "
			<< _FormatCoord(last.y_out * fCoordinateScale) << " "
			<< _FormatCoord(first.x_in * fCoordinateScale) << " "
			<< _FormatCoord(first.y_in * fCoordinateScale) << " "
			<< _FormatCoord(first.x * fCoordinateScale) << " "
			<< _FormatCoord(first.y * fCoordinateScale) << " Z";
	}

	return pathData.str();
}

std::string
SVGWriter::_ShapeToSVG(const Shape& shape, const Icon& icon, int shapeIndex)
{
	std::ostringstream svg;

	float opacity = 1.0f;
	std::string fillColor;
	std::string additionalDefs;

	if (shape.styleIndex >= 0 && shape.styleIndex < static_cast<int>(icon.styles.size())) {
		const Style& style = icon.styles[shape.styleIndex];

		if (style.isGradient) {
			std::string gradientId = _GenerateID();
			fillColor = "url(#" + gradientId + ")";
			additionalDefs = _GradientToSVG(style.gradient, gradientId, style.name);
		} else {
			fillColor = _ColorToCSS(style.solidColor);
			opacity = _GetColorAlpha(style.solidColor);
		}
	}

	bool isStroke = false;
	Transformer strokeTrans;

	for (size_t i = 0; i < shape.transformers.size(); ++i) {
		const Transformer& trans = shape.transformers[i];
		if (trans.type == TRANSFORMER_STROKE) {
			isStroke = true;
			strokeTrans = trans;
			break;
		}
	}

	if (!additionalDefs.empty()) {
		svg << "<g>\n";
		svg << "<defs>\n" << additionalDefs << "</defs>\n";
	}

	if (!shape.pathIndices.empty()) {
		svg << "<path";

		std::string elementId;
		if (fIncludeNames) {
			bool isDefaultName = shape.name.empty() || 
				(shape.name.size() > 2 && shape.name[0] == '<' &&
				 shape.name[shape.name.size()-1] == '>');

			if (isDefaultName) {
				elementId = "shape_" + utils::ToString(shapeIndex);
			} else {
				elementId = shape.name;
			}
		} else {
			elementId = "shape_" + utils::ToString(shapeIndex);
		}

		svg << " id=\"" << elementId << "\"";

		svg << " d=\"";
		for (size_t i = 0; i < shape.pathIndices.size(); ++i) {
			int pathIdx = shape.pathIndices[i];
			if (pathIdx >= 0 && pathIdx < static_cast<int>(icon.paths.size())) {
				svg << _PathToSVG(icon.paths[pathIdx]) << " ";
			}
		}
		svg << "\"";

		if (shape.hasTransform && shape.transform.size() == 6) {
			svg << " transform=\"" << _TransformToSVG(shape.transform) << "\"";
		}

		std::string style;
		if (isStroke) {
			double scaledWidth = strokeTrans.width * fCoordinateScale;
			style = "fill:none;stroke:" + fillColor + ";stroke-width:" + _FormatCoord(scaledWidth);
			style += ";stroke-linejoin:" + utils::GetLineJoinName(strokeTrans.lineJoin);
			style += ";stroke-linecap:" + utils::GetLineCapName(strokeTrans.lineCap);
		} else {
			style = "fill:" + fillColor + ";stroke:none";
		}

		svg << " style=\"" << style << "\"";

		if (opacity < 1.0f && fillColor.find("url(") != 0) {
			if (isStroke) {
				svg << " stroke-opacity=\"" << utils::FormatFixed(opacity, 2) << "\"";
			} else {
				svg << " fill-opacity=\"" << utils::FormatFixed(opacity, 2) << "\"";
			}
		}

		svg << " />\n";
	}

	if (!additionalDefs.empty())
		svg << "</g>\n";
	
	return svg.str();
}

std::string
SVGWriter::_TransformToSVG(const std::vector<double>& matrix)
{
	if (matrix.size() >= 6) {
		std::ostringstream result;
		result << "matrix("
			<< utils::FormatFixed(matrix[0], 6) << " "
			<< utils::FormatFixed(matrix[1], 6) << " "
			<< utils::FormatFixed(matrix[2], 6) << " "
			<< utils::FormatFixed(matrix[3], 6) << " "
			<< _FormatCoord(matrix[4] * fCoordinateScale) << " "
			<< _FormatCoord(matrix[5] * fCoordinateScale) << ")";
		return result.str();
	}
	return "";
}

std::string
SVGWriter::_GenerateID()
{
	return "grad" + utils::ToString(++fIdCounter);
}

}
