// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/LProjectPlayerController.h"

#include "Components/InputComponent.h"
#include "Encounter/LProjectEncounterDirector.h"
#include "Engine/World.h"

ALProjectPlayerController::ALProjectPlayerController()
{
	// Quarterview ARPG controls are cursor-driven (right mouse to move, left mouse to attack).
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void ALProjectPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::R, IE_Pressed, this, &ALProjectPlayerController::HandleRetryPressed);
	}
}

void ALProjectPlayerController::HandleRetryPressed()
{
	if (UWorld* World = GetWorld())
	{
		if (ULProjectEncounterDirector* Director = World->GetSubsystem<ULProjectEncounterDirector>())
		{
			Director->RetryEncounter();
		}
	}
}
