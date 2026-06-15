// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Effects/LProjectGameplayEffect.h"
#include "LProjectGE_Damage.generated.h"

/**
 * Instant damage payload: runs ULProjectExecCalc_Damage. The attacker assigns SetByCaller.Damage and
 * SetByCaller.StaggerDamage on the spec, then applies this to a target ASC. The universal hit effect
 * shared by the player basic attack, boss patterns, and the counter.
 */
UCLASS()
class LPROJECT_API ULProjectGE_Damage : public ULProjectGameplayEffect
{
	GENERATED_BODY()

public:
	ULProjectGE_Damage();
};
