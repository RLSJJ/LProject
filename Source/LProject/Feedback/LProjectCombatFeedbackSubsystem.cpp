// Copyright Epic Games, Inc. All Rights Reserved.

#include "Feedback/LProjectCombatFeedbackSubsystem.h"

#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "Feedback/LProjectCameraShake_Hit.h"
#include "Feedback/LProjectDamageNumberActor.h"
#include "Kismet/GameplayStatics.h"

void ULProjectCombatFeedbackSubsystem::Tick(float DeltaTime)
{
	// Restore time after a hit-stop on REAL time, since the dilation we set also slows world timers.
	if (bHitStopActive)
	{
		if (const UWorld* W = GetWorld())
		{
			if (W->GetRealTimeSeconds() >= HitStopRealEndTime)
			{
				bHitStopActive = false;
				UGameplayStatics::SetGlobalTimeDilation(W, 1.0f);
			}
		}
	}
}

TStatId ULProjectCombatFeedbackSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULProjectCombatFeedbackSubsystem, STATGROUP_Tickables);
}

void ULProjectCombatFeedbackSubsystem::ReportHit(AActor* TargetActor, float Damage, bool bTargetIsBoss, bool bGroggy)
{
	if (Damage <= 0.0f || !TargetActor)
	{
		return;
	}

	const bool bHeavy = Damage >= HeavyHitThreshold || bGroggy;

	// Floating number above the target.
	const float UpOffset = bTargetIsBoss ? 320.0f : 130.0f;
	const FVector SpawnLoc = TargetActor->GetActorLocation() + FVector(0.0f, 0.0f, UpOffset);
	SpawnDamageNumber(SpawnLoc, Damage, bTargetIsBoss, bGroggy);

	// Camera shake, scaled by how big the hit was (the player's camera always feels it).
	const float ShakeScale = FMath::Clamp(Damage / 45.0f, 0.35f, 3.0f) * (bGroggy ? 1.5f : 1.0f);
	PlayCameraShake(ShakeScale);

	// Hit-stop only on weighty hits so the fight punches without constant stutter.
	if (bHeavy)
	{
		RequestHitStop(bGroggy ? 0.04f : 0.06f, bGroggy ? 0.09f : 0.06f);
	}

	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, SpawnLoc);
	}
}

void ULProjectCombatFeedbackSubsystem::SpawnDamageNumber(const FVector& Location,
    float Damage,
    bool bTargetIsBoss,
    bool bGroggy)
{
	UWorld* W = GetWorld();
	if (!W)
	{
		return;
	}

	// Colour/size encode the hit: cyan groggy-crit, orange heavy, gold normal-on-boss, red on the player.
	FLinearColor Color;
	float WorldSize;
	if (!bTargetIsBoss)
	{
		Color = FLinearColor(1.0f, 0.25f, 0.2f); // damage to the player
		WorldSize = 52.0f;
	}
	else if (bGroggy)
	{
		Color = FLinearColor(0.35f, 0.9f, 1.0f);
		WorldSize = 96.0f;
	}
	else if (Damage >= HeavyHitThreshold)
	{
		Color = FLinearColor(1.0f, 0.55f, 0.1f);
		WorldSize = 82.0f;
	}
	else
	{
		Color = FLinearColor(1.0f, 0.86f, 0.4f);
		WorldSize = 64.0f;
	}

	const FVector Vel(FMath::FRandRange(-45.0f, 45.0f), FMath::FRandRange(-45.0f, 45.0f), 150.0f);
	const FVector Jitter(FMath::FRandRange(-20.0f, 20.0f), FMath::FRandRange(-20.0f, 20.0f), 0.0f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (ALProjectDamageNumberActor* Num =
	        W->SpawnActor<ALProjectDamageNumberActor>(ALProjectDamageNumberActor::StaticClass(),
	            Location + Jitter,
	            FRotator::ZeroRotator,
	            Params))
	{
		Num->Init(Damage, Color, WorldSize, Vel);
	}
}

void ULProjectCombatFeedbackSubsystem::PlayCameraShake(float Scale)
{
	const TSubclassOf<UCameraShakeBase> ShakeClass =
	    HitShakeClass ? HitShakeClass : TSubclassOf<UCameraShakeBase>(ULProjectCameraShake_Hit::StaticClass());
	if (!ShakeClass)
	{
		return;
	}
	if (APlayerCameraManager* Cam = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		Cam->StartCameraShake(ShakeClass, Scale);
	}
}

void ULProjectCombatFeedbackSubsystem::RequestHitStop(float TimeScale, float RealDuration)
{
	UWorld* W = GetWorld();
	if (!W)
	{
		return;
	}
	UGameplayStatics::SetGlobalTimeDilation(W, FMath::Clamp(TimeScale, 0.01f, 1.0f));
	HitStopRealEndTime = W->GetRealTimeSeconds() + RealDuration;
	bHitStopActive = true;
}
