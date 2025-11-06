/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <cmath>

class MathUtils {
public:
	enum AlphaGroupType {
		ALPHA_TRANSPARENT = 0,
		ALPHA_SEMI_TRANSPARENT = 1,
		ALPHA_MOSTLY_OPAQUE = 2,
		ALPHA_OPAQUE = 3
	};

	static const unsigned char	ALPHA_THRESHOLD_TRANSPARENT = 10;
	static const unsigned char	ALPHA_THRESHOLD_SEMI = 128;
	static const unsigned char	ALPHA_THRESHOLD_OPAQUE = 250;

	static void					Init();

	static bool					Solve3x3(double M[3][3], double B[3], double X[3]);
	static bool					Solve3x3Normalized(double M[3][3], double B[3], double X[3]);

	static double				SRGBToLinear(double v);
	static double				LinearToSRGB(double v);

	static double				LumaD(double r, double g, double b);

	static double				CalculateSaturation(unsigned char r, unsigned char g, unsigned char b);

	static inline int			AlphaGroup(unsigned char a)
	{
		if (a < ALPHA_THRESHOLD_TRANSPARENT) return ALPHA_TRANSPARENT;
		if (a < ALPHA_THRESHOLD_SEMI) return ALPHA_SEMI_TRANSPARENT;
		if (a < ALPHA_THRESHOLD_OPAQUE) return ALPHA_MOSTLY_OPAQUE;
		return ALPHA_OPAQUE;
	}

	static inline bool			IsTransparent(unsigned char a)
	{
		return a < ALPHA_THRESHOLD_TRANSPARENT;
	}

	static inline bool			IsOpaque(unsigned char a)
	{
		return a >= ALPHA_THRESHOLD_OPAQUE;
	}

	static inline bool			IsSemiTransparent(unsigned char a) {
		return a >= ALPHA_THRESHOLD_TRANSPARENT && a < ALPHA_THRESHOLD_OPAQUE;
	}

	static double				PerceptualColorDistance(unsigned char r1, unsigned char g1, unsigned char b1, unsigned char a1,
													unsigned char r2, unsigned char g2, unsigned char b2, unsigned char a2);

	static double				PerceptualColorDistanceForMerge(unsigned char r1, unsigned char g1, unsigned char b1, unsigned char a1,
													unsigned char r2, unsigned char g2, unsigned char b2, unsigned char a2);

	static double				AdaptiveThreshold(int paletteSize, double baseThreshold);

	static int					GetSquare(int diff);
	static int					GetShift(int level);

	static const double	MAX_DISTANCE;

private:
	static void					_InitTables();

	static bool					sInitialized;
	static int					sSquares[512];
	static int					sShift[9];
};

#endif
