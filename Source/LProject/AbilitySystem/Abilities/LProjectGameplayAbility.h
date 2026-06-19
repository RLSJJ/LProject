// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "LProjectGameplayAbility.generated.h"

class UGameplayEffect;

/**
 * Base class for all LProject abilities. Sets project-wide instancing/replication defaults and shares the
 * Identity (awakening resource) helper so every ability builds/spends the resource through the GAS pipeline.
 */
UCLASS(Abstract)
class LPROJECT_API ULProjectGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	ULProjectGameplayAbility();

protected:
	/** Apply an Identity delta (+build / -spend) via GE_IdentityGain — clamped to [0, IdentityMax]. */
	void ApplyIdentityDelta(float Delta);

	/** Identity-resource effect (defaults to ULProjectGE_IdentityGain). */
	UPROPERTY(EditDefaultsOnly, Category = "Identity")
	TSubclassOf<UGameplayEffect> IdentityGainEffect;
};
