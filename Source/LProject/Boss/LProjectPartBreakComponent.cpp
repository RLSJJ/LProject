// Copyright Epic Games, Inc. All Rights Reserved.

#include "Boss/LProjectPartBreakComponent.h"

#include "AbilitySystem/Attributes/LProjectAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Boss/LProjectBossCharacter.h"
#include "Core/LProjectGameplayTags.h"

ULProjectPartBreakComponent::ULProjectPartBreakComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULProjectPartBreakComponent::BeginPlay()
{
	Super::BeginPlay();

	Boss = Cast<ALProjectBossCharacter>(GetOwner());

	if (Parts.Num() == 0)
	{
		FLProjectBossPart Core;
		Core.PartId = FName(TEXT("Core"));
		Core.MaxDurability = 6000.0f;
		Core.DefenseReductionOnBreak = 20.0f;
		Parts.Add(Core);
	}

	for (FLProjectBossPart& Part : Parts)
	{
		Part.Current = Part.MaxDurability;
		Part.bBroken = false;
	}
}

void ULProjectPartBreakComponent::ApplyDamageToPrimaryPart(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	for (int32 Index = 0; Index < Parts.Num(); ++Index)
	{
		if (!Parts[Index].bBroken)
		{
			Parts[Index].Current -= Amount;
			if (Parts[Index].Current <= 0.0f)
			{
				BreakPart(Index);
			}
			return;
		}
	}
}

void ULProjectPartBreakComponent::ApplyPartDamage(FName PartId, float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	for (int32 Index = 0; Index < Parts.Num(); ++Index)
	{
		if (Parts[Index].PartId == PartId && !Parts[Index].bBroken)
		{
			Parts[Index].Current -= Amount;
			if (Parts[Index].Current <= 0.0f)
			{
				BreakPart(Index);
			}
			return;
		}
	}
}

void ULProjectPartBreakComponent::BreakPart(int32 PartIndex)
{
	if (!Parts.IsValidIndex(PartIndex) || Parts[PartIndex].bBroken)
	{
		return;
	}

	FLProjectBossPart& Part = Parts[PartIndex];
	Part.bBroken = true;
	Part.Current = 0.0f;

	// Weakness reward: permanently lower the boss's Defense and flag it for the HUD.
	if (ALProjectBossCharacter* B = Boss.Get())
	{
		if (UAbilitySystemComponent* ASC = B->GetAbilitySystemComponent())
		{
			const float NewDefense = FMath::Max(0.0f,
			    ASC->GetNumericAttributeBase(ULProjectAttributeSet::GetDefenseAttribute()) -
			        Part.DefenseReductionOnBreak);
			ASC->SetNumericAttributeBase(ULProjectAttributeSet::GetDefenseAttribute(), NewDefense);
			ASC->AddLooseGameplayTag(TAG_Debuff_DefenseDown);
			if (!ASC->HasMatchingGameplayTag(TAG_State_Boss_PartBroken))
			{
				ASC->AddLooseGameplayTag(TAG_State_Boss_PartBroken);
			}
		}
	}

	OnPartBroken.Broadcast(Part.PartId);
}

int32 ULProjectPartBreakComponent::GetBrokenPartCount() const
{
	int32 Count = 0;
	for (const FLProjectBossPart& Part : Parts)
	{
		if (Part.bBroken)
		{
			++Count;
		}
	}
	return Count;
}
