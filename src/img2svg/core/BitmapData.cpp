/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

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
}

bool
BitmapData::IsValid() const
{
	return fWidth > 0 && fHeight > 0 && 
		   fData.size() == static_cast<size_t>(fWidth * fHeight * 4);
}

unsigned char
BitmapData::GetPixelComponent(int x, int y, int component) const
{
	if (x < 0 || x >= fWidth || y < 0 || y >= fHeight || component < 0 || component > 3)
		return 0;

	int index = (y * fWidth + x) * 4 + component;
	return fData[index];
}

void
BitmapData::SetPixelComponent(int x, int y, int component, unsigned char value)
{
	if (x < 0 || x >= fWidth || y < 0 || y >= fHeight || component < 0 || component > 3)
		return;

	int index = (y * fWidth + x) * 4 + component;
	fData[index] = value;
}
