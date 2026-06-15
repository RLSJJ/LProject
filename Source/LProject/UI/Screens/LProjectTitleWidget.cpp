// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Screens/LProjectTitleWidget.h"

#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Flow/LProjectGameFlowSubsystem.h"
#include "UI/Style/LProjectWidgetStyle.h"

void ULProjectTitleWidget::BuildScreen()
{
	AddFullScreenImage(TEXT("Tex_Vignette_Dark"));
	AddImage(TEXT("Tex_BossSilhouette"),
	    FVector2D(760, 760),
	    FLinearColor(0.9f, 0.3f, 0.25f, 0.16f),
	    HAlign_Center,
	    VAlign_Center,
	    FMargin(0, 80, 0, 0));
	AddImage(TEXT("Tex_TitleLogo"),
	    FVector2D(920, 345),
	    FLinearColor::White,
	    HAlign_Center,
	    VAlign_Top,
	    FMargin(0, 70, 0, 0));

	UVerticalBox* Col = AddCenterColumn(VAlign_Center, FMargin(0, 160, 0, 0));

	UButton* Start = MakeButton(TEXT("START RAID"));
	Start->OnClicked.AddDynamic(this, &ULProjectTitleWidget::OnStartClicked);
	if (UVerticalBoxSlot* S = Col->AddChildToVerticalBox(Start))
	{
		S->SetPadding(FMargin(0, 8));
		S->SetHorizontalAlignment(HAlign_Center);
	}

	UButton* Quit = MakeButton(TEXT("QUIT"));
	Quit->OnClicked.AddDynamic(this, &ULProjectTitleWidget::OnQuitClicked);
	if (UVerticalBoxSlot* S = Col->AddChildToVerticalBox(Quit))
	{
		S->SetPadding(FMargin(0, 8));
		S->SetHorizontalAlignment(HAlign_Center);
	}

	UTextBlock* Version = MakeText(TEXT("LProject  -  greybox build"), 12, LProjectUI::ColTextDim);
	if (UOverlaySlot* VS = RootOverlay->AddChildToOverlay(Version))
	{
		VS->SetHorizontalAlignment(HAlign_Right);
		VS->SetVerticalAlignment(VAlign_Bottom);
		VS->SetPadding(FMargin(0, 0, 24, 16));
	}
}

void ULProjectTitleWidget::OnStartClicked()
{
	if (ULProjectGameFlowSubsystem* F = Flow())
	{
		F->GoToReady();
	}
}

void ULProjectTitleWidget::OnQuitClicked()
{
	if (ULProjectGameFlowSubsystem* F = Flow())
	{
		F->QuitGame();
	}
}
