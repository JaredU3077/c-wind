# Browserwind 3D FPS Game Makefile

# Compiler and flags
CXX = g++
BASE_CXXFLAGS = -std=c++17 -Wall -Wextra -I/opt/homebrew/opt/raylib/include

# Release flags (default)
RELEASE_CXXFLAGS = $(BASE_CXXFLAGS) -O2 -DNDEBUG
# Debug flags
DEBUG_CXXFLAGS = $(BASE_CXXFLAGS) -g -DBROWSERWIND_DEBUG

# Default to release flags
CXXFLAGS = $(RELEASE_CXXFLAGS)

LDFLAGS = -L/opt/homebrew/opt/raylib/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

# For Windows (uncomment if building on Windows)
# LDFLAGS = -lraylib -lopengl32 -lgdi32 -lwinmm

# For Linux (uncomment if building on Linux)
# LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Source files (CLEANED AND OPTIMIZED)
SRC = main.cpp game.cpp menu_system.cpp render_system.cpp environment_manager.cpp npc.cpp player_system.cpp world_builder.cpp game_state.cpp input_manager.cpp config.cpp debug_system.cpp collision_system.cpp environmental_object.cpp dialog_system.cpp combat.cpp render_utils.cpp interaction_system.cpp performance_system.cpp ui_system.cpp inventory.cpp ui_theme_optimized.cpp ui_notification.cpp ui_animation.cpp ui_audio.cpp math_utils.cpp

# Alternative main using Game Engine class (for testing)
SRC_ENGINE = main_new.cpp environment_manager.cpp npc.cpp player_system.cpp world_builder.cpp game_state.cpp input_manager.cpp config.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = Browserwind

# Default target (Release build)
all: $(TARGET)

# Debug build target
debug: CXXFLAGS = $(DEBUG_CXXFLAGS)
debug: $(TARGET)

# Game Engine architecture build (testing new architecture)
engine: CXXFLAGS = $(DEBUG_CXXFLAGS)
engine: SRC = $(SRC_ENGINE)
engine: OBJ = $(SRC:.cpp=.o)
engine: $(TARGET)

# Build target
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJ) $(TARGET) test_runner constants.h.o

# Run the game (Release)
run: all
	./$(TARGET)

# Run the game (Debug)
run-debug: debug
	./$(TARGET)

# Build and run tests
test: test_runner
	./test_runner

# Build test runner
test_runner: tests/main_test_runner.cpp tests/camera_tests.cpp tests/rendering_tests.cpp tests/performance_tests.cpp tests/input_tests.cpp tests/inventory_tests.cpp inventory.o
	$(CXX) $(CXXFLAGS) $^ -o test_runner $(LDFLAGS)

# Run full validation (build game, run tests, clean up)
validate: clean debug test clean-tests
	@echo "🎉 Browserwind validation complete!"

# Clean test files
clean-tests:
	rm -f test_runner

# Run tests with the automated script
validate-auto: run_tests.sh
	./run_tests.sh

# Install dependencies (Linux)
install-deps:
	sudo apt-get update
	sudo apt-get install libraylib-dev build-essential

.PHONY: all clean run test validate clean-tests validate-auto install-deps debug run-debug
