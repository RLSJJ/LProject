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
 * Holds core vitals only. Specialized sets — Stagger/무력화, resource/identity, part durability —
 * will be added as separate UAttributeSet subclasses in later phases.
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
	//~ End UAttributeSet interface

	/** Current health. Reaches 0 -> dead. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Vitals")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(ULProjectAttributeSet, Health);

	/** Maximum health. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Vitals")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(ULProjectAttributeSet, MaxHealth);

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
};
