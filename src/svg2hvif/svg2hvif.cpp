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
		std::cerr << "Error: Conversion failed for " << inFile << std::endl;
		return 1;
	}

	if (!writer.WriteToFile(outFile)) {
		std::cerr << "Error: Could not write to output file " << outFile << std::endl;
		return 1;
	}

	if (verbose) {
		std::cout << "Successfully converted " << inFile << " to " << outFile << std::endl;
	}

	return 0;
}
