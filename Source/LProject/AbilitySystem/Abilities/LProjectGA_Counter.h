// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LProjectGameplayAbility.h"
#include "LProjectGA_Counter.generated.h"

class UGameplayEffect;

/**
 * Player counter/parry. Only succeeds while the boss is broadcasting a counterable window
 * (State.Boss.Counterable) and the player is in range: it interrupts the boss's pattern, bursts its
 * stagger gauge (fast-tracking groggy), applies a bleed DoT, rewards the player with an attack buff,
 * and grants brief i-frames. Off-window it just whiffs. The counter-window mechanic.
 */
UCLASS()
class LPROJECT_API ULProjectGA_Counter : public ULProjectGameplayAbility
{
	GENERATED_BODY()

public:
	ULProjectGA_Counter();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	    const FGameplayAbilityActorInfo* ActorInfo,
	    const FGameplayAbilityActivationInfo ActivationInfo,
	    const FGameplayEventData* TriggerEventData) override;

protected:
	/** Ends i-frames and the ability. */
	void FinishCounter();

	UPROPERTY(EditDefaultsOnly, Category = "Counter")
	float Range = 750.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Counter")
	float CounterDamage = 50.0f;

	/** Large stagger burst that fast-tracks the groggy window. */
	UPROPERTY(EditDefaultsOnly, Category = "Counter")
	float StaggerBurst = 1500.0f;

	/** Bleed magnitude per tick applied on a successful counter. */
	UPROPERTY(EditDefaultsOnly, Category = "Counter")
	float BleedPerTick = 15.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Counter")
	float IFrameDuration = 0.6f;

	UPROPERTY(EditDefaultsOnly, Category = "Counter")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Counter")
	TSubclassOf<UGameplayEffect> BleedEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Counter")
	TSubclassOf<UGameplayEffect> RewardBuff;

	FTimerHandle IFrameTimerHandle;
};
