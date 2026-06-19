// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/LProjectGameMode.h"
#include "LProjectEncounterGameMode.generated.h"

class ALProjectBossCharacter;

/**
 * Encounter game mode for the raid arena. Subclasses the default game mode (so the player/controller
 * wiring is reused), then on BeginPlay spawns the boss in front of the player, registers boss + player
 * with the EncounterDirector, and starts the fight. Set this as the World Settings GameMode override (or
 * the GlobalDefaultGameMode) to drop straight into the boss encounter.
 */
UCLASS()
class LPROJECT_API ALProjectEncounterGameMode : public ALProjectGameMode
{
	GENERATED_BODY()

public:
	ALProjectEncounterGameMode();

protected:
	virtual void BeginPlay() override;

	/** Boss to spawn (defaults to ALProjectBossCharacter). */
	UPROPERTY(EditDefaultsOnly, Category = "Encounter")
	TSubclassOf<ALProjectBossCharacter> BossClass;

	/** Distance in front of the player to spawn the boss. */
	UPROPERTY(EditDefaultsOnly, Category = "Encounter")
	float BossSpawnDistance = 1600.0f;

	/** Dev fast-path: skip Title/Ready and drop straight into a live fight (also via cvar lp.SkipFrontEnd 1). */
	UPROPERTY(EditDefaultsOnly, Category = "Encounter")
	bool bAutoStartEncounter = false;
};
