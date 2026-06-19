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
	if (!Data)
	{
		EnterIdle();
		return;
	}

	// Anti-repeat: re-roll a few times so the boss doesn't slam the identical move back-to-back, which
	// reads as a random generator rather than an authored rotation.
	const FLProjectBossAttackPattern* Picked = nullptr;
	for (int32 Attempt = 0; Attempt < 4; ++Attempt)
	{
		Picked = Data->SelectPattern(ActivePhaseTags, RandStream);
		if (!Picked || !bHasLastSignature || Picked->MakeSignature() != LastPatternSignature)
		{
			break;
		}
	}
	if (!Picked)
	{
		EnterIdle();
		return;
	}

	CurrentPattern = *Picked;
	LastPatternSignature = Picked->MakeSignature();
	bHasLastSignature = true;
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
		    StrikeRotation,
		    CurrentPattern.bSafeInside);
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

	if (bDrawDebugStrike)
	{
		DrawDebugSphere(World,
		    StrikeLocation,
		    FMath::Max(CurrentPattern.AoESize.X, 100.0f),
		    16,
		    FColor::Magenta,
		    false,
		    0.6f);
	}

	// Broad candidate gather: a sphere big enough to include targets OUTSIDE the shape too (safe-zone
	// mechanics hit those who are NOT standing in the telegraph), then filter by shape membership.
	const float GatherRadius = FMath::Max(CurrentPattern.AoESize.X, CurrentPattern.AoESize.Y) + 2500.0f;
	FCollisionObjectQueryParams ObjectParams(ECC_Pawn);
	FCollisionQueryParams QueryParams(FName(TEXT("LProjectBossStrike")), false, B);

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(Overlaps,
	    StrikeLocation,
	    FQuat::Identity,
	    ObjectParams,
	    FCollisionShape::MakeSphere(GatherRadius),
	    QueryParams);

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(B);
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1.0f, Context);
	if (!SpecHandle.IsValid())
	{
		return;
	}
	SpecHandle.Data->SetSetByCallerMagnitude(TAG_SetByCaller_Damage, CurrentPattern.Damage);
	SpecHandle.Data->SetSetByCallerMagnitude(TAG_SetByCaller_StaggerDamage, CurrentPattern.StaggerDamage);

	// Gap-closer: the boss lunges forward on the strike, relocating across the arena (claims space).
	if (CurrentPattern.ChargeStrength > 0.0f)
	{
		const FVector Launch = StrikeRotation.Vector() * CurrentPattern.ChargeStrength + FVector(0, 0, 120.0f);
		B->LaunchCharacter(Launch, true, false);
	}

	TSet<AActor*> AlreadyHit;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || HitActor == B || AlreadyHit.Contains(HitActor))
		{
			continue;
		}

		// Normal: hit if inside the shape. Safe-zone (bSafeInside): hit if OUTSIDE it.
		const bool bInside = IsLocationInStrikeShape(HitActor->GetActorLocation());
		if (bInside == CurrentPattern.bSafeInside)
		{
			continue;
		}

		AlreadyHit.Add(HitActor);

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (!TargetASC || TargetASC->HasMatchingGameplayTag(TAG_State_Invulnerable))
		{
			continue; // i-frames (dash/counter) dodge the hit
		}
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data, TargetASC);

		// Knockback: launch the target away from the strike center (player gets airborne pushback).
		if (CurrentPattern.KnockbackStrength > 0.0f)
		{
			if (ACharacter* HitChar = Cast<ACharacter>(HitActor))
			{
				FVector Away = HitActor->GetActorLocation() - StrikeLocation;
				Away.Z = 0.0f;
				Away = Away.GetSafeNormal();
				const FVector Launch = Away * CurrentPattern.KnockbackStrength + FVector(0, 0, 350.0f);
				HitChar->LaunchCharacter(Launch, true, true);
			}
		}
	}
}

bool ULProjectBossPatternRunnerComponent::IsLocationInStrikeShape(const FVector& Loc) const
{
	FVector ToActor = Loc - StrikeLocation;
	ToActor.Z = 0.0f;
	const float Dist2D = ToActor.Size2D();

	switch (CurrentPattern.Shape)
	{
	case ELProjectTelegraphShape::Box:
	{
		// Into the strike's local frame: |x| <= halfX, |y| <= halfY.
		const FVector Local = StrikeRotation.UnrotateVector(ToActor);
		return FMath::Abs(Local.X) <= CurrentPattern.AoESize.X && FMath::Abs(Local.Y) <= CurrentPattern.AoESize.Y;
	}
	case ELProjectTelegraphShape::Cone:
	{
		if (Dist2D > CurrentPattern.AoESize.X || Dist2D <= KINDA_SMALL_NUMBER)
		{
			return Dist2D <= KINDA_SMALL_NUMBER; // at the apex counts as inside
		}
		const float CosHalf = FMath::Cos(FMath::DegreesToRadians(CurrentPattern.AoESize.Y));
		return FVector::DotProduct(ToActor / Dist2D, StrikeRotation.Vector()) >= CosHalf;
	}
	case ELProjectTelegraphShape::Circle:
	default:
		return Dist2D <= CurrentPattern.AoESize.X;
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

	// 5) PHASE 2+: knockback slam — a fast point-blank burst that launches the player away.
	FLProjectBossAttackPattern KnockbackSlam;
	KnockbackSlam.Shape = ELProjectTelegraphShape::Circle;
	KnockbackSlam.AoESize = FVector(560.0f, 0.0f, 0.0f);
	KnockbackSlam.TargetMode = ELProjectTelegraphTarget::SelfLocation;
	KnockbackSlam.TelegraphDuration = 1.3f;
	KnockbackSlam.Damage = 55.0f;
	KnockbackSlam.StaggerDamage = 0.0f;
	KnockbackSlam.KnockbackStrength = 1500.0f;
	KnockbackSlam.SelectionWeight = 1.0f;
	KnockbackSlam.RequiredPhaseTags.AddTag(TAG_Phase_2);
	DefaultPatternData->Patterns.Add(KnockbackSlam);

	// 6) PHASE 2+: charge — the boss telegraphs a forward lane, then LUNGES across it (gap-closer that
	//    relocates the fight and knocks the player aside if caught).
	FLProjectBossAttackPattern Charge;
	Charge.Shape = ELProjectTelegraphShape::Box;
	Charge.AoESize = FVector(900.0f, 180.0f, 0.0f);
	Charge.TargetMode = ELProjectTelegraphTarget::SelfForward;
	Charge.StrikeOffset = FVector(900.0f, 0.0f, 0.0f);
	Charge.TelegraphDuration = 1.5f;
	Charge.Damage = 50.0f;
	Charge.KnockbackStrength = 1100.0f;
	Charge.ChargeStrength = 1900.0f;
	Charge.SelectionWeight = 0.9f;
	Charge.RequiredPhaseTags.AddTag(TAG_Phase_2);
	DefaultPatternData->Patterns.Add(Charge);

	// 7) PHASE 3+: safe-zone — the telegraphed ring is the ONLY safe spot; stand IN it or get hit.
	FLProjectBossAttackPattern SafeRing;
	SafeRing.Shape = ELProjectTelegraphShape::Circle;
	SafeRing.AoESize = FVector(420.0f, 0.0f, 0.0f);
	SafeRing.TargetMode = ELProjectTelegraphTarget::SelfLocation;
	SafeRing.TelegraphDuration = 2.2f;
	SafeRing.Damage = 80.0f;
	SafeRing.bSafeInside = true; // damages everyone OUTSIDE the ring
	SafeRing.SelectionWeight = 0.8f;
	SafeRing.RequiredPhaseTags.AddTag(TAG_Phase_3);
	DefaultPatternData->Patterns.Add(SafeRing);
}
