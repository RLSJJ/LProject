// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Attributes/LProjectBossAttributeSet.h"

#include "Net/UnrealNetwork.h"

ULProjectBossAttributeSet::ULProjectBossAttributeSet()
{
	InitStaggerMax(3000.0f);
	InitStaggerCurrent(3000.0f);
}

void ULProjectBossAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(ULProjectBossAttributeSet, StaggerCurrent, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULProjectBossAttributeSet, StaggerMax, COND_None, REPNOTIFY_Always);
}

void ULProjectBossAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetStaggerCurrentAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetStaggerMax());
	}
	else if (Attribute == GetStaggerMaxAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void ULProjectBossAttributeSet::OnRep_StaggerCurrent(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULProjectBossAttributeSet, StaggerCurrent, OldValue);
}

void ULProjectBossAttributeSet::OnRep_StaggerMax(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULProjectBossAttributeSet, StaggerMax, OldValue);
}
