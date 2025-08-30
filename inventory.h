#ifndef INVENTORY_H
#define INVENTORY_H

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

/**
 * Fantasy-themed inventory system for Browserwind
 * Supports items, equipment, and RPG mechanics inspired by Morrowind
 */

// Forward declarations
class Player;

/**
 * Types of magical and mundane items found in the realm
 */
enum class ItemType {
    WEAPON,
    ARMOR,
    SHIELD,
    CONSUMABLE,
    SCROLL,
    INGREDIENT,
    TREASURE,
    TOOL,
    QUEST_ITEM,
    MISC
};

/**
 * Equipment slots for worn items
 */
enum class EquipmentSlot {
    MAIN_HAND,      // Primary weapon
    OFF_HAND,       // Shield or secondary weapon
    HEAD,           // Helmet
    CHEST,          // Chestplate/robe
    LEGS,           // Greaves/pants
    FEET,           // Boots
    RING_LEFT,      // Left ring
    RING_RIGHT,     // Right ring
    AMULET,         // Necklace/amulet
    NONE            // Not equippable
};

/**
 * Weapon categories for combat calculations
 */
enum class WeaponType {
    LONGSWORD,
    SHORTSWORD,
    DAGGER,
    MACE,
    AXE,
    BOW,
    STAFF,
    WAND,
    UNARMED
};

/**
 * Armor categories for protection calculations
 */
enum class ArmorType {
    LIGHT,          // Leather, cloth
    MEDIUM,         // Chain mail, scale
    HEAVY,          // Plate, ebony
    ROBE            // Magical robes
};

/**
 * Item rarity affects color display and value
 */
enum class ItemRarity {
    COMMON,         // White
    UNCOMMON,       // Green
    RARE,           // Blue
    EPIC,           // Purple
    LEGENDARY,      // Orange
    ARTIFACT        // Red
};

/**
 * Stats that items can modify
 */
struct ItemStats {
    int damage = 0;         // Weapon damage
    int armor = 0;          // Damage reduction
    int health = 0;         // Health bonus
    int mana = 0;           // Mana bonus
    int stamina = 0;        // Stamina bonus
    int strength = 0;       // Attribute bonus
    int intelligence = 0;   // Attribute bonus
    int agility = 0;        // Attribute bonus
    int luck = 0;           // Attribute bonus
    
    // Resistance bonuses (0-100%)
    int fireResist = 0;
    int coldResist = 0;
    int shockResist = 0;
    int poisonResist = 0;
    
    // Special properties
    bool waterWalking = false;
    bool nightVision = false;
    bool levitation = false;
};

/**
 * Base class for all items in the realm
 * Represents everything from mystical artifacts to common tools
 */
class MysticalItem {
public:
    MysticalItem(const std::string& name, ItemType type, float weight = 1.0f, int value = 1);
    virtual ~MysticalItem() = default;

    // Core properties
    const std::string& getName() const { return name_; }
    const std::string& getDescription() const { return description_; }
    ItemType getType() const { return type_; }
    float getWeight() const { return weight_; }
    int getValue() const { return value_; }
    ItemRarity getRarity() const { return rarity_; }
    const ItemStats& getStats() const { return stats_; }
    
    // Equipment properties
    EquipmentSlot getEquipmentSlot() const { return equipSlot_; }
    bool isEquippable() const { return equipSlot_ != EquipmentSlot::NONE; }
    bool isStackable() const { return stackable_; }
    int getStackSize() const { return stackSize_; }
    int getMaxStack() const { return maxStack_; }
    
    // Condition/durability
    float getDurability() const { return durability_; }
    float getMaxDurability() const { return maxDurability_; }
    bool isBroken() const { return durability_ <= 0.0f; }
    
    // Setters for configuration
    void setDescription(const std::string& desc) { description_ = desc; }
    void setRarity(ItemRarity rarity) { rarity_ = rarity; }
    void setEquipmentSlot(EquipmentSlot slot) { equipSlot_ = slot; }
    void setStackable(bool stackable, int maxStack = 1) { 
        stackable_ = stackable; 
        maxStack_ = maxStack; 
    }
    void setStats(const ItemStats& stats) { stats_ = stats; }
    void setDurability(float current, float maximum = -1.0f);
    
    // Item manipulation
    bool canStackWith(const MysticalItem& other) const;
    void addToStack(int amount) { stackSize_ = std::min(stackSize_ + amount, maxStack_); }
    void removeFromStack(int amount) { stackSize_ = std::max(0, stackSize_ - amount); }
    void takeDamage(float damage);
    void repair(float amount);
    
    // Virtual methods for specialized items
    virtual bool canUse() const { return true; }
    virtual std::string getDisplayName() const;
    virtual std::string getTooltip() const;

protected:
    std::string name_;
    std::string description_;
    ItemType type_;
    ItemRarity rarity_ = ItemRarity::COMMON;
    float weight_;
    int value_;
    
    // Equipment properties
    EquipmentSlot equipSlot_ = EquipmentSlot::NONE;
    ItemStats stats_;
    
    // Stacking
    bool stackable_ = false;
    int stackSize_ = 1;
    int maxStack_ = 1;
    
    // Durability
    float durability_ = 100.0f;
    float maxDurability_ = 100.0f;
};

/**
 * Specialized weapon class with combat properties
 */
class EnchantedWeapon : public MysticalItem {
public:
    EnchantedWeapon(const std::string& name, WeaponType weaponType, int damage, float weight = 3.0f);
    
    WeaponType getWeaponType() const { return weaponType_; }
    float getAttackSpeed() const { return attackSpeed_; }
    float getCritChance() const { return critChance_; }
    
    void setAttackSpeed(float speed) { attackSpeed_ = speed; }
    void setCritChance(float chance) { critChance_ = chance; }
    
    std::string getTooltip() const override;

private:
    WeaponType weaponType_;
    float attackSpeed_ = 1.0f;      // Attacks per second
    float critChance_ = 0.05f;      // 5% base crit chance
};

/**
 * Specialized armor class with protection properties
 */
class GuardianArmor : public MysticalItem {
public:
    GuardianArmor(const std::string& name, ArmorType armorType, int armor, EquipmentSlot slot);
    
    ArmorType getArmorType() const { return armorType_; }
    
    std::string getTooltip() const override;

private:
    ArmorType armorType_;
};

/**
 * Consumable items like potions and food
 */
class AlchemicalPotion : public MysticalItem {
public:
    AlchemicalPotion(const std::string& name, const ItemStats& effects, int duration = 0);
    
    const ItemStats& getEffects() const { return effects_; }
    int getDuration() const { return duration_; }  // Duration in seconds, 0 = instant
    bool isInstant() const { return duration_ == 0; }
    
    bool canUse() const override { return stackSize_ > 0; }
    std::string getTooltip() const override;

private:
    ItemStats effects_;     // Effects when consumed
    int duration_;          // Effect duration in seconds
};

/**
 * Player inventory management system
 * Handles item storage, organization, and weight calculations
 */
class AdventurerInventory {
public:
    AdventurerInventory(float maxWeight = 100.0f, int maxSlots = 50);
    ~AdventurerInventory() = default;

    // Item management
    bool addItem(std::shared_ptr<MysticalItem> item, int quantity = 1);
    bool removeItem(const std::string& itemName, int quantity = 1);
    bool removeItem(std::shared_ptr<MysticalItem> item, int quantity = 1);
    std::shared_ptr<MysticalItem> findItem(const std::string& itemName) const;
    std::vector<std::shared_ptr<MysticalItem>> findItemsByType(ItemType type) const;
    
    // Inventory state
    bool hasItem(const std::string& itemName, int quantity = 1) const;
    int getItemQuantity(const std::string& itemName) const;
    float getCurrentWeight() const;
    float getMaxWeight() const { return maxWeight_; }
    int getUsedSlots() const { return static_cast<int>(items_.size()); }
    int getMaxSlots() const { return maxSlots_; }
    bool isFull() const { return getUsedSlots() >= maxSlots_; }
    bool isOverweight() const { return getCurrentWeight() > maxWeight_; }
    
    // Organization
    void sortByType();
    void sortByValue();
    void sortByWeight();
    void sortByName();
    
    // Access
    const std::vector<std::shared_ptr<MysticalItem>>& getAllItems() const { return items_; }
    std::vector<std::shared_ptr<MysticalItem>> getEquippableItems() const;
    
    // Capacity management
    void setMaxWeight(float weight) { maxWeight_ = weight; }
    void setMaxSlots(int slots) { maxSlots_ = slots; }
    
private:
    std::vector<std::shared_ptr<MysticalItem>> items_;
    float maxWeight_;
    int maxSlots_;
    
    // Helper methods
    bool canAddItem(const std::shared_ptr<MysticalItem>& item, int quantity) const;
    std::shared_ptr<MysticalItem> findStackableItem(const std::shared_ptr<MysticalItem>& item) const;
};

/**
 * Equipment management system for worn items
 * Tracks what the player has equipped and calculates total bonuses
 */
class EquipmentManager {
public:
    EquipmentManager();
    ~EquipmentManager() = default;

    // Equipment management
    bool equipItem(std::shared_ptr<MysticalItem> item);
    std::shared_ptr<MysticalItem> unequipItem(EquipmentSlot slot);
    std::shared_ptr<MysticalItem> getEquippedItem(EquipmentSlot slot) const;
    bool isSlotOccupied(EquipmentSlot slot) const;
    
    // Stats calculation
    ItemStats getTotalStats() const;
    int getTotalArmor() const;
    int getTotalDamage() const;
    float getTotalWeight() const;
    
    // Equipment sets/combos
    bool hasFullArmorSet() const;
    std::vector<std::string> getActiveSetBonuses() const;
    
    // Access
    const std::unordered_map<EquipmentSlot, std::shared_ptr<MysticalItem>>& getAllEquipment() const { 
        return equipment_; 
    }
    
    // Durability management
    void damageEquipment(EquipmentSlot slot, float damage);
    void repairEquipment(EquipmentSlot slot, float amount);
    std::vector<EquipmentSlot> getBrokenEquipment() const;

private:
    std::unordered_map<EquipmentSlot, std::shared_ptr<MysticalItem>> equipment_;
    
    // Helper methods
    bool canEquipItem(const std::shared_ptr<MysticalItem>& item) const;
    std::string getSlotName(EquipmentSlot slot) const;
};

/**
 * Complete inventory system that combines storage and equipment
 * This is the main interface for player item management
 */
class InventorySystem {
public:
    InventorySystem(float maxWeight = 150.0f, int maxSlots = 60);
    ~InventorySystem() = default;

    // Main inventory access
    AdventurerInventory& getInventory() { return inventory_; }
    const AdventurerInventory& getInventory() const { return inventory_; }
    
    // Equipment access
    EquipmentManager& getEquipment() { return equipment_; }
    const EquipmentManager& getEquipment() const { return equipment_; }
    
    // Combined operations
    bool equipItemFromInventory(const std::string& itemName);
    bool unequipItemToInventory(EquipmentSlot slot);
    
    // Item creation helpers (for testing and world generation)
    std::shared_ptr<EnchantedWeapon> createWeapon(const std::string& name, WeaponType type, int damage);
    std::shared_ptr<GuardianArmor> createArmor(const std::string& name, ArmorType type, int armor, EquipmentSlot slot);
    std::shared_ptr<AlchemicalPotion> createPotion(const std::string& name, const ItemStats& effects, int duration = 0);
    std::shared_ptr<MysticalItem> createMiscItem(const std::string& name, ItemType type, float weight = 1.0f, int value = 1);
    
    // Player stats integration
    ItemStats getTotalBonuses() const;
    float getCarryCapacity() const;
    bool isOverencumbered() const;
    
    // Save/load support
    void saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);
    
    // Debug and testing
    void printInventoryStatus() const;
    void addStartingItems();  // Add some basic starting gear

private:
    AdventurerInventory inventory_;
    EquipmentManager equipment_;
};

// Global utility functions for item management
namespace ItemUtils {
    std::string itemTypeToString(ItemType type);
    std::string equipmentSlotToString(EquipmentSlot slot);
    std::string weaponTypeToString(WeaponType type);
    std::string armorTypeToString(ArmorType type);
    std::string rarityToString(ItemRarity rarity);
    
    // Color codes for UI display
    const char* getRarityColor(ItemRarity rarity);
    
    // Item value calculations
    int calculateItemValue(const MysticalItem& item);
    float calculateItemWeight(const MysticalItem& item);
}

#endif // INVENTORY_H
