// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/LProjectPlayerController.h"

#include "Components/InputComponent.h"
#include "Engine/GameInstance.h"
#include "Flow/LProjectGameFlowSubsystem.h"

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
	// Route through the flow owner so dev (R key) and shipped (RETRY button) paths are identical.
	// Only meaningful on a Result screen — Flow::Retry() guards illegal states, but gate here too so the
	// key is inert on Title/Ready and never yanks the player into a fight from a menu.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (ULProjectGameFlowSubsystem* Flow = GI->GetSubsystem<ULProjectGameFlowSubsystem>())
		{
			const ELProjectGameFlowState State = Flow->GetState();
			if (State == ELProjectGameFlowState::ResultVictory || State == ELProjectGameFlowState::ResultDefeat)
			{
				Flow->Retry();
			}
		}
	}
}
