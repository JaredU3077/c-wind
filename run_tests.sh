#!/bin/bash

# Browserwind Validation Test Runner
# This script builds and runs all validation tests

set -e  # Exit on any error

echo "🧪 Browserwind Validation Test Runner"
echo "====================================="
echo

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "Browserwind.cpp" ]; then
    print_error "Browserwind.cpp not found. Please run this script from the project root directory."
    exit 1
fi

print_status "Building Browserwind game..."
if make clean && make; then
    print_success "Game built successfully"
else
    print_error "Failed to build game"
    exit 1
fi

print_status "Building test suite..."
if g++ -std=c++11 -Wall -Wextra -O2 -I/opt/homebrew/opt/raylib/include \
   tests/main_test_runner.cpp \
   tests/camera_tests.cpp \
   tests/rendering_tests.cpp \
   tests/performance_tests.cpp \
   tests/input_tests.cpp \
   -o test_runner \
   -L/opt/homebrew/opt/raylib/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo; then
    print_success "Test suite built successfully"
else
    print_error "Failed to build test suite"
    exit 1
fi

print_status "Running validation tests..."
echo

if ./test_runner; then
    print_success "All validation tests passed!"
    echo
    print_status "🎮 Game is ready to play!"
    print_status "Run './Browserwind' to start the game"
else
    print_error "Some validation tests failed"
    exit 1
fi

echo
print_status "Cleaning up test artifacts..."
rm -f test_runner
print_success "Cleanup completed"

echo
echo "🎉 Browserwind validation complete!"
echo "=================================="
echo "✅ Game compiles successfully"
echo "✅ All validation tests pass"
echo "✅ Ready for gameplay!"
echo
