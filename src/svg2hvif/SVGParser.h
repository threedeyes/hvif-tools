/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef SVG_PARSER_H
#define SVG_PARSER_H

#include <string>
#include <vector>

#include "HVIFWriter.h"
#include "HVIFStructures.h"

struct NSVGimage;
struct NSVGshape;
struct NSVGpaint;
struct NSVGpath;

namespace hvif {

class SVGParser {
public:
	SVGParser() : fVerbose(false) {}

	bool ParseFile(const std::string& svgFile, hvif::HVIFWriter& writer);

	bool ParseBuffer(const char* svgData, size_t dataSize, hvif::HVIFWriter& writer);
	bool ParseBuffer(const std::vector<uint8_t>& svgData, hvif::HVIFWriter& writer);
	bool ParseString(const std::string& svgString, hvif::HVIFWriter& writer);

	void SetVerbose(bool verbose) { fVerbose = verbose; }

private:
	struct ParseState {
		float scale;
		float tx, ty;
		hvif::HVIFWriter* writer;
		std::vector<hvif::Style> styles;
		
		ParseState() : scale(1.0f), tx(0.0f), ty(0.0f), writer(NULL) {}
	};

	bool fVerbose;

	bool _ProcessImage(NSVGimage* image, hvif::HVIFWriter& writer);
	void _ProcessShape(NSVGshape* shape, ParseState& state);
	uint8_t _ProcessPath(NSVGpath* path, ParseState& state);
	uint8_t _AddStyle(const NSVGpaint& paint, float opacity, ParseState& state);
	void _InvertAffine(float out[6], const float in[6]);	
	void _CalculateGradientTransform(const NSVGpaint& paint, hvif::Gradient& grad, const ParseState& state);
};

}

#endif
