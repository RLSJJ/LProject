// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Boss/LProjectBossPatternData.h"
#include "LProjectBossPatternRunnerComponent.generated.h"

class ALProjectBossCharacter;
class ALProjectTelegraphActor;
class UGameplayEffect;

UENUM()
enum class ELProjectBossRunnerState : uint8
{
	Idle,
	Telegraph,
	Strike,
	Recovery
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLProjectCounterWindowEvent, bool, bIsOpen);

/**
 * The boss attack FSM. Idle -> Telegraph -> Strike -> Recovery: it picks a phase-valid pattern from the
 * pattern data, spawns an ATelegraphActor for the warning, then overlaps at strike time and applies the
 * shared damage GameplayEffect through the unified GAS pipeline. Pauses on groggy/death; counterable
 * patterns open a window the player's counter can interrupt. Falls back to built-in default patterns so
 * the boss attacks with zero authored assets.
 */
UCLASS(ClassGroup = (LProject), meta = (BlueprintSpawnableComponent))
class LPROJECT_API ULProjectBossPatternRunnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULProjectBossPatternRunnerComponent();

	virtual void
	TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Freeze/unfreeze the FSM (used by groggy and death). */
	void SetPaused(bool bInPaused);

	/** Enrage: scales the whole attack cadence (idle gap + telegraph/strike/recovery) down so the boss
	 *  attacks faster and dodge windows tighten. Set by the EncounterDirector on soft-enrage. */
	void SetEnraged(bool bInEnraged);

	/** Phase tags the EncounterDirector pushes; gate which patterns are selectable. */
	void SetActivePhaseTags(const FGameplayTagContainer& InPhaseTags)
	{
		ActivePhaseTags = InPhaseTags;
	}

	/** Swap the active pattern library (e.g. a phase-specific moveset). Null keeps the default. */
	void SetPatternData(ULProjectBossPatternData* InData)
	{
		PatternData = InData;
	}

	/** Cancel the in-flight pattern (counter success / groggy): destroys the telegraph, drops to recovery. */
	void InterruptCurrentPattern();

	bool IsCounterWindowOpen() const
	{
		return bCounterWindowOpen;
	}

	/** True while the boss is committed to a pattern (telegraphing/striking/recovering) — it should not
	 *  reposition during these. False only in Idle, when the boss is free to walk/reposition. */
	bool IsBusy() const
	{
		return State != ELProjectBossRunnerState::Idle;
	}

	bool IsTelegraphing() const
	{
		return State == ELProjectBossRunnerState::Telegraph;
	}

	bool IsStriking() const
	{
		return State == ELProjectBossRunnerState::Strike;
	}

	/** 0..1 progress through the current telegraph (for body windup animation). 0 outside Telegraph. */
	float GetTelegraphAlpha() const
	{
		if (State != ELProjectBossRunnerState::Telegraph || !bHasCurrent)
		{
			return 0.0f;
		}
		const float Dur = FMath::Max(CurrentPattern.TelegraphDuration * CadenceScale, KINDA_SMALL_NUMBER);
		return FMath::Clamp(PhaseTimer / Dur, 0.0f, 1.0f);
	}

	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FLProjectCounterWindowEvent OnCounterWindowChanged;

protected:
	virtual void BeginPlay() override;

	void EnterIdle();
	void StartPattern();
	void ExecuteStrike();
	/** True if a world location lies inside the current pattern's telegraph shape (circle/box/cone). */
	bool IsLocationInStrikeShape(const FVector& Loc) const;
	void SpawnTelegraph();
	void OpenCounterWindow();
	void CloseCounterWindow();
	FTransform ComputeStrikeTransform(const FLProjectBossAttackPattern& Pattern) const;
	const ULProjectBossPatternData* GetActivePatternData() const;
	void BuildDefaultPatternData();

	/** Authored pattern library. If null the runner builds a default set in BeginPlay. */
	UPROPERTY(EditAnywhere, Category = "Boss")
	TObjectPtr<ULProjectBossPatternData> PatternData;

	/** Damage effect every strike applies (defaults to ULProjectGE_Damage). */
	UPROPERTY(EditDefaultsOnly, Category = "Boss")
	TSubclassOf<UGameplayEffect> DamageEffect;

	/** Idle gap between patterns. */
	UPROPERTY(EditAnywhere, Category = "Boss")
	float IdleDelay = 1.2f;

	/** Cadence multiplier applied to all phase timings while enraged (<1 = faster, tighter dodges). */
	UPROPERTY(EditAnywhere, Category = "Boss", meta = (ClampMin = "0.2", ClampMax = "1.0"))
	float EnrageCadenceScale = 0.6f;

	/** Seed for reproducible pattern selection. */
	UPROPERTY(EditAnywhere, Category = "Boss")
	int32 RandomSeed = 1337;

	/** Draw the strike overlap volume for tuning. */
	UPROPERTY(EditAnywhere, Category = "Boss")
	bool bDrawDebugStrike = false;

private:
	UPROPERTY(Transient)
	TObjectPtr<ULProjectBossPatternData> DefaultPatternData;

	TWeakObjectPtr<ALProjectBossCharacter> Boss;
	TWeakObjectPtr<ALProjectTelegraphActor> ActiveTelegraph;

	ELProjectBossRunnerState State = ELProjectBossRunnerState::Idle;
	FLProjectBossAttackPattern CurrentPattern;
	bool bHasCurrent = false;
	float PhaseTimer = 0.0f;
	bool bPaused = false;
	bool bCounterWindowOpen = false;
	float CadenceScale = 1.0f;

	// Anti-repeat: signature of the last selected pattern so we don't pick it twice in a row.
	uint32 LastPatternSignature = 0;
	bool bHasLastSignature = false;

	FVector StrikeLocation = FVector::ZeroVector;
	FRotator StrikeRotation = FRotator::ZeroRotator;
	FGameplayTagContainer ActivePhaseTags;
	FRandomStream RandStream;
};
