// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/LProjectGA_Skill.h"

#include "AbilitySystem/Attributes/LProjectAttributeSet.h"
#include "AbilitySystem/Effects/LProjectGE_Cooldown.h"
#include "AbilitySystem/Effects/LProjectGE_Damage.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Core/LProjectGameplayTags.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

ULProjectGA_Skill::ULProjectGA_Skill()
{
	DamageEffect = ULProjectGE_Damage::StaticClass();
	CooldownGameplayEffectClass = ULProjectGE_Cooldown::StaticClass();
	ActivationBlockedTags.AddTag(TAG_State_Dead);
}

const FGameplayTagContainer* ULProjectGA_Skill::GetCooldownTags() const
{
	MutableCooldownTags.Reset();
	if (CooldownTag.IsValid())
	{
		MutableCooldownTags.AddTag(CooldownTag);
	}
	return &MutableCooldownTags;
}

void ULProjectGA_Skill::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo) const
{
	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (!CooldownGE || !CooldownTag.IsValid())
	{
		return;
	}

	FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), GetAbilityLevel());
	if (Spec.IsValid())
	{
		Spec.Data->DynamicGrantedTags.AddTag(CooldownTag);
		Spec.Data->SetSetByCallerMagnitude(TAG_SetByCaller_CooldownDuration, CooldownDuration);
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Spec);
	}
}

int32 ULProjectGA_Skill::ApplyShapeDamage(const FVector& Center,
    const FQuat& Rotation,
    const FCollisionShape& Shape,
    float Damage,
    float StaggerDamage,
    bool bConeFilter,
    float ConeHalfAngleDeg)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;
	if (!Avatar || !SourceASC || !World || !DamageEffect)
	{
		return 0;
	}

	if (bDrawDebug)
	{
		DrawDebugSphere(World, Center, FMath::Max(Shape.GetSphereRadius(), 80.0f), 16, FColor::Yellow, false, 0.5f);
	}

	FCollisionObjectQueryParams ObjectParams(ECC_Pawn);
	FCollisionQueryParams QueryParams(FName(TEXT("LProjectSkill")), false, Avatar);
	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(Overlaps, Center, Rotation, ObjectParams, Shape, QueryParams);

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(this);
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, GetAbilityLevel(), Context);
	if (!SpecHandle.IsValid())
	{
		return 0;
	}
	SpecHandle.Data->SetSetByCallerMagnitude(TAG_SetByCaller_Damage, Damage);
	SpecHandle.Data->SetSetByCallerMagnitude(TAG_SetByCaller_StaggerDamage, StaggerDamage);

	const FVector Forward = Rotation.GetForwardVector();
	const float CosHalf = FMath::Cos(FMath::DegreesToRadians(ConeHalfAngleDeg));

	int32 Hits = 0;
	TSet<AActor*> AlreadyHit;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || HitActor == Avatar || AlreadyHit.Contains(HitActor))
		{
			continue;
		}
		if (bConeFilter)
		{
			FVector ToActor = HitActor->GetActorLocation() - Center;
			ToActor.Z = 0.0f;
			if (!ToActor.IsNearlyZero() && FVector::DotProduct(ToActor.GetSafeNormal(), Forward) < CosHalf)
			{
				continue;
			}
		}
		AlreadyHit.Add(HitActor);

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (!TargetASC || TargetASC->HasMatchingGameplayTag(TAG_State_Invulnerable))
		{
			continue;
		}
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data, TargetASC);
		++Hits;
	}
	return Hits;
}

void ULProjectGA_Skill::GainIdentity(float Amount)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}
	const float Cur = ASC->GetNumericAttribute(ULProjectAttributeSet::GetIdentityAttribute());
	const float Max = ASC->GetNumericAttribute(ULProjectAttributeSet::GetIdentityMaxAttribute());
	ASC->SetNumericAttributeBase(ULProjectAttributeSet::GetIdentityAttribute(), FMath::Clamp(Cur + Amount, 0.0f, Max));
}

float ULProjectGA_Skill::GetIdentity() const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	return ASC ? ASC->GetNumericAttribute(ULProjectAttributeSet::GetIdentityAttribute()) : 0.0f;
}

float ULProjectGA_Skill::GetIdentityMax() const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	return ASC ? ASC->GetNumericAttribute(ULProjectAttributeSet::GetIdentityMaxAttribute()) : 1.0f;
}

FVector ULProjectGA_Skill::GetAimDirection() const
{
	if (const ACharacter* Avatar = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		if (APlayerController* PC = Cast<APlayerController>(Avatar->GetController()))
		{
			FHitResult Hit;
			if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit))
			{
				FVector Dir = Hit.ImpactPoint - Avatar->GetActorLocation();
				Dir.Z = 0.0f;
				if (!Dir.IsNearlyZero())
				{
					return Dir.GetSafeNormal();
				}
			}
		}
		return Avatar->GetActorForwardVector();
	}
	return FVector::ForwardVector;
}

void ULProjectGA_Skill::GrantIFramesAndEndLater(float Duration)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AddLooseGameplayTag(TAG_State_Invulnerable);
	}

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (UWorld* World = Avatar ? Avatar->GetWorld() : nullptr)
	{
		World->GetTimerManager().SetTimer(SkillTimerHandle, this, &ULProjectGA_Skill::FinishSkill, Duration, false);
	}
	else
	{
		FinishSkill();
	}
}

void ULProjectGA_Skill::FinishSkill()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(TAG_State_Invulnerable);
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
