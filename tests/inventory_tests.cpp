#include "../tests/test_framework.h"
#include "../inventory.h"
#include <iostream>
#include <memory>

/**
 * Comprehensive test suite for Browserwind inventory system
 * Tests all major functionality including items, equipment, and stats
 */

bool test_item_creation() {
    std::cout << "Testing item creation..." << std::endl;
    
    // Test basic item creation
    MysticalItem basicItem("Test Sword", ItemType::WEAPON, 2.5f, 100);
    TEST_ASSERT_EQUAL(basicItem.getName(), "Test Sword");
    TEST_ASSERT_EQUAL(basicItem.getType(), ItemType::WEAPON);
    TEST_ASSERT_EQUAL(basicItem.getWeight(), 2.5f);
    TEST_ASSERT_EQUAL(basicItem.getValue(), 100);
    
    // Test weapon creation
    EnchantedWeapon dagger("Test Dagger", WeaponType::DAGGER, 15);
    TEST_ASSERT_EQUAL(dagger.getWeaponType(), WeaponType::DAGGER);
    TEST_ASSERT_TRUE(dagger.getAttackSpeed() > 1.5f);  // Daggers should be fast
    TEST_ASSERT_TRUE(dagger.getCritChance() > 0.1f);   // Daggers should have high crit
    TEST_ASSERT_EQUAL(dagger.getStats().damage, 15);
    
    // Test armor creation
    GuardianArmor chestplate("Test Plate", ArmorType::HEAVY, 20, EquipmentSlot::CHEST);
    TEST_ASSERT_EQUAL(chestplate.getArmorType(), ArmorType::HEAVY);
    TEST_ASSERT_EQUAL(chestplate.getEquipmentSlot(), EquipmentSlot::CHEST);
    TEST_ASSERT_EQUAL(chestplate.getStats().armor, 20);
    TEST_ASSERT_TRUE(chestplate.getWeight() > 10.0f);  // Heavy armor should be heavy
    
    // Test potion creation
    ItemStats potionEffects;
    potionEffects.health = 50;
    AlchemicalPotion healingPotion("Test Health Potion", potionEffects);
    TEST_ASSERT_EQUAL(healingPotion.getEffects().health, 50);
    TEST_ASSERT_TRUE(healingPotion.isStackable());
    TEST_ASSERT_TRUE(healingPotion.isInstant());
    
    std::cout << "✅ Item creation tests passed!" << std::endl;
    return true;
}

bool test_inventory_management() {
    std::cout << "Testing inventory management..." << std::endl;
    
    AdventurerInventory inventory(100.0f, 20);  // 100kg, 20 slots
    
    // Test adding items
    auto sword = std::make_shared<EnchantedWeapon>("Iron Sword", WeaponType::LONGSWORD, 20);
    TEST_ASSERT_TRUE(inventory.addItem(sword));
    TEST_ASSERT_TRUE(inventory.hasItem("Iron Sword"));
    TEST_ASSERT_EQUAL(inventory.getUsedSlots(), 1);
    
    // Test stackable items
    auto potion = std::make_shared<AlchemicalPotion>("Health Potion", ItemStats{});
    TEST_ASSERT_TRUE(inventory.addItem(potion, 5));
    TEST_ASSERT_EQUAL(inventory.getItemQuantity("Health Potion"), 5);
    
    // Test weight limits
    float currentWeight = inventory.getCurrentWeight();
    auto heavyItem = std::make_shared<MysticalItem>("Heavy Boulder", ItemType::MISC, 200.0f, 1);
    TEST_ASSERT_FALSE(inventory.addItem(heavyItem));  // Should fail due to weight
    
    // Test item removal
    TEST_ASSERT_TRUE(inventory.removeItem("Health Potion", 2));
    TEST_ASSERT_EQUAL(inventory.getItemQuantity("Health Potion"), 3);
    
    // Test finding items
    auto foundSword = inventory.findItem("Iron Sword");
    TEST_ASSERT_TRUE(foundSword != nullptr);
    TEST_ASSERT_EQUAL(foundSword->getName(), "Iron Sword");
    
    // Test finding by type
    auto weapons = inventory.findItemsByType(ItemType::WEAPON);
    TEST_ASSERT_EQUAL(weapons.size(), 1);
    TEST_ASSERT_EQUAL(weapons[0]->getName(), "Iron Sword");
    
    std::cout << "✅ Inventory management tests passed!" << std::endl;
    return true;
}

bool test_equipment_system() {
    std::cout << "Testing equipment system..." << std::endl;
    
    EquipmentManager equipment;
    
    // Test equipment slots start empty
    TEST_ASSERT_FALSE(equipment.isSlotOccupied(EquipmentSlot::MAIN_HAND));
    TEST_ASSERT_TRUE(equipment.getEquippedItem(EquipmentSlot::MAIN_HAND) == nullptr);
    
    // Test equipping weapon
    auto sword = std::make_shared<EnchantedWeapon>("Steel Sword", WeaponType::LONGSWORD, 25);
    TEST_ASSERT_TRUE(equipment.equipItem(sword));
    TEST_ASSERT_TRUE(equipment.isSlotOccupied(EquipmentSlot::MAIN_HAND));
    
    auto equippedWeapon = equipment.getEquippedItem(EquipmentSlot::MAIN_HAND);
    TEST_ASSERT_TRUE(equippedWeapon != nullptr);
    TEST_ASSERT_EQUAL(equippedWeapon->getName(), "Steel Sword");
    
    // Test stats calculation
    auto totalStats = equipment.getTotalStats();
    TEST_ASSERT_EQUAL(totalStats.damage, 25);
    
    // Test equipping armor
    auto helmet = std::make_shared<GuardianArmor>("Steel Helmet", ArmorType::MEDIUM, 8, EquipmentSlot::HEAD);
    auto chestplate = std::make_shared<GuardianArmor>("Steel Chestplate", ArmorType::MEDIUM, 15, EquipmentSlot::CHEST);
    
    TEST_ASSERT_TRUE(equipment.equipItem(helmet));
    TEST_ASSERT_TRUE(equipment.equipItem(chestplate));
    
    // Test armor bonus calculation
    totalStats = equipment.getTotalStats();
    TEST_ASSERT_EQUAL(totalStats.armor, 23);  // 8 + 15
    TEST_ASSERT_EQUAL(totalStats.damage, 25); // Still has weapon damage
    
    // Test unequipping
    auto unequippedSword = equipment.unequipItem(EquipmentSlot::MAIN_HAND);
    TEST_ASSERT_TRUE(unequippedSword != nullptr);
    TEST_ASSERT_EQUAL(unequippedSword->getName(), "Steel Sword");
    TEST_ASSERT_FALSE(equipment.isSlotOccupied(EquipmentSlot::MAIN_HAND));
    
    // Test stats recalculation after unequipping
    totalStats = equipment.getTotalStats();
    TEST_ASSERT_EQUAL(totalStats.damage, 0);   // No weapon equipped
    TEST_ASSERT_EQUAL(totalStats.armor, 23);   // Still has armor
    
    std::cout << "✅ Equipment system tests passed!" << std::endl;
    return true;
}

bool test_item_stacking() {
    std::cout << "Testing item stacking..." << std::endl;
    
    AdventurerInventory inventory(50.0f, 10);
    
    // Create stackable potions
    ItemStats healEffect;
    healEffect.health = 25;
    auto potion1 = std::make_shared<AlchemicalPotion>("Health Potion", healEffect);
    auto potion2 = std::make_shared<AlchemicalPotion>("Health Potion", healEffect);
    
    // Test stacking
    TEST_ASSERT_TRUE(inventory.addItem(potion1, 3));
    TEST_ASSERT_EQUAL(inventory.getUsedSlots(), 1);  // Should use only 1 slot
    TEST_ASSERT_EQUAL(inventory.getItemQuantity("Health Potion"), 3);
    
    // Add more of the same item
    TEST_ASSERT_TRUE(inventory.addItem(potion2, 5));
    TEST_ASSERT_EQUAL(inventory.getUsedSlots(), 1);  // Still only 1 slot
    TEST_ASSERT_EQUAL(inventory.getItemQuantity("Health Potion"), 8);
    
    // Test stack limit (potions max at 10)
    TEST_ASSERT_TRUE(inventory.addItem(potion2, 2));
    TEST_ASSERT_EQUAL(inventory.getItemQuantity("Health Potion"), 10);  // Should cap at 10
    
    // Test that adding more creates a new stack
    TEST_ASSERT_TRUE(inventory.addItem(potion2, 1));
    TEST_ASSERT_EQUAL(inventory.getUsedSlots(), 2);  // Now should have 2 slots
    TEST_ASSERT_EQUAL(inventory.getItemQuantity("Health Potion"), 11);  // 10 + 1
    
    std::cout << "✅ Item stacking tests passed!" << std::endl;
    return true;
}

bool test_item_durability() {
    std::cout << "Testing item durability..." << std::endl;
    
    EnchantedWeapon sword("Test Sword", WeaponType::LONGSWORD, 20);
    
    // Test initial durability
    TEST_ASSERT_EQUAL(sword.getDurability(), 100.0f);
    TEST_ASSERT_EQUAL(sword.getMaxDurability(), 100.0f);
    TEST_ASSERT_FALSE(sword.isBroken());
    
    // Test taking damage
    sword.takeDamage(30.0f);
    TEST_ASSERT_EQUAL(sword.getDurability(), 70.0f);
    TEST_ASSERT_FALSE(sword.isBroken());
    
    // Test breaking
    sword.takeDamage(80.0f);  // Total 110 damage
    TEST_ASSERT_EQUAL(sword.getDurability(), 0.0f);
    TEST_ASSERT_TRUE(sword.isBroken());
    
    // Test repair
    sword.repair(50.0f);
    TEST_ASSERT_EQUAL(sword.getDurability(), 50.0f);
    TEST_ASSERT_FALSE(sword.isBroken());
    
    // Test over-repair protection
    sword.repair(100.0f);
    TEST_ASSERT_EQUAL(sword.getDurability(), 100.0f);  // Should cap at max
    
    std::cout << "✅ Item durability tests passed!" << std::endl;
    return true;
}

bool test_full_inventory_system() {
    std::cout << "Testing full inventory system..." << std::endl;
    
    InventorySystem fullSystem(200.0f, 50);
    
    // Test starting items
    fullSystem.addStartingItems();
    
    auto& inventory = fullSystem.getInventory();
    auto& equipment = fullSystem.getEquipment();
    
    // Verify starting equipment is equipped
    TEST_ASSERT_TRUE(equipment.isSlotOccupied(EquipmentSlot::MAIN_HAND));
    TEST_ASSERT_TRUE(equipment.isSlotOccupied(EquipmentSlot::CHEST));
    TEST_ASSERT_TRUE(equipment.isSlotOccupied(EquipmentSlot::FEET));
    
    // Verify stats are calculated correctly
    auto totalStats = fullSystem.getTotalBonuses();
    TEST_ASSERT_TRUE(totalStats.damage > 0);  // Should have weapon damage
    TEST_ASSERT_TRUE(totalStats.armor > 0);   // Should have armor protection
    
    // Test item creation helpers
    auto testWeapon = fullSystem.createWeapon("Test Blade", WeaponType::SHORTSWORD, 18);
    TEST_ASSERT_TRUE(testWeapon != nullptr);
    TEST_ASSERT_EQUAL(testWeapon->getWeaponType(), WeaponType::SHORTSWORD);
    
    auto testArmor = fullSystem.createArmor("Test Shield", ArmorType::MEDIUM, 12, EquipmentSlot::OFF_HAND);
    TEST_ASSERT_TRUE(testArmor != nullptr);
    TEST_ASSERT_EQUAL(testArmor->getEquipmentSlot(), EquipmentSlot::OFF_HAND);
    
    // Test equipping from inventory
    inventory.addItem(testWeapon);
    TEST_ASSERT_TRUE(fullSystem.equipItemFromInventory("Test Blade"));
    
    // Test unequipping to inventory
    TEST_ASSERT_TRUE(fullSystem.unequipItemToInventory(EquipmentSlot::MAIN_HAND));
    TEST_ASSERT_TRUE(inventory.hasItem("Test Blade"));
    
    std::cout << "✅ Full inventory system tests passed!" << std::endl;
    return true;
}

bool test_item_utils() {
    std::cout << "Testing item utilities..." << std::endl;
    
    // Test string conversions
    TEST_ASSERT_EQUAL(ItemUtils::itemTypeToString(ItemType::WEAPON), "Weapon");
    TEST_ASSERT_EQUAL(ItemUtils::weaponTypeToString(WeaponType::LONGSWORD), "Longsword");
    TEST_ASSERT_EQUAL(ItemUtils::armorTypeToString(ArmorType::HEAVY), "Heavy Armor");
    TEST_ASSERT_EQUAL(ItemUtils::rarityToString(ItemRarity::LEGENDARY), "Legendary");
    
    // Test value calculation
    MysticalItem commonItem("Common Item", ItemType::MISC, 1.0f, 100);
    TEST_ASSERT_EQUAL(ItemUtils::calculateItemValue(commonItem), 100);
    
    MysticalItem rareItem("Rare Item", ItemType::MISC, 1.0f, 100);
    rareItem.setRarity(ItemRarity::RARE);
    TEST_ASSERT_TRUE(ItemUtils::calculateItemValue(rareItem) > 100);  // Should be more valuable
    
    // Test broken item value
    MysticalItem brokenItem("Broken Item", ItemType::WEAPON, 1.0f, 100);
    brokenItem.setDurability(100.0f, 100.0f);
    brokenItem.takeDamage(100.0f);  // Break it
    TEST_ASSERT_TRUE(brokenItem.isBroken());
    TEST_ASSERT_TRUE(ItemUtils::calculateItemValue(brokenItem) < 100);  // Should be less valuable
    
    std::cout << "✅ Item utilities tests passed!" << std::endl;
    return true;
}

// Register inventory tests with the test framework
void registerInventoryTests() {
    REGISTER_TEST("🎒 Item Creation", test_item_creation);
    REGISTER_TEST("🎒 Inventory Management", test_inventory_management);
    REGISTER_TEST("🎒 Equipment System", test_equipment_system);
    REGISTER_TEST("🎒 Item Stacking", test_item_stacking);
    REGISTER_TEST("🎒 Item Durability", test_item_durability);
    REGISTER_TEST("🎒 Full System Integration", test_full_inventory_system);
    REGISTER_TEST("🎒 Item Utilities", test_item_utils);
}
