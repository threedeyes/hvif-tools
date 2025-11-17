/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef IMPORT_SVG_PARSER_H
#define IMPORT_SVG_PARSER_H

#include <string>
#include <vector>
#include "HaikuIcon.h"

struct NSVGimage;
struct NSVGshape;
struct NSVGpaint;
struct NSVGpath;

namespace haiku {

struct SVGParseOptions {
	float		targetSize;
	bool		preserveNames;
	bool		verbose;

	SVGParseOptions() : targetSize(64.0f), preserveNames(false), verbose(false) {}
};

class SVGParser {
public:
				SVGParser() {};
				~SVGParser() {};
	
	bool		Parse(const std::string& svgFile, Icon& icon, const SVGParseOptions& opts);
	bool		Parse(const std::string& svgFile, Icon& icon);
	bool		ParseString(const std::string& svg, Icon& icon, const SVGParseOptions& opts);
	bool		ParseString(const std::string& svg, Icon& icon);
	bool		ParseBuffer(const char* svgData, size_t dataSize, Icon& icon, const SVGParseOptions& opts);
	bool		ParseBuffer(const char* svgData, size_t dataSize, Icon& icon);

private:
	struct ParseState {
		float	scale;
		float	tx, ty;
		Icon*	icon;
		bool	verbose;

		ParseState() : scale(1.0f), tx(0.0f), ty(0.0f), icon(NULL), verbose(false) {}
	};

	bool		_ProcessImage(NSVGimage* image, Icon& icon, const SVGParseOptions& opts);
	void		_ProcessShape(NSVGshape* shape, ParseState& state);
	int			_ProcessPath(NSVGpath* path, ParseState& state);
	int			_AddStyle(const NSVGpaint& paint, float opacity, ParseState& state);
	Color		_NSVGColorToHaiku(unsigned int color, float opacity);
	void		_CalculateGradientTransform(const NSVGpaint& paint, Gradient& grad, const ParseState& state);
};

}

#endif
