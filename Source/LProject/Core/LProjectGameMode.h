// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LProjectGameMode.generated.h"

/**
 * Default game mode: wires the player character and controller. Encounter-specific game modes
 * (raid arena) will subclass this later.
 */
UCLASS()
class LPROJECT_API ALProjectGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALProjectGameMode();
};
