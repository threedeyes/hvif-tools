/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef IOM_STRUCTURES_H
#define IOM_STRUCTURES_H

#include <vector>
#include <string>

#include <stdint.h>

namespace iom {

enum GradientType {
	GRADIENT_LINEAR = 0,
	GRADIENT_CIRCULAR = 1,
	GRADIENT_DIAMOND = 2,
	GRADIENT_CONIC = 3,
	GRADIENT_XY = 4,
	GRADIENT_SQRT_XY = 5
};

enum InterpolationType {
	INTERPOLATION_LINEAR = 0,
	INTERPOLATION_SMOOTH = 1
};

enum TransformerType {
	TRANSFORMER_AFFINE = 0,
	TRANSFORMER_CONTOUR = 1,
	TRANSFORMER_PERSPECTIVE = 2,
	TRANSFORMER_STROKE = 3
};

struct ColorStop {
	uint32_t color;
	float offset;
	
	ColorStop() : color(0), offset(0.0f) {}

	bool operator==(const ColorStop& other) const {
		return color == other.color && offset == other.offset;
	}
};

struct Gradient {
	GradientType type;
	InterpolationType interpolation;
	bool inheritTransformation;
	std::vector<ColorStop> stops;
	std::vector<double> transform;
	bool hasTransform;
	
	Gradient() : type(GRADIENT_LINEAR), interpolation(INTERPOLATION_LINEAR),
		inheritTransformation(true), hasTransform(false) {}

	bool operator==(const Gradient& other) const {
		return type == other.type &&
			   interpolation == other.interpolation &&
			   inheritTransformation == other.inheritTransformation &&
			   stops == other.stops &&
			   transform == other.transform &&
			   hasTransform == other.hasTransform;
	}
};

struct Style {
	bool isGradient;
	uint32_t color;
	Gradient gradient;
	std::string name;
	
	Style() : isGradient(false), color(0xFF000000) {}

	bool operator==(const Style& other) const {
		if (isGradient != other.isGradient)
			return false;
		if (name != other.name)
			return false;
		if (isGradient)
			return gradient == other.gradient;
		return color == other.color;
	}
};

struct ControlPoint {
	float x, y;
	float x_in, y_in;
	float x_out, y_out;
	bool connected;
	
	ControlPoint() : x(0), y(0), x_in(0), y_in(0), x_out(0), y_out(0), connected(true) {}

	bool operator==(const ControlPoint& other) const {
		return x == other.x && y == other.y &&
			   x_in == other.x_in && y_in == other.y_in &&
			   x_out == other.x_out && y_out == other.y_out &&
			   connected == other.connected;
	}
};

struct Path {
	std::vector<ControlPoint> points;
	bool closed;
	std::string name;
	
	Path() : closed(false) {}

	bool operator==(const Path& other) const {
		return points == other.points &&
			   closed == other.closed &&
			   name == other.name;
	}
};

struct Transformer {
	TransformerType type;
	std::vector<double> matrix;
	double width;
	int32_t lineJoin;
	int32_t lineCap;
	double miterLimit;
	
	Transformer() : type(TRANSFORMER_AFFINE), width(1.0), 
		lineJoin(0), lineCap(0), miterLimit(4.0) {}

	bool operator==(const Transformer& other) const {
		return type == other.type &&
			   matrix == other.matrix &&
			   width == other.width &&
			   lineJoin == other.lineJoin &&
			   lineCap == other.lineCap &&
			   miterLimit == other.miterLimit;
	}
};

struct Shape {
	uint32_t what;
	std::vector<int> pathIndices;
	int styleIndex;
	std::vector<double> transform;
	bool hasTransform;
	bool hinting;
	float minVisibility;
	float maxVisibility;
	std::vector<Transformer> transformers;
	std::string name;

	Shape() : what(0), styleIndex(0), hasTransform(false), 
		hinting(false), minVisibility(0.0f), maxVisibility(4.0f) {}

	bool operator==(const Shape& other) const {
		return what == other.what &&
			   pathIndices == other.pathIndices &&
			   styleIndex == other.styleIndex &&
			   transform == other.transform &&
			   hasTransform == other.hasTransform &&
			   hinting == other.hinting &&
			   minVisibility == other.minVisibility &&
			   maxVisibility == other.maxVisibility &&
			   transformers == other.transformers &&
			   name == other.name;
	}
};

struct Icon {
	std::string filename;
	std::vector<Style> styles;
	std::vector<Path> paths;
	std::vector<Shape> shapes;
};

}

#endif
