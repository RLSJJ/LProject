// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Effects/LProjectGE_Damage.h"

#include "AbilitySystem/Calculations/LProjectExecCalc_Damage.h"

ULProjectGE_Damage::ULProjectGE_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayEffectExecutionDefinition ExecDef;
	ExecDef.CalculationClass = ULProjectExecCalc_Damage::StaticClass();
	Executions.Add(ExecDef);
}
