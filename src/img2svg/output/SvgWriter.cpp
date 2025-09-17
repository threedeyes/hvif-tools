/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <cmath>
#include <iomanip>
#include <map>
#include <set>

#include "SvgWriter.h"

SvgWriter::SvgWriter()
{
}

SvgWriter::~SvgWriter()
{
}

float
SvgWriter::_RoundToDecimal(float value, float places)
{
	return static_cast<float>(floor(value * pow(10, places) + 0.5) / pow(10, places));
}

void
SvgWriter::_WriteSvgPathString(std::ostringstream& stream, const std::string& description,
							const std::vector<std::vector<double>>& segments,
							const std::string& colorString, const TracingOptions& options)
{
	float scale = options.fScale;
	float lineControlPointRadius = options.fLineControlPointRadius;
	float quadraticControlPointRadius = options.fQuadraticControlPointRadius;
	float roundCoordinates = floor(options.fRoundCoordinates);

	stream << "\n  <path " << description << colorString << "d=\"";
	stream << "M " << segments[0][1] * scale << " " << segments[0][2] * scale;

	if (roundCoordinates == -1) {
		for (int pointIndex = 0; pointIndex < static_cast<int>(segments.size()); pointIndex++) {
			if (segments[pointIndex][0] == 1.0) {
				stream << " L " << segments[pointIndex][3] * scale << " "
					   << segments[pointIndex][4] * scale;
			} else {
				stream << " Q " << segments[pointIndex][3] * scale << " "
					   << segments[pointIndex][4] * scale << " "
					   << segments[pointIndex][5] * scale << " "
					   << segments[pointIndex][6] * scale;
			}
		}
	} else {
		for (int pointIndex = 0; pointIndex < static_cast<int>(segments.size()); pointIndex++) {
			if (segments[pointIndex][0] == 1.0) {
				stream << " L " << _RoundToDecimal(static_cast<float>(segments[pointIndex][3] * scale), roundCoordinates) << " "
					   << _RoundToDecimal(static_cast<float>(segments[pointIndex][4] * scale), roundCoordinates);
			} else {
				stream << " Q " << _RoundToDecimal(static_cast<float>(segments[pointIndex][3] * scale), roundCoordinates) << " "
					   << _RoundToDecimal(static_cast<float>(segments[pointIndex][4] * scale), roundCoordinates) << " "
					   << _RoundToDecimal(static_cast<float>(segments[pointIndex][5] * scale), roundCoordinates) << " "
					   << _RoundToDecimal(static_cast<float>(segments[pointIndex][6] * scale), roundCoordinates);
			}
		}
	}

	stream << " Z\" />";

	// Debug control points
	for (int pointIndex = 0; pointIndex < static_cast<int>(segments.size()); pointIndex++) {
		if ((lineControlPointRadius > 0) && (segments[pointIndex][0] == 1.0)) {
			stream << "\n  <circle cx=\"" << segments[pointIndex][3] * scale
				   << "\" cy=\"" << segments[pointIndex][4] * scale
				   << "\" r=\"" << lineControlPointRadius
				   << "\" fill=\"white\" stroke-width=\"" << lineControlPointRadius * 0.2
				   << "\" stroke=\"black\" />";
		}
		if ((quadraticControlPointRadius > 0) && (segments[pointIndex][0] == 2.0)) {
			stream << "\n  <circle cx=\"" << segments[pointIndex][3] * scale
				   << "\" cy=\"" << segments[pointIndex][4] * scale
				   << "\" r=\"" << quadraticControlPointRadius
				   << "\" fill=\"cyan\" stroke-width=\"" << quadraticControlPointRadius * 0.2
				   << "\" stroke=\"black\" />";
			stream << "\n  <circle cx=\"" << segments[pointIndex][5] * scale
				   << "\" cy=\"" << segments[pointIndex][6] * scale
				   << "\" r=\"" << quadraticControlPointRadius
				   << "\" fill=\"white\" stroke-width=\"" << quadraticControlPointRadius * 0.2
				   << "\" stroke=\"black\" />";
			stream << "\n  <line x1=\"" << segments[pointIndex][1] * scale
				   << "\" y1=\"" << segments[pointIndex][2] * scale
				   << "\" x2=\"" << segments[pointIndex][3] * scale
				   << "\" y2=\"" << segments[pointIndex][4] * scale
				   << "\" stroke-width=\"" << quadraticControlPointRadius * 0.2
				   << "\" stroke=\"cyan\" />";
			stream << "\n  <line x1=\"" << segments[pointIndex][3] * scale
				   << "\" y1=\"" << segments[pointIndex][4] * scale
				   << "\" x2=\"" << segments[pointIndex][5] * scale
				   << "\" y2=\"" << segments[pointIndex][6] * scale
				   << "\" stroke-width=\"" << quadraticControlPointRadius * 0.2
				   << "\" stroke=\"cyan\" />";
		}
	}
}

std::string
SvgWriter::GenerateSvg(const IndexedBitmap& indexedBitmap, const TracingOptions& options)
{
	int width = static_cast<int>(indexedBitmap.Width() * options.fScale);
	int height = static_cast<int>(indexedBitmap.Height() * options.fScale);

	std::ostringstream svgStream;

	svgStream << "<?xml version=\"1.0\" standalone=\"no\"?>\n";
	svgStream << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20010904//EN\"\n";
	svgStream << "  \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">\n";

	svgStream << "<svg ";
	if (options.fUseViewBox) {
		svgStream << "viewBox=\"0 0 " << width << " " << height << "\"";
	} else {
		svgStream << "width=\"" << width << "\" height=\"" << height << "\"";
	}
	svgStream << " version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\"";

	if (options.fShowDescription) {
		std::string description = options.fCustomDescription.empty() ?
			"Created with img2svg version 1.0" : options.fCustomDescription;
		svgStream << "\n  desc=\"" << description << "\"";
	}
	svgStream << ">";

	std::map<double, std::pair<int, int>> zIndexMap;
	double label;

	for (int k = 0; k < static_cast<int>(indexedBitmap.Layers().size()); k++) {
		if (k < static_cast<int>(indexedBitmap.Palette().size()) && indexedBitmap.Palette()[k][3] == 0) {
			continue;
		}

		for (int pathIndex = 0; pathIndex < static_cast<int>(indexedBitmap.Layers()[k].size()); pathIndex++) {
			if (!indexedBitmap.Layers()[k][pathIndex].empty()) {
				if (!indexedBitmap.Layers()[k][pathIndex][0].empty()) {
					label = (indexedBitmap.Layers()[k][pathIndex][0][2] * width) + indexedBitmap.Layers()[k][pathIndex][0][1];
					zIndexMap[label] = std::make_pair(k, pathIndex);
				}
			}
		}
	}

	std::string description;
	for (std::map<double, std::pair<int, int>>::iterator it = zIndexMap.begin();
		 it != zIndexMap.end(); ++it) {
		int layer = it->second.first;
		int path = it->second.second;
		
		if (layer >= static_cast<int>(indexedBitmap.Palette().size())) {
			continue;
		}

		if (options.fShowDescription) {
			std::ostringstream descriptionStream;
			descriptionStream << "desc=\"l " << layer << " p " << path << "\" ";
			description = descriptionStream.str();
		} else {
			description = "";
		}

		_WriteSvgPathString(svgStream, description, indexedBitmap.Layers()[layer][path],
						   _ColorToSvgString(indexedBitmap.Palette()[layer]), options);
	}

	svgStream << "\n</svg>\n";
	return svgStream.str();
}

std::string
SvgWriter::_ColorToSvgString(const std::vector<unsigned char>& color)
{
	std::ostringstream stream;
	stream << std::hex << std::setfill('0');
	stream << "#" << std::setw(2) << static_cast<int>(color[0]) 
		   << std::setw(2) << static_cast<int>(color[1]) 
		   << std::setw(2) << static_cast<int>(color[2]);

	std::string hexColor = stream.str();
	std::ostringstream result;

	double opacity = static_cast<double>(color[3]) / 255.0;

	if (opacity < 0.01) {
		result << "fill=\"none\" stroke=\"none\" ";
	} else if (opacity < 0.99) {
		result << "fill=\"" << hexColor << "\" stroke=\"" << hexColor
			   << "\" stroke-width=\"1\" opacity=\"" << opacity << "\" ";
	} else {
		result << "fill=\"" << hexColor << "\" stroke=\"" << hexColor
			   << "\" stroke-width=\"1\" ";
	}

	return result.str();
}

std::string
SvgWriter::OptimizeSvgString(const std::string& svgString, const TracingOptions& options)
{
	if (!options.fOptimizeSvg) {
		return svgString;
	}

	std::string optimized = svgString;

	if (options.fRemoveDuplicates) {
		optimized = _RemoveDuplicatePaths(optimized);
	}

	optimized = _CompactSvgCommands(optimized);

	return optimized;
}

std::string
SvgWriter::_RemoveDuplicatePaths(const std::string& svgString)
{
	std::string result = svgString;
	std::set<std::string> foundPaths;

	size_t position = 0;
	while ((position = result.find("<path", position)) != std::string::npos) {
		size_t endPosition = result.find("/>", position);
		if (endPosition == std::string::npos) break;

		std::string pathTag = result.substr(position, endPosition - position + 2);

		size_t dStart = pathTag.find("d=\"");
		if (dStart != std::string::npos) {
			dStart += 3;
			size_t dEnd = pathTag.find("\"", dStart);
			if (dEnd != std::string::npos) {
				std::string dAttribute = pathTag.substr(dStart, dEnd - dStart);

				if (foundPaths.find(dAttribute) != foundPaths.end()) {
					result.erase(position, endPosition - position + 2);
					continue;
				} else {
					foundPaths.insert(dAttribute);
				}
			}
		}

		position = endPosition + 2;
	}

	return result;
}

std::string
SvgWriter::_CompactSvgCommands(const std::string& svgString)
{
	std::string result = svgString;

	size_t position = 0;
	while ((position = result.find("  ", position)) != std::string::npos) {
		result.replace(position, 2, " ");
	}

	return result;
}
