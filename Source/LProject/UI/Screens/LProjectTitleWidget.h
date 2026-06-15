// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/LProjectScreenWidget.h"
#include "LProjectTitleWidget.generated.h"

/** Main menu: title logo over a dark vignette + boss silhouette, START RAID / QUIT. */
UCLASS()
class LPROJECT_API ULProjectTitleWidget : public ULProjectScreenWidget
{
	GENERATED_BODY()

protected:
	virtual void BuildScreen() override;

	UFUNCTION()
	void OnStartClicked();

	UFUNCTION()
	void OnQuitClicked();
};
