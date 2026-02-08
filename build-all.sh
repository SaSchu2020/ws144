#!/bin/bash
#
# Build script for cross-compiling libgpio for multiple architectures
# Usage: ./build-all.sh [clean]
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LIBGPIO_DIR="$SCRIPT_DIR/lgpio"

# Check if libgpio source directory exists
if [ ! -d "$LIBGPIO_DIR" ]; then
    echo "Error: libgpio source directory not found at $LIBGPIO_DIR"
    echo "Please ensure libgpio source is in the 'libgpio' subdirectory"
    exit 1
fi

cd "$LIBGPIO_DIR"

# Check if cross-compilation toolchains are installed
check_toolchain() {
    local arch=$1
    local compiler=$2
    
    if ! command -v "$compiler" &> /dev/null; then
        echo "Warning: Cross-compiler '$compiler' not found for $arch"
        echo "Install with: sudo apt-get install gcc-$compiler"
        return 1
    fi
    return 0
}

# Clean if requested
if [ "$1" == "clean" ]; then
    echo "Cleaning all builds..."
    make -f Makefile.cross clean-all
    exit 0
fi

echo "======================================"
echo "Building libgpio for all architectures"
echo "======================================"
echo ""

# Build for x86_64 (native)
echo "Building for x86_64..."
make -f Makefile.cross clean
make -f Makefile.cross ARCH=x86_64 -j$(nproc)
make -f Makefile.cross ARCH=x86_64 install
echo "✓ x86_64 build complete"
echo ""

# Build for ARM (32-bit) - Raspberry Pi Zero, Pi 1, etc.
if check_toolchain "ARM 32-bit" "arm-linux-gnueabihf-gcc"; then
    echo "Building for armv7l (32-bit ARM)..."
    make -f Makefile.cross clean
    make -f Makefile.cross ARCH=armv7l -j$(nproc)
    make -f Makefile.cross ARCH=armv7l install
    echo "✓ armv7l build complete"
    echo ""
else
    echo "⚠ Skipping armv7l build (toolchain not installed)"
    echo ""
fi

# Build for ARM64 (64-bit) - Raspberry Pi 3, 4, 5, etc.
if check_toolchain "ARM 64-bit" "aarch64-linux-gnu-gcc"; then
    echo "Building for aarch64 (64-bit ARM)..."
    make -f Makefile.cross clean
    make -f Makefile.cross ARCH=aarch64 -j$(nproc)
    make -f Makefile.cross ARCH=aarch64 install
    echo "✓ aarch64 build complete"
    echo ""
else
    echo "⚠ Skipping aarch64 build (toolchain not installed)"
    echo ""
fi

# Clean build artifacts
make -f Makefile.cross clean

echo "======================================"
echo "Build Summary"
echo "======================================"
echo "Libraries installed in:"
echo "  - dist/x64/lib"
echo "  - dist/arm/lib"
echo "  - dist/arm64/lib"
echo ""
echo "Headers installed in:"
echo "  - dist/x64/include"
echo "  - dist/arm/include"
echo "  - dist/arm64/include"
echo ""
echo "Done!"

