// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LProjectGameplayAbility.h"
#include "LProjectGA_Dash.generated.h"

/**
 * Dash: launches the avatar along its movement/facing direction and grants a brief
 * invulnerability window (i-frames) via TAG_State_Invulnerable. The project's first GAS ability.
 */
UCLASS()
class LPROJECT_API ULProjectGA_Dash : public ULProjectGameplayAbility
{
	GENERATED_BODY()

public:
	ULProjectGA_Dash();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	    const FGameplayAbilityActorInfo* ActorInfo,
	    const FGameplayAbilityActivationInfo ActivationInfo,
	    const FGameplayEventData* TriggerEventData) override;

protected:
	/** Removes i-frames and ends the ability. */
	void FinishDash();

	/** Launch impulse magnitude along the dash direction. */
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashStrength = 1500.0f;

	/** Duration of the dash / i-frame window in seconds. */
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashDuration = 0.25f;

	FTimerHandle DashTimerHandle;
};
