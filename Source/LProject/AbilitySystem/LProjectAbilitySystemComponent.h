// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "LProjectAbilitySystemComponent.generated.h"

/**
 * Project ability system component.
 *
 * Adds tag-driven input activation: input actions carry an InputTag, granted ability specs are
 * tagged with the same InputTag, and these methods activate the matching ability. Future home for
 * raid-mechanic helpers (counter windows, stagger application). Shared by player and boss.
 */
UCLASS(ClassGroup = (LProject), meta = (BlueprintSpawnableComponent))
class LPROJECT_API ULProjectAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	/** Activates every granted ability whose spec carries the given input tag. */
	void AbilityInputTagPressed(const FGameplayTag& InputTag);

	/** Reserved for charge/channel abilities; currently a no-op (no held abilities yet). */
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
};
