/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <climits>

#include "ColorCube.h"
#include "ColorNode.h"
#include "MathUtils.h"

static const int kMaxNodes = 266817;
static const int kMaxTreeDepth = 8;

ColorCube::ColorCube(const std::vector<std::vector<int> >& pixels, int maxColors, int skipValue)
	: fPixels(pixels)
	, fMaxColors(maxColors)
	, fColors(0)
	, fNodes(0)
	, fSkipValue(skipValue)
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
	int height = fPixels.size();
	int width = fPixels[0].size();

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int pixel = fPixels[y][x];

			if (pixel == fSkipValue)
				continue;

			int red   = (pixel >> 16) & 0xFF;
			int green = (pixel >> 8) & 0xFF;
			int blue  = pixel & 0xFF;
			int alpha = (pixel >> 24) & 0xFF;

			if (MathUtils::IsTransparent((unsigned char)alpha))
				continue;

			if (fNodes > kMaxNodes) {
				fRoot->PruneLevel();
				if (fDepth > 2)
					--fDepth;
			}

			ColorNode* node = fRoot;
			for (int level = 1; level <= fDepth; ++level) {
				int id = (((red > node->GetMidRed() ? 1 : 0) << 0) |
						 ((green > node->GetMidGreen() ? 1 : 0) << 1) |
						 ((blue > node->GetMidBlue() ? 1 : 0) << 2) |
						 ((alpha > node->GetMidAlpha() ? 1 : 0) << 3));

				if (!node->GetChild(id)) {
					node->SetChild(id, new ColorNode(node, id, level));
				}
				node = node->GetChild(id);
				node->AddPixelCount(ColorNode::GetShift(level));
			}

			node->IncrementUniqueCount();
			node->AddColor(red, green, blue, alpha);
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

	int height = fPixels.size();
	int width = fPixels[0].size();

	ColorSearchResult search;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int pixel = fPixels[y][x];

			if (pixel == fSkipValue)
				continue;

			int red   = (pixel >> 16) & 0xFF;
			int green = (pixel >> 8) & 0xFF;
			int blue  = pixel & 0xFF;
			int alpha = (pixel >> 24) & 0xFF;

			if (MathUtils::IsTransparent((unsigned char)alpha))
				continue;

			ColorNode* node = fRoot;
			for (;;) {
				int id = (((red > node->GetMidRed() ? 1 : 0) << 0) |
						 ((green > node->GetMidGreen() ? 1 : 0) << 1) |
						 ((blue > node->GetMidBlue() ? 1 : 0) << 2) |
						 ((alpha > node->GetMidAlpha() ? 1 : 0) << 3));

				if (!node->GetChild(id))
					break;

				node = node->GetChild(id);
			}

			search.distance = MathUtils::MAX_DISTANCE;
			node->GetParent()->FindClosestColor(red, green, blue, alpha, search);
		}
	}
}
