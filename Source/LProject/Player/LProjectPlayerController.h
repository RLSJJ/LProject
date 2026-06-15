// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LProjectPlayerController.generated.h"

/**
 * Player controller for the quarterview ARPG scheme: shows the mouse cursor and enables click
 * events so the pawn can move-to-cursor (right mouse) and attack (left mouse). Future HUD owner.
 */
UCLASS()
class LPROJECT_API ALProjectPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ALProjectPlayerController();

protected:
	virtual void SetupInputComponent() override;

	/** R key: ask the EncounterDirector to reset and restart the fight (test convenience). */
	void HandleRetryPressed();
};
