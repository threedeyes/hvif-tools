/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <iostream>
#include <fstream>
#include <cstdlib>

#include "HVIFParser.h"
#include "SVGRenderer.h"

int main(int argc, char** argv)
{
    if (argc < 3 || argc > 5) {
        std::cerr << "Usage: " << argv[0] << " <input.hvif> <output.svg> [width] [height]\n";
        std::cerr << "  width, height - optional output size (default: 64x64)\n";
        return 1;
    }

    if (!hvif::HVIFParser::IsValidHVIFFile(argv[1])) {
        std::cerr << "Error: File is not a valid HVIF file: " << argv[1] << std::endl;
        return 1;
    }

    int width = 64;
    int height = 64;

    if (argc >= 4) {
        width = std::atoi(argv[3]);
        if (width <= 0) width = 64;
    }

    if (argc >= 5) {
        height = std::atoi(argv[4]);
        if (height <= 0) height = 64;
    }

    hvif::HVIFParser parser;
    if (!parser.ParseFile(argv[1])) {
        std::cerr << "Error parsing HVIF file: " << parser.GetLastError() << std::endl;
        return 1;
    }

    const hvif::HVIFIcon& icon = parser.GetIcon();
    std::cout << "Parsed HVIF file: " << icon.filename << std::endl;
    std::cout << "  Styles: " << icon.styles.size() << std::endl;
    std::cout << "  Paths: " << icon.paths.size() << std::endl;
    std::cout << "  Shapes: " << icon.shapes.size() << std::endl;

    hvif::SVGRenderer renderer;
    std::string svg = renderer.RenderIcon(icon, width, height);

    std::ofstream output(argv[2]);
    if (!output.is_open()) {
        std::cerr << "Error: Cannot create output file: " << argv[2] << std::endl;
        return 1;
    }

    output << svg;
    output.close();

    std::cout << "Successfully converted " << argv[1] << " to " << argv[2] 
              << " (" << width << "x" << height << ")" << std::endl;

    return 0;
}
