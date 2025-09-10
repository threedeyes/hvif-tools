/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"

#include "SVGParser.h"
#include "HVIFWriter.h"

void PrintUsage(const char* prog) {
	std::cerr << "Usage: " << prog << " [-v | --verbose] input.svg output.hvif" << std::endl;
}

int main(int argc, char** argv) {
	if (argc < 3) {
		PrintUsage(argv[0]);
		return 1;
	}

	std::string inFile, outFile;
	bool verbose = false;

	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "-v" || arg == "--verbose") {
			verbose = true;
		} else if (inFile.empty()) {
			inFile = arg;
		} else if (outFile.empty()) {
			outFile = arg;
		}
	}

	if (inFile.empty() || outFile.empty()) {
		PrintUsage(argv[0]);
		return 1;
	}

	hvif::SVGParser parser;
	parser.SetVerbose(verbose);
	
	hvif::HVIFWriter writer;

	if (!parser.ParseFile(inFile, writer)) {
		std::cerr << "Error: SVG parsing failed for " << inFile.c_str() << std::endl;
		return 2;
	}

	// Check HVIF limitations before writing
	if (!writer.CheckHVIFLimitations()) {
		if (verbose) {
			std::cerr << "Error: SVG exceeds HVIF format limitations:" << std::endl;
			std::cerr << "  Styles: " << writer.GetStylesCount() << " (max " << hvif::MAX_STYLES << ")" << std::endl;
			std::cerr << "  Paths: " << writer.GetPathsCount() << " (max " << hvif::MAX_PATHS << ")" << std::endl;
			std::cerr << "  Shapes: " << writer.GetShapesCount() << " (max " << hvif::MAX_SHAPES << ")" << std::endl;
		} else {
			std::cerr << "Error: SVG is too complex for HVIF format" << std::endl;
		}
		return 3;
	}

	if (!writer.WriteToFile(outFile)) {
		std::cerr << "Error: Could not write to output file " << outFile.c_str() << std::endl;
		return 4;
	}

	if (verbose) {
		std::cout << "Successfully converted " << inFile.c_str() << " to " << outFile.c_str() << std::endl;
		std::cout << "  Styles: " << writer.GetStylesCount() << std::endl;
		std::cout << "  Paths: " << writer.GetPathsCount() << std::endl;
		std::cout << "  Shapes: " << writer.GetShapesCount() << std::endl;
	}

	return 0;
}
