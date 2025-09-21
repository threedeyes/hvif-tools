/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <cmath>

#include "PathScanner.h"

const unsigned char PathScanner::kPathScanDirectionLookup[16] = {
	0, 0, 3, 0, 1, 0, 3, 0, 0, 3, 3, 1, 0, 3, 0, 0
};

const bool PathScanner::kPathScanHolePathLookup[16] = {
	false, false, false, false, false, false, false, true,
	false, false, false, true, false, true, true, false
};

const char PathScanner::kPathScanCombinedLookup[16][4][4] = {
	{{-1,-1,-1,-1}, {-1,-1,-1,-1}, {-1,-1,-1,-1}, {-1,-1,-1,-1}}, // 0
	{{ 0, 1, 0,-1}, {-1,-1,-1,-1}, {-1,-1,-1,-1}, { 0, 2,-1, 0}}, // 1
	{{-1,-1,-1,-1}, {-1,-1,-1,-1}, { 0, 1, 0,-1}, { 0, 0, 1, 0}}, // 2
	{{ 0, 0, 1, 0}, {-1,-1,-1,-1}, { 0, 2,-1, 0}, {-1,-1,-1,-1}}, // 3
	{{-1,-1,-1,-1}, { 0, 0, 1, 0}, { 0, 3, 0, 1}, {-1,-1,-1,-1}}, // 4
	{{13, 3, 0, 1}, {13, 2,-1, 0}, { 7, 1, 0,-1}, { 7, 0, 1, 0}}, // 5
	{{-1,-1,-1,-1}, { 0, 1, 0,-1}, {-1,-1,-1,-1}, { 0, 3, 0, 1}}, // 6
	{{ 0, 3, 0, 1}, { 0, 2,-1, 0}, {-1,-1,-1,-1}, {-1,-1,-1,-1}}, // 7
	{{ 0, 3, 0, 1}, { 0, 2,-1, 0}, {-1,-1,-1,-1}, {-1,-1,-1,-1}}, // 8
	{{-1,-1,-1,-1}, { 0, 1, 0,-1}, {-1,-1,-1,-1}, { 0, 3, 0, 1}}, // 9
	{{11, 1, 0,-1}, {14, 0, 1, 0}, {14, 3, 0, 1}, {11, 2,-1, 0}}, // 10
	{{-1,-1,-1,-1}, { 0, 0, 1, 0}, { 0, 3, 0, 1}, {-1,-1,-1,-1}}, // 11
	{{ 0, 0, 1, 0}, {-1,-1,-1,-1}, { 0, 2,-1, 0}, {-1,-1,-1,-1}}, // 12
	{{-1,-1,-1,-1}, {-1,-1,-1,-1}, { 0, 1, 0,-1}, { 0, 0, 1, 0}}, // 13
	{{ 0, 1, 0,-1}, {-1,-1,-1,-1}, {-1,-1,-1,-1}, { 0, 2,-1, 0}}, // 14
	{{-1,-1,-1,-1}, {-1,-1,-1,-1}, {-1,-1,-1,-1}, {-1,-1,-1,-1}}  // 15
};

PathScanner::PathScanner()
{
}

PathScanner::~PathScanner()
{
}

std::vector<std::vector<std::vector<int>>>
PathScanner::CreateLayers(const IndexedBitmap& indexedBitmap)
{
	int value = 0;
	int arrayWidth = indexedBitmap.Array()[0].size();
	int arrayHeight = indexedBitmap.Array().size();
	int n1, n2, n3, n4, n5, n6, n7, n8;

	std::vector<std::vector<std::vector<int>>> layers(indexedBitmap.Palette().size());
	for (int k = 0; k < static_cast<int>(indexedBitmap.Palette().size()); k++) {
		layers[k].resize(arrayHeight);
		for (int j = 0; j < arrayHeight; j++) {
			layers[k][j].resize(arrayWidth, 0);
		}
	}

	// Edge detection for each color layer
	for (int j = 1; j < arrayHeight - 1; j++) {
		for (int i = 1; i < arrayWidth - 1; i++) {
			value = indexedBitmap.Array()[j][i];
			if (value < 0 || value >= static_cast<int>(indexedBitmap.Palette().size())) 
				continue;

			// Check neighbor pixels
			n1 = indexedBitmap.Array()[j-1][i-1] == value ? 1 : 0;
			n2 = indexedBitmap.Array()[j-1][i  ] == value ? 1 : 0;
			n3 = indexedBitmap.Array()[j-1][i+1] == value ? 1 : 0;
			n4 = indexedBitmap.Array()[j  ][i-1] == value ? 1 : 0;
			n5 = indexedBitmap.Array()[j  ][i+1] == value ? 1 : 0;
			n6 = indexedBitmap.Array()[j+1][i-1] == value ? 1 : 0;
			n7 = indexedBitmap.Array()[j+1][i  ] == value ? 1 : 0;
			n8 = indexedBitmap.Array()[j+1][i+1] == value ? 1 : 0;

			// Calculate edge type for current and neighboring pixels
			layers[value][j+1][i+1] = 1 + (n5 * 2) + (n8 * 4) + (n7 * 8);
			if (n4 == 0) {
				layers[value][j+1][i] = 0 + 2 + (n7 * 4) + (n6 * 8);
			}
			if (n2 == 0) {
				layers[value][j][i+1] = 0 + (n3 * 2) + (n5 * 4) + 8;
			}
			if (n1 == 0) {
				layers[value][j][i] = 0 + (n2 * 2) + 4 + (n4 * 8);
			}
		}
	}

	return layers;
}

std::vector<std::vector<std::vector<int>>>
PathScanner::ScanPaths(std::vector<std::vector<int>>& layerArray, float pathOmitThreshold)
{
	std::vector<std::vector<std::vector<int>>> paths;
	int positionX = 0, positionY = 0;
	int width = layerArray[0].size();
	int height = layerArray.size();
	int direction = 0;
	bool pathFinished = true;
	bool holePath = false;

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			if (layerArray[j][i] != 0 && layerArray[j][i] != 15) {
				positionX = i;
				positionY = j;
				paths.push_back(std::vector<std::vector<int>>());
				std::vector<std::vector<int>>& currentPath = paths.back();
				pathFinished = false;

				direction = kPathScanDirectionLookup[layerArray[positionY][positionX]];
				holePath = kPathScanHolePathLookup[layerArray[positionY][positionX]];

				int maxIterations = width * height;
				int iterations = 0;
				bool closed = false;

				while (!pathFinished && iterations < maxIterations) {
					std::vector<int> point(3);
					point[0] = positionX - 1;
					point[1] = positionY - 1;
					point[2] = layerArray[positionY][positionX];
					currentPath.push_back(point);

					int code = layerArray[positionY][positionX];
					if (code < 0 || code > 15) {
						pathFinished = true;
						closed = false;
						break;
					}

					const char* lookupRow = kPathScanCombinedLookup[code][direction];

					if (lookupRow[1] < 0) {
						pathFinished = true;
						closed = false;
						break;
					}

					layerArray[positionY][positionX] = lookupRow[0];
					direction = lookupRow[1];
					positionX += lookupRow[2];
					positionY += lookupRow[3];

					if (positionX < 0 || positionX >= width || positionY < 0 || positionY >= height) {
						pathFinished = true;
						closed = false;
						break;
					}

					if ((positionX - 1) == currentPath[0][0] && (positionY - 1) == currentPath[0][1]) {
						pathFinished = true;
						closed = true;
						break;
					}

					iterations++;
				}

				bool removePath = false;

				if (iterations >= maxIterations)
					removePath = true;
				else if (!closed)
					removePath = true;
				else if (holePath || currentPath.size() < pathOmitThreshold)
					removePath = true;

				if (removePath)
					paths.pop_back();
			}
		}
	}

	return paths;
}

std::vector<std::vector<std::vector<std::vector<int>>>>
PathScanner::ScanLayerPaths(const std::vector<std::vector<std::vector<int>>>& layers, int pathOmitThreshold)
{
	std::vector<std::vector<std::vector<std::vector<int>>>> batchPaths;
	for (int k = 0; k < static_cast<int>(layers.size()); k++) {
		std::vector<std::vector<int>> layerCopy = layers[k];
		batchPaths.push_back(ScanPaths(layerCopy, static_cast<float>(pathOmitThreshold)));
	}
	return batchPaths;
}

std::vector<std::vector<std::vector<double>>>
PathScanner::CreateInternodes(const std::vector<std::vector<std::vector<int>>>& paths)
{
	std::vector<std::vector<std::vector<double>>> internodes;

	for (int pathIndex = 0; pathIndex < static_cast<int>(paths.size()); pathIndex++) {
		internodes.push_back(std::vector<std::vector<double>>());
		std::vector<std::vector<double>>& currentInternodes = internodes.back();
		int pathLength = paths[pathIndex].size();

		if (pathLength < 2)
			continue;

		for (int pointIndex = 0; pointIndex < pathLength; pointIndex++) {
			int nextIndex = (pointIndex + 1) % pathLength;
			int nextIndex2 = (pointIndex + 2) % pathLength;

			std::vector<double> thisPoint(3);
			const std::vector<int>& point1 = paths[pathIndex][pointIndex];
			const std::vector<int>& point2 = paths[pathIndex][nextIndex];
			const std::vector<int>& point3 = paths[pathIndex][nextIndex2];

			thisPoint[0] = (point1[0] + point2[0]) / 2.0;
			thisPoint[1] = (point1[1] + point2[1]) / 2.0;
			double nextPointX = (point2[0] + point3[0]) / 2.0;
			double nextPointY = (point2[1] + point3[1]) / 2.0;

			// Calculate direction to next point
			if (thisPoint[0] < nextPointX) {
				if (thisPoint[1] < nextPointY) {
					thisPoint[2] = 1.0; // SouthEast
				} else if (thisPoint[1] > nextPointY) {
					thisPoint[2] = 7.0; // NorthEast
				} else {
					thisPoint[2] = 0.0; // East
				}
			} else if (thisPoint[0] > nextPointX) {
				if (thisPoint[1] < nextPointY) {
					thisPoint[2] = 3.0; // SouthWest
				} else if (thisPoint[1] > nextPointY) {
					thisPoint[2] = 5.0; // NorthWest
				} else {
					thisPoint[2] = 4.0; // West
				}
			} else {
				if (thisPoint[1] < nextPointY) {
					thisPoint[2] = 2.0; // South
				} else if (thisPoint[1] > nextPointY) {
					thisPoint[2] = 6.0; // North
				} else {
					thisPoint[2] = 8.0; // Center
				}
			}

			currentInternodes.push_back(thisPoint);
		}
	}

	return internodes;
}

std::vector<std::vector<std::vector<std::vector<double>>>>
PathScanner::CreateInternodes(const std::vector<std::vector<std::vector<std::vector<int>>>>& batchPaths)
{
	std::vector<std::vector<std::vector<std::vector<double>>>> batchInternodes;
	for (int k = 0; k < static_cast<int>(batchPaths.size()); k++) {
		batchInternodes.push_back(CreateInternodes(batchPaths[k]));
	}
	return batchInternodes;
}
