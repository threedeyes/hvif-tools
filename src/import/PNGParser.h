/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef IMPORT_PNG_PARSER_H
#define IMPORT_PNG_PARSER_H

#include <string>
#include <vector>

#include "HaikuIcon.h"
#include "BitmapData.h"
#include "TracingOptions.h"

#ifdef __HAIKU__
class BBitmap;
#endif

namespace haiku {

enum PNGVectorizationPreset {
	PRESET_ICON = 0,
	PRESET_ICON_GRADIENT = 1
};

struct PNGParseOptions {
	PNGVectorizationPreset	preset;
	bool					removeBackground;
	bool					verbose;

	PNGParseOptions() 
		: preset(PRESET_ICON)
		, removeBackground(false)
		, verbose(false) 
	{}
};

class PNGParser {
public:
				PNGParser();
				~PNGParser();

	bool		Parse(const std::string& file, Icon& icon, const PNGParseOptions& opts);
	bool		Parse(const std::string& file, Icon& icon);
	
	bool		ParseBuffer(const std::vector<uint8_t>& data, Icon& icon, const PNGParseOptions& opts);
	bool		ParseBuffer(const std::vector<uint8_t>& data, Icon& icon);

	std::string	GetLastError() const { return fLastError; }

private:
	TracingOptions 
				_CreateTracingOptions(const PNGParseOptions& opts);
	
	TracingOptions 
				_GetIconPreset();
	
	TracingOptions 
				_GetIconGradientPreset();

	BitmapData	_LoadBitmapFromFile(const std::string& file);
	BitmapData	_LoadBitmapFromBuffer(const std::vector<uint8_t>& data);

#ifdef __HAIKU__
	BitmapData	_ConvertBBitmapToBitmapData(BBitmap* bitmap);
#endif

	std::string	fLastError;
};

}

#endif
