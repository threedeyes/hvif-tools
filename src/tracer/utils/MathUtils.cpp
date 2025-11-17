/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "MathUtils.h"

bool MathUtils::sInitialized = false;
int MathUtils::sSquares[512];
int MathUtils::sShift[9];
const double MathUtils::MAX_DISTANCE = 999999.0;

void
MathUtils::Init()
{
	if (!sInitialized)
		_InitTables();
}

void
MathUtils::_InitTables()
{
	for (int i = -255; i <= 255; i++)
		sSquares[i + 255] = i * i;

	for (int i = 0; i < 9; i++)
		sShift[i] = 1 << (15 - i);

	sInitialized = true;
}

bool
MathUtils::Solve3x3(double M[3][3], double B[3], double X[3])
{
	int i, j, k;
	double A[3][4];
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++)
			A[i][j] = M[i][j];
		A[i][3] = B[i];
	}

	for (i = 0; i < 3; i++) {
		int piv = i;
		double maxabs = std::fabs(A[i][i]);
		for (k = i + 1; k < 3; k++) {
			double v = std::fabs(A[k][i]);
			if (v > maxabs) {
				maxabs = v;
				piv = k;
			}
		}

		double scale = 0.0;
		for (k = 0; k < 3; k++) {
			for (j = 0; j < 3; j++) {
				double val = std::fabs(M[k][j]);
				if (val > scale) scale = val;
			}
		}
		if (scale < 1e-100) scale = 1.0;

		double threshold = 1e-10 * scale;
		if (maxabs < threshold)
			return false;

		if (piv != i) {
			for (j = i; j < 4; j++) {
				double tmp = A[i][j];
				A[i][j] = A[piv][j];
				A[piv][j] = tmp;
			}
		}

		double diag = A[i][i];
		for (j = i; j < 4; j++)
			A[i][j] /= diag;

		for (k = 0; k < 3; k++) {
			if (k != i) {
				double f = A[k][i];
				for (j = i; j < 4; j++)
					A[k][j] -= f * A[i][j];
			}
		}
	}

	for (i = 0; i < 3; i++)
		X[i] = A[i][3];

	return true;
}

bool
MathUtils::Solve3x3Normalized(double M[3][3], double B[3], double X[3])
{
	double scale[3] = {1.0, 1.0, 1.0};
	double Mnorm[3][3];
	double Bnorm[3];

	for (int i = 0; i < 3; i++) {
		double maxVal = 0.0;
		for (int j = 0; j < 3; j++) {
			double val = std::fabs(M[i][j]);
			if (val > maxVal) maxVal = val;
		}
		if (maxVal > 1e-100)
			scale[i] = maxVal;
	}

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			Mnorm[i][j] = M[i][j] / scale[i];
		}
		Bnorm[i] = B[i] / scale[i];
	}

	if (!Solve3x3(Mnorm, Bnorm, X))
		return false;

	return true;
}

double
MathUtils::SRGBToLinear(double v)
{
	double u = v / 255.0;
	double L;
	if (u <= 0.04045)
		L = u / 12.92;
	else
		L = std::pow((u + 0.055) / 1.055, 2.4);
	return L * 255.0;
}

double
MathUtils::LinearToSRGB(double v)
{
	double u = v / 255.0;
	double S;
	if (u <= 0.0031308)
		S = 12.92 * u;
	else
		S = 1.055 * std::pow(u, 1.0 / 2.4) - 0.055;
	S *= 255.0;
	if (S < 0.0)
		S = 0.0;
	else if (S > 255.0)
		S = 255.0;
	return S;
}

double
MathUtils::LumaD(double r, double g, double b)
{
	return 0.2126 * r + 0.7152 * g + 0.0722 * b;
}

double
MathUtils::CalculateSaturation(unsigned char r, unsigned char g, unsigned char b)
{
	int maxVal = r;
	if (g > maxVal) maxVal = g;
	if (b > maxVal) maxVal = b;
	
	int minVal = r;
	if (g < minVal) minVal = g;
	if (b < minVal) minVal = b;

	if (maxVal == 0)
		return 0.0;

	return (double)(maxVal - minVal) / (double)maxVal;
}

double
MathUtils::PerceptualColorDistance(unsigned char r1, unsigned char g1, unsigned char b1, unsigned char a1,
								   unsigned char r2, unsigned char g2, unsigned char b2, unsigned char a2)
{
	if (IsTransparent(a1) && IsTransparent(a2))
		return 0.0;

	if (IsTransparent(a1) || IsTransparent(a2))
		return MAX_DISTANCE;

	double dr = (double)r1 - (double)r2;
	double dg = (double)g1 - (double)g2;
	double db = (double)b1 - (double)b2;
	double da = (double)a1 - (double)a2;

	double meanR = ((double)r1 + (double)r2) * 0.5;

	double weightR = 2.0 + meanR / 256.0;
	double weightG = 4.0;
	double weightB = 2.0 + (255.0 - meanR) / 256.0;

	double rgbDist = std::sqrt(weightR * dr * dr + weightG * dg * dg + weightB * db * db);

	double sat1 = CalculateSaturation(r1, g1, b1);
	double sat2 = CalculateSaturation(r2, g2, b2);
	double satDiff = std::fabs(sat1 - sat2);

	double satPenalty = satDiff * 30.0;

	double alphaPenalty = std::fabs(da) * 1.5;

	int group1 = AlphaGroup(a1);
	int group2 = AlphaGroup(a2);
	if (group1 != group2) {
		int groupDist = std::abs(group1 - group2);
		alphaPenalty += groupDist * 150.0;
	}

	return rgbDist + satPenalty + alphaPenalty;
}

double
MathUtils::PerceptualColorDistanceForMerge(unsigned char r1, unsigned char g1, unsigned char b1, unsigned char a1,
										   unsigned char r2, unsigned char g2, unsigned char b2, unsigned char a2)
{
	if (IsTransparent(a1) && IsTransparent(a2))
		return 0.0;

	if (IsTransparent(a1) || IsTransparent(a2))
		return MAX_DISTANCE;

	int group1 = AlphaGroup(a1);
	int group2 = AlphaGroup(a2);
	if (group1 != group2)
		return MAX_DISTANCE;

	double dr = (double)r1 - (double)r2;
	double dg = (double)g1 - (double)g2;
	double db = (double)b1 - (double)b2;
	double da = (double)a1 - (double)a2;

	double meanR = ((double)r1 + (double)r2) * 0.5;

	double weightR = 2.0 + meanR / 256.0;
	double weightG = 4.0;
	double weightB = 2.0 + (255.0 - meanR) / 256.0;

	double rgbDist = std::sqrt(weightR * dr * dr + weightG * dg * dg + weightB * db * db);

	double sat1 = CalculateSaturation(r1, g1, b1);
	double sat2 = CalculateSaturation(r2, g2, b2);
	double satDiff = std::fabs(sat1 - sat2);

	double satPenalty = satDiff * 25.0;

	double alphaPenalty = std::fabs(da) * 1.2;

	return rgbDist + satPenalty + alphaPenalty;
}

double
MathUtils::AdaptiveThreshold(int paletteSize, double baseThreshold)
{
	if (paletteSize <= 8)
		return baseThreshold * 0.8;
	if (paletteSize <= 16)
		return baseThreshold * 0.9;
	if (paletteSize <= 32)
		return baseThreshold * 1.0;
	if (paletteSize <= 48)
		return baseThreshold * 1.15;
	return baseThreshold * 1.3;
}

int
MathUtils::GetSquare(int diff)
{
	Init();
	if (diff < -255) diff = -255;
	if (diff > 255) diff = 255;
	return sSquares[diff + 255];
}

int
MathUtils::GetShift(int level)
{
	Init();
	if (level < 0) level = 0;
	if (level > 8) level = 8;
	return sShift[level];
}
