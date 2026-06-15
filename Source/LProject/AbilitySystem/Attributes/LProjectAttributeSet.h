// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "LProjectAttributeSet.generated.h"

// Canonical GAS accessor macro: generates GetXAttribute(), GetX(), SetX(), InitX() for an attribute.
// (The engine also ships ATTRIBUTE_ACCESSORS_BASIC with the same body; we define our own to match
// the widely-used GAS convention.)
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName)                                                                   \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)                                                         \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)                                                                       \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)                                                                       \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Base attribute set shared by all LProject combat actors (player avatar + raid boss).
 *
 * Holds core vitals (Health/MaxHealth) and combat primaries (AttackPower/Defense), plus a transient
 * Damage meta-attribute that the damage pipeline writes; PostGameplayEffectExecute turns that into a
 * Health subtraction. Boss-only attributes (Stagger/무력화) live in ULProjectBossAttributeSet.
 */
UCLASS()
class LPROJECT_API ULProjectAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	ULProjectAttributeSet();

	//~ Begin UAttributeSet interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	//~ End UAttributeSet interface

	/** Current health. Reaches 0 -> dead. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Vitals")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(ULProjectAttributeSet, Health);

	/** Maximum health. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Vitals")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(ULProjectAttributeSet, MaxHealth);

	/** Offensive scalar: the damage exec scales a hit's base magnitude by AttackPower/100. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackPower, Category = "Combat")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(ULProjectAttributeSet, AttackPower);

	/** Defensive scalar: the damage exec reduces incoming damage by up to 95% based on Defense/100. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Defense, Category = "Combat")
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(ULProjectAttributeSet, Defense);

	/**
	 * Transient incoming-damage meta-attribute. The damage exec writes the final computed damage here;
	 * PostGameplayEffectExecute subtracts it from Health and resets it to 0. NEVER replicated.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (HideFromModifiers))
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(ULProjectAttributeSet, Damage);

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_AttackPower(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Defense(const FGameplayAttributeData& OldValue);
};
