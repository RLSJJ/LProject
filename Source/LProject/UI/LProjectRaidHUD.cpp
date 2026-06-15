// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/LProjectRaidHUD.h"

#include "AbilitySystem/Attributes/LProjectBossAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Boss/LProjectBossCharacter.h"
#include "Character/LProjectCharacterBase.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Core/LProjectGameplayTags.h"
#include "Encounter/LProjectEncounterDirector.h"
#include "Engine/GameInstance.h"
#include "UI/Style/LProjectWidgetStyle.h"

namespace
{
UCanvasPanelSlot* Place(UCanvasPanel* Canvas,
    UWidget* W,
    FAnchors Anchors,
    FVector2D Alignment,
    FVector2D Pos,
    FVector2D Size,
    bool bAutoSize)
{
	UCanvasPanelSlot* S = Canvas->AddChildToCanvas(W);
	S->SetAnchors(Anchors);
	S->SetAlignment(Alignment);
	S->SetAutoSize(bAutoSize);
	S->SetPosition(Pos);
	if (!bAutoSize)
	{
		S->SetSize(Size);
	}
	return S;
}
} // namespace

UProgressBar* ULProjectRaidHUD::MakeBar(const TCHAR* FillTex, FLinearColor FillTint)
{
	UProgressBar* Bar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
	FProgressBarStyle St;
	St.BackgroundImage = LProjectUI::Brush(TEXT("Tex_BarBG"));
	St.FillImage = LProjectUI::Brush(FillTex);
	Bar->SetWidgetStyle(St);
	Bar->SetFillColorAndOpacity(FillTint);
	Bar->SetPercent(1.0f);
	return Bar;
}

TSharedRef<SWidget> ULProjectRaidHUD::RebuildWidget()
{
	if (!Root && WidgetTree)
	{
		Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
		WidgetTree->RootWidget = Root;
		BuildHUD();
	}
	return Super::RebuildWidget();
}

void ULProjectRaidHUD::BuildHUD()
{
	const FAnchors TopC(0.5f, 0.0f);
	const FAnchors TopL(0.0f, 0.0f);
	const FAnchors TopR(1.0f, 0.0f);
	const FAnchors Center(0.5f, 0.5f);
	const FAnchors BotL(0.0f, 1.0f);

	// --- Boss block (top center) ---
	BossNameText = MakeText_Internal(TEXT("BEHEMOTH"), 30, LProjectUI::ColGoldLight);
	Place(Root, BossNameText, TopC, FVector2D(0.5f, 0.0f), FVector2D(0, 22), FVector2D(0, 0), true);

	BossHealthBar = MakeBar(TEXT("Tex_Bar_Fill_Crimson"), LProjectUI::ColCrimsonLight);
	Place(Root, BossHealthBar, TopC, FVector2D(0.5f, 0.0f), FVector2D(0, 60), FVector2D(820, 26), false);

	BossBarCountText = MakeText_Internal(TEXT(""), 13, LProjectUI::ColText);
	Place(Root, BossBarCountText, TopC, FVector2D(0.5f, 0.0f), FVector2D(420, 62), FVector2D(0, 0), true);

	StaggerBar = MakeBar(TEXT("Tex_Bar_Fill_Amber"), FLinearColor(0.95f, 0.8f, 0.15f, 1.0f));
	Place(Root, StaggerBar, TopC, FVector2D(0.5f, 0.0f), FVector2D(0, 90), FVector2D(620, 12), false);

	GroggyBanner = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	GroggyBanner->SetBrush(LProjectUI::BrushSized(TEXT("Tex_Groggy_Glow"), FVector2D(360, 64)));
	Place(Root, GroggyBanner, TopC, FVector2D(0.5f, 0.0f), FVector2D(0, 104), FVector2D(360, 64), false);
	GroggyText = MakeText_Internal(TEXT("GROGGY"), 22, LProjectUI::ColOrange);
	Place(Root, GroggyText, TopC, FVector2D(0.5f, 0.0f), FVector2D(0, 116), FVector2D(0, 0), true);

	// --- Phase + enrage ---
	PhaseText = MakeText_Internal(TEXT("PHASE 1"), 16, LProjectUI::ColTextDim);
	Place(Root, PhaseText, TopL, FVector2D(0.0f, 0.0f), FVector2D(28, 26), FVector2D(0, 0), true);

	EnrageText = MakeText_Internal(TEXT("ENRAGE 05:00"), 18, LProjectUI::ColText);
	Place(Root, EnrageText, TopR, FVector2D(1.0f, 0.0f), FVector2D(-28, 26), FVector2D(0, 0), true);

	// --- Player block (bottom left) ---
	PlayerHealthText = MakeText_Internal(TEXT("HP"), 14, LProjectUI::ColText);
	Place(Root, PlayerHealthText, BotL, FVector2D(0.0f, 1.0f), FVector2D(40, -74), FVector2D(0, 0), true);
	PlayerHealthBar = MakeBar(TEXT("Tex_Bar_Fill_Green"), LProjectUI::ColGreen);
	Place(Root, PlayerHealthBar, BotL, FVector2D(0.0f, 1.0f), FVector2D(40, -52), FVector2D(360, 20), false);

	// --- Center counter prompt ---
	CounterBurst = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	CounterBurst->SetBrush(LProjectUI::BrushSized(TEXT("Tex_Counter_Burst"), FVector2D(320, 320)));
	Place(Root, CounterBurst, Center, FVector2D(0.5f, 0.5f), FVector2D(0, -120), FVector2D(320, 320), false);
	CounterText = MakeText_Internal(TEXT("COUNTER!  [Q]"), 30, LProjectUI::ColCyan);
	Place(Root, CounterText, Center, FVector2D(0.5f, 0.5f), FVector2D(0, -120), FVector2D(0, 0), true);

	GroggyBanner->SetVisibility(ESlateVisibility::Hidden);
	GroggyText->SetVisibility(ESlateVisibility::Hidden);
	CounterBurst->SetVisibility(ESlateVisibility::Hidden);
	CounterText->SetVisibility(ESlateVisibility::Hidden);
}

UTextBlock* ULProjectRaidHUD::MakeText_Internal(const FString& Str, int32 Size, FLinearColor Color)
{
	UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	T->SetText(FText::FromString(Str));
	T->SetFont(LProjectUI::Font(Size, true));
	T->SetColorAndOpacity(FSlateColor(Color));
	return T;
}

ULProjectEncounterDirector* ULProjectRaidHUD::Director() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (UWorld* W = GI->GetWorld())
		{
			return W->GetSubsystem<ULProjectEncounterDirector>();
		}
	}
	return nullptr;
}

void ULProjectRaidHUD::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);
	PulseTime += DeltaTime;
	const float Pulse = 0.55f + 0.45f * FMath::Sin(PulseTime * 6.0f);

	ULProjectEncounterDirector* D = Director();
	ALProjectBossCharacter* B = D ? D->GetBoss() : nullptr;

	if (B)
	{
		const float MaxHP = B->GetMaxHealth();
		const float Pct = MaxHP > 0.0f ? FMath::Clamp(B->GetHealth() / MaxHP, 0.0f, 1.0f) : 0.0f;
		const int32 Bars = B->GetHealthBarCount();
		const float Scaled = Pct * Bars;
		int32 BarsRem = FMath::Clamp(FMath::CeilToInt(Scaled - KINDA_SMALL_NUMBER), 0, Bars);
		float CurBar = Scaled - FMath::FloorToFloat(Scaled - KINDA_SMALL_NUMBER);
		if (Pct <= 0.0f)
		{
			BarsRem = 0;
			CurBar = 0.0f;
		}
		BossHealthBar->SetPercent(CurBar);
		BossBarCountText->SetText(FText::FromString(FString::Printf(TEXT("BARS  %d / %d"), BarsRem, Bars)));

		if (const ULProjectBossAttributeSet* BossSet = B->GetBossAttributeSet())
		{
			const float MaxSt = BossSet->GetStaggerMax();
			StaggerBar->SetPercent(
			    MaxSt > 0.0f ? FMath::Clamp(BossSet->GetStaggerCurrent() / MaxSt, 0.0f, 1.0f) : 0.0f);
		}

		const bool bGroggy = B->IsGroggy();
		GroggyBanner->SetVisibility(bGroggy ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
		GroggyText->SetVisibility(bGroggy ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
		if (bGroggy)
		{
			GroggyText->SetColorAndOpacity(FSlateColor(LProjectUI::ColOrange.CopyWithNewOpacity(Pulse)));
		}

		bool bCounter = false;
		if (UAbilitySystemComponent* BossASC = B->GetAbilitySystemComponent())
		{
			bCounter = BossASC->HasMatchingGameplayTag(TAG_State_Boss_Counterable);
		}
		CounterBurst->SetVisibility(bCounter ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
		CounterText->SetVisibility(bCounter ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
		if (bCounter)
		{
			CounterText->SetColorAndOpacity(FSlateColor(LProjectUI::ColCyan.CopyWithNewOpacity(Pulse)));
			CounterBurst->SetRenderOpacity(Pulse);
		}
	}

	if (D)
	{
		const FGameplayTag Phase = D->GetActivePhaseTag();
		int32 PhaseNum = 1;
		if (Phase == TAG_Phase_3)
		{
			PhaseNum = 3;
		}
		else if (Phase == TAG_Phase_2)
		{
			PhaseNum = 2;
		}
		PhaseText->SetText(FText::FromString(FString::Printf(TEXT("PHASE %d"), PhaseNum)));

		const float Enrage = D->GetEnrageSecondsRemaining();
		const int32 Mins = FMath::FloorToInt(Enrage / 60.0f);
		const int32 Secs = FMath::FloorToInt(Enrage) % 60;
		EnrageText->SetText(FText::FromString(FString::Printf(TEXT("ENRAGE  %02d:%02d"), Mins, Secs)));
		EnrageText->SetColorAndOpacity(FSlateColor(Enrage < 30.0f ? LProjectUI::ColCrimsonLight : LProjectUI::ColText));

		if (ALProjectCharacterBase* P = D->GetPlayer())
		{
			const float MaxHP = P->GetMaxHealth();
			const float HPct = MaxHP > 0.0f ? FMath::Clamp(P->GetHealth() / MaxHP, 0.0f, 1.0f) : 0.0f;
			PlayerHealthBar->SetPercent(HPct);
			PlayerHealthText->SetText(FText::FromString(
			    FString::Printf(TEXT("HP   %d / %d"), FMath::RoundToInt(P->GetHealth()), FMath::RoundToInt(MaxHP))));
		}
	}
}
