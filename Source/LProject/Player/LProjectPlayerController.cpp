// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/LProjectPlayerController.h"

ALProjectPlayerController::ALProjectPlayerController()
{
	// Quarterview ARPG controls are cursor-driven (right mouse to move, left mouse to attack).
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;
}
