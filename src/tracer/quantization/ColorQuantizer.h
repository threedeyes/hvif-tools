/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef COLOR_QUANTIZER_H
#define COLOR_QUANTIZER_H

#include <vector>

#include "BitmapData.h"
#include "IndexedBitmap.h"
#include "TracingOptions.h"

class ColorCube;

class ColorQuantizer {
public:
						ColorQuantizer();
						~ColorQuantizer();

	std::vector<int>	QuantizeImage(const std::vector<std::vector<int> >& pixels, int maxColors);
	std::vector<int>	QuantizeImageMasked(const std::vector<std::vector<int> >& pixels,
									int maxColors,
									int skipValue);

	IndexedBitmap		QuantizeColors(const BitmapData& bitmap,
									const std::vector<std::vector<unsigned char> >& palette,
									const TracingOptions& options);

private:
	void				_AdaptiveSpatialCoherence(std::vector<std::vector<int> >& indexArray,
									const BitmapData& bitmap,
									int width, int height,
									int radius, int passes);
	
	void				_MergeSimilarPaletteColors(std::vector<std::vector<unsigned char> >& palette,
									std::vector<std::vector<int> >& indexArray,
									int width, int height,
									int threshold);
	
	void				_RemapIndices(std::vector<std::vector<int> >& indexArray,
									const std::vector<int>& remapTable,
									int width, int height);

	double				_ComputeEdgeStrength(const BitmapData& bitmap, int cx, int cy);
};

#endif
