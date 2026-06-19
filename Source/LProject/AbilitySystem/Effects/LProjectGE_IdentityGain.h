// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Effects/LProjectGameplayEffect.h"
#include "LProjectGE_IdentityGain.generated.h"

/**
 * Instant Identity (awakening resource) delta, driven by SetByCaller.Identity on the spec. Positive
 * builds Identity (basic attack / skill use), negative spends it (awakening). Routing the resource through
 * a GameplayEffect — instead of a direct SetNumericAttributeBase — keeps it on the GAS pipeline so it is
 * predicted, cue-able, and consistent with every other attribute change. The attribute set clamps the
 * result to [0, IdentityMax].
 */
UCLASS()
class LPROJECT_API ULProjectGE_IdentityGain : public ULProjectGameplayEffect
{
	GENERATED_BODY()

public:
	ULProjectGE_IdentityGain();
};
