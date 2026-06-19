// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Calculations/LProjectExecCalc_Damage.h"

#include "AbilitySystem/Attributes/LProjectAttributeSet.h"
#include "AbilitySystem/Attributes/LProjectBossAttributeSet.h"
#include "Core/LProjectGameplayTags.h"

// Captures the attributes the formula reads. Both are read live (snapshot = false) so buffs/debuffs
// applied between spec creation and execution are reflected.
struct FLProjectDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Defense);

	FLProjectDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(ULProjectAttributeSet, AttackPower, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(ULProjectAttributeSet, Defense, Target, false);
	}
};

static const FLProjectDamageStatics& DamageStatics()
{
	static FLProjectDamageStatics Statics;
	return Statics;
}

ULProjectExecCalc_Damage::ULProjectExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
	RelevantAttributesToCapture.Add(DamageStatics().DefenseDef);
}

void ULProjectExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
    FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// Live combat primaries.
	float AttackPower = 100.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackPowerDef, EvalParams, AttackPower);
	AttackPower = FMath::Max(AttackPower, 0.0f);

	float Defense = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DefenseDef, EvalParams, Defense);
	Defense = FMath::Max(Defense, 0.0f);

	// Per-hit SetByCaller magnitudes (assigned by the attacker on the spec). Missing -> 0, no warning.
	const float BaseDamage = Spec.GetSetByCallerMagnitude(TAG_SetByCaller_Damage, false, 0.0f);
	const float StaggerDamage = Spec.GetSetByCallerMagnitude(TAG_SetByCaller_StaggerDamage, false, 0.0f);

	// True damage (e.g. the counter burst) skips Defense entirely.
	FGameplayTagContainer AssetTags;
	Spec.GetAllAssetTags(AssetTags);
	const bool bTrueDamage = AssetTags.HasTagExact(TAG_Damage_Type_True);

	// Defense mitigation, capped so high Defense never fully negates a hit.
	const float Mitigation = bTrueDamage ? 1.0f : (1.0f - FMath::Clamp(Defense / 100.0f, 0.0f, 0.95f));
	float FinalDamage = BaseDamage * (AttackPower / 100.0f) * Mitigation;

	// Groggy/무력화 burst window: damage to a groggy boss is amplified.
	if (FinalDamage > 0.0f && TargetASC && TargetASC->HasMatchingGameplayTag(TAG_State_Boss_Groggy))
	{
		FinalDamage *= GroggyDamageMultiplier;
	}

	if (FinalDamage > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(ULProjectAttributeSet::GetDamageAttribute(),
		    EGameplayModOp::Additive,
		    FinalDamage));
	}

	// Stagger only applies to targets that actually have the boss stagger gauge.
	if (StaggerDamage > 0.0f && TargetASC &&
	    TargetASC->HasAttributeSetForAttribute(ULProjectBossAttributeSet::GetStaggerCurrentAttribute()))
	{
		OutExecutionOutput.AddOutputModifier(
		    FGameplayModifierEvaluatedData(ULProjectBossAttributeSet::GetStaggerCurrentAttribute(),
		        EGameplayModOp::Additive,
		        -StaggerDamage));
	}
}
