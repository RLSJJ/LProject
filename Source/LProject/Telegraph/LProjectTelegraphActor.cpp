// Copyright Epic Games, Inc. All Rights Reserved.

#include "Telegraph/LProjectTelegraphActor.h"

#include "DrawDebugHelpers.h"

ALProjectTelegraphActor::ALProjectTelegraphActor()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorEnableCollision(false);

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void ALProjectTelegraphActor::InitTelegraph(ELProjectTelegraphShape InShape,
    const FVector& InSize,
    float InDuration,
    const FVector& InLocation,
    const FRotator& InRotation)
{
	Shape = InShape;
	Size = InSize;
	Duration = FMath::Max(InDuration, 0.05f);
	Elapsed = 0.0f;
	SetActorLocationAndRotation(InLocation, InRotation);
	bInitialized = true;
}

void ALProjectTelegraphActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bInitialized)
	{
		return;
	}

	Elapsed += DeltaSeconds;
	const float Fill = FMath::Clamp(Elapsed / Duration, 0.0f, 1.0f);
	DrawTelegraph(Fill);

	// The strike lands when the fill completes; the runner handles damage, so we just clean up.
	if (Elapsed >= Duration)
	{
		Destroy();
	}
}

void ALProjectTelegraphActor::DrawTelegraph(float Fill) const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector Loc = GetActorLocation();
	// Fill colour ramps yellow -> red as the strike approaches.
	const FColor FillColor(255, static_cast<uint8>(255.0f * (1.0f - Fill)), 0);

	switch (Shape)
	{
	case ELProjectTelegraphShape::Circle:
	{
		const float Radius = Size.X;
		DrawDebugCylinder(World, Loc, Loc + FVector(0, 0, 16), Radius, 32, BorderColor, false, -1.0f, 0, 4.0f);
		if (Fill > 0.0f)
		{
			DrawDebugCylinder(World, Loc, Loc + FVector(0, 0, 12), Radius * Fill, 32, FillColor, false, -1.0f, 0, 2.0f);
		}
		break;
	}
	case ELProjectTelegraphShape::Box:
	{
		const FQuat Rot = GetActorQuat();
		DrawDebugBox(World, Loc, FVector(Size.X, Size.Y, 16), Rot, BorderColor, false, -1.0f, 0, 4.0f);
		if (Fill > 0.0f)
		{
			DrawDebugBox(World, Loc, FVector(Size.X * Fill, Size.Y, 12), Rot, FillColor, false, -1.0f, 0, 2.0f);
		}
		break;
	}
	case ELProjectTelegraphShape::Cone:
	{
		const float Length = Size.X;
		const float HalfAngle = FMath::DegreesToRadians(Size.Y);
		const FVector Dir = GetActorForwardVector();
		DrawDebugCone(World, Loc, Dir, Length, HalfAngle, HalfAngle, 24, BorderColor, false, -1.0f, 0, 4.0f);
		if (Fill > 0.0f)
		{
			DrawDebugCone(World, Loc, Dir, Length * Fill, HalfAngle, HalfAngle, 24, FillColor, false, -1.0f, 0, 2.0f);
		}
		break;
	}
	}
}
