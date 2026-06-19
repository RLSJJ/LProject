// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Screens/LProjectReadyWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Flow/LProjectGameFlowSubsystem.h"
#include "UI/Style/LProjectWidgetStyle.h"

void ULProjectReadyWidget::BuildScreen()
{
	AddFullScreenImage(TEXT("Tex_Vignette_Dark"));
	AddImage(TEXT("Tex_BossSilhouette"),
	    FVector2D(620, 620),
	    FLinearColor(0.9f, 0.3f, 0.25f, 0.30f),
	    HAlign_Center,
	    VAlign_Center,
	    FMargin(0, 40, 0, 0));

	UVerticalBox* Col = AddCenterColumn(VAlign_Fill, FMargin(0, 70, 0, 70));

	auto AddRow = [&](UWidget* W, FMargin Pad, EHorizontalAlignment H)
	{
		if (UVerticalBoxSlot* S = Col->AddChildToVerticalBox(W))
		{
			S->SetPadding(Pad);
			S->SetHorizontalAlignment(H);
		}
	};

	AddRow(MakeText(TEXT("R A I D"), 20, LProjectUI::ColTextDim), FMargin(0, 0, 0, 2), HAlign_Center);
	AddRow(MakeText(TEXT("BEHEMOTH"), 64, LProjectUI::ColGoldLight), FMargin(0, 0, 0, 0), HAlign_Center);
	AddRow(MakeText(TEXT("the Tideturner"), 22, LProjectUI::ColCrimsonLight), FMargin(0, 0, 0, 24), HAlign_Center);

	AddRow(MakeText(TEXT("Deplete its STAGGER to force GROGGY, then BURST.  Watch for the"), 16, LProjectUI::ColText),
	    FMargin(0, 0, 0, 2),
	    HAlign_Center);
	AddRow(MakeText(TEXT("cyan COUNTER window.  Survive the ENRAGE timer to clear."), 16, LProjectUI::ColText),
	    FMargin(0, 0, 0, 24),
	    HAlign_Center);

	AddRow(MakeText(TEXT("RMB Move    LMB Attack    Space Dash    F Counter    Q/W/E/R Skills"),
	           15,
	           LProjectUI::ColTextDim),
	    FMargin(0, 0, 0, 36),
	    HAlign_Center);

	// Buttons row.
	UHorizontalBox* Buttons = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UButton* Back = MakeButton(TEXT("BACK"), FVector2D(220, 64));
	Back->OnClicked.AddDynamic(this, &ULProjectReadyWidget::OnBackClicked);
	if (UHorizontalBoxSlot* S = Buttons->AddChildToHorizontalBox(Back))
	{
		S->SetPadding(FMargin(10, 0));
	}
	UButton* Enter = MakeButton(TEXT("ENTER RAID"), FVector2D(320, 64));
	Enter->OnClicked.AddDynamic(this, &ULProjectReadyWidget::OnEnterClicked);
	if (UHorizontalBoxSlot* S = Buttons->AddChildToHorizontalBox(Enter))
	{
		S->SetPadding(FMargin(10, 0));
	}
	AddRow(Buttons, FMargin(0, 8, 0, 0), HAlign_Center);
}

void ULProjectReadyWidget::OnEnterClicked()
{
	if (ULProjectGameFlowSubsystem* F = Flow())
	{
		F->EnterRaid();
	}
}

void ULProjectReadyWidget::OnBackClicked()
{
	if (ULProjectGameFlowSubsystem* F = Flow())
	{
		F->ReturnToTitle();
	}
}
