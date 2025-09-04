// ui_theme_optimized.h - High-performance, scalable theme system for Browserwind
#ifndef UI_THEME_OPTIMIZED_H
#define UI_THEME_OPTIMIZED_H

#include "raylib.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <array>

// ============================================================================
// PERFORMANCE OPTIMIZATIONS
// ============================================================================

#define UI_COLOR_CACHE_SIZE 256
#define UI_FONT_CACHE_SIZE 16
#define UI_THEME_VERSION "1.0.0"

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

namespace UITypes {
    enum class ThemeVariant {
        CLASSIC,        // Original fantasy theme
        DARK,          // Enhanced dark mode
        LIGHT,         // Light mode variant
        ACCESSIBLE,    // High contrast for accessibility
        CUSTOM         // User-defined theme
    };

    enum class ColorRole {
        // Core semantic colors
        PRIMARY,
        SECONDARY,
        ACCENT,
        SUCCESS,
        WARNING,
        ERROR,
        INFO,

        // UI state colors
        BACKGROUND,
        SURFACE,
        TEXT_PRIMARY,
        TEXT_SECONDARY,
        TEXT_DISABLED,
        BORDER,
        HOVER,
        SELECTED,
        PRESSED,

        // Status colors
        HEALTH,
        MANA,
        STAMINA,
        EXPERIENCE,

        // Rarity colors
        COMMON,
        UNCOMMON,
        RARE,
        EPIC,
        LEGENDARY,
        ARTIFACT,

        COLOR_ROLE_COUNT
    };

    enum class FontRole {
        HEADER,
        BODY,
        UI,
        MONOSPACE,
        FONT_ROLE_COUNT
    };

    enum class SpacingRole {
        XS,     // 4px
        SM,     // 8px
        MD,     // 16px
        LG,     // 24px
        XL,     // 32px
        XXL,    // 48px
        SPACING_ROLE_COUNT
    };
}

// ============================================================================
// HIGH-PERFORMANCE THEME SYSTEM
// ============================================================================

namespace UITypes {

struct ThemeMetrics {
    float screenWidth = 800.0f;
    float screenHeight = 600.0f;
    float dpiScale = 1.0f;
    bool highContrast = false;
    bool reducedMotion = false;
    std::string locale = "en";

    // Performance metrics
    mutable uint64_t colorCacheHits = 0;
    mutable uint64_t colorCacheMisses = 0;
    mutable uint64_t fontCacheHits = 0;
    mutable uint64_t fontCacheMisses = 0;
};

struct ColorDefinition {
    std::array<uint8_t, 4> rgba;  // RGBA values
    std::string name;             // Human-readable name
    std::string description;      // Usage description

    Color toRaylibColor() const noexcept {
        return Color{rgba[0], rgba[1], rgba[2], rgba[3]};
    }

    Color withAlpha(float alpha) const noexcept {
        return Color{rgba[0], rgba[1], rgba[2], static_cast<uint8_t>(rgba[3] * alpha)};
    }
};

struct FontDefinition {
    std::string name;
    std::string filePath;
    int baseSize = 16;
    mutable Font font;  // Will be initialized when loaded
    mutable bool loaded = false;

    bool isLoaded() const { return loaded && font.texture.id != 0; }
};

struct SpacingDefinition {
    float value;
    std::string name;
    std::string description;
};

// ============================================================================
// THEME DATA STRUCTURE
// ============================================================================

class ThemeData {
public:
    ThemeData() = default;
    ~ThemeData() = default;

    // Prevent copying, allow moving
    ThemeData(const ThemeData&) = delete;
    ThemeData& operator=(const ThemeData&) = delete;
    ThemeData(ThemeData&&) = default;
    ThemeData& operator=(ThemeData&&) = default;

    // Color management
    void setColor(ColorRole role, ColorDefinition color);
    const ColorDefinition* getColor(ColorRole role) const noexcept;
    Color getRaylibColor(ColorRole role, float alpha = 1.0f) const noexcept;

    // Font management
    void setFont(FontRole role, FontDefinition fontDef);
    const FontDefinition* getFont(FontRole role) const noexcept;
    const Font* getRaylibFont(FontRole role) const noexcept;

    // Spacing management
    void setSpacing(SpacingRole role, SpacingDefinition spacing);
    float getSpacing(SpacingRole role) const noexcept;

    // Theme properties
    void setName(const std::string& name) { name_ = name; }
    void setVersion(const std::string& version) { version_ = version; }
    void setDescription(const std::string& desc) { description_ = desc; }
    void setAuthor(const std::string& author) { author_ = author; }

    const std::string& getName() const { return name_; }
    const std::string& getVersion() const { return version_; }
    const std::string& getDescription() const { return description_; }
    const std::string& getAuthor() const { return author_; }

    // Serialization
    std::string serialize() const;
    bool deserialize(const std::string& data);

    // Validation
    bool validate() const;
    std::vector<std::string> getValidationErrors() const;

private:
    std::string name_;
    std::string version_ = UI_THEME_VERSION;
    std::string description_;
    std::string author_;

    std::array<ColorDefinition, static_cast<size_t>(ColorRole::COLOR_ROLE_COUNT)> colors_;
    std::array<FontDefinition, static_cast<size_t>(FontRole::FONT_ROLE_COUNT)> fonts_;
    std::array<SpacingDefinition, static_cast<size_t>(SpacingRole::SPACING_ROLE_COUNT)> spacing_;
};

// ============================================================================
// HIGH-PERFORMANCE THEME MANAGER
// ============================================================================

class ThemeManager {
public:
    static ThemeManager& getInstance() {
        static ThemeManager instance;
        return instance;
    }

    // Theme management
    bool loadTheme(ThemeVariant variant);
    bool loadCustomTheme(const std::string& themePath);
    bool saveCurrentTheme(const std::string& themePath) const;

    // Color access (highly optimized)
    Color getColor(ColorRole role, float alpha = 1.0f) const noexcept;
    Color getColorWithState(ColorRole baseRole, bool hovered = false,
                           bool selected = false, bool pressed = false) const noexcept;

    // Font access (cached)
    const Font* getFont(FontRole role) const noexcept;

    // Spacing access
    float getSpacing(SpacingRole role) const noexcept;

    // Theme switching
    bool switchTheme(ThemeVariant variant);
    ThemeVariant getCurrentVariant() const { return currentVariant_; }

    // Responsive utilities
    float scaleForDPI(float value) const noexcept;
    Rectangle scaleRectForDPI(const Rectangle& rect) const noexcept;
    Vector2 scaleVecForDPI(const Vector2& vec) const noexcept;

    // Accessibility
    bool isHighContrast() const noexcept { return metrics_.highContrast; }
    void setHighContrast(bool enabled);
    bool prefersReducedMotion() const noexcept { return metrics_.reducedMotion; }
    void setReducedMotion(bool enabled);

    // Performance monitoring
    void getCacheStats(uint64_t& colorHits, uint64_t& colorMisses,
                      uint64_t& fontHits, uint64_t& fontMisses) const;
    void resetCacheStats();

    // Theme modification
    bool modifyColor(ColorRole role, const Color& newColor);
    bool modifyFont(FontRole role, const std::string& fontPath, int baseSize);

    // Debug functions
    uint32_t getCacheKey(ColorRole role, float alpha = 1.0f) const noexcept;

    // Validation
    bool validateCurrentTheme() const;
    std::vector<std::string> getValidationErrors() const;

private:
    ThemeManager();
    ~ThemeManager();

    // Prevent copying
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    // Internal theme storage
    std::unordered_map<ThemeVariant, std::unique_ptr<ThemeData>> themes_;
    ThemeVariant currentVariant_ = ThemeVariant::CLASSIC;
    ThemeMetrics metrics_;

    // Performance caches
    mutable std::array<Color, UI_COLOR_CACHE_SIZE> colorCache_;
    mutable std::array<uint8_t, UI_COLOR_CACHE_SIZE> cacheValid_;
    mutable uint32_t cacheIndex_ = 0;

    // Font cache
    mutable std::array<const Font*, UI_FONT_CACHE_SIZE> fontCache_;
    mutable std::array<uint8_t, UI_FONT_CACHE_SIZE> fontCacheValid_;
    mutable uint32_t fontCacheIndex_ = 0;

    // Internal methods
    void initializeDefaultThemes();
    void createClassicTheme();
    void createDarkTheme();
    void createLightTheme();
    void createAccessibleTheme();

    uint32_t getFontCacheKey(FontRole role) const noexcept;

    void updateColorCache() const;
    void updateFontCache() const;
};

// ============================================================================
// CONVENIENCE FUNCTIONS
// ============================================================================

// Color access
inline Color GetThemeColor(UITypes::ColorRole role, float alpha = 1.0f) {
    return ThemeManager::getInstance().getColor(role, alpha);
}

inline Color GetThemeColorWithState(UITypes::ColorRole baseRole,
                                   bool hovered = false, bool selected = false, bool pressed = false) {
    return ThemeManager::getInstance().getColorWithState(baseRole, hovered, selected, pressed);
}

// Font access
inline const Font* GetThemeFont(UITypes::FontRole role) {
    return ThemeManager::getInstance().getFont(role);
}

// Spacing access
inline float GetThemeSpacing(UITypes::SpacingRole role) {
    return ThemeManager::getInstance().getSpacing(role);
}

// Responsive scaling
inline float ScaleForDPI(float value) {
    return ThemeManager::getInstance().scaleForDPI(value);
}

inline Rectangle ScaleRectForDPI(const Rectangle& rect) {
    return ThemeManager::getInstance().scaleRectForDPI(rect);
}

inline Vector2 ScaleVecForDPI(const Vector2& vec) {
    return ThemeManager::getInstance().scaleVecForDPI(vec);
}

// Debug functions
void DebugPrintThemeColors(UITypes::ThemeVariant variant);

// Utility functions
std::string FormatItemName(const std::string& name, size_t maxLength = 20);
std::string FormatValue(int value);
std::string FormatWeight(float weight);
std::string FormatPercentage(float value);

} // namespace UITypes

#endif // UI_THEME_OPTIMIZED_H
