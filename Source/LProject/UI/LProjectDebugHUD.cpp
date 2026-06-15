// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/LProjectDebugHUD.h"

#include "AbilitySystem/Attributes/LProjectBossAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Boss/LProjectBossCharacter.h"
#include "Boss/LProjectPartBreakComponent.h"
#include "Character/LProjectCharacterBase.h"
#include "Core/LProjectGameplayTags.h"
#include "Encounter/LProjectEncounterDirector.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

void ALProjectDebugHUD::DrawBar(float X,
    float Y,
    float Width,
    float Height,
    float Pct,
    const FLinearColor& Background,
    const FLinearColor& Fill,
    int32 Segments)
{
	DrawRect(Background, X, Y, Width, Height);
	DrawRect(Fill, X, Y, Width * FMath::Clamp(Pct, 0.0f, 1.0f), Height);

	if (Segments > 1)
	{
		const FLinearColor Divider(0.0f, 0.0f, 0.0f, 0.8f);
		for (int32 i = 1; i < Segments; ++i)
		{
			DrawRect(Divider, X + Width * (static_cast<float>(i) / Segments) - 1.0f, Y, 2.0f, Height);
		}
	}
}

void ALProjectDebugHUD::DrawHUD()
{
	Super::DrawHUD();

	UWorld* World = GetWorld();
	if (!Canvas || !World)
	{
		return;
	}

	UFont* Font = GEngine ? GEngine->GetMediumFont() : nullptr;
	const float CW = Canvas->SizeX;
	const float CH = Canvas->SizeY;

	const FLinearColor DarkBG(0.05f, 0.05f, 0.05f, 0.7f);
	const FLinearColor White = FLinearColor::White;

	ULProjectEncounterDirector* Director = World->GetSubsystem<ULProjectEncounterDirector>();

	// --- Boss block (top center) ---
	ALProjectBossCharacter* Boss = Director ? Director->GetBoss() : nullptr;
	if (Boss)
	{
		const float BarW = CW * 0.6f;
		const float BarX = (CW - BarW) * 0.5f;
		const float BarY = CH * 0.07f;

		DrawText(TEXT("BEHEMOTH"), White, BarX, BarY - 24.0f, Font);

		const float MaxHP = Boss->GetMaxHealth();
		const float HpPct = MaxHP > 0.0f ? Boss->GetHealth() / MaxHP : 0.0f;
		DrawBar(BarX, BarY, BarW, 24.0f, HpPct, DarkBG, FLinearColor(0.85f, 0.1f, 0.1f), Boss->GetHealthBarCount());

		// Stagger gauge under the HP bar.
		if (const ULProjectBossAttributeSet* BossSet = Boss->GetBossAttributeSet())
		{
			const float MaxStagger = BossSet->GetStaggerMax();
			const float StaggerPct = MaxStagger > 0.0f ? BossSet->GetStaggerCurrent() / MaxStagger : 0.0f;
			DrawBar(BarX, BarY + 28.0f, BarW, 10.0f, StaggerPct, DarkBG, FLinearColor(0.95f, 0.8f, 0.1f));
		}

		float InfoY = BarY + 42.0f;
		if (Boss->IsGroggy())
		{
			DrawText(TEXT("GROGGY! (무력화) - 2x damage"), FLinearColor(1.0f, 0.5f, 0.0f), BarX, InfoY, Font);
			InfoY += 18.0f;
		}
		if (const ULProjectPartBreakComponent* Parts = Boss->GetPartBreak())
		{
			DrawText(FString::Printf(TEXT("Parts broken: %d / %d"), Parts->GetBrokenPartCount(), Parts->GetPartCount()),
			    White,
			    BarX,
			    InfoY,
			    Font);
		}

		// Counter prompt (center screen) while the window is open.
		if (UAbilitySystemComponent* BossASC = Boss->GetAbilitySystemComponent())
		{
			if (BossASC->HasMatchingGameplayTag(TAG_State_Boss_Counterable))
			{
				DrawText(TEXT("COUNTER!  [Q]"),
				    FLinearColor(0.1f, 0.9f, 1.0f),
				    CW * 0.5f - 70.0f,
				    CH * 0.42f,
				    Font,
				    1.6f);
			}
		}
	}

	// --- Encounter info (top left) ---
	if (Director)
	{
		const FString PhaseStr =
		    Director->GetActivePhaseTag().IsValid() ? Director->GetActivePhaseTag().ToString() : TEXT("-");
		DrawText(FString::Printf(TEXT("Phase: %s"), *PhaseStr), White, CW * 0.02f, CH * 0.07f, Font);

		const float Enrage = Director->GetEnrageSecondsRemaining();
		const int32 Mins = FMath::FloorToInt(Enrage / 60.0f);
		const int32 Secs = FMath::FloorToInt(Enrage) % 60;
		const FLinearColor EnrageColor = Enrage < 30.0f ? FLinearColor::Red : White;
		DrawText(FString::Printf(TEXT("Enrage: %02d:%02d"), Mins, Secs), EnrageColor, CW * 0.02f, CH * 0.10f, Font);

		// (Win/lose banner is owned by the flow's Result screen now, not the debug HUD.)
	}

	// --- Player HP (bottom left) ---
	if (ALProjectCharacterBase* Player = Cast<ALProjectCharacterBase>(GetOwningPawn()))
	{
		const float MaxHP = Player->GetMaxHealth();
		const float HpPct = MaxHP > 0.0f ? Player->GetHealth() / MaxHP : 0.0f;
		DrawText(FString::Printf(TEXT("HP  %d / %d"), FMath::RoundToInt(Player->GetHealth()), FMath::RoundToInt(MaxHP)),
		    White,
		    CW * 0.02f,
		    CH * 0.88f - 18.0f,
		    Font);
		DrawBar(CW * 0.02f, CH * 0.88f, CW * 0.25f, 18.0f, HpPct, DarkBG, FLinearColor(0.2f, 0.85f, 0.2f));
	}
}
