# Browserwind - Advanced 3D FPS Game Engine

Browserwind is a comprehensive 3D first-person shooter game engine built with raylib, featuring advanced systems for movement, combat, NPC interaction, building exploration, and real-time debugging. Inspired by classic RPGs like Browserwind, this vertical slice demonstrates professional game development techniques in a single C++ file.

## 🎮 Core Features

### Movement & Camera System
- **First-person camera**: Smooth mouse look with 60° field of view
- **WASD movement**: Responsive keyboard controls with collision detection
- **Physics-based jumping**: Realistic gravity and ground detection
- **Collision system**: Advanced building and NPC collision detection with wall sliding

### Combat System
- **Melee combat**: Longsword swing mechanics with visual blade trails
- **Hit detection**: Range-based target acquisition and scoring
- **Combat feedback**: Visual swing arcs, range indicators, and hit effects
- **Statistics tracking**: Accuracy, swing count, and score management

### World Interaction
- **Building system**: Enter/exit buildings with seamless transitions and improved door detection
- **NPC dialog system**: Interactive conversations with clickable options and enhanced UI
- **Interactive environment**: Town square with well, trees, and decorative elements (all with collision)
- **Context-sensitive prompts**: Dynamic interaction indicators with range visualization
- **Improved interaction feedback**: Visual range rings, better prompt timing, and hover effects

### Developer Tools
- **Real-time testing panel**: Live feature validation and debugging
- **Performance monitoring**: FPS counter and statistics display
- **Collision visualization**: Real-time collision detection feedback
- **Debug overlays**: Mouse position, camera coordinates, and system state

## 🏗️ Architecture Overview

### Core Systems Architecture

```
Browserwind Game Engine
├── Input System (raylib)
│   ├── Keyboard (WASD, Space, E, ESC)
│   ├── Mouse (Look, Click, Capture/Release)
│   └── Button Mapping
├── Rendering System (raylib OpenGL)
│   ├── 3D Camera (First-person perspective)
│   ├── 3D Models (Cubes, Cylinders, Spheres)
│   ├── 2D UI (Text, Rectangles, Buttons)
│   └── Textures & Colors
├── Physics System (Custom)
│   ├── Gravity & Jumping
│   ├── Collision Detection
│   ├── Position Resolution
│   └── Movement Constraints
├── Game Systems (Custom)
│   ├── Combat System
│   ├── Dialog System
│   ├── Building System
│   └── NPC System
└── Debug System (Custom)
    ├── Real-time Testing
    ├── Statistics Tracking
    └── Performance Monitoring
```

### Key Data Structures

#### `LongswordSwing` Struct
```cpp
struct LongswordSwing {
    Vector3 startPosition;    // Swing origin point
    Vector3 endPosition;      // Swing target point
    Vector3 direction;        // Normalized swing direction vector
    bool active;              // Whether swing is currently animating
    float progress;           // Animation progress (0.0 to 1.0)
    float lifetime;           // Remaining animation time
};
```
**Purpose**: Manages melee weapon swing animations and hit detection
**Usage**: Created when player presses left mouse button, tracks swing progress for visual feedback and combat resolution
**Features**: Visual blade trail, range indicators, directional aiming, cooldown management

#### `Building` Struct
```cpp
struct Building {
    Vector3 position;         // Building center position
    Vector3 size;             // Building dimensions (width, height, depth)
    Vector3 entrance;         // Door position for entry/exit
    Color color;              // Building exterior color
    bool canEnter;            // Whether building is accessible
    std::string name;         // Building display name
    std::string description;  // Building description
};
```
**Purpose**: Represents interactive buildings in the game world
**Usage**: Defines tavern, general store, and town hall with unique entrances and properties
**Features**: Door collision detection, interior rendering, entrance markers, building-specific NPCs

#### `NPC` Struct
```cpp
struct NPC {
    Vector3 position;         // NPC standing position
    std::string name;         // NPC display name
    std::string dialog;       // Initial greeting dialog
    Color color;              // NPC body color
    bool canInteract;         // Whether NPC accepts interaction
    float interactionRadius;  // Distance for interaction trigger
};
```
**Purpose**: Defines non-player characters with dialog capabilities
**Usage**: Bartender, shopkeeper, mayor, and guard captain with unique personalities
**Features**: Location-based visibility, interaction radius, dialog branching, mouse-click responses

#### `Target` Struct
```cpp
struct Target {
    Vector3 position;         // Target world position
    bool active;              // Whether target is hittable
    bool hit;                 // Whether target was recently hit
    float hitTime;            // Timestamp of last hit
    Color color;              // Target visual color
};
```
**Purpose**: Combat training targets for melee practice
**Usage**: Colored cubes positioned around the map for accuracy testing
**Features**: Respawn system, hit feedback, scoring integration, visual state management

## 🎯 Gameplay Mechanics

### Movement System
- **Camera Control**: Mouse movement controls yaw/pitch with smooth interpolation
- **Player Movement**: WASD keys move player relative to camera direction
- **Jumping**: Spacebar initiates jump with gravity-based descent
- **Collision**: Cylinder-based player collision with buildings and NPCs
- **Wall Sliding**: Smooth movement along collision surfaces

### Combat System
- **Weapon Mechanics**: Left mouse button triggers longsword swing
- **Swing Animation**: Visual blade trail with progress-based rendering
- **Hit Detection**: Distance and direction-based target acquisition
- **Cooldown System**: Prevents rapid successive attacks
- **Scoring**: Points awarded for successful hits, accuracy tracking

### Interaction System
- **Building Entry**: Press 'E' near building doors to enter/exit
- **NPC Dialog**: Press 'E' near NPCs to initiate conversation
- **Dialog Options**: Click buttons to select conversation choices
- **Mouse Capture**: Automatic cursor management during gameplay vs dialogs

### World Design
- **Town Layout**: Simple town square with well, trees, and two main buildings facing inward
- **Building Types**:
  - **Mayor's Building**: Blue building with brown door, Mayor White NPC (north side)
  - **Shop Keeper's Building**: Red building with brown door, Buster Shoppin NPC (east side)
- **Environmental Objects**: Town well and decorative trees with full collision

## 🛠️ Technical Implementation

### Rendering Pipeline
1. **3D World Rendering**: Camera setup, model drawing, lighting with 1920x1080 resolution
2. **Building System**: Visible brown doors, collision detection, opaque interiors
3. **NPC Rendering**: Detailed humanoid characters with arms, legs, and enhanced visibility
4. **UI Overlay**: 2D interface elements with enhanced mouseover effects
5. **Enhanced Debug Panel**: Comprehensive real-time testing and feature status display
6. **Dialog System**: Modal conversation interface with mouse interaction
7. **Visual Feedback**: Dynamic status indicators and pulsing interaction prompts

### Physics Implementation
- **Gravity System**: Constant downward acceleration with ground clamping
- **Collision Detection**: Capsule-based collision for player, AABB for buildings, cylinder for NPCs
- **Ground Object Collision**: Collision detection for trees, well, and environmental objects
- **Movement Resolution**: Advanced multi-directional wall sliding and position correction
- **Door Detection**: Precise building entry/exit with improved door area detection

### State Management
- **Game State**: Tracks current location (outdoor/indoor), active dialogs
- **Input State**: Manages mouse capture/release during different game modes
- **Combat State**: Tracks active swings, cooldowns, and hit detection
- **Interaction State**: Manages proximity detection and prompt display
- **Testing State**: Real-time feature validation and status tracking

### Visual Enhancements
- **High-Resolution Graphics**: Optimized 1920x1080 display with crisp visuals
- **Visible Door System**: Brown doors with handles, frames, and identification signs
- **Opaque Interior Walls**: Buildings completely block outside view when inside
- **Humanoid NPCs**: Detailed characters with heads, bodies, arms, and legs
- **Enhanced Mouseover Effects**: Pulsing indicators and highlight effects for interactive objects
- **Collision Debug Visualization**: Red wireframe boxes show collision boundaries

### Performance Optimizations
- **High Resolution**: Optimized for 1920x1080 display with 60 FPS target
- **Frame Rate Limiting**: Delta time calculations for consistent gameplay
- **Efficient Rendering**: Minimal draw calls with batched operations
- **Memory Management**: Static arrays for fixed-size game objects
- **Update Loops**: Time-based updates for smooth animation

## 📊 Enhanced Real-Time Testing System

The game features a comprehensive and visually organized testing framework that validates all player functions:

### Organized Test Categories
- **🎮 Movement & Controls**: Mouse capture, WASD movement, jumping, mouse look
- **⚔️ Combat System**: Melee attacks, hit detection, combat accuracy
- **🏛️ World Interaction**: Building entry, NPC dialog, environmental collision
- **🔧 System Status**: Mouse state, current location, dialog activity

### Enhanced Debug Features
- **Visual Status Panel**: Large, organized display with emojis and clear status indicators
- **Real-time Updates**: Dynamic status changes as you test features
- **Comprehensive Coverage**: All player functions are tracked and displayed
- **Color-coded Status**: 🟢 Working | 🟠 Untested | 🔴 Broken
- **System Information**: Mouse state, location, dialog status
- **Performance Metrics**: FPS counter with enhanced statistics display

## 🎮 Controls & Interface

### Primary Controls
- **WASD**: Move forward/backward/left/right
- **Mouse**: Look around (first-person camera)
- **Space**: Jump (physics-based with gravity)
- **Left Mouse Button**: Attack with longsword
- **E**: Interact with buildings/NPCs
- **ESC** (single press): Toggle mouse capture on/off
- **ESC** (double press within 0.5s): Quit game
- **Alt+F4**: Alternative way to close the game

### Visual Interface
- **Crosshair**: Center screen aiming reticle (hidden when mouse released)
- **FPS Counter**: Top-left performance indicator
- **Test Panel**: Bottom-left feature validation display
- **Stats Panel**: Game statistics and scoring
- **Interaction Prompts**: Context-sensitive action hints
- **Dialog Window**: Modal conversation interface
- **Mouse State Indicator**: Shows current mouse capture state
- **Recapture Prompt**: "CLICK TO PLAY" when mouse is released

## 🏃‍♂️ Getting Started

### Prerequisites (macOS Optimized)

**Required Software:**
- macOS 10.12 or later
- Xcode Command Line Tools
- Homebrew package manager

**Dependencies:**
- raylib 4.0+ (graphics and input library)

### Installation & Setup

1. **Install Dependencies:**
   ```bash
   # Install Homebrew (if not already installed)
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

   # Install raylib
   brew install raylib
   ```

2. **Clone and Build:**
```bash
   # Navigate to project directory
   cd "/Users/u/Documents/c wind"

   # Build the game
   make

   # Or build manually for macOS
g++ Browserwind.cpp -o Browserwind -std=c++11 -Wall -Wextra -O2 \
   -I/opt/homebrew/opt/raylib/include \
   -L/opt/homebrew/opt/raylib/lib \
   -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
```

3. **Run the Game:**
   ```bash
   # Execute the game
   ./Browserwind

   # Or use the make target
   make run
   ```

### Testing the Game

The game includes built-in testing features. When you run it:
1. Look for the "REAL-TIME TESTING" panel in the bottom-left
2. Test each feature to see it turn from orange ("WAIT") to green ("PASS")
3. Use the statistics panel to track your performance
4. Watch for collision warnings and interaction prompts

### Troubleshooting

**Common Issues:**
- **"raylib.h not found"**: Ensure raylib is installed via `brew install raylib`
- **Permission denied**: Make executable with `chmod +x Browserwind`
- **Linking errors**: Verify Homebrew paths and raylib installation
- **Mouse not captured**: Check for dialog windows or system interruptions

**Performance Notes:**
- Target: 60 FPS on modern macOS systems
- Window Size: 800x450 pixels (optimized for development)
- Memory Usage: Minimal (< 50MB) due to single-file architecture

## 🏗️ Project Structure

```
Browserwind/
├── Browserwind.cpp          # Main game source (1122 lines)
├── Browserwind.o            # Object file (compiled)
├── Browserwind              # Executable binary
├── Makefile                 # Build configuration
├── README.md                # This documentation
├── run_tests.sh            # Test automation script
└── tests/                   # Unit test framework
    ├── camera_tests.cpp
    ├── input_tests.cpp
    ├── main_test_runner.cpp
    ├── performance_tests.cpp
    └── rendering_tests.cpp
```

### File Descriptions

- **`Browserwind.cpp`**: Complete game implementation in a single file
  - Architecture: Modular function-based design
  - Systems: Rendering, physics, input, AI, UI
  - Features: 8 major gameplay systems implemented
  - Code Style: Professional C++11 with comprehensive comments

- **`Makefile`**: Cross-platform build configuration
  - Targets: `all`, `clean`, `run`, `test`, `validate`
  - Platforms: macOS (primary), Linux, Windows support
  - Optimization: O2 level with debugging symbols

- **`run_tests.sh`**: Automated testing and validation
  - Integration: Compiles and runs all test suites
  - Validation: Feature completeness verification
  - Reporting: Detailed test results and coverage

## 🎯 Development Philosophy

### Single-File Architecture
- **Advantages**: Easy deployment, clear dependencies, rapid prototyping
- **Implementation**: All systems integrated in one cohesive codebase
- **Maintenance**: Comprehensive commenting and modular function organization

### Real-Time Testing
- **Quality Assurance**: Immediate feedback on feature implementation
- **Developer Experience**: Visual validation without external tools
- **Debugging**: Integrated performance monitoring and state inspection

### Educational Value
- **Learning Resource**: Complete game systems in understandable code
- **Best Practices**: Professional C++ patterns and raylib usage
- **Extensibility**: Modular design allows easy feature addition

## 🚀 Advanced Features

### Physics Simulation
- **Gravity**: Realistic falling with terminal velocity consideration
- **Collision**: Multi-object collision detection with resolution
- **Movement**: Smooth interpolation and frame-rate independent updates

### AI & Interaction
- **NPC Behavior**: Location-based visibility and interaction
- **Dialog System**: Branching conversations with state management
- **Context Awareness**: Proximity-based interaction triggers

### Visual Effects
- **Particle Systems**: Sword swing trails and hit effects
- **Dynamic Lighting**: Color-coded feedback systems
- **UI Animation**: Smooth transitions and hover effects

## 📈 Performance Metrics

**Target Specifications:**
- **Platform**: macOS 10.12+
- **Resolution**: 800x450 windowed
- **Frame Rate**: 60 FPS sustained
- **Memory**: < 50MB RAM usage
- **CPU**: Minimal load on modern systems

**Optimization Features:**
- **Efficient Rendering**: Minimal draw calls, batched operations
- **Memory Management**: Static allocation, no dynamic memory
- **Update Loops**: Time-based calculations for consistency
- **Resource Loading**: Compile-time asset integration

## 🔮 Future Enhancements

### Potential Additions
- **Multiplayer**: Networked gameplay support
- **Save System**: Persistent game state management
- **Inventory**: Item management and equipment
- **Quests**: Story-driven gameplay mechanics
- **Audio**: Sound effects and background music
- **Advanced Graphics**: Shaders, textures, and lighting

### Architecture Improvements
- **Modular Design**: Separate files for different systems
- **Asset Management**: External resource loading system
- **Configuration**: Settings and preferences management
- **Localization**: Multi-language support

## 📜 License & Usage

This project serves as a comprehensive example of modern C++ game development using raylib. The code is structured for educational purposes and can be used as a foundation for more complex game projects.

**Educational Value:**
- Complete game architecture demonstration
- Professional coding practices
- Real-time testing methodologies
- Cross-platform development techniques

**Commercial Use:**
- Modify and extend for your own projects
- Use as a reference for game development
- Adapt systems for different game genres

## 🚀 Recent Improvements (v1.1)

### Enhanced Collision System
- **Capsule-based Player Collision**: More accurate player collision using capsule shapes instead of simple spheres
- **Multi-directional Wall Sliding**: Improved wall sliding that works in both X and Z directions simultaneously for smoother movement
- **Ground Object Collision**: Added collision detection for trees, well, and other environmental objects
- **Better Door Detection**: Improved building entry/exit with more precise door area detection and smoother transitions

### Improved Interaction System
- **Enhanced Dialog UI**: Better button detection with generous hit areas, hover effects, and visual feedback
- **Visual Range Indicators**: Subtle rings around NPCs and building doors to show interaction ranges
- **Improved Prompt System**: Better timing for interaction prompts with animated backgrounds and clearer visual feedback
- **Dialog Mode Indicators**: Visual indicators when in dialog mode, including crosshair changes and mode display
- **Better Mouse Management**: Improved cursor capture/release handling during different game states

### Technical Enhancements
- **Performance Optimizations**: Reduced player radius for smoother movement while maintaining collision accuracy
- **Visual Feedback**: Added pulsing effects for interaction prompts and hover states for buttons
- **Code Organization**: Better separation of collision detection functions and improved maintainability
- **Error Handling**: More robust collision resolution with fallback mechanisms

### Gameplay Improvements
- **Smoother Movement**: Reduced collision buffer and improved wall sliding for more responsive controls
- **Better Building Navigation**: More precise door areas and smoother transitions between indoor/outdoor areas
- **Enhanced NPC Interaction**: Visual range indicators and improved dialog button responsiveness
- **Environmental Interaction**: Collision with trees and well prevents getting stuck on environmental objects
- **Improved ESC Handling**: Press ESC once to release mouse cursor, press ESC again to close the application

---

**Browserwind v1.1** - An enhanced 3D FPS game engine with improved collision detection, interaction systems, and visual feedback, built with raylib.
