/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "TracingOptions.h"

TracingOptions::TracingOptions()
{
	SetDefaults();
}

void
TracingOptions::SetDefaults()
{
	fLineThreshold = 2.0f;
	fQuadraticThreshold = 0.5f;
	fPathOmitThreshold = 10.0f;

	fNumberOfColors = 8.0f;
	fColorQuantizationCycles = 16.0f;

	fScale = 1.0f;
	fRoundCoordinates = 1.0f;
	fShowDescription = true;
	fCustomDescription = "";
	fUseViewBox = false;

	fBlurRadius = 0.0f;
	fBlurDelta = 20.0f;

	fRemoveBackground = false;
	fBackgroundMethod = AUTO;
	fBackgroundTolerance = 10;
	fMinBackgroundRatio = 0.3;

	fDouglasPeuckerEnabled = false;
	fDouglasPeuckerTolerance = 0.5f;
	fDouglasPeuckerCurveProtection = 0.5f;

	fDetectGeometry = false;
	fLineTolerance = 2.0f;
	fCircleTolerance = 5.0f;
	fMinCircleRadius = 3.0f;
	fMaxCircleRadius = 1000.0f;

	fOptimizeSvg = true;
	fRemoveDuplicates = true;

	fAggressiveSimplification = false;
	fCollinearTolerance = 1.0f;
	fMinSegmentLength = 2.0f;
	fCurveSmoothing = 0.0f;

	fFilterSmallObjects = true;
	fMinObjectArea = 10.0f;
	fMinObjectWidth = 3.0f;
	fMinObjectHeight = 3.0f;
	fMinObjectPerimeter = 12.0f;

	fVisvalingamWhyattEnabled = false;
	fVisvalingamWhyattTolerance = 1.0f;

	fDetectGradients = false;
	fGradientSampleStride = 2;
	fGradientMinR2 = 0.6f;
	fGradientMinDelta = 18.0f;
	fGradientMinSize = 6.0f;
	fGradientMaxSubdiv = 8;
	fGradientMinSamples = 40;

	fGradientMinR2Total = 0.72f;
	fGradientMinAlpha = 12;
	fGradientUseLinearRGB = true;

	fRegionMergeBoundaryColorTol = 18.0f;
	fRegionMergeAngleToleranceDeg = 30.0f;
	fRegionMergeMinBoundaryCount = 5;
	fRegionMergeUseLinearRGB = false;

	fKeepHolePaths = true;
	fMinHolePathRatio = 0.0f;

	fSpatialCoherence = true;
	fSpatialCoherenceRadius = 2;
	fSpatialCoherencePasses = 2;

	fProgressCallback = NULL;
	fProgressUserData = NULL;
}

void
TracingOptions::SetProgressCallback(ProgressCallback callback, void* userData)
{
	fProgressCallback = callback;
	fProgressUserData = userData;
}
