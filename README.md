# hvif-tools

A small set of command‑line tools to work with the Haiku Vector Icon Format (HVIF)

## Features
- hvif2svg
  - Parses .hvif with header validation.
  - Renders paths (points/curves), shapes, and styles (solid colors and gradients).
  - Supports fills and strokes (via HVIF transformers STROKE/CONTOUR).
  - Linear and radial gradients with gradientTransform; proper stop opacity.
  - Uses HVIF’s internal grid (viewBox 0 0 6528 6528 = 64 × 102 units).

- svg2hvif
  - SVG parsing via NanoSVG (header‑only).
  - Normalizes coordinates to HVIF’s 64 × 64 space and centers content.
  - Converts fill and stroke (caps/joins/miter limit), stroke widths, and opacity.
  - Linear and radial gradients with calculated HVIF gradient transforms.
  - CLI and buffer‑to‑buffer example for embedding into apps/pipelines.

- Cross‑platform: Linux, macOS, Windows and Haiku

## Build
Requirements:
- CMake 3.16+
- A C++ compiler

Commands:
```bash
git clone https://github.com/your-org/hvif-tools.git  --recursive
cd hvif-tools
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

Executables will appear in the build directory.

## Usage
- HVIF → SVG:
```bash
hvif2svg input.hvif output.svg [width] [height]
# Example:
hvif2svg icon.hvif icon.svg
```

- SVG → HVIF:
```bash
svg2hvif [-v | --verbose] input.svg output.hvif
# Examples:
svg2hvif -v logo.svg logo.hvif
svg2hvif input.svg output.hvif
```

## Limitations and notes
- Not a full HVIF implementation:
  - AFFINE/PERSPECTIVE transformers are not supported (hvif2svg errors on AFFINE).
  - Some gradient types (DIAMOND/CONIC/XY/SQRTXY) are rendered as radial in SVG.
- Hinting and LOD scale flags are read/partially considered but don’t map directly to SVG.
- Coordinate quantization and HVIF’s internal grid (×102) may lead to minor differences from source SVGs.
- Shape transforms: hvif2svg outputs matrix/translate when present; svg2hvif currently focuses on gradient transforms rather than exporting general shape transforms.

## Acknowledgements
- [NanoSVG](https://github.com/memononen/nanosvg) — lightweight SVG parser.
