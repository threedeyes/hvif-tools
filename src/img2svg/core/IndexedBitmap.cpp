/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "IndexedBitmap.h"

IndexedBitmap::IndexedBitmap()
	: fWidth(0)
	, fHeight(0)
{
}

IndexedBitmap::IndexedBitmap(const std::vector<std::vector<int>>& indexArray,
							const std::vector<std::vector<unsigned char>>& palette)
	: fArray(indexArray)
	, fPalette(palette)
{
	if (!indexArray.empty() && !indexArray[0].empty()) {
		fWidth = indexArray[0].size() - 2;
		fHeight = indexArray.size() - 2;
	}
}

void
IndexedBitmap::SetLayers(const std::vector<std::vector<std::vector<std::vector<double>>>>& layers)
{
	fLayers = layers;
}
