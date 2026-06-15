// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/LProjectScreenWidget.h"
#include "LProjectReadyWidget.generated.h"

/** Pre-fight raid lobby: boss name/silhouette, mechanic primer + controls, ENTER RAID / BACK. */
UCLASS()
class LPROJECT_API ULProjectReadyWidget : public ULProjectScreenWidget
{
	GENERATED_BODY()

protected:
	virtual void BuildScreen() override;

	UFUNCTION()
	void OnEnterClicked();

	UFUNCTION()
	void OnBackClicked();
};
