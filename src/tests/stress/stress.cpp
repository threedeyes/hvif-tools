/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>

#include "IconConverter.h"

void PrintUsage(const char* prog)
{
	std::cerr << "Icon Format Stress Test\n";
	std::cerr << "Usage: " << prog << " <input> <output> [options]\n";
	std::cerr << "\n";
	std::cerr << "Performs random format conversions in memory to test data integrity.\n";
	std::cerr << "\n";
	std::cerr << "Options:\n";
	std::cerr << "  -n, --iterations <n>  Number of iterations (default: 100)\n";
	std::cerr << "  -v, --verbose         Show detailed progress\n";
	std::cerr << "  --formats <list>      Comma-separated list of formats to test\n";
	std::cerr << "                        (hvif,iom,svg or subset, default: all)\n";
	std::cerr << "  --no-svg              Exclude SVG from random conversions\n";
	std::cerr << "  --seed <n>            Random seed for reproducible tests\n";
	std::cerr << "\n";
	std::cerr << "Examples:\n";
	std::cerr << "  " << prog << " icon.hvif result.svg -n 50\n";
	std::cerr << "  " << prog << " icon.iom test.hvif -v --no-svg\n";
	std::cerr << "  " << prog << " icon.svg out.iom --formats hvif,iom\n";
}

struct TestStats {
	int iterations;
	int conversions;
	int errors;
	std::vector<std::string> errorMessages;
	
	TestStats() : iterations(0), conversions(0), errors(0) {}
};

bool CompareIcons(const haiku::Icon& icon1, const haiku::Icon& icon2, bool verbose)
{
	bool same = true;
	
	if (icon1.styles.size() != icon2.styles.size()) {
		if (verbose) {
			std::cerr << "  Mismatch: styles count " << icon1.styles.size() 
			          << " vs " << icon2.styles.size() << "\n";
		}
		same = false;
	}
	
	if (icon1.paths.size() != icon2.paths.size()) {
		if (verbose) {
			std::cerr << "  Mismatch: paths count " << icon1.paths.size() 
			          << " vs " << icon2.paths.size() << "\n";
		}
		same = false;
	}
	
	if (icon1.shapes.size() != icon2.shapes.size()) {
		if (verbose) {
			std::cerr << "  Mismatch: shapes count " << icon1.shapes.size() 
			          << " vs " << icon2.shapes.size() << "\n";
		}
		same = false;
	}
	
	for (size_t i = 0; i < icon1.paths.size() && i < icon2.paths.size(); ++i) {
		if (icon1.paths[i].points.size() != icon2.paths[i].points.size()) {
			if (verbose) {
				std::cerr << "  Mismatch: path[" << i << "] points count " 
				          << icon1.paths[i].points.size() << " vs " 
				          << icon2.paths[i].points.size() << "\n";
			}
			same = false;
		}
	}
	
	return same;
}

std::string FormatName(haiku::IconFormat format)
{
	return haiku::IconConverter::FormatToString(format);
}

int main(int argc, char** argv)
{
	if (argc < 3) {
		PrintUsage(argv[0]);
		return 1;
	}
	
	std::string inputFile;
	std::string outputFile;
	int iterations = 100;
	bool verbose = false;
	unsigned int seed = (unsigned int)time(NULL);
	bool useSeed = false;
	
	std::vector<haiku::IconFormat> testFormats;
	testFormats.push_back(haiku::FORMAT_HVIF);
	testFormats.push_back(haiku::FORMAT_IOM);
	testFormats.push_back(haiku::FORMAT_SVG);
	
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		
		if (arg == "-h" || arg == "--help") {
			PrintUsage(argv[0]);
			return 0;
		} else if (arg == "-n" || arg == "--iterations") {
			if (i + 1 < argc) {
				iterations = std::atoi(argv[++i]);
				if (iterations < 1) iterations = 1;
				if (iterations > 10000) iterations = 10000;
			}
		} else if (arg == "-v" || arg == "--verbose") {
			verbose = true;
		} else if (arg == "--seed") {
			if (i + 1 < argc) {
				seed = (unsigned int)std::atoi(argv[++i]);
				useSeed = true;
			}
		} else if (arg == "--no-svg") {
			testFormats.clear();
			testFormats.push_back(haiku::FORMAT_HVIF);
			testFormats.push_back(haiku::FORMAT_IOM);
		} else if (arg == "--formats") {
			if (i + 1 < argc) {
				testFormats.clear();
				std::string formatList = argv[++i];
				if (formatList.find("hvif") != std::string::npos)
					testFormats.push_back(haiku::FORMAT_HVIF);
				if (formatList.find("iom") != std::string::npos)
					testFormats.push_back(haiku::FORMAT_IOM);
				if (formatList.find("svg") != std::string::npos)
					testFormats.push_back(haiku::FORMAT_SVG);
			}
		} else if (arg[0] == '-') {
			std::cerr << "Unknown option: " << arg << "\n";
			return 1;
		} else {
			if (inputFile.empty()) {
				inputFile = arg;
			} else if (outputFile.empty()) {
				outputFile = arg;
			}
		}
	}
	
	if (inputFile.empty() || outputFile.empty()) {
		std::cerr << "Error: Input and output files required\n";
		PrintUsage(argv[0]);
		return 1;
	}
	
	if (testFormats.empty()) {
		std::cerr << "Error: At least one format must be enabled\n";
		return 1;
	}
	
	srand(seed);
	
	std::cout << "Icon Format Stress Test (in-memory)\n";
	std::cout << "===================================\n";
	std::cout << "Input file:  " << inputFile << "\n";
	std::cout << "Output file: " << outputFile << "\n";
	std::cout << "Iterations:  " << iterations << "\n";
	std::cout << "Random seed: " << seed << (useSeed ? " (user provided)" : " (auto)") << "\n";
	std::cout << "Test formats: ";
	for (size_t i = 0; i < testFormats.size(); ++i) {
		if (i > 0) std::cout << ", ";
		std::cout << FormatName(testFormats[i]);
	}
	std::cout << "\n\n";
	
	haiku::Icon originalIcon = haiku::IconConverter::Load(inputFile, haiku::FORMAT_AUTO);
	if (!haiku::IconConverter::GetLastError().empty()) {
		std::cerr << "Error loading input file: " << haiku::IconConverter::GetLastError() << "\n";
		return 1;
	}
	
	std::cout << "Original icon loaded:\n";
	std::cout << "  Styles: " << originalIcon.styles.size() << "\n";
	std::cout << "  Paths:  " << originalIcon.paths.size() << "\n";
	std::cout << "  Shapes: " << originalIcon.shapes.size() << "\n\n";
	
	TestStats stats;
	stats.iterations = iterations;
	
	haiku::Icon currentIcon = originalIcon;
	
	for (int iter = 0; iter < iterations; ++iter) {
		haiku::IconFormat sourceFormat = testFormats[rand() % testFormats.size()];
		haiku::IconFormat targetFormat = testFormats[rand() % testFormats.size()];
		
		if (testFormats.size() > 1) {
			while (targetFormat == sourceFormat) {
				targetFormat = testFormats[rand() % testFormats.size()];
			}
		}
		
		if (verbose) {
			std::cout << "Iteration " << (iter + 1) << "/" << iterations << ": "
			          << FormatName(sourceFormat) << " -> "
			          << FormatName(targetFormat) << " ... ";
			std::cout.flush();
		}
		
		std::vector<uint8_t> buffer;
		haiku::ConvertOptions optsOut;
		optsOut.verbose = false;
		
		bool saveOk = haiku::IconConverter::SaveToBuffer(currentIcon, buffer, sourceFormat, optsOut);
		if (!saveOk) {
			std::string error = haiku::IconConverter::GetLastError();
			stats.errors++;
			stats.errorMessages.push_back("Iteration " + haiku::IconConverter::FormatToString(sourceFormat) + ": save failed: " + error);
			if (verbose) {
				std::cout << "SAVE FAILED\n";
				std::cerr << "  Error: " << error << "\n";
			}
			continue;
		}
		
		std::vector<uint8_t> outBuffer;
		haiku::ConvertOptions convertOpts;
		convertOpts.verbose = false;
		bool convertOk = haiku::IconConverter::ConvertBuffer(buffer, sourceFormat, outBuffer, targetFormat, convertOpts);
		if (!convertOk) {
			std::string error = haiku::IconConverter::GetLastError();
			stats.errors++;
			stats.errorMessages.push_back("Iteration convert: " + error);
			if (verbose) {
				std::cout << "CONVERT FAILED\n";
				std::cerr << "  Error: " << error << "\n";
			}
			continue;
		}
		
		haiku::Icon loadedIcon = haiku::IconConverter::LoadFromBuffer(outBuffer, targetFormat);
		if (!haiku::IconConverter::GetLastError().empty()) {
			std::string error = haiku::IconConverter::GetLastError();
			stats.errors++;
			stats.errorMessages.push_back("Iteration load result: " + error);
			if (verbose) {
				std::cout << "LOAD FAILED\n";
				std::cerr << "  Error: " << error << "\n";
			}
			continue;
		}
		
		stats.conversions++;
		
		if (verbose) {
			bool same = CompareIcons(currentIcon, loadedIcon, false);
			std::cout << "OK";
			if (!same) {
				std::cout << " (data changed)";
			}
			std::cout << "\n";
		} else if ((iter + 1) % 10 == 0) {
			std::cout << ".";
			std::cout.flush();
		}
		
		currentIcon = loadedIcon;
	}
	
	if (!verbose) {
		std::cout << "\n";
	}
	
	std::cout << "\nSaving final result to file...\n";
	haiku::IconFormat outputFormat = haiku::IconConverter::DetectFormatByExtension(outputFile);
	haiku::ConvertOptions finalOpts;
	finalOpts.verbose = verbose;
	
	if (!haiku::IconConverter::Save(currentIcon, outputFile, outputFormat, finalOpts)) {
		std::cerr << "Error saving final result: " << haiku::IconConverter::GetLastError() << "\n";
		return 1;
	}
	
	haiku::Icon finalIcon = haiku::IconConverter::Load(outputFile, outputFormat);
	
	std::cout << "\nTest Results:\n";
	std::cout << "=============\n";
	std::cout << "Total iterations:       " << stats.iterations << "\n";
	std::cout << "Successful conversions: " << stats.conversions << "\n";
	std::cout << "Errors:                 " << stats.errors << "\n";
	std::cout << "Success rate:           " 
	          << (stats.iterations > 0 ? (stats.conversions * 100 / stats.iterations) : 0) << "%\n\n";
	
	if (!stats.errorMessages.empty()) {
		std::cout << "Errors encountered:\n";
		for (size_t i = 0; i < stats.errorMessages.size(); ++i) {
			std::cout << "  " << stats.errorMessages[i] << "\n";
		}
		std::cout << "\n";
	}
	
	std::cout << "Original icon:\n";
	std::cout << "  Styles: " << originalIcon.styles.size() << "\n";
	std::cout << "  Paths:  " << originalIcon.paths.size() << "\n";
	std::cout << "  Shapes: " << originalIcon.shapes.size() << "\n\n";
	
	std::cout << "Final icon (from file " << outputFile << "):\n";
	std::cout << "  Styles: " << finalIcon.styles.size() << "\n";
	std::cout << "  Paths:  " << finalIcon.paths.size() << "\n";
	std::cout << "  Shapes: " << finalIcon.shapes.size() << "\n\n";
	
	bool dataSame = CompareIcons(originalIcon, finalIcon, verbose);
	
	if (dataSame) {
		std::cout << "Data integrity check: PASSED\n";
	} else {
		std::cout << "Data integrity check: FAILED\n";
		std::cout << "  Warning: Icon structure changed during conversions\n";
	}
	
	if (stats.errors == 0 && dataSame) {
		std::cout << "\nAll tests PASSED\n";
		return 0;
	} else if (stats.errors > 0) {
		std::cout << "\nTests completed with " << stats.errors << " errors\n";
		return 1;
	} else {
		std::cout << "\nTests completed but data integrity affected\n";
		return 2;
	}
}
