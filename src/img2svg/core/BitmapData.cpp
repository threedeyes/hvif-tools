/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <climits>

#include "BitmapData.h"

BitmapData::BitmapData()
	: fWidth(0)
	, fHeight(0)
{
}

BitmapData::BitmapData(int width, int height, const std::vector<unsigned char>& data)
	: fWidth(width)
	, fHeight(height)
	, fData(data)
{
	if (fWidth < 0 || fHeight < 0) {
		fWidth = 0;
		fHeight = 0;
		fData.clear();
		return;
	}

	if (fWidth > 0 && fHeight > INT_MAX / fWidth) {
		fWidth = 0;
		fHeight = 0;
		fData.clear();
		return;
	}

	int pixelCount = fWidth * fHeight;
	if (pixelCount > INT_MAX / 4) {
		fWidth = 0;
		fHeight = 0;
		fData.clear();
		return;
	}

	size_t requiredSize = static_cast<size_t>(pixelCount) * 4;

	if (fData.size() != requiredSize)
		fData.resize(requiredSize, 0);
}

bool
BitmapData::IsValid() const
{
	if (fWidth <= 0 || fHeight <= 0)
		return false;

	if (fWidth > INT_MAX / fHeight)
		return false;

	int pixelCount = fWidth * fHeight;
	if (pixelCount > INT_MAX / 4)
		return false;

	size_t requiredSize = static_cast<size_t>(pixelCount) * 4;

	return fData.size() == requiredSize;
}

unsigned char
BitmapData::GetPixelComponent(int x, int y, int component) const
{
	if (x < 0 || x >= fWidth || y < 0 || y >= fHeight || component < 0 || component > 3)
		return 0;

	int index = (y * fWidth + x) * 4 + component;
	if (index >= static_cast<int>(fData.size()))
		return 0;

	return fData[index];
}

void
BitmapData::SetPixelComponent(int x, int y, int component, unsigned char value)
{
	if (x < 0 || x >= fWidth || y < 0 || y >= fHeight || component < 0 || component > 3)
		return;

	int index = (y * fWidth + x) * 4 + component;
	if (index >= static_cast<int>(fData.size()))
		return;

	fData[index] = value;
}
