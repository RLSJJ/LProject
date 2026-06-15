// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/LProjectScreenWidget.h"
#include "LProjectResultWidget.generated.h"

/**
 * End-of-fight result screen, parameterized by the flow state (ResultVictory/ResultDefeat). Shows the
 * banner, run stats (clear time, phase reached, boss HP% remaining, parts broken, attempts), and the
 * RETRY / RETURN TO TITLE buttons.
 */
UCLASS()
class LPROJECT_API ULProjectResultWidget : public ULProjectScreenWidget
{
	GENERATED_BODY()

protected:
	virtual void BuildScreen() override;

	UFUNCTION()
	void OnRetryClicked();

	UFUNCTION()
	void OnTitleClicked();
};
