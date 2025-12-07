////////////////////////////////////////////////////////////////////////
// Crystal Server - an opensource roleplaying game
////////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////

#include "creatures/players/proficiencies/proficiencies.hpp"
#include "creatures/players/proficiencies/proficiencies_definitions.hpp"

#include "config/configmanager.hpp"
#include "creatures/players/player.hpp"
#include "lib/di/container.hpp"
#include "utils/pugicast.hpp"
#include <utils/tools.hpp>
#include "items/item.hpp"

Proficiencies &Proficiencies::getInstance() {
	return inject<Proficiencies>();
}

static WeaponProficiencyPerkType_t getPerkType(const std::string& str) {
    static const std::unordered_map<std::string, WeaponProficiencyPerkType_t> map = {
        {"attackDamage", PROFICIENCY_PERK_ATTACK_DAMAGE},
        {"defense", PROFICIENCY_PERK_DEFENSE},
        {"shieldMod", PROFICIENCY_PERK_WEAPON_SHIELD_MOD},
        {"skillBonus", PROFICIENCY_PERK_SKILLID_BONUS},
        {"specialMagicLevel", PROFICIENCY_PERK_SPECIAL_MAGIC_LEVEL},
        {"augment", PROFICIENCY_PERK_AUGMENT_TYPE},
        {"bestiaryDamage", PROFICIENCY_PERK_BESTIARY_DAMAGE},
        {"damageBoss", PROFICIENCY_PERK_DAMAGE_GAIN_BOSS_AND_SINISTER_EMBRACED},
        {"critChance", PROFICIENCY_PERK_CRITICAL_HIT_CHANCE},
        {"critChanceSpell", PROFICIENCY_PERK_CRITICAL_HIT_CHANCE_FOR_ELEMENT_ID_SPELLS_AND_RUNES},
        {"critChanceRune", PROFICIENCY_PERK_CRITICAL_HIT_CHANCE_FOR_OFFENSIVE_RUNES},
        {"critChanceAuto", PROFICIENCY_PERK_CRITICAL_HIT_CHANCE_FOR_AUTOATTACK},
        {"critExtraDamage", PROFICIENCY_PERK_CRITICAL_EXTRA_DAMAGE},
        {"critExtraDamageSpell", PROFICIENCY_PERK_CRITICAL_EXTRA_DAMAGE_FOR_ELEMENT_ID_SPELLS_AND_RUNES},
        {"critExtraDamageRune", PROFICIENCY_PERK_CRITICAL_EXTRA_DAMAGE_FOR_OFFENSIVE_RUNES},
        {"critExtraDamageAuto", PROFICIENCY_PERK_CRITICAL_EXTRA_DAMAGE_FOR_AUTOATTACK},
        {"manaLeech", PROFICIENCY_PERK_MANA_LEECH},
        {"lifeLeech", PROFICIENCY_PERK_LIFE_LEECH},
        {"manaOnHit", PROFICIENCY_PERK_MANA_GAIN_ONHIT},
        {"lifeOnHit", PROFICIENCY_PERK_LIFE_GAIN_ONHIT},
        {"manaOnKill", PROFICIENCY_PERK_MANA_GAIN_ONKILL},
        {"lifeOnKill", PROFICIENCY_PERK_LIFE_GAIN_ONKILL},
        {"damageAtRange", PROFICIENCY_PERK_GAIN_DAMAGE_AT_RANGE},
        {"rangedHitChance", PROFICIENCY_PERK_RANGED_HIT_CHANCE},
        {"attackRange", PROFICIENCY_PERK_ATTACK_RANGE},
        {"skillDamageAuto", PROFICIENCY_PERK_SKILLID_PERCENTAGE_AS_EXTRA_DAMAGE_FOR_AUTOATTACK},
        {"skillDamageSpell", PROFICIENCY_PERK_SKILLID_PERCENTAGE_AS_EXTRA_DAMAGE_FOR_SPELLS},
        {"skillHealingSpell", PROFICIENCY_PERK_SKILLID_PERCENTAGE_AS_EXTRA_HEALING_FOR_SPELLS}
    };
    auto it = map.find(str);
    return it != map.end() ? it->second : PROFICIENCY_PERK_ATTACK_DAMAGE;
}

static int8_t getSkillId(const std::string& str) {
    static const std::unordered_map<std::string, int8_t> map = {
        {"magic", PROFICIENCY_SKILL_MAGIC},
        {"shield", PROFICIENCY_SKILL_SHIELD},
        {"distance", PROFICIENCY_SKILL_DISTANCE},
        {"sword", PROFICIENCY_SKILL_SWORD},
        {"club", PROFICIENCY_SKILL_CLUB},
        {"axe", PROFICIENCY_SKILL_AXE},
        {"fist", PROFICIENCY_SKILL_FIST},
        {"fishing", PROFICIENCY_SKILL_FISHING}
    };
    auto it = map.find(str);
    return it != map.end() ? it->second : 0;
}

static int32_t getDamageType(const std::string& str) {
    static const std::unordered_map<std::string, int32_t> map = {
        {"fire", PROFICIENCY_DAMAGETYPE_FIRE},
        {"earth", PROFICIENCY_DAMAGETYPE_EARTH},
        {"energy", PROFICIENCY_DAMAGETYPE_ENERGY},
        {"ice", PROFICIENCY_DAMAGETYPE_ICE},
        {"holy", PROFICIENCY_DAMAGETYPE_HOLY},
        {"death", PROFICIENCY_DAMAGETYPE_DEATH},
        {"healing", PROFICIENCY_DAMAGETYPE_HEALING}
    };
    auto it = map.find(str);
    return it != map.end() ? it->second : 0;
}

static uint8_t getAugmentType(const std::string& str) {
    static const std::unordered_map<std::string, uint8_t> map = {
        {"baseDamage", PROFICIENCY_AUGMENTTYPE_BASE_DAMAGE},
		{"healing", PROFICIENCY_AUGMENTTYPE_HEALING},
        {"cooldown", PROFICIENCY_AUGMENTTYPE_COOLDOWN},
        {"increasedDamage", PROFICIENCY_AUGMENTTYPE_INCREASED_DAMAGE},
        {"lifeLeech", PROFICIENCY_AUGMENTTYPE_LIFE_LEECH},
        {"manaLeech", PROFICIENCY_AUGMENTTYPE_MANA_LEECH},
        {"critExtraDamage", PROFICIENCY_AUGMENTTYPE_CRITICAL_EXTRA_DAMAGE},
        {"critChance", PROFICIENCY_AUGMENTTYPE_CRITICAL_HIT_CHANCE}
        // Adicione outros augments conforme necessário baseado no proficiencies_definitions.hpp
    };
    auto it = map.find(str);
    return it != map.end() ? it->second : 0;
}

// --- XML ---

bool Proficiencies::loadFromXml(bool /* reloading */) {
    const auto &fileName = g_configManager().getString(CORE_DIRECTORY) + "/items/proficiencies.xml";
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(fileName.c_str());

    if (!result) {
        g_logger().error("[Proficiencies] Failed to parse {}: {}", fileName, result.description());
        return false;
    }

    loaded = true;
    proficienciesMap.clear();

    for (auto proficiencyNode : doc.child("proficiencies").children("proficiency")) {
        uint32_t proficiencyId = proficiencyNode.attribute("id").as_uint();
        // O nome é só para organização visual no XML, não é usado na struct base, 
        // mas ajuda muito a saber o que você está editando.
        
        // Contar quantos levels existem para inicializar a struct
        uint8_t maxLevel = 0;
        for (auto lvl : proficiencyNode.children("level")) { maxLevel++; }

        WeaponProficiencyStruct proficiencyStruct(proficiencyId, maxLevel);

        for (auto levelNode : proficiencyNode.children("level")) {
            uint8_t levelId = levelNode.attribute("id").as_uint();
            
            // Contar perks
            uint8_t maxPerks = 0;
            for (auto perk : levelNode.children("perk")) { maxPerks++; }

            ProficiencyLevelStruct levelStruct(levelId, maxPerks);

            uint8_t slotIndex = 1;
            for (auto perkNode : levelNode.children("perk")) {
                std::string typeStr = perkNode.attribute("type").as_string();
                WeaponProficiencyPerkType_t perkType = getPerkType(typeStr);
                float value = perkNode.attribute("value").as_float();

                ProficiencyPerk perk(slotIndex, perkType, value);

                // Atributos Opcionais (String baseados)
                if (perkNode.attribute("skill")) {
                    perk.skillId = getSkillId(perkNode.attribute("skill").as_string());
                }

                if (perkNode.attribute("damageType")) {
                    perk.damageType = getDamageType(perkNode.attribute("damageType").as_string());
                }
                
                // ElementId e DamageType geralmente usam o mesmo enum no seu sistema
                if (perkNode.attribute("element")) {
                    perk.damageType = getDamageType(perkNode.attribute("element").as_string());
                }

                if (perkNode.attribute("augment")) {
                    perk.augmentType = getAugmentType(perkNode.attribute("augment").as_string());
                }

                // Atributos Numéricos diretos
                if (perkNode.attribute("range")) {
                    perk.range = perkNode.attribute("range").as_uint();
                }

                if (perkNode.attribute("spellId")) {
                    perk.spellId = perkNode.attribute("spellId").as_uint();
                }

                if (perkNode.attribute("bestiaryId")) {
                    perk.bestiaryId = perkNode.attribute("bestiaryId").as_uint();
                }

                levelStruct.proficiencyDataPerks.emplace_back(std::move(perk));
                slotIndex++;
            }
            proficiencyStruct.proficiencyDataLevel.emplace_back(std::move(levelStruct));
        }

        if (proficienciesMap.count(proficiencyId)) {
            g_logger().warn("[Proficiencies] Duplicate proficiencyId {} in XML!", proficiencyId);
        }

        proficienciesMap[proficiencyId] = std::move(proficiencyStruct);
    }

    return true;
}

bool Proficiencies::reload() {
    proficienciesMap.clear();
    loaded = false;
    return loadFromXml(true);
}


const WeaponProficiencyStruct* Proficiencies::getProficiencyByItemId(uint16_t itemId) const {
	const ItemType &itemType = Item::items[itemId];
	if (!itemType.proficiencyId) {
		g_logger().error("[{}] itemId {} not found proficiencyId", __FUNCTION__, itemId);
		return nullptr;
	}

	auto it = proficienciesMap.find(itemType.proficiencyId);
	if (it == proficienciesMap.end()) {
		g_logger().error("[{}] Proficiency ID {} not found", __FUNCTION__, itemType.proficiencyId);
		return nullptr;
	}
	return &it->second;
}

uint8_t Proficiencies::getMaxProficiencyLevelForItem(uint16_t itemId) const {
	const ItemType &itemType = Item::items[itemId];
	if (!itemType.proficiencyId) {
		g_logger().error("[{}] itemId {} not found proficiencyId", __FUNCTION__, itemId);
		return 0;
	}

	const auto* proficiency = getProficiencyByItemId(itemType.proficiencyId);
	if (!proficiency) {
		return 0;
	}

	return proficiency->maxProficiencyLevel;
}

uint8_t Proficiencies::getMaxPerksPerProficiencyLevelForItem(uint16_t itemId, uint8_t level) const {
	const ItemType &itemType = Item::items[itemId];
	if (!itemType.proficiencyId) {
		g_logger().error("[{}] itemId {} not found proficiencyId", __FUNCTION__, itemId);
		return 0;
	}

	const auto* proficiency = getProficiencyByItemId(itemType.proficiencyId);
	if (!proficiency) {
		return 0;
	}

	for (const auto &data : proficiency->proficiencyDataLevel) {
		if (data.proficiencyLevel == level) {
			return data.maxProficiencyPerks;
		}
	}
	return 0;
}
