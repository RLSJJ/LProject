// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "LProjectAbilitySet.generated.h"

class ULProjectGameplayAbility;
class UAttributeSet;
class UAbilitySystemComponent;

/** An ability to grant, plus the input tag that activates it. */
USTRUCT(BlueprintType)
struct FLProjectAbilitySet_GameplayAbility
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ULProjectGameplayAbility> Ability = nullptr;

	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;

	UPROPERTY(EditDefaultsOnly, meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/** An attribute set class to instantiate and grant. */
USTRUCT(BlueprintType)
struct FLProjectAbilitySet_AttributeSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAttributeSet> AttributeSet = nullptr;
};

/**
 * A grantable bundle of abilities + attribute sets. The player and each boss reference one,
 * so their kit is data, not hardcoded. Adding a mechanic = add an entry here.
 */
UCLASS()
class LPROJECT_API ULProjectAbilitySet : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Grants all contained attribute sets + abilities to the ASC (authority only). */
	void GiveToAbilitySystem(UAbilitySystemComponent* ASC, UObject* SourceObject = nullptr) const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Abilities", meta = (TitleProperty = "Ability"))
	TArray<FLProjectAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Attributes", meta = (TitleProperty = "AttributeSet"))
	TArray<FLProjectAbilitySet_AttributeSet> GrantedAttributes;
};
