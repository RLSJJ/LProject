// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "LProjectAssetManager.generated.h"

/**
 * Project asset manager.
 *
 * Sole purpose right now: force GAS global data initialization at a deterministic point
 * (StartInitialLoading), the Epic/Lyra pattern. In UE5.3+ InitGlobalData() is auto-called on
 * first GAS use and is idempotent, but doing it here guarantees ordering before any
 * AbilitySystemComponent is created. Registered via DefaultEngine.ini:
 *   [/Script/Engine.Engine] AssetManagerClassName=/Script/LProject.LProjectAssetManager
 */
UCLASS()
class LPROJECT_API ULProjectAssetManager : public UAssetManager
{
	GENERATED_BODY()

protected:
	//~ Begin UAssetManager interface
	virtual void StartInitialLoading() override;
	//~ End UAssetManager interface
};
