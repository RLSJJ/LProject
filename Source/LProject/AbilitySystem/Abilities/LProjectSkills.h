// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LProjectGA_Skill.h"
#include "LProjectSkills.generated.h"

class UGameplayEffect;

/** Q — gap-closer: i-frame lunge toward the cursor + a light hit on arrival. */
UCLASS()
class LPROJECT_API ULProjectGA_Charge : public ULProjectGA_Skill
{
	GENERATED_BODY()

public:
	ULProjectGA_Charge();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	    const FGameplayAbilityActorInfo* ActorInfo,
	    const FGameplayAbilityActivationInfo ActivationInfo,
	    const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	float LaunchStrength = 1900.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	float Damage = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	float StaggerDamage = 120.0f;
};

/** W — heavy front cleave: the groggy-window damage dump (no i-frames). */
UCLASS()
class LPROJECT_API ULProjectGA_Cleave : public ULProjectGA_Skill
{
	GENERATED_BODY()

public:
	ULProjectGA_Cleave();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	    const FGameplayAbilityActorInfo* ActorInfo,
	    const FGameplayAbilityActivationInfo ActivationInfo,
	    const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	float Damage = 70.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	float StaggerDamage = 400.0f;
};

/** E — ranged bolt: a line of damage toward the cursor (mobile poke). */
UCLASS()
class LPROJECT_API ULProjectGA_Bolt : public ULProjectGA_Skill
{
	GENERATED_BODY()

public:
	ULProjectGA_Bolt();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	    const FGameplayAbilityActorInfo* ActorInfo,
	    const FGameplayAbilityActivationInfo ActivationInfo,
	    const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	float Damage = 30.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	float StaggerDamage = 80.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	float Range = 1200.0f;
};

/** R — Awakening: at full Identity, an i-frame super-AoE burst + self attack buff, spends all Identity. */
UCLASS()
class LPROJECT_API ULProjectGA_Awakening : public ULProjectGA_Skill
{
	GENERATED_BODY()

public:
	ULProjectGA_Awakening();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	    const FGameplayAbilityActorInfo* ActorInfo,
	    const FGameplayAbilityActivationInfo ActivationInfo,
	    const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	float Damage = 200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	float StaggerDamage = 800.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	float Radius = 750.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	TSubclassOf<UGameplayEffect> SelfBuff;
};
