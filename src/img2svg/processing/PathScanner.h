/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef PATH_SCANNER_H
#define PATH_SCANNER_H

#include <vector>
#include "IndexedBitmap.h"

class PathScanner {
public:
							PathScanner();
							~PathScanner();

	std::vector<std::vector<std::vector<int>>> 
							CreateLayers(const IndexedBitmap& indexedBitmap);

	std::vector<std::vector<std::vector<int>>> 
							ScanPaths(std::vector<std::vector<int>>& layerArray, 
									float pathOmitThreshold);

	std::vector<std::vector<std::vector<std::vector<int>>>> 
							ScanLayerPaths(const std::vector<std::vector<std::vector<int>>>& layers, 
										 int pathOmitThreshold);

	std::vector<std::vector<std::vector<double>>> 
							CreateInternodes(const std::vector<std::vector<std::vector<int>>>& paths);

	std::vector<std::vector<std::vector<std::vector<double>>>> 
							CreateInternodes(const std::vector<std::vector<std::vector<std::vector<int>>>>& batchPaths);

private:
	static const unsigned char kPathScanDirectionLookup[16];
	static const bool       kPathScanHolePathLookup[16];
	static const char       kPathScanCombinedLookup[16][4][4];
};

#endif // PATH_SCANNER_H
