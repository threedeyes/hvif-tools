/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <iostream>
#include <fstream>

#include "ImageTracer.h"
#include "ColorQuantizer.h"
#include "PathScanner.h"
#include "PathTracer.h"
#include "PathSimplifier.h"
#include "GeometryDetector.h"
#include "SvgWriter.h"
#include "SelectiveBlur.h"
#include "BackgroundRemover.h"

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

    std::vector<std::vector<unsigned char>> palette =
        _CreatePalette(processedBitmap, static_cast<int>(options.fNumberOfColors));

    ColorQuantizer quantizer;
    IndexedBitmap indexedBitmap = quantizer.QuantizeColors(processedBitmap, palette, options);

    PathScanner pathScanner;
    std::vector<std::vector<std::vector<int>>> rawLayers =
        pathScanner.CreateLayers(indexedBitmap);

    std::vector<std::vector<std::vector<std::vector<int>>>> batchPaths =
        pathScanner.ScanLayerPaths(rawLayers, static_cast<int>(options.fPathOmitThreshold));

    std::vector<std::vector<std::vector<std::vector<double>>>> batchInternodes =
        pathScanner.CreateInternodes(batchPaths);

    PathTracer tracer;
    std::vector<std::vector<std::vector<std::vector<double>>>> layers(batchInternodes.size());
    for (int k = 0; k < static_cast<int>(batchInternodes.size()); k++) {
        layers[k] = tracer.BatchTracePaths(batchInternodes[k], 
                                          options.fLineThreshold, 
                                          options.fQuadraticThreshold);
    }

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
        PathSimplifier simplifier;
        layers = simplifier.BatchTracePathsWithSimplification(layers, options);
    }

    if (options.fDetectGeometry) {
        GeometryDetector detector;
        layers = detector.BatchLayerGeometryDetection(layers, options);
    }

    indexedBitmap.SetLayers(layers);
    return indexedBitmap;
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

std::vector<std::vector<unsigned char>>
ImageTracer::_CreatePalette(const BitmapData& bitmap, int colorCount)
{
	std::vector<std::vector<int>> pixels(bitmap.Height());
	for (int y = 0; y < bitmap.Height(); y++) {
		pixels[y].resize(bitmap.Width());
		for (int x = 0; x < bitmap.Width(); x++) {
			int red   = bitmap.GetPixelComponent(x, y, 0);
			int green = bitmap.GetPixelComponent(x, y, 1);
			int blue  = bitmap.GetPixelComponent(x, y, 2);
			int alpha = bitmap.GetPixelComponent(x, y, 3);
			pixels[y][x] = (alpha << 24) | (red << 16) | (green << 8) | blue;
		}
	}

	ColorQuantizer quantizer;
	std::vector<int> palette = quantizer.QuantizeImage(pixels, colorCount);
	std::vector<std::vector<unsigned char>> bytePalette;

	bool hasTransparency = false;
	for (int y = 0; y < bitmap.Height() && !hasTransparency; y++) {
		for (int x = 0; x < bitmap.Width(); x++) {
			if (bitmap.GetPixelComponent(x, y, 3) == 0) {
				hasTransparency = true;
				break;
			}
		}
	}

	if (hasTransparency) {
		std::vector<unsigned char> transparentColor(4);
		transparentColor[0] = 0;  // R
		transparentColor[1] = 0;  // G
		transparentColor[2] = 0;  // B
		transparentColor[3] = 0;  // A
		bytePalette.push_back(transparentColor);
	}

	for (int i = 0; i < static_cast<int>(palette.size()); i++) {
		std::vector<unsigned char> color(4);
		color[0] = (palette[i] >> 16) & 0xFF;  // R
		color[1] = (palette[i] >> 8) & 0xFF;   // G
		color[2] = palette[i] & 0xFF;          // B
		color[3] = (palette[i] >> 24) & 0xFF;  // A

		if (hasTransparency && color[3] == 0)
			continue;

		bytePalette.push_back(color);
	}

	return bytePalette;
}
