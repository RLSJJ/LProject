// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Effects/LProjectGE_Cooldown.h"

#include "Core/LProjectGameplayTags.h"
#include "GameplayEffect.h"

ULProjectGE_Cooldown::ULProjectGE_Cooldown()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = TAG_SetByCaller_CooldownDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
}
