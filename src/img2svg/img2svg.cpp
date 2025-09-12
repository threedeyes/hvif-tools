/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <iostream>
#include <cstring>
#include <cstdlib>

#include "ImageTracer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void
PrintUsage(const char* programName)
{
	TracingOptions defaults;

	std::cout << "Usage: " << programName << " <input_file> <output_file> [options]\n\n";

	std::cout << "Basic tracing parameters:\n";
	std::cout << "  --ltres <value>              Line threshold (default: " << defaults.fLineThreshold << ")\n";
	std::cout << "  --pathomit <value>           Path omit threshold (default: " << defaults.fPathOmitThreshold << ")\n";
	std::cout << "  --qtres <value>              Quadratic threshold (default: " << defaults.fQuadraticThreshold << ")\n";
	std::cout << "\n";

	std::cout << "Color quantization:\n";
	std::cout << "  --colors <value>             Number of colors (default: " << static_cast<int>(defaults.fNumberOfColors) << ")\n";
	std::cout << "  --colorquantcycles <value>   Color quantization cycles (default: " << static_cast<int>(defaults.fColorQuantizationCycles) << ")\n";
	std::cout << "\n";

	std::cout << "Preprocessing:\n";
	std::cout << "  --blurdelta <value>          Blur delta (default: " << defaults.fBlurDelta << ")\n";
	std::cout << "  --blurradius <value>         Blur radius (default: " << defaults.fBlurRadius << ")\n";
	std::cout << "\n";

	std::cout << "Path simplification:\n";
	std::cout << "  --aggressive_simplify <value> Aggressive path simplification (0=off, 1=on, default: " << static_cast<int>(defaults.fAggressiveSimplification) << ")\n";
	std::cout << "  --collinear_tolerance <value> Tolerance for merging collinear segments (default: " << defaults.fCollinearTolerance << ")\n";
	std::cout << "  --curve_smoothing <value>     Additional curve smoothing factor (default: " << defaults.fCurveSmoothing << ")\n";
	std::cout << "  --douglas <value>            Enable Douglas-Peucker (0=off, 1=on, default: " << static_cast<int>(defaults.fDouglasPeuckerEnabled) << ")\n";
	std::cout << "  --douglas_curves <value>     Protect curves in Douglas-Peucker (0=off, 1=on, default: " << defaults.fDouglasPeuckerCurveProtection << ")\n";
	std::cout << "  --douglas_tolerance <value>  Douglas-Peucker tolerance (default: " << defaults.fDouglasPeuckerTolerance << ")\n";
	std::cout << "  --min_segment_length <value> Minimum segment length to keep (default: " << defaults.fMinSegmentLength << ")\n";
	std::cout << "\n";

	std::cout << "Geometry detection:\n";
	std::cout << "  --circle_tolerance <value>   Circle detection tolerance (default: " << defaults.fCircleTolerance << ")\n";
	std::cout << "  --detect_geometry <value>    Enable geometry detection (0=off, 1=on, default: " << static_cast<int>(defaults.fDetectGeometry) << ")\n";
	std::cout << "  --line_tolerance <value>     Line detection tolerance (default: " << defaults.fLineTolerance << ")\n";
	std::cout << "  --max_circle_radius <value>  Maximum circle radius (default: " << defaults.fMaxCircleRadius << ")\n";
	std::cout << "  --min_circle_radius <value>  Minimum circle radius (default: " << defaults.fMinCircleRadius << ")\n";
	std::cout << "\n";

	std::cout << "Filtering:\n";
	std::cout << "  --filter_small <value>       Enable small object filtering (0=off, 1=on, default: " << static_cast<int>(defaults.fFilterSmallObjects) << ")\n";
	std::cout << "  --min_area <value>           Minimum object area in pixels (default: " << defaults.fMinObjectArea << ")\n";
	std::cout << "  --min_height <value>         Minimum object height in pixels (default: " << defaults.fMinObjectHeight << ")\n";
	std::cout << "  --min_perimeter <value>      Minimum object perimeter in pixels (default: " << defaults.fMinObjectPerimeter << ")\n";
	std::cout << "  --min_width <value>          Minimum object width in pixels (default: " << defaults.fMinObjectWidth << ")\n";
	std::cout << "\n";

	std::cout << "SVG output\n";
	std::cout << "  --desc <value>               Add description (0=off, 1=on, default: " << static_cast<int>(defaults.fShowDescription) << ")\n";
	std::cout << "  --roundcoords <value>        Round coordinates precision (-1=auto, default: " << defaults.fRoundCoordinates << ")\n";
	std::cout << "  --scale <value>              Scale factor (default: " << defaults.fScale << ")\n";
	std::cout << "  --viewbox <value>            Use viewbox instead of width/height (0=off, 1=on, default: " << static_cast<int>(defaults.fUseViewBox) << ")\n";
	std::cout << "\n";

	std::cout << "SVG optimization:\n";
	std::cout << "  --optimize_svg <value>       Enable SVG optimization (0=off, 1=on, default: " << static_cast<int>(defaults.fOptimizeSvg) << ")\n";
	std::cout << "  --remove_duplicates <value>  Remove duplicate paths (0=off, 1=on, default: " << static_cast<int>(defaults.fRemoveDuplicates) << ")\n";
	std::cout << "\n";

	std::cout << "Debug:\n";
	std::cout << "  --lcpr <value>               Linear control point radius for debug (default: " << defaults.fLineControlPointRadius << ")\n";
	std::cout << "  --qcpr <value>               Quadratic control point radius for debug (default: " << defaults.fQuadraticControlPointRadius << ")\n";
	std::cout << "\n";

	std::cout << "Help:\n";
	std::cout << "  --help                       Show this help\n";
	std::cout << "\n";
	std::cout << "Examples:\n";
	std::cout << "  " << programName << " input.png output.svg\n";
	std::cout << "  " << programName << " input.jpg output.svg -colors 16 -scale 2\n";
	std::cout << "  " << programName << " input.png output.svg -douglas 1 -optimize_svg 1\n";
}

float
ParseFloat(const char* string)
{
	char* endPointer;
	float value = strtof(string, &endPointer);
	if (*endPointer != '\0') {
		std::cerr << "Warning: Invalid float value: " << string << std::endl;
		return 0.0f;
	}
	return value;
}

BitmapData
LoadBitmapData(const std::string& filename)
{
	int width, height, channels;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 4);

	if (!data) {
		std::cerr << "Error: Could not load image: " << filename << std::endl;
		return BitmapData();
	}

	std::vector<unsigned char> bitmapData(data, data + (width * height * 4));
	stbi_image_free(data);

	return BitmapData(width, height, bitmapData);
}

int
main(int argc, char* argv[])
{
	if (argc < 3) {
		std::cerr << "Error: Missing required arguments\n";
		PrintUsage(argv[0]);
		return 1;
	}

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "--help") == 0) {
			PrintUsage(argv[0]);
			return 0;
		}
	}

	std::string inputFile = argv[1];
	std::string outputFile = argv[2];

	TracingOptions options;

	for (int i = 3; i < argc; i++) {
		if (i + 1 < argc) {
			if (strcmp(argv[i], "--ltres") == 0) {
				options.fLineThreshold = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--qtres") == 0) {
				options.fQuadraticThreshold = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--pathomit") == 0) {
				options.fPathOmitThreshold = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--colors") == 0) {
				options.fNumberOfColors = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--colorquantcycles") == 0) {
				options.fColorQuantizationCycles = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--scale") == 0) {
				options.fScale = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--roundcoords") == 0) {
				options.fRoundCoordinates = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--lcpr") == 0) {
				options.fLineControlPointRadius = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--qcpr") == 0) {
				options.fQuadraticControlPointRadius = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--desc") == 0) {
				options.fShowDescription = ParseFloat(argv[++i]) > 0.5f;
			} else if (strcmp(argv[i], "--viewbox") == 0) {
				options.fUseViewBox = ParseFloat(argv[++i]) > 0.5f;
			} else if (strcmp(argv[i], "--blurradius") == 0) {
				options.fBlurRadius = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--blurdelta") == 0) {
				options.fBlurDelta = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--douglas") == 0) {
				options.fDouglasPeuckerEnabled = ParseFloat(argv[++i]) > 0.5f;
			} else if (strcmp(argv[i], "--douglas_tolerance") == 0) {
				options.fDouglasPeuckerTolerance = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--douglas_curves") == 0) {
				options.fDouglasPeuckerCurveProtection = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--detect_geometry") == 0) {
				options.fDetectGeometry = ParseFloat(argv[++i]) > 0.5f;
			} else if (strcmp(argv[i], "--line_tolerance") == 0) {
				options.fLineTolerance = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--circle_tolerance") == 0) {
				options.fCircleTolerance = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--min_circle_radius") == 0) {
				options.fMinCircleRadius = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--max_circle_radius") == 0) {
				options.fMaxCircleRadius = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--optimize_svg") == 0) {
				options.fOptimizeSvg = ParseFloat(argv[++i]) > 0.5f;
			} else if (strcmp(argv[i], "--remove_duplicates") == 0) {
				options.fRemoveDuplicates = ParseFloat(argv[++i]) > 0.5f;
			} else if (strcmp(argv[i], "--aggressive_simplify") == 0) {
				options.fAggressiveSimplification = ParseFloat(argv[++i]) > 0.5f;
			} else if (strcmp(argv[i], "--collinear_tolerance") == 0) {
				options.fCollinearTolerance = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--min_segment_length") == 0) {
				options.fMinSegmentLength = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--curve_smoothing") == 0) {
				options.fCurveSmoothing = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--filter_small") == 0) {
				options.fFilterSmallObjects = ParseFloat(argv[++i]) > 0.5f;
			} else if (strcmp(argv[i], "--min_area") == 0) {
				options.fMinObjectArea = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--min_width") == 0) {
				options.fMinObjectWidth = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--min_height") == 0) {
				options.fMinObjectHeight = ParseFloat(argv[++i]);
			} else if (strcmp(argv[i], "--min_perimeter") == 0) {
				options.fMinObjectPerimeter = ParseFloat(argv[++i]);
			} else {
				std::cerr << "Warning: Unknown option: " << argv[i] << std::endl;
				i++;
			}
		} else {
			std::cerr << "Warning: Option " << argv[i] << " requires a value" << std::endl;
		}
	}

	try {
		BitmapData bitmap = LoadBitmapData(inputFile);
		if (!bitmap.IsValid()) {
			std::cerr << "Error: Failed to load image: " << inputFile << std::endl;
			return 1;
		}

		ImageTracer tracer;
		std::string svgData = tracer.BitmapToSvg(bitmap, options);

		if (!tracer.SaveSvg(outputFile, svgData)) {
			std::cerr << "Error: Failed to save SVG file: " << outputFile << std::endl;
			return 1;
		}

		std::cout << "Conversion completed successfully!" << std::endl;

	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << "Error: Unknown exception occurred" << std::endl;
		return 1;
	}

	return 0;
}
