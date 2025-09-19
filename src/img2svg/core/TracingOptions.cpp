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
	fLineThreshold = 1.0f;
	fQuadraticThreshold = 1.0f;
	fPathOmitThreshold = 10.0f;

	fNumberOfColors = 8.0f;
	fColorQuantizationCycles = 10.0f;

	fScale = 1.0f;
	fRoundCoordinates = 1.0f;
	fLineControlPointRadius = 0.0f;
	fQuadraticControlPointRadius = 0.0f;
	fShowDescription = true;
	fCustomDescription = "";
	fUseViewBox = false;

	fBlurRadius = 0.0f;
	fBlurDelta = 20.0f;

	fRemoveBackground = false;
	fBackgroundMethod = BackgroundDetectionMethod::COMBINED;
	fBackgroundTolerance = 10;
	fMinBackgroundRatio = 0.3;

	fDouglasPeuckerEnabled = false;
	fDouglasPeuckerTolerance = 2.0f;
	fDouglasPeuckerCurveProtection = 0.5f;

	fDetectGeometry = false;
	fLineTolerance = 2.0f;
	fCircleTolerance = 1.5f;
	fMinCircleRadius = 3.0f;
	fMaxCircleRadius = 1000.0f;

	fOptimizeSvg = true;
	fRemoveDuplicates = true;

	fAggressiveSimplification = false;
	fCollinearTolerance = 1.0f;
	fMinSegmentLength = 2.0f;
	fCurveSmoothing = 0.0f;

	fFilterSmallObjects = false;
	fMinObjectArea = 10.0f;
	fMinObjectWidth = 3.0f;
	fMinObjectHeight = 3.0f;
	fMinObjectPerimeter = 12.0f;

	fVisvalingamWhyattEnabled = false;
	fVisvalingamWhyattTolerance = 1.0f;
}
