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
	EDGE_ANALYSIS,
	FLOOD_FILL,
	DOMINANT_COLOR,
	CLUSTERING,
	COMBINED
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

struct BackgroundCandidate {
	ColorKey color;
	int frequency;
	double edgeScore;
	double connectivityScore;

	BackgroundCandidate() : frequency(0), edgeScore(0.0), connectivityScore(0.0) {}
};

struct ColorFrequencyComparator {
	bool operator()(const std::pair<ColorKey, int>& a, const std::pair<ColorKey, int>& b) const {
		return a.second > b.second;
	}
};

class BackgroundRemover {
public:
	BackgroundRemover();
	~BackgroundRemover();

	BitmapData RemoveBackground(const BitmapData& bitmap,
								BackgroundDetectionMethod method,
								int tolerance);

	ColorKey DetectBackgroundByEdgeAnalysis(const BitmapData& bitmap, int tolerance);
	ColorKey DetectBackgroundByFloodFill(const BitmapData& bitmap, int tolerance);
	ColorKey DetectBackgroundByDominantColor(const BitmapData& bitmap);
	ColorKey DetectBackgroundByClustering(const BitmapData& bitmap, int tolerance);
	ColorKey DetectBackgroundCombined(const BitmapData& bitmap, int tolerance);

	void SetColorTolerance(int tolerance) { fColorTolerance = tolerance; }
	void SetMinBackgroundRatio(double ratio) { fMinBackgroundRatio = ratio; }
	void SetEdgeWeight(double weight) { fEdgeWeight = weight; }

private:
	bool ColorsMatch(const ColorKey& c1, const ColorKey& c2, int tolerance) const;
	int CalculateColorDistance(const ColorKey& c1, const ColorKey& c2) const;

	std::vector<ColorKey> GetEdgeColors(const BitmapData& bitmap) const;
	std::map<ColorKey, int> GetColorHistogram(const BitmapData& bitmap) const;

	int FloodFillCount(const BitmapData& bitmap, int startX, int startY,
					const ColorKey& targetColor, int tolerance,
					std::vector<std::vector<bool> >& visited) const;

	double CalculateEdgeScore(const BitmapData& bitmap, const ColorKey& color, int tolerance) const;
	double CalculateConnectivityScore(const BitmapData& bitmap, const ColorKey& color, int tolerance) const;

	BitmapData ApplyBackgroundRemoval(const BitmapData& bitmap, const ColorKey& backgroundColor, int tolerance) const;

	std::vector<BackgroundCandidate> ClusterSimilarColors(const std::map<ColorKey, int>& histogram, int tolerance) const;

	ColorKey GetPixelColor(const BitmapData& bitmap, int x, int y) const;
	void FloodFillMark(const BitmapData& bitmap, int startX, int startY,
					const ColorKey& targetColor, int tolerance,
					std::vector<std::vector<bool> >& visited,
					std::vector<std::vector<bool> >& toRemove) const;

	int fColorTolerance;
	double fMinBackgroundRatio;
	double fEdgeWeight;
};

#endif // BACKGROUND_REMOVER_H
