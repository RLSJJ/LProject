// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Telegraph/LProjectTelegraphActor.h"
#include "LProjectBossPatternData.generated.h"

/** Where a pattern's telegraph/strike is anchored. */
UENUM(BlueprintType)
enum class ELProjectTelegraphTarget : uint8
{
	/** Centered on the player (dodge-the-circle). */
	PlayerLocation,
	/** Centered on the boss (PBAoE / slam). */
	SelfLocation,
	/** In front of the boss, offset by StrikeOffset.X (frontal cleave / line). */
	SelfForward
};

/**
 * One tunable boss attack. Pure data — no art, no code. The runner reads these to spawn a telegraph
 * and then strike. AoESize follows the ATelegraphActor convention: Circle -> X = radius;
 * Box -> (X,Y) = half-extents; Cone -> X = length, Y = half-angle (deg).
 */
USTRUCT(BlueprintType)
struct FLProjectBossAttackPattern
{
	GENERATED_BODY()

	/** Identifier (for logs/HUD/telemetry). */
	UPROPERTY(EditDefaultsOnly, Category = "Pattern")
	FGameplayTag PatternId;

	UPROPERTY(EditDefaultsOnly, Category = "Pattern|Shape")
	ELProjectTelegraphShape Shape = ELProjectTelegraphShape::Circle;

	UPROPERTY(EditDefaultsOnly, Category = "Pattern|Shape")
	FVector AoESize = FVector(350.0f, 0.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Pattern|Shape")
	ELProjectTelegraphTarget TargetMode = ELProjectTelegraphTarget::PlayerLocation;

	/** Forward offset (X) applied for SelfForward targeting. */
	UPROPERTY(EditDefaultsOnly, Category = "Pattern|Shape")
	FVector StrikeOffset = FVector(450.0f, 0.0f, 0.0f);

	/** Warning time before the hit lands (the player's window to dodge). */
	UPROPERTY(EditDefaultsOnly, Category = "Pattern|Timing")
	float TelegraphDuration = 1.5f;

	/** Active-hit window length. */
	UPROPERTY(EditDefaultsOnly, Category = "Pattern|Timing")
	float StrikeDuration = 0.2f;

	/** Recovery before the next pattern can start. */
	UPROPERTY(EditDefaultsOnly, Category = "Pattern|Timing")
	float RecoveryDuration = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Pattern|Damage")
	float Damage = 30.0f;

	/** Stagger this pattern deals (only relevant if it can hit a stagger-bearing target). */
	UPROPERTY(EditDefaultsOnly, Category = "Pattern|Damage")
	float StaggerDamage = 0.0f;

	/** Opens a counter window for its telegraph duration (player can interrupt with the counter). */
	UPROPERTY(EditDefaultsOnly, Category = "Pattern")
	bool bCounterable = false;

	/** Launch strength applied to hit targets, away from the strike center (0 = no knockback). */
	UPROPERTY(EditDefaultsOnly, Category = "Pattern|Mechanic")
	float KnockbackStrength = 0.0f;

	/** On strike, the boss itself lunges forward by this strength (a gap-closer / charge that relocates
	 *  it across the arena). 0 = the boss stays put. */
	UPROPERTY(EditDefaultsOnly, Category = "Pattern|Mechanic")
	float ChargeStrength = 0.0f;

	/**
	 * Inverts the danger: the telegraphed shape is the SAFE zone — targets OUTSIDE it are hit instead of
	 * those inside (a stand-in / safe-spot mechanic, the opposite of dodge-the-circle).
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Pattern|Mechanic")
	bool bSafeInside = false;

	/** Relative weight in random selection. */
	UPROPERTY(EditDefaultsOnly, Category = "Pattern")
	float SelectionWeight = 1.0f;

	/** Only selectable while the encounter's active phase tags contain all of these (empty = any phase). */
	UPROPERTY(EditDefaultsOnly, Category = "Pattern")
	FGameplayTagContainer RequiredPhaseTags;

	/** Cheap identity used for anti-repeat selection (distinguishes patterns by shape/target/timing/size). */
	uint32 MakeSignature() const
	{
		uint32 H = GetTypeHash(static_cast<uint8>(Shape));
		H = HashCombine(H, GetTypeHash(static_cast<uint8>(TargetMode)));
		H = HashCombine(H, GetTypeHash(FMath::RoundToInt(TelegraphDuration * 10.0f)));
		H = HashCombine(H, GetTypeHash(FMath::RoundToInt(Damage)));
		H = HashCombine(H, GetTypeHash(FMath::RoundToInt(AoESize.X)));
		return H;
	}
};

/**
 * The boss's whole attack library. Filterable per phase; the runner picks weighted-random from the
 * patterns valid for the current phase. This is the data-driven pattern asset the design calls for.
 */
UCLASS()
class LPROJECT_API ULProjectBossPatternData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Patterns", meta = (TitleProperty = "PatternId"))
	TArray<FLProjectBossAttackPattern> Patterns;

	/**
	 * Weighted-random pattern valid for the given phase tags. Returns nullptr if none qualify.
	 * Uses the supplied stream so boss behaviour is reproducible for demos/replays.
	 */
	const FLProjectBossAttackPattern* SelectPattern(const FGameplayTagContainer& ActivePhaseTags,
	    FRandomStream& Stream) const;
};
