// Copyright Epic Games, Inc. All Rights Reserved.

#include "Input/LProjectInputConfig.h"

const UInputAction* ULProjectInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag) const
{
	for (const FLProjectInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}
	return nullptr;
}

const UInputAction* ULProjectInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag) const
{
	for (const FLProjectInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}
	return nullptr;
}
