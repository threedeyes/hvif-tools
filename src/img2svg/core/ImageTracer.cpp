/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>

#include "ImageTracer.h"
#include "ColorQuantizer.h"
#include "PathScanner.h"
#include "PathTracer.h"
#include "PathSimplifier.h"
#include "GeometryDetector.h"
#include "SvgWriter.h"
#include "SelectiveBlur.h"
#include "BackgroundRemover.h"
#include "VisvalingamWhyatt.h"
#include "GradientDetector.h"
#include "RegionMerger.h"
#include "MathUtils.h"
#include "PathHierarchy.h"
#include "SharedEdgeRegistry.h"

ImageTracer::ImageTracer()
{
}

ImageTracer::~ImageTracer()
{
}

std::string
ImageTracer::BitmapToSvg(const BitmapData& bitmap, const TracingOptions& options)
{
	IndexedBitmap indexedBitmap = BitmapToTraceData(bitmap, options);
	SvgWriter svgWriter;
	std::string svgString = svgWriter.GenerateSvg(indexedBitmap, options);

	if (options.fOptimizeSvg) {
		svgString = svgWriter.OptimizeSvgString(svgString, options);
	}

	return svgString;
}

IndexedBitmap
ImageTracer::BitmapToTraceData(const BitmapData& bitmap, const TracingOptions& options)
{
	BitmapData processedBitmap = bitmap;

	if (options.fRemoveBackground) {
		BackgroundRemover remover;
		remover.SetColorTolerance(options.fBackgroundTolerance);
		remover.SetMinBackgroundRatio(options.fMinBackgroundRatio);
		processedBitmap = remover.RemoveBackground(processedBitmap,
												  options.fBackgroundMethod,
												  options.fBackgroundTolerance);
	}

	if (options.fBlurRadius > 0) {
		SelectiveBlur blur;
		processedBitmap = blur.BlurBitmap(processedBitmap,
										options.fBlurRadius,
										options.fBlurDelta);
	}

	std::vector<std::vector<unsigned char> > palette =
		_CreatePalette(processedBitmap, static_cast<int>(options.fNumberOfColors), options);

	ColorQuantizer quantizer;
	IndexedBitmap indexedBitmap = quantizer.QuantizeColors(processedBitmap, palette, options);

	if (options.fDetectGradients) {
		RegionMerger merger;
		indexedBitmap = merger.MergeRegions(indexedBitmap, processedBitmap, options);
	}

	PathScanner pathScanner;
	std::vector<std::vector<std::vector<int> > > rawLayers =
		pathScanner.CreateLayers(indexedBitmap);

	std::vector<std::vector<std::vector<std::vector<int> > > > batchPaths =
		pathScanner.ScanLayerPaths(rawLayers, options);

	std::vector<std::vector<std::vector<std::vector<double> > > > batchInternodes =
		pathScanner.CreateInternodes(batchPaths);

	if (options.fVisvalingamWhyattEnabled) {
		VisvalingamWhyatt vw;
		batchInternodes = vw.BatchSimplifyLayerInternodes(batchInternodes, options.fVisvalingamWhyattTolerance);
	}

	PathTracer tracer;
	std::vector<std::vector<std::vector<std::vector<double> > > > layers(batchInternodes.size());
	for (int k = 0; k < static_cast<int>(batchInternodes.size()); k++) {
		layers[k] = tracer.BatchTracePaths(batchInternodes[k],
										  options.fLineThreshold,
										  options.fQuadraticThreshold);
	}

	bool needSimplification = options.fFilterSmallObjects ||
							   options.fDouglasPeuckerEnabled ||
							   options.fCollinearTolerance > 0 ||
							   options.fMinSegmentLength > 0 ||
							   options.fCurveSmoothing > 0;

	SharedEdgeRegistry* masterRegistry = NULL;

	try {
		masterRegistry = new SharedEdgeRegistry(16.0);

		if (needSimplification) {
			if (options.fFilterSmallObjects) {
				PathSimplifier simplifier;
				layers = simplifier.BatchFilterSmallObjects(layers, options);
			}

			if (options.fDouglasPeuckerEnabled) {
				PathSimplifier simplifier;
				layers = simplifier.BatchLayerDouglasPeucker(layers, options);
			}

			if (options.fCollinearTolerance > 0 ||
				options.fMinSegmentLength > 0 ||
				options.fCurveSmoothing > 0) {

				masterRegistry->RegisterPaths(layers, indexedBitmap);
				masterRegistry->UnifyCoordinates(0.25);

				PathSimplifier simplifier;
				layers = simplifier.BatchTracePathsWithSimplification(layers, options, masterRegistry);
			}
		}

		if (options.fDetectGeometry) {
			GeometryDetector detector;
			layers = detector.BatchLayerGeometryDetection(layers, options);
		}

		masterRegistry->RegisterPaths(layers, indexedBitmap);
		masterRegistry->UnifyCoordinates(0.15);
		masterRegistry->UpdatePaths(layers);

		delete masterRegistry;
		masterRegistry = NULL;

	} catch (...) {
		delete masterRegistry;
		throw;
	}

	indexedBitmap.SetLayers(layers);

	PathHierarchy hierarchy;
	hierarchy.AnalyzeHierarchy(indexedBitmap);

	_FixWindingOrder(indexedBitmap);

	if (options.fDetectGradients) {
		GradientDetector grad;
		std::vector<std::vector<IndexedBitmap::LinearGradient> > grads =
			grad.DetectLinearGradients(indexedBitmap, processedBitmap, layers, options);
		indexedBitmap.SetLinearGradients(grads);
	}

	return indexedBitmap;
}

void
ImageTracer::_FixWindingOrder(IndexedBitmap& indexed)
{
	std::vector<std::vector<std::vector<std::vector<double> > > > layers = indexed.Layers();
	const std::vector<std::vector<IndexedBitmap::PathMetadata> >& metadata = indexed.PathsMetadata();

	if (metadata.empty())
		return;

	PathHierarchy hierarchy;

	for (size_t k = 0; k < layers.size(); k++) {
		if (k >= metadata.size()) continue;

		for (size_t i = 0; i < layers[k].size(); i++) {
			if (i >= metadata[k].size()) continue;
			if (layers[k][i].empty()) continue;

			double area = 0.0;
			for (size_t j = 0; j < layers[k][i].size(); j++) {
				const std::vector<double>& seg = layers[k][i][j];
				if (seg.size() < 4) continue;

				double x1 = seg[1];
				double y1 = seg[2];
				double x2 = (seg[0] == 1.0) ? seg[3] : seg[5];
				double y2 = (seg[0] == 1.0) ? seg[4] : seg[6];

				area += (x1 * y2 - x2 * y1);
			}

			bool isClockwise = (area < 0);
			bool shouldBeClockwise = !metadata[k][i].isHole;

			if (isClockwise != shouldBeClockwise) {
				hierarchy.ReversePathSegments(layers[k][i]);
			}
		}
	}

	indexed.SetLayers(layers);
}

bool
ImageTracer::SaveSvg(const std::string& filename, const std::string& svgData)
{
	std::ofstream file(filename.c_str());
	if (!file)
		return false;

	file << svgData;
	file.close();
	return true;
}

double
ImageTracer::_CalculateBrightness(unsigned char r, unsigned char g, unsigned char b)
{
	int maxVal = r;
	if (g > maxVal) maxVal = g;
	if (b > maxVal) maxVal = b;

	return (double)maxVal / 255.0;
}

int
ImageTracer::_FindNearestColorIndex(unsigned char r, unsigned char g, unsigned char b, unsigned char a,
									const std::vector<std::vector<unsigned char> >& palette,
									bool skipTransparent)
{
	double bestDistance = MathUtils::MAX_DISTANCE;
	int bestIndex = skipTransparent ? 1 : 0;

	int startIdx = (skipTransparent && !palette.empty() && MathUtils::IsTransparent(palette[0][3])) ? 1 : 0;

	for (int i = startIdx; i < static_cast<int>(palette.size()); i++) {
		if (MathUtils::IsTransparent(palette[i][3]))
			continue;

		double distance = MathUtils::PerceptualColorDistance(
			r, g, b, a,
			palette[i][0], palette[i][1], palette[i][2], palette[i][3]
		);

		if (distance < bestDistance) {
			bestDistance = distance;
			bestIndex = i;
		}
	}

	return bestIndex;
}

void
ImageTracer::_SelectRepresentativeColor(const std::vector<PixelSample>& samples,
										unsigned char& outR,
										unsigned char& outG,
										unsigned char& outB,
										unsigned char& outA,
										int colorCount)
{
	if (samples.empty()) {
		outR = outG = outB = 128;
		outA = 255;
		return;
	}

	std::vector<unsigned char> rValues, gValues, bValues, aValues;
	rValues.reserve(samples.size());
	gValues.reserve(samples.size());
	bValues.reserve(samples.size());
	aValues.reserve(samples.size());

	for (int i = 0; i < static_cast<int>(samples.size()); i++) {
		rValues.push_back(samples[i].r);
		gValues.push_back(samples[i].g);
		bValues.push_back(samples[i].b);
		aValues.push_back(samples[i].a);
	}

	std::sort(rValues.begin(), rValues.end());
	std::sort(gValues.begin(), gValues.end());
	std::sort(bValues.begin(), bValues.end());
	std::sort(aValues.begin(), aValues.end());

	int medianIdx = samples.size() / 2;
	unsigned char medianR = rValues[medianIdx];
	unsigned char medianG = gValues[medianIdx];
	unsigned char medianB = bValues[medianIdx];
	unsigned char medianA = aValues[medianIdx];

	double medianSat = MathUtils::CalculateSaturation(medianR, medianG, medianB);

	int highSatCount = 0;
	double highSatThreshold = MathUtils::AdaptiveThreshold(colorCount, 0.35) / 100.0;

	for (int i = 0; i < static_cast<int>(samples.size()); i++) {
		if (samples[i].saturation >= highSatThreshold) {
			highSatCount++;
		}
	}

	double highSatRatio = (double)highSatCount / (double)samples.size();

	if (medianSat < 0.15 && highSatRatio < 0.1) {
		outR = medianR;
		outG = medianG;
		outB = medianB;
		outA = medianA;
		return;
	}

	double weightSum = 0.0;
	double rSum = 0.0, gSum = 0.0, bSum = 0.0, aSum = 0.0;

	for (int i = 0; i < static_cast<int>(samples.size()); i++) {
		double satWeight = samples[i].saturation * samples[i].saturation;
		double brightnessBonus = 1.0;

		if (samples[i].brightness < 0.2) {
			brightnessBonus = 0.5;
		} else if (samples[i].brightness > 0.8) {
			brightnessBonus = 1.2;
		}

		double weight = (satWeight + 0.05) * brightnessBonus;

		weightSum += weight;
		rSum += samples[i].r * weight;
		gSum += samples[i].g * weight;
		bSum += samples[i].b * weight;
		aSum += samples[i].a * weight;
	}

	unsigned char weightedR = static_cast<unsigned char>(rSum / weightSum + 0.5);
	unsigned char weightedG = static_cast<unsigned char>(gSum / weightSum + 0.5);
	unsigned char weightedB = static_cast<unsigned char>(bSum / weightSum + 0.5);
	unsigned char weightedA = static_cast<unsigned char>(aSum / weightSum + 0.5);

	double blendFactor = 0.5 + (highSatRatio - 0.15) * 0.667;
	if (blendFactor < 0.5) blendFactor = 0.5;
	if (blendFactor > 0.75) blendFactor = 0.75;

	outR = static_cast<unsigned char>(medianR * (1.0 - blendFactor) + weightedR * blendFactor + 0.5);
	outG = static_cast<unsigned char>(medianG * (1.0 - blendFactor) + weightedG * blendFactor + 0.5);
	outB = static_cast<unsigned char>(medianB * (1.0 - blendFactor) + weightedB * blendFactor + 0.5);
	outA = static_cast<unsigned char>(medianA * (1.0 - blendFactor) + weightedA * blendFactor + 0.5);
}

std::vector<std::vector<unsigned char> >
ImageTracer::_CreatePalette(const BitmapData& bitmap, int colorCount, const TracingOptions& options)
{
	bool hasTransparency = false;

	for (int y = 0; y < bitmap.Height(); y++) {
		for (int x = 0; x < bitmap.Width(); x++) {
			unsigned char a = bitmap.GetPixelComponent(x, y, 3);
			if (MathUtils::IsTransparent(a)) {
				hasTransparency = true;
				break;
			}
		}
		if (hasTransparency) break;
	}

	std::vector<std::vector<int> > pixels(bitmap.Height());
	for (int y = 0; y < bitmap.Height(); y++) {
		pixels[y].resize(bitmap.Width());
		for (int x = 0; x < bitmap.Width(); x++) {
			int r = bitmap.GetPixelComponent(x, y, 0);
			int g = bitmap.GetPixelComponent(x, y, 1);
			int b = bitmap.GetPixelComponent(x, y, 2);
			int a = bitmap.GetPixelComponent(x, y, 3);

			if (MathUtils::IsTransparent((unsigned char)a)) {
				pixels[y][x] = -1;
				continue;
			}

			pixels[y][x] = (a << 24) | (r << 16) | (g << 8) | b;
		}
	}

	ColorQuantizer quantizer;
	std::vector<int> initialPalette = quantizer.QuantizeImageMasked(pixels, colorCount, -1);

	std::vector<std::vector<unsigned char> > bytePalette;
	for (int i = 0; i < static_cast<int>(initialPalette.size()); i++) {
		std::vector<unsigned char> color(4);
		color[0] = (initialPalette[i] >> 16) & 0xFF;
		color[1] = (initialPalette[i] >> 8) & 0xFF;
		color[2] = initialPalette[i] & 0xFF;
		color[3] = (initialPalette[i] >> 24) & 0xFF;
		bytePalette.push_back(color);
	}

	int maxIterations = static_cast<int>(options.fColorQuantizationCycles);
	if (maxIterations < 1) maxIterations = 1;
	if (maxIterations > 50) maxIterations = 50;

	double totalChangeHistory = 0.0;
	int consecutiveSmallChanges = 0;

	for (int iteration = 0; iteration < maxIterations; iteration++) {
		std::vector<std::vector<PixelSample> > colorSamples(bytePalette.size());

		for (int y = 0; y < bitmap.Height(); y++) {
			for (int x = 0; x < bitmap.Width(); x++) {
				if (pixels[y][x] == -1)
					continue;

				int r = bitmap.GetPixelComponent(x, y, 0);
				int g = bitmap.GetPixelComponent(x, y, 1);
				int b = bitmap.GetPixelComponent(x, y, 2);
				int a = bitmap.GetPixelComponent(x, y, 3);

				if (MathUtils::IsTransparent((unsigned char)a))
					continue;

				int bestIdx = _FindNearestColorIndex(r, g, b, a, bytePalette, false);

				PixelSample sample;
				sample.r = r;
				sample.g = g;
				sample.b = b;
				sample.a = a;
				sample.saturation = MathUtils::CalculateSaturation(r, g, b);
				sample.brightness = _CalculateBrightness(r, g, b);

				colorSamples[bestIdx].push_back(sample);
			}
		}

		double iterationChange = 0.0;

		for (int i = 0; i < static_cast<int>(bytePalette.size()); i++) {
			if (colorSamples[i].empty())
				continue;

			unsigned char oldR = bytePalette[i][0];
			unsigned char oldG = bytePalette[i][1];
			unsigned char oldB = bytePalette[i][2];
			unsigned char oldA = bytePalette[i][3];

			unsigned char newR, newG, newB, newA;
			_SelectRepresentativeColor(colorSamples[i], newR, newG, newB, newA, colorCount);

			double colorChange = MathUtils::PerceptualColorDistance(
				oldR, oldG, oldB, oldA,
				newR, newG, newB, newA
			);

			iterationChange += colorChange;

			bytePalette[i][0] = newR;
			bytePalette[i][1] = newG;
			bytePalette[i][2] = newB;
			bytePalette[i][3] = newA;
		}

		totalChangeHistory += iterationChange;

		double convergenceThreshold = MathUtils::AdaptiveThreshold(colorCount, 5.0);

		if (iterationChange < convergenceThreshold) {
			consecutiveSmallChanges++;
			if (consecutiveSmallChanges >= 2 && iteration >= 3) {
				break;
			}
		} else {
			consecutiveSmallChanges = 0;
		}

		if (iteration >= 5) {
			double avgChange = totalChangeHistory / (iteration + 1);
			if (iterationChange < avgChange * 0.05) {
				break;
			}
		}
	}

	std::vector<std::vector<unsigned char> > finalPalette;

	if (hasTransparency) {
		std::vector<unsigned char> transparentColor(4);
		transparentColor[0] = 0;
		transparentColor[1] = 0;
		transparentColor[2] = 0;
		transparentColor[3] = 0;
		finalPalette.push_back(transparentColor);
	}

	for (int i = 0; i < static_cast<int>(bytePalette.size()); i++) {
		finalPalette.push_back(bytePalette[i]);
	}

	return finalPalette;
}
