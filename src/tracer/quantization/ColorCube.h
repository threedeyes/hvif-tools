/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef COLOR_CUBE_H
#define COLOR_CUBE_H

#include <vector>
#include "MathUtils.h"

class ColorNode;

struct ColorSearchResult {
	double					distance;
	int						colorNumber;

							ColorSearchResult() : distance(MathUtils::MAX_DISTANCE), colorNumber(0) {}
};

class ColorCube {
public:
							ColorCube(const std::vector<std::vector<int> >& pixels, int maxColors, int skipValue = 2147483647);
							~ColorCube();

	void					ClassifyColors();
	void					ReduceColors();
	void					AssignColors();

	std::vector<int>		GetColormap() const { return fColormap; }

private:
	friend class ColorNode;

	std::vector<std::vector<int> > fPixels;
	int						fMaxColors;
	std::vector<int>		fColormap;

	ColorNode*				fRoot;
	int						fDepth;
	int						fColors;
	int						fNodes;
	int						fSkipValue;
};

#endif
