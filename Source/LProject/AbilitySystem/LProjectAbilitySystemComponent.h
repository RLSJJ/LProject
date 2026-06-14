// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "LProjectAbilitySystemComponent.generated.h"

/**
 * Project ability system component.
 *
 * Skeleton for now. Future home for: ability input binding, gameplay-tag-driven state queries,
 * and raid-mechanic helpers (counter windows, stagger application). Shared by player and boss.
 */
UCLASS(ClassGroup = (LProject), meta = (BlueprintSpawnableComponent))
class LPROJECT_API ULProjectAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
};
