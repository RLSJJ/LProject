// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LProjectCombatFeedbackSubsystem.generated.h"

class UCameraShakeBase;
class USoundBase;
class ALProjectDamageNumberActor;

/**
 * The single "juice" seam for combat. Every resolved hit funnels through ReportHit() (called from the
 * AttributeSet's PostGameplayEffectExecute, the one damage seam), and this subsystem fans it out into the
 * feel layer: a scaled camera shake, brief hit-stop on heavy/groggy hits, and a floating damage number.
 * Hooks (HitSound / shake class) are data so real assets drop in later. A tickable world subsystem so it
 * can restore hit-stop on REAL time (unaffected by the time dilation it sets).
 */
UCLASS()
class LPROJECT_API ULProjectCombatFeedbackSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	//~ End FTickableGameObject

	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override
	{
		return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
	}

	/**
	 * Report a resolved hit so the feel layer can react.
	 * @param TargetActor   who took the damage (number spawns above them)
	 * @param Damage        final HP removed
	 * @param bTargetIsBoss the player struck the boss (vs the player being hit) — drives colour/intensity
	 * @param bGroggy       the target is groggy (crit-feel burst)
	 */
	void ReportHit(AActor* TargetActor, float Damage, bool bTargetIsBoss, bool bGroggy);

	/** Briefly slow time for impact weight; restored on real time after RealDuration. */
	void RequestHitStop(float TimeScale, float RealDuration);

protected:
	void SpawnDamageNumber(const FVector& Location, float Damage, bool bTargetIsBoss, bool bGroggy);
	void PlayCameraShake(float Scale);

	/** Combat impact shake (defaults to ULProjectCameraShake_Hit). */
	UPROPERTY(EditDefaultsOnly, Category = "Feedback")
	TSubclassOf<UCameraShakeBase> HitShakeClass;

	/** Optional impact SFX hook (null-safe; real audio drops in here later). */
	UPROPERTY(EditDefaultsOnly, Category = "Feedback")
	TObjectPtr<USoundBase> HitSound;

	/** Damage at/above which a hit also triggers hit-stop + a bigger shake. */
	UPROPERTY(EditDefaultsOnly, Category = "Feedback")
	float HeavyHitThreshold = 120.0f;

private:
	bool bHitStopActive = false;
	double HitStopRealEndTime = 0.0;
};
