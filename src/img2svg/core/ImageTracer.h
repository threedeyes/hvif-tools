/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef IMAGE_TRACER_H
#define IMAGE_TRACER_H

#include "BitmapData.h"
#include "IndexedBitmap.h"
#include "TracingOptions.h"
#include <string>
#include <vector>

class ImageTracer {
public:
							ImageTracer();
							~ImageTracer();

	std::string             BitmapToSvg(const BitmapData& bitmap,
									const TracingOptions& options = TracingOptions());

	IndexedBitmap           BitmapToTraceData(const BitmapData& bitmap,
									const TracingOptions& options = TracingOptions());

	bool                    SaveSvg(const std::string& filename,
									const std::string& svgData);

private:
	std::vector<std::vector<unsigned char>> 
							_CreatePalette(const BitmapData& bitmap, int colorCount);
};

#endif // IMAGE_TRACER_H
