/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstring>

#include "nanosvg.h"

#include "SVGParser.h"
#include "Utils.h"

namespace haiku {

bool
SVGParser::Parse(const std::string& svgFile, Icon& icon)
{
	SVGParseOptions opts;
	return Parse(svgFile, icon, opts);
}

bool
SVGParser::Parse(const std::string& svgFile, Icon& icon, const SVGParseOptions& opts)
{
	NSVGimage* image = nsvgParseFromFile(svgFile.c_str(), "px", 96.0f);
	if (!image) {
		if (opts.verbose)
			std::cerr << "Error: Could not parse SVG file " << svgFile << std::endl;
		return false;
	}

	bool result = _ProcessImage(image, icon, opts);
	nsvgDelete(image);
	return result;
}

bool
SVGParser::ParseString(const std::string& svg, Icon& icon)
{
	SVGParseOptions opts;
	return ParseString(svg, icon, opts);
}

bool
SVGParser::ParseString(const std::string& svg, Icon& icon, const SVGParseOptions& opts)
{
	std::vector<char> buffer;
	buffer.reserve(svg.size() + 1);
	for (size_t i = 0; i < svg.size(); ++i)
		buffer.push_back(svg[i]);
	buffer.push_back('\0');

	NSVGimage* image = nsvgParse(&buffer[0], "px", 96.0f);
	if (!image) {
		if (opts.verbose)
			std::cerr << "Error: Could not parse SVG data" << std::endl;
		return false;
	}

	bool result = _ProcessImage(image, icon, opts);
	nsvgDelete(image);
	return result;
}

bool
SVGParser::ParseBuffer(const char* svgData, size_t dataSize, Icon& icon)
{
	SVGParseOptions opts;
	return ParseBuffer(svgData, dataSize, icon, opts);
}

bool
SVGParser::ParseBuffer(const char* svgData, size_t dataSize, Icon& icon, const SVGParseOptions& opts)
{
	std::string svgString(svgData, dataSize);
	return ParseString(svgString, icon, opts);
}

bool
SVGParser::_ProcessImage(NSVGimage* image, Icon& icon, const SVGParseOptions& opts)
{
	ParseState state;
	state.icon = &icon;
	state.verbose = opts.verbose;

	float svg_w = image->width;
	float svg_h = image->height;

	state.scale = opts.targetSize / std::max(svg_w, svg_h);
	state.tx = (opts.targetSize - svg_w * state.scale) / 2.0f;
	state.ty = (opts.targetSize - svg_h * state.scale) / 2.0f;

	if (opts.verbose) {
		std::cout << "SVG dimensions: " << svg_w << "x" << svg_h 
			<< ", scale: " << state.scale 
			<< ", translate: (" << state.tx << ", " << state.ty << ")" << std::endl;
	}

	for (NSVGshape* shape = image->shapes; shape != NULL; shape = shape->next) {
		_ProcessShape(shape, image, state);
	}

	return true;
}

void
SVGParser::_ProcessShape(NSVGshape* shape, NSVGimage* image, ParseState& state)
{
	if (!(shape->flags & NSVG_FLAGS_VISIBLE))
		return;

	if (shape->mask != NULL) {
		_ProcessMaskedShape(shape, state);
		return;
	}

	std::string shapeName = "";
	if (shape->id[0] != '\0')
		shapeName = shape->id;

	int fillStyleIndex = -1;
	int strokeStyleIndex = -1;
	Transformer strokeTransformer;
	bool hasFill = false;
	bool hasStroke = false;

	if (shape->fill.type != NSVG_PAINT_NONE) {
		fillStyleIndex = _AddStyle(shape->fill, shape->opacity, state);
		hasFill = true;
	}

	if (shape->stroke.type != NSVG_PAINT_NONE && shape->strokeWidth > 0.0f) {
		strokeStyleIndex = _AddStyle(shape->stroke, shape->opacity, state);
		strokeTransformer.type = TRANSFORMER_STROKE;
		strokeTransformer.width = shape->strokeWidth * state.scale;
		strokeTransformer.lineCap = utils::MapCapFromNanoSVG(shape->strokeLineCap);
		strokeTransformer.lineJoin = utils::MapJoinFromNanoSVG(shape->strokeLineJoin);
		strokeTransformer.miterLimit = shape->miterLimit;
		hasStroke = true;
	}

	if (hasStroke) {
		std::vector<int> strokePathIndices;
		for (NSVGpath* path = shape->paths; path != NULL; path = path->next) {
			int pathIndex = _ProcessPath(path, state);
			if (pathIndex >= 0)
				strokePathIndices.push_back(pathIndex);
		}

		if (!strokePathIndices.empty()) {
			Shape strokeShape;
			strokeShape.styleIndex = strokeStyleIndex;
			strokeShape.hasTransform = false;
			strokeShape.pathIndices = strokePathIndices;
			strokeShape.transformers.push_back(strokeTransformer);
			strokeShape.name = shapeName;
			state.icon->shapes.push_back(strokeShape);
		}
	}

	if (hasFill) {
		std::vector<int> fillPathIndices;
		for (NSVGpath* path = shape->paths; path != NULL; path = path->next) {
			int pathIndex = _ProcessPath(path, state);
			if (pathIndex >= 0)
				fillPathIndices.push_back(pathIndex);
		}

		if (!fillPathIndices.empty()) {
			Shape fillShape;
			fillShape.styleIndex = fillStyleIndex;
			fillShape.hasTransform = false;
			fillShape.pathIndices = fillPathIndices;
			fillShape.name = shapeName;
			state.icon->shapes.push_back(fillShape);
		}
	}
}

void
SVGParser::_ProcessMaskedShape(NSVGshape* shape, ParseState& state)
{
	NSVGmask* mask = shape->mask;
	if (!mask || !mask->shapes)
		return;

	int fillStyleIndex = -1;
	float opacity = shape->opacity;

	if (shape->fill.type != NSVG_PAINT_NONE) {
		fillStyleIndex = _AddStyle(shape->fill, opacity, state);
	} else if (shape->stroke.type != NSVG_PAINT_NONE) {
		fillStyleIndex = _AddStyle(shape->stroke, opacity, state);
	} else {
		return;
	}

	for (NSVGshape* maskShape = mask->shapes; maskShape != NULL; maskShape = maskShape->next) {
		if (!(maskShape->flags & NSVG_FLAGS_VISIBLE))
			continue;

		std::vector<int> pathIndices;
		for (NSVGpath* path = maskShape->paths; path != NULL; path = path->next) {
			int pathIndex = _ProcessPath(path, state);
			if (pathIndex >= 0)
				pathIndices.push_back(pathIndex);
		}

		if (pathIndices.empty())
			continue;

		Shape iconShape;
		iconShape.styleIndex = fillStyleIndex;
		iconShape.hasTransform = false;
		iconShape.pathIndices = pathIndices;

		if (maskShape->stroke.type != NSVG_PAINT_NONE && maskShape->strokeWidth > 0.0f) {
			Transformer contour;
			contour.type = TRANSFORMER_CONTOUR;
			contour.width = maskShape->strokeWidth * state.scale;
			contour.lineJoin = utils::MapJoinFromNanoSVG(maskShape->strokeLineJoin);
			contour.miterLimit = maskShape->miterLimit;
			iconShape.transformers.push_back(contour);
		}

		state.icon->shapes.push_back(iconShape);
	}
}

int
SVGParser::_ProcessPath(NSVGpath* path, ParseState& state)
{
	if (path->npts < 2)
		return -1;

	Path iconPath;
	iconPath.closed = path->closed;

	for (int i = 0; i < path->npts - 1; i += 3) {
		float* p = &path->pts[i * 2];
		float p0_x = p[0];
		float p0_y = p[1];
		float c1_x = p[2];
		float c1_y = p[3];
		float c2_x = p[4];
		float c2_y = p[5];
		float p1_x = p[6];
		float p1_y = p[7];

		if (i == 0) {
			PathPoint pt;
			pt.x = p0_x * state.scale + state.tx;
			pt.y = p0_y * state.scale + state.ty;
			pt.x_in = pt.x;
			pt.y_in = pt.y;
			pt.x_out = c1_x * state.scale + state.tx;
			pt.y_out = c1_y * state.scale + state.ty;
			pt.connected = false;
			iconPath.points.push_back(pt);
		} else {
			iconPath.points[iconPath.points.size() - 1].x_out = c1_x * state.scale + state.tx;
			iconPath.points[iconPath.points.size() - 1].y_out = c1_y * state.scale + state.ty;
		}

		PathPoint pt;
		pt.x = p1_x * state.scale + state.tx;
		pt.y = p1_y * state.scale + state.ty;
		pt.x_in = c2_x * state.scale + state.tx;
		pt.y_in = c2_y * state.scale + state.ty;
		pt.x_out = pt.x;
		pt.y_out = pt.y;
		pt.connected = false;
		iconPath.points.push_back(pt);
	}

	if (iconPath.closed && iconPath.points.size() > 1) {
		if (path->npts >= 4) {
			float* last_segment_pts = &path->pts[(path->npts - 4) * 2];
			if (last_segment_pts + 5 < &path->pts[path->npts * 2]) {
				iconPath.points[0].x_in = last_segment_pts[4] * state.scale + state.tx;
				iconPath.points[0].y_in = last_segment_pts[5] * state.scale + state.ty;
			}
		}

		const PathPoint& first = iconPath.points[0];
		PathPoint& last = iconPath.points[iconPath.points.size() - 1];
		if (utils::FloatEqual(static_cast<float>(first.x), static_cast<float>(last.x), 0.01f) && 
			utils::FloatEqual(static_cast<float>(first.y), static_cast<float>(last.y), 0.01f)) {
			iconPath.points[0].x_in = last.x_in;
			iconPath.points[0].y_in = last.y_in;
			iconPath.points.pop_back();
		}
	}

	int pathIndex = static_cast<int>(state.icon->paths.size());
	state.icon->paths.push_back(iconPath);
	return pathIndex;
}

int
SVGParser::_AddStyle(const NSVGpaint& paint, float opacity, ParseState& state)
{
	Style style;

	if (paint.type == NSVG_PAINT_COLOR) {
		style.isGradient = false;
		style.solidColor = _NSVGColorToHaiku(paint.color, opacity);
	} else if (paint.type >= NSVG_PAINT_LINEAR_GRADIENT) {
		style.isGradient = true;
		NSVGgradient* grad = paint.gradient;

		if (paint.type == NSVG_PAINT_LINEAR_GRADIENT)
			style.gradient.type = GRADIENT_LINEAR;
		else
			style.gradient.type = GRADIENT_RADIAL;

		style.gradient.interpolation = INTERPOLATION_LINEAR;

		for (int i = 0; i < grad->nstops; ++i) {
			ColorStop stop;
			stop.color = _NSVGColorToHaiku(grad->stops[i].color, opacity);
			stop.offset = grad->stops[i].offset;
			style.gradient.stops.push_back(stop);
		}

		_CalculateGradientTransform(paint, style.gradient, state);
	} else {
		style.isGradient = false;
		style.solidColor = _NSVGColorToHaiku(0x000000FF, opacity);
	}

	for (size_t i = 0; i < state.icon->styles.size(); ++i) {
		const Style& existing = state.icon->styles[i];
		if (existing.isGradient == style.isGradient) {
			if (!style.isGradient && existing.solidColor.argb == style.solidColor.argb) {
				return static_cast<int>(i);
			}
		}
	}

	int styleIndex = static_cast<int>(state.icon->styles.size());
	state.icon->styles.push_back(style);
	return styleIndex;
}

Color
SVGParser::_NSVGColorToHaiku(unsigned int color, float opacity)
{
	uint8_t r = color & 0xff;
	uint8_t g = (color >> 8) & 0xff;
	uint8_t b = (color >> 16) & 0xff;
	uint8_t a = (color >> 24) & 0xff;

	a = static_cast<uint8_t>(a * opacity);

	uint32_t argb = (static_cast<uint32_t>(a) << 24) | 
	                (static_cast<uint32_t>(r) << 16) | 
	                (static_cast<uint32_t>(g) << 8) | 
	                static_cast<uint32_t>(b);
	return Color(argb);
}

void
SVGParser::_CalculateGradientTransform(const NSVGpaint& paint, Gradient& grad, const ParseState& state)
{
	NSVGgradient* g = paint.gradient;
	if (!g)
		return;

	float M[6];
	utils::InvertAffine(M, g->xform);

	double a = 1.0, b = 0.0, c = 0.0, d = 1.0, tx = 0.0, ty = 0.0;

	if (paint.type == NSVG_PAINT_LINEAR_GRADIENT) {
		double baseA = 0.0, baseB = 1.0/128.0;
		double baseC = -1.0/128.0, baseD = 0.0;
		double baseTx = -0.5, baseTy = 0.5;

		double newA = a * baseA + b * baseC;
		double newB = a * baseB + b * baseD;
		double newC = c * baseA + d * baseC;
		double newD = c * baseB + d * baseD;
		double newTx = tx * baseA + ty * baseC + baseTx;
		double newTy = tx * baseB + ty * baseD + baseTy;

		a = newA; b = newB; c = newC; d = newD; tx = newTx; ty = newTy;
	} else if (paint.type == NSVG_PAINT_RADIAL_GRADIENT) {
		double baseA = 0.0, baseB = 1.0/64.0;
		double baseC = -1.0/64.0, baseD = 0.0;
		double baseTx = 0.0, baseTy = 0.0;

		double newA = a * baseA + b * baseC;
		double newB = a * baseB + b * baseD;
		double newC = c * baseA + d * baseC;
		double newD = c * baseB + d * baseD;
		double newTx = tx * baseA + ty * baseC + baseTx;
		double newTy = tx * baseB + ty * baseD + baseTy;

		a = newA; b = newB; c = newC; d = newD; tx = newTx; ty = newTy;
	}

	{
		double invA = M[0], invB = M[1], invC = M[2], invD = M[3], invTx = M[4], invTy = M[5];

		double newA = a * invA + b * invC;
		double newB = a * invB + b * invD;
		double newC = c * invA + d * invC;
		double newD = c * invB + d * invD;
		double newTx = tx * invA + ty * invC + invTx;
		double newTy = tx * invB + ty * invD + invTy;

		a = newA; b = newB; c = newC; d = newD; tx = newTx; ty = newTy;
	}

	{
		double s = state.scale;
		double shiftX = state.tx;
		double shiftY = state.ty;

		a *= s;
		b *= s;
		c *= s;
		d *= s;
		tx = tx * s + shiftX;
		ty = ty * s + shiftY;
	}

	grad.transform.clear();
	grad.transform.push_back(a);
	grad.transform.push_back(b);
	grad.transform.push_back(c);
	grad.transform.push_back(d);
	grad.transform.push_back(tx);
	grad.transform.push_back(ty);
	grad.hasTransform = true;
}

}
