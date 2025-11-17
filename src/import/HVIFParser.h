/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef IMPORT_HVIF_PARSER_H
#define IMPORT_HVIF_PARSER_H

#include "HVIFStructures.h"

namespace hvif {

class HVIFParser {
public:
							HVIFParser();
							~HVIFParser();

	bool					ParseFile(const std::string& filename);
	bool					ParseData(const std::vector<uint8_t>& data, const std::string& filename = "");

	static bool				IsValidHVIFFile(const std::string& filename);
	static bool				IsValidHVIFData(const std::vector<uint8_t>& data);

	const HVIFIcon&			GetIcon() const { return *fIcon; }
	const uint8_t*			GetIconData() const { return fData; }
	const size_t			GetIconDataSize() const { return fSize; }

	HVIFIcon* TakeIcon() {
		HVIFIcon* icon = fIcon; 
		fIcon = NULL; 
		return icon;
	}

	const std::string&		GetLastError() const { return fLastError; }

private:
	HVIFIcon*				fIcon;
	std::string				fLastError;

	const uint8_t*			fData;
	size_t					fSize;
	size_t					fPos;

	bool					_ParseHeader();
	bool					_ReadStyle(Style& style);
	bool					_ReadPath(Path& path);
	bool					_ReadShape(Shape& shape);
	bool					_ReadColor(Color& color, ColorTags tag);
	bool					_ReadGradient(Gradient& gradient);
	bool					_ReadStops(std::vector<GradientStop>& stops, uint8_t count, ColorTags format);
	bool					_ReadTransformers(std::vector<Transformer>& transformers, uint8_t count);
	bool					_ReadMatrix(std::vector<float>& matrix);
	bool					_ReadCoords(std::vector<float>& points, int count);
	bool					_ReadControls(std::vector<float>& points, uint8_t pointCount);
	std::vector<uint8_t>	_ParseCommands(const std::vector<uint8_t>& buffer, uint8_t pointCount);
	float					_ParseFloat24(const uint8_t* bytes);

	bool					_ReadByte(uint8_t& value);
	bool					_ReadBytes(std::vector<uint8_t>& data, size_t count);
	bool					_CheckBounds(size_t needed);
	void					_SetError(const std::string& error);
};

}

#endif
