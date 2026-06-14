// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/LProjectAssetManager.h"

#include "AbilitySystemGlobals.h"

void ULProjectAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	// Force GAS global data init (TargetData struct cache, gameplay cue manager, global tags,
	// attribute defaults). Auto-called on first GAS use in UE5.3+ and idempotent, but doing it
	// here makes init order deterministic before any ability system component spawns.
	UAbilitySystemGlobals::Get().InitGlobalData();
}
