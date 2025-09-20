/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef INDEXED_BITMAP_H
#define INDEXED_BITMAP_H

#include <vector>

class IndexedBitmap {
public:
	struct LinearGradient {
		bool valid;
		double x1, y1, x2, y2;
		unsigned char c1[4];
		unsigned char c2[4];

		LinearGradient()
			: valid(false), x1(0), y1(0), x2(0), y2(0)
		{
			c1[0] = c1[1] = c1[2] = 255; c1[3] = 255;
			c2[0] = c2[1] = c2[2] = 255; c2[3] = 255;
		}
	};

							IndexedBitmap();
							IndexedBitmap(const std::vector<std::vector<int>>& indexArray,
										const std::vector<std::vector<unsigned char>>& palette);

	int                     Width() const { return fWidth; }
	int                     Height() const { return fHeight; }

	const std::vector<std::vector<int>>& Array() const { return fArray; }
	const std::vector<std::vector<unsigned char>>& Palette() const { return fPalette; }
	const std::vector<std::vector<std::vector<std::vector<double>>>>& Layers() const { return fLayers; }

	void                    SetLayers(const std::vector<std::vector<std::vector<std::vector<double>>>>& layers);

	const std::vector<std::vector<LinearGradient> >& LinearGradients() const { return fLinearGradients; }
	void                    SetLinearGradients(const std::vector<std::vector<LinearGradient>>& gradients) { fLinearGradients = gradients; }

private:
	int                     fWidth;
	int                     fHeight;
	std::vector<std::vector<int>> fArray;
	std::vector<std::vector<unsigned char>> fPalette;
	std::vector<std::vector<std::vector<std::vector<double>>>> fLayers;

	std::vector<std::vector<LinearGradient>> fLinearGradients;
};

#endif // INDEXED_BITMAP_H
