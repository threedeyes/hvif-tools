/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <fstream>
#include <sstream>
#include <iostream>

#include "IOMParser.h"

namespace iom {

template<typename T>
std::string ToString(T value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

IOMParser::IOMParser()
	: fIcon(NULL)
{
}

IOMParser::~IOMParser()
{
	delete fIcon;
}

bool
IOMParser::ParseFile(const std::string& filename)
{
	std::ifstream file(filename.c_str(), std::ios::binary);
	if (!file.is_open()) {
		_SetError("Cannot open file: " + filename);
		return false;
	}

	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	file.seekg(0, std::ios::beg);

	if (size < 4) {
		_SetError("File too small");
		return false;
	}

	char* buffer = new char[size];
	file.read(buffer, size);
	file.close();

	if (buffer[0] != 'I' || buffer[1] != 'M' || buffer[2] != 'S' || buffer[3] != 'G') {
		delete[] buffer;
		_SetError("Not a valid IOM file (missing IMSG signature)");
		return false;
	}

	BMessage message;
	status_t result = message.Unflatten(buffer + 4, size - 4);
	delete[] buffer;

	if (result != B_OK) {
		_SetError("Failed to unflatten BMessage");
		return false;
	}

	delete fIcon;
	fIcon = new Icon();
	fIcon->filename = filename;

	return _ParseMessage(message);
}

bool
IOMParser::ParseMessage(const BMessage& message)
{
	delete fIcon;
	fIcon = new Icon();
	fIcon->filename = "<from memory>";

	fLastError.clear();

	return _ParseMessage(message);
}

bool
IOMParser::_ParseMessage(const BMessage& message)
{
	BMessage stylesContainer, pathsContainer, shapesContainer;
	
	if (message.FindMessage("styles", &stylesContainer) == B_OK) {
		int32_t styleCount = 0;
		stylesContainer.GetInfo("style", NULL, &styleCount);

		for (int32_t i = 0; i < styleCount; i++) {
			BMessage styleMsg;
			if (stylesContainer.FindMessage("style", i, &styleMsg) == B_OK) {
				Style style;
				if (_ParseStyle(styleMsg, style))
					fIcon->styles.push_back(style);
			}
		}
	}

	if (message.FindMessage("paths", &pathsContainer) == B_OK) {
		int32_t pathCount = 0;
		pathsContainer.GetInfo("path", NULL, &pathCount);

		for (int32_t i = 0; i < pathCount; i++) {
			BMessage pathMsg;
			if (pathsContainer.FindMessage("path", i, &pathMsg) == B_OK) {
				Path path;
				if (_ParsePath(pathMsg, path))
					fIcon->paths.push_back(path);
			}
		}
	}

	if (message.FindMessage("shapes", &shapesContainer) == B_OK) {
		int32_t shapeCount = 0;
		shapesContainer.GetInfo("shape", NULL, &shapeCount);

		for (int32_t i = 0; i < shapeCount; i++) {
			BMessage shapeMsg;
			if (shapesContainer.FindMessage("shape", i, &shapeMsg) == B_OK) {
				Shape shape;
				if (_ParseShape(shapeMsg, shape))
					fIcon->shapes.push_back(shape);
			}
		}
	}

	return true;
}

bool
IOMParser::_ParseStyle(const BMessage& styleMsg, Style& style)
{
	const char* name = styleMsg.GetString("name", NULL);
	if (name)
		style.name = name;

	int32_t color;
	if (styleMsg.FindInt32("color", &color) == B_OK) {
		style.isGradient = false;
		style.color = static_cast<uint32_t>(color);
	}

	BMessage gradMsg;
	if (styleMsg.FindMessage("gradient", &gradMsg) == B_OK) {
		style.isGradient = true;
		if (!_ParseGradient(gradMsg, style.gradient))
			return false;
	}

	return true;
}

bool
IOMParser::_ParseGradient(const BMessage& gradMsg, Gradient& gradient)
{
	int32_t type, interp;
	bool inherit;

	if (gradMsg.FindInt32("type", &type) == B_OK)
		gradient.type = static_cast<GradientType>(type);

	if (gradMsg.FindInt32("interpolation", &interp) == B_OK)
		gradient.interpolation = static_cast<InterpolationType>(interp);

	if (gradMsg.FindBool("inherit transformation", &inherit) == B_OK)
		gradient.inheritTransformation = inherit;

	const void* matrix;
	ssize_t matrixSize;
	if (gradMsg.FindData("transformation", B_DOUBLE_TYPE, &matrix, &matrixSize) == B_OK) {
		int count = matrixSize / sizeof(double);
		if (count == 6) {
			const double* m = static_cast<const double*>(matrix);
			gradient.transform.assign(m, m + 6);
			gradient.hasTransform = true;
		}
	}

	int32_t colorCount = 0;
	gradMsg.GetInfo("color", NULL, &colorCount);

	for (int32_t i = 0; i < colorCount; i++) {
		int32_t color;
		float offset;

		if (gradMsg.FindInt32("color", i, &color) == B_OK &&
			gradMsg.FindFloat("offset", i, &offset) == B_OK) {
			ColorStop stop;
			stop.color = static_cast<uint32_t>(color);
			stop.offset = offset;
			gradient.stops.push_back(stop);
		}
	}

	return true;
}

bool
IOMParser::_ParsePath(const BMessage& pathMsg, Path& path)
{
	const char* name = pathMsg.GetString("name", NULL);
	if (name)
		path.name = name;

	bool closed;
	if (pathMsg.FindBool("path closed", &closed) == B_OK)
		path.closed = closed;

	int32_t pointCount = 0;
	pathMsg.GetInfo("point", NULL, &pointCount);

	for (int32_t i = 0; i < pointCount; i++) {
		BPoint point, pointIn, pointOut;
		bool connected;

		if (pathMsg.FindPoint("point", i, &point) == B_OK &&
			pathMsg.FindPoint("point in", i, &pointIn) == B_OK &&
			pathMsg.FindPoint("point out", i, &pointOut) == B_OK &&
			pathMsg.FindBool("connected", i, &connected) == B_OK) {

			ControlPoint cp;
			cp.x = point.x;
			cp.y = point.y;
			cp.x_in = pointIn.x;
			cp.y_in = pointIn.y;
			cp.x_out = pointOut.x;
			cp.y_out = pointOut.y;
			cp.connected = connected;
			path.points.push_back(cp);
		}
	}

	return true;
}

bool
IOMParser::_ParseShape(const BMessage& shapeMsg, Shape& shape)
{
	shape.what = shapeMsg.what;

	const char* name = shapeMsg.GetString("name", NULL);
	if (name)
		shape.name = name;

	int32_t styleRef;
	if (shapeMsg.FindInt32("style ref", &styleRef) == B_OK) {
		shape.styleIndex = styleRef;
	}

	int32_t pathCount = 0;
	shapeMsg.GetInfo("path ref", NULL, &pathCount);
	for (int32_t i = 0; i < pathCount; i++) {
		int32_t pathRef;
		if (shapeMsg.FindInt32("path ref", i, &pathRef) == B_OK) {
			shape.pathIndices.push_back(pathRef);
		}
	}

	const void* matrix;
	ssize_t matrixSize;
	if (shapeMsg.FindData("transformation", B_DOUBLE_TYPE, &matrix, &matrixSize) == B_OK) {
		int count = matrixSize / sizeof(double);
		if (count == 6) {
			const double* m = static_cast<const double*>(matrix);
			shape.transform.assign(m, m + 6);
			shape.hasTransform = true;
		}
	}

	bool hinting;
	if (shapeMsg.FindBool("hinting", &hinting) == B_OK)
		shape.hinting = hinting;

	float minVis, maxVis;
	if (shapeMsg.FindFloat("min visibility scale", &minVis) == B_OK)
		shape.minVisibility = minVis;

	if (shapeMsg.FindFloat("max visibility scale", &maxVis) == B_OK)
		shape.maxVisibility = maxVis;

	int32_t transCount = 0;
	shapeMsg.GetInfo("transformer", NULL, &transCount);
	for (int32_t i = 0; i < transCount; i++) {
		BMessage transMsg;
		if (shapeMsg.FindMessage("transformer", i, &transMsg) == B_OK) {
			Transformer trans;
			if (_ParseTransformer(transMsg, trans))
				shape.transformers.push_back(trans);
		}
	}

	return true;
}

bool
IOMParser::_ParseTransformer(const BMessage& transMsg, Transformer& transformer)
{
	std::string name = transMsg.GetString("name", "");

	if (name == "Stroke") {
		transformer.type = TRANSFORMER_STROKE;

		double width;
		int32_t lineCap, lineJoin;
		double miterLimit;

		if (transMsg.FindDouble("width", &width) == B_OK)
			transformer.width = width;
		if (transMsg.FindInt32("line cap", &lineCap) == B_OK)
			transformer.lineCap = lineCap;
		if (transMsg.FindInt32("line join", &lineJoin) == B_OK)
			transformer.lineJoin = lineJoin;
		if (transMsg.FindDouble("miter limit", &miterLimit) == B_OK)
			transformer.miterLimit = miterLimit;
	} else if (name == "Contour") {
		transformer.type = TRANSFORMER_CONTOUR;

		double width;
		if (transMsg.FindDouble("width", &width) == B_OK)
			transformer.width = width;
	}

	return true;
}

void
IOMParser::_SetError(const std::string& error)
{
	fLastError = error;
}

}
