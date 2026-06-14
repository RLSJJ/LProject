// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Attributes/LProjectAttributeSet.h"

#include "Net/UnrealNetwork.h"

ULProjectAttributeSet::ULProjectAttributeSet()
{
	InitMaxHealth(100.0f);
	InitHealth(100.0f);
}

void ULProjectAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// REPNOTIFY_Always so OnRep fires even when the replicated value equals the local predicted value.
	DOREPLIFETIME_CONDITION_NOTIFY(ULProjectAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULProjectAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void ULProjectAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void ULProjectAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULProjectAttributeSet, Health, OldValue);
}

void ULProjectAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULProjectAttributeSet, MaxHealth, OldValue);
}
