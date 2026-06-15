// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/LProjectGA_Counter.h"

#include "AbilitySystem/Effects/LProjectGE_AttackUp.h"
#include "AbilitySystem/Effects/LProjectGE_Damage.h"
#include "AbilitySystem/Effects/LProjectGE_DamageOverTime.h"
#include "AbilitySystemComponent.h"
#include "Boss/LProjectBossCharacter.h"
#include "Boss/LProjectBossPatternRunnerComponent.h"
#include "Core/LProjectGameplayTags.h"
#include "Encounter/LProjectEncounterDirector.h"
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

	ULProjectEncounterDirector* Director = World->GetSubsystem<ULProjectEncounterDirector>();
	ALProjectBossCharacter* Boss = Director ? Director->GetBoss() : nullptr;
	UAbilitySystemComponent* BossASC = Boss ? Boss->GetAbilitySystemComponent() : nullptr;

	const bool bInRange = Boss && FVector::Dist(Avatar->GetActorLocation(), Boss->GetActorLocation()) <= Range;
	const bool bSuccess = bInRange && BossASC && BossASC->HasMatchingGameplayTag(TAG_State_Boss_Counterable);

	if (!bSuccess)
	{
		// Whiff: off-window or out of range.
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// Interrupt the boss's in-flight pattern.
	if (ULProjectBossPatternRunnerComponent* Runner = Boss->GetPatternRunner())
	{
		Runner->InterruptCurrentPattern();
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
