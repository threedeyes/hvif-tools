/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <iostream>

#include "SvgWriter.h"
#include "MathUtils.h"

static inline bool _NearlyEqual(double a, double b, double eps)
{
	return std::fabs(a - b) < eps;
}

static inline bool _SamePoint(double x1, double y1, double x2, double y2, double eps)
{
	return _NearlyEqual(x1, x2, eps) && _NearlyEqual(y1, y2, eps);
}

SvgWriter::SvgWriter()
{
}

SvgWriter::~SvgWriter()
{
}

std::string
SvgWriter::_HexColor(unsigned char r, unsigned char g, unsigned char b)
{
	static const char* hexdig = "0123456789ABCDEF";
	char buf[8];
	buf[0] = '#';
	buf[1] = hexdig[(r >> 4) & 0xF];
	buf[2] = hexdig[r & 0xF];
	buf[3] = hexdig[(g >> 4) & 0xF];
	buf[4] = hexdig[g & 0xF];
	buf[5] = hexdig[(b >> 4) & 0xF];
	buf[6] = hexdig[b & 0xF];
	buf[7] = '\0';
	return std::string(buf);
}

std::string
SvgWriter::_ColorToSvgString(const std::vector<unsigned char>& color, double strokeWidth)
{
	std::ostringstream stream;
	stream << std::hex << std::setfill('0');
	stream << "#" << std::setw(2) << static_cast<int>(color[0]) 
		   << std::setw(2) << static_cast<int>(color[1]) 
		   << std::setw(2) << static_cast<int>(color[2]);

	std::string hexColor = stream.str();
	std::ostringstream result;

	double opacity = static_cast<double>(color[3]) / 255.0;

	if (color[3] == 0) {
		result << "fill=\"none\" stroke=\"none\" ";
	} else if (color[3] < 255) {
		result << "fill=\"" << hexColor << "\" stroke=\"none\" ";
		if (opacity < 0.999) {
			result << "opacity=\"" << opacity << "\" ";
		}
	} else {
		result << "fill=\"" << hexColor << "\" stroke=\"" << hexColor << "\" "
			   << "stroke-width=\"1.5\" paint-order=\"stroke\" "
			   << "stroke-linejoin=\"round\" stroke-linecap=\"round\" ";
	}

	return result.str();
}

float
SvgWriter::_RoundToDecimal(float value, float places)
{
	return static_cast<float>(floor(value * pow(10, places) + 0.5) / pow(10, places));
}

void
SvgWriter::_WriteSvgPathString(std::ostringstream& stream,
								const std::string& description,
								const std::vector<std::vector<double> >& segments,
								const std::string& fillPaint,
								const TracingOptions& options)
{
	if (segments.empty())
		return;

	const double eps = 1e-6;
	std::vector<std::vector<double> > pts;
	pts.reserve(segments.size() + 1);

	std::vector<double> p0(2);
	p0[0] = segments[0][1];
	p0[1] = segments[0][2];
	pts.push_back(p0);

	for (int i = 0; i < static_cast<int>(segments.size()); i++) {
		double ex = (segments[i][0] == 1.0) ? segments[i][3] : segments[i][5];
		double ey = (segments[i][0] == 1.0) ? segments[i][4] : segments[i][6];

		if (!_SamePoint(pts.back()[0], pts.back()[1], ex, ey, eps)) {
			std::vector<double> pe(2);
			pe[0] = ex; pe[1] = ey;
			pts.push_back(pe);
		}
	}

	int uniqueCount = 0;
	for (int i = 0; i < static_cast<int>(pts.size()); i++) {
		bool dup = false;
		for (int j = 0; j < i; j++) {
			if (_SamePoint(pts[i][0], pts[i][1], pts[j][0], pts[j][1], eps)) {
				dup = true;
				break;
			}
		}
		if (!dup)
			uniqueCount++;
	}

	if (uniqueCount < 3)
		return;

	float scale = options.fScale;
	float roundCoordinates = floor(options.fRoundCoordinates);

	stream << "\n  <path " << description << fillPaint
		   << "d=\"";

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
}

void
SvgWriter::_WriteSvgCompoundPath(std::ostringstream& stream,
								const std::string& description,
								const std::vector<std::vector<std::vector<double> > >& allPaths,
								const std::vector<int>& pathIndices,
								const std::string& fillPaint,
								const TracingOptions& options)
{
	if (pathIndices.empty())
		return;

	float scale = options.fScale;
	float roundCoordinates = floor(options.fRoundCoordinates);

	stream << "\n  <path " << description << fillPaint
		   << "fill-rule=\"evenodd\" d=\"";

	for (size_t g = 0; g < pathIndices.size(); g++) {
		int pathIdx = pathIndices[g];

		if (pathIdx < 0 || pathIdx >= static_cast<int>(allPaths.size()))
			continue;

		const std::vector<std::vector<double> >& segments = allPaths[pathIdx];

		if (segments.empty())
			continue;

		const double eps = 1e-6;
		std::vector<std::vector<double> > pts;
		pts.reserve(segments.size() + 1);

		std::vector<double> p0(2);
		p0[0] = segments[0][1];
		p0[1] = segments[0][2];
		pts.push_back(p0);

		for (int i = 0; i < static_cast<int>(segments.size()); i++) {
			double ex = (segments[i][0] == 1.0) ? segments[i][3] : segments[i][5];
			double ey = (segments[i][0] == 1.0) ? segments[i][4] : segments[i][6];

			if (!_SamePoint(pts.back()[0], pts.back()[1], ex, ey, eps)) {
				std::vector<double> pe(2);
				pe[0] = ex; pe[1] = ey;
				pts.push_back(pe);
			}
		}

		int uniqueCount = 0;
		for (int i = 0; i < static_cast<int>(pts.size()); i++) {
			bool dup = false;
			for (int j = 0; j < i; j++) {
				if (_SamePoint(pts[i][0], pts[i][1], pts[j][0], pts[j][1], eps)) {
					dup = true;
					break;
				}
			}
			if (!dup)
				uniqueCount++;
		}

		if (uniqueCount < 3)
			continue;

		if (g > 0) stream << " ";

		stream << "M " << segments[0][1] * scale << " " << segments[0][2] * scale;

		if (roundCoordinates == -1) {
			for (int j = 0; j < static_cast<int>(segments.size()); j++) {
				if (segments[j][0] == 1.0) {
					stream << " L " << segments[j][3] * scale << " "
						   << segments[j][4] * scale;
				} else {
					stream << " Q " << segments[j][3] * scale << " "
						   << segments[j][4] * scale << " "
						   << segments[j][5] * scale << " "
						   << segments[j][6] * scale;
				}
			}
		} else {
			for (int j = 0; j < static_cast<int>(segments.size()); j++) {
				if (segments[j][0] == 1.0) {
					stream << " L " << _RoundToDecimal(static_cast<float>(segments[j][3] * scale), roundCoordinates) << " "
						   << _RoundToDecimal(static_cast<float>(segments[j][4] * scale), roundCoordinates);
				} else {
					stream << " Q " << _RoundToDecimal(static_cast<float>(segments[j][3] * scale), roundCoordinates) << " "
						   << _RoundToDecimal(static_cast<float>(segments[j][4] * scale), roundCoordinates) << " "
						   << _RoundToDecimal(static_cast<float>(segments[j][5] * scale), roundCoordinates) << " "
						   << _RoundToDecimal(static_cast<float>(segments[j][6] * scale), roundCoordinates);
				}
			}
		}

		stream << " Z";
	}

	stream << "\" />";
}

void
SvgWriter::_WriteLinearGradientDef(std::ostringstream& defs,
								   const IndexedBitmap::LinearGradient& g,
								   const std::string& id,
								   const TracingOptions& options)
{
	std::string c1 = _HexColor(g.c1[0], g.c1[1], g.c1[2]);
	std::string c2 = _HexColor(g.c2[0], g.c2[1], g.c2[2]);
	double o1 = (double)g.c1[3] / 255.0;
	double o2 = (double)g.c2[3] / 255.0;

	double s = (double)options.fScale;

	defs << "<linearGradient id=\"" << id << "\" gradientUnits=\"userSpaceOnUse\" "
		 << "x1=\"" << g.x1 * s << "\" y1=\"" << g.y1 * s << "\" x2=\"" << g.x2 * s << "\" y2=\"" << g.y2 * s << "\">";
	defs << "<stop offset=\"0%\" stop-color=\"" << c1 << "\"";
	if (o1 < 0.999) defs << " stop-opacity=\"" << _RoundToDecimal((float)o1, 3.0f) << "\"";
	defs << "/>";
	defs << "<stop offset=\"100%\" stop-color=\"" << c2 << "\"";
	if (o2 < 0.999) defs << " stop-opacity=\"" << _RoundToDecimal((float)o2, 3.0f) << "\"";
	defs << "/>";
	defs << "</linearGradient>";
}

struct RenderGroup {
	int layerIndex;
	int parentPathIndex;
	double area;
	std::vector<int> pathIndices;

	bool operator<(const RenderGroup& other) const {
		return area > other.area;
	}
};

bool
SvgWriter::_IsHoleTransparent(const std::vector<std::vector<double> >& path,
							  const IndexedBitmap& indexed)
{
	if (path.empty()) return true;

	double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
	for (size_t i = 0; i < path.size(); i++) {
		if (path[i].size() < 2) continue;
		minX = std::min(minX, path[i][1]);
		maxX = std::max(maxX, path[i][1]);
		minY = std::min(minY, path[i][2]);
		maxY = std::max(maxY, path[i][2]);
	}

	int w = indexed.Width();
	int h = indexed.Height();

	for (int steps = 0; steps < 5; steps++) {
		double checkY = minY + (maxY - minY) * (0.3 + 0.1 * steps);
		int y = (int)checkY;
		if (y < 0 || y >= h) continue;

		std::vector<double> intersections;
		for (size_t i = 0; i < path.size(); i++) {
			double x1 = path[i][1], y1 = path[i][2];
			double x2, y2;
			if (path[i][0] == 1.0) { x2 = path[i][3]; y2 = path[i][4]; }
			else { x2 = path[i][5]; y2 = path[i][6]; }

			if ((y1 > checkY) != (y2 > checkY)) {
				double x = (x2 - x1) * (checkY - y1) / (y2 - y1) + x1;
				intersections.push_back(x);
			}
		}

		std::sort(intersections.begin(), intersections.end());

		for (size_t i = 0; i < intersections.size(); i += 2) {
			if (i + 1 >= intersections.size()) break;
			double midX = (intersections[i] + intersections[i+1]) * 0.5;
			int x = (int)midX;
			if (x < 0 || x >= w) continue;

			int idx = indexed.Array()[y+1][x+1];
			if (idx < 0)
				return true;

			const std::vector<unsigned char>& p = indexed.Palette()[idx];
			if (MathUtils::IsTransparent(p[3])) return true;

			return false;
		}
	}

	return true;
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

	const std::vector<std::vector<IndexedBitmap::LinearGradient> >& grads = indexedBitmap.LinearGradients();
	bool hasGradients = false;
	for (size_t k = 0; k < grads.size(); k++) {
		for (size_t i = 0; i < grads[k].size(); i++) {
			if (grads[k][i].valid) { hasGradients = true; break; }
		}
		if (hasGradients) break;
	}

	if (hasGradients) {
		std::ostringstream defs;
		defs << "\n<defs>";
		for (size_t k = 0; k < grads.size(); k++) {
			for (size_t i = 0; i < grads[k].size(); i++) {
				if (grads[k][i].valid) {
					std::ostringstream id;
					id << "lg_" << k << "_" << i;
					defs << "\n";
					_WriteLinearGradientDef(defs, grads[k][i], id.str(), options);
				}
			}
		}
		defs << "\n</defs>";
		svgStream << defs.str();
	}

	const std::vector<std::vector<std::vector<std::vector<double> > > >& layers = indexedBitmap.Layers();
	const std::vector<std::vector<IndexedBitmap::PathMetadata> >& metadata = indexedBitmap.PathsMetadata();

	std::vector<RenderGroup> renderQueue;
	std::vector<std::map<int, std::vector<int> > > layerHoles(layers.size());

	for (size_t k = 0; k < layers.size(); k++) {
		if (k < indexedBitmap.Palette().size() && indexedBitmap.Palette()[k][3] == 0) {
			continue; // Skip transparent layers
		}
		if (k >= metadata.size()) continue;

		for (size_t i = 0; i < layers[k].size(); i++) {
			if (layers[k][i].empty()) continue;
			if (i >= metadata[k].size()) continue;

			if (metadata[k][i].isHole) {
				int parent = metadata[k][i].parentPathIndex;
				if (parent != -1) {
					layerHoles[k][parent].push_back((int)i);
				}
			}
		}
	}

	for (size_t k = 0; k < layers.size(); k++) {
		if (k < indexedBitmap.Palette().size() && indexedBitmap.Palette()[k][3] == 0) {
			continue;
		}
		if (k >= metadata.size()) continue;

		for (size_t i = 0; i < layers[k].size(); i++) {
			if (layers[k][i].empty()) continue;
			if (i >= metadata[k].size()) continue;

			if (!metadata[k][i].isHole) {
				RenderGroup group;
				group.layerIndex = (int)k;
				group.parentPathIndex = (int)i;
				group.area = metadata[k][i].area;

				group.pathIndices.push_back((int)i);

				if (layerHoles[k].count((int)i)) {
					const std::vector<int>& holes = layerHoles[k][(int)i];
					for (size_t h = 0; h < holes.size(); h++) {
						int holeIdx = holes[h];
						if (_IsHoleTransparent(layers[k][holeIdx], indexedBitmap)) {
							group.pathIndices.push_back(holeIdx);
						}
					}
				}

				renderQueue.push_back(group);
			}
		}
	}

	// Sort groups by parent area descending
	std::sort(renderQueue.begin(), renderQueue.end());

	std::string description;
	for (size_t i = 0; i < renderQueue.size(); ++i) {
		const RenderGroup& group = renderQueue[i];
		int layer = group.layerIndex;
		int parentPath = group.parentPathIndex;

		if (options.fShowDescription) {
			std::ostringstream descriptionStream;
			descriptionStream << "desc=\"l " << layer << " p " << parentPath;
			if (group.pathIndices.size() > 1) {
				descriptionStream << " +" << (group.pathIndices.size() - 1) << "h";
			}
			descriptionStream << "\" ";
			description = descriptionStream.str();
		} else {
			description = "";
		}

		std::string colorString = _ColorToSvgString(indexedBitmap.Palette()[layer], 1.0);

		const std::vector<std::vector<IndexedBitmap::LinearGradient> >& gradsRef = grads;
		if (layer < static_cast<int>(gradsRef.size()) &&
			parentPath < static_cast<int>(gradsRef[layer].size()) &&
			gradsRef[layer][parentPath].valid) {
			std::ostringstream gradientFill;
			std::string gradUrl = "url(#lg_" + std::to_string(layer) + "_" + std::to_string(parentPath) + ")";

			const auto& g = gradsRef[layer][parentPath];
			bool isOpaque = (g.c1[3] >= 255 && g.c2[3] >= 255);

			gradientFill << "fill=\"" << gradUrl << "\" ";

			if (isOpaque) {
				gradientFill << "stroke=\"" << gradUrl << "\" "
							 << "stroke-width=\"1.5\" paint-order=\"stroke\" "
							 << "stroke-linejoin=\"round\" stroke-linecap=\"round\" ";
			} else {
				gradientFill << "stroke=\"none\" ";
			}

			colorString = gradientFill.str();
		}

		if (group.pathIndices.size() > 1) {
			_WriteSvgCompoundPath(svgStream, description, layers[layer],
							   group.pathIndices, colorString, options);
		} else {
			_WriteSvgPathString(svgStream, description, layers[layer][parentPath],
							   colorString, options);
		}
	}

	svgStream << "\n</svg>\n";
	return svgStream.str();
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
