// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Effects/LProjectGameplayEffect.h"
#include "LProjectGE_Cooldown.generated.h"

/**
 * Generic skill cooldown: a duration effect whose length is a SetByCaller (SetByCaller.CooldownDuration).
 * Each skill ability injects its own cooldown tag onto the spec's dynamic granted tags and sets the
 * duration, so one effect serves every skill and the HUD can query remaining time by tag.
 */
UCLASS()
class LPROJECT_API ULProjectGE_Cooldown : public ULProjectGameplayEffect
{
	GENERATED_BODY()

public:
	ULProjectGE_Cooldown();
};
