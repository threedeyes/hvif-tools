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

	std::vector<int>        QuantizeImage(const std::vector<std::vector<int>>& pixels, int maxColors);

	IndexedBitmap           QuantizeColors(const BitmapData& bitmap,
										const std::vector<std::vector<unsigned char>>& palette,
										const TracingOptions& options);

private:
	void                    _InitializeTables();

	static bool             sTablesInitialized;
	static int              sSquares[256 + 256 + 1];
	static int              sShift[9];
};

#endif // COLOR_QUANTIZER_H
