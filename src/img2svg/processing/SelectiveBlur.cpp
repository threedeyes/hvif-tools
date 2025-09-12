/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <cmath>
#include <cstdlib>

#include "SelectiveBlur.h"

const double SelectiveBlur::kGaussianKernels[5][11] = {
	{0.27901, 0.44198, 0.27901}, // radius 1
	{0.135336, 0.228569, 0.272192, 0.228569, 0.135336}, // radius 2
	{0.086776, 0.136394, 0.178908, 0.195843, 0.178908, 0.136394, 0.086776}, // radius 3
	{0.063327, 0.093095, 0.122589, 0.144599, 0.152781, 0.144599, 0.122589, 0.093095, 0.063327}, // radius 4
	{0.049692, 0.069304, 0.089767, 0.107988, 0.120651, 0.125194, 0.120651, 0.107988, 0.089767, 0.069304, 0.049692} // radius 5
};

SelectiveBlur::SelectiveBlur()
{
}

SelectiveBlur::~SelectiveBlur()
{
}

BitmapData
SelectiveBlur::BlurBitmap(const BitmapData& bitmap, float radius, float delta)
{
	BitmapData result(bitmap.Width(), bitmap.Height(),
					std::vector<unsigned char>(bitmap.Width() * bitmap.Height() * 4));

	int radiusInt = static_cast<int>(floor(radius));
	if (radiusInt < 1)
		return bitmap;

	if (radiusInt > 5)
		radiusInt = 5;
	
	int deltaInt = static_cast<int>(abs(delta));
	if (deltaInt > 1024)
		deltaInt = 1024;

	const double* gaussianKernel = kGaussianKernels[radiusInt - 1];

	// Horizontal blur pass
	for (int y = 0; y < bitmap.Height(); y++) {
		for (int x = 0; x < bitmap.Width(); x++) {
			double redAccumulator = 0, greenAccumulator = 0, blueAccumulator = 0, alphaAccumulator = 0;
			double weightAccumulator = 0;

			for (int k = -radiusInt; k < (radiusInt + 1); k++) {
				if (((x + k) > 0) && ((x + k) < bitmap.Width())) {
					redAccumulator   += bitmap.GetPixelComponent(x + k, y, 0) * gaussianKernel[k + radiusInt];
					greenAccumulator += bitmap.GetPixelComponent(x + k, y, 1) * gaussianKernel[k + radiusInt];
					blueAccumulator  += bitmap.GetPixelComponent(x + k, y, 2) * gaussianKernel[k + radiusInt];
					alphaAccumulator += bitmap.GetPixelComponent(x + k, y, 3) * gaussianKernel[k + radiusInt];
					weightAccumulator += gaussianKernel[k + radiusInt];
				}
			}

			result.SetPixelComponent(x, y, 0, static_cast<unsigned char>(floor(redAccumulator / weightAccumulator)));
			result.SetPixelComponent(x, y, 1, static_cast<unsigned char>(floor(greenAccumulator / weightAccumulator)));
			result.SetPixelComponent(x, y, 2, static_cast<unsigned char>(floor(blueAccumulator / weightAccumulator)));
			result.SetPixelComponent(x, y, 3, static_cast<unsigned char>(floor(alphaAccumulator / weightAccumulator)));
		}
	}

	std::vector<unsigned char> horizontalBlurredData = result.Data();

	// Vertical blur pass
	for (int y = 0; y < bitmap.Height(); y++) {
		for (int x = 0; x < bitmap.Width(); x++) {
			double redAccumulator = 0, greenAccumulator = 0, blueAccumulator = 0, alphaAccumulator = 0;
			double weightAccumulator = 0;

			for (int k = -radiusInt; k < (radiusInt + 1); k++) {
				if (((y + k) > 0) && ((y + k) < bitmap.Height())) {
					int index = (((y + k) * bitmap.Width()) + x) * 4;
					redAccumulator   += horizontalBlurredData[index] * gaussianKernel[k + radiusInt];
					greenAccumulator += horizontalBlurredData[index + 1] * gaussianKernel[k + radiusInt];
					blueAccumulator  += horizontalBlurredData[index + 2] * gaussianKernel[k + radiusInt];
					alphaAccumulator += horizontalBlurredData[index + 3] * gaussianKernel[k + radiusInt];
					weightAccumulator += gaussianKernel[k + radiusInt];
				}
			}

			result.SetPixelComponent(x, y, 0, static_cast<unsigned char>(floor(redAccumulator / weightAccumulator)));
			result.SetPixelComponent(x, y, 1, static_cast<unsigned char>(floor(greenAccumulator / weightAccumulator)));
			result.SetPixelComponent(x, y, 2, static_cast<unsigned char>(floor(blueAccumulator / weightAccumulator)));
			result.SetPixelComponent(x, y, 3, static_cast<unsigned char>(floor(alphaAccumulator / weightAccumulator)));
		}
	}

	// Selective blur: preserve edges
	for (int y = 0; y < bitmap.Height(); y++) {
		for (int x = 0; x < bitmap.Width(); x++) {
			int difference = abs(result.GetPixelComponent(x, y, 0) - bitmap.GetPixelComponent(x, y, 0)) +
							abs(result.GetPixelComponent(x, y, 1) - bitmap.GetPixelComponent(x, y, 1)) +
							abs(result.GetPixelComponent(x, y, 2) - bitmap.GetPixelComponent(x, y, 2)) +
							abs(result.GetPixelComponent(x, y, 3) - bitmap.GetPixelComponent(x, y, 3));

			if (difference > deltaInt) {
				result.SetPixelComponent(x, y, 0, bitmap.GetPixelComponent(x, y, 0));
				result.SetPixelComponent(x, y, 1, bitmap.GetPixelComponent(x, y, 1));
				result.SetPixelComponent(x, y, 2, bitmap.GetPixelComponent(x, y, 2));
				result.SetPixelComponent(x, y, 3, bitmap.GetPixelComponent(x, y, 3));
			}
		}
	}

	return result;
}
