// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "LProjectGameplayEffect.generated.h"

/**
 * Thin project base for natively-authored GameplayEffects (damage, DoT, buffs, debuffs, cooldowns) so
 * shared defaults can live in one place. Mirrors how ULProjectGameplayAbility bases all abilities.
 */
UCLASS()
class LPROJECT_API ULProjectGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	ULProjectGameplayEffect();
};
