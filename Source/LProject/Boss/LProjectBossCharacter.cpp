// Copyright Epic Games, Inc. All Rights Reserved.

#include "Boss/LProjectBossCharacter.h"

#include "AbilitySystem/Attributes/LProjectAttributeSet.h"
#include "AbilitySystem/Attributes/LProjectBossAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimSequence.h"
#include "Boss/LProjectBossPatternRunnerComponent.h"
#include "Boss/LProjectPartBreakComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/LProjectGameplayTags.h"
#include "Engine/SkeletalMesh.h"
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

	// Test visual: imported Fox skeletal mesh, scaled up to a beast, with Survey/Walk/Run animations.
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FoxMesh(
	    TEXT("/Game/TestVisual/Fox/Fox/SkeletalMeshes/Fox.Fox"));
	if (FoxMesh.Succeeded())
	{
		VisualMesh = FoxMesh.Object;
	}
	static ConstructorHelpers::FObjectFinder<UAnimSequence> FoxIdle(
	    TEXT("/Game/TestVisual/Fox/Fox/SkeletalMeshes/FoxSurvey.FoxSurvey"));
	if (FoxIdle.Succeeded())
	{
		IdleAnim = FoxIdle.Object;
	}
	static ConstructorHelpers::FObjectFinder<UAnimSequence> FoxWalk(
	    TEXT("/Game/TestVisual/Fox/Fox/SkeletalMeshes/FoxWalk.FoxWalk"));
	if (FoxWalk.Succeeded())
	{
		WalkAnim = FoxWalk.Object;
	}
	static ConstructorHelpers::FObjectFinder<UAnimSequence> FoxRun(
	    TEXT("/Game/TestVisual/Fox/Fox/SkeletalMeshes/FoxRun.FoxRun"));
	if (FoxRun.Succeeded())
	{
		RunAnim = FoxRun.Object;
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

	ApplyTestVisual();

	if (HasAuthority())
	{
		GrantBossKit();
	}
}

void ALProjectBossCharacter::ApplyTestVisual()
{
	if (!ConfigureTestVisualMesh(VisualMesh, VisualTargetHeight, FRotator(0.0f, VisualMeshYaw, 0.0f)))
	{
		return;
	}

	if (DevVisualMesh)
	{
		DevVisualMesh->SetVisibility(false);
	}
	if (GetMesh())
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		CurrentAnim = IdleAnim;
		if (CurrentAnim)
		{
			GetMesh()->PlayAnimation(CurrentAnim, true);
		}
	}
}

void ALProjectBossCharacter::UpdateLocomotionAnim()
{
	if (!GetMesh())
	{
		return;
	}

	const float Speed = GetVelocity().Size2D();
	UAnimSequence* Desired = IdleAnim;
	if (Speed > 300.0f && RunAnim)
	{
		Desired = RunAnim;
	}
	else if (Speed > 10.0f && WalkAnim)
	{
		Desired = WalkAnim;
	}

	if (Desired && Desired != CurrentAnim)
	{
		CurrentAnim = Desired;
		GetMesh()->PlayAnimation(Desired, true);
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

	// Death edge: tear down any in-flight pattern + groggy timer so nothing outlives the corpse
	// (orphaned telegraph drawing, a pending ExitGroggy un-pausing a dead boss).
	if (Data.NewValue <= 0.0f)
	{
		GetWorldTimerManager().ClearTimer(GroggyTimerHandle);
		bGroggy = false;
		if (PatternRunner)
		{
			PatternRunner->InterruptCurrentPattern();
			PatternRunner->SetPaused(true);
		}
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

	// If the boss died during the groggy window, leave it down — don't refill/resume a corpse.
	if (!IsAlive())
	{
		OnGroggyEnd.Broadcast();
		return;
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

	UpdateLocomotionAnim();

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
