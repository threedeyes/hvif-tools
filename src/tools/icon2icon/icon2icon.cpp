/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <iostream>
#include <cstdlib>
#include <cstring>

#include "IconConverter.h"

void PrintUsage(const char* prog)
{
	std::cerr << "Usage: " << prog << " <input> <output> [options]\n";
	std::cerr << "\n";
	std::cerr << "Input format is auto-detected by file signature.\n";
	std::cerr << "Output format is determined by -f option or file extension.\n";
	std::cerr << "\n";
	std::cerr << "Options:\n";
	std::cerr << "  -f, --format <fmt>       Output format: hvif, iom, svg, png (default: auto)\n";
	std::cerr << "  -v, --verbose            Show conversion details\n";
	std::cerr << "  --names                  Preserve element names\n";
	std::cerr << "\n";
	std::cerr << "SVG/PNG output options:\n";
	std::cerr << "  --width <n>              Output width (default: 64)\n";
	std::cerr << "  --height <n>             Output height (default: 64)\n";
	std::cerr << "  --scale <f>              PNG scale factor (default: 1.0)\n";
	std::cerr << "\n";
	std::cerr << "PNG input options:\n";
	std::cerr << "  --preset <name>          Vectorization preset:\n";
	std::cerr << "                           - icon (default): simple icons, no gradients\n";
	std::cerr << "                           - icon-gradient: icons with gradient support\n";
	std::cerr << "  --remove-bg              Remove background from PNG (auto-detect)\n";
	std::cerr << "\n";
	std::cerr << "Other:\n";
	std::cerr << "  --detect                 Only detect and print input format\n";
	std::cerr << "\n";
	std::cerr << "Examples:\n";
	std::cerr << "  " << prog << " icon.hvif icon.svg\n";
	std::cerr << "  " << prog << " icon.svg icon.dat -f hvif\n";
	std::cerr << "  " << prog << " icon.hvif icon.png --width 128 --height 128\n";
	std::cerr << "  " << prog << " icon.png icon.hvif --preset icon-gradient\n";
	std::cerr << "  " << prog << " logo.png logo.svg --preset icon-gradient --remove-bg\n";
	std::cerr << "  " << prog << " unknown.file --detect\n";
}

haiku::IconFormat ParseFormatString(const std::string& format)
{
	if (format == "auto") return haiku::FORMAT_AUTO;
	if (format == "hvif") return haiku::FORMAT_HVIF;
	if (format == "iom") return haiku::FORMAT_IOM;
	if (format == "svg") return haiku::FORMAT_SVG;
	if (format == "png") return haiku::FORMAT_PNG;
	return haiku::FORMAT_AUTO;
}

haiku::PNGVectorizationPreset ParsePresetString(const std::string& preset)
{
	if (preset == "icon") return haiku::PRESET_ICON;
	if (preset == "icon-gradient") return haiku::PRESET_ICON_GRADIENT;
	return haiku::PRESET_ICON;
}

int main(int argc, char** argv)
{
	if (argc < 2) {
		PrintUsage(argv[0]);
		return 1;
	}
	
	std::string inFile;
	std::string outFile;
	haiku::IconFormat outputFormat = haiku::FORMAT_AUTO;
	bool detectOnly = false;
	
	haiku::ConvertOptions opts;
	
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		
		if (arg == "-h" || arg == "--help") {
			PrintUsage(argv[0]);
			return 0;
		} else if (arg == "--detect") {
			detectOnly = true;
		} else if (arg == "-f" || arg == "--format") {
			if (i + 1 < argc) {
				outputFormat = ParseFormatString(argv[++i]);
			} else {
				std::cerr << "Error: " << arg << " requires an argument\n";
				return 1;
			}
		} else if (arg == "-v" || arg == "--verbose") {
			opts.verbose = true;
		} else if (arg == "--names") {
			opts.preserveNames = true;
		} else if (arg == "--width") {
			if (i + 1 < argc) {
				opts.svgWidth = std::atoi(argv[++i]);
				opts.pngWidth = opts.svgWidth;
				if (opts.svgWidth <= 0) {
					opts.svgWidth = 64;
					opts.pngWidth = 64;
				}
			} else {
				std::cerr << "Error: --width requires an argument\n";
				return 1;
			}
		} else if (arg == "--height") {
			if (i + 1 < argc) {
				opts.svgHeight = std::atoi(argv[++i]);
				opts.pngHeight = opts.svgHeight;
				if (opts.svgHeight <= 0) {
					opts.svgHeight = 64;
					opts.pngHeight = 64;
				}
			} else {
				std::cerr << "Error: --height requires an argument\n";
				return 1;
			}
		} else if (arg == "--scale") {
			if (i + 1 < argc) {
				opts.pngScale = static_cast<float>(std::atof(argv[++i]));
				if (opts.pngScale <= 0.0f) opts.pngScale = 1.0f;
			} else {
				std::cerr << "Error: --scale requires an argument\n";
				return 1;
			}
		} else if (arg == "--preset") {
			if (i + 1 < argc) {
				opts.pngPreset = ParsePresetString(argv[++i]);
			} else {
				std::cerr << "Error: --preset requires an argument\n";
				return 1;
			}
		} else if (arg == "--remove-bg") {
			opts.pngRemoveBackground = true;
		} else if (arg[0] == '-') {
			std::cerr << "Error: Unknown option " << arg << "\n";
			PrintUsage(argv[0]);
			return 1;
		} else {
			if (inFile.empty()) {
				inFile = arg;
			} else if (outFile.empty()) {
				outFile = arg;
			} else {
				std::cerr << "Error: Too many arguments\n";
				PrintUsage(argv[0]);
				return 1;
			}
		}
	}
	
	if (inFile.empty()) {
		std::cerr << "Error: No input file specified\n";
		PrintUsage(argv[0]);
		return 1;
	}
	
	if (detectOnly) {
		haiku::IconFormat detectedFormat = haiku::IconConverter::DetectFormat(inFile);
		std::cout << "File: " << inFile << "\n";
		std::cout << "Detected format: " << haiku::IconConverter::FormatToString(detectedFormat) << "\n";
		
		if (opts.verbose) {
			haiku::Icon icon = haiku::IconConverter::Load(inFile, detectedFormat);
			if (haiku::IconConverter::GetLastError().empty()) {
				std::cout << "  Styles: " << icon.styles.size() << "\n";
				std::cout << "  Paths: " << icon.paths.size() << "\n";
				std::cout << "  Shapes: " << icon.shapes.size() << "\n";
			}
		}
		return 0;
	}
	
	if (outFile.empty()) {
		std::cerr << "Error: No output file specified\n";
		PrintUsage(argv[0]);
		return 1;
	}
	
	if (!haiku::IconConverter::Convert(inFile, outFile, outputFormat, opts)) {
		std::cerr << "Error: " << haiku::IconConverter::GetLastError() << std::endl;
		return 1;
	}
	
	if (opts.verbose) {
		haiku::Icon icon = haiku::IconConverter::Load(inFile, haiku::FORMAT_AUTO);
		std::cout << "Conversion successful!\n";
		std::cout << "  Styles: " << icon.styles.size() << "\n";
		std::cout << "  Paths: " << icon.paths.size() << "\n";
		std::cout << "  Shapes: " << icon.shapes.size() << "\n";
		
		haiku::IconFormat inputFormat = haiku::IconConverter::DetectFormat(inFile);
		if (inputFormat == haiku::FORMAT_PNG) {
			std::cout << "  PNG preset: ";
			switch (opts.pngPreset) {
				case haiku::PRESET_ICON:
					std::cout << "icon (simple, no gradients)";
					break;
				case haiku::PRESET_ICON_GRADIENT:
					std::cout << "icon-gradient (with gradient support)";
					break;
			}
			std::cout << "\n";
			if (opts.pngRemoveBackground) {
				std::cout << "  Background removal: enabled\n";
			}
		}
	} else {
		std::cout << "Successfully converted " << inFile << " to " << outFile << "\n";
	}
	
	return 0;
}
