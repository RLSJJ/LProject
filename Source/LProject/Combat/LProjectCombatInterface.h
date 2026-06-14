// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LProjectCombatInterface.generated.h"

UINTERFACE(MinimalAPI)
class ULProjectCombatant : public UInterface
{
	GENERATED_BODY()
};

/**
 * Contract shared by anything that fights: the player, the raid boss, and adds.
 * Lets systems (telegraphs, damage, UI) query/operate on combatants without knowing concrete types.
 */
class ILProjectCombatant
{
	GENERATED_BODY()

public:
	virtual float GetHealth() const = 0;
	virtual float GetMaxHealth() const = 0;
	virtual bool IsAlive() const = 0;
};
