// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "LProjectExecCalc_Damage.generated.h"

/**
 * The single combat-math seam. Captures the source's AttackPower and the target's Defense (both live),
 * reads the per-hit SetByCaller.Damage / SetByCaller.StaggerDamage magnitudes off the spec, and outputs:
 *   - the Damage meta-attribute on ULProjectAttributeSet (final HP damage), and
 *   - a negative StaggerCurrent modifier on ULProjectBossAttributeSet (only if the target is a boss).
 *
 * Final damage = Base * (AttackPower/100) * (1 - clamp(Defense/100, 0, 0.95)) * (groggy ? bonus : 1).
 * Every attacker (player basic attack, boss patterns, counter, DoT) reuses this one calculation.
 */
UCLASS()
class LPROJECT_API ULProjectExecCalc_Damage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	ULProjectExecCalc_Damage();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	    FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

	/** Bonus damage multiplier applied while the target boss is groggy/무력화. */
	static constexpr float GroggyDamageMultiplier = 2.0f;
};
