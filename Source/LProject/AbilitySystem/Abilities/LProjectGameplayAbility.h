// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "LProjectGameplayAbility.generated.h"

/**
 * Base class for all LProject abilities. Sets project-wide instancing/replication defaults.
 */
UCLASS(Abstract)
class LPROJECT_API ULProjectGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	ULProjectGameplayAbility();
};
