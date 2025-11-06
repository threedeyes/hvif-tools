/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <algorithm>
#include <queue>
#include <cmath>
#include <cstdlib>

#include "BackgroundRemover.h"

BackgroundRemover::BackgroundRemover()
	: fColorTolerance(10)
	, fMinBackgroundRatio(0.3)
{
}

BackgroundRemover::~BackgroundRemover()
{
}

BitmapData
BackgroundRemover::RemoveBackground(const BitmapData& bitmap, 
									BackgroundDetectionMethod method, 
									int tolerance)
{
	if (!bitmap.IsValid())
		return bitmap;

	ColorKey backgroundColor;

	switch (method) {
		case SIMPLE:
			backgroundColor = _DetectBackgroundSimple(bitmap, tolerance);
			break;
		case AUTO:
		default:
			backgroundColor = _DetectBackgroundAuto(bitmap, tolerance);
			break;
	}

	return _ApplyBackgroundRemoval(bitmap, backgroundColor, tolerance);
}

ColorKey
BackgroundRemover::_DetectBackgroundSimple(const BitmapData& bitmap, int tolerance)
{
	int width = bitmap.Width();
	int height = bitmap.Height();
	std::vector<std::vector<bool> > visited(height, std::vector<bool>(width, false));

	struct FloodResult {
		ColorKey color;
		int area;
	};

	std::vector<FloodResult> results;

	std::vector<std::pair<int, int> > startPoints;
	startPoints.push_back(std::make_pair(0, 0));
	startPoints.push_back(std::make_pair(width-1, 0));
	startPoints.push_back(std::make_pair(0, height-1));
	startPoints.push_back(std::make_pair(width-1, height-1));

	for (std::vector<std::pair<int, int> >::iterator it = startPoints.begin(); it != startPoints.end(); ++it) {
		int x = it->first;
		int y = it->second;

		if (!visited[y][x]) {
			ColorKey startColor = _GetPixelColor(bitmap, x, y);
			int area = _FloodFillCount(bitmap, x, y, startColor, tolerance, visited);

			if (area > 0) {
				FloodResult result;
				result.color = startColor;
				result.area = area;
				results.push_back(result);
			}
		}
	}

	int maxArea = 0;
	ColorKey bestColor = {0, 0, 0, 255};

	for (std::vector<FloodResult>::const_iterator it = results.begin(); it != results.end(); ++it) {
		if (it->area > maxArea) {
			maxArea = it->area;
			bestColor = it->color;
		}
	}

	return bestColor;
}

ColorKey
BackgroundRemover::_DetectBackgroundAuto(const BitmapData& bitmap, int tolerance)
{
	ColorKey floodColor = _DetectBackgroundSimple(bitmap, tolerance);

	double floodScore = _CalculateEdgeScore(bitmap, floodColor, tolerance) * 2.0 +
						_CalculateConnectivityScore(bitmap, floodColor, tolerance);

	int width = bitmap.Width();
	int height = bitmap.Height();
	int totalPixels = width * height;

	int edgePixelCount = (width * 2) + ((height - 2) * 2);
	std::vector<ColorKey> edgeColors;
	edgeColors.reserve(edgePixelCount);

	for (int x = 0; x < width; x++) {
		edgeColors.push_back(_GetPixelColor(bitmap, x, 0));
		edgeColors.push_back(_GetPixelColor(bitmap, x, height-1));
	}

	for (int y = 1; y < height-1; y++) {
		edgeColors.push_back(_GetPixelColor(bitmap, 0, y));
		edgeColors.push_back(_GetPixelColor(bitmap, width-1, y));
	}

	std::map<ColorKey, int> edgeHistogram;
	for (std::vector<ColorKey>::const_iterator it = edgeColors.begin(); it != edgeColors.end(); ++it) {
		bool found = false;
		for (std::map<ColorKey, int>::iterator mapIt = edgeHistogram.begin(); mapIt != edgeHistogram.end(); ++mapIt) {
			if (_ColorsMatch(*it, mapIt->first, tolerance)) {
				mapIt->second++;
				found = true;
				break;
			}
		}
		if (!found)
			edgeHistogram[*it] = 1;
	}

	ColorKey edgeColor = {0, 0, 0, 255};
	int maxEdgeCount = 0;

	for (std::map<ColorKey, int>::const_iterator it = edgeHistogram.begin(); it != edgeHistogram.end(); ++it) {
		if (it->second > maxEdgeCount) {
			maxEdgeCount = it->second;
			edgeColor = it->first;
		}
	}

	double edgeScore = _CalculateEdgeScore(bitmap, edgeColor, tolerance) * 2.0 +
					   _CalculateConnectivityScore(bitmap, edgeColor, tolerance);

	if (_ColorsMatch(floodColor, edgeColor, tolerance))
		return floodColor;

	return (edgeScore > floodScore) ? edgeColor : floodColor;
}

bool
BackgroundRemover::_ColorsMatch(const ColorKey& c1, const ColorKey& c2, int tolerance) const
{
	return _CalculateColorDistance(c1, c2) <= tolerance;
}

int
BackgroundRemover::_CalculateColorDistance(const ColorKey& c1, const ColorKey& c2) const
{
	int dr = abs(static_cast<int>(c1.r) - static_cast<int>(c2.r));
	int dg = abs(static_cast<int>(c1.g) - static_cast<int>(c2.g));
	int db = abs(static_cast<int>(c1.b) - static_cast<int>(c2.b));
	int da = abs(static_cast<int>(c1.a) - static_cast<int>(c2.a));

	return dr + dg + db + da;
}

int
BackgroundRemover::_FloodFillCount(const BitmapData& bitmap, int startX, int startY,
								const ColorKey& targetColor, int tolerance,
								std::vector<std::vector<bool> >& visited) const
{
	int width = bitmap.Width();
	int height = bitmap.Height();
	int count = 0;

	std::queue<std::pair<int, int> > queue;
	queue.push(std::make_pair(startX, startY));

	while (!queue.empty()) {
		std::pair<int, int> point = queue.front();
		queue.pop();
		int x = point.first;
		int y = point.second;

		if (x < 0 || x >= width || y < 0 || y >= height || visited[y][x])
			continue;

		ColorKey currentColor = _GetPixelColor(bitmap, x, y);

		if (!_ColorsMatch(currentColor, targetColor, tolerance))
			continue;

		visited[y][x] = true;
		count++;

		queue.push(std::make_pair(x+1, y));
		queue.push(std::make_pair(x-1, y));
		queue.push(std::make_pair(x, y+1));
		queue.push(std::make_pair(x, y-1));
	}

	return count;
}

double
BackgroundRemover::_CalculateEdgeScore(const BitmapData& bitmap, const ColorKey& color, int tolerance) const
{
	int width = bitmap.Width();
	int height = bitmap.Height();
	int matchCount = 0;
	int totalCount = 0;

	for (int x = 0; x < width; x++) {
		totalCount += 2;
		if (_ColorsMatch(_GetPixelColor(bitmap, x, 0), color, tolerance))
			matchCount++;
		if (_ColorsMatch(_GetPixelColor(bitmap, x, height-1), color, tolerance))
			matchCount++;
	}

	for (int y = 1; y < height-1; y++) {
		totalCount += 2;
		if (_ColorsMatch(_GetPixelColor(bitmap, 0, y), color, tolerance))
			matchCount++;
		if (_ColorsMatch(_GetPixelColor(bitmap, width-1, y), color, tolerance))
			matchCount++;
	}

	return totalCount > 0 ? static_cast<double>(matchCount) / totalCount : 0.0;
}

double
BackgroundRemover::_CalculateConnectivityScore(const BitmapData& bitmap, const ColorKey& color, int tolerance) const
{
	int width = bitmap.Width();
	int height = bitmap.Height();
	std::vector<std::vector<bool> > visited(height, std::vector<bool>(width, false));

	int maxArea = 0;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (!visited[y][x]) {
				ColorKey currentColor = _GetPixelColor(bitmap, x, y);

				if (_ColorsMatch(currentColor, color, tolerance)) {
					int area = _FloodFillCount(bitmap, x, y, color, tolerance, visited);
					maxArea = std::max(maxArea, area);
				}
			}
		}
	}

	int totalPixels = width * height;
	return static_cast<double>(maxArea) / totalPixels;
}

BitmapData
BackgroundRemover::_ApplyBackgroundRemoval(const BitmapData& bitmap, const ColorKey& backgroundColor, int tolerance) const
{
	int width = bitmap.Width();
	int height = bitmap.Height();
	std::vector<unsigned char> newData = bitmap.Data();

	std::vector<std::vector<bool> > visited(height, std::vector<bool>(width, false));
	std::vector<std::vector<bool> > toRemove(height, std::vector<bool>(width, false));

	for (int x = 0; x < width; x++) {
		if (!visited[0][x]) {
			ColorKey color = _GetPixelColor(bitmap, x, 0);
			if (_ColorsMatch(color, backgroundColor, tolerance)) {
				_FloodFillMark(bitmap, x, 0, backgroundColor, tolerance, visited, toRemove);
			}
		}

		if (!visited[height-1][x]) {
			ColorKey color = _GetPixelColor(bitmap, x, height-1);
			if (_ColorsMatch(color, backgroundColor, tolerance)) {
				_FloodFillMark(bitmap, x, height-1, backgroundColor, tolerance, visited, toRemove);
			}
		}
	}

	for (int y = 1; y < height-1; y++) {
		if (!visited[y][0]) {
			ColorKey color = _GetPixelColor(bitmap, 0, y);
			if (_ColorsMatch(color, backgroundColor, tolerance)) {
				_FloodFillMark(bitmap, 0, y, backgroundColor, tolerance, visited, toRemove);
			}
		}

		if (!visited[y][width-1]) {
			ColorKey color = _GetPixelColor(bitmap, width-1, y);
			if (_ColorsMatch(color, backgroundColor, tolerance)) {
				_FloodFillMark(bitmap, width-1, y, backgroundColor, tolerance, visited, toRemove);
			}
		}
	}

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (toRemove[y][x]) {
				int index = (y * width + x) * 4;
				newData[index + 3] = 0;
			}
		}
	}

	return BitmapData(width, height, newData);
}

ColorKey
BackgroundRemover::_GetPixelColor(const BitmapData& bitmap, int x, int y) const
{
	ColorKey color = {
		bitmap.GetPixelComponent(x, y, 0),
		bitmap.GetPixelComponent(x, y, 1),
		bitmap.GetPixelComponent(x, y, 2),
		bitmap.GetPixelComponent(x, y, 3)
	};
	return color;
}

void
BackgroundRemover::_FloodFillMark(const BitmapData& bitmap, int startX, int startY,
								const ColorKey& targetColor, int tolerance,
								std::vector<std::vector<bool> >& visited,
								std::vector<std::vector<bool> >& toRemove) const
{
	int width = bitmap.Width();
	int height = bitmap.Height();

	std::queue<std::pair<int, int> > queue;
	queue.push(std::make_pair(startX, startY));

	while (!queue.empty()) {
		std::pair<int, int> point = queue.front();
		queue.pop();
		int x = point.first;
		int y = point.second;

		if (x < 0 || x >= width || y < 0 || y >= height || visited[y][x])
			continue;

		ColorKey currentColor = _GetPixelColor(bitmap, x, y);

		if (!_ColorsMatch(currentColor, targetColor, tolerance))
			continue;

		visited[y][x] = true;
		toRemove[y][x] = true;

		queue.push(std::make_pair(x+1, y));
		queue.push(std::make_pair(x-1, y));
		queue.push(std::make_pair(x, y+1));
		queue.push(std::make_pair(x, y-1));
	}
}
