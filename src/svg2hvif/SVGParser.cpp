/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <iostream>
#include <cmath>
#include <vector>
#include <cstring>
#include <algorithm>

#include "nanosvg.h"

#include "SVGParser.h"
#include "Utils.h"

namespace hvif {

bool
SVGParser::ParseFile(const std::string& svgFile, hvif::HVIFWriter& writer)
{
	NSVGimage* image = nsvgParseFromFile(svgFile.c_str(), "px", 96.0f);
	if (!image) {
		if (fVerbose) std::cerr << "Error: Could not parse SVG file " << svgFile << std::endl;
		return false;
	}

	bool result = _ProcessImage(image, writer);
	nsvgDelete(image);
	return result;
}

bool
SVGParser::ParseBuffer(const char* svgData, size_t dataSize, hvif::HVIFWriter& writer)
{
	std::string svgString(svgData, dataSize);
	return ParseString(svgString, writer);
}

bool
SVGParser::ParseBuffer(const std::vector<uint8_t>& svgData, hvif::HVIFWriter& writer)
{
	return ParseBuffer(reinterpret_cast<const char*>(&svgData[0]), svgData.size(), writer);
}

bool
SVGParser::ParseString(const std::string& svgString, hvif::HVIFWriter& writer)
{
	std::vector<char> buffer(svgString.begin(), svgString.end());
	buffer.push_back('\0');

	NSVGimage* image = nsvgParse(&buffer[0], "px", 96.0f);
	if (!image) {
		if (fVerbose) std::cerr << "Error: Could not parse SVG data" << std::endl;
		return false;
	}

	bool result = _ProcessImage(image, writer);
	nsvgDelete(image);
	return result;
}

bool
SVGParser::_ProcessImage(NSVGimage* image, hvif::HVIFWriter& writer)
{
	ParseState state;
	state.writer = &writer;

	float svg_w = image->width;
	float svg_h = image->height;

	state.scale = 64.0f / std::max(svg_w, svg_h);
	state.tx = (64.0f - svg_w * state.scale) / 2.0f;
	state.ty = (64.0f - svg_h * state.scale) / 2.0f;

	if (fVerbose) {
		std::cout << "SVG dimensions: " << svg_w << "x" << svg_h 
				  << ", scale: " << state.scale 
				  << ", translate: (" << state.tx << ", " << state.ty << ")" << std::endl;
	}

	for (NSVGshape* shape = image->shapes; shape != NULL; shape = shape->next) {
		_ProcessShape(shape, state);
	}

	return true;
}

void
SVGParser::_ProcessShape(NSVGshape* shape, ParseState& state)
{
	if (!(shape->flags & NSVG_FLAGS_VISIBLE))
		return;

	if (shape->fill.type != NSVG_PAINT_NONE) {
		uint8_t styleIndex = _AddStyle(shape->fill, shape->opacity, state);
		hvif::Shape hvifShape;
		hvifShape.styleIndex = styleIndex;
		for (NSVGpath* path = shape->paths; path != NULL; path = path->next) {
			uint8_t pathIndex = _ProcessPath(path, state);
			hvifShape.pathIndices.push_back(pathIndex);
		}
		state.writer->AddShape(hvifShape);
	}

	if (shape->stroke.type != NSVG_PAINT_NONE && shape->strokeWidth > 0.0f) {
		uint8_t styleIndex = _AddStyle(shape->stroke, shape->opacity, state);
		hvif::Shape hvifShape;
		hvifShape.styleIndex = styleIndex;
		for (NSVGpath* path = shape->paths; path != NULL; path = path->next) {
			uint8_t pathIndex = _ProcessPath(path, state);
			hvifShape.pathIndices.push_back(pathIndex);
		}
		hvif::Transformer t;
		t.tag = hvif::STROKE;
		t.width = shape->strokeWidth * state.scale;
		t.lineCap  = utils::MapCapFromNanoSVG(shape->strokeLineCap);
		t.lineJoin = utils::MapJoinFromNanoSVG(shape->strokeLineJoin);
		t.miterLimit = static_cast<uint8_t>(
			utils::clamp<int>(static_cast<int>(utils::RoundToLong(shape->miterLimit)), 0, 255)
		);
		hvifShape.transformers.push_back(t);
		state.writer->AddShape(hvifShape);
	}
}

uint8_t
SVGParser::_ProcessPath(NSVGpath* path, ParseState& state)
{
	if (path->npts < 2)
		return 0;

	hvif::InternalPath hvifPath;
	hvifPath.closed = path->closed;

	for (int i = 0; i < path->npts - 1; i += 3) {
		float* p = &path->pts[i * 2];
		float p0[2] = {p[0], p[1]};
		float c1[2] = {p[2], p[3]};
		float c2[2] = {p[4], p[5]};
		float p1[2] = {p[6], p[7]};

		if (i == 0) {
			hvif::PathNode startNode;
			startNode.x = p0[0] * state.scale + state.tx;
			startNode.y = p0[1] * state.scale + state.ty;
			startNode.x_out = c1[0] * state.scale + state.tx;
			startNode.y_out = c1[1] * state.scale + state.ty;
			startNode.x_in = startNode.x;
			startNode.y_in = startNode.y;
			hvifPath.nodes.push_back(startNode);
		}

		if (!hvifPath.nodes.empty()) {
			hvifPath.nodes.back().x_out = c1[0] * state.scale + state.tx;
			hvifPath.nodes.back().y_out = c1[1] * state.scale + state.ty;
		}

		hvif::PathNode endNode;
		endNode.x = p1[0] * state.scale + state.tx;
		endNode.y = p1[1] * state.scale + state.ty;
		endNode.x_in = c2[0] * state.scale + state.tx;
		endNode.y_in = c2[1] * state.scale + state.ty;
		endNode.x_out = endNode.x;
		endNode.y_out = endNode.y;

		hvifPath.nodes.push_back(endNode);
	}

	if (hvifPath.closed && hvifPath.nodes.size() > 1) {
		if (path->npts >= 4) {
			float* last_segment_pts = &path->pts[(path->npts - 4) * 2];
			if (last_segment_pts + 5 < &path->pts[path->npts * 2]) {
				hvifPath.nodes[0].x_in = last_segment_pts[4] * state.scale + state.tx;
				hvifPath.nodes[0].y_in = last_segment_pts[5] * state.scale + state.ty;
			}
		}

		if (hvifPath.nodes.size() > 1) {
			const hvif::PathNode& firstNode = hvifPath.nodes[0];
			hvif::PathNode& lastNode = hvifPath.nodes.back();
			if (utils::FloatEqual(firstNode.x, lastNode.x) && utils::FloatEqual(firstNode.y, lastNode.y)) {
				hvifPath.nodes[0].x_in = lastNode.x_in;
				hvifPath.nodes[0].y_in = lastNode.y_in;
				hvifPath.nodes.pop_back();
			}
		}
	}

	return state.writer->AddInternalPath(hvifPath);
}

uint8_t
SVGParser::_AddStyle(const NSVGpaint& paint, float opacity, ParseState& state)
{
	hvif::Style style;
	if (paint.type == NSVG_PAINT_COLOR) {
		style.isGradient = false;
		unsigned int c = paint.color;
		style.color.tag = hvif::RGBA;

		uint8_t r = c & 0xff;
		uint8_t g = (c >> 8) & 0xff;
		uint8_t b = (c >> 16) & 0xff;
		uint8_t alpha = (c >> 24) & 0xff;

		style.color.data.push_back(r);
		style.color.data.push_back(g);
		style.color.data.push_back(b);
		style.color.data.push_back(static_cast<uint8_t>(alpha * opacity));
	} else if (paint.type >= NSVG_PAINT_LINEAR_GRADIENT) {
		style.isGradient = true;
		NSVGgradient* grad = paint.gradient;
		style.gradient.type = (paint.type == NSVG_PAINT_LINEAR_GRADIENT) ? 
				hvif::LINEAR : hvif::RADIAL;
		style.gradient.flags = 0;
		for (int i = 0; i < grad->nstops; ++i) {
			unsigned int c = grad->stops[i].color;
			hvif::GradientStop stop;
			stop.offset = static_cast<uint8_t>(grad->stops[i].offset * 255.0f);
			stop.color.tag = hvif::RGBA;

			uint8_t r = c & 0xff;
			uint8_t g = (c >> 8) & 0xff;
			uint8_t b = (c >> 16) & 0xff;
			uint8_t alpha = (c >> 24) & 0xff;

			stop.color.data.push_back(r);
			stop.color.data.push_back(g);
			stop.color.data.push_back(b);
			stop.color.data.push_back(static_cast<uint8_t>(alpha * opacity));
			style.gradient.stops.push_back(stop);
		}
		_CalculateGradientTransform(paint, style.gradient, state);
	} else {
		style.isGradient = false;
		style.color.tag = hvif::RGBA;
		style.color.data.push_back(0);
		style.color.data.push_back(0);
		style.color.data.push_back(0);
		style.color.data.push_back(static_cast<uint8_t>(255 * opacity));
	}
	return state.writer->AddStyle(style);
}

void
SVGParser::_CalculateGradientTransform(const NSVGpaint& paint, hvif::Gradient& grad, const ParseState& state)
{
	NSVGgradient* g = paint.gradient;
	if (!g) return;

	float M[6];
	utils::InvertAffine(M, g->xform);

	if (paint.type == NSVG_PAINT_LINEAR_GRADIENT) {
		float x1 = M[4];
		float y1 = M[5];
		float dx = M[2];
		float dy = M[3];

		float x1_h = x1 * state.scale + state.tx;
		float y1_h = y1 * state.scale + state.ty;
		float x2_h = (x1 + dx) * state.scale + state.tx;
		float y2_h = (y1 + dy) * state.scale + state.ty;

		float vx = x2_h - x1_h;
		float vy = y2_h - y1_h;
		float len = sqrt(vx * vx + vy * vy);
		if (len < 1e-6f) return;

		const float hvif_native_length = 128.0f;
		float s = len / hvif_native_length;
		float angle = atan2(vy, vx);
		float c = cos(angle);
		float sng = sin(angle);

		float cx = 0.5f * (x1_h + x2_h);
		float cy = 0.5f * (y1_h + y2_h);

		grad.matrix.push_back(c * s);
		grad.matrix.push_back(sng * s);
		grad.matrix.push_back(-sng * s);
		grad.matrix.push_back(c * s);
		grad.matrix.push_back(cx);
		grad.matrix.push_back(cy);
		grad.hasMatrix = true;
	} else if (paint.type == NSVG_PAINT_RADIAL_GRADIENT) {
		float a = M[0], b = M[1], c = M[2], d = M[3], tx = M[4], ty = M[5];
		float s = state.scale;
		
		grad.matrix.push_back((a * s) / 64.0f);
		grad.matrix.push_back((b * s) / 64.0f);
		grad.matrix.push_back((c * s) / 64.0f);
		grad.matrix.push_back((d * s) / 64.0f);
		grad.matrix.push_back(tx * s + state.tx);
		grad.matrix.push_back(ty * s + state.ty);
		grad.hasMatrix = true;
	}
}

}
