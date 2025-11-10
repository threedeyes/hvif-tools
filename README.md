# hvif-tools

A comprehensive set of command-line tools and extensions for working with Haiku Vector Icon Format (HVIF) and Icon-O-Matic (IOM) files.

## Components

### Core Conversion Tools

**hvif2svg** - Convert HVIF files to SVG format
- Full HVIF format support with coordinate transformation
- Solid colors and gradients (linear, radial, diamond, conic, XY, sqrt-XY)
- Fill and stroke rendering via transformers
- HVIF internal grid mapping (64×64 to 6528×6528)

**svg2hvif** - Convert SVG files to HVIF format
- SVG parsing via NanoSVG library
- Automatic coordinate normalization and centering
- Fill, stroke, gradient support with transform calculation
- HVIF format limitation validation (max 255 styles/paths/shapes)

**iom2svg** - Convert Icon-O-Matic files to SVG
- BMessage-based IOM format parsing
- Named elements support (preserve as SVG IDs with --names flag)
- Full transformation matrices and gradient interpolation

**svg2iom** - Convert SVG files to Icon-O-Matic format
- Complete SVG path parsing with BMessage output
- Gradient conversion with transformations
- Preserves vector quality

**iom2hvif** - Convert Icon-O-Matic files to HVIF format (Haiku only)
- Native Haiku implementation using Haiku API
- Direct IOM to HVIF conversion without intermediate formats
- Write icons to HVIF files or file/folder attributes
- Batch processing support for multiple files
- Based on Haiku Icon-O-Matic implementation

**img2svg** - Advanced bitmap to SVG vectorization
- Color quantization with adaptive palettes
- Multiple simplification algorithms (Douglas-Peucker, Visvalingam-Whyatt)
- Geometric shape detection
- Background removal with auto-detection
- Gradient detection and region merging

### Integration

**Inkscape Extensions** - Seamless HVIF/IOM import and export
- Python-based extensions for Inkscape 1.0+
- Requires corresponding CLI tools in PATH

**Windows Thumbnail Provider** - Explorer integration
- COM-based thumbnail generation for HVIF and IOM files
- Requires registration via regsvr32

### Development Tools

**msg2txt** - BMessage binary format dump utility
- Structure and field enumeration
- Optional value display and hex dump
- Offset-based reading

## Building

### Requirements

- CMake 3.16 or later
- C++ compiler
- Git (for submodules)

External dependencies (included as submodules):
- NanoSVG (header-only)
- STB (header-only)

Platform-specific dependencies:
- **Haiku:** libagg (Anti-Grain Geometry), be library (for iom2hvif)

### Quick Start

```bash
git clone https://github.com/your-org/hvif-tools.git --recursive
cd hvif-tools
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### Build Options

**Tools:**
- `BUILD_ALL_TOOLS` - Build all CLI tools (default: ON)
- `BUILD_HVIF2SVG` - Build hvif2svg converter (default: ON)
- `BUILD_SVG2HVIF` - Build svg2hvif converter (default: ON)
- `BUILD_IOM2SVG` - Build iom2svg converter (default: ON)
- `BUILD_SVG2IOM` - Build svg2iom converter (default: ON)
- `BUILD_IMG2SVG` - Build img2svg vectorizer (default: ON)
- `BUILD_MSG2TXT` - Build msg2txt debug tool (default: OFF)
- `BUILD_IOM2HVIF` - Build iom2hvif converter (default: ON on Haiku, unavailable elsewhere)

**Integration:**
- `BUILD_HVIF4WIN` - Build Windows thumbnail provider (default: ON on Windows)
- `BUILD_INKSCAPE_EXTENSIONS` - Install Inkscape extensions (default: OFF)
- `INKSCAPE_EXT_DIR` - Custom Inkscape extensions directory (default: platform-specific)

**Development:**
- `HVIF_TOOLS_WARNINGS` - Enable extra compiler warnings (default: OFF)
- `HVIF_TOOLS_WERROR` - Treat warnings as errors (default: OFF)

### Build Examples

Build only HVIF converters:
```bash
cmake -B build -DBUILD_ALL_TOOLS=OFF -DBUILD_HVIF2SVG=ON -DBUILD_SVG2HVIF=ON
cmake --build build
```

Build with Inkscape extensions:
```bash
cmake -B build -DBUILD_INKSCAPE_EXTENSIONS=ON
cmake --build build
cmake --install build
```

Build with custom Inkscape extensions path:
```bash
cmake -B build \
    -DBUILD_INKSCAPE_EXTENSIONS=ON \
    -DINKSCAPE_EXT_DIR=/custom/path/to/inkscape/extensions
cmake --build build
cmake --install build
```

Development build:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DHVIF_TOOLS_WARNINGS=ON
cmake --build build
```

## Installation

System-wide:
```bash
cmake --install build --prefix /usr/local
```

### Inkscape Extensions Installation

When `BUILD_INKSCAPE_EXTENSIONS=ON`, extensions are installed to platform-specific directories:

**Default paths:**
- **Linux:** `~/.config/inkscape/extensions`
- **macOS:** `~/Library/Application Support/org.inkscape.Inkscape/config/inkscape/extensions`
- **Windows:** `%APPDATA%\inkscape\extensions`
- **Haiku:** `~/config/settings/inkscape/extensions`

**Custom installation path:**

Use `INKSCAPE_EXT_DIR` to override the default:
```bash
cmake -B build -DBUILD_INKSCAPE_EXTENSIONS=ON -DINKSCAPE_EXT_DIR=/path/to/extensions
cmake --install build
```

**Manual installation:**

Copy extension files directly:
```bash
cp inkscape/*.{py,inx} <inkscape-extensions-dir>/
```

On Unix-like systems, make Python scripts executable:
```bash
chmod +x <inkscape-extensions-dir>/*.py
```

## Usage

### Basic Conversion

```bash
hvif2svg icon.hvif icon.svg 256 256
svg2hvif icon.svg icon.hvif
iom2svg icon.iom icon.svg 128 128 --names
svg2iom icon.svg icon.iom
```

### Icon-O-Matic to HVIF (Haiku only)

**Convert to HVIF file:**
```bash
iom2hvif -o output.hvif input.iom
```

**Batch mode (write to file attributes):**
```bash
# Process single file
iom2hvif myicon.iom

# Process multiple files
iom2hvif *.iom
iom2hvif icon1.iom icon2.iom icon3.iom
```

**Write to another file's attribute:**
```bash
iom2hvif -a /boot/home/MyApp app.iom
```

**Custom attribute name:**
```bash
iom2hvif --attr-name CUSTOM:ICON -a target.file icon.iom
```

**Verbose output:**
```bash
iom2hvif -v icon1.iom icon2.iom
```

**iom2hvif options:**
```
-o <file.hvif>      Write to HVIF file (single input only)
-a <target>         Write to target's attribute (single input only)
--attr-name <name>  Attribute name (default: BEOS:ICON)
-v, --verbose       Verbose output
-h, --help          Show help
```

**Example output:**

Silent on success:
```bash
$ iom2hvif -o app.hvif app.iom
$
```

Batch processing:
```bash
$ iom2hvif *.iom
Processed 15 file(s): 15 succeeded, 0 failed
```

Verbose mode:
```bash
$ iom2hvif -v icon1.iom icon2.iom
Processing [1/2]: icon1.iom
  Styles: 2, Paths: 3, Shapes: 2
  Done
Processing [2/2]: icon2.iom
  Styles: 1, Paths: 1, Shapes: 1
  Done

Processed 2 file(s): 2 succeeded, 0 failed
```

### Bitmap Vectorization

```bash
img2svg photo.jpg vector.svg --colors 32 --scale 2
img2svg logo.png logo.svg --remove_bg 1 --detect_geometry 1
img2svg --help
```

### Debug BMessage

```bash
msg2txt example.msg       # Show BMessage structure
msg2txt -o 4 -v icon.iom  # Show IOM BMessage structure with values
msg2txt -x example.msg    # Hex dump
```

## Platform Notes

### Windows

Build with Visual Studio:
```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Thumbnail provider registration (requires administrator):
```cmd
regsvr32 HVIFThumbnailProvider.dll
```

### Linux

Install dependencies:
```bash
# Debian/Ubuntu
sudo apt install build-essential cmake git

# Fedora
sudo dnf install gcc-c++ cmake git
```

### Haiku

Install dependencies:
```bash
pkgman install cmake gcc libagg_devel
```

Build all tools including iom2hvif:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Technical Details

### HVIF Format
- Grid: 64×64 logical units → 6528×6528 internal
- Coordinate precision: 24-bit fixed-point
- Limits: 255 styles, 255 paths, 255 shapes
- Gradient types: Linear, Radial, Diamond, Conic, XY, Sqrt-XY

### IOM Format
- Container: BMessage binary format
- Precision: Double-precision floating point
- Features: Named elements, full transformation matrices

### SVG Compatibility
- Supported: Paths, basic shapes, fill/stroke, gradients, transforms, opacity
- Not supported: Text (convert to paths), filters, masks, patterns, animations

### Haiku File Attributes
On Haiku, icons can be stored as file attributes (metadata). The `iom2hvif` tool supports writing HVIF data to:
- **BEOS:ICON** (default) - Standard Haiku vector icon attribute
- Custom attribute names via `--attr-name` option

## License

MIT License

Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com

## Acknowledgements

- [NanoSVG](https://github.com/memononen/nanosvg) by Mikko Mononen - SVG parser
- [STB](https://github.com/nothings/stb) by Sean Barrett - Image loading
- [imagetracerjs](https://github.com/jankovicsandras/imagetracerjs) by András Jankovics - Vectorization algorithms inspiration
- [Anti-Grain Geometry](https://sourceforge.net/projects/agg/) - 2D graphics rendering library
- Stephan Aßmus - Original Haiku Icon-O-Matic implementation
- Haiku Project - HVIF and Icon-O-Matic formats
- Inkscape Project - Extension API

## Links

### Haiku Vector Icon Format
- **Haiku OS:** https://www.haiku-os.org/
- **Icon-O-Matic Documentation:** https://www.haiku-os.org/docs/userguide/en/applications/icon-o-matic.html
- **HVIF Format Article:** https://blog.leahhanson.us/post/recursecenter2016/haiku_icons.html

### Tools and Libraries
- **Inkscape:** https://inkscape.org/
- **NanoSVG:** https://github.com/memononen/nanosvg
- **STB Libraries:** https://github.com/nothings/stb
- **imagetracerjs:** https://github.com/jankovicsandras/imagetracerjs
- **Anti-Grain Geometry:** https://sourceforge.net/projects/agg/
