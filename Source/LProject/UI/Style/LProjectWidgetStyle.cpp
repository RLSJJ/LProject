// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Style/LProjectWidgetStyle.h"

#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"

namespace LProjectUI
{
const FLinearColor ColBG = FLinearColor(0.015f, 0.018f, 0.027f, 1.0f);
const FLinearColor ColSlate = FLinearColor(0.055f, 0.066f, 0.094f, 1.0f);
const FLinearColor ColSlateLight = FLinearColor(0.10f, 0.12f, 0.16f, 1.0f);
const FLinearColor ColGold = FLinearColor(0.62f, 0.50f, 0.22f, 1.0f);
const FLinearColor ColGoldLight = FLinearColor(0.85f, 0.72f, 0.36f, 1.0f);
const FLinearColor ColCrimson = FLinearColor(0.50f, 0.13f, 0.13f, 1.0f);
const FLinearColor ColCrimsonLight = FLinearColor(0.78f, 0.25f, 0.20f, 1.0f);
const FLinearColor ColOrange = FLinearColor(0.90f, 0.45f, 0.10f, 1.0f);
const FLinearColor ColCyan = FLinearColor(0.30f, 0.85f, 1.0f, 1.0f);
const FLinearColor ColGreen = FLinearColor(0.20f, 0.70f, 0.32f, 1.0f);
const FLinearColor ColText = FLinearColor(0.91f, 0.89f, 0.85f, 1.0f);
const FLinearColor ColTextDim = FLinearColor(0.62f, 0.62f, 0.60f, 1.0f);

UTexture2D* LoadTex(const TCHAR* ShortName)
{
	const FString Path = FString::Printf(TEXT("/Game/UI/Tex/%s.%s"), ShortName, ShortName);
	return LoadObject<UTexture2D>(nullptr, *Path);
}

FSlateBrush Brush(const TCHAR* ShortName, FLinearColor Tint)
{
	FSlateBrush B;
	if (UTexture2D* Tex = LoadTex(ShortName))
	{
		B.SetResourceObject(Tex);
		B.ImageSize = FVector2D(Tex->GetSizeX(), Tex->GetSizeY());
		B.DrawAs = ESlateBrushDrawType::Image;
		B.TintColor = FSlateColor(Tint);
	}
	else
	{
		// Fallback so screens still render without art.
		B.DrawAs = ESlateBrushDrawType::RoundedBox;
		B.TintColor = FSlateColor(Tint.A < 1.0f ? Tint : ColSlate);
		B.OutlineSettings.Color = FSlateColor(ColGold);
		B.OutlineSettings.Width = 2.0f;
		B.OutlineSettings.CornerRadii = FVector4(8, 8, 8, 8);
		B.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
	}
	return B;
}

FSlateBrush BrushSized(const TCHAR* ShortName, FVector2D DrawSize, FLinearColor Tint)
{
	FSlateBrush B = Brush(ShortName, Tint);
	B.ImageSize = DrawSize;
	return B;
}

FSlateBrush ColorBrush(FLinearColor Color)
{
	FSlateBrush B;
	B.DrawAs = ESlateBrushDrawType::Box;
	B.TintColor = FSlateColor(Color);
	return B;
}

FSlateFontInfo Font(int32 Size, bool bBold)
{
	return FCoreStyle::GetDefaultFontStyle(bBold ? "Bold" : "Regular", Size);
}
} // namespace LProjectUI
