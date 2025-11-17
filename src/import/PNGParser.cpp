/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <iostream>
#include <fstream>

#include "PNGParser.h"
#include "SVGParser.h"
#include "ImageTracer.h"

#ifdef __HAIKU__
#include <Application.h>
#include <TranslationUtils.h>
#include <Bitmap.h>
#include <BitmapStream.h>
#include <File.h>
#include <TranslatorRoster.h>
#include <DataIO.h>
#include <String.h>
#else
#include "stb_image.h"
#endif

namespace haiku {

PNGParser::PNGParser()
{
#ifdef __HAIKU__
	if (be_app == NULL)
		be_app = new BApplication("application/x-vnd.hvif-tools");
#endif
}

PNGParser::~PNGParser()
{
}

bool
PNGParser::Parse(const std::string& file, Icon& icon)
{
	PNGParseOptions opts;
	return Parse(file, icon, opts);
}

bool
PNGParser::Parse(const std::string& file, Icon& icon, const PNGParseOptions& opts)
{
	fLastError.clear();

	BitmapData bitmap = _LoadBitmapFromFile(file);
	if (!bitmap.IsValid()) {
		fLastError = "Failed to load PNG file: " + file;
		return false;
	}

	TracingOptions tracingOpts = _CreateTracingOptions(opts);
	
	ImageTracer tracer;
	std::string svgString = tracer.BitmapToSvg(bitmap, tracingOpts);
	
	if (svgString.empty()) {
		fLastError = "Vectorization failed";
		return false;
	}

	SVGParser svgParser;
	SVGParseOptions svgOpts;
	svgOpts.targetSize = 64.0f;
	svgOpts.preserveNames = false;
	svgOpts.verbose = opts.verbose;

	if (!svgParser.ParseString(svgString, icon, svgOpts)) {
		fLastError = "SVG parsing failed after vectorization";
		return false;
	}

	icon.filename = file;
	return true;
}

bool
PNGParser::ParseBuffer(const std::vector<uint8_t>& data, Icon& icon)
{
	PNGParseOptions opts;
	return ParseBuffer(data, icon, opts);
}

bool
PNGParser::ParseBuffer(const std::vector<uint8_t>& data, Icon& icon, const PNGParseOptions& opts)
{
	fLastError.clear();

	BitmapData bitmap = _LoadBitmapFromBuffer(data);
	if (!bitmap.IsValid()) {
		fLastError = "Failed to load PNG from buffer";
		return false;
	}

	TracingOptions tracingOpts = _CreateTracingOptions(opts);
	
	ImageTracer tracer;
	std::string svgString = tracer.BitmapToSvg(bitmap, tracingOpts);
	
	if (svgString.empty()) {
		fLastError = "Vectorization failed";
		return false;
	}

	SVGParser svgParser;
	SVGParseOptions svgOpts;
	svgOpts.targetSize = 64.0f;
	svgOpts.preserveNames = false;
	svgOpts.verbose = opts.verbose;

	if (!svgParser.ParseString(svgString, icon, svgOpts)) {
		fLastError = "SVG parsing failed after vectorization";
		return false;
	}

	icon.filename = "<from buffer>";
	return true;
}

TracingOptions
PNGParser::_CreateTracingOptions(const PNGParseOptions& opts)
{
	TracingOptions tracingOpts;

	switch (opts.preset) {
		case PRESET_ICON:
			tracingOpts = _GetIconPreset();
			break;
		case PRESET_ICON_GRADIENT:
			tracingOpts = _GetIconGradientPreset();
			break;
		default:
			tracingOpts = _GetIconPreset();
			break;
	}

	if (opts.removeBackground) {
		tracingOpts.fRemoveBackground = true;
		tracingOpts.fBackgroundMethod = AUTO;
		tracingOpts.fBackgroundTolerance = 10;
	}

	if (opts.verbose) {
		tracingOpts.fProgressCallback = NULL;
		tracingOpts.fProgressUserData = NULL;
	}

	return tracingOpts;
}

TracingOptions
PNGParser::_GetIconPreset()
{
	TracingOptions opts;
	
	opts.fNumberOfColors = 8.0f;
	opts.fColorQuantizationCycles = 16.0f;
	
	opts.fRemoveBackground = false;
	
	opts.fBlurRadius = 0.5f;
	opts.fBlurDelta = 20.0f;
	
	opts.fAggressiveSimplification = true;
	opts.fCollinearTolerance = 0.5f;
	opts.fMinSegmentLength = 1.5f;
	opts.fCurveSmoothing = 0.0f;
	
	opts.fDouglasPeuckerEnabled = true;
	opts.fDouglasPeuckerTolerance = 0.3f;
	opts.fDouglasPeuckerCurveProtection = 0.5f;
	
	opts.fVisvalingamWhyattEnabled = true;
	opts.fVisvalingamWhyattTolerance = 0.8f;
	
	opts.fDetectGeometry = true;
	opts.fLineTolerance = 1.5f;
	opts.fCircleTolerance = 3.0f;
	opts.fMinCircleRadius = 2.0f;
	opts.fMaxCircleRadius = 1000.0f;
	
	opts.fFilterSmallObjects = true;
	opts.fMinObjectArea = 4.0f;
	opts.fMinObjectWidth = 2.0f;
	opts.fMinObjectHeight = 2.0f;
	opts.fMinObjectPerimeter = 8.0f;
	
	opts.fDetectGradients = false;
	
	opts.fOptimizeSvg = true;
	opts.fRemoveDuplicates = true;
	
	opts.fLineThreshold = 2.0f;
	opts.fQuadraticThreshold = 0.5f;
	opts.fPathOmitThreshold = 8.0f;
	
	opts.fSpatialCoherence = true;
	opts.fSpatialCoherenceRadius = 2;
	opts.fSpatialCoherencePasses = 2;
	
	return opts;
}

TracingOptions
PNGParser::_GetIconGradientPreset()
{
	TracingOptions opts;
	
	opts.fNumberOfColors = 16.0f;
	opts.fColorQuantizationCycles = 20.0f;
	
	opts.fRemoveBackground = false;
	
	opts.fBlurRadius = 1.0f;
	opts.fBlurDelta = 25.0f;
	
	opts.fAggressiveSimplification = false;
	opts.fCollinearTolerance = 1.0f;
	opts.fMinSegmentLength = 2.0f;
	opts.fCurveSmoothing = 0.2f;
	
	opts.fDouglasPeuckerEnabled = true;
	opts.fDouglasPeuckerTolerance = 0.5f;
	opts.fDouglasPeuckerCurveProtection = 0.8f;
	
	opts.fVisvalingamWhyattEnabled = true;
	opts.fVisvalingamWhyattTolerance = 1.2f;
	
	opts.fDetectGeometry = true;
	opts.fLineTolerance = 2.0f;
	opts.fCircleTolerance = 4.0f;
	opts.fMinCircleRadius = 3.0f;
	opts.fMaxCircleRadius = 1000.0f;
	
	opts.fFilterSmallObjects = true;
	opts.fMinObjectArea = 6.0f;
	opts.fMinObjectWidth = 2.5f;
	opts.fMinObjectHeight = 2.5f;
	opts.fMinObjectPerimeter = 10.0f;
	
	opts.fDetectGradients = true;
	opts.fGradientSampleStride = 2;
	opts.fGradientMinR2 = 0.65f;
	opts.fGradientMinDelta = 15.0f;
	opts.fGradientMinSize = 8.0f;
	
	opts.fOptimizeSvg = true;
	opts.fRemoveDuplicates = true;
	
	opts.fLineThreshold = 2.0f;
	opts.fQuadraticThreshold = 0.8f;
	opts.fPathOmitThreshold = 10.0f;
	
	opts.fSpatialCoherence = true;
	opts.fSpatialCoherenceRadius = 2;
	opts.fSpatialCoherencePasses = 2;
	
	return opts;
}

BitmapData
PNGParser::_LoadBitmapFromFile(const std::string& file)
{
#ifdef __HAIKU__
	BFile bfile(file.c_str(), B_READ_ONLY);
	if (bfile.InitCheck() != B_OK) {
		return BitmapData();
	}

	BTranslatorRoster* roster = BTranslatorRoster::Default();
	if (!roster) {
		return BitmapData();
	}

	BBitmapStream stream;
	status_t result = roster->Translate(&bfile, NULL, NULL, &stream, B_TRANSLATOR_BITMAP);
	if (result != B_OK) {
		return BitmapData();
	}

	BBitmap* bitmap = NULL;
	stream.DetachBitmap(&bitmap);
	
	if (!bitmap) {
		return BitmapData();
	}

	BitmapData bitmapData = _ConvertBBitmapToBitmapData(bitmap);
	delete bitmap;
	return bitmapData;
#else
	int width, height, channels;
	unsigned char* data = stbi_load(file.c_str(), &width, &height, &channels, 4);

	if (!data) {
		return BitmapData();
	}

	std::vector<unsigned char> bitmapData(data, data + (width * height * 4));
	stbi_image_free(data);

	return BitmapData(width, height, bitmapData);
#endif
}

BitmapData
PNGParser::_LoadBitmapFromBuffer(const std::vector<uint8_t>& buffer)
{
#ifdef __HAIKU__
	BMemoryIO memIO((void*)&buffer[0], buffer.size());
	
	BTranslatorRoster* roster = BTranslatorRoster::Default();
	if (!roster) {
		return BitmapData();
	}

	BBitmapStream stream;
	status_t result = roster->Translate(&memIO, NULL, NULL, &stream, B_TRANSLATOR_BITMAP);
	if (result != B_OK) {
		return BitmapData();
	}

	BBitmap* bitmap = NULL;
	stream.DetachBitmap(&bitmap);
	
	if (!bitmap) {
		return BitmapData();
	}

	BitmapData bitmapData = _ConvertBBitmapToBitmapData(bitmap);
	delete bitmap;
	return bitmapData;
#else
	int width, height, channels;
	unsigned char* data = stbi_load_from_memory(&buffer[0], buffer.size(), 
		&width, &height, &channels, 4);

	if (!data) {
		return BitmapData();
	}

	std::vector<unsigned char> bitmapData(data, data + (width * height * 4));
	stbi_image_free(data);

	return BitmapData(width, height, bitmapData);
#endif
}

#ifdef __HAIKU__
BitmapData
PNGParser::_ConvertBBitmapToBitmapData(BBitmap* bitmap)
{
	if (!bitmap || !bitmap->IsValid()) {
		return BitmapData();
	}

	BRect bounds = bitmap->Bounds();
	int32 width = bounds.IntegerWidth() + 1;
	int32 height = bounds.IntegerHeight() + 1;

	std::vector<unsigned char> data;
	data.resize(width * height * 4);

	color_space colorSpace = bitmap->ColorSpace();
	const uint8* bits = (const uint8*)bitmap->Bits();
	int32 bpr = bitmap->BytesPerRow();

	for (int32 y = 0; y < height; y++) {
		for (int32 x = 0; x < width; x++) {
			int dstOffset = (y * width + x) * 4;
			int srcOffset = y * bpr + x * 4;

			if (colorSpace == B_RGBA32 || colorSpace == B_RGBA32_BIG) {
				data[dstOffset + 0] = bits[srcOffset + 2];
				data[dstOffset + 1] = bits[srcOffset + 1];
				data[dstOffset + 2] = bits[srcOffset + 0];
				data[dstOffset + 3] = bits[srcOffset + 3];
			} else if (colorSpace == B_RGB32 || colorSpace == B_RGB32_BIG) {
				data[dstOffset + 0] = bits[srcOffset + 2];
				data[dstOffset + 1] = bits[srcOffset + 1];
				data[dstOffset + 2] = bits[srcOffset + 0];
				data[dstOffset + 3] = 255;
			} else {
				data[dstOffset + 0] = 0;
				data[dstOffset + 1] = 0;
				data[dstOffset + 2] = 0;
				data[dstOffset + 3] = 255;
			}
		}
	}

	return BitmapData(width, height, data);
}
#endif

}
