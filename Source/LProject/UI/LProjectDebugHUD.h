// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LProjectDebugHUD.generated.h"

/**
 * Greybox debug HUD drawn with Canvas primitives (DrawText/DrawRect) — zero UMG/Slate deps, zero
 * assets. Polls the EncounterDirector + boss/player each frame to draw the boss's multi-bar HP, the
 * stagger/groggy gauge, the counter prompt, broken-part count, active phase, the enrage countdown, the
 * player's HP, and the win/lose banner. A system-first readout for testing every base system.
 */
UCLASS()
class LPROJECT_API ALProjectDebugHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

protected:
	/** Draws a background + proportional fill bar, with optional segment dividers (multi-bar HP). */
	void DrawBar(float X,
	    float Y,
	    float Width,
	    float Height,
	    float Pct,
	    const FLinearColor& Background,
	    const FLinearColor& Fill,
	    int32 Segments = 1);
};
