#ifndef CONFIG_H
#define CONFIG_H

#include <string>

struct GameConfig {
    int windowWidth = 1920;
    int windowHeight = 1080;
    int targetFPS = 60;
    bool fullscreen = false;
    std::string windowTitle = "Browserwind - 3D FPS Game (1920x1080)";
};

GameConfig loadConfig(const std::string& configPath);

#endif