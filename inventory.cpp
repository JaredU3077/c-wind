#include "inventory.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

// ============================================================================
// MysticalItem Implementation
// ============================================================================

MysticalItem::MysticalItem(const std::string& name, ItemType type, float weight, int value)
    : name_(name), type_(type), weight_(weight), value_(value) {
    // Set default description based on type
    switch (type_) {
        case ItemType::WEAPON:
            description_ = "A weapon forged in the ancient smithies";
            break;
        case ItemType::ARMOR:
            description_ = "Protective gear crafted by skilled armorers";
            break;
        case ItemType::CONSUMABLE:
            description_ = "A consumable item with mystical properties";
            break;
        case ItemType::TREASURE:
            description_ = "A valuable treasure from forgotten ruins";
            break;
        case ItemType::QUEST_ITEM:
            description_ = "An important item for your quest";
            break;
        default:
            description_ = "A mysterious item of unknown origin";
            break;
    }
}

void MysticalItem::setDurability(float current, float maximum) {
    durability_ = current;
    if (maximum >= 0.0f) {
        maxDurability_ = maximum;
    }
    // Ensure durability doesn't exceed maximum
    durability_ = std::min(durability_, maxDurability_);
}

bool MysticalItem::canStackWith(const MysticalItem& other) const {
    return stackable_ && 
           other.stackable_ && 
           name_ == other.name_ && 
           type_ == other.type_ &&
           stackSize_ < maxStack_;
}

void MysticalItem::takeDamage(float damage) {
    durability_ = std::max(0.0f, durability_ - damage);
}

void MysticalItem::repair(float amount) {
    durability_ = std::min(maxDurability_, durability_ + amount);
}

std::string MysticalItem::getDisplayName() const {
    std::string displayName = name_;
    
    // Add stack size if stackable
    if (stackable_ && stackSize_ > 1) {
        displayName += " (" + std::to_string(stackSize_) + ")";
    }
    
    // Add condition indicator
    if (maxDurability_ > 0.0f) {
        float condition = durability_ / maxDurability_;
        if (condition <= 0.25f) {
            displayName += " [Broken]";
        } else if (condition <= 0.5f) {
            displayName += " [Poor]";
        } else if (condition <= 0.75f) {
            displayName += " [Fair]";
        }
        // Good/Excellent items don't show condition
    }
    
    return displayName;
}

std::string MysticalItem::getTooltip() const {
    std::stringstream tooltip;
    tooltip << name_ << "\n";
    tooltip << description_ << "\n\n";
    
    // Basic properties
    tooltip << "Type: " << ItemUtils::itemTypeToString(type_) << "\n";
    tooltip << "Weight: " << weight_ << " kg\n";
    tooltip << "Value: " << value_ << " gold\n";
    
    if (rarity_ != ItemRarity::COMMON) {
        tooltip << "Rarity: " << ItemUtils::rarityToString(rarity_) << "\n";
    }
    
    // Durability
    if (maxDurability_ > 0.0f) {
        tooltip << "Condition: " << static_cast<int>((durability_ / maxDurability_) * 100) << "%\n";
    }
    
    // Stats
    const auto& stats = getStats();
    if (stats.damage > 0) tooltip << "Damage: +" << stats.damage << "\n";
    if (stats.armor > 0) tooltip << "Armor: +" << stats.armor << "\n";
    if (stats.health > 0) tooltip << "Health: +" << stats.health << "\n";
    if (stats.mana > 0) tooltip << "Mana: +" << stats.mana << "\n";
    if (stats.stamina > 0) tooltip << "Stamina: +" << stats.stamina << "\n";
    
    return tooltip.str();
}

// ============================================================================
// EnchantedWeapon Implementation
// ============================================================================

EnchantedWeapon::EnchantedWeapon(const std::string& name, WeaponType weaponType, int damage, float weight)
    : MysticalItem(name, ItemType::WEAPON, weight, damage * 10), weaponType_(weaponType) {
    
    equipSlot_ = EquipmentSlot::MAIN_HAND;
    stats_.damage = damage;
    
    // Set weapon-specific properties
    switch (weaponType_) {
        case WeaponType::DAGGER:
            attackSpeed_ = 2.0f;
            critChance_ = 0.15f;
            description_ = "A swift blade perfect for quick strikes";
            break;
        case WeaponType::LONGSWORD:
            attackSpeed_ = 1.0f;
            critChance_ = 0.08f;
            description_ = "A noble blade favored by knights and warriors";
            break;
        case WeaponType::MACE:
            attackSpeed_ = 0.8f;
            critChance_ = 0.05f;
            description_ = "A heavy weapon that crushes armor and bone";
            break;
        case WeaponType::BOW:
            attackSpeed_ = 1.2f;
            critChance_ = 0.12f;
            equipSlot_ = EquipmentSlot::MAIN_HAND;
            description_ = "A ranged weapon for hunters and scouts";
            break;
        case WeaponType::STAFF:
            attackSpeed_ = 0.6f;
            critChance_ = 0.03f;
            description_ = "A conduit for magical energies";
            break;
        default:
            attackSpeed_ = 1.0f;
            critChance_ = 0.05f;
            break;
    }
    
    setDurability(100.0f, 100.0f);
}

std::string EnchantedWeapon::getTooltip() const {
    std::stringstream tooltip;
    tooltip << MysticalItem::getTooltip();
    tooltip << "\nWeapon Type: " << ItemUtils::weaponTypeToString(weaponType_) << "\n";
    tooltip << "Attack Speed: " << attackSpeed_ << "/sec\n";
    tooltip << "Critical Chance: " << static_cast<int>(critChance_ * 100) << "%\n";
    return tooltip.str();
}

// ============================================================================
// GuardianArmor Implementation
// ============================================================================

GuardianArmor::GuardianArmor(const std::string& name, ArmorType armorType, int armor, EquipmentSlot slot)
    : MysticalItem(name, ItemType::ARMOR, 0.0f, armor * 15), armorType_(armorType) {
    
    equipSlot_ = slot;
    stats_.armor = armor;
    
    // Set armor-specific properties
    switch (armorType_) {
        case ArmorType::LIGHT:
            weight_ = 2.0f;
            description_ = "Light armor that provides mobility and stealth";
            break;
        case ArmorType::MEDIUM:
            weight_ = 8.0f;
            description_ = "Balanced protection for versatile adventurers";
            break;
        case ArmorType::HEAVY:
            weight_ = 15.0f;
            description_ = "Heavy armor providing maximum protection";
            break;
        case ArmorType::ROBE:
            weight_ = 1.0f;
            description_ = "Magical robes woven with arcane threads";
            stats_.mana += armor; // Robes also provide mana bonus
            break;
    }
    
    setDurability(100.0f, 100.0f);
}

std::string GuardianArmor::getTooltip() const {
    std::stringstream tooltip;
    tooltip << MysticalItem::getTooltip();
    tooltip << "\nArmor Type: " << ItemUtils::armorTypeToString(armorType_) << "\n";
    tooltip << "Equipment Slot: " << ItemUtils::equipmentSlotToString(equipSlot_) << "\n";
    return tooltip.str();
}

// ============================================================================
// AlchemicalPotion Implementation
// ============================================================================

AlchemicalPotion::AlchemicalPotion(const std::string& name, const ItemStats& effects, int duration)
    : MysticalItem(name, ItemType::CONSUMABLE, 0.5f, 50), effects_(effects), duration_(duration) {
    
    setStackable(true, 10);  // Potions stack up to 10
    
    if (duration_ > 0) {
        description_ = "A magical elixir with temporary effects";
    } else {
        description_ = "A healing potion with instant effects";
    }
}

std::string AlchemicalPotion::getTooltip() const {
    std::stringstream tooltip;
    tooltip << MysticalItem::getTooltip();
    
    tooltip << "\nEffects:\n";
    if (effects_.health > 0) tooltip << "  Restores " << effects_.health << " health\n";
    if (effects_.mana > 0) tooltip << "  Restores " << effects_.mana << " mana\n";
    if (effects_.stamina > 0) tooltip << "  Restores " << effects_.stamina << " stamina\n";
    
    if (duration_ > 0) {
        tooltip << "\nDuration: " << duration_ << " seconds\n";
    } else {
        tooltip << "\nEffect: Instant\n";
    }
    
    return tooltip.str();
}

// ============================================================================
// AdventurerInventory Implementation
// ============================================================================

AdventurerInventory::AdventurerInventory(float maxWeight, int maxSlots)
    : maxWeight_(maxWeight), maxSlots_(maxSlots) {
    items_.reserve(maxSlots_);
}

bool AdventurerInventory::addItem(std::shared_ptr<MysticalItem> item, int quantity) {
    if (!item || quantity <= 0) {
        return false;
    }
    
    // Check if item can be stacked with existing items
    if (item->isStackable()) {
        auto existingItem = findStackableItem(item);
        if (existingItem) {
            int canAdd = std::min(quantity, existingItem->getMaxStack() - existingItem->getStackSize());
            if (canAdd > 0) {
                existingItem->addToStack(canAdd);
                quantity -= canAdd;
            }
        }
    }
    
    // Add remaining quantity as new items
    while (quantity > 0 && canAddItem(item, 1)) {
        auto newItem = std::shared_ptr<MysticalItem>(new MysticalItem(*item));
        if (item->isStackable()) {
            int stackAmount = std::min(quantity, item->getMaxStack());
            newItem->addToStack(stackAmount - 1); // -1 because items start with size 1
            quantity -= stackAmount;
        } else {
            quantity--;
        }
        items_.push_back(newItem);
    }
    
    return quantity == 0; // True if all items were added
}

bool AdventurerInventory::removeItem(const std::string& itemName, int quantity) {
    auto item = findItem(itemName);
    return item ? removeItem(item, quantity) : false;
}

bool AdventurerInventory::removeItem(std::shared_ptr<MysticalItem> item, int quantity) {
    if (!item || quantity <= 0) {
        return false;
    }
    
    auto it = std::find(items_.begin(), items_.end(), item);
    if (it == items_.end()) {
        return false;
    }
    
    if (item->isStackable()) {
        if (item->getStackSize() >= quantity) {
            item->removeFromStack(quantity);
            if (item->getStackSize() <= 0) {
                items_.erase(it);
            }
            return true;
        }
    } else if (quantity == 1) {
        items_.erase(it);
        return true;
    }
    
    return false;
}

std::shared_ptr<MysticalItem> AdventurerInventory::findItem(const std::string& itemName) const {
    auto it = std::find_if(items_.begin(), items_.end(),
        [&itemName](const std::shared_ptr<MysticalItem>& item) {
            return item->getName() == itemName;
        });
    return (it != items_.end()) ? *it : nullptr;
}

std::vector<std::shared_ptr<MysticalItem>> AdventurerInventory::findItemsByType(ItemType type) const {
    std::vector<std::shared_ptr<MysticalItem>> result;
    std::copy_if(items_.begin(), items_.end(), std::back_inserter(result),
        [type](const std::shared_ptr<MysticalItem>& item) {
            return item->getType() == type;
        });
    return result;
}

bool AdventurerInventory::hasItem(const std::string& itemName, int quantity) const {
    return getItemQuantity(itemName) >= quantity;
}

int AdventurerInventory::getItemQuantity(const std::string& itemName) const {
    int total = 0;
    for (const auto& item : items_) {
        if (item->getName() == itemName) {
            total += item->getStackSize();
        }
    }
    return total;
}

float AdventurerInventory::getCurrentWeight() const {
    float totalWeight = 0.0f;
    for (const auto& item : items_) {
        totalWeight += item->getWeight() * item->getStackSize();
    }
    return totalWeight;
}

void AdventurerInventory::sortByType() {
    std::sort(items_.begin(), items_.end(),
        [](const std::shared_ptr<MysticalItem>& a, const std::shared_ptr<MysticalItem>& b) {
            return static_cast<int>(a->getType()) < static_cast<int>(b->getType());
        });
}

void AdventurerInventory::sortByValue() {
    std::sort(items_.begin(), items_.end(),
        [](const std::shared_ptr<MysticalItem>& a, const std::shared_ptr<MysticalItem>& b) {
            return a->getValue() > b->getValue();
        });
}

void AdventurerInventory::sortByWeight() {
    std::sort(items_.begin(), items_.end(),
        [](const std::shared_ptr<MysticalItem>& a, const std::shared_ptr<MysticalItem>& b) {
            return a->getWeight() < b->getWeight();
        });
}

void AdventurerInventory::sortByName() {
    std::sort(items_.begin(), items_.end(),
        [](const std::shared_ptr<MysticalItem>& a, const std::shared_ptr<MysticalItem>& b) {
            return a->getName() < b->getName();
        });
}

void AdventurerInventory::sortByRarity() {
    std::sort(items_.begin(), items_.end(),
        [](const std::shared_ptr<MysticalItem>& a, const std::shared_ptr<MysticalItem>& b) {
            return static_cast<int>(a->getRarity()) > static_cast<int>(b->getRarity());
        });
}

// ============================================================================
// Search and Filtering Implementation
// ============================================================================

std::vector<std::shared_ptr<MysticalItem>> AdventurerInventory::searchItems(const std::string& query) const {
    std::vector<std::shared_ptr<MysticalItem>> results;

    if (query.empty()) {
        return items_;  // Return all items if no query
    }

    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

    std::copy_if(items_.begin(), items_.end(), std::back_inserter(results),
        [&lowerQuery](const std::shared_ptr<MysticalItem>& item) {
            std::string itemName = item->getName();
            std::string itemDesc = item->getDescription();

            // Convert to lowercase for case-insensitive search
            std::transform(itemName.begin(), itemName.end(), itemName.begin(), ::tolower);
            std::transform(itemDesc.begin(), itemDesc.end(), itemDesc.begin(), ::tolower);

            // Search in name and description
            return itemName.find(lowerQuery) != std::string::npos ||
                   itemDesc.find(lowerQuery) != std::string::npos ||
                   ItemUtils::itemTypeToString(item->getType()).find(lowerQuery) != std::string::npos ||
                   ItemUtils::rarityToString(item->getRarity()).find(lowerQuery) != std::string::npos;
        });

    return results;
}

std::vector<std::shared_ptr<MysticalItem>> AdventurerInventory::filterByType(ItemType type) const {
    std::vector<std::shared_ptr<MysticalItem>> results;
    std::copy_if(items_.begin(), items_.end(), std::back_inserter(results),
        [type](const std::shared_ptr<MysticalItem>& item) {
            return item->getType() == type;
        });
    return results;
}

std::vector<std::shared_ptr<MysticalItem>> AdventurerInventory::filterByRarity(ItemRarity minRarity) const {
    std::vector<std::shared_ptr<MysticalItem>> results;
    std::copy_if(items_.begin(), items_.end(), std::back_inserter(results),
        [minRarity](const std::shared_ptr<MysticalItem>& item) {
            return static_cast<int>(item->getRarity()) >= static_cast<int>(minRarity);
        });
    return results;
}

std::vector<std::shared_ptr<MysticalItem>> AdventurerInventory::getEquippableItems() const {
    std::vector<std::shared_ptr<MysticalItem>> result;
    std::copy_if(items_.begin(), items_.end(), std::back_inserter(result),
        [](const std::shared_ptr<MysticalItem>& item) {
            return item->isEquippable();
        });
    return result;
}

bool AdventurerInventory::canAddItem(const std::shared_ptr<MysticalItem>& item, int quantity) const {
    // Check slot availability
    if (isFull() && !item->isStackable()) {
        return false;
    }
    
    // Check weight limit
    float additionalWeight = item->getWeight() * quantity;
    if (getCurrentWeight() + additionalWeight > maxWeight_) {
        return false;
    }
    
    return true;
}

std::shared_ptr<MysticalItem> AdventurerInventory::findStackableItem(const std::shared_ptr<MysticalItem>& item) const {
    auto it = std::find_if(items_.begin(), items_.end(),
        [&item](const std::shared_ptr<MysticalItem>& existing) {
            return existing->canStackWith(*item);
        });
    return (it != items_.end()) ? *it : nullptr;
}

// ============================================================================
// EquipmentManager Implementation
// ============================================================================

EquipmentManager::EquipmentManager() {
    // Initialize all equipment slots as empty
    for (int i = 0; i <= static_cast<int>(EquipmentSlot::AMULET); ++i) {
        equipment_[static_cast<EquipmentSlot>(i)] = nullptr;
    }
}

bool EquipmentManager::equipItem(std::shared_ptr<MysticalItem> item) {
    if (!item || !canEquipItem(item)) {
        return false;
    }
    
    EquipmentSlot slot = item->getEquipmentSlot();
    
    // Unequip existing item if slot is occupied
    if (isSlotOccupied(slot)) {
        std::cout << "Unequipping " << equipment_[slot]->getName() 
                  << " to make room for " << item->getName() << std::endl;
    }
    
    equipment_[slot] = item;
    std::cout << "Equipped " << item->getName() << " in " << getSlotName(slot) << std::endl;
    
    return true;
}

std::shared_ptr<MysticalItem> EquipmentManager::unequipItem(EquipmentSlot slot) {
    auto item = equipment_[slot];
    if (item) {
        equipment_[slot] = nullptr;
        std::cout << "Unequipped " << item->getName() << std::endl;
    }
    return item;
}

std::shared_ptr<MysticalItem> EquipmentManager::getEquippedItem(EquipmentSlot slot) const {
    auto it = equipment_.find(slot);
    return (it != equipment_.end()) ? it->second : nullptr;
}

bool EquipmentManager::isSlotOccupied(EquipmentSlot slot) const {
    auto item = getEquippedItem(slot);
    return item != nullptr;
}

ItemStats EquipmentManager::getTotalStats() const {
    ItemStats total;
    
    for (const auto& [slot, item] : equipment_) {
        if (item && !item->isBroken()) {
            const auto& stats = item->getStats();
            total.damage += stats.damage;
            total.armor += stats.armor;
            total.health += stats.health;
            total.mana += stats.mana;
            total.stamina += stats.stamina;
            total.strength += stats.strength;
            total.intelligence += stats.intelligence;
            total.agility += stats.agility;
            total.luck += stats.luck;
            total.fireResist += stats.fireResist;
            total.coldResist += stats.coldResist;
            total.shockResist += stats.shockResist;
            total.poisonResist += stats.poisonResist;
            
            // Special properties (any equipped item with these grants the bonus)
            if (stats.waterWalking) total.waterWalking = true;
            if (stats.nightVision) total.nightVision = true;
            if (stats.levitation) total.levitation = true;
        }
    }
    
    return total;
}

int EquipmentManager::getTotalArmor() const {
    return getTotalStats().armor;
}

int EquipmentManager::getTotalDamage() const {
    return getTotalStats().damage;
}

float EquipmentManager::getTotalWeight() const {
    float totalWeight = 0.0f;
    for (const auto& [slot, item] : equipment_) {
        if (item) {
            totalWeight += item->getWeight();
        }
    }
    return totalWeight;
}

bool EquipmentManager::hasFullArmorSet() const {
    return isSlotOccupied(EquipmentSlot::HEAD) &&
           isSlotOccupied(EquipmentSlot::CHEST) &&
           isSlotOccupied(EquipmentSlot::LEGS) &&
           isSlotOccupied(EquipmentSlot::FEET);
}

std::vector<std::string> EquipmentManager::getActiveSetBonuses() const {
    std::vector<std::string> bonuses;
    
    if (hasFullArmorSet()) {
        bonuses.push_back("Full Armor Set: +10% damage resistance");
    }
    
    // Check for weapon + shield combo
    auto mainHand = getEquippedItem(EquipmentSlot::MAIN_HAND);
    auto offHand = getEquippedItem(EquipmentSlot::OFF_HAND);
    if (mainHand && offHand && offHand->getType() == ItemType::SHIELD) {
        bonuses.push_back("Sword & Board: +15% block chance");
    }
    
    return bonuses;
}

void EquipmentManager::damageEquipment(EquipmentSlot slot, float damage) {
    auto item = getEquippedItem(slot);
    if (item) {
        item->takeDamage(damage);
        if (item->isBroken()) {
            std::cout << "Warning: " << item->getName() << " has broken!" << std::endl;
        }
    }
}

void EquipmentManager::repairEquipment(EquipmentSlot slot, float amount) {
    auto item = getEquippedItem(slot);
    if (item) {
        item->repair(amount);
    }
}

std::vector<EquipmentSlot> EquipmentManager::getBrokenEquipment() const {
    std::vector<EquipmentSlot> broken;
    for (const auto& [slot, item] : equipment_) {
        if (item && item->isBroken()) {
            broken.push_back(slot);
        }
    }
    return broken;
}

bool EquipmentManager::canEquipItem(const std::shared_ptr<MysticalItem>& item) const {
    return item && item->isEquippable() && !item->isBroken();
}

std::string EquipmentManager::getSlotName(EquipmentSlot slot) const {
    return ItemUtils::equipmentSlotToString(slot);
}

// ============================================================================
// InventorySystem Implementation
// ============================================================================

InventorySystem::InventorySystem(float maxWeight, int maxSlots)
    : inventory_(maxWeight, maxSlots), equipment_() {
}

bool InventorySystem::equipItemFromInventory(const std::string& itemName) {
    auto item = inventory_.findItem(itemName);
    if (!item || !item->isEquippable()) {
        std::cout << "Cannot equip " << itemName << ": item not found or not equippable" << std::endl;
        return false;
    }
    
    // Unequip existing item in that slot first
    EquipmentSlot slot = item->getEquipmentSlot();
    auto existingItem = equipment_.unequipItem(slot);
    if (existingItem) {
        inventory_.addItem(existingItem);
    }
    
    // Remove from inventory and equip
    if (inventory_.removeItem(item, 1)) {
        return equipment_.equipItem(item);
    }
    
    return false;
}

bool InventorySystem::unequipItemToInventory(EquipmentSlot slot) {
    auto item = equipment_.unequipItem(slot);
    if (item) {
        return inventory_.addItem(item);
    }
    return false;
}

std::shared_ptr<EnchantedWeapon> InventorySystem::createWeapon(const std::string& name, WeaponType type, int damage) {
    return std::make_shared<EnchantedWeapon>(name, type, damage);
}

std::shared_ptr<GuardianArmor> InventorySystem::createArmor(const std::string& name, ArmorType type, int armor, EquipmentSlot slot) {
    return std::make_shared<GuardianArmor>(name, type, armor, slot);
}

std::shared_ptr<AlchemicalPotion> InventorySystem::createPotion(const std::string& name, const ItemStats& effects, int duration) {
    return std::make_shared<AlchemicalPotion>(name, effects, duration);
}

std::shared_ptr<MysticalItem> InventorySystem::createMiscItem(const std::string& name, ItemType type, float weight, int value) {
    return std::make_shared<MysticalItem>(name, type, weight, value);
}

ItemStats InventorySystem::getTotalBonuses() const {
    return equipment_.getTotalStats();
}

float InventorySystem::getCarryCapacity() const {
    return inventory_.getMaxWeight();
}

bool InventorySystem::isOverencumbered() const {
    return inventory_.isOverweight();
}

void InventorySystem::printInventoryStatus() const {
    std::cout << "\n=== ADVENTURER'S INVENTORY ===" << std::endl;
    std::cout << "Weight: " << inventory_.getCurrentWeight() << "/" << inventory_.getMaxWeight() << " kg" << std::endl;
    std::cout << "Slots: " << inventory_.getUsedSlots() << "/" << inventory_.getMaxSlots() << std::endl;
    
    if (inventory_.isOverweight()) {
        std::cout << "*** OVERENCUMBERED - Movement speed reduced! ***" << std::endl;
    }
    
    std::cout << "\nItems:" << std::endl;
    for (const auto& item : inventory_.getAllItems()) {
        std::cout << "  " << item->getDisplayName() 
                  << " (" << item->getWeight() << " kg, " << item->getValue() << " gold)" << std::endl;
    }
    
    std::cout << "\n=== EQUIPPED GEAR ===" << std::endl;
    auto totalStats = equipment_.getTotalStats();
    std::cout << "Total Armor: " << totalStats.armor << std::endl;
    std::cout << "Total Damage Bonus: " << totalStats.damage << std::endl;
    
    for (const auto& [slot, item] : equipment_.getAllEquipment()) {
        if (item) {
            std::cout << "  " << ItemUtils::equipmentSlotToString(slot) 
                      << ": " << item->getDisplayName() << std::endl;
        }
    }
    
    auto setBonuses = equipment_.getActiveSetBonuses();
    if (!setBonuses.empty()) {
        std::cout << "\nSet Bonuses:" << std::endl;
        for (const auto& bonus : setBonuses) {
            std::cout << "  " << bonus << std::endl;
        }
    }
}

void InventorySystem::addStartingItems() {
    // Starting weapon - ADD AND EQUIP
    auto rustyDagger = createWeapon("Rusty Dagger", WeaponType::DAGGER, 8);
    rustyDagger->setDescription("A well-worn blade, dull but serviceable");
    rustyDagger->setDurability(60.0f);
    equipment_.equipItem(rustyDagger);  // **CRITICAL FIX**: Equip the weapon!
    
    // Starting armor - ADD AND EQUIP
    auto leatherVest = createArmor("Traveler's Vest", ArmorType::LIGHT, 5, EquipmentSlot::CHEST);
    leatherVest->setDescription("Simple leather protection for the road");
    equipment_.equipItem(leatherVest);  // **CRITICAL FIX**: Equip the armor!
    
    auto leatherBoots = createArmor("Worn Boots", ArmorType::LIGHT, 2, EquipmentSlot::FEET);
    leatherBoots->setDescription("Comfortable boots for long journeys");
    equipment_.equipItem(leatherBoots);  // **CRITICAL FIX**: Equip the boots!
    
    // Starting consumables
    ItemStats healingEffect;
    healingEffect.health = 25;
    auto healthPotion = createPotion("Minor Healing Potion", healingEffect);
    healthPotion->setDescription("Restores a small amount of health instantly");
    inventory_.addItem(healthPotion, 3);
    
    ItemStats manaEffect;
    manaEffect.mana = 20;
    auto manaPotion = createPotion("Minor Mana Potion", manaEffect);
    manaPotion->setDescription("Restores a small amount of magical energy");
    inventory_.addItem(manaPotion, 2);
    
    // Starting miscellaneous items
    auto torch = createMiscItem("Torch", ItemType::TOOL, 0.5f, 5);
    torch->setDescription("Provides light in dark places");
    torch->setStackable(true, 10);
    inventory_.addItem(torch, 5);
    
    auto goldCoin = createMiscItem("Gold Septim", ItemType::TREASURE, 0.01f, 1);
    goldCoin->setDescription("The standard currency of the realm");
    goldCoin->setStackable(true, 1000);
    inventory_.addItem(goldCoin, 50);
    
    // **MORE INTERESTING STARTER ITEMS**
    
    // Magic items for testing
    auto magicRing = createMiscItem("Ring of Minor Protection", ItemType::MISC, 0.1f, 200);
    magicRing->setDescription("A simple ring with weak protective enchantments");
    magicRing->setEquipmentSlot(EquipmentSlot::RING_LEFT);
    magicRing->setRarity(ItemRarity::UNCOMMON);
    ItemStats ringStats;
    ringStats.armor = 3;
    ringStats.health = 10;
    magicRing->setStats(ringStats);
    inventory_.addItem(magicRing);
    
    // Secondary weapon for testing weapon swapping
    auto shortBow = createWeapon("Hunter's Bow", WeaponType::BOW, 12);
    shortBow->setDescription("A reliable bow for hunting and combat");
    shortBow->setRarity(ItemRarity::UNCOMMON);
    inventory_.addItem(shortBow);
    
    // Magical consumables
    ItemStats buffEffect;
    buffEffect.strength = 5;
    buffEffect.agility = 3;
    auto buffPotion = createPotion("Potion of Heroism", buffEffect, 60);  // 60 second duration
    buffPotion->setDescription("Temporarily enhances physical capabilities");
    buffPotion->setRarity(ItemRarity::RARE);
    inventory_.addItem(buffPotion, 2);
    
    // Quest-like items
    auto mysteriousKey = createMiscItem("Ancient Bronze Key", ItemType::QUEST_ITEM, 0.2f, 0);
    mysteriousKey->setDescription("An old key with mysterious runes carved into its surface");
    mysteriousKey->setRarity(ItemRarity::RARE);
    inventory_.addItem(mysteriousKey);
    
    // Valuable treasure
    auto gem = createMiscItem("Sapphire Gem", ItemType::TREASURE, 0.1f, 500);
    gem->setDescription("A brilliant blue gemstone that sparkles with inner light");
    gem->setRarity(ItemRarity::EPIC);
    inventory_.addItem(gem);
    
    // Crafting ingredients
    auto ingredient = createMiscItem("Dragon Scale", ItemType::INGREDIENT, 0.3f, 150);
    ingredient->setDescription("A shimmering scale from an ancient dragon");
    ingredient->setRarity(ItemRarity::LEGENDARY);
    ingredient->setStackable(true, 20);
    inventory_.addItem(ingredient, 3);
    
    std::cout << "Starting inventory loaded with diverse adventuring gear and magical items!" << std::endl;
}

// ============================================================================
// ItemUtils Implementation
// ============================================================================

namespace ItemUtils {

std::string itemTypeToString(ItemType type) {
    switch (type) {
        case ItemType::WEAPON: return "Weapon";
        case ItemType::ARMOR: return "Armor";
        case ItemType::SHIELD: return "Shield";
        case ItemType::CONSUMABLE: return "Consumable";
        case ItemType::SCROLL: return "Scroll";
        case ItemType::INGREDIENT: return "Ingredient";
        case ItemType::TREASURE: return "Treasure";
        case ItemType::TOOL: return "Tool";
        case ItemType::QUEST_ITEM: return "Quest Item";
        case ItemType::MISC: return "Miscellaneous";
        default: return "Unknown";
    }
}

std::string equipmentSlotToString(EquipmentSlot slot) {
    switch (slot) {
        case EquipmentSlot::MAIN_HAND: return "Main Hand";
        case EquipmentSlot::OFF_HAND: return "Off Hand";
        case EquipmentSlot::HEAD: return "Head";
        case EquipmentSlot::CHEST: return "Chest";
        case EquipmentSlot::LEGS: return "Legs";
        case EquipmentSlot::FEET: return "Feet";
        case EquipmentSlot::RING_LEFT: return "Left Ring";
        case EquipmentSlot::RING_RIGHT: return "Right Ring";
        case EquipmentSlot::AMULET: return "Amulet";
        case EquipmentSlot::NONE: return "Not Equippable";
        default: return "Unknown Slot";
    }
}

std::string weaponTypeToString(WeaponType type) {
    switch (type) {
        case WeaponType::LONGSWORD: return "Longsword";
        case WeaponType::SHORTSWORD: return "Shortsword";
        case WeaponType::DAGGER: return "Dagger";
        case WeaponType::MACE: return "Mace";
        case WeaponType::AXE: return "Axe";
        case WeaponType::BOW: return "Bow";
        case WeaponType::STAFF: return "Staff";
        case WeaponType::WAND: return "Wand";
        case WeaponType::UNARMED: return "Unarmed";
        default: return "Unknown Weapon";
    }
}

std::string armorTypeToString(ArmorType type) {
    switch (type) {
        case ArmorType::LIGHT: return "Light Armor";
        case ArmorType::MEDIUM: return "Medium Armor";
        case ArmorType::HEAVY: return "Heavy Armor";
        case ArmorType::ROBE: return "Magical Robe";
        default: return "Unknown Armor";
    }
}

std::string rarityToString(ItemRarity rarity) {
    switch (rarity) {
        case ItemRarity::COMMON: return "Common";
        case ItemRarity::UNCOMMON: return "Uncommon";
        case ItemRarity::RARE: return "Rare";
        case ItemRarity::EPIC: return "Epic";
        case ItemRarity::LEGENDARY: return "Legendary";
        case ItemRarity::ARTIFACT: return "Artifact";
        default: return "Unknown";
    }
}

const char* getRarityColor(ItemRarity rarity) {
    switch (rarity) {
        case ItemRarity::COMMON: return "\033[37m";      // White
        case ItemRarity::UNCOMMON: return "\033[32m";    // Green
        case ItemRarity::RARE: return "\033[34m";        // Blue
        case ItemRarity::EPIC: return "\033[35m";        // Purple
        case ItemRarity::LEGENDARY: return "\033[33m";   // Orange/Yellow
        case ItemRarity::ARTIFACT: return "\033[31m";    // Red
        default: return "\033[0m";                       // Reset
    }
}

int calculateItemValue(const MysticalItem& item) {
    int baseValue = item.getValue();
    float rarityMultiplier = 1.0f;
    
    switch (item.getRarity()) {
        case ItemRarity::UNCOMMON: rarityMultiplier = 2.0f; break;
        case ItemRarity::RARE: rarityMultiplier = 5.0f; break;
        case ItemRarity::EPIC: rarityMultiplier = 10.0f; break;
        case ItemRarity::LEGENDARY: rarityMultiplier = 25.0f; break;
        case ItemRarity::ARTIFACT: rarityMultiplier = 100.0f; break;
        default: rarityMultiplier = 1.0f; break;
    }
    
    // Factor in condition
    float conditionMultiplier = item.getMaxDurability() > 0 ? 
        (item.getDurability() / item.getMaxDurability()) : 1.0f;
    
    return static_cast<int>(baseValue * rarityMultiplier * conditionMultiplier);
}

float calculateItemWeight(const MysticalItem& item) {
    float baseWeight = item.getWeight();
    
    // Broken items are harder to carry effectively
    if (item.isBroken()) {
        baseWeight *= 1.2f;
    }
    
    return baseWeight;
}

} // namespace ItemUtils
