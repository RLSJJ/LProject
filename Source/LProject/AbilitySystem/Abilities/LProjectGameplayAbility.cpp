// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/LProjectGameplayAbility.h"

#include "AbilitySystem/Effects/LProjectGE_IdentityGain.h"
#include "AbilitySystemComponent.h"
#include "Core/LProjectGameplayTags.h"

ULProjectGameplayAbility::ULProjectGameplayAbility()
{
	// One instance per actor (state lives on the avatar); locally predicted for responsiveness.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	IdentityGainEffect = ULProjectGE_IdentityGain::StaticClass();
}

void ULProjectGameplayAbility::ApplyIdentityDelta(float Delta)
{
	if (Delta == 0.0f || !IdentityGainEffect)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);
	const FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(IdentityGainEffect, GetAbilityLevel());
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(TAG_SetByCaller_Identity, Delta);
		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
	}
}
