// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "LProjectEncounterDirector.generated.h"

class ALProjectBossCharacter;
class ALProjectCharacterBase;
class ULProjectBossPatternData;

/** One HP-gated phase: when boss Health% drops to/below the threshold, this phase's tag becomes active. */
USTRUCT(BlueprintType)
struct FLProjectEncounterPhase
{
	GENERATED_BODY()

	/** Boss Health% (0-1) at/below which this phase activates. List in descending order. */
	UPROPERTY(EditAnywhere, Category = "Phase")
	float HealthPctThreshold = 1.0f;

	/** Phase tag pushed to the pattern runner (gates which patterns are selectable). */
	UPROPERTY(EditAnywhere, Category = "Phase", meta = (Categories = "Phase"))
	FGameplayTag PhaseTag;

	/** Optional pattern-library swap when this phase begins. */
	UPROPERTY(EditAnywhere, Category = "Phase")
	TObjectPtr<ULProjectBossPatternData> PhasePatterns;
};

/** Current outcome of the encounter (polled by the HUD). */
UENUM(BlueprintType)
enum class ELProjectEncounterOutcome : uint8
{
	InProgress,
	Won,
	Lost
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLProjectEncounterStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLProjectPhaseChanged, FGameplayTag, PhaseTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLProjectEncounterEnded, bool, bWon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLProjectEnrageTick, float, SecondsRemaining);

/**
 * Owns the solo-raid encounter flow. Registers the boss + player, watches boss Health% to fire phase
 * gates (swapping the active phase tags the pattern runner consumes), runs the enrage DPS-check timer,
 * and resolves win (boss dead) / lose (player dead or enrage) / retry (reset both, re-arm). Broadcasts
 * delegates the raid HUD + flow subsystem bind to. A world subsystem so any system can reach it via GetSubsystem.
 */
UCLASS()
class LPROJECT_API ULProjectEncounterDirector : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem / UWorldSubsystem
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override
	{
		return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
	}
	//~ End USubsystem / UWorldSubsystem

	//~ Begin FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	//~ End FTickableGameObject

	void RegisterBoss(ALProjectBossCharacter* InBoss);
	void RegisterPlayer(ALProjectCharacterBase* InPlayer);

	UFUNCTION(BlueprintCallable, Category = "Encounter")
	void StartEncounter();

	UFUNCTION(BlueprintCallable, Category = "Encounter")
	void RetryEncounter();

	/** Stop a live encounter without resolving win/lose (e.g. the player backs out to the Title menu). */
	UFUNCTION(BlueprintCallable, Category = "Encounter")
	void AbortEncounter();

	UFUNCTION(BlueprintPure, Category = "Encounter")
	bool IsEnraged() const
	{
		return bEnraged;
	}

	UFUNCTION(BlueprintPure, Category = "Encounter")
	bool IsEncounterActive() const
	{
		return bEncounterActive;
	}

	UFUNCTION(BlueprintPure, Category = "Encounter")
	float GetEnrageSecondsRemaining() const
	{
		return EnrageSecondsRemaining;
	}

	UFUNCTION(BlueprintPure, Category = "Encounter")
	FGameplayTag GetActivePhaseTag() const
	{
		return ActivePhaseTag;
	}

	UFUNCTION(BlueprintPure, Category = "Encounter")
	ELProjectEncounterOutcome GetOutcome() const
	{
		return Outcome;
	}

	const FGameplayTagContainer& GetActivePhaseTags() const
	{
		return ActivePhaseTags;
	}

	ALProjectBossCharacter* GetBoss() const;
	ALProjectCharacterBase* GetPlayer() const;

	// --- Events the HUD/other systems bind to ---
	UPROPERTY(BlueprintAssignable, Category = "Encounter")
	FLProjectEncounterStarted OnEncounterStarted;

	UPROPERTY(BlueprintAssignable, Category = "Encounter")
	FLProjectPhaseChanged OnPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Encounter")
	FLProjectEncounterEnded OnEncounterEnded;

	UPROPERTY(BlueprintAssignable, Category = "Encounter")
	FLProjectEnrageTick OnEnrageTick;

protected:
	void EnterPhase(int32 PhaseIndex);
	void EndEncounter(bool bWon);
	void BuildDefaultPhases();

	/** Soft-enrage: while the clock is under SoftEnrageSeconds the boss gains the Enraged buff (faster,
	 *  harder). At 0 the hard enrage wipes the player. Makes the timer a real escalation, not just a wall. */
	void ApplySoftEnrage();

	/** Total time before the enrage hard-wipes the player. */
	UPROPERTY(EditDefaultsOnly, Category = "Encounter")
	float EnrageDuration = 300.0f;

	/** Seconds remaining at which the boss enters soft-enrage (gains the Enraged buff). */
	UPROPERTY(EditDefaultsOnly, Category = "Encounter")
	float SoftEnrageSeconds = 45.0f;

	/** Duration of the untargetable roar when crossing into a new phase. */
	UPROPERTY(EditDefaultsOnly, Category = "Encounter")
	float PhaseTransitionDuration = 1.6f;

	/** Buff applied to the boss on soft-enrage (defaults to ULProjectGE_AttackUp). */
	UPROPERTY(EditDefaultsOnly, Category = "Encounter")
	TSubclassOf<class UGameplayEffect> EnrageBuffEffect;

	/** HP-gated phases (descending threshold). Built with defaults if left empty. */
	UPROPERTY(EditDefaultsOnly, Category = "Encounter")
	TArray<FLProjectEncounterPhase> Phases;

private:
	TWeakObjectPtr<ALProjectBossCharacter> Boss;
	TWeakObjectPtr<ALProjectCharacterBase> Player;

	bool bEncounterActive = false;
	bool bEnraged = false;
	ELProjectEncounterOutcome Outcome = ELProjectEncounterOutcome::InProgress;
	int32 CurrentPhaseIndex = -1;
	float EnrageSecondsRemaining = 0.0f;
	int32 LastBroadcastEnrageSecond = -1;

	FGameplayTag ActivePhaseTag;
	FGameplayTagContainer ActivePhaseTags;
};
