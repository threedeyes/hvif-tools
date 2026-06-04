/*
 * Copyright 2025-2026, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <iostream>
#include <chrono>
#include <sstream>
#include <string>
#include <cstdlib>

#include "embedded_ui.h"

#include "httplib.h"
#include "ImageTracer.h"
#include "TracingOptions.h"

#ifndef __HAIKU__
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

void PrintUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --host <ip>      Bind to network interface (default: 0.0.0.0)\n";
    std::cout << "  --port <port>    Listen on port (default: 8200)\n";
    std::cout << "  --help           Show this help message\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << programName << "\n";
    std::cout << "  " << programName << " --port 8080\n";
    std::cout << "  " << programName << " --host 127.0.0.1 --port 9000\n";
}

std::string escape_json(const std::string& s) {
    std::ostringstream o;
    for (char c : s) {
        if (c == '"') o << "\\\"";
        else if (c == '\\') o << "\\\\";
        else if (c == '\b') o << "\\b";
        else if (c == '\f') o << "\\f";
        else if (c == '\n') o << "\\n";
        else if (c == '\r') o << "\\r";
        else if (c == '\t') o << "\\t";
        else o << c;
    }
    return o.str();
}

float get_float(const httplib::Request& req, const std::string& name, float def) {
    if (req.form.has_field(name)) {
        try { return std::stof(req.form.get_field(name)); } 
        catch (...) { return def; }
    }
    return def;
}

bool get_bool(const httplib::Request& req, const std::string& name, bool def) {
    if (req.form.has_field(name)) {
        std::string val = req.form.get_field(name);
        return val == "true" || val == "1";
    }
    return def;
}

int main(int argc, char** argv) {
    std::string host = "0.0.0.0";
    int port = 8200;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            PrintUsage(argv[0]);
            return 0;
        } else if (arg == "--host" && i + 1 < argc) {
            host = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            try {
                port = std::stoi(argv[++i]);
            } catch (...) {
                std::cerr << "Error: Invalid port number: " << argv[i] << "\n";
                return 1;
            }
        } else {
            std::cerr << "Warning: Unknown option: " << arg << "\n";
            PrintUsage(argv[0]);
            return 1;
        }
    }

    httplib::Server svr;

    svr.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content(embedded_index_html, "text/html; charset=utf-8");
    });

    svr.Get("/api/defaults", [](const httplib::Request& req, httplib::Response& res) {
        TracingOptions opt;
        std::ostringstream json;
        
        json << "{";
        json << "\"colors\": " << opt.fNumberOfColors << ",";
        json << "\"colorquantcycles\": " << opt.fColorQuantizationCycles << ",";
        json << "\"ltres\": " << opt.fLineThreshold << ",";
        json << "\"qtres\": " << opt.fQuadraticThreshold << ",";
        json << "\"pathomit\": " << opt.fPathOmitThreshold << ",";
        json << "\"remove_bg\": " << (opt.fRemoveBackground ? "true" : "false") << ",";
        json << "\"bg_method\": " << (int)opt.fBackgroundMethod << ",";
        json << "\"bg_tolerance\": " << opt.fBackgroundTolerance << ",";
        json << "\"bg_ratio\": " << opt.fMinBackgroundRatio << ",";
        json << "\"blurradius\": " << opt.fBlurRadius << ",";
        json << "\"blurdelta\": " << opt.fBlurDelta << ",";
        json << "\"douglas\": " << (opt.fDouglasPeuckerEnabled ? "true" : "false") << ",";
        json << "\"douglas_tolerance\": " << opt.fDouglasPeuckerTolerance << ",";
        json << "\"douglas_curves\": " << (opt.fDouglasPeuckerCurveProtection > 0.5f ? "true" : "false") << ",";
        json << "\"aggressive_simplify\": " << (opt.fAggressiveSimplification ? "true" : "false") << ",";
        json << "\"collinear_tolerance\": " << opt.fCollinearTolerance << ",";
        json << "\"min_segment_length\": " << opt.fMinSegmentLength << ",";
        json << "\"curve_smoothing\": " << opt.fCurveSmoothing << ",";
        json << "\"vw_enable\": " << (opt.fVisvalingamWhyattEnabled ? "true" : "false") << ",";
        json << "\"vw_tolerance\": " << opt.fVisvalingamWhyattTolerance << ",";
        json << "\"detect_gradients\": " << (opt.fDetectGradients ? "true" : "false") << ",";
        json << "\"grad_r2\": " << opt.fGradientMinR2 << ",";
        json << "\"grad_delta\": " << opt.fGradientMinDelta << ",";
        json << "\"grad_stride\": " << opt.fGradientSampleStride << ",";
        json << "\"detect_geometry\": " << (opt.fDetectGeometry ? "true" : "false") << ",";
        json << "\"line_tolerance\": " << opt.fLineTolerance << ",";
        json << "\"circle_tolerance\": " << opt.fCircleTolerance << ",";
        json << "\"filter_small\": " << (opt.fFilterSmallObjects ? "true" : "false") << ",";
        json << "\"min_area\": " << opt.fMinObjectArea << ",";
        json << "\"min_perimeter\": " << opt.fMinObjectPerimeter;
        json << "}";

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(json.str(), "application/json");
    });

    svr.Post("/api/vectorize", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");

        if (!req.form.has_file("image")) {
            res.status = 400;
            res.set_content("{\"error\": \"No image uploaded\"}", "application/json");
            return;
        }

        const auto& file = req.form.get_file("image");

        TracingOptions opt;
        opt.fUseViewBox = true;
        
        opt.fNumberOfColors = get_float(req, "colors", opt.fNumberOfColors);
        opt.fColorQuantizationCycles = get_float(req, "colorquantcycles", opt.fColorQuantizationCycles);
        opt.fLineThreshold = get_float(req, "ltres", opt.fLineThreshold);
        opt.fQuadraticThreshold = get_float(req, "qtres", opt.fQuadraticThreshold);
        opt.fPathOmitThreshold = get_float(req, "pathomit", opt.fPathOmitThreshold);
        
        opt.fRemoveBackground = get_bool(req, "remove_bg", opt.fRemoveBackground);
        int bg_m = (int)get_float(req, "bg_method", 1.0f);
        opt.fBackgroundMethod = (bg_m == 0 || bg_m == 1) ? (BackgroundDetectionMethod)bg_m : AUTO;
        opt.fBackgroundTolerance = (int)get_float(req, "bg_tolerance", opt.fBackgroundTolerance);
        opt.fMinBackgroundRatio = get_float(req, "bg_ratio", opt.fMinBackgroundRatio);

        opt.fBlurRadius = get_float(req, "blurradius", opt.fBlurRadius);
        opt.fBlurDelta = get_float(req, "blurdelta", opt.fBlurDelta);

        opt.fDouglasPeuckerEnabled = get_bool(req, "douglas", opt.fDouglasPeuckerEnabled);
        opt.fDouglasPeuckerTolerance = get_float(req, "douglas_tolerance", opt.fDouglasPeuckerTolerance);
        opt.fDouglasPeuckerCurveProtection = get_bool(req, "douglas_curves", opt.fDouglasPeuckerCurveProtection) ? 1.0f : 0.0f;
        opt.fAggressiveSimplification = get_bool(req, "aggressive_simplify", opt.fAggressiveSimplification);
        opt.fCollinearTolerance = get_float(req, "collinear_tolerance", opt.fCollinearTolerance);
        opt.fMinSegmentLength = get_float(req, "min_segment_length", opt.fMinSegmentLength);
        opt.fCurveSmoothing = get_float(req, "curve_smoothing", opt.fCurveSmoothing);

        opt.fVisvalingamWhyattEnabled = get_bool(req, "vw_enable", opt.fVisvalingamWhyattEnabled);
        opt.fVisvalingamWhyattTolerance = get_float(req, "vw_tolerance", opt.fVisvalingamWhyattTolerance);

        opt.fDetectGradients = get_bool(req, "detect_gradients", opt.fDetectGradients);
        opt.fGradientMinR2 = get_float(req, "grad_r2", opt.fGradientMinR2);
        opt.fGradientMinDelta = get_float(req, "grad_delta", opt.fGradientMinDelta);
        opt.fGradientSampleStride = (int)get_float(req, "grad_stride", opt.fGradientSampleStride);

        opt.fDetectGeometry = get_bool(req, "detect_geometry", opt.fDetectGeometry);
        opt.fLineTolerance = get_float(req, "line_tolerance", opt.fLineTolerance);
        opt.fCircleTolerance = get_float(req, "circle_tolerance", opt.fCircleTolerance);

        opt.fFilterSmallObjects = get_bool(req, "filter_small", opt.fFilterSmallObjects);
        opt.fMinObjectArea = get_float(req, "min_area", opt.fMinObjectArea);
        opt.fMinObjectPerimeter = get_float(req, "min_perimeter", opt.fMinObjectPerimeter);

        int width, height, channels;
        unsigned char* img_data = stbi_load_from_memory(
            reinterpret_cast<const stbi_uc*>(file.content.data()),
            file.content.size(),
            &width, &height, &channels, 4
        );

        if (!img_data) {
            res.status = 400;
            res.set_content("{\"error\": \"Could not decode uploaded image\"}", "application/json");
            return;
        }

        std::vector<unsigned char> bitmapDataArray(img_data, img_data + (width * height * 4));
        stbi_image_free(img_data);
        
        BitmapData bitmap(width, height, bitmapDataArray);

        auto start_time = std::chrono::high_resolution_clock::now();
        
        ImageTracer tracer;
        std::string svgData;
        try {
            svgData = tracer.BitmapToSvg(bitmap, opt);
        } catch (const std::exception& e) {
            res.status = 500;
            std::string err_json = "{\"error\": \"" + escape_json(e.what()) + "\"}";
            res.set_content(err_json, "application/json");
            return;
        } catch (...) {
            res.status = 500;
            res.set_content("{\"error\": \"Unknown vectorization error\"}", "application/json");
            return;
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        std::string jsonResponse = "{\"timeMs\": " + std::to_string(time_ms) + ", \"svg\": \"" + escape_json(svgData) + "\"}";
        res.set_content(jsonResponse, "application/json");
    });

    std::cout << "Starting img2svg Studio Server...\n";
    std::cout << "Available CPU cores: " << std::thread::hardware_concurrency() << "\n";
    std::cout << "Listening on http://" << host << ":" << port << "\n";

    if (!svr.listen(host.c_str(), port)) {
        std::cerr << "Error: Failed to bind to " << host << ":" << port << "\n";
        return 1;
    }

    return 0;
}
