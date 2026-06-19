// Copyright Epic Games, Inc. All Rights Reserved.

#include "Encounter/LProjectEncounterGameMode.h"

#include "Boss/LProjectBossCharacter.h"
#include "Boss/LProjectBossPatternRunnerComponent.h"
#include "Character/LProjectCharacterBase.h"
#include "Encounter/LProjectEncounterDirector.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Flow/LProjectGameFlowSubsystem.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"

static TAutoConsoleVariable<int32> CVarSkipFrontEnd(TEXT("lp.SkipFrontEnd"),
    0,
    TEXT("1 = skip Title/Ready and drop straight into a live fight."),
    ECVF_Default);

ALProjectEncounterGameMode::ALProjectEncounterGameMode()
{
	BossClass = ALProjectBossCharacter::StaticClass();
	// In-fight readout is the UMG ULProjectRaidHUD, shown by the flow subsystem during the Encounter state.
}

void ALProjectEncounterGameMode::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);

	// Place the boss in front of the player, facing back toward them; lift it so it settles on the ground.
	FVector SpawnLocation(BossSpawnDistance, 0.0f, 200.0f);
	FRotator SpawnRotation = FRotator::ZeroRotator;
	if (PlayerPawn)
	{
		const FVector PlayerLoc = PlayerPawn->GetActorLocation();
		SpawnLocation = PlayerLoc + PlayerPawn->GetActorForwardVector() * BossSpawnDistance;
		SpawnLocation.Z = PlayerLoc.Z + 150.0f;
		SpawnRotation = (PlayerLoc - SpawnLocation).Rotation();
		SpawnRotation.Pitch = 0.0f;
		SpawnRotation.Roll = 0.0f;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (!BossClass)
	{
		BossClass = ALProjectBossCharacter::StaticClass();
	}
	ALProjectBossCharacter* SpawnedBoss =
	    World->SpawnActor<ALProjectBossCharacter>(BossClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (ULProjectEncounterDirector* Director = World->GetSubsystem<ULProjectEncounterDirector>())
	{
		Director->RegisterBoss(SpawnedBoss);
		Director->RegisterPlayer(Cast<ALProjectCharacterBase>(PlayerPawn));
		// NOTE: do NOT StartEncounter here — the flow owner starts the fight when the player enters the raid.
	}

	// Freeze the boss so it cannot act behind the menus.
	if (SpawnedBoss && SpawnedBoss->GetPatternRunner())
	{
		SpawnedBoss->GetPatternRunner()->SetPaused(true);
	}

	// Hand off first-frame control to the flow owner (Title -> Ready -> Encounter), or skip in dev.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (ULProjectGameFlowSubsystem* Flow = GI->GetSubsystem<ULProjectGameFlowSubsystem>())
		{
			const bool bSkip = bAutoStartEncounter || CVarSkipFrontEnd.GetValueOnGameThread() != 0;
			if (bSkip)
			{
				Flow->EnterRaid();
			}
			else
			{
				Flow->BeginFlow();
			}
		}
	}
}

void ALProjectEncounterGameMode::RetryEncounter()
{
	if (UWorld* World = GetWorld())
	{
		if (ULProjectEncounterDirector* Director = World->GetSubsystem<ULProjectEncounterDirector>())
		{
			Director->RetryEncounter();
		}
	}
}
