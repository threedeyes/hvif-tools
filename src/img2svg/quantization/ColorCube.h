/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef COLOR_CUBE_H
#define COLOR_CUBE_H

#include <vector>

class ColorNode;

struct ColorSearchResult {
	int                     distance;
	int                     colorNumber;

							ColorSearchResult() : distance(0), colorNumber(0) {}
};

class ColorCube {
public:
							ColorCube(const std::vector<std::vector<int>>& pixels, int maxColors);
							~ColorCube();

	void                    ClassifyColors();
	void                    ReduceColors();
	void                    AssignColors();

	std::vector<int>        GetColormap() const { return fColormap; }

private:
	friend class ColorNode;

	std::vector<std::vector<int>> fPixels;
	int                     fMaxColors;
	std::vector<int>        fColormap;

	ColorNode*              fRoot;
	int                     fDepth;
	int                     fColors;
	int                     fNodes;
};

#endif // COLOR_CUBE_H
