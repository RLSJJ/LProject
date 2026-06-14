// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/LProjectAbilitySystemComponent.h"

void ULProjectAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	// Collect matching handles under the list lock, then activate outside it (activation can
	// mutate the activatable-abilities list).
	TArray<FGameplayAbilitySpecHandle> HandlesToActivate;
	{
		ABILITYLIST_SCOPE_LOCK();
		for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
		{
			if (Spec.Ability && Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
			{
				HandlesToActivate.Add(Spec.Handle);
			}
		}
	}

	for (const FGameplayAbilitySpecHandle& Handle : HandlesToActivate)
	{
		TryActivateAbility(Handle);
	}
}

void ULProjectAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	// No held/charge abilities yet. Intentionally minimal; expand when such abilities exist.
}
