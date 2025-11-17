/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <fstream>

#include "IOMWriter.h"

namespace iom {

static const uint32_t TRANSFORMER_STROKE_FLAGS		= 'strk';
static const uint32_t TRANSFORMER_AFFINE_FLAGS		= 'affn';
static const uint32_t TRANSFORMER_CONTOUR_FLAGS		= 'cntr';
static const uint32_t TRANSFORMER_PERSPECTIVE_FLAGS	= 'prsp';
static const uint32_t TRANSFORMER_SHAPE_FLAGS		= 'shps';

bool
IOMWriter::WriteToFile(const std::string& filename, const Icon& icon)
{
	std::vector<uint8_t> buffer;
	if (!WriteToBuffer(buffer, icon))
		return false;
	
	std::ofstream file(filename.c_str(), std::ios::binary);
	if (!file.is_open())
		return false;
	
	file.write((const char*)&buffer[0], buffer.size());
	file.close();
	
	return file.good();
}

bool
IOMWriter::WriteToBuffer(std::vector<uint8_t>& buffer, const Icon& icon)
{
	haiku_compat::BMessage msg(1);
	_BuildMessage(msg, icon);

	ssize_t size = msg.FlattenedSize();
	if (size <= 0)
		return false;

	std::vector<char> flatBuffer;
	flatBuffer.resize(size);

	if (msg.Flatten(&flatBuffer[0], size) != haiku_compat::B_OK)
		return false;

	buffer.clear();
	buffer.push_back('I');
	buffer.push_back('M');
	buffer.push_back('S');
	buffer.push_back('G');

	for (size_t i = 0; i < flatBuffer.size(); ++i)
		buffer.push_back((uint8_t)flatBuffer[i]);

	return true;
}

void
IOMWriter::_BuildMessage(haiku_compat::BMessage& msg, const Icon& icon)
{
	_AddPathsToMessage(msg, icon);
	_AddStylesToMessage(msg, icon);
	_AddShapesToMessage(msg, icon);
}

void
IOMWriter::_AddStylesToMessage(haiku_compat::BMessage& msg, const Icon& icon)
{
	haiku_compat::BMessage stylesContainer(1);
	stylesContainer.MakeEmpty();

	for (size_t i = 0; i < icon.styles.size(); ++i) {
		_AddStyleToMessage(stylesContainer, icon.styles[i], (int)i);
	}
	msg.AddMessage("styles", &stylesContainer);
}

void
IOMWriter::_AddPathsToMessage(haiku_compat::BMessage& msg, const Icon& icon)
{
	haiku_compat::BMessage pathsContainer(1);
	pathsContainer.MakeEmpty();

	for (size_t i = 0; i < icon.paths.size(); ++i) {
		_AddPathToMessage(pathsContainer, icon.paths[i], (int)i);
	}
	msg.AddMessage("paths", &pathsContainer);
}

void
IOMWriter::_AddShapesToMessage(haiku_compat::BMessage& msg, const Icon& icon)
{
	haiku_compat::BMessage shapesContainer(1);
	shapesContainer.MakeEmpty();

	for (size_t i = 0; i < icon.shapes.size(); ++i) {
		_AddShapeToMessage(shapesContainer, icon.shapes[i], (int)i);
	}
	msg.AddMessage("shapes", &shapesContainer);
}

void
IOMWriter::_AddStyleToMessage(haiku_compat::BMessage& container, const Style& style, int index)
{
	haiku_compat::BMessage styleMsg(1);

	if (!style.name.empty())
		styleMsg.AddString("name", style.name);
	else
		styleMsg.AddString("name", "<style>");

	if (style.isGradient) {
		if (style.gradient.stops.size() > 0) {
			styleMsg.AddInt32("color", (int32_t)style.gradient.stops[0].color);
		} else {
			styleMsg.AddInt32("color", (int32_t)0xFF000000);
		}

		haiku_compat::BMessage gradMsg(1);
		_AddGradientToMessage(gradMsg, style.gradient);
		styleMsg.AddMessage("gradient", &gradMsg);
	} else {
		styleMsg.AddInt32("color", (int32_t)style.color);
	}

	container.AddMessage("style", &styleMsg);
}

void
IOMWriter::_AddPathToMessage(haiku_compat::BMessage& container, const Path& path, int index)
{
	haiku_compat::BMessage pathMsg(1);

	if (!path.name.empty())
		pathMsg.AddString("name", path.name);
	else
		pathMsg.AddString("name", "<path>");

	size_t pointCount = path.points.size();

	for (size_t i = 0; i < pointCount; ++i) {
		const ControlPoint& cp = path.points[i];
		pathMsg.AddPoint("point", haiku_compat::BPoint(cp.x, cp.y));
	}

	for (size_t i = 0; i < pointCount; ++i) {
		const ControlPoint& cp = path.points[i];
		pathMsg.AddPoint("point in", haiku_compat::BPoint(cp.x_in, cp.y_in));
	}

	for (size_t i = 0; i < pointCount; ++i) {
		const ControlPoint& cp = path.points[i];
		pathMsg.AddPoint("point out", haiku_compat::BPoint(cp.x_out, cp.y_out));
	}

	for (size_t i = 0; i < pointCount; ++i) {
		pathMsg.AddBool("connected", false);
	}

	pathMsg.AddBool("path closed", path.closed);

	container.AddMessage("path", &pathMsg);
}

void
IOMWriter::_AddShapeToMessage(haiku_compat::BMessage& container, const Shape& shape, int index)
{
	haiku_compat::BMessage shapeMsg(1);

	shapeMsg.AddInt32("type", TRANSFORMER_SHAPE_FLAGS);

	shapeMsg.AddInt32("style ref", shape.styleIndex);

	for (size_t i = 0; i < shape.pathIndices.size(); ++i) {
		shapeMsg.AddInt32("path ref", shape.pathIndices[i]);
	}

	if (!shape.name.empty())
		shapeMsg.AddString("name", shape.name);
	else
		shapeMsg.AddString("name", "");

	shapeMsg.AddBool("hinting", shape.hinting);

	for (size_t i = 0; i < shape.transformers.size(); ++i) {
		haiku_compat::BMessage transMsg(1);
		_AddTransformerToMessage(transMsg, shape.transformers[i]);
		shapeMsg.AddMessage("transformer", &transMsg);
	}

	if (shape.hasTransform && shape.transform.size() >= 6) {
		shapeMsg.AddData("transformation", haiku_compat::B_DOUBLE_TYPE,
			&shape.transform[0], 6 * sizeof(double), true);
	} else {
		double identity[6] = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
		shapeMsg.AddData("transformation", haiku_compat::B_DOUBLE_TYPE,
			identity, 6 * sizeof(double), true);
	}

	shapeMsg.AddFloat("min visibility scale", shape.minVisibility);
	shapeMsg.AddFloat("max visibility scale", shape.maxVisibility);

	container.AddMessage("shape", &shapeMsg);
}

void
IOMWriter::_AddGradientToMessage(haiku_compat::BMessage& msg, const Gradient& grad)
{
	msg.AddString("class", "Gradient");
	msg.AddString("class", "Gradient"); // TODO: it's present in original Icon-O-Matic files, but why???

	if (grad.hasTransform && grad.transform.size() >= 6) {
		msg.AddData("transformation", haiku_compat::B_DOUBLE_TYPE, 
			&grad.transform[0], 6 * sizeof(double), true);
	} else {
		double identity[6] = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
		msg.AddData("transformation", haiku_compat::B_DOUBLE_TYPE,
			identity, 6 * sizeof(double), true);
	}

	for (size_t i = 0; i < grad.stops.size(); ++i) {
		msg.AddInt32("color", (int32_t)grad.stops[i].color);
	}

	for (size_t i = 0; i < grad.stops.size(); ++i) {
		msg.AddFloat("offset", grad.stops[i].offset);
	}

	msg.AddInt32("type", (int32_t)grad.type);
	msg.AddInt32("interpolation", (int32_t)grad.interpolation);	
	msg.AddBool("inherit transformation", grad.inheritTransformation);
}

void
IOMWriter::_AddTransformerToMessage(haiku_compat::BMessage& msg, const Transformer& trans)
{
	haiku_compat::BMessage::Private msgPrivate(msg);

	if (msgPrivate.InitHeader() != haiku_compat::B_OK)
		return;

	switch (trans.type) {
		case TRANSFORMER_STROKE:
			msgPrivate.GetMessageHeader()->flags = TRANSFORMER_STROKE_FLAGS;
			msg.AddString("name", "Stroke");
			msg.AddInt32("line cap", trans.lineCap);
			msg.AddInt32("line join", trans.lineJoin);
			msg.AddInt32("inner join", 1);
			msg.AddDouble("width", trans.width);
			msg.AddDouble("miter limit", trans.miterLimit);
			msg.AddDouble("inner miter limit", 1.01);
			msg.AddDouble("shorten", 0.0);
			break;

		case TRANSFORMER_AFFINE:
			msgPrivate.GetMessageHeader()->flags = TRANSFORMER_AFFINE_FLAGS;
			msg.AddString("name", "Affine");
			if (trans.matrix.size() >= 6) {
				msg.AddData("transformation", haiku_compat::B_DOUBLE_TYPE,
					&trans.matrix[0], 6 * sizeof(double), true);
			}
			break;

		case TRANSFORMER_CONTOUR:
			msgPrivate.GetMessageHeader()->flags = TRANSFORMER_CONTOUR_FLAGS;
			msg.AddString("name", "Contour");
			msg.AddDouble("width", trans.width);
			break;

		case TRANSFORMER_PERSPECTIVE:
			msgPrivate.GetMessageHeader()->flags = TRANSFORMER_PERSPECTIVE_FLAGS;
			msg.AddString("name", "Perspective");
			break;
	}
}

}
