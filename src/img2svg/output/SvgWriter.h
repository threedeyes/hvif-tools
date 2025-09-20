/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef SVG_WRITER_H
#define SVG_WRITER_H

#include <string>
#include <sstream>

#include "IndexedBitmap.h"
#include "TracingOptions.h"

class SvgWriter {
public:
							SvgWriter();
							~SvgWriter();

	std::string             GenerateSvg(const IndexedBitmap& indexedBitmap, 
										const TracingOptions& options);

	std::string             OptimizeSvgString(const std::string& svgString,
										const TracingOptions& options);

private:
	float                   _RoundToDecimal(float value, float places);

	void                    _WriteSvgPathString(std::ostringstream& stream,
										const std::string& description,
										const std::vector<std::vector<double>>& segments,
										const std::string& fillPaint,
										float fillOpacity,
										const TracingOptions& options);

	std::string             _ColorToSvgString(const std::vector<unsigned char>& color);
	std::string             _RemoveDuplicatePaths(const std::string& svgString);
	std::string             _CompactSvgCommands(const std::string& svgString);

	void                    _WriteLinearGradientDef(std::ostringstream& defs,
										const IndexedBitmap::LinearGradient& g,
										const std::string& id);

	std::string             _HexColor(unsigned char r, unsigned char g, unsigned char b);
};

#endif // SVG_WRITER_H
