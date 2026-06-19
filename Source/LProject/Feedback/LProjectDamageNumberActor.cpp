// Copyright Epic Games, Inc. All Rights Reserved.

#include "Feedback/LProjectDamageNumberActor.h"

#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"

ALProjectDamageNumberActor::ALProjectDamageNumberActor()
{
	PrimaryActorTick.bCanEverTick = true;

	Text = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Text"));
	SetRootComponent(Text);
	Text->SetHorizontalAlignment(EHTA_Center);
	Text->SetVerticalAlignment(EVRTA_TextCenter);
	Text->SetWorldSize(BaseSize);
	// Numbers should read over geometry/FX, not be occluded by the boss body.
	Text->SetRenderCustomDepth(true);
	Text->bCastDynamicShadow = false;
	Text->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ALProjectDamageNumberActor::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeTime + 0.1f); // safety net if Tick is ever suppressed
}

void ALProjectDamageNumberActor::Init(float Amount, FLinearColor Color, float WorldSize, const FVector& InVelocity)
{
	BaseSize = WorldSize;
	Velocity = InVelocity;
	if (Text)
	{
		Text->SetText(FText::AsNumber(FMath::RoundToInt(Amount)));
		Text->SetTextRenderColor(Color.ToFColor(true));
		Text->SetWorldSize(0.01f); // starts tiny; pops in over the first frames
	}
}

void ALProjectDamageNumberActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	Age += DeltaSeconds;
	if (Age >= LifeTime)
	{
		Destroy();
		return;
	}

	// Drift up with a little decel + horizontal damping (a small pop arc).
	AddActorWorldOffset(Velocity * DeltaSeconds);
	Velocity.X *= FMath::Max(0.0f, 1.0f - DeltaSeconds * 3.0f);
	Velocity.Y *= FMath::Max(0.0f, 1.0f - DeltaSeconds * 3.0f);
	Velocity.Z -= 180.0f * DeltaSeconds; // gentle gravity so it arcs

	const float T = LifeTime > 0.0f ? Age / LifeTime : 1.0f;
	if (Text)
	{
		// Pop in over the first 12%, hold, then shrink out over the last 25%.
		float SizeScale;
		if (T < 0.12f)
		{
			SizeScale = FMath::InterpEaseOut(0.0f, 1.15f, T / 0.12f, 2.0f);
		}
		else if (T > 0.75f)
		{
			SizeScale = FMath::Lerp(1.0f, 0.0f, (T - 0.75f) / 0.25f);
		}
		else
		{
			SizeScale = FMath::Lerp(1.15f, 1.0f, (T - 0.12f) / 0.63f);
		}
		Text->SetWorldSize(BaseSize * FMath::Max(SizeScale, 0.0f));

		// Billboard: face the local camera so the number always reads.
		if (const APlayerCameraManager* Cam = UGameplayStatics::GetPlayerCameraManager(this, 0))
		{
			const FVector Dir = GetActorLocation() - Cam->GetCameraLocation();
			if (!Dir.IsNearlyZero())
			{
				Text->SetWorldRotation(Dir.Rotation());
			}
		}
	}
}
