/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <climits>
#include <cmath>
#include <algorithm>

#include "ColorQuantizer.h"
#include "ColorCube.h"
#include "SelectiveBlur.h"

static const int kMaxRgb = 255;
static const int kMaxTreeDepth = 8;

bool ColorQuantizer::sTablesInitialized = false;
int ColorQuantizer::sSquares[256 + 256 + 1];
int ColorQuantizer::sShift[9];

ColorQuantizer::ColorQuantizer()
{
	_InitializeTables();
}

ColorQuantizer::~ColorQuantizer()
{
}

void
ColorQuantizer::_InitializeTables()
{
	if (sTablesInitialized)
		return;

	for (int i = -kMaxRgb; i <= kMaxRgb; i++)
		sSquares[i + kMaxRgb] = i * i;

	for (int i = 0; i < kMaxTreeDepth + 1; ++i)
		sShift[i] = 1 << (15 - i);

	sTablesInitialized = true;
}

std::vector<int>
ColorQuantizer::QuantizeImage(const std::vector<std::vector<int>>& pixels, int maxColors)
{
	_InitializeTables();

	ColorCube cube(pixels, maxColors);
	cube.ClassifyColors();
	cube.ReduceColors();
	cube.AssignColors();
	return cube.GetColormap();
}

IndexedBitmap
ColorQuantizer::QuantizeColors(const BitmapData& bitmap,
							const std::vector<std::vector<unsigned char>>& palette,
							const TracingOptions& options)
{
	BitmapData processedBitmap = bitmap;

	// Apply selective blur if enabled
	if (options.fBlurRadius > 0) {
		SelectiveBlur blur;
		processedBitmap = blur.BlurBitmap(processedBitmap, 
										options.fBlurRadius, 
										options.fBlurDelta);
	}

	int cycles = static_cast<int>(options.fColorQuantizationCycles);

	// Create indexed color array with boundary filled with -1
	std::vector<std::vector<int>> indexArray(processedBitmap.Height() + 2);
	for (int j = 0; j < processedBitmap.Height() + 2; j++) {
		indexArray[j].resize(processedBitmap.Width() + 2, -1);
	}

	std::vector<std::vector<unsigned char>> workingPalette = palette;
	std::vector<std::vector<long>> paletteAccumulator(palette.size());
	for (int k = 0; k < static_cast<int>(palette.size()); k++) {
		paletteAccumulator[k].resize(5, 0);
	}

	// Check for transparent color in palette
	bool hasTransparentColor = false;
	int transparentIndex = -1;
	if (!palette.empty() && palette[0][3] == 0) {
		hasTransparentColor = true;
		transparentIndex = 0;
	}

	// Color quantization cycles
	for (int cycle = 0; cycle < cycles; cycle++) {
		// Average colors from the second iteration
		if (cycle > 0) {
			for (int k = 0; k < static_cast<int>(workingPalette.size()); k++) {
				if (paletteAccumulator[k][4] > 0) {
					workingPalette[k][0] = static_cast<unsigned char>(paletteAccumulator[k][0] / paletteAccumulator[k][4]);
					workingPalette[k][1] = static_cast<unsigned char>(paletteAccumulator[k][1] / paletteAccumulator[k][4]);
					workingPalette[k][2] = static_cast<unsigned char>(paletteAccumulator[k][2] / paletteAccumulator[k][4]);
					workingPalette[k][3] = static_cast<unsigned char>(paletteAccumulator[k][3] / paletteAccumulator[k][4]);
				}
			}
		}

		// Reset palette accumulator
		for (int i = 0; i < static_cast<int>(palette.size()); i++) {
			for (int j = 0; j < 5; j++) {
				paletteAccumulator[i][j] = 0;
			}
		}

		// Process all pixels
		for (int y = 0; y < processedBitmap.Height(); y++) {
			for (int x = 0; x < processedBitmap.Width(); x++) {
				unsigned char red   = processedBitmap.GetPixelComponent(x, y, 0);
				unsigned char green = processedBitmap.GetPixelComponent(x, y, 1);
				unsigned char blue  = processedBitmap.GetPixelComponent(x, y, 2);
				unsigned char alpha = processedBitmap.GetPixelComponent(x, y, 3);

				// Handle fully transparent pixels
				if (alpha == 0) {
					if (hasTransparentColor) {
						indexArray[y + 1][x + 1] = transparentIndex;
						paletteAccumulator[transparentIndex][4]++;
					} else {
						indexArray[y + 1][x + 1] = 0;
						paletteAccumulator[0][4]++;
					}
					continue;
				}

				// Find closest color by measuring distance
				long closestDistance = 256L * 256L;
				int closestIndex = hasTransparentColor ? 1 : 0;

				for (int k = (hasTransparentColor ? 1 : 0); k < static_cast<int>(workingPalette.size()); k++) {
					long redDiff   = abs(static_cast<int>(workingPalette[k][0]) - static_cast<int>(red));
					long greenDiff = abs(static_cast<int>(workingPalette[k][1]) - static_cast<int>(green));
					long blueDiff  = abs(static_cast<int>(workingPalette[k][2]) - static_cast<int>(blue));
					long alphaDiff = abs(static_cast<int>(workingPalette[k][3]) - static_cast<int>(alpha));
					long distance = redDiff + greenDiff + blueDiff + (alphaDiff * 4); // weighted alpha

					if (distance < closestDistance) {
						closestDistance = distance;
						closestIndex = k;
					}
				}

				// Accumulate for averaging
				paletteAccumulator[closestIndex][0] += red;
				paletteAccumulator[closestIndex][1] += green;
				paletteAccumulator[closestIndex][2] += blue;
				paletteAccumulator[closestIndex][3] += alpha;
				paletteAccumulator[closestIndex][4]++;

				indexArray[y + 1][x + 1] = closestIndex;
			}
		}
	}

	return IndexedBitmap(indexArray, workingPalette);
}
