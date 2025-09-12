/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <climits>

#include "ColorCube.h"
#include "ColorNode.h"

static const int kMaxNodes = 266817;
static const int kMaxTreeDepth = 8;

ColorCube::ColorCube(const std::vector<std::vector<int>>& pixels, int maxColors)
	: fPixels(pixels)
	, fMaxColors(maxColors)
	, fColors(0)
	, fNodes(0)
{
	int colorCount = maxColors;
	for (fDepth = 1; colorCount != 0; fDepth++) {
		colorCount /= 4;
	}
	if (fDepth > 1) {
		--fDepth;
	}
	if (fDepth > kMaxTreeDepth) {
		fDepth = kMaxTreeDepth;
	} else if (fDepth < 2) {
		fDepth = 2;
	}

	fRoot = new ColorNode(this);
}

ColorCube::~ColorCube()
{
	delete fRoot;
}

void
ColorCube::ClassifyColors()
{
	int width = fPixels.size();
	int height = fPixels[0].size();
	
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			int pixel = fPixels[x][y];
			int red   = (pixel >> 16) & 0xFF;
			int green = (pixel >> 8) & 0xFF;
			int blue  = pixel & 0xFF;
			
			if (fNodes > kMaxNodes) {
				fRoot->PruneLevel();
				--fDepth;
			}

			ColorNode* node = fRoot;
			for (int level = 1; level <= fDepth; ++level) {
				int id = (((red > node->GetMidRed() ? 1 : 0) << 0) |
						 ((green > node->GetMidGreen() ? 1 : 0) << 1) |
						 ((blue > node->GetMidBlue() ? 1 : 0) << 2));

				if (!node->GetChild(id)) {
					node->SetChild(id, new ColorNode(node, id, level));
				}
				node = node->GetChild(id);
				node->AddPixelCount(ColorNode::GetShift(level));
			}

			node->IncrementUniqueCount();
			node->AddColor(red, green, blue);
		}
	}
}

void
ColorCube::ReduceColors()
{
	int threshold = 1;
	while (fColors > fMaxColors) {
		fColors = 0;
		threshold = fRoot->Reduce(threshold, INT_MAX);
	}
}

void
ColorCube::AssignColors()
{
	fColormap.resize(fColors);

	fColors = 0;
	fRoot->CreateColormap();

	int width = fPixels.size();
	int height = fPixels[0].size();

	ColorSearchResult search;

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			int pixel = fPixels[x][y];
			int red   = (pixel >> 16) & 0xFF;
			int green = (pixel >> 8) & 0xFF;
			int blue  = pixel & 0xFF;

			ColorNode* node = fRoot;
			for (;;) {
				int id = (((red > node->GetMidRed() ? 1 : 0) << 0) |
						 ((green > node->GetMidGreen() ? 1 : 0) << 1) |
						 ((blue > node->GetMidBlue() ? 1 : 0) << 2));
						 
				if (!node->GetChild(id)) {
					break;
				}
				node = node->GetChild(id);
			}

			search.distance = INT_MAX;
			node->GetParent()->FindClosestColor(red, green, blue, search);
		}
	}
}
