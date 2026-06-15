// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LProjectScreenWidget.generated.h"

class UOverlay;
class UImage;
class UTextBlock;
class UButton;
class UVerticalBox;
class ULProjectGameFlowSubsystem;

/**
 * Base for full-screen menu widgets built entirely in C++ (no WBP asset). Constructs a root overlay in
 * RebuildWidget and offers helpers to add styled images/text/buttons over the PNG-brush UI style.
 */
UCLASS()
class LPROJECT_API ULProjectScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;

protected:
	/** Subclasses populate RootOverlay here. */
	virtual void BuildScreen()
	{
	}

	ULProjectGameFlowSubsystem* Flow() const;

	/** Full-screen image (e.g. vignette/background), stretched to fill. */
	UImage* AddFullScreenImage(const TCHAR* TexName, FLinearColor Tint = FLinearColor::White);

	/** A free image added to the overlay at a given alignment. */
	UImage* AddImage(const TCHAR* TexName,
	    FVector2D Size,
	    FLinearColor Tint,
	    EHorizontalAlignment H,
	    EVerticalAlignment V,
	    FMargin InPadding = FMargin(0));

	UTextBlock* MakeText(const FString& Str, int32 Size, FLinearColor Color, bool bBold = true) const;

	/** Styled menu button (PNG brushes + centered label). Caller binds OnClicked + places it. */
	UButton* MakeButton(const FString& Label, FVector2D Size = FVector2D(320, 72));

	/** Centered vertical box added to the overlay (for stacking content/buttons). */
	UVerticalBox* AddCenterColumn(EVerticalAlignment V = VAlign_Center, FMargin InPadding = FMargin(0));

	UPROPERTY()
	TObjectPtr<UOverlay> RootOverlay;
};
