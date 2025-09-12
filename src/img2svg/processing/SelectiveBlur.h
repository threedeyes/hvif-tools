/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef SELECTIVE_BLUR_H
#define SELECTIVE_BLUR_H

#include "BitmapData.h"

class SelectiveBlur {
public:
							SelectiveBlur();
							~SelectiveBlur();

	BitmapData              BlurBitmap(const BitmapData& bitmap,
									float radius, float delta);

private:
	static const double     kGaussianKernels[5][11];
};

#endif // SELECTIVE_BLUR_H
