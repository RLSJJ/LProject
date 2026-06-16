// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/LProjectGA_BasicAttack.h"

#include "AbilitySystem/Attributes/LProjectAttributeSet.h"
#include "AbilitySystem/Effects/LProjectGE_Damage.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Core/LProjectGameplayTags.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

ULProjectGA_BasicAttack::ULProjectGA_BasicAttack()
{
	DamageEffect = ULProjectGE_Damage::StaticClass();

	// Cannot attack while dead.
	ActivationBlockedTags.AddTag(TAG_State_Dead);
}

void ULProjectGA_BasicAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* Avatar = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;
	if (!Avatar || !SourceASC || !World || !DamageEffect)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Box in front of the avatar, oriented to its facing.
	const FQuat Rotation = Avatar->GetActorQuat();
	const FVector Center = Avatar->GetActorLocation() + Avatar->GetActorForwardVector() * Range;
	const FCollisionShape Box = FCollisionShape::MakeBox(FVector(HitHalfExtent));

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LProjectBasicAttack), false, Avatar);
	FCollisionObjectQueryParams ObjectParams(ECC_Pawn);

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(Overlaps, Center, Rotation, ObjectParams, Box, QueryParams);

	if (bDrawDebugHit)
	{
		DrawDebugBox(World, Center, Box.GetExtent(), Rotation, FColor::Red, false, 0.4f, 0, 2.0f);
	}

	// Build the damage spec once and apply it to each unique combatant we hit.
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(this);
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, GetAbilityLevel(), Context);
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(TAG_SetByCaller_Damage, BaseDamage);
		SpecHandle.Data->SetSetByCallerMagnitude(TAG_SetByCaller_StaggerDamage, StaggerDamage);

		TSet<AActor*> AlreadyHit;
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* HitActor = Overlap.GetActor();
			if (!HitActor || HitActor == Avatar || AlreadyHit.Contains(HitActor))
			{
				continue;
			}
			AlreadyHit.Add(HitActor);

			if (UAbilitySystemComponent* TargetASC =
			        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor))
			{
				SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data, TargetASC);
			}
		}

		// Basic attack is the Identity engine: each hit builds the awakening resource.
		if (AlreadyHit.Num() > 0)
		{
			const float Cur = SourceASC->GetNumericAttribute(ULProjectAttributeSet::GetIdentityAttribute());
			const float Max = SourceASC->GetNumericAttribute(ULProjectAttributeSet::GetIdentityMaxAttribute());
			SourceASC->SetNumericAttributeBase(ULProjectAttributeSet::GetIdentityAttribute(),
			    FMath::Clamp(Cur + IdentityPerHit * AlreadyHit.Num(), 0.0f, Max));
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
