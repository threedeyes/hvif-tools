/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef BACKGROUND_REMOVER_H
#define BACKGROUND_REMOVER_H

#include <vector>
#include <map>

#include "BitmapData.h"

enum BackgroundDetectionMethod {
	SIMPLE = 0,
	AUTO = 1
};

struct ColorKey {
	unsigned char r, g, b, a;

	bool operator==(const ColorKey& other) const {
		return r == other.r && g == other.g && b == other.b && a == other.a;
	}

	bool operator<(const ColorKey& other) const {
		if (r != other.r) return r < other.r;
		if (g != other.g) return g < other.g;
		if (b != other.b) return b < other.b;
		return a < other.a;
	}
};

class BackgroundRemover {
public:
							BackgroundRemover();
							~BackgroundRemover();

	BitmapData				RemoveBackground(const BitmapData& bitmap,
											BackgroundDetectionMethod method,
											int tolerance);

	void					SetColorTolerance(int tolerance) { fColorTolerance = tolerance; }
	void					SetMinBackgroundRatio(double ratio) { fMinBackgroundRatio = ratio; }

private:
	ColorKey				_DetectBackgroundSimple(const BitmapData& bitmap, int tolerance);
	ColorKey				_DetectBackgroundAuto(const BitmapData& bitmap, int tolerance);

	bool					_ColorsMatch(const ColorKey& c1, const ColorKey& c2, int tolerance) const;
	int						_CalculateColorDistance(const ColorKey& c1, const ColorKey& c2) const;

	int						_FloodFillCount(const BitmapData& bitmap, int startX, int startY,
											const ColorKey& targetColor, int tolerance,
											std::vector<std::vector<bool> >& visited) const;

	double					_CalculateEdgeScore(const BitmapData& bitmap, const ColorKey& color, int tolerance) const;
	double					_CalculateConnectivityScore(const BitmapData& bitmap, const ColorKey& color, int tolerance) const;

	BitmapData				_ApplyBackgroundRemoval(const BitmapData& bitmap, const ColorKey& backgroundColor, int tolerance) const;

	ColorKey				_GetPixelColor(const BitmapData& bitmap, int x, int y) const;
	void					_FloodFillMark(const BitmapData& bitmap, int startX, int startY,
										const ColorKey& targetColor, int tolerance,
										std::vector<std::vector<bool> >& visited,
										std::vector<std::vector<bool> >& toRemove) const;

	int						fColorTolerance;
	double					fMinBackgroundRatio;
};

#endif
