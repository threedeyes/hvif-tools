/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef IMAGE_TRACER_H
#define IMAGE_TRACER_H

#include <string>
#include <vector>

#include "BitmapData.h"
#include "IndexedBitmap.h"
#include "TracingOptions.h"

class ImageTracer {
public:
							ImageTracer();
							~ImageTracer();

	std::string				BitmapToSvg(const BitmapData& bitmap,
									const TracingOptions& options = TracingOptions());

	IndexedBitmap			BitmapToTraceData(const BitmapData& bitmap,
									const TracingOptions& options = TracingOptions());

	bool					SaveSvg(const std::string& filename,
									const std::string& svgData);

private:
	struct PixelSample {
		unsigned char r, g, b, a;
		double saturation;
		double brightness;

		PixelSample() : r(0), g(0), b(0), a(0), saturation(0), brightness(0) {}
	};

	std::vector<std::vector<unsigned char>> 
							_CreatePalette(const BitmapData& bitmap, int colorCount, const TracingOptions& options);

	double					_CalculateSaturation(unsigned char r, unsigned char g, unsigned char b);
	double					_CalculateBrightness(unsigned char r, unsigned char g, unsigned char b);
	double					_CalculateColorDistance(unsigned char r1, unsigned char g1, unsigned char b1,
													unsigned char r2, unsigned char g2, unsigned char b2);

	int						_FindNearestColorIndex(unsigned char r, unsigned char g, unsigned char b, unsigned char a,
												  const std::vector<std::vector<unsigned char> >& palette,
												  bool skipTransparent);

	void					_SelectRepresentativeColor(const std::vector<PixelSample>& samples,
													  unsigned char& outR,
													  unsigned char& outG,
													  unsigned char& outB,
													  unsigned char& outA,
													  int colorCount);

	void					_FixWindingOrder(IndexedBitmap& indexed);
};

#endif
