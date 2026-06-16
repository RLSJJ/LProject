// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LProjectGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "LProjectGA_Skill.generated.h"

class UGameplayEffect;

/**
 * Base for the QWER skill kit. Wires the shared GAS cooldown (one ULProjectGE_Cooldown carrying each
 * skill's CooldownTag + SetByCaller duration, so the HUD can read remaining time by tag), an Identity
 * build-and-spend hook, and a reusable shape-damage helper that routes through the one SetByCaller
 * damage GE. Subclasses set CooldownTag/CooldownDuration and implement their own ActivateAbility.
 */
UCLASS(Abstract)
class LPROJECT_API ULProjectGA_Skill : public ULProjectGameplayAbility
{
	GENERATED_BODY()

public:
	ULProjectGA_Skill();

	virtual const FGameplayTagContainer* GetCooldownTags() const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	    const FGameplayAbilityActorInfo* ActorInfo,
	    const FGameplayAbilityActivationInfo ActivationInfo) const override;

protected:
	/** Overlap-damage a shape, applying the damage GE to every enemy ASC hit. Returns enemies hit. */
	int32 ApplyShapeDamage(const FVector& Center,
	    const FQuat& Rotation,
	    const FCollisionShape& Shape,
	    float Damage,
	    float StaggerDamage,
	    bool bConeFilter = false,
	    float ConeHalfAngleDeg = 45.0f);

	/** Add (or, with a negative value, spend) Identity, clamped to [0, IdentityMax]. */
	void GainIdentity(float Amount);

	float GetIdentity() const;
	float GetIdentityMax() const;

	/** Grant i-frames (State.Invulnerable) now and end the ability after Duration, removing them. */
	void GrantIFramesAndEndLater(float Duration);

	/** Aim direction (toward the cursor on the ground), falling back to the avatar's facing. */
	FVector GetAimDirection() const;

	void FinishSkill();

	/** Cooldown tag this skill grants while on cooldown (set by subclass ctor). */
	UPROPERTY(EditDefaultsOnly, Category = "Skill", meta = (Categories = "Cooldown"))
	FGameplayTag CooldownTag;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	float CooldownDuration = 5.0f;

	/** Identity gained when the skill lands / activates. */
	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	float IdentityGain = 600.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	bool bDrawDebug = false;

private:
	mutable FGameplayTagContainer MutableCooldownTags;
	FTimerHandle SkillTimerHandle;
};
