// ui_theme_optimized.cpp - High-performance theme system implementation
#include "ui_theme_optimized.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <iostream>

namespace UITypes {

// ============================================================================
// THEME DATA IMPLEMENTATION
// ============================================================================

void ThemeData::setColor(ColorRole role, ColorDefinition color) {
    size_t index = static_cast<size_t>(role);
    if (index < colors_.size()) {
        colors_[index] = color;
    }
}

const ColorDefinition* ThemeData::getColor(ColorRole role) const noexcept {
    size_t index = static_cast<size_t>(role);
    if (index < colors_.size()) {
        return &colors_[index];
    }
    return nullptr;
}

Color ThemeData::getRaylibColor(ColorRole role, float alpha) const noexcept {
    const ColorDefinition* def = getColor(role);
    if (def) {
        return def->withAlpha(alpha);
    }
    // Return a fallback color if role not found
    return Color{255, 255, 255, 255};
}

void ThemeData::setFont(FontRole role, FontDefinition fontDef) {
    size_t index = static_cast<size_t>(role);
    if (index < fonts_.size()) {
        fonts_[index] = fontDef;
    }
}

const FontDefinition* ThemeData::getFont(FontRole role) const noexcept {
    size_t index = static_cast<size_t>(role);
    if (index < fonts_.size()) {
        return &fonts_[index];
    }
    return nullptr;
}

const Font* ThemeData::getRaylibFont(FontRole role) const noexcept {
    const FontDefinition* def = getFont(role);
    if (def && def->isLoaded()) {
        return &def->font;
    }
    return nullptr;
}

void ThemeData::setSpacing(SpacingRole role, SpacingDefinition spacing) {
    size_t index = static_cast<size_t>(role);
    if (index < spacing_.size()) {
        spacing_[index] = spacing;
    }
}

float ThemeData::getSpacing(SpacingRole role) const noexcept {
    size_t index = static_cast<size_t>(role);
    if (index < spacing_.size()) {
        return spacing_[index].value;
    }
    return 8.0f; // Default spacing
}

std::string ThemeData::serialize() const {
    std::ostringstream oss;
    oss << "THEME:" << name_ << "\n";
    oss << "VERSION:" << version_ << "\n";
    oss << "DESCRIPTION:" << description_ << "\n";
    oss << "AUTHOR:" << author_ << "\n";

    // Serialize colors
    for (size_t i = 0; i < colors_.size(); ++i) {
        const auto& color = colors_[i];
        oss << "COLOR:" << i << ":"
            << static_cast<int>(color.rgba[0]) << ","
            << static_cast<int>(color.rgba[1]) << ","
            << static_cast<int>(color.rgba[2]) << ","
            << static_cast<int>(color.rgba[3]) << ":"
            << color.name << ":" << color.description << "\n";
    }

    // Serialize fonts
    for (size_t i = 0; i < fonts_.size(); ++i) {
        const auto& font = fonts_[i];
        oss << "FONT:" << i << ":" << font.name << ":" << font.filePath << ":" << font.baseSize << "\n";
    }

    // Serialize spacing
    for (size_t i = 0; i < spacing_.size(); ++i) {
        const auto& spacing = spacing_[i];
        oss << "SPACING:" << i << ":" << spacing.value << ":" << spacing.name << ":" << spacing.description << "\n";
    }

    return oss.str();
}

bool ThemeData::deserialize(const std::string& data) {
    std::istringstream iss(data);
    std::string line;

    while (std::getline(iss, line)) {
        std::istringstream lineStream(line);
        std::string type;
        std::getline(lineStream, type, ':');

        if (type == "THEME") {
            std::getline(lineStream, name_);
        } else if (type == "VERSION") {
            std::getline(lineStream, version_);
        } else if (type == "DESCRIPTION") {
            std::getline(lineStream, description_);
        } else if (type == "AUTHOR") {
            std::getline(lineStream, author_);
        } else if (type == "COLOR") {
            size_t index;
            lineStream >> index;
            if (index < colors_.size()) {
                std::string rgbaStr, name, desc;
                std::getline(lineStream, rgbaStr, ':');
                std::getline(lineStream, name, ':');
                std::getline(lineStream, desc);

                // Parse RGBA
                std::istringstream rgbaStream(rgbaStr.substr(1)); // Skip first character
                std::string component;
                for (int i = 0; i < 4 && std::getline(rgbaStream, component, ','); ++i) {
                    colors_[index].rgba[i] = static_cast<uint8_t>(std::stoi(component));
                }
                colors_[index].name = name;
                colors_[index].description = desc;
            }
        }
        // Similar parsing for FONT and SPACING...
    }

    return validate();
}

bool ThemeData::validate() const {
    // Validate that all required colors are defined
    for (size_t i = 0; i < colors_.size(); ++i) {
        if (colors_[i].rgba[3] == 0) { // Alpha should not be zero for most colors
            return false;
        }
    }

    // Validate that required fonts are defined
    for (size_t i = 0; i < fonts_.size(); ++i) {
        if (fonts_[i].name.empty()) {
            return false;
        }
    }

    // Validate spacing values
    for (size_t i = 0; i < spacing_.size(); ++i) {
        if (spacing_[i].value <= 0.0f) {
            return false;
        }
    }

    return true;
}

std::vector<std::string> ThemeData::getValidationErrors() const {
    std::vector<std::string> errors;

    for (size_t i = 0; i < colors_.size(); ++i) {
        if (colors_[i].rgba[3] == 0) {
            errors.push_back("Color " + std::to_string(i) + " has zero alpha");
        }
    }

    for (size_t i = 0; i < fonts_.size(); ++i) {
        if (fonts_[i].name.empty()) {
            errors.push_back("Font " + std::to_string(i) + " has no name");
        }
    }

    for (size_t i = 0; i < spacing_.size(); ++i) {
        if (spacing_[i].value <= 0.0f) {
            errors.push_back("Spacing " + std::to_string(i) + " has invalid value: " + std::to_string(spacing_[i].value));
        }
    }

    return errors;
}

// ============================================================================
// THEME MANAGER IMPLEMENTATION
// ============================================================================

ThemeManager::ThemeManager() {
    // Initialize caches
    std::fill(cacheValid_.begin(), cacheValid_.end(), 0);
    std::fill(fontCacheValid_.begin(), fontCacheValid_.end(), 0);

    // Initialize default themes
    initializeDefaultThemes();

    // Load classic theme by default
    loadTheme(ThemeVariant::CLASSIC);
}

ThemeManager::~ThemeManager() {
    // Cleanup will be handled by unique_ptr destructors
}

void ThemeManager::initializeDefaultThemes() {
    createClassicTheme();
    createDarkTheme();
    createLightTheme();
    createAccessibleTheme();
}

void ThemeManager::createClassicTheme() {
    auto classicTheme = std::make_unique<ThemeData>();
    classicTheme->setName("Browserwind Fantasy");
    classicTheme->setVersion("1.1.0");
    classicTheme->setDescription("Gold/brown medieval fantasy theme");
    classicTheme->setAuthor("Browserwind Team");

    // BRIGHT FANTASY THEME - High contrast, clearly visible colors
    // Primary colors - Rich and vibrant
    classicTheme->setColor(ColorRole::PRIMARY, {{100, 150, 200, 255}, "Primary Inventory Blue", "Matching inventory border blue"});
    classicTheme->setColor(ColorRole::SECONDARY, {{200, 180, 160, 255}, "Secondary Bright Bronze", "Ultra bright bronze"});
    classicTheme->setColor(ColorRole::ACCENT, {{255, 220, 160, 255}, "Accent Ultra Gold", "Ultra bright gold accent"});

    classicTheme->setColor(ColorRole::SUCCESS, {{160, 255, 180, 255}, "Success Ultra Green", "Ultra bright forest green"});
    classicTheme->setColor(ColorRole::WARNING, {{255, 240, 140, 255}, "Warning Ultra Amber", "Ultra bright amber"});
    classicTheme->setColor(ColorRole::ERROR, {{255, 160, 160, 255}, "Error Ultra Crimson", "Ultra bright crimson"});
    classicTheme->setColor(ColorRole::INFO, {{180, 220, 255, 255}, "Info Ultra Azure", "Ultra bright azure blue"});

    // UI state colors - MATCHING INVENTORY DARK THEME for consistency
    classicTheme->setColor(ColorRole::BACKGROUND, {{25, 30, 45, 255}, "Background Inventory Dark", "Matching inventory dark background"});
    classicTheme->setColor(ColorRole::SURFACE, {{35, 40, 55, 255}, "Surface Inventory Title", "Matching inventory title bar"});
    classicTheme->setColor(ColorRole::TEXT_PRIMARY, {{220, 230, 255, 255}, "Text Inventory Bright", "Matching inventory bright text"});

    classicTheme->setColor(ColorRole::TEXT_SECONDARY, {{160, 180, 200, 255}, "Text Inventory Muted", "Matching inventory subtitle text"});
    classicTheme->setColor(ColorRole::TEXT_DISABLED, {{120, 130, 150, 255}, "Text Inventory Disabled", "Muted disabled text"});
    classicTheme->setColor(ColorRole::BORDER, {{100, 150, 200, 255}, "Border Inventory Blue", "Matching inventory blue border"});
    classicTheme->setColor(ColorRole::HOVER, {{120, 170, 220, 220}, "Hover Inventory Blue", "Matching inventory blue with transparency"});
    classicTheme->setColor(ColorRole::SELECTED, {{140, 190, 240, 240}, "Selected Inventory Bright", "Bright inventory blue selection"});
    classicTheme->setColor(ColorRole::PRESSED, {{80, 130, 180, 255}, "Pressed Inventory Dark", "Darker inventory blue for pressed state"});

    // Status colors with fantasy flair - Bright and visible
    classicTheme->setColor(ColorRole::HEALTH, {{255, 80, 80, 255}, "Health Bright Red", "Bright red"});
    classicTheme->setColor(ColorRole::MANA, {{80, 120, 255, 255}, "Mana Bright Blue", "Bright blue"});
    classicTheme->setColor(ColorRole::STAMINA, {{80, 200, 80, 255}, "Stamina Bright Green", "Bright green"});
    classicTheme->setColor(ColorRole::EXPERIENCE, {{255, 220, 80, 255}, "Experience Bright Gold", "Bright gold"});

    // Classic theme colors configured successfully

    // Rarity colors for items - High contrast and readable
    classicTheme->setColor(ColorRole::COMMON, {{180, 180, 180, 255}, "Common Light Gray", "Bright light gray"});
    classicTheme->setColor(ColorRole::UNCOMMON, {{120, 220, 120, 255}, "Uncommon Bright Green", "Bright green"});
    classicTheme->setColor(ColorRole::RARE, {{100, 180, 255, 255}, "Rare Bright Blue", "Bright blue"});
    classicTheme->setColor(ColorRole::EPIC, {{220, 120, 255, 255}, "Epic Bright Purple", "Bright purple"});
    classicTheme->setColor(ColorRole::LEGENDARY, {{255, 180, 60, 255}, "Legendary Bright Orange", "Bright orange"});
    classicTheme->setColor(ColorRole::ARTIFACT, {{255, 100, 180, 255}, "Artifact Bright Pink", "Bright pink"});

    // Define fonts with fantasy names
    FontDefinition headerFont{"Medieval Header", "assets/fonts/medieval_header.ttf", 24, Font{}, false};
    FontDefinition bodyFont{"Medieval Body", "assets/fonts/medieval_body.ttf", 16, Font{}, false};
    FontDefinition uiFont{"Medieval UI", "assets/fonts/medieval_ui.ttf", 14, Font{}, false};
    FontDefinition monoFont{"Medieval Mono", "assets/fonts/medieval_mono.ttf", 14, Font{}, false};

    classicTheme->setFont(FontRole::HEADER, headerFont);
    classicTheme->setFont(FontRole::BODY, bodyFont);
    classicTheme->setFont(FontRole::UI, uiFont);
    classicTheme->setFont(FontRole::MONOSPACE, monoFont);

    // Define spacing for consistent layouts
    classicTheme->setSpacing(SpacingRole::XS, {4.0f, "Extra Small", "4px spacing"});
    classicTheme->setSpacing(SpacingRole::SM, {8.0f, "Small", "8px spacing"});
    classicTheme->setSpacing(SpacingRole::MD, {16.0f, "Medium", "16px spacing"});
    classicTheme->setSpacing(SpacingRole::LG, {24.0f, "Large", "24px spacing"});
    classicTheme->setSpacing(SpacingRole::XL, {32.0f, "Extra Large", "32px spacing"});
    classicTheme->setSpacing(SpacingRole::XXL, {48.0f, "Extra Extra Large", "48px spacing"});

    themes_[ThemeVariant::CLASSIC] = std::move(classicTheme);
}

void ThemeManager::createDarkTheme() {
    auto darkTheme = std::make_unique<ThemeData>();
    darkTheme->setName("Browserwind Dark");
    darkTheme->setDescription("Enhanced dark mode for low-light environments");

    // Dark theme colors (darker variants)
    darkTheme->setColor(ColorRole::BACKGROUND, {{5, 5, 10, 255}, "Dark Background", "Very deep space"});
    darkTheme->setColor(ColorRole::SURFACE, {{15, 15, 20, 255}, "Dark Surface", "Deep void"});
    darkTheme->setColor(ColorRole::TEXT_PRIMARY, {{240, 240, 240, 255}, "Bright Text", "High contrast text"});
    darkTheme->setColor(ColorRole::TEXT_SECONDARY, {{200, 200, 200, 255}, "Bright Secondary", "High contrast secondary"});

    // Copy other colors from classic theme but with slight adjustments
    auto classicTheme = themes_[ThemeVariant::CLASSIC].get();
    for (size_t i = 0; i < static_cast<size_t>(ColorRole::COLOR_ROLE_COUNT); ++i) {
        ColorRole role = static_cast<ColorRole>(i);
        if (role != ColorRole::BACKGROUND && role != ColorRole::SURFACE &&
            role != ColorRole::TEXT_PRIMARY && role != ColorRole::TEXT_SECONDARY) {
            const ColorDefinition* colorDef = classicTheme->getColor(role);
            if (colorDef) {
                darkTheme->setColor(role, *colorDef);
            }
        }
    }

    // Copy fonts and spacing
    for (size_t i = 0; i < static_cast<size_t>(FontRole::FONT_ROLE_COUNT); ++i) {
        FontRole role = static_cast<FontRole>(i);
        const FontDefinition* fontDef = classicTheme->getFont(role);
        if (fontDef) {
            darkTheme->setFont(role, *fontDef);
        }
    }

    for (size_t i = 0; i < static_cast<size_t>(SpacingRole::SPACING_ROLE_COUNT); ++i) {
        SpacingRole role = static_cast<SpacingRole>(i);
        SpacingDefinition spacing{classicTheme->getSpacing(role), "", ""};
        darkTheme->setSpacing(role, spacing);
    }

    themes_[ThemeVariant::DARK] = std::move(darkTheme);
}

void ThemeManager::createLightTheme() {
    auto lightTheme = std::make_unique<ThemeData>();
    lightTheme->setName("Browserwind Light");
    lightTheme->setDescription("Light mode variant for bright environments");

    // Light theme colors
    lightTheme->setColor(ColorRole::BACKGROUND, {{245, 245, 250, 255}, "Light Background", "Soft light background"});
    lightTheme->setColor(ColorRole::SURFACE, {{255, 255, 255, 255}, "Light Surface", "Pure white surface"});
    lightTheme->setColor(ColorRole::TEXT_PRIMARY, {{20, 20, 30, 255}, "Dark Text", "High contrast dark text"});
    lightTheme->setColor(ColorRole::TEXT_SECONDARY, {{60, 60, 70, 255}, "Dark Secondary", "Dark secondary text"});
    lightTheme->setColor(ColorRole::BORDER, {{200, 200, 210, 255}, "Light Border", "Subtle light border"});

    // Adjust other colors for light theme
    lightTheme->setColor(ColorRole::PRIMARY, {{70, 70, 90, 255}, "Light Primary", "Soft blue primary"});
    lightTheme->setColor(ColorRole::SECONDARY, {{120, 110, 100, 255}, "Light Secondary", "Light parchment"});

    themes_[ThemeVariant::LIGHT] = std::move(lightTheme);
}

void ThemeManager::createAccessibleTheme() {
    auto accessibleTheme = std::make_unique<ThemeData>();
    accessibleTheme->setName("Browserwind Accessible");
    accessibleTheme->setDescription("High contrast theme for accessibility");

    // High contrast colors meeting WCAG AAA standards
    accessibleTheme->setColor(ColorRole::BACKGROUND, {{0, 0, 0, 255}, "Black Background", "Pure black for maximum contrast"});
    accessibleTheme->setColor(ColorRole::SURFACE, {{32, 32, 32, 255}, "Dark Gray Surface", "Dark gray surface"});
    accessibleTheme->setColor(ColorRole::TEXT_PRIMARY, {{255, 255, 255, 255}, "White Text", "Pure white text"});
    accessibleTheme->setColor(ColorRole::TEXT_SECONDARY, {{200, 200, 200, 255}, "Light Gray Text", "Light gray secondary text"});
    accessibleTheme->setColor(ColorRole::BORDER, {{255, 255, 255, 255}, "White Border", "White borders"});
    accessibleTheme->setColor(ColorRole::ACCENT, {{255, 255, 0, 255}, "Yellow Accent", "High contrast yellow"});
    accessibleTheme->setColor(ColorRole::ERROR, {{255, 0, 0, 255}, "Red Error", "Pure red for errors"});
    accessibleTheme->setColor(ColorRole::SUCCESS, {{0, 255, 0, 255}, "Green Success", "Pure green for success"});

    themes_[ThemeVariant::ACCESSIBLE] = std::move(accessibleTheme);
}

bool ThemeManager::loadTheme(ThemeVariant variant) {
    auto it = themes_.find(variant);
    if (it != themes_.end()) {
        currentVariant_ = variant;
        updateColorCache();
        updateFontCache();
        return true;
    }
    return false;
}

bool ThemeManager::switchTheme(ThemeVariant variant) {
    return loadTheme(variant);
}

Color ThemeManager::getColor(ColorRole role, float alpha) const noexcept {
    uint32_t cacheKey = getCacheKey(role, alpha);
    uint32_t cacheIndex = cacheKey % UI_COLOR_CACHE_SIZE;

    // Check cache first
    if (cacheValid_[cacheIndex]) {
        metrics_.colorCacheHits++;
        return colorCache_[cacheIndex];
    }

    // Cache miss - get from theme
    metrics_.colorCacheMisses++;
    auto it = themes_.find(currentVariant_);
    if (it != themes_.end()) {
        Color color = it->second->getRaylibColor(role, alpha);

        // Color retrieved successfully from theme system

        // Store in cache
        colorCache_[cacheIndex] = color;
        cacheValid_[cacheIndex] = 1;

        return color;
    }

    // Fallback color
    std::cout << "ThemeManager::getColor - FALLBACK for role " << static_cast<int>(role) << std::endl;
    return Color{255, 255, 255, 255};
}

Color ThemeManager::getColorWithState(ColorRole baseRole, bool hovered,
                                     bool selected, bool pressed) const noexcept {
    Color baseColor = getColor(baseRole);

    if (pressed) {
        return getColor(ColorRole::PRESSED, 1.0f);
    } else if (selected) {
        return getColor(ColorRole::SELECTED, 1.0f);
    } else if (hovered) {
        return getColor(ColorRole::HOVER, 1.0f);
    }

    return baseColor;
}

const Font* ThemeManager::getFont(FontRole role) const noexcept {
    uint32_t cacheKey = getFontCacheKey(role);

    // Check cache first
    if (fontCacheValid_[cacheKey % UI_FONT_CACHE_SIZE]) {
        metrics_.fontCacheHits++;
        return fontCache_[cacheKey % UI_FONT_CACHE_SIZE];
    }

    // Cache miss - get from theme
    metrics_.fontCacheMisses++;
    auto it = themes_.find(currentVariant_);
    if (it != themes_.end()) {
        const Font* font = it->second->getRaylibFont(role);

        // Try to load font if not loaded - with comprehensive safety
        if (!font) {
            const FontDefinition* fontDef = it->second->getFont(role);
            if (fontDef && !fontDef->loaded) {
                // **PERFORMANCE FIX**: Disable expensive font loading entirely
                // Skip expensive font loading - return nullptr to use default font
                return nullptr;
            } else if (fontDef && fontDef->loaded) {
                // Font is already loaded - validate it's still valid
                if (fontDef->font.texture.id != 0) {
                    font = &fontDef->font;
                } else {
                    std::cerr << "Warning: Previously loaded font is now invalid for role "
                             << static_cast<int>(role) << ". Using default font." << std::endl;
                    static Font defaultFont = GetFontDefault();
                    font = &defaultFont;
                }
            } else {
                // No font definition found
                std::cerr << "Warning: No font definition found for role "
                         << static_cast<int>(role) << ". Using default font." << std::endl;
                static Font defaultFont = GetFontDefault();
                font = &defaultFont;
            }
        }

        // Store in cache
        if (font) {
            uint32_t cacheIndex = cacheKey % UI_FONT_CACHE_SIZE;
            fontCache_[cacheIndex] = font;
            fontCacheValid_[cacheIndex] = 1;
        }

        return font;
    }

    // No theme found
    std::cerr << "Warning: No theme found for variant " << static_cast<int>(currentVariant_)
             << ". Using default font." << std::endl;
    static Font defaultFont = GetFontDefault();
    return &defaultFont;
}

float ThemeManager::getSpacing(SpacingRole role) const noexcept {
    auto it = themes_.find(currentVariant_);
    if (it != themes_.end()) {
        return it->second->getSpacing(role);
    }
    return 8.0f; // Default spacing
}

uint32_t ThemeManager::getCacheKey(ColorRole role, float alpha) const noexcept {
    // Create a unique key from role and alpha using a better distribution
    uint32_t roleInt = static_cast<uint32_t>(role);
    uint32_t alphaInt = static_cast<uint32_t>(alpha * 1000.0f); // Convert to integer

    // Use a simple hash-like function to distribute keys better
    // This avoids the collision issue with the previous bit-shifting approach
    uint32_t combined = roleInt * 31 + alphaInt * 7;  // Use prime numbers for better distribution
    return combined % UI_COLOR_CACHE_SIZE;
}

uint32_t ThemeManager::getFontCacheKey(FontRole role) const noexcept {
    return static_cast<uint32_t>(role);
}

void ThemeManager::updateColorCache() const {
    std::fill(cacheValid_.begin(), cacheValid_.end(), 0);
}

void ThemeManager::updateFontCache() const {
    std::fill(fontCacheValid_.begin(), fontCacheValid_.end(), 0);
}

// Responsive scaling utilities
float ThemeManager::scaleForDPI(float value) const noexcept {
    return value * metrics_.dpiScale;
}

Rectangle ThemeManager::scaleRectForDPI(const Rectangle& rect) const noexcept {
    return Rectangle{
        rect.x * metrics_.dpiScale,
        rect.y * metrics_.dpiScale,
        rect.width * metrics_.dpiScale,
        rect.height * metrics_.dpiScale
    };
}

Vector2 ThemeManager::scaleVecForDPI(const Vector2& vec) const noexcept {
    return Vector2{vec.x * metrics_.dpiScale, vec.y * metrics_.dpiScale};
}

void ThemeManager::setHighContrast(bool enabled) {
    metrics_.highContrast = enabled;
    if (enabled) {
        switchTheme(ThemeVariant::ACCESSIBLE);
    }
}

void ThemeManager::setReducedMotion(bool enabled) {
    metrics_.reducedMotion = enabled;
}

void ThemeManager::getCacheStats(uint64_t& colorHits, uint64_t& colorMisses,
                                uint64_t& fontHits, uint64_t& fontMisses) const {
    colorHits = metrics_.colorCacheHits;
    colorMisses = metrics_.colorCacheMisses;
    fontHits = metrics_.fontCacheHits;
    fontMisses = metrics_.fontCacheMisses;
}

void ThemeManager::resetCacheStats() {
    metrics_.colorCacheHits = 0;
    metrics_.colorCacheMisses = 0;
    metrics_.fontCacheHits = 0;
    metrics_.fontCacheMisses = 0;
}

// ============================================================================
// DEBUG FUNCTIONS
// ============================================================================

void DebugPrintThemeColors(ThemeVariant variant) {
    ThemeManager& mgr = ThemeManager::getInstance();
    std::cout << "=== THEME COLOR DEBUG ===" << std::endl;
    std::cout << "Theme Variant: " << static_cast<int>(variant) << std::endl;

    // Print all color roles
    for (int i = 0; i < static_cast<int>(ColorRole::COLOR_ROLE_COUNT); ++i) {
        ColorRole role = static_cast<ColorRole>(i);
        uint32_t cacheKey = mgr.getCacheKey(role, 1.0f);
        uint32_t cacheIndex = cacheKey % UI_COLOR_CACHE_SIZE;

        Color color = mgr.getColor(role);
        std::cout << "Role " << i << " CacheKey: " << cacheKey << " CacheIndex: " << cacheIndex
                  << " Color: (" << (int)color.r << "," << (int)color.g << "," << (int)color.b << ")" << std::endl;
    }
    std::cout << "=== END THEME COLOR DEBUG ===" << std::endl;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

std::string FormatItemName(const std::string& name, size_t maxLength) {
    if (name.length() <= maxLength) {
        return name;
    }
    return name.substr(0, maxLength - 3) + "...";
}

std::string FormatValue(int value) {
    return std::to_string(value) + " gp";
}

std::string FormatWeight(float weight) {
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%.1f lbs", weight);
    return buffer;
}

std::string FormatPercentage(float value) {
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%.1f%%", value * 100.0f);
    return buffer;
}

} // namespace UITypes
