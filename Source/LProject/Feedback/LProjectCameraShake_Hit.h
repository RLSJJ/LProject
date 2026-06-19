// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "LProjectCameraShake_Hit.generated.h"

/**
 * Self-contained decaying-oscillation shake pattern (no EngineCameras plugin dependency). A few sines on
 * rotation + location that damp to zero over Duration. Amplitude is multiplied by the shake's total scale,
 * so the CombatFeedback subsystem can drive light taps to heavy groggy bursts from one class.
 */
UCLASS()
class ULProjectHitShakePattern : public UCameraShakePattern
{
	GENERATED_BODY()

public:
	ULProjectHitShakePattern(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Category = "Shake")
	float Duration = 0.28f;

	/** Peak rotational shake in degrees (pitch/yaw). */
	UPROPERTY(EditAnywhere, Category = "Shake")
	float RotAmplitude = 1.6f;

	/** Peak positional shake in cm. */
	UPROPERTY(EditAnywhere, Category = "Shake")
	float LocAmplitude = 6.0f;

	/** Oscillation frequency in Hz. */
	UPROPERTY(EditAnywhere, Category = "Shake")
	float Frequency = 22.0f;

protected:
	virtual void GetShakePatternInfoImpl(FCameraShakeInfo& OutInfo) const override;
	virtual void StartShakePatternImpl(const FCameraShakePatternStartParams& Params) override;
	virtual void UpdateShakePatternImpl(const FCameraShakePatternUpdateParams& Params,
	    FCameraShakePatternUpdateResult& OutResult) override;
	virtual bool IsFinishedImpl() const override;

private:
	float Elapsed = 0.0f;
};

/** Combat impact camera shake. Scale per hit via StartCameraShake's Scale parameter. */
UCLASS()
class LPROJECT_API ULProjectCameraShake_Hit : public UCameraShakeBase
{
	GENERATED_BODY()

public:
	ULProjectCameraShake_Hit(const FObjectInitializer& ObjectInitializer);
};
