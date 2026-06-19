// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Effects/LProjectGE_IdentityGain.h"

#include "AbilitySystem/Attributes/LProjectAttributeSet.h"
#include "Core/LProjectGameplayTags.h"

ULProjectGE_IdentityGain::ULProjectGE_IdentityGain()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = TAG_SetByCaller_Identity;

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = ULProjectAttributeSet::GetIdentityAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	Modifiers.Add(Modifier);
}
