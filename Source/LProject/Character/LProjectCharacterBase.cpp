// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/LProjectCharacterBase.h"

#include "AbilitySystem/LProjectAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/LProjectAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Feedback/LProjectHitReactComponent.h"

ALProjectCharacterBase::ALProjectCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	AbilitySystemComponent = CreateDefaultSubobject<ULProjectAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	// Single-player: cheapest mode. Switch to Mixed if ever extended to multiplayer with an owned ASC.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	// Created as a subobject of the ASC owner -> auto-registered with the ASC at init time.
	AttributeSet = CreateDefaultSubobject<ULProjectAttributeSet>(TEXT("AttributeSet"));

	// Asset-free hit reaction (mesh squash punch); triggered by the CombatFeedback subsystem on hits.
	HitReact = CreateDefaultSubobject<ULProjectHitReactComponent>(TEXT("HitReact"));
}

UAbilitySystemComponent* ALProjectCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

float ALProjectCharacterBase::GetHealth() const
{
	return AttributeSet ? AttributeSet->GetHealth() : 0.0f;
}

float ALProjectCharacterBase::GetMaxHealth() const
{
	return AttributeSet ? AttributeSet->GetMaxHealth() : 0.0f;
}

bool ALProjectCharacterBase::IsAlive() const
{
	return GetHealth() > 0.0f;
}

void ALProjectCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitAbilityActorInfo();
}

void ALProjectCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	InitAbilityActorInfo();
}

void ALProjectCharacterBase::InitAbilityActorInfo()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

bool ALProjectCharacterBase::ConfigureTestVisualMesh(USkeletalMesh* VisualMesh,
    float TargetHeightCm,
    const FRotator& MeshRotation)
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!VisualMesh || !MeshComp)
	{
		return false;
	}

	MeshComp->SetSkeletalMesh(VisualMesh);

	// Auto-fit: scale so the mesh's height matches TargetHeightCm (glTF imports come in at arbitrary
	// scale), then drop it so its feet sit at the capsule bottom.
	const FBoxSphereBounds Bounds = VisualMesh->GetBounds();
	const float FullHeight = FMath::Max(Bounds.BoxExtent.Z * 2.0f, 1.0f);
	const float FitScale = TargetHeightCm / FullHeight;
	MeshComp->SetRelativeScale3D(FVector(FitScale));

	const float CapsuleHalf = GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 90.0f;
	const float BottomLocalZ = Bounds.Origin.Z - Bounds.BoxExtent.Z;
	MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -CapsuleHalf - BottomLocalZ * FitScale));
	MeshComp->SetRelativeRotation(MeshRotation);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	return true;
}
