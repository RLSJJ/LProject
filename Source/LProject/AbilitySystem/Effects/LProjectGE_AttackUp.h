// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Effects/LProjectGameplayEffect.h"
#include "LProjectGE_AttackUp.generated.h"

/**
 * Timed offensive buff: adds a flat AttackPower bonus for its duration. Demonstrates the duration
 * stat-modifier (buff/debuff) pattern; e.g. granted to the player as a counter reward.
 */
UCLASS()
class LPROJECT_API ULProjectGE_AttackUp : public ULProjectGameplayEffect
{
	GENERATED_BODY()

public:
	ULProjectGE_AttackUp();
};
