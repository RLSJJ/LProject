// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Screens/LProjectResultWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Flow/LProjectGameFlowSubsystem.h"
#include "UI/Style/LProjectWidgetStyle.h"

void ULProjectResultWidget::BuildScreen()
{
	const ULProjectGameFlowSubsystem* F = Flow();
	const bool bWon = F && F->GetState() == ELProjectGameFlowState::ResultVictory;
	const FLProjectRunStats Stats = F ? F->GetRunStats() : FLProjectRunStats();

	AddFullScreenImage(TEXT("Tex_Vignette_Dark"));
	if (!bWon)
	{
		AddFullScreenImage(TEXT("Tex_Vignette_Red"), FLinearColor(1, 1, 1, 0.6f));
	}

	UVerticalBox* Col = AddCenterColumn(VAlign_Center);
	auto AddRow = [&](UWidget* W, FMargin Pad)
	{
		if (UVerticalBoxSlot* S = Col->AddChildToVerticalBox(W))
		{
			S->SetPadding(Pad);
			S->SetHorizontalAlignment(HAlign_Center);
		}
	};

	// Banner.
	UImage* Banner = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	Banner->SetBrush(LProjectUI::BrushSized(bWon ? TEXT("Tex_Result_Banner_Victory") : TEXT("Tex_Result_Banner_Defeat"),
	    FVector2D(900, 225)));
	Banner->SetDesiredSizeOverride(FVector2D(900, 225));
	AddRow(Banner, FMargin(0, 0, 0, 10));

	const FString Sub = bWon ? TEXT("Behemoth has fallen.") : TEXT("The raid has wiped.");
	AddRow(MakeText(Sub, 22, bWon ? LProjectUI::ColGoldLight : LProjectUI::ColCrimsonLight), FMargin(0, 0, 0, 24));

	// Stat lines.
	const int32 Mins = FMath::FloorToInt(Stats.ClearTimeSeconds / 60.0f);
	const int32 Secs = FMath::FloorToInt(Stats.ClearTimeSeconds) % 60;
	auto Stat = [&](const FString& K, const FString& V)
	{
		UHorizontalBox* Box = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		if (UHorizontalBoxSlot* S = Box->AddChildToHorizontalBox(MakeText(K, 18, LProjectUI::ColTextDim)))
		{
			S->SetPadding(FMargin(0, 0, 16, 0));
		}
		Box->AddChildToHorizontalBox(MakeText(V, 18, LProjectUI::ColText));
		AddRow(Box, FMargin(0, 3));
	};

	Stat(bWon ? TEXT("CLEAR TIME") : TEXT("TIME SURVIVED"), FString::Printf(TEXT("%02d:%02d"), Mins, Secs));
	Stat(TEXT("PHASE REACHED"), FString::Printf(TEXT("%d / 3"), Stats.PhaseReached));
	if (!bWon)
	{
		Stat(TEXT("BOSS HP LEFT"), FString::Printf(TEXT("%.0f%%"), Stats.BossHealthPctAtEnd * 100.0f));
	}
	Stat(TEXT("PARTS BROKEN"), FString::Printf(TEXT("%d"), Stats.PartsBroken));
	Stat(TEXT("ATTEMPTS"), FString::Printf(TEXT("%d"), Stats.Attempts));

	// Buttons.
	UHorizontalBox* Buttons = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UButton* Retry = MakeButton(TEXT("RETRY"), FVector2D(260, 64));
	Retry->OnClicked.AddDynamic(this, &ULProjectResultWidget::OnRetryClicked);
	if (UHorizontalBoxSlot* S = Buttons->AddChildToHorizontalBox(Retry))
	{
		S->SetPadding(FMargin(10, 0));
	}
	UButton* Title = MakeButton(TEXT("RETURN TO TITLE"), FVector2D(320, 64));
	Title->OnClicked.AddDynamic(this, &ULProjectResultWidget::OnTitleClicked);
	if (UHorizontalBoxSlot* S = Buttons->AddChildToHorizontalBox(Title))
	{
		S->SetPadding(FMargin(10, 0));
	}
	AddRow(Buttons, FMargin(0, 28, 0, 0));
}

void ULProjectResultWidget::OnRetryClicked()
{
	if (ULProjectGameFlowSubsystem* F = Flow())
	{
		F->Retry();
	}
}

void ULProjectResultWidget::OnTitleClicked()
{
	if (ULProjectGameFlowSubsystem* F = Flow())
	{
		F->ReturnToTitle();
	}
}
