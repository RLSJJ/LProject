// Copyright Epic Games, Inc. All Rights Reserved.

#include "Feedback/LProjectCameraShake_Hit.h"

ULProjectHitShakePattern::ULProjectHitShakePattern(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void ULProjectHitShakePattern::GetShakePatternInfoImpl(FCameraShakeInfo& OutInfo) const
{
	OutInfo.Duration = FCameraShakeDuration(Duration);
}

void ULProjectHitShakePattern::StartShakePatternImpl(const FCameraShakePatternStartParams& Params)
{
	Elapsed = 0.0f;
}

void ULProjectHitShakePattern::UpdateShakePatternImpl(const FCameraShakePatternUpdateParams& Params,
    FCameraShakePatternUpdateResult& OutResult)
{
	Elapsed += Params.DeltaTime;

	const float Alpha = Duration > 0.0f ? FMath::Clamp(1.0f - Elapsed / Duration, 0.0f, 1.0f) : 0.0f;
	const float Damp = Alpha * Alpha; // ease-out envelope
	const float Scale = Params.GetTotalScale() * Damp;
	const float W = Frequency * 2.0f * PI;

	OutResult.Rotation.Pitch = FMath::Sin(Elapsed * W) * RotAmplitude * Scale;
	OutResult.Rotation.Yaw = FMath::Sin(Elapsed * W * 1.7f + 1.3f) * RotAmplitude * Scale;
	OutResult.Rotation.Roll = FMath::Sin(Elapsed * W * 1.3f + 2.1f) * RotAmplitude * 0.5f * Scale;
	OutResult.Location.X = FMath::Sin(Elapsed * W * 1.1f) * LocAmplitude * Scale;
	OutResult.Location.Y = FMath::Sin(Elapsed * W * 1.5f + 0.7f) * LocAmplitude * Scale;
	OutResult.FOV = 0.0f;
}

bool ULProjectHitShakePattern::IsFinishedImpl() const
{
	return Elapsed >= Duration;
}

ULProjectCameraShake_Hit::ULProjectCameraShake_Hit(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
	ChangeRootShakePattern<ULProjectHitShakePattern>();
}
