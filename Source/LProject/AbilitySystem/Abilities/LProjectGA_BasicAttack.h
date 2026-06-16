// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LProjectGameplayAbility.h"
#include "LProjectGA_BasicAttack.generated.h"

class UGameplayEffect;

/**
 * Player left-click basic attack (평타). On activation it sweeps a box in front of the avatar (Pawn
 * object-type overlap — no custom collision channel needed), de-duplicates hits, and applies the
 * shared damage GameplayEffect with SetByCaller magnitudes to every combatant it finds. Greybox: an
 * instant trace on activation, no montage. One click = one swing (bound on the Started input event).
 */
UCLASS()
class LPROJECT_API ULProjectGA_BasicAttack : public ULProjectGameplayAbility
{
	GENERATED_BODY()

public:
	ULProjectGA_BasicAttack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	    const FGameplayAbilityActorInfo* ActorInfo,
	    const FGameplayAbilityActivationInfo ActivationInfo,
	    const FGameplayEventData* TriggerEventData) override;

protected:
	/** Distance in front of the avatar that the hit box is centered. */
	UPROPERTY(EditDefaultsOnly, Category = "BasicAttack")
	float Range = 150.0f;

	/** Half-size of the box swept for hits (X = reach, Y = width, Z = height). */
	UPROPERTY(EditDefaultsOnly, Category = "BasicAttack")
	FVector HitHalfExtent = FVector(90.0f, 110.0f, 100.0f);

	/** Base damage magnitude, before AttackPower/Defense scaling in the exec calc. */
	UPROPERTY(EditDefaultsOnly, Category = "BasicAttack")
	float BaseDamage = 25.0f;

	/** Stagger/무력화 dealt to a boss target on hit. */
	UPROPERTY(EditDefaultsOnly, Category = "BasicAttack")
	float StaggerDamage = 60.0f;

	/** Identity built per enemy hit (basic attack is the awakening-resource engine). */
	UPROPERTY(EditDefaultsOnly, Category = "BasicAttack")
	float IdentityPerHit = 250.0f;

	/** Damage effect to apply (defaults to ULProjectGE_Damage). */
	UPROPERTY(EditDefaultsOnly, Category = "BasicAttack")
	TSubclassOf<UGameplayEffect> DamageEffect;

	/** Draw the hit box for tuning. */
	UPROPERTY(EditDefaultsOnly, Category = "BasicAttack")
	bool bDrawDebugHit = false;
};
