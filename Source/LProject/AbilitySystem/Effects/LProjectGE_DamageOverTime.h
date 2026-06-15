// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Effects/LProjectGameplayEffect.h"
#include "LProjectGE_DamageOverTime.generated.h"

/**
 * Duration damage-over-time (bleed/burn). Re-runs the shared damage exec every Period via SetByCaller,
 * so DoT obeys the same AttackPower/Defense formula as a direct hit. The DoT pattern, tunable per-apply.
 */
UCLASS()
class LPROJECT_API ULProjectGE_DamageOverTime : public ULProjectGameplayEffect
{
	GENERATED_BODY()

public:
	ULProjectGE_DamageOverTime();
};
