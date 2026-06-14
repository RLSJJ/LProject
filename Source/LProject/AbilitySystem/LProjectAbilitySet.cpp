// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/LProjectAbilitySet.h"

#include "AbilitySystem/Abilities/LProjectGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"

void ULProjectAbilitySet::GiveToAbilitySystem(UAbilitySystemComponent* ASC, UObject* SourceObject) const
{
	if (!ASC || !ASC->IsOwnerActorAuthoritative())
	{
		return;
	}

	// Attribute sets first (abilities may read them).
	for (const FLProjectAbilitySet_AttributeSet& SetToGrant : GrantedAttributes)
	{
		if (SetToGrant.AttributeSet)
		{
			UAttributeSet* NewSet = NewObject<UAttributeSet>(ASC->GetOwner(), SetToGrant.AttributeSet);
			ASC->AddAttributeSetSubobject(NewSet);
		}
	}

	// Abilities, tagging each spec with its input tag so input can activate it by tag.
	for (const FLProjectAbilitySet_GameplayAbility& AbilityToGrant : GrantedGameplayAbilities)
	{
		if (!AbilityToGrant.Ability)
		{
			continue;
		}

		FGameplayAbilitySpec Spec(AbilityToGrant.Ability, AbilityToGrant.AbilityLevel, INDEX_NONE, SourceObject);
		if (AbilityToGrant.InputTag.IsValid())
		{
			Spec.GetDynamicSpecSourceTags().AddTag(AbilityToGrant.InputTag);
		}
		ASC->GiveAbility(Spec);
	}
}
