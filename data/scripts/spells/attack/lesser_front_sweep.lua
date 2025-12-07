local combat = Combat()
combat:setParameter(COMBAT_PARAM_TYPE, COMBAT_PHYSICALDAMAGE)
combat:setParameter(COMBAT_PARAM_EFFECT, CONST_ME_HITAREA)
combat:setParameter(COMBAT_PARAM_BLOCKARMOR, 1)
combat:setParameter(COMBAT_PARAM_USECHARGES, 1)
combat:setArea(createCombatArea(AREA_WAVE6, AREADIAGONAL_WAVE6))

function onGetFormulaValues(player, skill, attack, factor)
	local skillTotal = skill * attack
	local levelTotal = player:getLevel() / 5
	
    local min = -((skillTotal * 0.044 / 5) + (34.1 / 5) + levelTotal)
	local max = -((skillTotal * 0.088 / 5) + (49.5 / 5) + levelTotal)

end

combat:setCallback(CALLBACK_PARAM_SKILLVALUE, "onGetFormulaValues")

local spell = Spell("instant")

function spell.onCastSpell(creature, var)
	return combat:execute(creature, var)
end

spell:group("attack")
spell:id(271)
spell:name("Lesser Front Sweep")
spell:words("exori infir min")
spell:castSound(SOUND_EFFECT_TYPE_SPELL_LESSER_FRONT_SWEEP)
spell:level(1)
spell:mana(6)
spell:isPremium(false)
spell:needDirection(true)
spell:needWeapon(true)
spell:cooldown(6 * 1000)
spell:groupCooldown(2 * 1000)
spell:needLearn(false)
spell:vocation("knight;true", "elite knight;true")
spell:register()
