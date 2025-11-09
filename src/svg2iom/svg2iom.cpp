/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <iostream>
#include <string>
#include <cstring>

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"

#include "SVGParser.h"
#include "IOMWriter.h"

void PrintUsage(const char* prog)
{
	std::cerr << "Usage: " << prog << " <input.svg> <output.iom>" << std::endl;
}

int main(int argc, char** argv)
{
	if (argc < 3) {
		PrintUsage(argv[0]);
		return 1;
	}

	std::string inFile;
	std::string outFile;

	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (inFile.empty()) {
			inFile = arg;
		} else if (outFile.empty()) {
			outFile = arg;
		}
	}

	if (inFile.empty() || outFile.empty()) {
		PrintUsage(argv[0]);
		return 1;
	}

	iom::SVGParser converter;

	iom::Icon icon;
	if (!converter.ParseFile(inFile, icon)) {
		std::cerr << "Error: SVG parsing failed for " << inFile.c_str() << std::endl;
		return 2;
	}

	iom::IOMWriter writer;
	if (!writer.WriteToFile(outFile, icon)) {
		std::cerr << "Error: Could not write to output file " << outFile.c_str() << std::endl;
		return 3;
	}

	return 0;
}
