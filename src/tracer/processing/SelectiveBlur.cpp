/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <cmath>
#include <cstdlib>

#include "SelectiveBlur.h"

SelectiveBlur::SelectiveBlur()
{
}

SelectiveBlur::~SelectiveBlur()
{
}

std::vector<double>
SelectiveBlur::_GenerateGaussianKernel(int radius)
{
	std::vector<double> kernel(radius * 2 + 1);
	double sigma = radius / 2.0;
	if (sigma < 0.5) sigma = 0.5;

	double sum = 0.0;

	for (int i = -radius; i <= radius; i++) {
		double val = exp(-(i * i) / (2.0 * sigma * sigma));
		kernel[i + radius] = val;
		sum += val;
	}

	for (int i = 0; i < static_cast<int>(kernel.size()); i++) {
		kernel[i] /= sum;
	}

	return kernel;
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

	std::vector<double> gaussianKernel = _GenerateGaussianKernel(radiusInt);

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
