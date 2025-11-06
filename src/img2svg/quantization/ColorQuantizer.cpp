/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <climits>
#include <cmath>
#include <algorithm>
#include <map>
#include <iostream>

#include "ColorQuantizer.h"
#include "ColorCube.h"
#include "SelectiveBlur.h"
#include "MathUtils.h"

ColorQuantizer::ColorQuantizer()
{
	MathUtils::Init();
}

ColorQuantizer::~ColorQuantizer()
{
}

std::vector<int>
ColorQuantizer::QuantizeImage(const std::vector<std::vector<int> >& pixels, int maxColors)
{
	MathUtils::Init();

	ColorCube cube(pixels, maxColors);
	cube.ClassifyColors();
	cube.ReduceColors();
	cube.AssignColors();
	return cube.GetColormap();
}

std::vector<int>
ColorQuantizer::QuantizeImageMasked(const std::vector<std::vector<int> >& pixels, int maxColors, int skipValue)
{
	MathUtils::Init();

	ColorCube cube(pixels, maxColors, skipValue);
	cube.ClassifyColors();
	cube.ReduceColors();
	cube.AssignColors();
	return cube.GetColormap();
}

double
ColorQuantizer::_ComputeEdgeStrength(const BitmapData& bitmap, int cx, int cy)
{
	static const int sobelX[3][3] = {
		{-1, 0, 1},
		{-2, 0, 2},
		{-1, 0, 1}
	};

	static const int sobelY[3][3] = {
		{-1, -2, -1},
		{ 0,  0,  0},
		{ 1,  2,  1}
	};

	double gx = 0, gy = 0;

	for (int dy = -1; dy <= 1; dy++) {
		for (int dx = -1; dx <= 1; dx++) {
			int x = cx + dx;
			int y = cy + dy;

			if (x < 0 || x >= bitmap.Width() || y < 0 || y >= bitmap.Height())
				continue;

			int r = bitmap.GetPixelComponent(x, y, 0);
			int g = bitmap.GetPixelComponent(x, y, 1);
			int b = bitmap.GetPixelComponent(x, y, 2);

			double luma = (r * 299 + g * 587 + b * 114) / 1000.0;

			gx += luma * sobelX[dy+1][dx+1];
			gy += luma * sobelY[dy+1][dx+1];
		}
	}

	return sqrt(gx*gx + gy*gy);
}

void
ColorQuantizer::_AdaptiveSpatialCoherence(std::vector<std::vector<int> >& indexArray,
										const BitmapData& bitmap,
										int width, int height,
										int radius, int passes)
{
	if (radius < 1 || passes < 1)
		return;

	int paletteSize = 0;
	for (int y = 1; y < height + 1; y++) {
		for (int x = 1; x < width + 1; x++) {
			if (indexArray[y][x] > paletteSize) {
				paletteSize = indexArray[y][x];
			}
		}
	}
	paletteSize++;

	std::vector<std::vector<double> > edgeStrength(height + 2);
	for (int y = 0; y < height + 2; y++) {
		edgeStrength[y].resize(width + 2, 0.0);
	}

	for (int y = 1; y < height + 1; y++) {
		for (int x = 1; x < width + 1; x++) {
			edgeStrength[y][x] = _ComputeEdgeStrength(bitmap, x - 1, y - 1);
		}
	}

	std::vector<std::vector<bool> > isColorBoundary(height + 2);
	for (int y = 0; y < height + 2; y++) {
		isColorBoundary[y].resize(width + 2, false);
	}

	for (int y = 1; y < height + 1; y++) {
		for (int x = 1; x < width + 1; x++) {
			int centerIdx = indexArray[y][x];

			bool hasDifferentNeighbor = false;
			for (int dy = -1; dy <= 1; dy++) {
				for (int dx = -1; dx <= 1; dx++) {
					if (dx == 0 && dy == 0) continue;
					int nx = x + dx;
					int ny = y + dy;
					if (nx >= 1 && nx < width + 1 && ny >= 1 && ny < height + 1) {
						if (indexArray[ny][nx] != centerIdx) {
							hasDifferentNeighbor = true;
							break;
						}
					}
				}
				if (hasDifferentNeighbor) break;
			}

			isColorBoundary[y][x] = hasDifferentNeighbor;
		}
	}

	double edgeThreshold = 20.0;
	if (paletteSize <= 8) {
		edgeThreshold = 15.0;
	} else if (paletteSize <= 16) {
		edgeThreshold = 18.0;
	}

	std::vector<std::vector<int> > temp = indexArray;

	int actualPasses = passes;
	if (paletteSize > 32) actualPasses += 1;
	if (paletteSize > 48) actualPasses += 1;

	for (int pass = 0; pass < actualPasses; pass++) {
		for (int y = 1; y < height + 1; y++) {
			for (int x = 1; x < width + 1; x++) {
				int centerIdx = indexArray[y][x];
				if (centerIdx < 0)
					continue;

				if (isColorBoundary[y][x] && edgeStrength[y][x] > edgeThreshold * 0.5)
					continue;

				if (edgeStrength[y][x] > edgeThreshold)
					continue;

				std::map<int, int> histogram;
				int totalVotes = 0;

				for (int dy = -radius; dy <= radius; dy++) {
					for (int dx = -radius; dx <= radius; dx++) {
						int ny = y + dy;
						int nx = x + dx;
						if (ny >= 1 && ny < height + 1 && nx >= 1 && nx < width + 1) {
							int idx = indexArray[ny][nx];
							if (idx >= 0) {
								histogram[idx]++;
								totalVotes++;
							}
						}
					}
				}

				if (totalVotes == 0) {
					temp[y][x] = centerIdx;
					continue;
				}

				int maxCount = 0;
				int mostFrequent = centerIdx;

				for (std::map<int, int>::const_iterator it = histogram.begin(); 
					 it != histogram.end(); ++it) {
					if (it->second > maxCount) {
						maxCount = it->second;
						mostFrequent = it->first;
					}
				}

				double consensusRatio = (double)maxCount / (double)totalVotes;
				double threshold = 0.65;

				if (edgeStrength[y][x] < edgeThreshold * 0.3)
					threshold = 0.50;
				else if (edgeStrength[y][x] < edgeThreshold * 0.6)
					threshold = 0.55;

				if (paletteSize > 32)
					threshold = 0.60;

				if (consensusRatio >= threshold) {
					temp[y][x] = mostFrequent;
				} else {
					temp[y][x] = centerIdx;
				}
			}
		}

		indexArray = temp;
	}
}

void
ColorQuantizer::_RemapIndices(std::vector<std::vector<int> >& indexArray,
							 const std::vector<int>& remapTable,
							 int width, int height)
{
	for (int y = 1; y < height + 1; y++) {
		for (int x = 1; x < width + 1; x++) {
			int idx = indexArray[y][x];
			if (idx >= 0 && idx < static_cast<int>(remapTable.size())) {
				indexArray[y][x] = remapTable[idx];
			}
		}
	}
}

void
ColorQuantizer::_MergeSimilarPaletteColors(std::vector<std::vector<unsigned char> >& palette,
										  std::vector<std::vector<int> >& indexArray,
										  int width, int height,
										  int threshold)
{
	if (palette.size() < 2)
		return;

	int originalPaletteSize = palette.size();

	if (originalPaletteSize <= 12)
		return;

	bool hasTransparentAtZero = (!palette.empty() && MathUtils::IsTransparent(palette[0][3]));

	std::vector<int> usage(palette.size(), 0);
	for (int y = 1; y < height + 1; y++) {
		for (int x = 1; x < width + 1; x++) {
			int idx = indexArray[y][x];
			if (idx >= 0 && idx < static_cast<int>(palette.size())) {
				usage[idx]++;
			}
		}
	}

	std::vector<int> remapTable(palette.size());
	for (int i = 0; i < static_cast<int>(palette.size()); i++) {
		remapTable[i] = i;
	}

	std::vector<bool> merged(palette.size(), false);

	if (hasTransparentAtZero) {
		merged[0] = true;
	}

	double adaptiveThreshold = MathUtils::AdaptiveThreshold(originalPaletteSize, threshold);

	int startIdx = hasTransparentAtZero ? 1 : 0;

	for (int i = startIdx; i < static_cast<int>(palette.size()); i++) {
		if (merged[i])
			continue;

		if (MathUtils::IsTransparent(palette[i][3]))
			continue;

		for (int j = i + 1; j < static_cast<int>(palette.size()); j++) {
			if (merged[j])
				continue;

			if (MathUtils::IsTransparent(palette[j][3]))
				continue;

			double dist = MathUtils::PerceptualColorDistanceForMerge(
				palette[i][0], palette[i][1], palette[i][2], palette[i][3],
				palette[j][0], palette[j][1], palette[j][2], palette[j][3]
			);

			if (dist < adaptiveThreshold) {
				merged[j] = true;
				remapTable[j] = i;
			}
		}
	}

	for (int i = 0; i < static_cast<int>(remapTable.size()); i++) {
		int target = i;
		while (remapTable[target] != target) {
			target = remapTable[target];
		}
		remapTable[i] = target;
	}

	_RemapIndices(indexArray, remapTable, width, height);

	std::vector<std::vector<unsigned char> > newPalette;
	std::vector<int> finalRemap(palette.size(), -1);

	for (int i = 0; i < static_cast<int>(palette.size()); i++) {
		if (!merged[i] || (hasTransparentAtZero && i == 0)) {
			finalRemap[i] = newPalette.size();
			newPalette.push_back(palette[i]);
		}
	}

	for (int i = 0; i < static_cast<int>(palette.size()); i++) {
		if (merged[i] && !(hasTransparentAtZero && i == 0)) {
			finalRemap[i] = finalRemap[remapTable[i]];
		}
	}

	_RemapIndices(indexArray, finalRemap, width, height);

	palette = newPalette;
}

IndexedBitmap
ColorQuantizer::QuantizeColors(const BitmapData& bitmap,
							const std::vector<std::vector<unsigned char> >& palette,
							const TracingOptions& options)
{
	int paletteSize = static_cast<int>(palette.size());

	std::vector<std::vector<int> > indexArray(bitmap.Height() + 2);
	for (int j = 0; j < bitmap.Height() + 2; j++) {
		indexArray[j].resize(bitmap.Width() + 2, -1);
	}

	std::vector<std::vector<unsigned char> > workingPalette = palette;

	bool hasTransparentColor = false;
	int transparentIndex = -1;
	if (!palette.empty() && MathUtils::IsTransparent(palette[0][3])) {
		hasTransparentColor = true;
		transparentIndex = 0;
	}

	for (int y = 0; y < bitmap.Height(); y++) {
		for (int x = 0; x < bitmap.Width(); x++) {
			unsigned char red   = bitmap.GetPixelComponent(x, y, 0);
			unsigned char green = bitmap.GetPixelComponent(x, y, 1);
			unsigned char blue  = bitmap.GetPixelComponent(x, y, 2);
			unsigned char alpha = bitmap.GetPixelComponent(x, y, 3);

			if (MathUtils::IsTransparent(alpha)) {
				if (hasTransparentColor) {
					indexArray[y + 1][x + 1] = transparentIndex;
				} else {
					indexArray[y + 1][x + 1] = -1;
				}
				continue;
			}

			double closestDistance = MathUtils::MAX_DISTANCE;
			int closestIndex = hasTransparentColor ? 1 : 0;

			bool foundValidColor = false;

			for (int k = (hasTransparentColor ? 1 : 0); k < static_cast<int>(workingPalette.size()); k++) {
				unsigned char paletteAlpha = workingPalette[k][3];

				if (MathUtils::IsTransparent(paletteAlpha))
					continue;

				double distance = MathUtils::PerceptualColorDistance(
					red, green, blue, alpha,
					workingPalette[k][0], workingPalette[k][1], workingPalette[k][2], workingPalette[k][3]
				);

				if (distance < closestDistance) {
					closestDistance = distance;
					closestIndex = k;
					foundValidColor = true;
				}
			}

			if (foundValidColor) {
				indexArray[y + 1][x + 1] = closestIndex;
			} else {
				if (hasTransparentColor) {
					indexArray[y + 1][x + 1] = transparentIndex;
				} else {
					indexArray[y + 1][x + 1] = -1;
				}
			}
		}
	}

	if (paletteSize > 16) {
		double baseMergeThreshold = 18.0;
		double adaptiveMerge = MathUtils::AdaptiveThreshold(paletteSize, baseMergeThreshold);
		
		_MergeSimilarPaletteColors(workingPalette, indexArray, 
								  bitmap.Width(), 
								  bitmap.Height(),
								  (int)adaptiveMerge);
	}

	if (options.fSpatialCoherence && paletteSize > 12 && paletteSize <= 24) {
		_AdaptiveSpatialCoherence(indexArray, 
							bitmap,
							bitmap.Width(), 
							bitmap.Height(),
							options.fSpatialCoherenceRadius,
							options.fSpatialCoherencePasses);
	}

	return IndexedBitmap(indexArray, workingPalette);
}
