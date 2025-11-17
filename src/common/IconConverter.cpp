/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

#include "IconConverter.h"
#include "IconAdapter.h"
#include "SVGWriter.h"
#include "SVGParser.h"
#include "HVIFParser.h"
#include "HVIFWriter.h"
#include "IOMParser.h"
#include "IOMWriter.h"
#include "PNGWriter.h"
#include "PNGParser.h"
#include "Utils.h"

namespace haiku {

std::string IconConverter::sLastError;

static bool
IsDegenerateSegment(const PathPoint& prev, const PathPoint& curr)
{
	if (!utils::DoubleEqual(prev.x, curr.x) || !utils::DoubleEqual(prev.y, curr.y))
		return false;

	if (!utils::DoubleEqual(prev.x_out, prev.x) || !utils::DoubleEqual(prev.y_out, prev.y))
		return false;

	if (!utils::DoubleEqual(curr.x_in, curr.x) || !utils::DoubleEqual(curr.y_in, curr.y))
		return false;

	return true;
}

static void
CleanupPath(Path& path)
{
	if (path.points.size() < 2)
		return;

	std::vector<PathPoint> cleaned;
	cleaned.reserve(path.points.size());

	cleaned.push_back(path.points[0]);

	if (path.points.size() > 2) {
		for (size_t i = 1; i + 1 < path.points.size(); ++i) {
			const PathPoint& prev = cleaned.back();
			const PathPoint& curr = path.points[i];

			if (IsDegenerateSegment(prev, curr))
				continue;

			cleaned.push_back(curr);
		}
	}

	cleaned.push_back(path.points.back());

	if (cleaned.size() == 1 && path.closed)
		path.closed = false;

	path.points.swap(cleaned);
}

static void
CleanupIconPaths(Icon& icon)
{
	for (size_t i = 0; i < icon.paths.size(); ++i)
		CleanupPath(icon.paths[i]);
}

static bool
PathPointsEqual(const PathPoint& a, const PathPoint& b)
{
	if (!utils::DoubleEqual(a.x, b.x) || !utils::DoubleEqual(a.y, b.y))
		return false;
	if (!utils::DoubleEqual(a.x_in, b.x_in) || !utils::DoubleEqual(a.y_in, b.y_in))
		return false;
	if (!utils::DoubleEqual(a.x_out, b.x_out) || !utils::DoubleEqual(a.y_out, b.y_out))
		return false;
	if (a.connected != b.connected)
		return false;
	return true;
}

static bool
PathsEqual(const Path& a, const Path& b)
{
	if (a.closed != b.closed)
		return false;
	if (a.points.size() != b.points.size())
		return false;
	if (a.name != b.name)
		return false;

	for (size_t i = 0; i < a.points.size(); ++i) {
		if (!PathPointsEqual(a.points[i], b.points[i]))
			return false;
	}
	return true;
}

static void
DeduplicateIconPaths(Icon& icon)
{
	if (icon.paths.empty())
		return;

	std::vector<Path> uniquePaths;
	uniquePaths.reserve(icon.paths.size());
	std::vector<int> oldToNew(icon.paths.size(), -1);

	for (size_t i = 0; i < icon.paths.size(); ++i) {
		const Path& p = icon.paths[i];
		int existingIndex = -1;
		for (size_t j = 0; j < uniquePaths.size(); ++j) {
			if (PathsEqual(uniquePaths[j], p)) {
				existingIndex = static_cast<int>(j);
				break;
			}
		}
		if (existingIndex >= 0) {
			oldToNew[i] = existingIndex;
		} else {
			int newIndex = static_cast<int>(uniquePaths.size());
			uniquePaths.push_back(p);
			oldToNew[i] = newIndex;
		}
	}

	for (size_t i = 0; i < icon.shapes.size(); ++i) {
		for (size_t j = 0; j < icon.shapes[i].pathIndices.size(); ++j) {
			int oldIndex = icon.shapes[i].pathIndices[j];
			if (oldIndex >= 0 && static_cast<size_t>(oldIndex) < oldToNew.size()) {
				icon.shapes[i].pathIndices[j] = oldToNew[oldIndex];
			}
		}
	}

	icon.paths.swap(uniquePaths);
}

bool
IconConverter::Convert(const std::string& inputFile, IconFormat inputFormat,
	const std::string& outputFile, IconFormat outputFormat)
{
	ConvertOptions opts;
	return Convert(inputFile, inputFormat, outputFile, outputFormat, opts);
}

bool
IconConverter::Convert(const std::string& inputFile, IconFormat inputFormat,
	const std::string& outputFile, IconFormat outputFormat, const ConvertOptions& opts)
{
	sLastError.clear();

	IconFormat actualInputFormat = inputFormat;
	if (actualInputFormat == FORMAT_AUTO) {
		actualInputFormat = DetectFormat(inputFile);
		if (opts.verbose) {
			std::cout << "Detected input format: " << FormatToString(actualInputFormat) << std::endl;
		}
	}

	IconFormat actualOutputFormat = outputFormat;
	if (actualOutputFormat == FORMAT_AUTO) {
		actualOutputFormat = DetectFormatByExtension(outputFile);
		if (opts.verbose) {
			std::cout << "Detected output format: " << FormatToString(actualOutputFormat) << std::endl;
		}
	}

	Icon icon = LoadWithOptions(inputFile, actualInputFormat, opts);
	if (!sLastError.empty())
		return false;

	ConvertOptions adjustedOpts = opts;

	if (actualOutputFormat == FORMAT_SVG) {
		if (actualInputFormat == FORMAT_HVIF) {
			adjustedOpts.svgViewBox = "0 0 6528 6528";
			adjustedOpts.coordinateScale = 102.0f;
		} else {
			if (adjustedOpts.svgViewBox.empty() || adjustedOpts.svgViewBox == "0 0 6528 6528") {
				adjustedOpts.svgViewBox = "0 0 64 64";
			}
			if (adjustedOpts.coordinateScale == 102.0f) {
				adjustedOpts.coordinateScale = 1.0f;
			}
		}
	}

	return Save(icon, outputFile, actualOutputFormat, adjustedOpts);
}

bool
IconConverter::Convert(const std::string& inputFile, const std::string& outputFile,
	IconFormat outputFormat, const ConvertOptions& opts)
{
	return Convert(inputFile, FORMAT_AUTO, outputFile, outputFormat, opts);
}

bool
IconConverter::Convert(const std::string& inputFile, const std::string& outputFile, IconFormat outputFormat)
{
	ConvertOptions opts;
	return Convert(inputFile, FORMAT_AUTO, outputFile, outputFormat, opts);
}

IconFormat
IconConverter::DetectFormat(const std::string& file)
{
	IconFormat format = DetectFormatBySignature(file);
	if (format != FORMAT_AUTO)
		return format;

	return DetectFormatByExtension(file);
}

IconFormat
IconConverter::DetectFormatBySignature(const std::string& file)
{
	std::ifstream f(file.c_str(), std::ios::binary);
	if (!f.is_open())
		return FORMAT_AUTO;

	std::vector<uint8_t> header(512);
	f.read(reinterpret_cast<char*>(&header[0]), header.size());
	size_t bytesRead = f.gcount();
	f.close();

	if (bytesRead < 4)
		return FORMAT_AUTO;

	if (header[0] == 0x6E && header[1] == 0x63 && 
		header[2] == 0x69 && header[3] == 0x66) {
		return FORMAT_HVIF;
	}

	if (header[0] == 'I' && header[1] == 'M' && 
		header[2] == 'S' && header[3] == 'G') {
		return FORMAT_IOM;
	}

	if (header[0] == 0x89 && header[1] == 'P' && 
		header[2] == 'N' && header[3] == 'G') {
		return FORMAT_PNG;
	}

	for (size_t i = 0; i < bytesRead && i < 100; ++i) {
		if (header[i] == '<') {
			if (i + 4 < bytesRead) {
				if (header[i+1] == '?' && header[i+2] == 'x' && 
					header[i+3] == 'm' && header[i+4] == 'l') {
					for (size_t j = i + 5; j < bytesRead - 4; ++j) {
						if (header[j] == '<' && header[j+1] == 's' && 
							header[j+2] == 'v' && header[j+3] == 'g') {
							return FORMAT_SVG;
						}
					}
				}

				if (header[i+1] == 's' && header[i+2] == 'v' && 
					header[i+3] == 'g' && (header[i+4] == ' ' || 
					header[i+4] == '>' || header[i+4] == '\t' || 
					header[i+4] == '\n' || header[i+4] == '\r')) {
					return FORMAT_SVG;
				}

				if (header[i+1] == '!' && header[i+2] == '-' && header[i+3] == '-') {
					size_t j = i + 4;
					while (j < bytesRead - 3) {
						if (header[j] == '-' && header[j+1] == '-' && header[j+2] == '>') {
							i = j + 2;
							break;
						}
						j++;
					}
				}
			}
		}
	}

	return FORMAT_AUTO;
}

IconFormat
IconConverter::DetectFormatByExtension(const std::string& file)
{
	size_t dotPos = file.rfind('.');
	if (dotPos == std::string::npos)
		return FORMAT_HVIF;

	std::string ext = file.substr(dotPos + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	if (ext == "hvif")
		return FORMAT_HVIF;
	else if (ext == "iom")
		return FORMAT_IOM;
	else if (ext == "svg")
		return FORMAT_SVG;
	else if (ext == "png")
		return FORMAT_PNG;
	else
		return FORMAT_HVIF;
}

std::string
IconConverter::FormatToString(IconFormat format)
{
	switch (format) {
		case FORMAT_AUTO: return "AUTO";
		case FORMAT_HVIF: return "HVIF";
		case FORMAT_IOM: return "IOM";
		case FORMAT_SVG: return "SVG";
		case FORMAT_PNG: return "PNG";
		default: return "Unknown";
	}
}

Icon
IconConverter::Load(const std::string& file, IconFormat format)
{
	ConvertOptions opts;
	return LoadWithOptions(file, format, opts);
}

Icon
IconConverter::LoadWithOptions(const std::string& file, IconFormat format, const ConvertOptions& opts)
{
	Icon icon;

	IconFormat actualFormat = format;
	if (actualFormat == FORMAT_AUTO) {
		actualFormat = DetectFormat(file);
	}

	switch (actualFormat) {
		case FORMAT_HVIF:
			icon = LoadHVIF(file);
			break;
		case FORMAT_IOM:
			icon = LoadIOM(file);
			break;
		case FORMAT_SVG:
			icon = LoadSVG(file, opts);
			break;
		case FORMAT_PNG:
			icon = LoadPNG(file, opts);
			break;
		default:
			SetError("Unknown input format");
			break;
	}

	return icon;
}

bool
IconConverter::Save(const Icon& icon, const std::string& file, IconFormat format)
{
	ConvertOptions opts;
	return Save(icon, file, format, opts);
}

bool
IconConverter::Save(const Icon& icon, const std::string& file, IconFormat format, const ConvertOptions& opts)
{
	IconFormat actualFormat = format;
	if (actualFormat == FORMAT_AUTO) {
		actualFormat = DetectFormatByExtension(file);
	}

	switch (actualFormat) {
		case FORMAT_HVIF:
			return SaveHVIF(icon, file);
		case FORMAT_IOM:
			return SaveIOM(icon, file);
		case FORMAT_SVG:
			return SaveSVG(icon, file, opts);
		case FORMAT_PNG:
			return SavePNG(icon, file, opts);
		default:
			SetError("Unknown output format");
			return false;
	}
}

Icon
IconConverter::LoadFromBuffer(const std::vector<uint8_t>& data, IconFormat format)
{
	Icon icon;
	sLastError.clear();

	IconFormat actualFormat = format;
	if (actualFormat == FORMAT_AUTO) {
		if (data.size() >= 4) {
			if (data[0] == 0x6E && data[1] == 0x63 && data[2] == 0x69 && data[3] == 0x66)
				actualFormat = FORMAT_HVIF;
			else if (data[0] == 'I' && data[1] == 'M' && data[2] == 'S' && data[3] == 'G')
				actualFormat = FORMAT_IOM;
			else if (data[0] == 0x89 && data[1] == 'P' && data[2] == 'N' && data[3] == 'G')
				actualFormat = FORMAT_PNG;
			else {
				actualFormat = FORMAT_SVG;
			}
		} else {
			actualFormat = FORMAT_HVIF;
		}
	}

	ConvertOptions opts;

	switch (actualFormat) {
		case FORMAT_HVIF:
			icon = LoadHVIFBuffer(data);
			break;
		case FORMAT_IOM:
			icon = LoadIOMBuffer(data);
			break;
		case FORMAT_SVG:
			icon = LoadSVGBuffer(data, opts);
			break;
		case FORMAT_PNG:
			icon = LoadPNGBuffer(data, opts);
			break;
		default:
			SetError("Unknown input format");
			break;
	}

	return icon;
}

bool
IconConverter::SaveToBuffer(const Icon& icon, std::vector<uint8_t>& buffer, IconFormat format, const ConvertOptions& opts)
{
	sLastError.clear();

	IconFormat actualFormat = format;
	if (actualFormat == FORMAT_AUTO)
		actualFormat = FORMAT_HVIF;

	switch (actualFormat) {
		case FORMAT_HVIF:
			return SaveHVIFBuffer(icon, buffer);
		case FORMAT_IOM:
			return SaveIOMBuffer(icon, buffer);
		case FORMAT_SVG:
			return SaveSVGBuffer(icon, buffer, opts);
		case FORMAT_PNG:
			return SavePNGBuffer(icon, buffer, opts);
		default:
			SetError("Unknown output format");
			return false;
	}
}

bool
IconConverter::SaveToBuffer(const Icon& icon, std::vector<uint8_t>& buffer, IconFormat format)
{
	ConvertOptions opts;
	return SaveToBuffer(icon, buffer, format, opts);
}

bool
IconConverter::ConvertBuffer(const std::vector<uint8_t>& inputData, IconFormat inputFormat,
	std::vector<uint8_t>& outputData, IconFormat outputFormat, const ConvertOptions& opts)
{
	sLastError.clear();

	IconFormat actualInputFormat = inputFormat;
	if (actualInputFormat == FORMAT_AUTO) {
		if (inputData.size() >= 4) {
			if (inputData[0] == 0x6E && inputData[1] == 0x63 && inputData[2] == 0x69 && inputData[3] == 0x66)
				actualInputFormat = FORMAT_HVIF;
			else if (inputData[0] == 'I' && inputData[1] == 'M' && inputData[2] == 'S' && inputData[3] == 'G')
				actualInputFormat = FORMAT_IOM;
			else if (inputData[0] == 0x89 && inputData[1] == 'P' && inputData[2] == 'N' && inputData[3] == 'G')
				actualInputFormat = FORMAT_PNG;
			else {
				actualInputFormat = FORMAT_SVG;
			}
		} else {
			actualInputFormat = FORMAT_HVIF;
		}
	}

	Icon icon;
	switch (actualInputFormat) {
		case FORMAT_HVIF:
			icon = LoadHVIFBuffer(inputData);
			break;
		case FORMAT_IOM:
			icon = LoadIOMBuffer(inputData);
			break;
		case FORMAT_SVG:
			icon = LoadSVGBuffer(inputData, opts);
			break;
		case FORMAT_PNG:
			icon = LoadPNGBuffer(inputData, opts);
			break;
		default:
			SetError("Unknown input format");
			return false;
	}

	if (!sLastError.empty())
		return false;

	ConvertOptions adjustedOpts = opts;
	if (outputFormat == FORMAT_SVG) {
		if (actualInputFormat == FORMAT_HVIF || 
			(actualInputFormat == FORMAT_AUTO && inputData.size() >= 4 &&
			inputData[0] == 0x6E && inputData[1] == 0x63 && inputData[2] == 0x69 && inputData[3] == 0x66)) {
			adjustedOpts.svgViewBox = "0 0 6528 6528";
			adjustedOpts.coordinateScale = 102.0f;
		} else {
			if (adjustedOpts.svgViewBox.empty() || adjustedOpts.svgViewBox == "0 0 6528 6528") {
				adjustedOpts.svgViewBox = "0 0 64 64";
			}
			if (adjustedOpts.coordinateScale == 102.0f) {
				adjustedOpts.coordinateScale = 1.0f;
			}
		}
	}

	return SaveToBuffer(icon, outputData, outputFormat, adjustedOpts);
}

bool
IconConverter::ConvertBuffer(const std::vector<uint8_t>& inputData, IconFormat inputFormat,
	std::vector<uint8_t>& outputData, IconFormat outputFormat)
{
	ConvertOptions opts;
	return ConvertBuffer(inputData, inputFormat, outputData, outputFormat, opts);
}

std::string
IconConverter::GetLastError()
{
	return sLastError;
}

void
IconConverter::SetError(const std::string& error)
{
	sLastError = error;
}

Icon
IconConverter::LoadHVIF(const std::string& file)
{
	Icon icon;

	hvif::HVIFParser parser;
	if (!parser.ParseFile(file)) {
		SetError("HVIF parsing failed: " + parser.GetLastError());
		return icon;
	}

	icon = adapter::HVIFAdapter::FromHVIF(parser.GetIcon());
	return icon;
}

Icon
IconConverter::LoadIOM(const std::string& file)
{
	Icon icon;

	iom::IOMParser parser;
	if (!parser.ParseFile(file)) {
		SetError("IOM parsing failed: " + parser.GetLastError());
		return icon;
	}

	icon = adapter::IOMAdapter::FromIOM(parser.GetIcon());
	return icon;
}

Icon
IconConverter::LoadSVG(const std::string& file, const ConvertOptions& opts)
{
	Icon icon;

	SVGParseOptions parseOpts;
	parseOpts.targetSize = 64.0f;
	parseOpts.preserveNames = opts.preserveNames;
	parseOpts.verbose = opts.verbose;

	SVGParser parser;
	if (!parser.Parse(file, icon, parseOpts)) {
		SetError("SVG parsing failed");
		return icon;
	}

	return icon;
}

Icon
IconConverter::LoadPNG(const std::string& file, const ConvertOptions& opts)
{
	Icon icon;

	PNGParser parser;
	PNGParseOptions pngOpts;
	pngOpts.preset = opts.pngPreset;
	pngOpts.removeBackground = opts.pngRemoveBackground;
	pngOpts.verbose = opts.verbose;

	if (!parser.Parse(file, icon, pngOpts)) {
		SetError("PNG parsing failed: " + parser.GetLastError());
		return icon;
	}

	return icon;
}

Icon
IconConverter::LoadHVIFBuffer(const std::vector<uint8_t>& data)
{
	Icon icon;

	if (data.size() < 4) {
		SetError("HVIF buffer too small");
		return icon;
	}

	hvif::HVIFParser parser;
	if (!parser.ParseData(data, "")) {
		SetError("HVIF parsing failed: " + parser.GetLastError());
		return icon;
	}

	icon = adapter::HVIFAdapter::FromHVIF(parser.GetIcon());
	return icon;
}

Icon
IconConverter::LoadIOMBuffer(const std::vector<uint8_t>& data)
{
	Icon icon;

	if (data.size() < 4) {
		SetError("IOM buffer too small");
		return icon;
	}

	if (data[0] != 'I' || data[1] != 'M' || data[2] != 'S' || data[3] != 'G') {
		SetError("IOM buffer does not start with IMSG");
		return icon;
	}

	haiku_compat::BMessage msg;
	haiku_compat::status_t result = msg.Unflatten((const char*)&data[4], (ssize_t)(data.size() - 4));
	if (result != haiku_compat::B_OK) {
		SetError("Failed to unflatten BMessage from buffer");
		return icon;
	}

	iom::IOMParser parser;
	if (!parser.ParseMessage(msg)) {
		SetError("IOM parsing failed: " + parser.GetLastError());
		return icon;
	}

	icon = adapter::IOMAdapter::FromIOM(parser.GetIcon());
	return icon;
}

Icon
IconConverter::LoadSVGBuffer(const std::vector<uint8_t>& data, const ConvertOptions& opts)
{
	Icon icon;

	SVGParseOptions parseOpts;
	parseOpts.targetSize = 64.0f;
	parseOpts.preserveNames = opts.preserveNames;
	parseOpts.verbose = opts.verbose;

	if (data.empty()) {
		SetError("SVG buffer is empty");
		return icon;
	}

	SVGParser parser;
	if (!parser.ParseBuffer((const char*)&data[0], data.size(), icon, parseOpts)) {
		SetError("SVG parsing failed");
		return icon;
	}

	return icon;
}

Icon
IconConverter::LoadPNGBuffer(const std::vector<uint8_t>& data, const ConvertOptions& opts)
{
	Icon icon;

	if (data.size() < 8) {
		SetError("PNG buffer too small");
		return icon;
	}

	PNGParser parser;
	PNGParseOptions pngOpts;
	pngOpts.preset = opts.pngPreset;
	pngOpts.removeBackground = opts.pngRemoveBackground;
	pngOpts.verbose = opts.verbose;

	if (!parser.ParseBuffer(data, icon, pngOpts)) {
		SetError("PNG parsing failed: " + parser.GetLastError());
		return icon;
	}

	return icon;
}

bool
IconConverter::SaveHVIF(const Icon& icon, const std::string& file)
{
	Icon tmp = icon;
	CleanupIconPaths(tmp);
	DeduplicateIconPaths(tmp);

	hvif::HVIFWriter writer;

	std::vector<uint8_t> styleIndexMap;
	std::vector<uint8_t> pathIndexMap;

	for (size_t i = 0; i < tmp.styles.size(); ++i) {
		hvif::Style hvifStyle;
		hvifStyle.isGradient = tmp.styles[i].isGradient;

		if (hvifStyle.isGradient) {
			const Gradient& grad = tmp.styles[i].gradient;
			hvifStyle.gradient.type = static_cast<hvif::GradientTypes>(grad.type);
			hvifStyle.gradient.flags = 0;
			hvifStyle.gradient.hasMatrix = grad.hasTransform;

			if (grad.hasTransform && grad.transform.size() >= 6) {
				for (size_t j = 0; j < grad.transform.size(); ++j) {
					hvifStyle.gradient.matrix.push_back(static_cast<float>(grad.transform[j]));
				}
			}

			for (size_t j = 0; j < grad.stops.size(); ++j) {
				hvif::GradientStop stop;
				stop.offset = static_cast<uint8_t>(grad.stops[j].offset * 255.0f);

				hvif::Color stopColor;
				stopColor.tag = hvif::RGBA;
				stopColor.data.push_back(grad.stops[j].color.Red());
				stopColor.data.push_back(grad.stops[j].color.Green());
				stopColor.data.push_back(grad.stops[j].color.Blue());
				stopColor.data.push_back(grad.stops[j].color.Alpha());
				stop.color = stopColor;

				hvifStyle.gradient.stops.push_back(stop);
			}
		} else {
			hvif::Color color;
			color.tag = hvif::RGBA;
			color.data.push_back(tmp.styles[i].solidColor.Red());
			color.data.push_back(tmp.styles[i].solidColor.Green());
			color.data.push_back(tmp.styles[i].solidColor.Blue());
			color.data.push_back(tmp.styles[i].solidColor.Alpha());
			hvifStyle.color = color;
		}

		uint8_t newIndex = writer.AddStyle(hvifStyle);
		styleIndexMap.push_back(newIndex);
	}

	for (size_t i = 0; i < tmp.paths.size(); ++i) {
		hvif::InternalPath hvifPath;
		hvifPath.closed = tmp.paths[i].closed;

		for (size_t j = 0; j < tmp.paths[i].points.size(); ++j) {
			const PathPoint& pt = tmp.paths[i].points[j];

			hvif::PathNode node;
			node.x = static_cast<float>(pt.x);
			node.y = static_cast<float>(pt.y);
			node.x_in = static_cast<float>(pt.x_in);
			node.y_in = static_cast<float>(pt.y_in);
			node.x_out = static_cast<float>(pt.x_out);
			node.y_out = static_cast<float>(pt.y_out);
			hvifPath.nodes.push_back(node);
		}

		uint8_t newIndex = writer.AddInternalPath(hvifPath);
		pathIndexMap.push_back(newIndex);
	}

	for (size_t i = 0; i < tmp.shapes.size(); ++i) {
		hvif::Shape hvifShape;

		if (tmp.shapes[i].styleIndex >= 0 && 
			tmp.shapes[i].styleIndex < static_cast<int>(styleIndexMap.size())) {
			hvifShape.styleIndex = styleIndexMap[tmp.shapes[i].styleIndex];
		} else {
			hvifShape.styleIndex = 0;
		}

		for (size_t j = 0; j < tmp.shapes[i].pathIndices.size(); ++j) {
			int oldPathIndex = tmp.shapes[i].pathIndices[j];
			if (oldPathIndex >= 0 && oldPathIndex < static_cast<int>(pathIndexMap.size())) {
				hvifShape.pathIndices.push_back(pathIndexMap[oldPathIndex]);
			}
		}

		hvifShape.hasTransform = tmp.shapes[i].hasTransform;
		if (hvifShape.hasTransform && tmp.shapes[i].transform.size() >= 6) {
			hvifShape.transformType = "matrix";
			for (size_t j = 0; j < tmp.shapes[i].transform.size(); ++j) {
				hvifShape.transform.push_back(static_cast<float>(tmp.shapes[i].transform[j]));
			}
		}

		for (size_t j = 0; j < tmp.shapes[i].transformers.size(); ++j) {
			const Transformer& trans = tmp.shapes[i].transformers[j];
			hvif::Transformer hvifTrans;

			if (trans.type == TRANSFORMER_STROKE) {
				hvifTrans.tag = hvif::STROKE;
				hvifTrans.width = static_cast<float>(trans.width);
				hvifTrans.lineJoin = static_cast<uint8_t>(trans.lineJoin);
				hvifTrans.lineCap = static_cast<uint8_t>(trans.lineCap);
				hvifTrans.miterLimit = static_cast<uint8_t>(trans.miterLimit);
			} else if (trans.type == TRANSFORMER_CONTOUR) {
				hvifTrans.tag = hvif::CONTOUR;
				hvifTrans.width = static_cast<float>(trans.width);
				hvifTrans.lineJoin = static_cast<uint8_t>(trans.lineJoin);
				hvifTrans.miterLimit = static_cast<uint8_t>(trans.miterLimit);
			} else if (trans.type == TRANSFORMER_AFFINE) {
				hvifTrans.tag = hvif::AFFINE;
				for (size_t k = 0; k < trans.matrix.size(); ++k) {
					hvifTrans.data.push_back(static_cast<float>(trans.matrix[k]));
				}
			} else {
				hvifTrans.tag = hvif::PERSPECTIVE;
			}

			hvifShape.transformers.push_back(hvifTrans);
		}

		hvifShape.minLOD = static_cast<uint8_t>(utils::clamp(
			static_cast<int>(tmp.shapes[i].minLOD * 255.0f / 4.0f), 0, 255));
		hvifShape.maxLOD = static_cast<uint8_t>(utils::clamp(
			static_cast<int>(tmp.shapes[i].maxLOD * 255.0f / 4.0f), 0, 255));
		hvifShape.hasLOD = (hvifShape.minLOD != 0 || hvifShape.maxLOD != 255);

		writer.AddShape(hvifShape);
	}

	if (!writer.CheckHVIFLimitations()) {
		SetError("Icon exceeds HVIF format limitations (max 255 styles/paths/shapes)");
		return false;
	}

	if (!writer.WriteToFile(file)) {
		SetError("Failed to write HVIF file");
		return false;
	}

	return true;
}

bool
IconConverter::SaveHVIFBuffer(const Icon& icon, std::vector<uint8_t>& buffer)
{
	Icon tmp = icon;
	CleanupIconPaths(tmp);
	DeduplicateIconPaths(tmp);

	hvif::HVIFWriter writer;

	std::vector<uint8_t> styleIndexMap;
	std::vector<uint8_t> pathIndexMap;

	for (size_t i = 0; i < tmp.styles.size(); ++i) {
		hvif::Style hvifStyle;
		hvifStyle.isGradient = tmp.styles[i].isGradient;

		if (hvifStyle.isGradient) {
			const Gradient& grad = tmp.styles[i].gradient;
			hvifStyle.gradient.type = static_cast<hvif::GradientTypes>(grad.type);
			hvifStyle.gradient.flags = 0;
			hvifStyle.gradient.hasMatrix = grad.hasTransform;

			if (grad.hasTransform && grad.transform.size() >= 6) {
				for (size_t j = 0; j < grad.transform.size(); ++j) {
					hvifStyle.gradient.matrix.push_back(static_cast<float>(grad.transform[j]));
				}
			}

			for (size_t j = 0; j < grad.stops.size(); ++j) {
				hvif::GradientStop stop;
				stop.offset = static_cast<uint8_t>(grad.stops[j].offset * 255.0f);

				hvif::Color stopColor;
				stopColor.tag = hvif::RGBA;
				stopColor.data.push_back(grad.stops[j].color.Red());
				stopColor.data.push_back(grad.stops[j].color.Green());
				stopColor.data.push_back(grad.stops[j].color.Blue());
				stopColor.data.push_back(grad.stops[j].color.Alpha());
				stop.color = stopColor;

				hvifStyle.gradient.stops.push_back(stop);
			}
		} else {
			hvif::Color color;
			color.tag = hvif::RGBA;
			color.data.push_back(tmp.styles[i].solidColor.Red());
			color.data.push_back(tmp.styles[i].solidColor.Green());
			color.data.push_back(tmp.styles[i].solidColor.Blue());
			color.data.push_back(tmp.styles[i].solidColor.Alpha());
			hvifStyle.color = color;
		}

		uint8_t newIndex = writer.AddStyle(hvifStyle);
		styleIndexMap.push_back(newIndex);
	}

	for (size_t i = 0; i < tmp.paths.size(); ++i) {
		hvif::InternalPath hvifPath;
		hvifPath.closed = tmp.paths[i].closed;

		for (size_t j = 0; j < tmp.paths[i].points.size(); ++j) {
			const PathPoint& pt = tmp.paths[i].points[j];

			hvif::PathNode node;
			node.x = static_cast<float>(pt.x);
			node.y = static_cast<float>(pt.y);
			node.x_in = static_cast<float>(pt.x_in);
			node.y_in = static_cast<float>(pt.y_in);
			node.x_out = static_cast<float>(pt.x_out);
			node.y_out = static_cast<float>(pt.y_out);
			hvifPath.nodes.push_back(node);
		}

		uint8_t newIndex = writer.AddInternalPath(hvifPath);
		pathIndexMap.push_back(newIndex);
	}

	for (size_t i = 0; i < tmp.shapes.size(); ++i) {
		hvif::Shape hvifShape;

		if (tmp.shapes[i].styleIndex >= 0 && 
			tmp.shapes[i].styleIndex < static_cast<int>(styleIndexMap.size())) {
			hvifShape.styleIndex = styleIndexMap[tmp.shapes[i].styleIndex];
		} else {
			hvifShape.styleIndex = 0;
		}

		for (size_t j = 0; j < tmp.shapes[i].pathIndices.size(); ++j) {
			int oldPathIndex = tmp.shapes[i].pathIndices[j];
			if (oldPathIndex >= 0 && oldPathIndex < static_cast<int>(pathIndexMap.size())) {
				hvifShape.pathIndices.push_back(pathIndexMap[oldPathIndex]);
			}
		}

		hvifShape.hasTransform = tmp.shapes[i].hasTransform;
		if (hvifShape.hasTransform && tmp.shapes[i].transform.size() >= 6) {
			hvifShape.transformType = "matrix";
			for (size_t j = 0; j < tmp.shapes[i].transform.size(); ++j) {
				hvifShape.transform.push_back(static_cast<float>(tmp.shapes[i].transform[j]));
			}
		}

		for (size_t j = 0; j < tmp.shapes[i].transformers.size(); ++j) {
			const Transformer& trans = tmp.shapes[i].transformers[j];
			hvif::Transformer hvifTrans;

			if (trans.type == TRANSFORMER_STROKE) {
				hvifTrans.tag = hvif::STROKE;
				hvifTrans.width = static_cast<float>(trans.width);
				hvifTrans.lineJoin = static_cast<uint8_t>(trans.lineJoin);
				hvifTrans.lineCap = static_cast<uint8_t>(trans.lineCap);
				hvifTrans.miterLimit = static_cast<uint8_t>(trans.miterLimit);
			} else if (trans.type == TRANSFORMER_CONTOUR) {
				hvifTrans.tag = hvif::CONTOUR;
				hvifTrans.width = static_cast<float>(trans.width);
				hvifTrans.lineJoin = static_cast<uint8_t>(trans.lineJoin);
				hvifTrans.miterLimit = static_cast<uint8_t>(trans.miterLimit);
			} else if (trans.type == TRANSFORMER_AFFINE) {
				hvifTrans.tag = hvif::AFFINE;
				for (size_t k = 0; k < trans.matrix.size(); ++k) {
					hvifTrans.data.push_back(static_cast<float>(trans.matrix[k]));
				}
			} else {
				hvifTrans.tag = hvif::PERSPECTIVE;
			}

			hvifShape.transformers.push_back(hvifTrans);
		}

		hvifShape.minLOD = static_cast<uint8_t>(utils::clamp(
			static_cast<int>(tmp.shapes[i].minLOD * 255.0f / 4.0f), 0, 255));
		hvifShape.maxLOD = static_cast<uint8_t>(utils::clamp(
			static_cast<int>(tmp.shapes[i].maxLOD * 255.0f / 4.0f), 0, 255));
		hvifShape.hasLOD = (hvifShape.minLOD != 0 || hvifShape.maxLOD != 255);

		writer.AddShape(hvifShape);
	}

	if (!writer.CheckHVIFLimitations()) {
		SetError("Icon exceeds HVIF format limitations (max 255 styles/paths/shapes)");
		return false;
	}

	buffer = writer.WriteToBuffer();
	if (buffer.empty()) {
		SetError("Failed to write HVIF buffer");
		return false;
	}

	return true;
}

bool
IconConverter::SaveIOM(const Icon& icon, const std::string& file)
{
	Icon tmp = icon;
	CleanupIconPaths(tmp);
	DeduplicateIconPaths(tmp);

	iom::Icon iomIcon = adapter::IOMAdapter::ToIOM(tmp);

	iom::IOMWriter writer;
	if (!writer.WriteToFile(file, iomIcon)) {
		SetError("Failed to write IOM file");
		return false;
	}

	return true;
}

bool
IconConverter::SaveIOMBuffer(const Icon& icon, std::vector<uint8_t>& buffer)
{
	Icon tmp = icon;
	CleanupIconPaths(tmp);
	DeduplicateIconPaths(tmp);

	iom::Icon iomIcon = adapter::IOMAdapter::ToIOM(tmp);

	iom::IOMWriter writer;
	if (!writer.WriteToBuffer(buffer, iomIcon)) {
		SetError("Failed to write IOM buffer");
		return false;
	}

	return true;
}

bool
IconConverter::SaveSVG(const Icon& icon, const std::string& file, const ConvertOptions& opts)
{
	Icon tmp = icon;
	CleanupIconPaths(tmp);
	DeduplicateIconPaths(tmp);

	SVGWriterOptions writerOpts;
	writerOpts.width = opts.svgWidth;
	writerOpts.height = opts.svgHeight;
	writerOpts.viewBox = opts.svgViewBox;
	writerOpts.includeNames = opts.preserveNames;
	writerOpts.coordinateScale = opts.coordinateScale;

	SVGWriter writer;
	std::string svg = writer.Write(tmp, writerOpts);

	std::ofstream output(file.c_str());
	if (!output.is_open()) {
		SetError("Cannot create output file: " + file);
		return false;
	}

	output << svg;
	output.close();

	if (!output.good()) {
		SetError("Failed to write SVG file");
		return false;
	}

	return true;
}

bool
IconConverter::SaveSVGBuffer(const Icon& icon, std::vector<uint8_t>& buffer, const ConvertOptions& opts)
{
	Icon tmp = icon;
	CleanupIconPaths(tmp);
	DeduplicateIconPaths(tmp);

	SVGWriterOptions writerOpts;
	writerOpts.width = opts.svgWidth;
	writerOpts.height = opts.svgHeight;
	writerOpts.viewBox = opts.svgViewBox;
	writerOpts.includeNames = opts.preserveNames;
	writerOpts.coordinateScale = opts.coordinateScale;

	SVGWriter writer;
	std::string svg = writer.Write(tmp, writerOpts);

	buffer.clear();
	buffer.insert(buffer.end(), svg.begin(), svg.end());

	return true;
}

bool
IconConverter::SavePNG(const Icon& icon, const std::string& file, const ConvertOptions& opts)
{
	Icon tmp = icon;
	CleanupIconPaths(tmp);
	DeduplicateIconPaths(tmp);

	PNGWriter writer;
	PNGWriterOptions pngOpts;

	pngOpts.width = opts.pngWidth;
	pngOpts.height = opts.pngHeight;
	pngOpts.scale = opts.pngScale;

	if (!writer.WriteToFile(tmp, file, pngOpts)) {
		SetError("Failed to write PNG file");
		return false;
	}

	return true;
}

bool
IconConverter::SavePNGBuffer(const Icon& icon, std::vector<uint8_t>& buffer, const ConvertOptions& opts)
{
	Icon tmp = icon;
	CleanupIconPaths(tmp);
	DeduplicateIconPaths(tmp);

	PNGWriter writer;
	PNGWriterOptions pngOpts;

	pngOpts.width = opts.pngWidth;
	pngOpts.height = opts.pngHeight;
	pngOpts.scale = opts.pngScale;

	if (!writer.WriteToBuffer(tmp, buffer, pngOpts)) {
		SetError("Failed to write PNG buffer");
		return false;
	}

	return true;
}

}
