// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/LProjectGA_Counter.h"

#include "AbilitySystem/Effects/LProjectGE_AttackUp.h"
#include "AbilitySystem/Effects/LProjectGE_Damage.h"
#include "AbilitySystem/Effects/LProjectGE_DamageOverTime.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Combat/LProjectCombatInterface.h"
#include "Core/LProjectGameplayTags.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "TimerManager.h"

ULProjectGA_Counter::ULProjectGA_Counter()
{
	DamageEffect = ULProjectGE_Damage::StaticClass();
	BleedEffect = ULProjectGE_DamageOverTime::StaticClass();
	RewardBuff = ULProjectGE_AttackUp::StaticClass();

	ActivationBlockedTags.AddTag(TAG_State_Dead);
}

void ULProjectGA_Counter::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
	if (!Avatar || !SourceASC || !World)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Find a counterable combatant in range — decoupled from the boss/encounter concrete types: overlap
	// nearby pawns, keep the nearest one that (a) implements ILProjectCombatant and (b) is currently
	// broadcasting the counterable window. This lets the counter work against any counterable enemy.
	const FVector Origin = Avatar->GetActorLocation();
	FCollisionObjectQueryParams ObjectParams(ECC_Pawn);
	FCollisionQueryParams QueryParams(FName(TEXT("LProjectCounter")), false, Avatar);

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(Overlaps,
	    Origin,
	    FQuat::Identity,
	    ObjectParams,
	    FCollisionShape::MakeSphere(Range),
	    QueryParams);

	AActor* TargetActor = nullptr;
	UAbilitySystemComponent* TargetASC = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate || Candidate == Avatar || !Cast<ILProjectCombatant>(Candidate))
		{
			continue;
		}
		UAbilitySystemComponent* CandidateASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Candidate);
		if (!CandidateASC || !CandidateASC->HasMatchingGameplayTag(TAG_State_Boss_Counterable))
		{
			continue;
		}
		const float DistSq = FVector::DistSquared(Origin, Candidate->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			TargetActor = Candidate;
			TargetASC = CandidateASC;
		}
	}

	if (!TargetActor || !TargetASC)
	{
		// Whiff: nothing counterable in range / off-window.
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UAbilitySystemComponent* BossASC = TargetASC;

	// Interrupt the target's in-flight action through the combatant interface (no concrete-type coupling).
	if (ILProjectCombatant* Combatant = Cast<ILProjectCombatant>(TargetActor))
	{
		Combatant->NotifyCountered();
	}

	// Stagger burst + chip damage.
	if (DamageEffect)
	{
		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		Context.AddSourceObject(this);
		FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageEffect, 1.0f, Context);
		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(TAG_SetByCaller_Damage, CounterDamage);
			Spec.Data->SetSetByCallerMagnitude(TAG_SetByCaller_StaggerDamage, StaggerBurst);
			// Counter chip is true damage — the exec calc skips Defense mitigation for this tag.
			Spec.Data->AddDynamicAssetTag(TAG_Damage_Type_True);
			SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data, BossASC);
		}
	}

	// Bleed DoT on the boss.
	if (BleedEffect)
	{
		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		Context.AddSourceObject(this);
		FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(BleedEffect, 1.0f, Context);
		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(TAG_SetByCaller_Damage, BleedPerTick);
			SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data, BossASC);
		}
	}

	// Reward the player with a temporary attack buff (applied to self).
	if (RewardBuff)
	{
		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		Context.AddSourceObject(this);
		FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(RewardBuff, 1.0f, Context);
		if (Spec.IsValid())
		{
			SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data, SourceASC);
		}
	}

	// Brief i-frames; the ability stays active until they end.
	SourceASC->AddLooseGameplayTag(TAG_State_Invulnerable);
	World->GetTimerManager().SetTimer(IFrameTimerHandle,
	    this,
	    &ULProjectGA_Counter::FinishCounter,
	    IFrameDuration,
	    false);
}

void ULProjectGA_Counter::FinishCounter()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(TAG_State_Invulnerable);
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
