/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef HAIKU_ICON_H
#define HAIKU_ICON_H

#include <vector>
#include <string>
#include <stdint.h>

namespace haiku {

struct Color {
	uint32_t argb;

	Color() : argb(0xFF000000) {}
	explicit Color(uint32_t c) : argb(c) {}

	uint8_t Alpha() const { return (argb >> 24) & 0xFF; }
	uint8_t Red() const { return (argb >> 16) & 0xFF; }
	uint8_t Green() const { return (argb >> 8) & 0xFF; }
	uint8_t Blue() const { return argb & 0xFF; }
	
	void SetAlpha(uint8_t a) { argb = (argb & 0x00FFFFFF) | (static_cast<uint32_t>(a) << 24); }
	void SetRed(uint8_t r) { argb = (argb & 0xFF00FFFF) | (static_cast<uint32_t>(r) << 16); }
	void SetGreen(uint8_t g) { argb = (argb & 0xFFFF00FF) | (static_cast<uint32_t>(g) << 8); }
	void SetBlue(uint8_t b) { argb = (argb & 0xFFFFFF00) | static_cast<uint32_t>(b); }
};

enum GradientType {
	GRADIENT_LINEAR = 0,
	GRADIENT_RADIAL = 1,
	GRADIENT_DIAMOND = 2,
	GRADIENT_CONIC = 3,
	GRADIENT_XY = 4,
	GRADIENT_SQRT_XY = 5
};

enum InterpolationType {
	INTERPOLATION_LINEAR = 0,
	INTERPOLATION_SMOOTH = 1
};

struct ColorStop {
	Color color;
	float offset;

	ColorStop() : offset(0.0f) {}
	ColorStop(const Color& c, float o) : color(c), offset(o) {}
};

struct Gradient {
	GradientType type;
	InterpolationType interpolation;
	std::vector<double> transform;
	std::vector<ColorStop> stops;
	bool hasTransform;

	Gradient() : type(GRADIENT_LINEAR), interpolation(INTERPOLATION_LINEAR), hasTransform(false) {}
};

struct Style {
	std::string name;
	bool isGradient;
	Color solidColor;
	Gradient gradient;

	Style() : isGradient(false) {}
};

struct PathPoint {
	double x, y;
	double x_in, y_in;
	double x_out, y_out;
	bool connected;

	PathPoint() : x(0), y(0), x_in(0), y_in(0), x_out(0), y_out(0), connected(true) {}
};

struct Path {
	std::string name;
	std::vector<PathPoint> points;
	bool closed;

	Path() : closed(false) {}
};

enum TransformerType {
	TRANSFORMER_AFFINE = 0,
	TRANSFORMER_CONTOUR = 1,
	TRANSFORMER_PERSPECTIVE = 2,
	TRANSFORMER_STROKE = 3
};

struct Transformer {
	TransformerType type;
	std::vector<double> matrix;
	double width;
	int lineJoin;
	int lineCap;
	double miterLimit;

	Transformer() : type(TRANSFORMER_AFFINE), width(1.0), lineJoin(0), lineCap(0), miterLimit(4.0) {}
};

struct Shape {
	std::string name;
	int styleIndex;
	std::vector<int> pathIndices;
	std::vector<Transformer> transformers;
	std::vector<double> transform;
	bool hasTransform;
	float minLOD;
	float maxLOD;

	Shape() : styleIndex(0), hasTransform(false), minLOD(0.0f), maxLOD(255.0f) {}
};

struct Icon {
	std::string filename;
	std::vector<Style> styles;
	std::vector<Path> paths;
	std::vector<Shape> shapes;
};

}

#endif
