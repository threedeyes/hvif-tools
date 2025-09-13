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
	, fEdgeWeight(2.0)
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
		case EDGE_ANALYSIS:
			backgroundColor = DetectBackgroundByEdgeAnalysis(bitmap, tolerance);
			break;
		case FLOOD_FILL:
			backgroundColor = DetectBackgroundByFloodFill(bitmap, tolerance);
			break;
		case DOMINANT_COLOR:
			backgroundColor = DetectBackgroundByDominantColor(bitmap);
			break;
		case CLUSTERING:
			backgroundColor = DetectBackgroundByClustering(bitmap, tolerance);
			break;
		case COMBINED:
		default:
			backgroundColor = DetectBackgroundCombined(bitmap, tolerance);
			break;
	}

	return ApplyBackgroundRemoval(bitmap, backgroundColor, tolerance);
}

ColorKey
BackgroundRemover::DetectBackgroundByEdgeAnalysis(const BitmapData& bitmap, int tolerance)
{
	std::vector<ColorKey> edgeColors = GetEdgeColors(bitmap);
	std::map<ColorKey, int> colorCount;

	for (std::vector<ColorKey>::const_iterator it = edgeColors.begin(); it != edgeColors.end(); ++it) {
		const ColorKey& color = *it;
		bool found = false;
		for (std::map<ColorKey, int>::iterator mapIt = colorCount.begin(); mapIt != colorCount.end(); ++mapIt) {
			if (ColorsMatch(color, mapIt->first, tolerance)) {
				mapIt->second++;
				found = true;
				break;
			}
		}
		if (!found)
			colorCount[color] = 1;
	}

	ColorKey mostFrequent = {0, 0, 0, 255};
	int maxCount = 0;

	for (std::map<ColorKey, int>::const_iterator it = colorCount.begin(); it != colorCount.end(); ++it) {
		if (it->second > maxCount) {
			maxCount = it->second;
			mostFrequent = it->first;
		}
	}

	return mostFrequent;
}

ColorKey
BackgroundRemover::DetectBackgroundByFloodFill(const BitmapData& bitmap, int tolerance)
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
			ColorKey startColor = {
				bitmap.GetPixelComponent(x, y, 0),
				bitmap.GetPixelComponent(x, y, 1),
				bitmap.GetPixelComponent(x, y, 2),
				bitmap.GetPixelComponent(x, y, 3)
			};

			int area = FloodFillCount(bitmap, x, y, startColor, tolerance, visited);

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
BackgroundRemover::DetectBackgroundByDominantColor(const BitmapData& bitmap)
{
	std::map<ColorKey, int> histogram = GetColorHistogram(bitmap);

	ColorKey dominantColor = {0, 0, 0, 255};
	int maxCount = 0;

	for (std::map<ColorKey, int>::const_iterator it = histogram.begin(); it != histogram.end(); ++it) {
		if (it->second > maxCount) {
			maxCount = it->second;
			dominantColor = it->first;
		}
	}

	return dominantColor;
}

ColorKey
BackgroundRemover::DetectBackgroundByClustering(const BitmapData& bitmap, int tolerance)
{
	std::map<ColorKey, int> histogram = GetColorHistogram(bitmap);
	std::vector<BackgroundCandidate> clusters = ClusterSimilarColors(histogram, tolerance);

	ColorKey bestColor = {0, 0, 0, 255};
	double bestScore = -1.0;

	for (std::vector<BackgroundCandidate>::iterator it = clusters.begin(); it != clusters.end(); ++it) {
		it->edgeScore = CalculateEdgeScore(bitmap, it->color, tolerance);
		it->connectivityScore = CalculateConnectivityScore(bitmap, it->color, tolerance);

		double score = (static_cast<double>(it->frequency) / (bitmap.Width() * bitmap.Height())) * 0.3 +
						it->edgeScore * 0.4 +
						it->connectivityScore * 0.3;

		if (score > bestScore) {
			bestScore = score;
			bestColor = it->color;
		}
	}

	return bestColor;
}

ColorKey
BackgroundRemover::DetectBackgroundCombined(const BitmapData& bitmap, int tolerance)
{
	std::vector<BackgroundCandidate> candidates;

	ColorKey edgeColor = DetectBackgroundByEdgeAnalysis(bitmap, tolerance);
	BackgroundCandidate edgeCandidate;
	edgeCandidate.color = edgeColor;
	edgeCandidate.edgeScore = CalculateEdgeScore(bitmap, edgeColor, tolerance);
	edgeCandidate.connectivityScore = CalculateConnectivityScore(bitmap, edgeColor, tolerance);
	candidates.push_back(edgeCandidate);

	ColorKey floodColor = DetectBackgroundByFloodFill(bitmap, tolerance);
	if (!ColorsMatch(floodColor, edgeColor, tolerance)) {
		BackgroundCandidate floodCandidate;
		floodCandidate.color = floodColor;
		floodCandidate.edgeScore = CalculateEdgeScore(bitmap, floodColor, tolerance);
		floodCandidate.connectivityScore = CalculateConnectivityScore(bitmap, floodColor, tolerance);
		candidates.push_back(floodCandidate);
	}

	ColorKey dominantColor = DetectBackgroundByDominantColor(bitmap);
	bool alreadyExists = false;
	for (std::vector<BackgroundCandidate>::const_iterator it = candidates.begin(); it != candidates.end(); ++it) {
		if (ColorsMatch(dominantColor, it->color, tolerance)) {
			alreadyExists = true;
			break;
		}
	}

	if (!alreadyExists) {
		BackgroundCandidate dominantCandidate;
		dominantCandidate.color = dominantColor;
		dominantCandidate.edgeScore = CalculateEdgeScore(bitmap, dominantColor, tolerance);
		dominantCandidate.connectivityScore = CalculateConnectivityScore(bitmap, dominantColor, tolerance);
		candidates.push_back(dominantCandidate);
	}

	double bestScore = -1.0;
	ColorKey bestColor = {0, 0, 0, 255};

	for (std::vector<BackgroundCandidate>::const_iterator it = candidates.begin(); it != candidates.end(); ++it) {
		double score = it->edgeScore * fEdgeWeight + it->connectivityScore;
		if (score > bestScore) {
			bestScore = score;
			bestColor = it->color;
		}
	}

	return bestColor;
}

bool
BackgroundRemover::ColorsMatch(const ColorKey& c1, const ColorKey& c2, int tolerance) const
{
	return CalculateColorDistance(c1, c2) <= tolerance;
}

int
BackgroundRemover::CalculateColorDistance(const ColorKey& c1, const ColorKey& c2) const
{
	int dr = abs(static_cast<int>(c1.r) - static_cast<int>(c2.r));
	int dg = abs(static_cast<int>(c1.g) - static_cast<int>(c2.g));
	int db = abs(static_cast<int>(c1.b) - static_cast<int>(c2.b));
	int da = abs(static_cast<int>(c1.a) - static_cast<int>(c2.a));

	return dr + dg + db + da;
}

std::vector<ColorKey>
BackgroundRemover::GetEdgeColors(const BitmapData& bitmap) const
{
	std::vector<ColorKey> edgeColors;
	int width = bitmap.Width();
	int height = bitmap.Height();

	for (int x = 0; x < width; x++) {
		ColorKey topColor = {
			bitmap.GetPixelComponent(x, 0, 0),
			bitmap.GetPixelComponent(x, 0, 1),
			bitmap.GetPixelComponent(x, 0, 2),
			bitmap.GetPixelComponent(x, 0, 3)
		};
		edgeColors.push_back(topColor);

		ColorKey bottomColor = {
			bitmap.GetPixelComponent(x, height-1, 0),
			bitmap.GetPixelComponent(x, height-1, 1),
			bitmap.GetPixelComponent(x, height-1, 2),
			bitmap.GetPixelComponent(x, height-1, 3)
		};
		edgeColors.push_back(bottomColor);
	}

	for (int y = 1; y < height-1; y++) {
		ColorKey leftColor = {
			bitmap.GetPixelComponent(0, y, 0),
			bitmap.GetPixelComponent(0, y, 1),
			bitmap.GetPixelComponent(0, y, 2),
			bitmap.GetPixelComponent(0, y, 3)
		};
		edgeColors.push_back(leftColor);

		ColorKey rightColor = {
			bitmap.GetPixelComponent(width-1, y, 0),
			bitmap.GetPixelComponent(width-1, y, 1),
			bitmap.GetPixelComponent(width-1, y, 2),
			bitmap.GetPixelComponent(width-1, y, 3)
		};
		edgeColors.push_back(rightColor);
	}

	return edgeColors;
}

int
BackgroundRemover::FloodFillCount(const BitmapData& bitmap, int startX, int startY, 
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

		ColorKey currentColor = {
			bitmap.GetPixelComponent(x, y, 0),
			bitmap.GetPixelComponent(x, y, 1),
			bitmap.GetPixelComponent(x, y, 2),
			bitmap.GetPixelComponent(x, y, 3)
		};

		if (!ColorsMatch(currentColor, targetColor, tolerance))
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
BackgroundRemover::CalculateEdgeScore(const BitmapData& bitmap, const ColorKey& color, int tolerance) const
{
	std::vector<ColorKey> edgeColors = GetEdgeColors(bitmap);
	int matchCount = 0;

	for (std::vector<ColorKey>::const_iterator it = edgeColors.begin(); it != edgeColors.end(); ++it) {
		if (ColorsMatch(color, *it, tolerance)) {
			matchCount++;
		}
	}

	return static_cast<double>(matchCount) / edgeColors.size();
}

double
BackgroundRemover::CalculateConnectivityScore(const BitmapData& bitmap, const ColorKey& color, int tolerance) const
{
	int width = bitmap.Width();
	int height = bitmap.Height();
	std::vector<std::vector<bool> > visited(height, std::vector<bool>(width, false));

	int maxArea = 0;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (!visited[y][x]) {
				ColorKey currentColor = {
					bitmap.GetPixelComponent(x, y, 0),
					bitmap.GetPixelComponent(x, y, 1),
					bitmap.GetPixelComponent(x, y, 2),
					bitmap.GetPixelComponent(x, y, 3)
				};

				if (ColorsMatch(currentColor, color, tolerance)) {
					int area = FloodFillCount(bitmap, x, y, color, tolerance, visited);
					maxArea = std::max(maxArea, area);
				}
			}
		}
	}

	int totalPixels = width * height;
	return static_cast<double>(maxArea) / totalPixels;
}

BitmapData
BackgroundRemover::ApplyBackgroundRemoval(const BitmapData& bitmap, const ColorKey& backgroundColor, int tolerance) const
{
	int width = bitmap.Width();
	int height = bitmap.Height();
	std::vector<unsigned char> newData = bitmap.Data();

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			ColorKey currentColor = {
				bitmap.GetPixelComponent(x, y, 0),
				bitmap.GetPixelComponent(x, y, 1),
				bitmap.GetPixelComponent(x, y, 2),
				bitmap.GetPixelComponent(x, y, 3)
			};

			if (ColorsMatch(currentColor, backgroundColor, tolerance)) {				
				int index = (y * width + x) * 4;
				newData[index + 3] = 0; // Set transparent pixel
			}
		}
	}

	return BitmapData(width, height, newData);
}

std::map<ColorKey, int>
BackgroundRemover::GetColorHistogram(const BitmapData& bitmap) const
{
	std::map<ColorKey, int> histogram;

	for (int y = 0; y < bitmap.Height(); y++) {
		for (int x = 0; x < bitmap.Width(); x++) {
			ColorKey color = {
				bitmap.GetPixelComponent(x, y, 0),
				bitmap.GetPixelComponent(x, y, 1),
				bitmap.GetPixelComponent(x, y, 2),
				bitmap.GetPixelComponent(x, y, 3)
			};
			histogram[color]++;
		}
	}

	return histogram;
}

std::vector<BackgroundCandidate>
BackgroundRemover::ClusterSimilarColors(const std::map<ColorKey, int>& histogram, int tolerance) const
{
	std::vector<BackgroundCandidate> clusters;
	std::vector<std::pair<ColorKey, int> > colorList;

	for (std::map<ColorKey, int>::const_iterator it = histogram.begin(); it != histogram.end(); ++it) {
		colorList.push_back(*it);
	}

	ColorFrequencyComparator comparator;
	std::sort(colorList.begin(), colorList.end(), comparator);

	std::vector<bool> processed(colorList.size(), false);

	for (int i = 0; i < static_cast<int>(colorList.size()); i++) {
		if (processed[i]) continue;

		BackgroundCandidate cluster;
		cluster.color = colorList[i].first;
		cluster.frequency = colorList[i].second;

		for (int j = i + 1; j < static_cast<int>(colorList.size()); j++) {
			if (!processed[j] && ColorsMatch(colorList[i].first, colorList[j].first, tolerance)) {
				cluster.frequency += colorList[j].second;
				processed[j] = true;
			}
		}

		processed[i] = true;
		clusters.push_back(cluster);

		if (clusters.size() >= 10) break;
	}

	return clusters;
}
