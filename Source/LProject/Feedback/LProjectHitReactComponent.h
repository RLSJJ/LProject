// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LProjectHitReactComponent.generated.h"

class USkeletalMeshComponent;

/**
 * A brief, asset-free hit reaction: when the owner takes a hit, its visual mesh does a quick squash-and-
 * stretch punch (wider + shorter, easing back) so impacts read on the body even without authored hit-react
 * montages. Captures the mesh's fitted base scale lazily on the first hit, then animates a multiplicative
 * envelope on top for ~0.18s. The CombatFeedback subsystem triggers it on every resolved hit.
 */
UCLASS(ClassGroup = (LProject), meta = (BlueprintSpawnableComponent))
class LPROJECT_API ULProjectHitReactComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULProjectHitReactComponent();

	virtual void
	TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Start a hit-react punch. Intensity ~ [0.5, 2] scales how hard the squash reads. */
	void PlayHitReact(float Intensity);

protected:
	/** Squash duration in seconds. */
	UPROPERTY(EditDefaultsOnly, Category = "HitReact")
	float Duration = 0.18f;

	/** Peak horizontal stretch fraction at Intensity 1 (e.g. 0.18 = +18% wide). */
	UPROPERTY(EditDefaultsOnly, Category = "HitReact")
	float SquashAmount = 0.18f;

private:
	USkeletalMeshComponent* GetMesh() const;

	bool bBaseCaptured = false;
	FVector BaseScale = FVector::OneVector;

	bool bPlaying = false;
	float Elapsed = 0.0f;
	float CurrentIntensity = 1.0f;
};
