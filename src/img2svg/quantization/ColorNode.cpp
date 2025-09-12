/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <climits>
#include <cmath>

#include "ColorNode.h"
#include "ColorCube.h"

static const int kMaxRgb = 255;
static const int kMaxTreeDepth = 8;

static int sSquares[kMaxRgb + kMaxRgb + 1];
static int sShift[kMaxTreeDepth + 1];
static bool sTablesInitialized = false;

static void
InitializeTables()
{
	if (sTablesInitialized)
		return;

	for (int i = -kMaxRgb; i <= kMaxRgb; i++)
		sSquares[i + kMaxRgb] = i * i;

	for (int i = 0; i < kMaxTreeDepth + 1; ++i)
		sShift[i] = 1 << (15 - i);

	sTablesInitialized = true;
}

ColorNode::ColorNode(ColorCube* cube)
	: fCube(cube)
	, fParent(this)
	, fChildren(8, nullptr)
	, fChildCount(0)
	, fId(0)
	, fLevel(0)
	, fNumberPixels(INT_MAX)
	, fUniqueCount(0)
	, fTotalRed(0)
	, fTotalGreen(0)
	, fTotalBlue(0)
	, fColorNumber(0)
{
	InitializeTables();

	fMidRed = (kMaxRgb + 1) >> 1;
	fMidGreen = (kMaxRgb + 1) >> 1;
	fMidBlue = (kMaxRgb + 1) >> 1;
}

ColorNode::ColorNode(ColorNode* parent, int nodeId, int nodeLevel)
	: fCube(parent->fCube)
	, fParent(parent)
	, fChildren(8, nullptr)
	, fChildCount(0)
	, fId(nodeId)
	, fLevel(nodeLevel)
	, fNumberPixels(0)
	, fUniqueCount(0)
	, fTotalRed(0)
	, fTotalGreen(0)
	, fTotalBlue(0)
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
}

ColorNode::~ColorNode()
{
	for (int i = 0; i < 8; i++)
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
	fParent->fChildren[fId] = nullptr;
	--fCube->fNodes;
	fCube = nullptr;
	fParent = nullptr;
}

void
ColorNode::PruneLevel()
{
	if (fChildCount != 0) {
		for (int id = 0; id < 8; id++) {
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
		for (int id = 0; id < 8; id++) {
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
		for (int id = 0; id < 8; id++) {
			if (fChildren[id]) {
				fChildren[id]->CreateColormap();
			}
		}
	}

	if (fUniqueCount != 0) {
		int red   = ((fTotalRed + (fUniqueCount >> 1)) / fUniqueCount);
		int green = ((fTotalGreen + (fUniqueCount >> 1)) / fUniqueCount);
		int blue  = ((fTotalBlue + (fUniqueCount >> 1)) / fUniqueCount);

		fCube->fColormap[fCube->fColors] = (0xFF << 24) | ((red & 0xFF) << 16) | 
										  ((green & 0xFF) << 8) | (blue & 0xFF);
		fColorNumber = fCube->fColors++;
	}
}

void
ColorNode::FindClosestColor(int red, int green, int blue, ColorSearchResult& search)
{
	if (fChildCount != 0) {
		for (int id = 0; id < 8; id++) {
			if (fChildren[id]) {
				fChildren[id]->FindClosestColor(red, green, blue, search);
			}
		}
	}

	if (fUniqueCount != 0) {
		int color = fCube->fColormap[fColorNumber];
		int distance = _CalculateDistance(color, red, green, blue);
		if (distance < search.distance) {
			search.distance = distance;
			search.colorNumber = fColorNumber;
		}
	}
}

ColorNode*
ColorNode::GetChild(int index) const
{
	if (index >= 0 && index < 8)
		return fChildren[index];

	return nullptr;
}

void
ColorNode::SetChild(int index, ColorNode* child)
{
	if (index >= 0 && index < 8)
		fChildren[index] = child;
}

void
ColorNode::AddColor(int red, int green, int blue)
{
	fTotalRed += red;
	fTotalGreen += green;
	fTotalBlue += blue;
}

int
ColorNode::GetShift(int level)
{
	InitializeTables();
	return sShift[level];
}

int
ColorNode::_CalculateDistance(int color, int red, int green, int blue)
{
	InitializeTables();

	return (sSquares[((color >> 16) & 0xFF) - red + kMaxRgb] +
			sSquares[((color >> 8) & 0xFF) - green + kMaxRgb] +
			sSquares[(color & 0xFF) - blue + kMaxRgb]);
}
