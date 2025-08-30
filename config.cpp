#include "config.h"
#include "raylib.h"  // For FileExists
#include <fstream>

GameConfig loadConfig(const std::string& configPath) {
    GameConfig config; // Defaults
    if (FileExists(configPath.c_str())) {
        std::ifstream file(configPath);
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("windowWidth=") == 0) {
                config.windowWidth = std::stoi(line.substr(12));
            } else if (line.find("windowHeight=") == 0) {
                config.windowHeight = std::stoi(line.substr(13));
            } else if (line.find("targetFPS=") == 0) {
                config.targetFPS = std::stoi(line.substr(10));
            } else if (line.find("fullscreen=") == 0) {
                config.fullscreen = (line.substr(11) == "true");
            } else if (line.find("windowTitle=") == 0) {
                config.windowTitle = line.substr(12);
            }
        }
    }
    return config;
}