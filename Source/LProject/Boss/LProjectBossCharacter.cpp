// Copyright Epic Games, Inc. All Rights Reserved.

#include "Boss/LProjectBossCharacter.h"

#include "AbilitySystem/Attributes/LProjectAttributeSet.h"
#include "AbilitySystem/Attributes/LProjectBossAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Boss/LProjectBossPatternRunnerComponent.h"
#include "Boss/LProjectPartBreakComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/LProjectGameplayTags.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ALProjectBossCharacter::ALProjectBossCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Large greybox footprint so the player has to walk up and patterns have room to read.
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->InitCapsuleSize(140.0f, 220.0f);
	}

	// Boss body: a big engine cube fitted to the capsule (collision stays on the capsule).
	DevVisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DevVisualMesh"));
	DevVisualMesh->SetupAttachment(GetCapsuleComponent());
	DevVisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DevVisualMesh->SetRelativeScale3D(FVector(2.8f, 2.8f, 4.4f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		DevVisualMesh->SetStaticMesh(CubeMesh.Object);
	}

	// Drives all boss attacks (telegraph -> strike) through the shared GAS pipeline.
	PatternRunner = CreateDefaultSubobject<ULProjectBossPatternRunnerComponent>(TEXT("PatternRunner"));

	// Tracks per-part durability; breaking a part weakens the boss.
	PartBreak = CreateDefaultSubobject<ULProjectPartBreakComponent>(TEXT("PartBreak"));

	// Camera/controller never rotate the boss; it faces the player manually in Tick.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = false;
	}
}

void ALProjectBossCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GrantBossKit();
	}
}

void ALProjectBossCharacter::GrantBossKit()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// Boss-only stagger attributes, granted on top of the base vitals set.
	BossAttributeSet = NewObject<ULProjectBossAttributeSet>(this);
	ASC->AddAttributeSetSubobject(BossAttributeSet.Get());

	// Initialise base vitals/combat (MaxHealth before Health so the Health clamp uses the new max).
	ASC->SetNumericAttributeBase(ULProjectAttributeSet::GetMaxHealthAttribute(), MaxHealthTotal);
	ASC->SetNumericAttributeBase(ULProjectAttributeSet::GetHealthAttribute(), MaxHealthTotal);
	ASC->SetNumericAttributeBase(ULProjectAttributeSet::GetAttackPowerAttribute(), BossAttackPower);
	ASC->SetNumericAttributeBase(ULProjectAttributeSet::GetDefenseAttribute(), BossDefense);

	// Initialise stagger (StaggerMax before StaggerCurrent for the same reason).
	ASC->SetNumericAttributeBase(ULProjectBossAttributeSet::GetStaggerMaxAttribute(), StaggerMaxValue);
	ASC->SetNumericAttributeBase(ULProjectBossAttributeSet::GetStaggerCurrentAttribute(), StaggerMaxValue);

	// Drive groggy off the stagger gauge reaching 0.
	ASC->GetGameplayAttributeValueChangeDelegate(ULProjectBossAttributeSet::GetStaggerCurrentAttribute())
	    .AddUObject(this, &ALProjectBossCharacter::OnStaggerChanged);

	// Route damage taken into the part-break component.
	ASC->GetGameplayAttributeValueChangeDelegate(ULProjectAttributeSet::GetHealthAttribute())
	    .AddUObject(this, &ALProjectBossCharacter::OnHealthChanged);
}

void ALProjectBossCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	// Only damage (Health decreases) feeds part durability; healing/refills are ignored.
	const float DamageTaken = Data.OldValue - Data.NewValue;
	if (DamageTaken > 0.0f && PartBreak)
	{
		PartBreak->ApplyDamageToPrimaryPart(DamageTaken);
	}
}

void ALProjectBossCharacter::OnStaggerChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue <= 0.0f && !bGroggy && IsAlive())
	{
		EnterGroggy();
	}
}

void ALProjectBossCharacter::EnterGroggy()
{
	if (bGroggy)
	{
		return;
	}
	bGroggy = true;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->AddLooseGameplayTag(TAG_State_Boss_Groggy);
	}

	// Stop attacking: cancel any in-flight pattern, then freeze the runner for the window.
	if (PatternRunner)
	{
		PatternRunner->InterruptCurrentPattern();
		PatternRunner->SetPaused(true);
	}
	OnGroggyStart.Broadcast();

	GetWorldTimerManager().SetTimer(GroggyTimerHandle,
	    this,
	    &ALProjectBossCharacter::ExitGroggy,
	    GroggyDuration,
	    false);
}

void ALProjectBossCharacter::ExitGroggy()
{
	if (!bGroggy)
	{
		return;
	}
	bGroggy = false;
	GetWorldTimerManager().ClearTimer(GroggyTimerHandle);

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->RemoveLooseGameplayTag(TAG_State_Boss_Groggy);
		// Refill the stagger gauge for the next groggy cycle.
		if (BossAttributeSet)
		{
			ASC->SetNumericAttributeBase(ULProjectBossAttributeSet::GetStaggerCurrentAttribute(),
			    BossAttributeSet->GetStaggerMax());
		}
	}

	// Resume attacking.
	if (PatternRunner)
	{
		PatternRunner->SetPaused(false);
	}
	OnGroggyEnd.Broadcast();
}

float ALProjectBossCharacter::GetMaxHealthPerBar() const
{
	return GetMaxHealth() / static_cast<float>(GetHealthBarCount());
}

void ALProjectBossCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Face the player (yaw only); stop once dead.
	if (!IsAlive())
	{
		return;
	}

	if (const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
		ToPlayer.Z = 0.0f;
		if (!ToPlayer.IsNearlyZero())
		{
			const FRotator Target(0.0f, ToPlayer.Rotation().Yaw, 0.0f);
			SetActorRotation(FMath::RInterpConstantTo(GetActorRotation(), Target, DeltaSeconds, FacingInterpSpeed));
		}
	}
}
