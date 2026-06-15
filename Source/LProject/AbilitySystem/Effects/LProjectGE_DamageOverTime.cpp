// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Effects/LProjectGE_DamageOverTime.h"

#include "AbilitySystem/Calculations/LProjectExecCalc_Damage.h"

ULProjectGE_DamageOverTime::ULProjectGE_DamageOverTime()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(6.0f));
	Period = FScalableFloat(1.0f);
	bExecutePeriodicEffectOnApplication = true;

	FGameplayEffectExecutionDefinition ExecDef;
	ExecDef.CalculationClass = ULProjectExecCalc_Damage::StaticClass();
	Executions.Add(ExecDef);
}
