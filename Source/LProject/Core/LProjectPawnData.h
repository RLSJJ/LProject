// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LProjectPawnData.generated.h"

class ULProjectAbilitySet;
class ULProjectInputConfig;
class UInputMappingContext;

/**
 * Defines a pawn archetype as data: its abilities, its input config, and its key bindings.
 * The player and each boss reference one PawnData, so new archetypes are content, not code.
 */
UCLASS()
class LPROJECT_API ULProjectPawnData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Abilities + attribute sets granted to this pawn. */
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TObjectPtr<ULProjectAbilitySet> AbilitySet;

	/** Maps input actions to gameplay tags (native + ability inputs). */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<ULProjectInputConfig> InputConfig;

	/** Key -> action bindings added to the Enhanced Input subsystem on possession. */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
};
