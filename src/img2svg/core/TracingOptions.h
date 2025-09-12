/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef TRACING_OPTIONS_H
#define TRACING_OPTIONS_H

class TracingOptions {
public:
							TracingOptions();

	void					SetDefaults();

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
	float					fLineControlPointRadius;
	float					fQuadraticControlPointRadius;
	bool					fShowDescription;
	bool					fUseViewBox;

	// Preprocessing
	float					fBlurRadius;
	float					fBlurDelta;

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
};

#endif // TRACING_OPTIONS_H
