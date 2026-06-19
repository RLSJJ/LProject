// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LProjectDamageNumberActor.generated.h"

class UTextRenderComponent;

/**
 * A short-lived floating combat number. World-space UTextRenderComponent (no UMG/asset dependency):
 * pops in, drifts up with a little horizontal scatter, billboards to the camera, then shrinks out and
 * self-destructs. Spawned by the CombatFeedback subsystem on every resolved hit. Colour/size encode the
 * hit (normal / heavy / groggy-crit). Asset-free so it works in a greybox build and reads clearly.
 */
UCLASS()
class LPROJECT_API ALProjectDamageNumberActor : public AActor
{
	GENERATED_BODY()

public:
	ALProjectDamageNumberActor();

	virtual void Tick(float DeltaSeconds) override;

	/** Configure and start the rise/fade. Call right after spawn. */
	void Init(float Amount, FLinearColor Color, float WorldSize, const FVector& InVelocity);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Feedback")
	TObjectPtr<UTextRenderComponent> Text;

	/** Total lifetime in seconds. */
	UPROPERTY(EditDefaultsOnly, Category = "Feedback")
	float LifeTime = 0.85f;

private:
	float Age = 0.0f;
	float BaseSize = 64.0f;
	FVector Velocity = FVector::ZeroVector;
};
