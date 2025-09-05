/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstdlib>

#include "SVGRenderer.h"

namespace hvif {

template<typename T>
std::string ToString(T value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

SVGRenderer::SVGRenderer() : fIdCounter(0)
{
}

SVGRenderer::~SVGRenderer()
{
}

std::string
SVGRenderer::RenderIcon(const HVIFIcon& icon, int width, int height)
{
	std::ostringstream svg;
	std::string id = _GenerateID();

	svg << "<svg width=\"" << width << "\" height=\"" << height 
		<< "\" viewBox=\"0 0 6528 6528\" xmlns=\"http://www.w3.org/2000/svg\">\n";

	for (size_t i = 0; i < icon.shapes.size(); i++) {
		svg << _ShapeToSVG(icon.shapes[i], icon, id, static_cast<int>(i));
	}

	svg << "</svg>";
	return svg.str();
}

std::string
SVGRenderer::_ColorToCSS(const Color& color)
{
	std::ostringstream css;
	css << "#";

	switch (color.tag) {
		case K:
		case KA: {
			if (!color.data.empty()) {
				int gray = color.data[0];
				css << std::hex << std::setw(2) << std::setfill('0') << gray
					<< std::setw(2) << std::setfill('0') << gray
					<< std::setw(2) << std::setfill('0') << gray;
			} else {
				css << "000000";
			}
			break;
		}
		case RGB:
		case RGBA: {
			if (color.data.size() >= 3) {
				for (int i = 0; i < 3; i++) {
					css << std::hex << std::setw(2) << std::setfill('0') 
						<< static_cast<int>(color.data[i]);
				}
			} else {
				css << "000000";
			}
			break;
		}
		default:
			css << "000000";
			break;
	}

	return css.str();
}

float
SVGRenderer::_GetColorAlpha(const Color& color)
{
	if (color.tag == KA && color.data.size() >= 2) {
		return static_cast<float>(color.data[1]) / 255.0f;
	} else if (color.tag == RGBA && color.data.size() >= 4) {
		return static_cast<float>(color.data[3]) / 255.0f;
	}
	return 1.0f;
}

std::string
SVGRenderer::_GradientToSVG(const Gradient& grad, const std::string& id)
{
	std::ostringstream svg;

	std::string tagName = (grad.type == LINEAR) ? "linearGradient" : "radialGradient";

	svg << "<" << tagName << " id=\"" << id << "\" gradientUnits=\"userSpaceOnUse\"";

	if (grad.hasMatrix) {
		svg << " gradientTransform=\"" << _MatrixToSVG(grad.matrix) << "\"";
	}

	if (grad.type == LINEAR) {
		float x1 = -64.0f * 102;
		float x2 = 64.0f * 102;
		float y1 = -64.0f * 102;
		float y2 = -64.0f * 102;
		svg << " x1=\"" << x1 << "\" x2=\"" << x2 << "\" y1=\"" << y1 << "\" y2=\"" << y2 << "\"";
	} else {
		svg << " cx=\"0\" cy=\"0\" r=\"" << (64 * 102) << "\"";
	}

	svg << ">\n";

	for (size_t i = 0; i < grad.stops.size(); i++) {
		const GradientStop& stop = grad.stops[i];
		float offset = static_cast<float>(stop.offset) / 2.55f;
		std::string stopColor = _ColorToCSS(stop.color);
		float alpha = _GetColorAlpha(stop.color);

		svg << "<stop offset=\"" << std::fixed << std::setprecision(2) << offset << "%\" stop-color=\"" << stopColor << "\"";

		if (alpha < 1.0f) {
			svg << " stop-opacity=\"" << std::fixed << std::setprecision(2) << alpha << "\"";
		}

		svg << " />\n";
	}

	svg << "</" << tagName << ">\n";

	return svg.str();
}

std::string
SVGRenderer::_ShapeToSVG(const Shape& shape, const HVIFIcon& icon, 
									 const std::string& id, int shapeIndex)
{
	std::ostringstream svg;

	std::vector<Path> paths;
	for (size_t i = 0; i < shape.pathIndices.size(); i++) {
		uint8_t pathIndex = shape.pathIndices[i];
		if (pathIndex < icon.paths.size()) {
			paths.push_back(icon.paths[pathIndex]);
		}
	}

	float opacity = 1.0f;
	std::string fillColor;
	std::string additionalDefs;

	if (shape.styleIndex < icon.styles.size()) {
		const Style& style = icon.styles[shape.styleIndex];
		if (style.isGradient) {
			std::string gradientId = id + "-g" + ToString(static_cast<int>(shape.styleIndex));
			fillColor = "url(#" + gradientId + ")";
			additionalDefs = _GradientToSVG(style.gradient, gradientId);
			opacity = 1.0f;
		} else {
			fillColor = _ColorToCSS(style.color);
			opacity = _GetColorAlpha(style.color);
		}
	}

	std::string effectType = "fill";
	Transformer strokeEffect;
	bool hasStroke = false;

	for (size_t i = 0; i < shape.transformers.size(); i++) {
		const Transformer& transformer = shape.transformers[i];

		if (transformer.tag == STROKE) {
			effectType = "stroke";
			strokeEffect = transformer;
			hasStroke = true;
			break;
		} else if (transformer.tag == CONTOUR) {
			if (opacity >= 1.0f - 1e-6f) {
				effectType = "stroke";
				strokeEffect = transformer;
				hasStroke = true;
				break;
			} else {
				// Ignore contour for transparent shapes
				continue;
			}
		}
	}

	if (!additionalDefs.empty()) {
		svg << "<g>\n";
		svg << "<defs>\n" << additionalDefs << "</defs>\n";
	}

	std::string pathData = _PathToSVG(paths);

	svg << "<path d=\"" << pathData << "\" ";

	if (shape.hasTransform) {
		svg << "transform=\"" << _TransformToSVG(shape.transform, shape.transformType) << "\" ";
	}

	std::string style;
	if (effectType == "stroke") {
		style = "fill:none;stroke:" + fillColor + ";stroke-width:" + ToString(strokeEffect.width);
		style += ";stroke-linejoin:" + _GetLineJoinName(strokeEffect.lineJoin);
		style += ";stroke-linecap:" + _GetLineCapName(strokeEffect.lineCap);
	} else {
		style = "fill:" + fillColor + ";stroke:none";
	}

	svg << "style=\"" << style << "\"";

	if (opacity < 1.0f && fillColor.find("url(") != 0) {
		if (effectType == "stroke") {
			svg << " stroke-opacity=\"" << opacity << "\"";
		} else {
			svg << " fill-opacity=\"" << opacity << "\"";
		}
	}

	svg << " />\n";

	if (!additionalDefs.empty()) {
		svg << "</g>\n";
	}

	return svg.str();
}

std::string
SVGRenderer::_PathToSVG(const std::vector<Path>& paths)
{
	std::ostringstream pathData;
	
	for (int idx = static_cast<int>(paths.size()) - 1; idx >= 0; idx--) {
		const Path& path = paths[idx];
		
		if (path.type == "points") {
			if (path.points.size() >= 2) {
				pathData << "M " << path.points[0] << " " << path.points[1] << " L";
				for (size_t i = 2; i < path.points.size(); i++) {
					pathData << " " << path.points[i];
				}
				if (path.closed) {
					pathData << " Z";
				}
				pathData << " ";
			}
		} else if (path.type == "curves" && path.points.size() >= 6) {
			float x0 = path.points[0];
			float y0 = path.points[1];
			float xout = path.points[4];
			float yout = path.points[5];

			pathData << "M " << x0 << " " << y0;

			for (size_t i = 6; i < path.points.size(); i += 6) {
				if (i + 5 < path.points.size()) {
					float x = path.points[i];
					float y = path.points[i + 1];
					float xin = path.points[i + 2];
					float yin = path.points[i + 3];
					float xout_next = path.points[i + 4];
					float yout_next = path.points[i + 5];
					
					pathData << " C " << xout << " " << yout << " " 
							 << xin << " " << yin << " " << x << " " << y;

					xout = xout_next;
					yout = yout_next;
				}
			}

			if (path.closed && path.points.size() >= 6) {
				float xin0 = path.points[2];
				float yin0 = path.points[3];
				pathData << " C " << xout << " " << yout << " " 
						 << xin0 << " " << yin0 << " " << x0 << " " << y0;
				pathData << " Z";
			}
			pathData << " ";
		}
	}

	return pathData.str();
}

std::string
SVGRenderer::_TransformToSVG(const std::vector<float>& transform, 
										 const std::string& type)
{
	if (type == "matrix") {
		return _MatrixToSVG(transform);
	} else if (type == "translate") {
		if (transform.size() >= 2) {
			return "translate(" + ToString(transform[0]) + " " + 
				   ToString(transform[1]) + ")";
		}
	}
	return "";
}

std::string
SVGRenderer::_MatrixToSVG(const std::vector<float>& matrix)
{
	if (matrix.size() >= 6) {
		std::ostringstream result;
		result << "matrix(" << matrix[0] << " " << matrix[1] << " " 
			   << matrix[2] << " " << matrix[3] << " " 
			   << (matrix[4] * 102) << " " << (matrix[5] * 102) << ")";
		return result.str();        
	}
	return "";
}

std::string
SVGRenderer::_GetLineJoinName(uint8_t lineJoin)
{
	switch (lineJoin) {
		case MITER: return "miter";
		case ROUND: return "round";
		case BEVEL: return "bevel";
		case MITER_ROUND: return "miter";
		default: return "miter";
	}
}

std::string
SVGRenderer::_GetLineCapName(uint8_t lineCap)
{
	switch (lineCap) {
		case BUTT: return "butt";
		case SQUARE: return "square";
		case ROUND_CAP: return "round";
		default: return "butt";
	}
}

std::string
SVGRenderer::_GenerateID()
{
	return "hvif" + ToString(++fIdCounter);
}

}
