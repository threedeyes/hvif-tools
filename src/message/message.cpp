/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <fstream>
#include <iostream>
#include <cstring>

#include "BMessage.h"

static void
print_usage(const char* program)
{
	std::cerr << "Usage: " << program << " [OPTIONS] <file>\n"
		<< "Options:\n"
		<< "  -v, --values    Show field values\n"
		<< "  -x, --hexdump   Show full hex dump\n"
		<< "  -h, --help      Show this help message\n";
}


static void
hexdump(const char* data, size_t size, size_t maxBytes = 0)
{
	if (maxBytes > 0 && size > maxBytes)
		size = maxBytes;

	for (size_t i = 0; i < size; i += 16) {
		printf("  %04zx:", i);

		for (size_t j = 0; j < 16; j++) {
			if (i + j < size)
				printf(" %02x", (unsigned char)data[i + j]);
			else
				printf("   ");
		}

		printf("  |");
		for (size_t j = 0; j < 16 && (i + j) < size; j++) {
			unsigned char c = data[i + j];
			printf("%c", (c >= 32 && c < 127) ? c : '.');
		}
		printf("|\n");
	}
}


int
main(int argc, char* argv[])
{
	if (argc < 2) {
		print_usage(argv[0]);
		return 1;
	}

	bool showValues = false;
	bool showHexdump = false;
	const char* filename = NULL;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--values") == 0) {
			showValues = true;
		} else if (strcmp(argv[i], "-x") == 0
			|| strcmp(argv[i], "--hexdump") == 0) {
			showHexdump = true;
		} else if (strcmp(argv[i], "-h") == 0
			|| strcmp(argv[i], "--help") == 0) {
			print_usage(argv[0]);
			return 0;
		} else {
			filename = argv[i];
		}
	}

	if (filename == NULL) {
		std::cerr << "Error: No input file specified\n";
		print_usage(argv[0]);
		return 1;
	}

	std::ifstream file(filename, std::ios::binary);
	if (!file) {
		std::cerr << "Error: Cannot open file '" << filename << "'\n";
		return 1;
	}

	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	file.seekg(0, std::ios::beg);

	if (size == 0) {
		std::cerr << "Error: File is empty\n";
		return 1;
	}

	char* buffer = new char[size];
	file.read(buffer, size);
	file.close();

	if (showHexdump) {
		std::cout << "File: " << filename << "\n";
		std::cout << "Size: " << size << " bytes\n\n";
		std::cout << "Hex dump:\n";
		hexdump(buffer, size);
		std::cout << "\n";
	}

	BMessage message;
	status_t result = message.Unflatten(buffer, size);
	delete[] buffer;

	if (result != B_OK) {
		std::cerr << "Error: Failed to unflatten message (status="
			<< result << ")\n";
		return 1;
	}

	message.PrintToStream(showValues);

	return 0;
}
