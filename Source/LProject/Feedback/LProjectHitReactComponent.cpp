// Copyright Epic Games, Inc. All Rights Reserved.

#include "Feedback/LProjectHitReactComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

ULProjectHitReactComponent::ULProjectHitReactComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // only ticks while a hit-react is playing
}

USkeletalMeshComponent* ULProjectHitReactComponent::GetMesh() const
{
	if (const ACharacter* Owner = Cast<ACharacter>(GetOwner()))
	{
		return Owner->GetMesh();
	}
	return nullptr;
}

void ULProjectHitReactComponent::PlayHitReact(float Intensity)
{
	USkeletalMeshComponent* Mesh = GetMesh();
	if (!Mesh)
	{
		return;
	}

	// Capture the fitted base scale once (after the test-visual fit has been applied in BeginPlay).
	if (!bBaseCaptured)
	{
		BaseScale = Mesh->GetRelativeScale3D();
		bBaseCaptured = true;
	}

	CurrentIntensity = FMath::Clamp(Intensity, 0.3f, 2.5f);
	Elapsed = 0.0f;
	bPlaying = true;
	SetComponentTickEnabled(true);
}

void ULProjectHitReactComponent::TickComponent(float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bPlaying)
	{
		return;
	}

	USkeletalMeshComponent* Mesh = GetMesh();
	if (!Mesh)
	{
		bPlaying = false;
		SetComponentTickEnabled(false);
		return;
	}

	Elapsed += DeltaTime;
	const float T = Duration > 0.0f ? FMath::Clamp(Elapsed / Duration, 0.0f, 1.0f) : 1.0f;

	if (T >= 1.0f)
	{
		Mesh->SetRelativeScale3D(BaseScale);
		bPlaying = false;
		SetComponentTickEnabled(false);
		return;
	}

	// Envelope: 0 -> 1 -> 0 over the duration (one squash). Squash = wider XY, shorter Z.
	const float Env = FMath::Sin(T * PI) * CurrentIntensity * SquashAmount;
	const FVector Mul(1.0f + Env, 1.0f + Env, 1.0f - Env * 0.7f);
	Mesh->SetRelativeScale3D(BaseScale * Mul);
}
