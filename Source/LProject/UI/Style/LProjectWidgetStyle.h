// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"
#include "Fonts/SlateFontInfo.h"

class UTexture2D;

/**
 * Central UI style: one place for the dark-fantasy palette, the imported PNG brushes, and fonts so
 * every widget stays cohesive. Brush loaders fall back to a tinted color box when a texture is missing,
 * so screens still render before/without art.
 */
namespace LProjectUI
{
// --- Palette (matches the arena/HUD constants and the generated PNG art) ---
extern const FLinearColor ColBG;         // near-black backdrop
extern const FLinearColor ColSlate;      // panel base
extern const FLinearColor ColSlateLight; // raised slate
extern const FLinearColor ColGold;       // primary accent
extern const FLinearColor ColGoldLight;  // hover/highlight gold
extern const FLinearColor ColCrimson;    // boss / danger / defeat
extern const FLinearColor ColCrimsonLight;
extern const FLinearColor ColOrange;  // groggy / enrage
extern const FLinearColor ColCyan;    // counter
extern const FLinearColor ColGreen;   // player HP
extern const FLinearColor ColText;    // primary text
extern const FLinearColor ColTextDim; // secondary text

/** Load an imported UI texture by short name (e.g. TEXT("Tex_Button_Normal")). Null if missing. */
UTexture2D* LoadTex(const TCHAR* ShortName);

/** Image brush from an imported texture; falls back to a tinted rounded box if the texture is absent. */
FSlateBrush Brush(const TCHAR* ShortName, FLinearColor Tint = FLinearColor::White);

/** Image brush with an explicit draw size (for icons/keycaps). */
FSlateBrush BrushSized(const TCHAR* ShortName, FVector2D DrawSize, FLinearColor Tint = FLinearColor::White);

/** Solid color box brush (no texture). */
FSlateBrush ColorBrush(FLinearColor Color);

/** Project font at the given size. */
FSlateFontInfo Font(int32 Size, bool bBold = true);
} // namespace LProjectUI
