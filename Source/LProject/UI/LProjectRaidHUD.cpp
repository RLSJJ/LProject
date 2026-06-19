// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/LProjectRaidHUD.h"

#include "AbilitySystem/Attributes/LProjectAttributeSet.h"
#include "AbilitySystem/Attributes/LProjectBossAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Boss/LProjectBossCharacter.h"
#include "Character/LProjectCharacterBase.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Core/LProjectGameplayTags.h"
#include "Encounter/LProjectEncounterDirector.h"
#include "Engine/GameInstance.h"
#include "GameplayEffect.h"
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
	Place(Root, PlayerHealthText, BotL, FVector2D(0.0f, 1.0f), FVector2D(40, -92), FVector2D(0, 0), true);
	PlayerHealthBar = MakeBar(TEXT("Tex_Bar_Fill_Green"), LProjectUI::ColGreen);
	Place(Root, PlayerHealthBar, BotL, FVector2D(0.0f, 1.0f), FVector2D(40, -70), FVector2D(360, 20), false);
	IdentityBar = MakeBar(TEXT("Tex_Bar_Fill_Gold"), LProjectUI::ColGoldLight);
	IdentityBar->SetPercent(0.0f);
	Place(Root, IdentityBar, BotL, FVector2D(0.0f, 1.0f), FVector2D(40, -46), FVector2D(360, 10), false);

	// --- Skill bar (bottom center) ---
	UHorizontalBox* SkillRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	AddSkillSlot(SkillRow, TEXT("Tex_Skill_Attack"), TEXT("LMB"), FGameplayTag(), false);
	AddSkillSlot(SkillRow, TEXT("Tex_Skill_Charge"), TEXT("Q"), TAG_Cooldown_Charge, false);
	AddSkillSlot(SkillRow, TEXT("Tex_Skill_Cleave"), TEXT("W"), TAG_Cooldown_Cleave, false);
	AddSkillSlot(SkillRow, TEXT("Tex_Skill_Bolt"), TEXT("E"), TAG_Cooldown_Bolt, false);
	AddSkillSlot(SkillRow, TEXT("Tex_Skill_Awakening"), TEXT("R"), TAG_Cooldown_Awakening, true);
	AddSkillSlot(SkillRow, TEXT("Tex_Skill_Dash"), TEXT("SPC"), FGameplayTag(), false);
	AddSkillSlot(SkillRow, TEXT("Tex_Skill_Counter"), TEXT("F"), FGameplayTag(), false);
	Place(Root, SkillRow, FAnchors(0.5f, 1.0f), FVector2D(0.5f, 1.0f), FVector2D(0, -16), FVector2D(0, 0), true);

	// --- Center counter prompt ---
	CounterBurst = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	CounterBurst->SetBrush(LProjectUI::BrushSized(TEXT("Tex_Counter_Burst"), FVector2D(320, 320)));
	Place(Root, CounterBurst, Center, FVector2D(0.5f, 0.5f), FVector2D(0, -120), FVector2D(320, 320), false);
	CounterText = MakeText_Internal(TEXT("COUNTER!  [F]"), 30, LProjectUI::ColCyan);
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

UAbilitySystemComponent* ULProjectRaidHUD::PlayerASC() const
{
	if (ULProjectEncounterDirector* D = Director())
	{
		if (ALProjectCharacterBase* P = D->GetPlayer())
		{
			return P->GetAbilitySystemComponent();
		}
	}
	return nullptr;
}

void ULProjectRaidHUD::AddSkillSlot(UHorizontalBox* Row,
    const TCHAR* IconTex,
    const FString& KeyLabel,
    FGameplayTag CooldownTag,
    bool bAwakening)
{
	const FVector2D SlotSize(78, 78);
	UOverlay* SlotBox = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());

	UImage* Frame = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	Frame->SetBrush(LProjectUI::BrushSized(TEXT("Tex_SkillSlot_Frame"), SlotSize));
	if (UOverlaySlot* OS = SlotBox->AddChildToOverlay(Frame))
	{
		OS->SetHorizontalAlignment(HAlign_Fill);
		OS->SetVerticalAlignment(VAlign_Fill);
	}

	UImage* Icon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	Icon->SetBrush(LProjectUI::BrushSized(IconTex, FVector2D(60, 60)));
	if (UOverlaySlot* OS = SlotBox->AddChildToOverlay(Icon))
	{
		OS->SetHorizontalAlignment(HAlign_Center);
		OS->SetVerticalAlignment(VAlign_Center);
	}

	UImage* Dark = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	Dark->SetBrush(LProjectUI::ColorBrush(FLinearColor(0, 0, 0, 0.6f)));
	Dark->SetVisibility(ESlateVisibility::Hidden);
	if (UOverlaySlot* OS = SlotBox->AddChildToOverlay(Dark))
	{
		OS->SetHorizontalAlignment(HAlign_Fill);
		OS->SetVerticalAlignment(VAlign_Fill);
	}

	UTextBlock* CDText = MakeText_Internal(TEXT(""), 22, LProjectUI::ColText);
	CDText->SetVisibility(ESlateVisibility::Hidden);
	if (UOverlaySlot* OS = SlotBox->AddChildToOverlay(CDText))
	{
		OS->SetHorizontalAlignment(HAlign_Center);
		OS->SetVerticalAlignment(VAlign_Center);
	}

	UTextBlock* Key = MakeText_Internal(KeyLabel, 11, LProjectUI::ColTextDim);
	if (UOverlaySlot* OS = SlotBox->AddChildToOverlay(Key))
	{
		OS->SetHorizontalAlignment(HAlign_Center);
		OS->SetVerticalAlignment(VAlign_Bottom);
	}

	if (UHorizontalBoxSlot* HS = Row->AddChildToHorizontalBox(SlotBox))
	{
		HS->SetPadding(FMargin(5, 0));
	}

	FLProjectSkillSlot S;
	S.Icon = Icon;
	S.CooldownDark = Dark;
	S.CooldownText = CDText;
	S.CooldownTag = CooldownTag;
	S.bAwakening = bAwakening;
	SkillSlots.Add(S);
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

	// Identity gauge + skill cooldowns.
	if (UAbilitySystemComponent* PASC = PlayerASC())
	{
		const float Id = PASC->GetNumericAttribute(ULProjectAttributeSet::GetIdentityAttribute());
		const float IdMax = PASC->GetNumericAttribute(ULProjectAttributeSet::GetIdentityMaxAttribute());
		const bool bFullIdentity = IdMax > 0.0f && Id >= IdMax * 0.99f;
		if (IdentityBar)
		{
			IdentityBar->SetPercent(IdMax > 0.0f ? FMath::Clamp(Id / IdMax, 0.0f, 1.0f) : 0.0f);
			IdentityBar->SetFillColorAndOpacity(
			    bFullIdentity ? LProjectUI::ColGoldLight.CopyWithNewOpacity(Pulse) : LProjectUI::ColGoldLight);
		}

		for (const FLProjectSkillSlot& S : SkillSlots)
		{
			float Remaining = 0.0f;
			if (S.CooldownTag.IsValid())
			{
				const FGameplayEffectQuery Query =
				    FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(S.CooldownTag));
				for (float T : PASC->GetActiveEffectsTimeRemaining(Query))
				{
					Remaining = FMath::Max(Remaining, T);
				}
			}
			const bool bOnCD = Remaining > 0.0f;
			if (S.CooldownDark)
			{
				S.CooldownDark->SetVisibility(bOnCD ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
			}
			if (S.CooldownText)
			{
				S.CooldownText->SetVisibility(bOnCD ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
				if (bOnCD)
				{
					S.CooldownText->SetText(FText::AsNumber(FMath::CeilToInt(Remaining)));
				}
			}
			if (S.bAwakening && S.Icon)
			{
				S.Icon->SetRenderOpacity(bFullIdentity && !bOnCD ? 1.0f : 0.35f);
			}
		}
	}
}
