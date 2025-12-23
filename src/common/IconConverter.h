/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef ICON_CONVERTER_H
#define ICON_CONVERTER_H

#include <string>
#include <vector>
#include "HaikuIcon.h"
#include "HVIFWriter.h"
#include "PNGParser.h"

namespace haiku {

enum IconFormat {
	FORMAT_AUTO,
	FORMAT_UNKNOWN,
	FORMAT_HVIF,
	FORMAT_IOM,
	FORMAT_SVG,
	FORMAT_PNG
};

struct ConvertOptions {
	int	svgWidth;
	int svgHeight;
	std::string svgViewBox;
	bool preserveNames;
	bool verbose;
	float coordinateScale;
	int pngWidth;
	int pngHeight;
	float pngScale;
	PNGVectorizationPreset pngPreset;
	bool pngRemoveBackground;
	
	ConvertOptions() 
		: svgWidth(64), svgHeight(64), svgViewBox("0 0 64 64"),
		preserveNames(false), verbose(false), coordinateScale(1.0f),
		pngWidth(64), pngHeight(64), pngScale(1.0f),
		pngPreset(PRESET_ICON), pngRemoveBackground(false) {}
};

class IconConverter {
public:
	static bool			Convert(const std::string& inputFile, IconFormat inputFormat,
							const std::string& outputFile, IconFormat outputFormat,
							const ConvertOptions& opts);
	
	static bool			Convert(const std::string& inputFile, IconFormat inputFormat,
							const std::string& outputFile, IconFormat outputFormat);

	static bool			Convert(const std::string& inputFile, const std::string& outputFile,
							IconFormat outputFormat, const ConvertOptions& opts);

	static bool			Convert(const std::string& inputFile, const std::string& outputFile,
							IconFormat outputFormat);

	static IconFormat	DetectFormat(const std::string& file);
	static IconFormat	DetectFormatBySignature(const std::string& file);
	static IconFormat	DetectFormatByExtension(const std::string& file);
	
	static Icon			Load(const std::string& file, IconFormat format);
	
	static bool			Save(const Icon& icon, const std::string& file,
							IconFormat format, const ConvertOptions& opts);

	static bool			Save(const Icon& icon, const std::string& file, IconFormat format);

	static std::string	GetLastError();
	static std::string	FormatToString(IconFormat format);

	static bool			ConvertBuffer(const std::vector<uint8_t>& inputData, IconFormat inputFormat,
							std::vector<uint8_t>& outputData, IconFormat outputFormat, const ConvertOptions& opts);

	static bool			ConvertBuffer(const std::vector<uint8_t>& inputData, IconFormat inputFormat,
							std::vector<uint8_t>& outputData, IconFormat outputFormat);

	static Icon			LoadFromBuffer(const std::vector<uint8_t>& data, IconFormat format);

	static bool			SaveToBuffer(const Icon& icon, std::vector<uint8_t>& buffer,
							IconFormat format, const ConvertOptions& opts);

	static bool			SaveToBuffer(const Icon& icon, std::vector<uint8_t>& buffer, IconFormat format);

private:
	static std::string	sLastError;

	static void			SetError(const std::string& error);
	static Icon			LoadHVIF(const std::string& file);
	static Icon			LoadIOM(const std::string& file);
	static Icon			LoadSVG(const std::string& file, const ConvertOptions& opts);
	static Icon			LoadPNG(const std::string& file, const ConvertOptions& opts);
	
	static Icon			LoadWithOptions(const std::string& file, IconFormat format, const ConvertOptions& opts);
	
	static bool			SaveHVIF(const Icon& icon, const std::string& file);
	static bool			SaveIOM(const Icon& icon, const std::string& file);
	static bool			SaveSVG(const Icon& icon, const std::string& file, const ConvertOptions& opts);
	static bool			SavePNG(const Icon& icon, const std::string& file, const ConvertOptions& opts);

	static Icon			LoadHVIFBuffer(const std::vector<uint8_t>& data);
	static Icon			LoadIOMBuffer(const std::vector<uint8_t>& data);
	static Icon			LoadSVGBuffer(const std::vector<uint8_t>& data, const ConvertOptions& opts);
	static Icon			LoadPNGBuffer(const std::vector<uint8_t>& data, const ConvertOptions& opts);
	static bool			SaveHVIFBuffer(const Icon& icon, std::vector<uint8_t>& buffer);
	static bool			SaveIOMBuffer(const Icon& icon, std::vector<uint8_t>& buffer);
	static bool			SaveSVGBuffer(const Icon& icon, std::vector<uint8_t>& buffer, const ConvertOptions& opts);
	static bool			SavePNGBuffer(const Icon& icon, std::vector<uint8_t>& buffer, const ConvertOptions& opts);

	static bool			_PrepareHVIFWriter(const Icon& icon, hvif::HVIFWriter& writer);
};

}

#endif
