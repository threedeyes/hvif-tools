/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <fstream>
#include <cmath>
#include <algorithm>
#include <cstring>

#include "HVIFWriter.h"

namespace hvif {

template<typename T>
static T clamp_value(const T& value, const T& min_val, const T& max_val) {
	return (value < min_val) ? min_val : (value > max_val) ? max_val : value;
}

static long round_to_long(double x) {
	return static_cast<long>(x + (x >= 0 ? 0.5 : -0.5));
}

uint8_t
HVIFWriter::AddStyle(const Style& style)
{
	for (size_t i = 0; i < fStyles.size(); ++i) {
		if (fStyles[i] == style) {
			return static_cast<uint8_t>(i);
		}
	}
	fStyles.push_back(style);
	return static_cast<uint8_t>(fStyles.size() - 1);
}

uint8_t
HVIFWriter::AddPath(const Path& path)
{
	for (size_t i = 0; i < fPaths.size(); ++i) {
		if (fPaths[i] == path) {
			return static_cast<uint8_t>(i);
		}
	}
	fPaths.push_back(path);
	return static_cast<uint8_t>(fPaths.size() - 1);
}

uint8_t
HVIFWriter::AddInternalPath(const InternalPath& path)
{
	for (size_t i = 0; i < fInternalPaths.size(); ++i) {
		if (fInternalPaths[i] == path) {
			return static_cast<uint8_t>(i);
		}
	}
	fInternalPaths.push_back(path);
	return static_cast<uint8_t>(fInternalPaths.size() - 1);
}

void
HVIFWriter::AddShape(const Shape& shape)
{
	fShapes.push_back(shape);
}

void
HVIFWriter::_WriteByte(std::vector<uint8_t>& buffer, uint8_t byte)
{
	buffer.push_back(byte);
}

void
HVIFWriter::_WriteCoord(std::vector<uint8_t>& buffer, float coord)
{
	if (coord >= -32.0f && coord <= 95.0f && fmod(coord, 1.0f) == 0.0f) {
		_WriteByte(buffer, static_cast<uint8_t>(coord + 32.0f));
	} else {
		uint16_t value = static_cast<uint16_t>((coord + 128.0f) * 102.0f);
		_WriteByte(buffer, static_cast<uint8_t>((value >> 8) | 0x80));
		_WriteByte(buffer, static_cast<uint8_t>(value & 0xFF));
	}
}

void
HVIFWriter::_WriteFloat24(std::vector<uint8_t>& buffer, float value)
{
	if (value == 0.0f) { 
		_WriteByte(buffer, 0); _WriteByte(buffer, 0); _WriteByte(buffer, 0); 
		return; 
	}
	uint32_t bits;
	memcpy(&bits, &value, sizeof(bits));
	int32_t exponent = ((bits >> 23) & 0xFF) - 127;
	uint32_t mantissa = bits & 0x7FFFFF;
	uint32_t sign = (bits >> 31);
	exponent += 32;

	if (exponent < 0)
		exponent = 0;

	if (exponent > 63)
		exponent = 63;

	uint32_t result = (sign << 23) | (exponent << 17) | (mantissa >> 6);
	_WriteByte(buffer, (result >> 16) & 0xFF);
	_WriteByte(buffer, (result >> 8) & 0xFF);
	_WriteByte(buffer, result & 0xFF);
}

void
HVIFWriter::_WriteMatrix(std::vector<uint8_t>& buffer, const std::vector<float>& matrix)
{
	for (size_t i = 0; i < 6 && i < matrix.size(); i++) {
		_WriteFloat24(buffer, matrix[i]);
	}
}

void
HVIFWriter::_WriteColorData(std::vector<uint8_t>& buffer, const Color& color, bool noAlpha, bool gray)
{
	if (gray) {
		if (!color.data.empty()) _WriteByte(buffer, color.data[0]);
		if (!noAlpha) {
			if (color.data.size() > 3) {
				_WriteByte(buffer, color.data[3]);
			} else {
				_WriteByte(buffer, 255);
			}
		}
	} else {
		if (color.data.size() >= 3) {
			_WriteByte(buffer, color.data[0]);
			_WriteByte(buffer, color.data[1]);
			_WriteByte(buffer, color.data[2]);
		}
		if (!noAlpha && color.data.size() > 3) {
			_WriteByte(buffer, color.data[3]);
		}
	}
}

void
HVIFWriter::_WriteStyleData(std::vector<uint8_t>& buffer, const Style& style)
{
	if (style.isGradient) {
		_WriteByte(buffer, GRADIENT);
		_WriteByte(buffer, static_cast<uint8_t>(style.gradient.type));

		uint8_t flags = style.gradient.flags;
		if (style.gradient.hasMatrix) flags |= TRANSFORM;

		bool allGray = true;
		for (size_t i = 0; i < style.gradient.stops.size(); ++i) {
			const GradientStop& stop = style.gradient.stops[i];
			if (stop.color.data.size() >= 3 &&
				(stop.color.data[0] != stop.color.data[1] || stop.color.data[1] != stop.color.data[2])) {
				allGray = false;
				break;
			}
		}
		
		bool hasAlphaChannel = false;
		for (size_t i = 0; i < style.gradient.stops.size(); ++i) {
			const GradientStop& stop = style.gradient.stops[i];
			if (stop.color.data.size() > 3) {
				hasAlphaChannel = true;
				break;
			}
		}
		
		if (!hasAlphaChannel)
			flags |= NO_ALPHA;

		if (allGray)
			flags |= GREYS;

		_WriteByte(buffer, flags);
		_WriteByte(buffer, static_cast<uint8_t>(style.gradient.stops.size()));
		if (style.gradient.hasMatrix) _WriteMatrix(buffer, style.gradient.matrix);

		for (size_t i = 0; i < style.gradient.stops.size(); ++i) {
			const GradientStop& stop = style.gradient.stops[i];
			_WriteByte(buffer, stop.offset);
			_WriteColorData(buffer, stop.color, !hasAlphaChannel, allGray);
		}
	} else {
		bool isGray = (style.color.data.size() >= 3 &&
					  style.color.data[0] == style.color.data[1] &&
					  style.color.data[1] == style.color.data[2]);
		bool hasAlpha = (style.color.data.size() > 3);
		
		if (isGray) {
			_WriteByte(buffer, hasAlpha ? KA : K);
		} else {
			_WriteByte(buffer, hasAlpha ? RGBA : RGB);
		}
		_WriteColorData(buffer, style.color, !hasAlpha, isGray);
	}
}

void
HVIFWriter::_WritePathData(std::vector<uint8_t>& buffer, const Path& path)
{
	uint8_t flags = 0;
	if (path.closed) flags |= PATH_FLAG_CLOSED;
	flags |= PATH_FLAG_USES_COMMANDS;

	_WriteByte(buffer, flags);
	
	size_t numPoints = path.points.size() / 2;
	_WriteByte(buffer, static_cast<uint8_t>(numPoints));
	
	for (size_t i = 0; i < (numPoints + 3) / 4; ++i) {
		uint8_t byte = 0;
		for (size_t j = 0; j < 4; ++j) {
			if (i * 4 + j < numPoints) {
				byte |= (CMD_LINE << (j * 2));
			}
		}
		_WriteByte(buffer, byte);
	}
	
	for (size_t i = 0; i < path.points.size(); ++i) {
		_WriteCoord(buffer, path.points[i]);
	}
}

void
HVIFWriter::_WriteInternalPathData(std::vector<uint8_t>& buffer, const InternalPath& path)
{
	uint8_t flags = 0;
	if (path.closed) flags |= PATH_FLAG_CLOSED;
	flags |= PATH_FLAG_USES_COMMANDS;

	_WriteByte(buffer, flags);
	_WriteByte(buffer, static_cast<uint8_t>(path.nodes.size()));

	for (size_t i = 0; i < (path.nodes.size() + 3) / 4; ++i) {
		uint8_t byte = 0;
		for (size_t j = 0; j < 4; ++j) {
			size_t node_idx = i * 4 + j;
			if (node_idx < path.nodes.size()) {
				const PathNode& node = path.nodes[node_idx];
				bool isLine = (fabs(node.x - node.x_in) < 1e-3 && 
							  fabs(node.y - node.y_in) < 1e-3 &&
							  fabs(node.x - node.x_out) < 1e-3 && 
							  fabs(node.y - node.y_out) < 1e-3);
				byte |= ((isLine ? CMD_LINE : CMD_CURVE) << (j * 2));
			}
		}
		_WriteByte(buffer, byte);
	}

	for (size_t i = 0; i < path.nodes.size(); ++i) {
		const PathNode& node = path.nodes[i];
		bool isLine = (fabs(node.x - node.x_in) < 1e-3 && 
					  fabs(node.y - node.y_in) < 1e-3 &&
					  fabs(node.x - node.x_out) < 1e-3 && 
					  fabs(node.y - node.y_out) < 1e-3);
		if (isLine) {
			_WriteCoord(buffer, node.x);
			_WriteCoord(buffer, node.y);
		} else {
			_WriteCoord(buffer, node.x);
			_WriteCoord(buffer, node.y);
			_WriteCoord(buffer, node.x_in);
			_WriteCoord(buffer, node.y_in);
			_WriteCoord(buffer, node.x_out);
			_WriteCoord(buffer, node.y_out);
		}
	}
}

void
HVIFWriter::_WriteShapeData(std::vector<uint8_t>& buffer, const Shape& shape)
{
	_WriteByte(buffer, 0x0A);
	_WriteByte(buffer, shape.styleIndex);
	_WriteByte(buffer, static_cast<uint8_t>(shape.pathIndices.size()));
	for (size_t i = 0; i < shape.pathIndices.size(); ++i) {
		_WriteByte(buffer, shape.pathIndices[i]);
	}

	uint8_t flags = 0;
	if (shape.hasTransform) flags |= SHAPE_FLAG_TRANSFORM;
	if (!shape.transformers.empty()) flags |= SHAPE_FLAG_HAS_TRANSFORMERS;
	_WriteByte(buffer, flags);

	if (shape.hasTransform) _WriteMatrix(buffer, shape.transform);

	if (!shape.transformers.empty()) {
		_WriteByte(buffer, static_cast<uint8_t>(shape.transformers.size()));
		for (size_t i = 0; i < shape.transformers.size(); ++i) {
			const Transformer& t = shape.transformers[i];
			_WriteByte(buffer, static_cast<uint8_t>(t.tag));

			int encodedWidth = static_cast<int>(round_to_long(t.width)) + 128;
			encodedWidth = clamp_value(encodedWidth, 0, 255);
			if (encodedWidth == 128 && t.width > 0.0f)
				encodedWidth = 129;
			_WriteByte(buffer, static_cast<uint8_t>(encodedWidth));

			uint8_t lineOptions = static_cast<uint8_t>((t.lineCap << 4) | (t.lineJoin & 0x0F));
			_WriteByte(buffer, lineOptions);

			uint8_t encodedMiter = static_cast<uint8_t>(
				clamp_value<int>(static_cast<int>(round_to_long(t.miterLimit)), 0, 255)
			);
			_WriteByte(buffer, encodedMiter);
		}
	}
}

std::vector<uint8_t>
HVIFWriter::GetData()
{
	std::vector<uint8_t> data;
	_WriteByte(data, 'n'); _WriteByte(data, 'c'); _WriteByte(data, 'i'); _WriteByte(data, 'f');

	_WriteByte(data, static_cast<uint8_t>(fStyles.size()));
	for (size_t i = 0; i < fStyles.size(); ++i) {
		_WriteStyleData(data, fStyles[i]);
	}

	_WriteByte(data, static_cast<uint8_t>(fPaths.size() + fInternalPaths.size()));
	for (size_t i = 0; i < fPaths.size(); ++i) {
		_WritePathData(data, fPaths[i]);
	}
	for (size_t i = 0; i < fInternalPaths.size(); ++i) {
		_WriteInternalPathData(data, fInternalPaths[i]);
	}

	_WriteByte(data, static_cast<uint8_t>(fShapes.size()));
	for (size_t i = 0; i < fShapes.size(); ++i) {
		_WriteShapeData(data, fShapes[i]);
	}

	return data;
}

std::vector<uint8_t>
HVIFWriter::WriteToBuffer()
{
	return GetData();
}

bool
HVIFWriter::WriteToBuffer(std::vector<uint8_t>& buffer)
{
	buffer = GetData();
	return !buffer.empty();
}

bool
HVIFWriter::WriteToFile(const std::string& filename)
{
	std::vector<uint8_t> data = GetData();
	std::ofstream file(filename.c_str(), std::ios::binary);
	if (!file.is_open()) return false;
	file.write(reinterpret_cast<const char*>(&data[0]), data.size());
	file.close();
	return file.good();
}

}
