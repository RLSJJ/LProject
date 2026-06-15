// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LProjectTelegraphActor.generated.h"

/** AoE telegraph footprint. Size semantics differ per shape (see ATelegraphActor::InitTelegraph). */
UENUM(BlueprintType)
enum class ELProjectTelegraphShape : uint8
{
	Circle,
	Box,
	Cone
};

/**
 * Greybox AoE warning. Purely visual: it draws an animated danger footprint (debug shapes, so it needs
 * zero art assets and is Substrate/Lumen-proof) whose inner fill grows from 0 to the full radius over
 * the warning duration, signalling exactly when the strike lands. The pattern runner does the actual
 * damage overlap at strike time, so this actor never touches gameplay. Self-destroys when it expires.
 *
 * Size convention (FVector): Circle -> X = radius; Box -> (X,Y) = half-extents; Cone -> X = length,
 * Y = half-angle in degrees. Z is unused.
 */
UCLASS()
class LPROJECT_API ALProjectTelegraphActor : public AActor
{
	GENERATED_BODY()

public:
	ALProjectTelegraphActor();

	virtual void Tick(float DeltaSeconds) override;

	/** Configure and start the telegraph. Call right after spawning. */
	void InitTelegraph(ELProjectTelegraphShape InShape,
	    const FVector& InSize,
	    float InDuration,
	    const FVector& InLocation,
	    const FRotator& InRotation);

protected:
	void DrawTelegraph(float Fill) const;

	UPROPERTY(VisibleAnywhere, Category = "Telegraph")
	ELProjectTelegraphShape Shape = ELProjectTelegraphShape::Circle;

	/** Shape-dependent dimensions (see class comment). */
	UPROPERTY(VisibleAnywhere, Category = "Telegraph")
	FVector Size = FVector(300.0f, 0.0f, 0.0f);

	UPROPERTY(VisibleAnywhere, Category = "Telegraph")
	float Duration = 1.5f;

	/** Outer ring colour (constant warning border). */
	UPROPERTY(EditDefaultsOnly, Category = "Telegraph")
	FColor BorderColor = FColor(255, 200, 0);

	float Elapsed = 0.0f;
	bool bInitialized = false;
};
