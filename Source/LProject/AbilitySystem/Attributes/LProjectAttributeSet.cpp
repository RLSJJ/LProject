// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Attributes/LProjectAttributeSet.h"

#include "AbilitySystem/Attributes/LProjectBossAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Core/LProjectGameplayTags.h"
#include "Engine/World.h"
#include "Feedback/LProjectCombatFeedbackSubsystem.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

ULProjectAttributeSet::ULProjectAttributeSet()
{
	InitMaxHealth(100.0f);
	InitHealth(100.0f);
	InitAttackPower(100.0f);
	InitDefense(0.0f);
	InitIdentityMax(10000.0f);
	InitIdentity(0.0f);
}

void ULProjectAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// REPNOTIFY_Always so OnRep fires even when the replicated value equals the local predicted value.
	DOREPLIFETIME_CONDITION_NOTIFY(ULProjectAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULProjectAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULProjectAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULProjectAttributeSet, Defense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULProjectAttributeSet, Identity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULProjectAttributeSet, IdentityMax, COND_None, REPNOTIFY_Always);
	// NOTE: Damage is a transient meta-attribute and is intentionally NOT replicated.
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
	else if (Attribute == GetAttackPowerAttribute() || Attribute == GetDefenseAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetIdentityAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetIdentityMax());
	}
	else if (Attribute == GetIdentityMaxAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void ULProjectAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Identity is built/spent by instant GEs (GE_IdentityGain); clamp the base here since instant effects
	// modify the base value directly and bypass the PreAttributeChange clamp.
	if (Data.EvaluatedData.Attribute == GetIdentityAttribute())
	{
		SetIdentity(FMath::Clamp(GetIdentity(), 0.0f, GetIdentityMax()));
		return;
	}

	// Only the Damage meta-attribute flows through here; all damage is routed via the exec calc.
	if (Data.EvaluatedData.Attribute != GetDamageAttribute())
	{
		return;
	}

	const float LocalDamage = GetDamage();
	SetDamage(0.0f);
	if (LocalDamage <= 0.0f)
	{
		return;
	}

	const float NewHealth = FMath::Clamp(GetHealth() - LocalDamage, 0.0f, GetMaxHealth());
	SetHealth(NewHealth);

	// Combat feel: route the resolved hit into the feedback subsystem (camera shake / hit-stop / number).
	// This is the single juice seam — every damage source already funnels through this one exec point.
	if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
	{
		AActor* Owner = GetOwningActor();
		UWorld* World = Owner ? Owner->GetWorld() : nullptr;
		if (World)
		{
			if (ULProjectCombatFeedbackSubsystem* FB = World->GetSubsystem<ULProjectCombatFeedbackSubsystem>())
			{
				const bool bTargetIsBoss =
				    ASC->HasAttributeSetForAttribute(ULProjectBossAttributeSet::GetStaggerCurrentAttribute());
				const bool bGroggy = ASC->HasMatchingGameplayTag(TAG_State_Boss_Groggy);
				FB->ReportHit(Owner, LocalDamage, bTargetIsBoss, bGroggy);
			}
		}
	}

	// Death: tag the owner so abilities/patterns can gate on it (the EncounterDirector resolves outcome).
	if (NewHealth <= 0.0f)
	{
		if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
		{
			if (!ASC->HasMatchingGameplayTag(TAG_State_Dead))
			{
				ASC->AddLooseGameplayTag(TAG_State_Dead);
			}
		}
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

void ULProjectAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULProjectAttributeSet, AttackPower, OldValue);
}

void ULProjectAttributeSet::OnRep_Defense(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULProjectAttributeSet, Defense, OldValue);
}

void ULProjectAttributeSet::OnRep_Identity(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULProjectAttributeSet, Identity, OldValue);
}

void ULProjectAttributeSet::OnRep_IdentityMax(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULProjectAttributeSet, IdentityMax, OldValue);
}
