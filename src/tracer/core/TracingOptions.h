/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef TRACING_OPTIONS_H
#define TRACING_OPTIONS_H

#include <string>

#include "BackgroundRemover.h"
#include "VectorizationProgress.h"

class TracingOptions {
public:
							TracingOptions();

	void					SetDefaults();
	void					SetProgressCallback(ProgressCallback callback, void* userData);

	// Basic tracing parameters
	float					fLineThreshold;
	float					fQuadraticThreshold;
	float					fPathOmitThreshold;

	// Color quantization
	float					fNumberOfColors;
	float					fColorQuantizationCycles;

	// Output scaling and formatting
	float					fScale;
	float					fRoundCoordinates;
	bool					fShowDescription;
	std::string				fCustomDescription;
	bool					fUseViewBox;

	// Preprocessing
	float					fBlurRadius;
	float					fBlurDelta;

	// Background removal
	bool					fRemoveBackground;
	BackgroundDetectionMethod fBackgroundMethod;
	int						fBackgroundTolerance;
	double					fMinBackgroundRatio;

	// Douglas-Peucker simplification
	bool					fDouglasPeuckerEnabled;
	float					fDouglasPeuckerTolerance;
	float					fDouglasPeuckerCurveProtection;

	// Geometry detection
	bool					fDetectGeometry;
	float					fLineTolerance;
	float					fCircleTolerance;
	float					fMinCircleRadius;
	float					fMaxCircleRadius;

	// SVG optimization
	bool					fOptimizeSvg;
	bool					fRemoveDuplicates;

	// Advanced simplification
	bool					fAggressiveSimplification;
	float					fCollinearTolerance;
	float					fMinSegmentLength;
	float					fCurveSmoothing;

	// Small object filtering
	bool					fFilterSmallObjects;
	float					fMinObjectArea;
	float					fMinObjectWidth;
	float					fMinObjectHeight;
	float					fMinObjectPerimeter;

	// Visvalingam-Whyatt simplification
	bool					fVisvalingamWhyattEnabled;
	float					fVisvalingamWhyattTolerance;

	// Gradient detection (base)
	bool					fDetectGradients;
	int						fGradientSampleStride;
	float					fGradientMinR2;
	float					fGradientMinDelta;
	float					fGradientMinSize;
	int						fGradientMaxSubdiv;
	int						fGradientMinSamples;

	// Gradient detection (improved)
	float					fGradientMinR2Total;
	int						fGradientMinAlpha;
	bool					fGradientUseLinearRGB;

	// Gradient detection (region merging)
	float					fRegionMergeBoundaryColorTol;
	float					fRegionMergeAngleToleranceDeg;
	int						fRegionMergeMinBoundaryCount;
	bool					fRegionMergeUseLinearRGB;

	// Path scanning
	bool					fKeepHolePaths;
	float					fMinHolePathRatio;

	// Spatial coherence
	bool					fSpatialCoherence;
	int						fSpatialCoherenceRadius;
	int						fSpatialCoherencePasses;

	// Progress callback
	ProgressCallback		fProgressCallback;
	void*					fProgressUserData;
};

#endif
