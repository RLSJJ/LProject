// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/LProjectGameplayAbility.h"

ULProjectGameplayAbility::ULProjectGameplayAbility()
{
	// One instance per actor (state lives on the avatar); locally predicted for responsiveness.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}
