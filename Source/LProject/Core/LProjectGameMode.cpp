// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/LProjectGameMode.h"

#include "Character/LProjectPlayerCharacter.h"
#include "Player/LProjectPlayerController.h"

ALProjectGameMode::ALProjectGameMode()
{
	DefaultPawnClass = ALProjectPlayerCharacter::StaticClass();
	PlayerControllerClass = ALProjectPlayerController::StaticClass();
}
