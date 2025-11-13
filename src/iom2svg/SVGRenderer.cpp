/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <sstream>
#include <iomanip>
#include <cmath>

#include "SVGRenderer.h"
#include "Utils.h"

namespace iom {

SVGRenderer::SVGRenderer(bool addNames) : fIdCounter(0), fAddNames(addNames)
{
}

SVGRenderer::~SVGRenderer()
{
}

std::string
SVGRenderer::RenderIcon(const Icon& icon, int width, int height)
{
	std::ostringstream svg;

	svg << "<svg width=\"" << width << "\" height=\"" << height 
		<< "\" viewBox=\"0 0 64 64\" xmlns=\"http://www.w3.org/2000/svg\">\n";

	for (size_t i = 0; i < icon.shapes.size(); i++) {
		if (icon.shapes[i].maxVisibility < 3.99f)
			continue;

		svg << _ShapeToSVG(icon.shapes[i], icon, static_cast<int>(i));
	}

	svg << "</svg>";
	return svg.str();
}

std::string
SVGRenderer::_ColorToCSS(uint32_t color)
{
	std::ostringstream css;
	css << "#" << std::hex << std::setw(2) << std::setfill('0')
		<< static_cast<int>(color & 0xFF)
		<< std::setw(2) << std::setfill('0')
		<< static_cast<int>((color >> 8) & 0xFF)
		<< std::setw(2) << std::setfill('0')
		<< static_cast<int>((color >> 16) & 0xFF);
	return css.str();
}

float
SVGRenderer::_GetColorAlpha(uint32_t color)
{
	return static_cast<float>((color >> 24) & 0xFF) / 255.0f;
}

std::string
SVGRenderer::_GradientToSVG(const Gradient& grad, const std::string& id, const std::string& styleName)
{
	std::ostringstream svg;

	std::string tagName = (grad.type == GRADIENT_LINEAR || grad.type == GRADIENT_CONIC) 
		? "linearGradient" : "radialGradient";

	svg << "<" << tagName << " id=\"" << id << "\"";

	if (fAddNames && !styleName.empty()) {
		svg << " data-name=\"" << styleName << "\"";
	}

	svg << " gradientUnits=\"userSpaceOnUse\"";

	if (grad.hasTransform && grad.transform.size() == 6) {
		svg << " gradientTransform=\"" << _TransformToSVG(grad.transform) << "\"";
	}

	if (grad.type == GRADIENT_LINEAR) {
		svg << " x1=\"-64\" x2=\"64\" y1=\"-64\" y2=\"-64\"";
	} else if (grad.type == GRADIENT_CONIC) {
		svg << " x1=\"64\" x2=\"-64\" y1=\"-64\" y2=\"-64\"";
	} else {
		svg << " cx=\"0\" cy=\"0\" r=\"64\"";
	}

	svg << ">\n";

	for (size_t i = 0; i < grad.stops.size(); i++) {
		const ColorStop& stop = grad.stops[i];
		std::string stopColor = _ColorToCSS(stop.color);
		float alpha = _GetColorAlpha(stop.color);

		svg << "<stop offset=\"" << utils::FormatFixed(stop.offset * 100.0, 2)
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
SVGRenderer::_PathToSVG(const Path& path)
{
	std::ostringstream pathData;

	if (path.points.empty())
		return "";

	const ControlPoint& first = path.points[0];
	pathData << "M " << first.x << " " << first.y;

	for (size_t i = 1; i < path.points.size(); i++) {
		const ControlPoint& prev = path.points[i - 1];
		const ControlPoint& curr = path.points[i];

		pathData << " C " << prev.x_out << " " << prev.y_out << " "
				 << curr.x_in << " " << curr.y_in << " "
				 << curr.x << " " << curr.y;
	}

	if (path.closed && path.points.size() > 1) {
		const ControlPoint& last = path.points[path.points.size() - 1];
		pathData << " C " << last.x_out << " " << last.y_out << " "
				 << first.x_in << " " << first.y_in << " "
				 << first.x << " " << first.y << " Z";
	}

	return pathData.str();
}

std::string
SVGRenderer::_ShapeToSVG(const Shape& shape, const Icon& icon, int shapeIndex)
{
	std::ostringstream svg;

	float opacity = 1.0f;
	std::string fillColor;
	std::string additionalDefs;

	if (shape.styleIndex < static_cast<int>(icon.styles.size())) {
		const Style& style = icon.styles[shape.styleIndex];

		if (style.isGradient) {
			std::string gradientId = _GenerateID();
			fillColor = "url(#" + gradientId + ")";
			additionalDefs = _GradientToSVG(style.gradient, gradientId);
		} else {
			fillColor = _ColorToCSS(style.color);
			opacity = _GetColorAlpha(style.color);
		}
	}

	bool isStroke = false;
	Transformer strokeTrans;

	for (size_t i = 0; i < shape.transformers.size(); i++) {
		const Transformer& trans = shape.transformers[i];
		if (trans.type == TRANSFORMER_STROKE) {
			isStroke = true;
			strokeTrans = trans;
			break;
		} else if (trans.type == TRANSFORMER_CONTOUR) {
			continue;
		}
	}

	if (!additionalDefs.empty()) {
		svg << "<g>\n";
		svg << "<defs>\n" << additionalDefs << "</defs>\n";
	}

	if (!shape.pathIndices.empty()) {
		svg << "<path";

		std::string elementId;
		if (fAddNames) {
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
		for (size_t i = 0; i < shape.pathIndices.size(); i++) {
			int pathIdx = shape.pathIndices[i];
			if (pathIdx >= 0 && pathIdx < static_cast<int>(icon.paths.size())) {
				svg << _PathToSVG(icon.paths[pathIdx]) << " ";
			}
		}
		svg << "\" ";

		if (shape.hasTransform && shape.transform.size() == 6) {
			svg << "transform=\"" << _TransformToSVG(shape.transform) << "\" ";
		}

		std::string style;
		if (isStroke) {
			style = "fill:none;stroke:" + fillColor + ";stroke-width:" + utils::ToString(strokeTrans.width);
			style += ";stroke-linejoin:" + utils::GetLineJoinName(strokeTrans.lineJoin);
			style += ";stroke-linecap:" + utils::GetLineCapName(strokeTrans.lineCap);
		} else {
			style = "fill:" + fillColor + ";stroke:none";
		}

		svg << "style=\"" << style << "\"";

		if (opacity < 1.0f && fillColor.find("url(") != 0) {
			if (isStroke) {
				svg << " stroke-opacity=\"" << opacity << "\"";
			} else {
				svg << " fill-opacity=\"" << opacity << "\"";
			}
		}

		svg << " />\n";
	}

	if (!additionalDefs.empty()) {
		svg << "</g>\n";
	}

	return svg.str();
}

std::string
SVGRenderer::_TransformToSVG(const std::vector<double>& matrix)
{
	if (matrix.size() >= 6) {
		std::ostringstream result;
		result << "matrix(" << utils::FormatFixed(matrix[0], 6) << " " << utils::FormatFixed(matrix[1], 6) << " "
			   << utils::FormatFixed(matrix[2], 6) << " " << utils::FormatFixed(matrix[3], 6) << " "
			   << utils::FormatFixed(matrix[4], 2) << " " << utils::FormatFixed(matrix[5], 2) << ")";
		return result.str();
	}
	return "";
}

std::string
SVGRenderer::_GenerateID()
{
	return "iom" + utils::ToString(++fIdCounter);
}

}
