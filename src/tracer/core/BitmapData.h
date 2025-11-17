/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef BITMAP_DATA_H
#define BITMAP_DATA_H

#include <vector>

class BitmapData {
public:
							BitmapData();
							BitmapData(int width, int height, 
									const std::vector<unsigned char>& data);

	int						Width() const { return fWidth; }
	int						Height() const { return fHeight; }
	const std::vector<unsigned char>& Data() const { return fData; }

	bool					IsValid() const;
	unsigned char			GetPixelComponent(int x, int y, int component) const;
	void					SetPixelComponent(int x, int y, int component, unsigned char value);

private:
	int						fWidth;
	int						fHeight;
	std::vector<unsigned char> fData;
};

#endif
