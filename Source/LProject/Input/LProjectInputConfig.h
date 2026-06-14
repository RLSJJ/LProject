// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "LProjectInputConfig.generated.h"

class UInputAction;

/** Pairs a UInputAction with the gameplay InputTag it represents. */
USTRUCT(BlueprintType)
struct FLProjectInputAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/**
 * Data-driven input mapping. Native actions are bound to functions (move/look); ability actions
 * are bound to ability activation by tag. Adding an input becomes data, not character code.
 */
UCLASS()
class LPROJECT_API ULProjectInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag) const;
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag) const;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (TitleProperty = "InputTag"))
	TArray<FLProjectInputAction> NativeInputActions;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (TitleProperty = "InputTag"))
	TArray<FLProjectInputAction> AbilityInputActions;
};
