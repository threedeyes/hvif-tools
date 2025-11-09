/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef SVG_PARSER_IOM_H
#define SVG_PARSER_IOM_H

#include <string>
#include <vector>
#include "IOMStructures.h"

struct NSVGimage;
struct NSVGshape;
struct NSVGpaint;
struct NSVGpath;

namespace iom {

class SVGParser {
public:
				SVGParser();
				~SVGParser();

	bool		ParseFile(const std::string& svgFile, Icon& icon);
	bool		ParseString(const std::string& svgString, Icon& icon);
	bool		ParseBuffer(const char* svgData, size_t dataSize, Icon& icon);

private:
	struct ConvertState {
		float scale;
		float tx, ty;
		Icon* icon;
		
		ConvertState() : scale(1.0f), tx(0.0f), ty(0.0f), icon(NULL) {}
	};

	bool		_ProcessImage(NSVGimage* image, Icon& icon);
	void		_ProcessShape(NSVGshape* shape, ConvertState& state);
	int			_ProcessPath(NSVGpath* path, ConvertState& state);
	int			_AddStyle(const NSVGpaint& paint, float opacity, ConvertState& state);
	uint32_t	_NSVGColorToIOM(unsigned int color, float opacity);
	void		_CalculateGradientTransform(const NSVGpaint& paint, Gradient& grad, const ConvertState& state);
	void		_InvertAffine(float out[6], const float in[6]);
};

}

#endif
