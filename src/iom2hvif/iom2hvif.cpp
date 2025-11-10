/*
 * Copyright 2006-2025, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Based on Haiku Icon-O-Matic implementation by:
 *   Stephan Aßmus <superstippi@gmx.de>
 *   Zardshard
 *
 * Adapted as standalone converter without GUI dependencies.
 */

#include <stdio.h>
#include <string.h>
#include <File.h>
#include <Node.h>
#include <Path.h>
#include <String.h>
#include "core/Icon.h"
#include "io/MessageImporter.h"
#include "io/FlatIconExporter.h"

#define MAX_INPUT_FILES 2048

static void
print_usage(const char* program)
{
    printf("Usage:\n");
    printf("  %s [options] <file.iom> [file2.iom ...]\n\n", program);
    printf("Options:\n");
    printf("  -o <file.hvif>       Write to HVIF file (requires single input)\n");
    printf("  -a <target>          Write to target's attribute (requires single input)\n");
    printf("  --attr-name <name>   Attribute name (default: BEOS:ICON)\n");
    printf("  -v, --verbose        Verbose output\n");
    printf("  -h, --help           Show this help\n\n");
    printf("Batch mode (default):\n");
    printf("  %s myicon.iom             - Write icon to myicon.iom's attribute\n", program);
    printf("  %s *.iom                  - Process all .iom files\n", program);
    printf("  %s icon1.iom icon2.iom    - Process multiple files\n\n", program);
    printf("Single file mode:\n");
    printf("  %s -o app.hvif app.iom    - Convert to HVIF file\n", program);
    printf("  %s -a MyApp app.iom       - Write to MyApp's attribute\n", program);
    printf("  %s -a /path/to/file icon.iom\n", program);
    printf("\nFormats:\n");
    printf("  .iom  - Icon-O-Matic native format (BMessage-based)\n");
    printf("  .hvif - Haiku Vector Icon Format (compact binary)\n");
    printf("\nLimits: Up to %d files per batch\n", MAX_INPUT_FILES);
}

static status_t
import_icon(const char* path, Icon& icon, bool verbose)
{
    BFile inFile;
    status_t ret = inFile.SetTo(path, B_READ_ONLY);
    if (ret != B_OK) {
        if (verbose)
            fprintf(stderr, "Failed to open '%s': %s\n", path, strerror(ret));
        return ret;
    }

    MessageImporter importer;
    ret = importer.Import(&icon, &inFile);
    if (ret != B_OK) {
        if (verbose)
            fprintf(stderr, "Failed to import '%s': %s\n", path, strerror(ret));
        return ret;
    }

    return B_OK;
}

static status_t
export_to_hvif(const Icon& icon, const char* path, bool verbose)
{
    BFile outFile;
    status_t ret = outFile.SetTo(path, B_CREATE_FILE | B_WRITE_ONLY | B_ERASE_FILE);
    if (ret != B_OK) {
        if (verbose)
            fprintf(stderr, "Failed to create '%s': %s\n", path, strerror(ret));
        return ret;
    }

    FlatIconExporter exporter;
    ret = exporter.Export(&icon, &outFile);
    if (ret != B_OK) {
        if (verbose)
            fprintf(stderr, "Failed to export to '%s': %s\n", path, strerror(ret));
        return ret;
    }

    return B_OK;
}

static status_t
export_to_attr(const Icon& icon, const char* path, const char* attrName, bool verbose)
{
    BNode node;
    status_t ret = node.SetTo(path);
    if (ret != B_OK) {
        if (verbose)
            fprintf(stderr, "Failed to open '%s': %s\n", path, strerror(ret));
        return ret;
    }

    FlatIconExporter exporter;
    ret = exporter.ExportToAttribute(&icon, &node, attrName);
    if (ret != B_OK) {
        if (verbose)
            fprintf(stderr, "Failed to write attribute to '%s': %s\n", path, strerror(ret));
        return ret;
    }

    return B_OK;
}

static int
process_single(const char* inputPath, const char* outputPath,
               const char* attrTarget, const char* attrName, bool verbose)
{
    if (verbose) {
        printf("Processing: %s\n", inputPath);
    }

    Icon icon;
    status_t ret = import_icon(inputPath, icon, verbose);
    if (ret != B_OK) {
        fprintf(stderr, "Error: Failed to import '%s': %s\n", inputPath, strerror(ret));
        return 1;
    }

    if (verbose) {
        printf("  Styles: %d, Paths: %d, Shapes: %d\n",
               (int)icon.Styles()->CountItems(),
               (int)icon.Paths()->CountItems(),
               (int)icon.Shapes()->CountItems());
    }

    if (outputPath) {
        if (verbose)
            printf("  Writing to: %s\n", outputPath);
        ret = export_to_hvif(icon, outputPath, verbose);
        if (ret != B_OK) {
            fprintf(stderr, "Error: Export failed\n");
            return 1;
        }
    } else if (attrTarget) {
        if (verbose)
            printf("  Writing attribute '%s' to: %s\n", attrName, attrTarget);
        ret = export_to_attr(icon, attrTarget, attrName, verbose);
        if (ret != B_OK) {
            fprintf(stderr, "Error: Attribute write failed\n");
            return 1;
        }
    }

    return 0;
}

static int
process_batch(const char** files, int count, const char* attrName, bool verbose)
{
    int succeeded = 0;
    int failed = 0;

    for (int i = 0; i < count; i++) {
        const char* path = files[i];
        
        if (verbose)
            printf("Processing [%d/%d]: %s\n", i + 1, count, path);

        Icon icon;
        status_t ret = import_icon(path, icon, false);
        if (ret != B_OK) {
            if (verbose)
                fprintf(stderr, "  Failed to import: %s\n", strerror(ret));
            failed++;
            continue;
        }

        if (verbose) {
            printf("  Styles: %d, Paths: %d, Shapes: %d\n",
                   (int)icon.Styles()->CountItems(),
                   (int)icon.Paths()->CountItems(),
                   (int)icon.Shapes()->CountItems());
        }

        ret = export_to_attr(icon, path, attrName, false);
        if (ret != B_OK) {
            if (verbose)
                fprintf(stderr, "  Failed to write attribute: %s\n", strerror(ret));
            failed++;
            continue;
        }

        if (verbose)
            printf("  Done\n");
        
        succeeded++;
    }

    if (count > 1 || failed > 0) {
        printf("\nProcessed %d file(s): %d succeeded, %d failed\n",
               count, succeeded, failed);
    }

    return failed > 0 ? 1 : 0;
}

int
main(int argc, char** argv)
{
    const char* outputPath = NULL;
    const char* attrTarget = NULL;
    const char* attrName = "BEOS:ICON";
    bool verbose = false;
    
    const char* inputFiles[MAX_INPUT_FILES];
    int inputFileCount = 0;
    bool maxFilesWarningShown = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose = true;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -o requires an argument\n");
                return 1;
            }
            outputPath = argv[++i];
        } else if (strcmp(argv[i], "-a") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -a requires an argument\n");
                return 1;
            }
            attrTarget = argv[++i];
        } else if (strcmp(argv[i], "--attr-name") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --attr-name requires an argument\n");
                return 1;
            }
            attrName = argv[++i];
        } else {
            if (inputFileCount >= MAX_INPUT_FILES) {
                if (!maxFilesWarningShown) {
                    fprintf(stderr, "Warning: Maximum %d files supported, ignoring the rest\n",
                            MAX_INPUT_FILES);
                    maxFilesWarningShown = true;
                }
            } else {
                inputFiles[inputFileCount++] = argv[i];
            }
        }
    }

    if (inputFileCount == 0) {
        fprintf(stderr, "Error: No input file(s) specified\n\n");
        print_usage(argv[0]);
        return 1;
    }

    if (outputPath && attrTarget) {
        fprintf(stderr, "Error: Cannot use both -o and -a\n");
        return 1;
    }

    if ((outputPath || attrTarget) && inputFileCount != 1) {
        fprintf(stderr, "Error: -o and -a require exactly one input file\n");
        return 1;
    }

    if (outputPath || attrTarget) {
        return process_single(inputFiles[0], outputPath, attrTarget, attrName, verbose);
    } else {
        return process_batch(inputFiles, inputFileCount, attrName, verbose);
    }
}
