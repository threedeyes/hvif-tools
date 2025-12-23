/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <fstream>
#include <algorithm>
#include <sstream>

#include "HVIFParser.h"
#include "Utils.h"

namespace hvif {

struct GradientStopComparator {
	bool operator()(const GradientStop& a, const GradientStop& b) const {
		return a.offset < b.offset;
	}
};

HVIFParser::HVIFParser()
	: fIcon(NULL), fData(NULL), fSize(0), fPos(0)
{
}

HVIFParser::~HVIFParser()
{
	delete fIcon;
}

bool
HVIFParser::ParseFile(const std::string& filename)
{
	std::ifstream file(filename.c_str(), std::ios::binary);
	if (!file.is_open()) {
		_SetError("Cannot open file: " + filename);
		return false;
	}

	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<uint8_t> data(size);
	file.read(reinterpret_cast<char*>(&data[0]), size);
	file.close();

	return ParseData(data, filename);
}

bool
HVIFParser::ParseData(const std::vector<uint8_t>& data, const std::string& filename)
{
	fData = &data[0];
	fSize = data.size();
	fPos = 0;
	fLastError.clear();

	delete fIcon;
	fIcon = new HVIFIcon();
	fIcon->filename = filename;

	if (!_ParseHeader())
		return false;

	uint8_t styleCount;
	if (!_ReadByte(styleCount))
		return false;

	fIcon->styles.reserve(styleCount);
	for (int i = 0; i < styleCount; i++) {
		Style style;
		if (!_ReadStyle(style)) {
			return false;
		}
		fIcon->styles.push_back(style);
	}

	uint8_t pathCount;
	if (!_ReadByte(pathCount))
		return false;

	fIcon->paths.reserve(pathCount);
	for (int i = 0; i < pathCount; i++) {
		Path path;
		if (!_ReadPath(path)) {
			return false;
		}
		fIcon->paths.push_back(path);
	}

	uint8_t shapeCount;
	if (!_ReadByte(shapeCount))
		return false;

	fIcon->shapes.reserve(shapeCount);
	for (int i = 0; i < shapeCount; i++) {
		Shape shape;
		if (!_ReadShape(shape)) {
			return false;
		}
		fIcon->shapes.push_back(shape);
	}

	if (fPos != fSize) {
		_SetError("Additional padding after hvif file");
		return false;
	}

	return true;
}

bool
HVIFParser::IsValidHVIFFile(const std::string& filename)
{
	std::ifstream file(filename.c_str(), std::ios::binary);
	if (!file.is_open())
		return false;
	
	std::vector<uint8_t> header(4);
	file.read(reinterpret_cast<char*>(&header[0]), 4);
	file.close();

	return IsValidHVIFData(header);
}

bool
HVIFParser::IsValidHVIFData(const std::vector<uint8_t>& data)
{
	if (data.size() < 4)
		return false;

	return data[0] == 0x6E && data[1] == 0x63 && 
		data[2] == 0x69 && data[3] == 0x66;
}

bool
HVIFParser::_ParseHeader()
{
	if (!_CheckBounds(4))
		return false;

	std::vector<uint8_t> headerData(fData, fData + 4);
	if (!IsValidHVIFData(headerData)) {
		_SetError("Not a valid HVIF file");
		return false;
	}

	fPos = 4;
	return true;
}

bool
HVIFParser::_ReadStyle(Style& style)
{
	uint8_t tag;
	if (!_ReadByte(tag))
		return false;
	
	if (tag == GRADIENT) {
		style.isGradient = true;
		return _ReadGradient(style.gradient);
	} else {
		style.isGradient = false;
		return _ReadColor(style.color, static_cast<ColorTags>(tag));
	}
}

bool
HVIFParser::_ReadPath(Path& path)
{
	uint8_t flags, pointCount;
	if (!_ReadByte(flags) || !_ReadByte(pointCount))
		return false;

	path.closed = (flags & CLOSED) != 0;

	if (flags & POINTS) {
		path.type = "points";
		if (!_ReadCoords(path.points, pointCount * 2)) {
			return false;
		}
	} else if (flags & COMMANDS) {
		path.type = "curves";
		if (!_ReadControls(path.points, pointCount)) {
			return false;
		}
	} else {
		path.type = "curves";
		if (!_ReadCoords(path.points, pointCount * 6)) {
			return false;
		}
	}

	return true;
}

bool
HVIFParser::_ReadShape(Shape& shape)
{
	uint8_t tag;
	if (!_ReadByte(tag))
		return false;
	
	if (tag != 0x0A) {
		_SetError("Unknown shape tag: " + utils::ToString(static_cast<int>(tag)));
		return false;
	}
	
	uint8_t pathCount;
	if (!_ReadByte(shape.styleIndex) || !_ReadByte(pathCount))
		return false;
	
	shape.pathIndices.clear();
	shape.pathIndices.reserve(pathCount);
	for (int i = 0; i < pathCount; i++) {
		uint8_t pathIndex;
		if (!_ReadByte(pathIndex)) {
			return false;
		}
		shape.pathIndices.push_back(pathIndex);
	}

	uint8_t flags;
	if (!_ReadByte(flags))
		return false;

	if (flags & MATRIX) {
		if (!_ReadMatrix(shape.transform)) {
			return false;
		}
		shape.transformType = "matrix";
		shape.hasTransform = true;
	} else if (flags & TRANSLATE) {
		if (!_ReadCoords(shape.transform, 2)) {
			return false;
		}
		shape.transformType = "translate";
		shape.hasTransform = true;
	}

	if (flags & LOD_SCALE) {
		uint8_t min, max;
		if (!_ReadByte(min) || !_ReadByte(max)) {
			return false;
		}
		shape.minLOD = min;
		shape.maxLOD = max;
		shape.hasLOD = true;
	}

	if (flags & TRANSFORMERS) {
		uint8_t count;
		if (!_ReadByte(count)) {
			return false;
		}
		if (!_ReadTransformers(shape.transformers, count)) {
			return false;
		}
	}

	return true;
}

bool
HVIFParser::_ReadColor(Color& color, ColorTags tag)
{
	color.tag = tag;
	
	switch (tag) {
		case RGBA:
			return _ReadBytes(color.data, 4);
		case RGB:
			return _ReadBytes(color.data, 3);
		case KA:
			return _ReadBytes(color.data, 2);
		case K:
			return _ReadBytes(color.data, 1);
		default:
			_SetError("Unknown color format: " + utils::ToString(static_cast<int>(tag)));
			return false;
	}
}

bool
HVIFParser::_ReadGradient(Gradient& gradient)
{
	uint8_t type, flags, stopCount;
	if (!_ReadByte(type) || !_ReadByte(flags) || !_ReadByte(stopCount))
		return false;

	gradient.type = static_cast<GradientTypes>(type);
	gradient.flags = flags;

	ColorTags colorFormat;
	if (gradient.flags & GREYS) {
		colorFormat = (gradient.flags & NO_ALPHA) ? K : KA;
	} else {
		colorFormat = (gradient.flags & NO_ALPHA) ? RGB : RGBA;
	}

	if (gradient.flags & TRANSFORM) {
		if (!_ReadMatrix(gradient.matrix)) {
			return false;
		}
		gradient.hasMatrix = true;
	}

	return _ReadStops(gradient.stops, stopCount, colorFormat);
}

bool
HVIFParser::_ReadStops(std::vector<GradientStop>& stops, uint8_t count, ColorTags format)
{
	stops.clear();
	stops.reserve(count);

	for (int i = 0; i < count; i++) {
		GradientStop stop;
		if (!_ReadByte(stop.offset) || !_ReadColor(stop.color, format)) {
			return false;
		}
		stops.push_back(stop);
	}

	std::sort(stops.begin(), stops.end(), GradientStopComparator());

	return true;
}

bool
HVIFParser::_ReadTransformers(std::vector<Transformer>& transformers, uint8_t count)
{
	transformers.clear();
	transformers.reserve(count);

	for (int i = 0; i < count; i++) {
		Transformer transformer;
		uint8_t tag;
		if (!_ReadByte(tag))
			return false;

		transformer.tag = static_cast<TransformerTags>(tag);

		switch (transformer.tag) {
			case AFFINE:
				transformer.data.clear();
				transformer.data.reserve(6);
				for (int j = 0; j < 6; j++) {
					std::vector<uint8_t> bytes;
					if (!_ReadBytes(bytes, 3))
						return false;
					transformer.data.push_back(_ParseFloat24(&bytes[0]));
				}
				break;
				
			case CONTOUR: {
				uint8_t width, lineJoin, miterLimit;
				if (!_ReadByte(width) || !_ReadByte(lineJoin) || !_ReadByte(miterLimit)) {
					return false;
				}
				transformer.width = (width - 128) * 102;
				transformer.lineJoin = lineJoin;
				transformer.miterLimit = miterLimit;
				break;
			}

			case PERSPECTIVE:
				transformer.data.clear();
				transformer.data.reserve(9);
				for (int j = 0; j < 9; j++) {
					std::vector<uint8_t> bytes;
					if (!_ReadBytes(bytes, 3))
						return false;
					transformer.data.push_back(_ParseFloat24(&bytes[0]));
				}
				break;

			case STROKE: {
				uint8_t width, lineOptions, miterLimit;
				if (!_ReadByte(width) || !_ReadByte(lineOptions) || !_ReadByte(miterLimit)) {
					return false;
				}
				transformer.width = (width - 128) * 102;
				transformer.lineJoin = lineOptions & 0x0F;
				transformer.lineCap = (lineOptions >> 4) & 0x0F;
				transformer.miterLimit = miterLimit;
				break;
			}

			default:
				_SetError("Unknown transformer tag: " + utils::ToString(static_cast<int>(transformer.tag)));
				return false;
		}

		transformers.push_back(transformer);
	}

	return true;
}

bool
HVIFParser::_ReadMatrix(std::vector<float>& matrix)
{
	matrix.clear();
	matrix.reserve(6);

	for (int i = 0; i < 6; i++) {
		std::vector<uint8_t> bytes;
		if (!_ReadBytes(bytes, 3)) {
			return false;
		}
		matrix.push_back(_ParseFloat24(&bytes[0]));
	}
	return true;
}

bool
HVIFParser::_ReadCoords(std::vector<float>& points, int count)
{
	points.clear();
	points.reserve(count);

	for (int i = 0; i < count; i++) {
		uint8_t v;
		if (!_ReadByte(v))
			return false;

		if (v >= 128) {
			uint8_t v2;
			if (!_ReadByte(v2)) {
				return false;
			}
			int value = ((v & 127) << 8) + v2;
			points.push_back(value - 128 * 102);
		} else {
			points.push_back(v * 102 - 32 * 102);
		}
	}

	return true;
}

bool
HVIFParser::_ReadControls(std::vector<float>& points, uint8_t pointCount)
{
	int commandBytes = (pointCount + 3) / 4;
	std::vector<uint8_t> commandData;
	if (!_ReadBytes(commandData, commandBytes))
		return false;

	std::vector<uint8_t> commands = _ParseCommands(commandData, pointCount);

	points.clear();
	std::vector<float> lastPoint(6, 0.0f);

	for (int i = 0; i < pointCount; i++) {
		uint8_t tag = commands[i];
		std::vector<float> coords;

		switch (tag) {
			case VLINE: {
				if (!_ReadCoords(coords, 1)) return false;
				std::vector<float> p(6);
				p[0] = coords[0]; p[1] = lastPoint[1];
				p[2] = coords[0]; p[3] = lastPoint[1];
				p[4] = coords[0]; p[5] = lastPoint[1];
				points.insert(points.end(), p.begin(), p.end());
				lastPoint = p;
				break;
			}

			case HLINE: {
				if (!_ReadCoords(coords, 1)) return false;
				std::vector<float> p(6);
				p[0] = lastPoint[0]; p[1] = coords[0];
				p[2] = lastPoint[0]; p[3] = coords[0];
				p[4] = lastPoint[0]; p[5] = coords[0];
				points.insert(points.end(), p.begin(), p.end());
				lastPoint = p;
				break;
			}

			case LINE: {
				std::vector<float> coordX, coordY;
				if (!_ReadCoords(coordX, 1) || !_ReadCoords(coordY, 1)) return false;
				std::vector<float> p(6);
				p[0] = coordX[0]; p[1] = coordY[0];
				p[2] = coordX[0]; p[3] = coordY[0];
				p[4] = coordX[0]; p[5] = coordY[0];
				points.insert(points.end(), p.begin(), p.end());
				lastPoint = p;
				break;
			}

			case CURVE: {
				std::vector<float> p(6);
				if (!_ReadCoords(p, 6)) return false;
				points.insert(points.end(), p.begin(), p.end());
				lastPoint = p;
				break;
			}
		}
	}

	return true;
}

std::vector<uint8_t>
HVIFParser::_ParseCommands(const std::vector<uint8_t>& buffer, uint8_t pointCount)
{
	std::vector<uint8_t> commands;

	for (size_t i = 0; i < buffer.size(); i++) {
		uint8_t byte = buffer[i];
		commands.push_back(byte & 0x03);
		commands.push_back((byte >> 2) & 0x03);
		commands.push_back((byte >> 4) & 0x03);
		commands.push_back((byte >> 6) & 0x03);
	}

	if (commands.size() > pointCount) {
		commands.resize(pointCount);
	}

	return commands;
}

float
HVIFParser::_ParseFloat24(const uint8_t* bytes)
{
	uint32_t shortValue = (bytes[0] << 16) | (bytes[1] << 8) | bytes[2];
	uint32_t sign = (shortValue & 0x800000) >> 23;
	int32_t exponent = ((shortValue & 0x7E0000) >> 17) - 32;
	uint32_t mantissa = (shortValue & 0x01FFFF) << 6;

	uint32_t value = (sign << 31) | ((exponent + 127) << 23) | mantissa;

	return *reinterpret_cast<float*>(&value);
}

bool
HVIFParser::_ReadByte(uint8_t& value)
{
	if (!_CheckBounds(1))
		return false;

	value = fData[fPos++];
	return true;
}

bool
HVIFParser::_ReadBytes(std::vector<uint8_t>& data, size_t count)
{
	if (!_CheckBounds(count))
		return false;

	data.assign(fData + fPos, fData + fPos + count);
	fPos += count;
	return true;
}

bool
HVIFParser::_CheckBounds(size_t needed)
{
	if (fPos + needed > fSize) {
		_SetError("Unexpected end of file");
		return false;
	}
	return true;
}

void
HVIFParser::_SetError(const std::string& error)
{
	fLastError = error;
}

}
