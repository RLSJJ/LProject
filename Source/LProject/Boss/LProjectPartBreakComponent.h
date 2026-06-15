// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LProjectPartBreakComponent.generated.h"

class ALProjectBossCharacter;

/** A destructible boss part with its own durability pool. */
USTRUCT(BlueprintType)
struct FLProjectBossPart
{
	GENERATED_BODY()

	/** Identifier for HUD/loot/logs (e.g. "Head", "LeftArm"). */
	UPROPERTY(EditAnywhere, Category = "Part")
	FName PartId = NAME_None;

	/** Damage this part absorbs before breaking. */
	UPROPERTY(EditAnywhere, Category = "Part")
	float MaxDurability = 6000.0f;

	/** Boss Defense permanently removed when this part breaks (weakness). */
	UPROPERTY(EditAnywhere, Category = "Part")
	float DefenseReductionOnBreak = 20.0f;

	// Runtime.
	float Current = 0.0f;
	bool bBroken = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLProjectPartBrokenEvent, FName, PartId);

/**
 * Tracks per-part durability for a boss. Damage is fed in (the boss routes incoming hits here); when a
 * part's pool empties it breaks: the boss permanently loses Defense (a weakness window the player earns),
 * gains the PartBroken/DefenseDown tags for the HUD, and OnPartBroken fires for loot/feedback hooks.
 * Greybox: parts are data; an optional mesh can be hidden on break later.
 */
UCLASS(ClassGroup = (LProject), meta = (BlueprintSpawnableComponent))
class LPROJECT_API ULProjectPartBreakComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULProjectPartBreakComponent();

	/** Feed damage to the first unbroken part (greybox routing). */
	void ApplyDamageToPrimaryPart(float Amount);

	/** Feed damage to a specific part by id. */
	void ApplyPartDamage(FName PartId, float Amount);

	UFUNCTION(BlueprintPure, Category = "Boss")
	int32 GetPartCount() const
	{
		return Parts.Num();
	}

	UFUNCTION(BlueprintPure, Category = "Boss")
	int32 GetBrokenPartCount() const;

	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FLProjectPartBrokenEvent OnPartBroken;

protected:
	virtual void BeginPlay() override;

	void BreakPart(int32 PartIndex);

	/** Editable part list. If empty, a single default "Core" part is created at BeginPlay. */
	UPROPERTY(EditAnywhere, Category = "Boss", meta = (TitleProperty = "PartId"))
	TArray<FLProjectBossPart> Parts;

private:
	TWeakObjectPtr<ALProjectBossCharacter> Boss;
};
