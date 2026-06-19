// Copyright Epic Games, Inc. All Rights Reserved.

#include "Boss/LProjectBossPatternRunnerComponent.h"

#include "AbilitySystem/Effects/LProjectGE_Damage.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Boss/LProjectBossCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Core/LProjectGameplayTags.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Telegraph/LProjectTelegraphActor.h"

ULProjectBossPatternRunnerComponent::ULProjectBossPatternRunnerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	DamageEffect = ULProjectGE_Damage::StaticClass();
}

void ULProjectBossPatternRunnerComponent::BeginPlay()
{
	Super::BeginPlay();

	Boss = Cast<ALProjectBossCharacter>(GetOwner());
	RandStream.Initialize(RandomSeed);

	if (!PatternData)
	{
		BuildDefaultPatternData();
	}

	EnterIdle();
}

void ULProjectBossPatternRunnerComponent::TickComponent(float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ALProjectBossCharacter* B = Boss.Get();
	if (!B || bPaused || !B->IsAlive())
	{
		return;
	}

	PhaseTimer += DeltaTime;

	switch (State)
	{
	case ELProjectBossRunnerState::Idle:
		if (PhaseTimer >= IdleDelay * CadenceScale)
		{
			StartPattern();
		}
		break;

	case ELProjectBossRunnerState::Telegraph:
		if (bHasCurrent && PhaseTimer >= CurrentPattern.TelegraphDuration * CadenceScale)
		{
			CloseCounterWindow();
			ExecuteStrike();
			State = ELProjectBossRunnerState::Strike;
			PhaseTimer = 0.0f;
		}
		break;

	case ELProjectBossRunnerState::Strike:
		if (!bHasCurrent || PhaseTimer >= CurrentPattern.StrikeDuration * CadenceScale)
		{
			State = ELProjectBossRunnerState::Recovery;
			PhaseTimer = 0.0f;
		}
		break;

	case ELProjectBossRunnerState::Recovery:
		if (!bHasCurrent || PhaseTimer >= CurrentPattern.RecoveryDuration * CadenceScale)
		{
			EnterIdle();
		}
		break;
	}
}

void ULProjectBossPatternRunnerComponent::SetEnraged(bool bInEnraged)
{
	CadenceScale = bInEnraged ? EnrageCadenceScale : 1.0f;
}

void ULProjectBossPatternRunnerComponent::EnterIdle()
{
	State = ELProjectBossRunnerState::Idle;
	PhaseTimer = 0.0f;
	bHasCurrent = false;
}

void ULProjectBossPatternRunnerComponent::StartPattern()
{
	const ULProjectBossPatternData* Data = GetActivePatternData();
	const FLProjectBossAttackPattern* Picked = Data ? Data->SelectPattern(ActivePhaseTags, RandStream) : nullptr;
	if (!Picked)
	{
		EnterIdle();
		return;
	}

	CurrentPattern = *Picked;
	bHasCurrent = true;

	const FTransform StrikeXform = ComputeStrikeTransform(CurrentPattern);
	StrikeLocation = StrikeXform.GetLocation();
	StrikeRotation = StrikeXform.Rotator();

	SpawnTelegraph();
	if (CurrentPattern.bCounterable)
	{
		OpenCounterWindow();
	}

	State = ELProjectBossRunnerState::Telegraph;
	PhaseTimer = 0.0f;
}

void ULProjectBossPatternRunnerComponent::SpawnTelegraph()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ALProjectTelegraphActor* Telegraph =
	    World->SpawnActor<ALProjectTelegraphActor>(ALProjectTelegraphActor::StaticClass(),
	        StrikeLocation,
	        StrikeRotation,
	        SpawnParams);
	if (Telegraph)
	{
		Telegraph->InitTelegraph(CurrentPattern.Shape,
		    CurrentPattern.AoESize,
		    CurrentPattern.TelegraphDuration,
		    StrikeLocation,
		    StrikeRotation);
		ActiveTelegraph = Telegraph;
	}
}

void ULProjectBossPatternRunnerComponent::ExecuteStrike()
{
	UWorld* World = GetWorld();
	ALProjectBossCharacter* B = Boss.Get();
	UAbilitySystemComponent* SourceASC = B ? B->GetAbilitySystemComponent() : nullptr;
	if (!World || !B || !SourceASC || !DamageEffect)
	{
		return;
	}

	const FQuat Rotation = StrikeRotation.Quaternion();
	FCollisionShape Shape;
	switch (CurrentPattern.Shape)
	{
	case ELProjectTelegraphShape::Box:
		Shape = FCollisionShape::MakeBox(FVector(CurrentPattern.AoESize.X, CurrentPattern.AoESize.Y, 250.0f));
		break;
	case ELProjectTelegraphShape::Circle:
	case ELProjectTelegraphShape::Cone:
	default:
		Shape = FCollisionShape::MakeSphere(CurrentPattern.AoESize.X);
		break;
	}

	if (bDrawDebugStrike)
	{
		DrawDebugSphere(World,
		    StrikeLocation,
		    Shape.GetSphereRadius() > 0 ? Shape.GetSphereRadius() : 100.0f,
		    16,
		    FColor::Magenta,
		    false,
		    0.6f);
	}

	FCollisionObjectQueryParams ObjectParams(ECC_Pawn);
	FCollisionQueryParams QueryParams(FName(TEXT("LProjectBossStrike")), false, B);

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(Overlaps, StrikeLocation, Rotation, ObjectParams, Shape, QueryParams);

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(B);
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1.0f, Context);
	if (!SpecHandle.IsValid())
	{
		return;
	}
	SpecHandle.Data->SetSetByCallerMagnitude(TAG_SetByCaller_Damage, CurrentPattern.Damage);
	SpecHandle.Data->SetSetByCallerMagnitude(TAG_SetByCaller_StaggerDamage, CurrentPattern.StaggerDamage);

	// Cone targeting: keep only actors within the half-angle of the strike's forward direction.
	const bool bConeFilter = CurrentPattern.Shape == ELProjectTelegraphShape::Cone;
	const FVector Forward = StrikeRotation.Vector();
	const float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(CurrentPattern.AoESize.Y));

	TSet<AActor*> AlreadyHit;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || HitActor == B || AlreadyHit.Contains(HitActor))
		{
			continue;
		}

		if (bConeFilter)
		{
			FVector ToActor = HitActor->GetActorLocation() - StrikeLocation;
			ToActor.Z = 0.0f;
			if (!ToActor.IsNearlyZero() && FVector::DotProduct(ToActor.GetSafeNormal(), Forward) < CosHalfAngle)
			{
				continue;
			}
		}

		AlreadyHit.Add(HitActor);

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (!TargetASC || TargetASC->HasMatchingGameplayTag(TAG_State_Invulnerable))
		{
			continue; // i-frames (dash/counter) dodge the hit
		}
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data, TargetASC);
	}
}

void ULProjectBossPatternRunnerComponent::InterruptCurrentPattern()
{
	CloseCounterWindow();
	if (ALProjectTelegraphActor* Telegraph = ActiveTelegraph.Get())
	{
		Telegraph->Destroy();
	}
	ActiveTelegraph = nullptr;

	State = ELProjectBossRunnerState::Recovery;
	PhaseTimer = 0.0f;
}

void ULProjectBossPatternRunnerComponent::SetPaused(bool bInPaused)
{
	bPaused = bInPaused;
}

void ULProjectBossPatternRunnerComponent::OpenCounterWindow()
{
	if (bCounterWindowOpen)
	{
		return;
	}
	bCounterWindowOpen = true;
	if (ALProjectBossCharacter* B = Boss.Get())
	{
		if (UAbilitySystemComponent* ASC = B->GetAbilitySystemComponent())
		{
			ASC->AddLooseGameplayTag(TAG_State_Boss_Counterable);
		}
	}
	OnCounterWindowChanged.Broadcast(true);
}

void ULProjectBossPatternRunnerComponent::CloseCounterWindow()
{
	if (!bCounterWindowOpen)
	{
		return;
	}
	bCounterWindowOpen = false;
	if (ALProjectBossCharacter* B = Boss.Get())
	{
		if (UAbilitySystemComponent* ASC = B->GetAbilitySystemComponent())
		{
			ASC->RemoveLooseGameplayTag(TAG_State_Boss_Counterable);
		}
	}
	OnCounterWindowChanged.Broadcast(false);
}

FTransform ULProjectBossPatternRunnerComponent::ComputeStrikeTransform(const FLProjectBossAttackPattern& Pattern) const
{
	const ALProjectBossCharacter* B = Boss.Get();
	if (!B)
	{
		return FTransform::Identity;
	}

	const FVector BossLoc = B->GetActorLocation();
	const float BossFeetZ = BossLoc.Z - B->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FRotator Rot = B->GetActorRotation();
	FVector Loc = BossLoc;

	switch (Pattern.TargetMode)
	{
	case ELProjectTelegraphTarget::PlayerLocation:
		if (const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(B, 0))
		{
			Loc = PlayerPawn->GetActorLocation();
			if (const ACharacter* PlayerChar = Cast<ACharacter>(PlayerPawn))
			{
				Loc.Z -= PlayerChar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			}
			else
			{
				Loc.Z = BossFeetZ;
			}
		}
		else
		{
			Loc.Z = BossFeetZ;
		}
		break;

	case ELProjectTelegraphTarget::SelfForward:
		Loc = BossLoc + B->GetActorForwardVector() * Pattern.StrikeOffset.X;
		Loc.Z = BossFeetZ;
		break;

	case ELProjectTelegraphTarget::SelfLocation:
	default:
		Loc.Z = BossFeetZ;
		break;
	}

	return FTransform(Rot, Loc);
}

const ULProjectBossPatternData* ULProjectBossPatternRunnerComponent::GetActivePatternData() const
{
	return PatternData ? PatternData : DefaultPatternData;
}

void ULProjectBossPatternRunnerComponent::BuildDefaultPatternData()
{
	DefaultPatternData = NewObject<ULProjectBossPatternData>(this, TEXT("DefaultBossPatternData"));

	// 1) Dodge-the-circle under the player.
	FLProjectBossAttackPattern CircleOnPlayer;
	CircleOnPlayer.PatternId = TAG_Phase_1; // placeholder id; ids are for logs only
	CircleOnPlayer.Shape = ELProjectTelegraphShape::Circle;
	CircleOnPlayer.AoESize = FVector(400.0f, 0.0f, 0.0f);
	CircleOnPlayer.TargetMode = ELProjectTelegraphTarget::PlayerLocation;
	CircleOnPlayer.TelegraphDuration = 1.6f;
	CircleOnPlayer.Damage = 30.0f;
	CircleOnPlayer.SelectionWeight = 1.0f;
	DefaultPatternData->Patterns.Add(CircleOnPlayer);

	// 2) Frontal box cleave.
	FLProjectBossAttackPattern FrontalCleave;
	FrontalCleave.Shape = ELProjectTelegraphShape::Box;
	FrontalCleave.AoESize = FVector(500.0f, 250.0f, 0.0f);
	FrontalCleave.TargetMode = ELProjectTelegraphTarget::SelfForward;
	FrontalCleave.StrikeOffset = FVector(550.0f, 0.0f, 0.0f);
	FrontalCleave.TelegraphDuration = 1.4f;
	FrontalCleave.Damage = 35.0f;
	FrontalCleave.SelectionWeight = 1.0f;
	DefaultPatternData->Patterns.Add(FrontalCleave);

	// 3) Point-blank slam centered on the boss.
	FLProjectBossAttackPattern Slam;
	Slam.Shape = ELProjectTelegraphShape::Circle;
	Slam.AoESize = FVector(650.0f, 0.0f, 0.0f);
	Slam.TargetMode = ELProjectTelegraphTarget::SelfLocation;
	Slam.TelegraphDuration = 1.8f;
	Slam.Damage = 40.0f;
	Slam.SelectionWeight = 0.8f;
	DefaultPatternData->Patterns.Add(Slam);

	// 4) Counterable cone charge (player can interrupt with the counter).
	FLProjectBossAttackPattern CounterCone;
	CounterCone.Shape = ELProjectTelegraphShape::Cone;
	CounterCone.AoESize = FVector(700.0f, 35.0f, 0.0f); // length, half-angle (deg)
	CounterCone.TargetMode = ELProjectTelegraphTarget::SelfForward;
	CounterCone.StrikeOffset = FVector(0.0f, 0.0f, 0.0f);
	CounterCone.TelegraphDuration = 2.0f;
	CounterCone.Damage = 45.0f;
	CounterCone.bCounterable = true;
	CounterCone.SelectionWeight = 0.7f;
	DefaultPatternData->Patterns.Add(CounterCone);
}
