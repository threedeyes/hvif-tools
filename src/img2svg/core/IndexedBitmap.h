/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef INDEXED_BITMAP_H
#define INDEXED_BITMAP_H

#include <vector>

class IndexedBitmap {
public:
							IndexedBitmap();
							IndexedBitmap(const std::vector<std::vector<int>>& indexArray,
										const std::vector<std::vector<unsigned char>>& palette);

	int                     Width() const { return fWidth; }
	int                     Height() const { return fHeight; }

	const std::vector<std::vector<int>>& Array() const { return fArray; }
	const std::vector<std::vector<unsigned char>>& Palette() const { return fPalette; }
	const std::vector<std::vector<std::vector<std::vector<double>>>>& Layers() const { return fLayers; }

	void                    SetLayers(const std::vector<std::vector<std::vector<std::vector<double>>>>& layers);

private:
	int                     fWidth;
	int                     fHeight;
	std::vector<std::vector<int>> fArray;
	std::vector<std::vector<unsigned char>> fPalette;
	std::vector<std::vector<std::vector<std::vector<double>>>> fLayers;
};

#endif // INDEXED_BITMAP_H
