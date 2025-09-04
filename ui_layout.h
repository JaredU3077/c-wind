// ui_layout.h - Responsive Layout System for Browserwind
#ifndef UI_LAYOUT_H
#define UI_LAYOUT_H

#include "raylib.h"
#include "ui_theme_optimized.h"
#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>

namespace UILayout {

// ============================================================================
// CONSTRAINT-BASED LAYOUT SYSTEM
// ============================================================================

enum class LayoutType {
    ABSOLUTE,       // Fixed positioning
    RELATIVE,       // Relative to parent
    FLEX_VERTICAL,  // Vertical flexbox
    FLEX_HORIZONTAL,// Horizontal flexbox
    GRID,          // CSS Grid-like
    STACK          // Z-index stacking
};

enum class Alignment {
    START,      // Align to start
    CENTER,     // Center alignment
    END,        // Align to end
    STRETCH,    // Stretch to fill
    SPACE_BETWEEN,  // Space between items
    SPACE_AROUND,   // Space around items
    SPACE_EVENLY    // Evenly distributed space
};

enum class SizeConstraint {
    FIXED,          // Fixed size in pixels
    PERCENTAGE,     // Percentage of parent
    AUTO,          // Size to content
    MIN_CONTENT,   // Minimum size based on content
    MAX_CONTENT    // Maximum size based on content
};

struct LayoutConstraint {
    SizeConstraint widthType = SizeConstraint::AUTO;
    SizeConstraint heightType = SizeConstraint::AUTO;
    float widthValue = 0.0f;      // pixels or percentage (0-100)
    float heightValue = 0.0f;     // pixels or percentage (0-100)
    float minWidth = 0.0f;
    float maxWidth = 9999.0f;
    float minHeight = 0.0f;
    float maxHeight = 9999.0f;
    float aspectRatio = 0.0f;     // width/height ratio (0 = no constraint)
};

// ============================================================================
// LAYOUT NODE HIERARCHY
// ============================================================================

class LayoutNode {
public:
    LayoutNode(const std::string& id = "", LayoutType type = LayoutType::RELATIVE);
    virtual ~LayoutNode() = default;

    // Node identification
    std::string getId() const { return id_; }
    void setId(const std::string& id) { id_ = id; }

    // Layout properties
    LayoutType getLayoutType() const { return layoutType_; }
    void setLayoutType(LayoutType type) { layoutType_ = type; }

    // Size and position
    Rectangle getBounds() const { return bounds_; }
    void setBounds(const Rectangle& bounds) { bounds_ = bounds; }
    Vector2 getPosition() const { return {bounds_.x, bounds_.y}; }
    Vector2 getSize() const { return {bounds_.width, bounds_.height}; }

    // Constraints
    LayoutConstraint getConstraints() const { return constraints_; }
    void setConstraints(const LayoutConstraint& constraints) { constraints_ = constraints; }

    // Alignment
    Alignment getHorizontalAlignment() const { return horizontalAlignment_; }
    void setHorizontalAlignment(Alignment align) { horizontalAlignment_ = align; }
    Alignment getVerticalAlignment() const { return verticalAlignment_; }
    void setVerticalAlignment(Alignment align) { verticalAlignment_ = align; }

    // Padding and margins
    float getPadding() const { return padding_; }
    void setPadding(float padding) { padding_ = padding; }
    float getMargin() const { return margin_; }
    void setMargin(float margin) { margin_ = margin; }

    // Hierarchy management
    LayoutNode* getParent() const { return parent_; }
    const std::vector<std::shared_ptr<LayoutNode>>& getChildren() const { return children_; }
    void addChild(std::shared_ptr<LayoutNode> child);
    void removeChild(const std::string& childId);
    std::shared_ptr<LayoutNode> findChild(const std::string& id) const;

    // Layout computation
    virtual void computeLayout(float parentWidth, float parentHeight);
    virtual Vector2 measureContent() const { return {0, 0}; }

    // Event handling
    virtual void onLayoutChanged() {}
    virtual void onSizeChanged() {}
    virtual void onPositionChanged() {}

    // Debug
    virtual void debugRender() const;
    void setDebugColor(Color color) { debugColor_ = color; }

protected:
    std::string id_;
    LayoutType layoutType_;
    Rectangle bounds_ = {0, 0, 0, 0};
    LayoutConstraint constraints_;
    Alignment horizontalAlignment_ = Alignment::START;
    Alignment verticalAlignment_ = Alignment::START;
    float padding_ = 0.0f;
    float margin_ = 0.0f;

    LayoutNode* parent_ = nullptr;
    std::vector<std::shared_ptr<LayoutNode>> children_;

    Color debugColor_ = RED;

    void computeAbsoluteLayout();
    void computeRelativeLayout();
    void computeFlexVerticalLayout();
    void computeFlexHorizontalLayout();
    void computeGridLayout();
};

// ============================================================================
// RESPONSIVE LAYOUT MANAGER
// ============================================================================

class LayoutManager {
public:
    static LayoutManager& getInstance() {
        static LayoutManager instance;
        return instance;
    }

    // Screen management
    void setScreenSize(int width, int height);
    Vector2 getScreenSize() const { return screenSize_; }
    float getDPIScale() const { return dpiScale_; }
    void setDPIScale(float scale) { dpiScale_ = scale; }

    // Root layout management
    void setRootLayout(std::shared_ptr<LayoutNode> root);
    std::shared_ptr<LayoutNode> getRootLayout() const { return rootLayout_; }

    // Node management
    std::shared_ptr<LayoutNode> createNode(const std::string& id,
                                          LayoutType type = LayoutType::RELATIVE);
    std::shared_ptr<LayoutNode> findNode(const std::string& id) const;
    void removeNode(const std::string& id);

    // Layout computation
    void recomputeLayout();
    void updateLayout();

    // Breakpoint system (responsive design)
    enum class Breakpoint {
        MOBILE,     // < 768px
        TABLET,     // 768px - 1024px
        DESKTOP,    // 1024px - 1440px
        WIDESCREEN  // > 1440px
    };

    Breakpoint getCurrentBreakpoint() const;
    void setBreakpoint(Breakpoint breakpoint);

    // Layout utilities
    Rectangle scaleRectForDPI(const Rectangle& rect) const;
    Vector2 scaleVecForDPI(const Vector2& vec) const;
    float scaleValueForDPI(float value) const;

    // Theme integration
    void applyThemeToLayout(std::shared_ptr<LayoutNode> node,
                           UITypes::ColorRole backgroundColor = UITypes::ColorRole::BACKGROUND,
                           UITypes::ColorRole borderColor = UITypes::ColorRole::BORDER);

    // Debug
    void enableDebugRendering(bool enabled) { debugRendering_ = enabled; }
    void renderDebugLayout() const;

private:
    LayoutManager();
    ~LayoutManager();

    // Prevent copying
    LayoutManager(const LayoutManager&) = delete;
    LayoutManager& operator=(const LayoutManager&) = delete;

    Vector2 screenSize_ = {800, 600};
    float dpiScale_ = 1.0f;
    std::shared_ptr<LayoutNode> rootLayout_;
    std::unordered_map<std::string, std::shared_ptr<LayoutNode>> nodeRegistry_;
    Breakpoint currentBreakpoint_ = Breakpoint::DESKTOP;
    bool debugRendering_ = false;

    void updateBreakpoint();
    Breakpoint calculateBreakpoint(float width) const;
};

// ============================================================================
// LAYOUT CONSTRAINTS AND UTILITIES
// ============================================================================

// Constraint builders
LayoutConstraint makeFixedConstraint(float width, float height);
LayoutConstraint makePercentageConstraint(float widthPercent, float heightPercent);
LayoutConstraint makeAutoConstraint();
LayoutConstraint makeMinMaxConstraint(float minWidth, float maxWidth,
                                     float minHeight, float maxHeight);

// Layout utilities
Vector2 calculateFlexSize(const std::vector<std::shared_ptr<LayoutNode>>& children,
                         LayoutType layoutType, float availableSpace);
void distributeSpace(std::vector<std::shared_ptr<LayoutNode>>& children,
                    float totalSpace, Alignment alignment);

// Responsive utilities
bool isMobileLayout();
bool isTabletLayout();
bool isDesktopLayout();
bool isWideScreenLayout();

// ============================================================================
// CONVENIENCE FUNCTIONS
// ============================================================================

// Quick node creation
std::shared_ptr<LayoutNode> createPanel(const std::string& id, float width, float height);
std::shared_ptr<LayoutNode> createButton(const std::string& id, float width, float height);
std::shared_ptr<LayoutNode> createTextBox(const std::string& id, float width);
std::shared_ptr<LayoutNode> createProgressBar(const std::string& id, float width, float height);

// Layout operations
void addChildToLayout(const std::string& parentId, std::shared_ptr<LayoutNode> child);
void removeChildFromLayout(const std::string& parentId, const std::string& childId);
void updateLayoutConstraints(const std::string& nodeId, const LayoutConstraint& constraints);

// Responsive layout helpers
Rectangle getSafeArea();  // Screen area avoiding notches/cutouts
Rectangle getContentArea(); // Usable content area
Vector2 getCenterPosition(const Rectangle& bounds);

} // namespace UILayout

#endif // UI_LAYOUT_H
