/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef EXPORT_PNG_WRITER_H
#define EXPORT_PNG_WRITER_H

#include <string>
#include <vector>
#include "HaikuIcon.h"

#ifdef __HAIKU__
class BBitmap;
#endif

namespace haiku {

struct PNGWriterOptions {
	int			width;
	int			height;
	float		scale;

	PNGWriterOptions() : width(64), height(64), scale(1.0f) {}
};

class PNGWriter {
public:
				PNGWriter();
				~PNGWriter();

	bool		WriteToFile(const Icon& icon, const std::string& filename, 
					const PNGWriterOptions& opts);

	bool		WriteToBuffer(const Icon& icon, std::vector<uint8_t>& buffer,
					const PNGWriterOptions& opts);

private:
	bool		_RasterizeIcon(const Icon& icon, const PNGWriterOptions& opts,
					std::vector<uint8_t>& pixelData, int& outWidth, int& outHeight);

	std::string _GenerateSVGString(const Icon& icon, int width, int height);

#ifdef __HAIKU__
	bool		_SaveBBitmapToPNG(BBitmap* bitmap, const std::string& filename);
	bool		_SaveBBitmapToPNGBuffer(BBitmap* bitmap, std::vector<uint8_t>& buffer);
	BBitmap*	_CreateBBitmapFromPixelData(const std::vector<uint8_t>& pixelData, 
					int width, int height);
#endif
};

}

#endif
