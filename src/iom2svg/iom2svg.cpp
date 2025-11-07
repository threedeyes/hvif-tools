/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>

#include "IOMParser.h"
#include "SVGRenderer.h"

int main(int argc, char** argv)
{
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " <input.iom> <output.svg> [width] [height] [--names]\n";
		std::cerr << "  width, height - optional output size (default: 64x64)\n";
		std::cerr << "  --names       - add element names as SVG id attributes\n";
		return 1;
	}

	int width = 64;
	int height = 64;
	bool addNames = false;

	for (int i = 3; i < argc; i++) {
		if (strcmp(argv[i], "--names") == 0) {
			addNames = true;
		} else if (width == 64 && height == 64) {
			width = std::atoi(argv[i]);
			if (width <= 0) width = 64;
			if (i + 1 < argc && strcmp(argv[i + 1], "--names") != 0) {
				height = std::atoi(argv[i + 1]);
				if (height <= 0) height = 64;
				i++;
			} else {
				height = width;
			}
		}
	}

	iom::IOMParser parser;
	if (!parser.ParseFile(argv[1])) {
		std::cerr << "Error parsing IOM file: " << parser.GetLastError() << std::endl;
		return 1;
	}

	const iom::Icon& icon = parser.GetIcon();
	std::cout << "Parsed IOM file: " << icon.filename << std::endl;
	std::cout << "  Styles: " << icon.styles.size() << std::endl;
	std::cout << "  Paths: " << icon.paths.size() << std::endl;
	std::cout << "  Shapes: " << icon.shapes.size() << std::endl;

	iom::SVGRenderer renderer(addNames);
	std::string svg = renderer.RenderIcon(icon, width, height);

	std::ofstream output(argv[2]);
	if (!output.is_open()) {
		std::cerr << "Error: Cannot create output file: " << argv[2] << std::endl;
		return 1;
	}

	output << svg;
	output.close();

	std::cout << "Successfully converted " << argv[1] << " to " << argv[2] 
			  << " (" << width << "x" << height << ")";
	if (addNames)
		std::cout << " with element names";
	std::cout << std::endl;

	return 0;
}
