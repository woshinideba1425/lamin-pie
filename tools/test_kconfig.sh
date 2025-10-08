#!/bin/bash
# Test script for Kconfig system

set -e

echo "=== Testing Kconfig System ==="

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAMINPIE_ROOT="$SCRIPT_DIR/.."
TOOLS_DIR="$LAMINPIE_ROOT/tools"

echo "LaminPie root: $LAMINPIE_ROOT"
echo "Tools directory: $TOOLS_DIR"

# Check if Python is available
if ! command -v python3 &> /dev/null; then
    echo "Error: Python3 is not installed"
    exit 1
fi

echo "Python3 found: $(python3 --version)"

# Check if Kconfig files exist
if [ ! -f "$LAMINPIE_ROOT/Kconfig" ]; then
    echo "Error: Main Kconfig file not found: $LAMINPIE_ROOT/Kconfig"
    exit 1
fi

if [ ! -f "$TOOLS_DIR/default_config.txt" ]; then
    echo "Error: Default config file not found: $TOOLS_DIR/default_config.txt"
    exit 1
fi

echo "Kconfig files found"

# Test Kconfig parser
echo "Testing Kconfig parser..."
cd "$TOOLS_DIR"
python3 kconfig_parser.py "$LAMINPIE_ROOT/Kconfig" -v

# Test configuration generator
echo "Testing configuration generator..."
python3 config_generator.py "$LAMINPIE_ROOT/Kconfig" \
    -d "$TOOLS_DIR/default_config.txt" \
    -o "$LAMINPIE_ROOT/test/include/sdkconfig.h" \
    -v

echo "Generated sdkconfig.h"

# Verify generated file
if [ -f "$LAMINPIE_ROOT/test/include/sdkconfig.h" ]; then
    echo "Generated sdkconfig.h exists"
    echo "File size: $(wc -l < "$LAMINPIE_ROOT/test/include/sdkconfig.h") lines"
else
    echo "Error: Generated sdkconfig.h not found"
    exit 1
fi

echo "=== Kconfig System Test Complete ==="
