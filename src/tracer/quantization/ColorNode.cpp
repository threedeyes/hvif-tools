/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <climits>
#include <cmath>

#include "ColorNode.h"
#include "ColorCube.h"
#include "MathUtils.h"

static const int kMaxRgb = 255;
static const int kMaxTreeDepth = 8;

ColorNode::ColorNode(ColorCube* cube)
	: fCube(cube)
	, fParent(this)
	, fChildren(16, NULL)
	, fChildCount(0)
	, fId(0)
	, fLevel(0)
	, fNumberPixels(INT_MAX)
	, fUniqueCount(0)
	, fTotalRed(0)
	, fTotalGreen(0)
	, fTotalBlue(0)
	, fTotalAlpha(0)
	, fColorNumber(0)
{
	MathUtils::Init();

	fMidRed = (kMaxRgb + 1) >> 1;
	fMidGreen = (kMaxRgb + 1) >> 1;
	fMidBlue = (kMaxRgb + 1) >> 1;
	fMidAlpha = (kMaxRgb + 1) >> 1;
}

ColorNode::ColorNode(ColorNode* parent, int nodeId, int nodeLevel)
	: fCube(parent->fCube)
	, fParent(parent)
	, fChildren(16, NULL)
	, fChildCount(0)
	, fId(nodeId)
	, fLevel(nodeLevel)
	, fNumberPixels(0)
	, fUniqueCount(0)
	, fTotalRed(0)
	, fTotalGreen(0)
	, fTotalBlue(0)
	, fTotalAlpha(0)
	, fColorNumber(0)
{
	++fCube->fNodes;
	if (fLevel == fCube->fDepth) {
		++fCube->fColors;
	}

	++fParent->fChildCount;
	fParent->fChildren[fId] = this;

	int bitIndex = (1 << (kMaxTreeDepth - fLevel)) >> 1;
	fMidRed = fParent->fMidRed + ((fId & 1) > 0 ? bitIndex : -bitIndex);
	fMidGreen = fParent->fMidGreen + ((fId & 2) > 0 ? bitIndex : -bitIndex);
	fMidBlue = fParent->fMidBlue + ((fId & 4) > 0 ? bitIndex : -bitIndex);
	fMidAlpha = fParent->fMidAlpha + ((fId & 8) > 0 ? bitIndex : -bitIndex);
}

ColorNode::~ColorNode()
{
	for (int i = 0; i < 16; i++)
		delete fChildren[i];
}

void
ColorNode::PruneChild()
{
	--fParent->fChildCount;
	fParent->fUniqueCount += fUniqueCount;
	fParent->fTotalRed += fTotalRed;
	fParent->fTotalGreen += fTotalGreen;
	fParent->fTotalBlue += fTotalBlue;
	fParent->fTotalAlpha += fTotalAlpha;
	fParent->fChildren[fId] = NULL;
	--fCube->fNodes;
	fCube = NULL;
	fParent = NULL;
}

void
ColorNode::PruneLevel()
{
	if (fChildCount != 0) {
		for (int id = 0; id < 16; id++) {
			if (fChildren[id]) {
				fChildren[id]->PruneLevel();
			}
		}
	}

	if (fLevel == fCube->fDepth)
		PruneChild();
}

int
ColorNode::Reduce(int threshold, int nextThreshold)
{
	if (fChildCount != 0) {
		for (int id = 0; id < 16; id++) {
			if (fChildren[id]) {
				nextThreshold = fChildren[id]->Reduce(threshold, nextThreshold);
			}
		}
	}

	if (fNumberPixels <= threshold) {
		PruneChild();
	} else {
		if (fUniqueCount != 0) {
			fCube->fColors++;
		}
		if (fNumberPixels < nextThreshold) {
			nextThreshold = fNumberPixels;
		}
	}

	return nextThreshold;
}

void
ColorNode::CreateColormap()
{
	if (fChildCount != 0) {
		for (int id = 0; id < 16; id++) {
			if (fChildren[id]) {
				fChildren[id]->CreateColormap();
			}
		}
	}

	if (fUniqueCount != 0) {
		int red   = ((fTotalRed + (fUniqueCount >> 1)) / fUniqueCount);
		int green = ((fTotalGreen + (fUniqueCount >> 1)) / fUniqueCount);
		int blue  = ((fTotalBlue + (fUniqueCount >> 1)) / fUniqueCount);
		int alpha = ((fTotalAlpha + (fUniqueCount >> 1)) / fUniqueCount);

		fCube->fColormap[fCube->fColors] = ((alpha & 0xFF) << 24) | ((red & 0xFF) << 16) |
										  ((green & 0xFF) << 8) | (blue & 0xFF);
		fColorNumber = fCube->fColors++;
	}
}

void
ColorNode::FindClosestColor(int red, int green, int blue, int alpha, ColorSearchResult& search)
{
	if (fChildCount != 0) {
		for (int id = 0; id < 16; id++) {
			if (fChildren[id]) {
				fChildren[id]->FindClosestColor(red, green, blue, alpha, search);
			}
		}
	}

	if (fUniqueCount != 0) {
		int color = fCube->fColormap[fColorNumber];
		double distance = MathUtils::PerceptualColorDistance(
			(unsigned char)red, (unsigned char)green, (unsigned char)blue, (unsigned char)alpha,
			(unsigned char)((color >> 16) & 0xFF),
			(unsigned char)((color >> 8) & 0xFF),
			(unsigned char)(color & 0xFF),
			(unsigned char)((color >> 24) & 0xFF)
		);

		if (distance < search.distance) {
			search.distance = distance;
			search.colorNumber = fColorNumber;
		}
	}
}

ColorNode*
ColorNode::GetChild(int index) const
{
	if (index >= 0 && index < 16)
		return fChildren[index];

	return NULL;
}

void
ColorNode::SetChild(int index, ColorNode* child)
{
	if (index >= 0 && index < 16)
		fChildren[index] = child;
}

void
ColorNode::AddColor(int red, int green, int blue, int alpha)
{
	fTotalRed += red;
	fTotalGreen += green;
	fTotalBlue += blue;
	fTotalAlpha += alpha;
}

int
ColorNode::GetShift(int level)
{
	return MathUtils::GetShift(level);
}
