// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LProjectGameFlowSubsystem.generated.h"

class UUserWidget;
class APlayerController;
class ULProjectEncounterDirector;

/** Front-to-back game flow states (launch -> title -> ready -> fight -> result). */
UENUM(BlueprintType)
enum class ELProjectGameFlowState : uint8
{
	Boot,
	Title,
	Ready,
	Encounter,
	ResultVictory,
	ResultDefeat
};

/** Per-run summary shown on the result screens. */
USTRUCT(BlueprintType)
struct FLProjectRunStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float ClearTimeSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 PhaseReached = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float BossHealthPctAtEnd = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 PartsBroken = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 Attempts = 1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLProjectFlowStateChanged, ELProjectGameFlowState, NewState);

/**
 * The single flow owner. A GameInstance subsystem (survives world reset) running an explicit state
 * machine and the ONLY caller of EncounterDirector Start/Retry. It swaps the full-screen menu widget per
 * state, applies the per-state input/cursor/pause contract, and drives Encounter->Result off the
 * director's OnEncounterEnded. The encounter GameMode hands control here instead of auto-starting.
 */
UCLASS()
class LPROJECT_API ULProjectGameFlowSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Called by the encounter GameMode once the boss is spawned, registered, and frozen. */
	void BeginFlow();

	void RequestState(ELProjectGameFlowState NewState);

	UFUNCTION(BlueprintPure, Category = "Flow")
	ELProjectGameFlowState GetState() const
	{
		return CurrentState;
	}

	/** Title -> Ready. */
	UFUNCTION(BlueprintCallable, Category = "Flow")
	void GoToReady();

	/** Ready -> Encounter (resets combatants and starts the fight). */
	UFUNCTION(BlueprintCallable, Category = "Flow")
	void EnterRaid();

	/** Result -> Encounter (reset + restart). Also the dev R-key path. */
	UFUNCTION(BlueprintCallable, Category = "Flow")
	void Retry();

	/** Result/Ready -> Title. */
	UFUNCTION(BlueprintCallable, Category = "Flow")
	void ReturnToTitle();

	UFUNCTION(BlueprintCallable, Category = "Flow")
	void QuitGame();

	UFUNCTION(BlueprintPure, Category = "Flow")
	const FLProjectRunStats& GetRunStats() const
	{
		return RunStats;
	}

	UPROPERTY(BlueprintAssignable, Category = "Flow")
	FLProjectFlowStateChanged OnFlowStateChanged;

protected:
	void ShowScreenForState(ELProjectGameFlowState State);
	void ApplyPresentation(bool bUIOnly, bool bShowCursor, bool bPaused);
	void StartOrResetEncounter();
	void CaptureRunStats();

	APlayerController* GetPC() const;
	ULProjectEncounterDirector* GetDirector() const;

	UFUNCTION()
	void HandleEncounterEnded(bool bWon);

	ELProjectGameFlowState CurrentState = ELProjectGameFlowState::Boot;

	UPROPERTY()
	TObjectPtr<UUserWidget> ActiveScreen;

	FLProjectRunStats RunStats;
	double EncounterStartTime = 0.0;
	bool bBoundEnded = false;
};
