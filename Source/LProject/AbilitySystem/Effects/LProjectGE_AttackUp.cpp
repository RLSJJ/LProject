// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Effects/LProjectGE_AttackUp.h"

#include "AbilitySystem/Attributes/LProjectAttributeSet.h"

ULProjectGE_AttackUp::ULProjectGE_AttackUp()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(10.0f));

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = ULProjectAttributeSet::GetAttackPowerAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(50.0f));
	Modifiers.Add(Modifier);
}
