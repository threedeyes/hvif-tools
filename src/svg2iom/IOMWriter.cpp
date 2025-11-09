/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <fstream>

#include "IOMWriter.h"

namespace iom {

static const uint32_t TRANSFORMER_STROKE_FLAGS      = 0x7374726b;
static const uint32_t TRANSFORMER_AFFINE_FLAGS      = 0x6166666e;
static const uint32_t TRANSFORMER_CONTOUR_FLAGS     = 0x636e7472;
static const uint32_t TRANSFORMER_PERSPECTIVE_FLAGS = 0x70727370;

IOMWriter::IOMWriter()
{
}

IOMWriter::~IOMWriter()
{
}

bool
IOMWriter::WriteToFile(const std::string& filename, const Icon& icon)
{
	std::vector<uint8_t> buffer;
	if (!WriteToBuffer(buffer, icon))
		return false;
	
	std::ofstream file(filename.c_str(), std::ios::binary);
	if (!file.is_open())
		return false;
	
	file.write(reinterpret_cast<const char*>(&buffer[0]), buffer.size());
	file.close();
	
	return file.good();
}

bool
IOMWriter::WriteToBuffer(std::vector<uint8_t>& buffer, const Icon& icon)
{
	BMessage msg(1);
	_BuildMessage(msg, icon);
	
	ssize_t size = msg.FlattenedSize();
	if (size <= 0)
		return false;
	
	std::vector<char> flatBuffer;
	flatBuffer.resize(size);
	
	if (msg.Flatten(&flatBuffer[0], size) != B_OK)
		return false;
	
	buffer.clear();
	buffer.push_back('I');
	buffer.push_back('M');
	buffer.push_back('S');
	buffer.push_back('G');
	
	for (size_t i = 0; i < flatBuffer.size(); ++i)
		buffer.push_back(static_cast<uint8_t>(flatBuffer[i]));
	
	return true;
}

void
IOMWriter::_BuildMessage(BMessage& msg, const Icon& icon)
{
	_AddPathsToMessage(msg, icon);
	_AddStylesToMessage(msg, icon);
	_AddShapesToMessage(msg, icon);
}

void
IOMWriter::_AddStylesToMessage(BMessage& msg, const Icon& icon)
{
	BMessage stylesContainer(1);
	stylesContainer.MakeEmpty();
	
	for (size_t i = 0; i < icon.styles.size(); ++i) {
		_AddStyleToMessage(stylesContainer, icon.styles[i], i);
	}
	msg.AddMessage("styles", &stylesContainer);
}

void
IOMWriter::_AddPathsToMessage(BMessage& msg, const Icon& icon)
{
	BMessage pathsContainer(1);
	pathsContainer.MakeEmpty();

	for (size_t i = 0; i < icon.paths.size(); ++i) {
		_AddPathToMessage(pathsContainer, icon.paths[i], i);
	}
	msg.AddMessage("paths", &pathsContainer);
}

void
IOMWriter::_AddShapesToMessage(BMessage& msg, const Icon& icon)
{
	BMessage shapesContainer(1);
	shapesContainer.MakeEmpty();
	
	for (size_t i = 0; i < icon.shapes.size(); ++i) {
		_AddShapeToMessage(shapesContainer, icon.shapes[i], i);
	}
	msg.AddMessage("shapes", &shapesContainer);
}

void
IOMWriter::_AddStyleToMessage(BMessage& container, const Style& style, int index)
{
	BMessage styleMsg(1);

	if (!style.name.empty())
		styleMsg.AddString("name", style.name);
	else
		styleMsg.AddString("name", "<style>");
	
	if (style.isGradient) {
		if (style.gradient.stops.size() > 0) {
			styleMsg.AddInt32("color", static_cast<int32_t>(style.gradient.stops[0].color));
		} else {
			styleMsg.AddInt32("color", static_cast<int32_t>(0xFF000000));
		}
		
		BMessage gradMsg(1);
		_AddGradientToMessage(gradMsg, style.gradient);
		styleMsg.AddMessage("gradient", &gradMsg);
	} else {
		styleMsg.AddInt32("color", static_cast<int32_t>(style.color));
	}
	
	container.AddMessage("style", &styleMsg);
}

void
IOMWriter::_AddPathToMessage(BMessage& container, const Path& path, int index)
{
	BMessage pathMsg(1);
	
	if (!path.name.empty())
		pathMsg.AddString("name", path.name);
	else
		pathMsg.AddString("name", "<path>");
	
	size_t pointCount = path.points.size();
	
	for (size_t i = 0; i < pointCount; ++i) {
		const ControlPoint& cp = path.points[i];
		pathMsg.AddPoint("point", BPoint(cp.x, cp.y));
	}
	
	for (size_t i = 0; i < pointCount; ++i) {
		const ControlPoint& cp = path.points[i];
		pathMsg.AddPoint("point in", BPoint(cp.x_in, cp.y_in));
	}
	
	for (size_t i = 0; i < pointCount; ++i) {
		const ControlPoint& cp = path.points[i];
		pathMsg.AddPoint("point out", BPoint(cp.x_out, cp.y_out));
	}
	
	for (size_t i = 0; i < pointCount; ++i) {
		pathMsg.AddBool("connected", false);
	}
	
	pathMsg.AddBool("path closed", path.closed);
	
	container.AddMessage("path", &pathMsg);
}

void
IOMWriter::_AddShapeToMessage(BMessage& container, const Shape& shape, int index)
{
	BMessage shapeMsg(1);
	
	shapeMsg.AddInt32("type", 0x73687073);
	
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
		BMessage transMsg(1);
		_AddTransformerToMessage(transMsg, shape.transformers[i]);
		shapeMsg.AddMessage("transformer", &transMsg);
	}
	
	if (shape.hasTransform && shape.transform.size() >= 6) {
		shapeMsg.AddData("transformation", B_DOUBLE_TYPE,
			&shape.transform[0], 6 * sizeof(double), true);
	} else {
		double identity[6] = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
		shapeMsg.AddData("transformation", B_DOUBLE_TYPE,
			identity, 6 * sizeof(double), true);
	}
	
	shapeMsg.AddFloat("min visibility scale", shape.minVisibility);
	shapeMsg.AddFloat("max visibility scale", shape.maxVisibility);
	
	container.AddMessage("shape", &shapeMsg);
}

void
IOMWriter::_AddGradientToMessage(BMessage& msg, const Gradient& grad)
{
	msg.AddString("class", "Gradient");
	msg.AddString("class", "Gradient");
	
	if (grad.hasTransform && grad.transform.size() >= 6) {
		msg.AddData("transformation", B_DOUBLE_TYPE, 
			&grad.transform[0], 6 * sizeof(double), true);
	} else {
		double identity[6] = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
		msg.AddData("transformation", B_DOUBLE_TYPE,
			identity, 6 * sizeof(double), true);
	}

	for (size_t i = 0; i < grad.stops.size(); ++i) {
		msg.AddInt32("color", static_cast<int32_t>(grad.stops[i].color));
	}

	for (size_t i = 0; i < grad.stops.size(); ++i) {
		msg.AddFloat("offset", grad.stops[i].offset);
	}

	msg.AddInt32("type", static_cast<int32_t>(grad.type));
	msg.AddInt32("interpolation", static_cast<int32_t>(grad.interpolation));	
	msg.AddBool("inherit transformation", grad.inheritTransformation);
}

void
IOMWriter::_AddTransformerToMessage(BMessage& msg, const Transformer& trans)
{
	BMessage::Private msgPrivate(msg);
	
	if (msgPrivate.InitHeader() != B_OK)
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
				msg.AddData("transformation", B_DOUBLE_TYPE,
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
