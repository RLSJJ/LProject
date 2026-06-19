// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/LProjectCharacterBase.h"
#include "LProjectBossCharacter.generated.h"

class UStaticMeshComponent;
class USkeletalMesh;
class UAnimSequence;
class ULProjectBossAttributeSet;
class ULProjectBossPatternRunnerComponent;
class ULProjectPartBreakComponent;
class ULProjectPawnData;
struct FOnAttributeChangeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLProjectBossSimpleEvent);

/**
 * Greybox raid boss. Subclasses ALProjectCharacterBase so it reuses the ASC, vitals, and combat
 * interfaces; adds a large box body, the boss stagger attribute set, multi-bar HP helpers, and the
 * groggy/무력화 state. Movement is AI-less: it faces the player each tick; all attacks come from the
 * pattern runner component (attached in a later layer). Spawned and wired by the EncounterDirector.
 */
UCLASS()
class LPROJECT_API ALProjectBossCharacter : public ALProjectCharacterBase
{
	GENERATED_BODY()

public:
	ALProjectBossCharacter();

	virtual void Tick(float DeltaSeconds) override;

	/** Number of HP segments the HUD draws across the boss's single MaxHealth pool. */
	UFUNCTION(BlueprintPure, Category = "Boss")
	int32 GetHealthBarCount() const
	{
		return FMath::Max(HealthBarCount, 1);
	}

	/** Max health represented by a single HP bar. */
	UFUNCTION(BlueprintPure, Category = "Boss")
	float GetMaxHealthPerBar() const;

	/** True while the boss is groggy/무력화 (stagger depleted). */
	UFUNCTION(BlueprintPure, Category = "Boss")
	bool IsGroggy() const
	{
		return bGroggy;
	}

	const ULProjectBossAttributeSet* GetBossAttributeSet() const
	{
		return BossAttributeSet;
	}

	ULProjectBossPatternRunnerComponent* GetPatternRunner() const
	{
		return PatternRunner;
	}

	ULProjectPartBreakComponent* GetPartBreak() const
	{
		return PartBreak;
	}

	/** Force the boss into / out of the groggy window (also called when stagger hits 0). */
	void EnterGroggy();
	void ExitGroggy();

	//~ Begin ILProjectCombatant
	/** Player landed a counter: interrupt the in-flight pattern (decoupled via the combatant interface). */
	virtual void NotifyCountered() override;
	//~ End ILProjectCombatant

	/** Dramatic phase change: the boss roars (rears up), goes briefly untargetable, and pauses its
	 *  attacks before escalating — a real phase-transition "moment", not just a HUD label flip. */
	void EnterPhaseTransition(float Duration);

	bool IsInPhaseTransition() const
	{
		return bPhaseTransition;
	}

	/** Broadcast when the groggy window opens / closes (HUD + runner pause hooks). */
	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FLProjectBossSimpleEvent OnGroggyStart;

	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FLProjectBossSimpleEvent OnGroggyEnd;

protected:
	virtual void BeginPlay() override;

	/** Adds the boss attribute set and initialises vitals/stagger (authority only). */
	void GrantBossKit();

	/** Watches StaggerCurrent; triggers groggy when it reaches 0. */
	void OnStaggerChanged(const FOnAttributeChangeData& Data);

	/** Watches Health; routes the damage taken into the part-break component. */
	void OnHealthChanged(const FOnAttributeChangeData& Data);

	// --- Components ---
	/** Greybox body: an engine cube scaled up. Collision stays on the capsule. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UStaticMeshComponent> DevVisualMesh;

	/** Drives data-driven attack patterns (Idle -> Telegraph -> Strike -> Recovery). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
	TObjectPtr<ULProjectBossPatternRunnerComponent> PatternRunner;

	/** Tracks per-part durability; breaking a part weakens the boss. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
	TObjectPtr<ULProjectPartBreakComponent> PartBreak;

	// --- Tunables ---
	/** Total HP pool (split into HealthBarCount visual bars). */
	UPROPERTY(EditDefaultsOnly, Category = "Boss")
	float MaxHealthTotal = 100000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Boss")
	int32 HealthBarCount = 5;

	UPROPERTY(EditDefaultsOnly, Category = "Boss")
	float BossAttackPower = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Boss")
	float BossDefense = 50.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Stagger")
	float StaggerMaxValue = 3000.0f;

	/** Seconds the groggy window stays open before stagger refills. */
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Stagger")
	float GroggyDuration = 6.0f;

	/** Degrees/second the boss yaws to face the player. */
	UPROPERTY(EditDefaultsOnly, Category = "Boss")
	float FacingInterpSpeed = 180.0f;

	// --- Movement (the boss repositions between attacks; it is not a stationary turret) ---
	/** Ideal distance the boss tries to keep from the player while free (in Idle). */
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Movement")
	float PreferredRange = 480.0f;

	/** Hysteresis band around PreferredRange where the boss holds position (no fidgeting). */
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Movement")
	float RangeDeadzone = 160.0f;

	/** Boss walk speed (cm/s); also gates which locomotion anim plays. */
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Movement")
	float MoveSpeed = 360.0f;

	/** Updates chase/reposition movement toward/away from the player while free. */
	void UpdateMovement();

	/** Optional data-driven kit. If null the boss uses the code defaults above. */
	UPROPERTY(EditDefaultsOnly, Category = "Boss")
	TObjectPtr<ULProjectPawnData> BossPawnData;

private:
	UPROPERTY()
	TObjectPtr<ULProjectBossAttributeSet> BossAttributeSet;

	bool bGroggy = false;
	FTimerHandle GroggyTimerHandle;

	void ExitPhaseTransition();
	bool bPhaseTransition = false;
	FTimerHandle PhaseTransitionTimerHandle;

	// --- Test visual (giant Fox skeletal mesh + speed-driven anim; replaces the dev cube) ---
	void ApplyTestVisual();
	void UpdateLocomotionAnim();

	/**
	 * Procedural attack "tell" (no authored montages): the body rears UP as the telegraph builds, then
	 * SLAMS down on the strike, so the player can read the wind-up off the boss itself — not just the
	 * ground decal. Applied as a vertical offset on the mesh (orientation-independent, doesn't fight the
	 * scale-based hit-react). Driven each tick from the pattern runner's telegraph/strike state.
	 */
	void UpdateAttackTell(float DeltaSeconds);

	/** Peak rear-up height (cm) at full telegraph. */
	UPROPERTY(EditDefaultsOnly, Category = "Boss")
	float WindupRiseHeight = 80.0f;

	float MeshBaseRelZ = 0.0f;
	bool bMeshBaseCaptured = false;
	float CurrentTellOffsetZ = 0.0f;

	UPROPERTY()
	TObjectPtr<USkeletalMesh> VisualMesh;

	UPROPERTY()
	TObjectPtr<UAnimSequence> IdleAnim;

	UPROPERTY()
	TObjectPtr<UAnimSequence> WalkAnim;

	UPROPERTY()
	TObjectPtr<UAnimSequence> RunAnim;

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	float VisualTargetHeight = 700.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	float VisualMeshYaw = -90.0f;

	UPROPERTY()
	TObjectPtr<UAnimSequence> CurrentAnim;
};
