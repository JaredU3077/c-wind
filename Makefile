# Browserwind 3D FPS Game Makefile

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I/opt/homebrew/opt/raylib/include
LDFLAGS = -L/opt/homebrew/opt/raylib/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

# For Windows (uncomment if building on Windows)
# LDFLAGS = -lraylib -lopengl32 -lgdi32 -lwinmm

# For Linux (uncomment if building on Linux)
# LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Source files (using main.cpp with modular architecture)
SRC = main.cpp collision_system.cpp combat.cpp dialog_system.cpp environment_manager.cpp environmental_object.cpp game_state.cpp interaction_system.cpp inventory.cpp npc.cpp player_system.cpp render_utils.cpp testing_system.cpp world_builder.cpp config.cpp input_manager.cpp performance.cpp ui_system.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = Browserwind

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJ) $(TARGET) test_runner

# Run the game
run: $(TARGET)
	./$(TARGET)

# Build and run tests
test: test_runner
	./test_runner

# Build test runner
test_runner: tests/main_test_runner.cpp tests/camera_tests.cpp tests/rendering_tests.cpp tests/performance_tests.cpp tests/input_tests.cpp tests/inventory_tests.cpp inventory.o
	$(CXX) $(CXXFLAGS) $^ -o test_runner $(LDFLAGS)

# Run full validation (build game, run tests, clean up)
validate: clean all test clean-tests
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

.PHONY: all clean run test validate clean-tests validate-auto install-deps
