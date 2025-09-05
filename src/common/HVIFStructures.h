/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef HVIF_STRUCTURES_H
#define HVIF_STRUCTURES_H

#include <vector>
#include <string>
#include <algorithm>

#include <stdint.h>

namespace hvif {

enum ColorTags {
	RGBA = 1,
	RGB = 3, 
	KA = 4,
	K = 5,
	GRADIENT = 2
};

enum GradientTypes {
	LINEAR = 0,
	RADIAL = 1,
	DIAMOND = 2,
	CONIC = 3,
	XY = 4,
	SQRTXY = 5
};

enum GradientFlags {
	TRANSFORM = 1 << 1,
	NO_ALPHA = 1 << 2,
	HEX_COLORS = 1 << 3,
	GREYS = 1 << 4
};

enum PathFlags {
	CLOSED = 1 << 1,
	COMMANDS = 1 << 2,
	POINTS = 1 << 3
};

enum PointTags {
	VLINE = 0,
	HLINE = 1,
	LINE = 2,
	CURVE = 3
};

enum ShapeFlags {
	MATRIX = 1 << 1,
	HINTING = 1 << 2,
	LOD_SCALE = 1 << 3,
	TRANSFORMERS = 1 << 4,
	TRANSLATE = 1 << 5
};

enum TransformerTags {
	AFFINE = 20,
	CONTOUR = 21,
	PERSPECTIVE = 22,
	STROKE = 23
};

enum LineJoins {
	MITER = 0,
	MITER_REVERT = 1,
	ROUND = 2,
	BEVEL = 3,
	MITER_ROUND = 4
};

enum LineCaps {
	BUTT = 0,
	SQUARE = 1,
	ROUND_CAP = 2
};

struct Color {
	ColorTags tag;
	std::vector<uint8_t> data;
	
	Color() : tag(RGBA) {}
	Color(ColorTags t) : tag(t) {}
	
	bool operator==(const Color& other) const {
		return tag == other.tag && data == other.data;
	}
};

struct GradientStop {
	uint8_t offset;
	Color color;
	
	GradientStop() : offset(0) {}
	
	bool operator==(const GradientStop& other) const {
		return offset == other.offset && color == other.color;
	}
};

struct Gradient {
	GradientTypes type;
	uint8_t flags;
	std::vector<float> matrix;
	std::vector<GradientStop> stops;
	bool hasMatrix;

	Gradient() : type(LINEAR), flags(0), hasMatrix(false) {}

	bool operator==(const Gradient& other) const {
		return type == other.type &&
			   flags == other.flags &&
			   matrix == other.matrix &&
			   hasMatrix == other.hasMatrix &&
			   stops == other.stops;
	}
};

struct Style {
	bool isGradient;
	Color color;
	Gradient gradient;

	Style() : isGradient(false) {}

	bool operator==(const Style& other) const {
		if (isGradient != other.isGradient) return false;
		if (isGradient) return gradient == other.gradient;
		return color == other.color;
	}
};

struct Path {
	std::string type;
	std::vector<float> points;
	bool closed;

	Path() : closed(false) {}

	bool operator==(const Path& other) const {
		return type == other.type &&
			   points == other.points &&
			   closed == other.closed;
	}
};

struct Transformer {
	TransformerTags tag;
	std::vector<float> data;
	
	float width;
	uint8_t lineJoin;
	uint8_t lineCap;
	uint8_t miterLimit;

	Transformer() : tag(AFFINE), width(0), lineJoin(0), lineCap(0), miterLimit(0) {}

	bool operator==(const Transformer& other) const {
		return tag == other.tag &&
			   data == other.data &&
			   width == other.width &&
			   lineJoin == other.lineJoin &&
			   lineCap == other.lineCap &&
			   miterLimit == other.miterLimit;
	}
};

struct Shape {
	uint8_t styleIndex;
	std::vector<uint8_t> pathIndices;
	std::vector<float> transform;
	std::string transformType;
	std::vector<Transformer> transformers;
	bool hasTransform;

	Shape() : styleIndex(0), hasTransform(false) {}

	bool operator==(const Shape& other) const {
		return styleIndex == other.styleIndex &&
			   pathIndices == other.pathIndices &&
			   transform == other.transform &&
			   transformType == other.transformType &&
			   transformers == other.transformers &&
			   hasTransform == other.hasTransform;
	}
};

struct HVIFIcon {
	std::string filename;
	std::vector<Style> styles;
	std::vector<Path> paths;
	std::vector<Shape> shapes;
};

}

#endif
