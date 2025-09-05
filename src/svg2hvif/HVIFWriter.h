/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef HVIF_WRITER_H
#define HVIF_WRITER_H

#include <vector>
#include <stdint.h>
#include <string>

#include "HVIFStructures.h"

namespace hvif {

enum PathCommandTags {
	CMD_VLINE = 0,
	CMD_HLINE = 1, 
	CMD_LINE = 2,
	CMD_CURVE = 3
};

enum InternalFlags {
	PATH_FLAG_CLOSED = 1 << 1,
	PATH_FLAG_USES_COMMANDS = 1 << 2,
	PATH_FLAG_NO_CURVES = 1 << 3,
	SHAPE_FLAG_TRANSFORM = 1 << 1,
	SHAPE_FLAG_HAS_TRANSFORMERS = 1 << 4
};

struct PathNode {
	float x, y;
	float x_in, y_in;
	float x_out, y_out;

	bool operator==(const PathNode& other) const {
		return x == other.x &&
			   y == other.y &&
			   x_in == other.x_in &&
			   y_in == other.y_in &&
			   x_out == other.x_out &&
			   y_out == other.y_out;
	}
};

struct InternalPath {
	std::vector<PathNode> nodes;
	bool closed;

	bool operator==(const InternalPath& other) const {
		return nodes == other.nodes && closed == other.closed;
	}
};

class HVIFWriter {
public:
	uint8_t AddStyle(const Style& style);
	uint8_t AddPath(const Path& path);
	uint8_t AddInternalPath(const InternalPath& path);
	void AddShape(const Shape& shape);

	bool WriteToFile(const std::string& filename);

	std::vector<uint8_t> WriteToBuffer();
	bool WriteToBuffer(std::vector<uint8_t>& buffer);

	std::vector<uint8_t> GetData();

private:
	std::vector<Style> fStyles;
	std::vector<Path> fPaths;
	std::vector<InternalPath> fInternalPaths;
	std::vector<Shape> fShapes;

	void _WriteByte(std::vector<uint8_t>& buffer, uint8_t byte);
	void _WriteCoord(std::vector<uint8_t>& buffer, float coord);
	void _WriteFloat24(std::vector<uint8_t>& buffer, float value);
	void _WriteMatrix(std::vector<uint8_t>& buffer, const std::vector<float>& matrix);
	void _WriteColorData(std::vector<uint8_t>& buffer, const Color& color, bool noAlpha, bool gray);
	void _WriteStyleData(std::vector<uint8_t>& buffer, const Style& style);
	void _WritePathData(std::vector<uint8_t>& buffer, const Path& path);
	void _WriteInternalPathData(std::vector<uint8_t>& buffer, const InternalPath& path);
	void _WriteShapeData(std::vector<uint8_t>& buffer, const Shape& shape);
};

}

#endif
