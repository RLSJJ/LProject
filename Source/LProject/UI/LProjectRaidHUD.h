// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "LProjectRaidHUD.generated.h"

class UImage;
class UTextBlock;
class UProgressBar;
class UHorizontalBox;
class ULProjectEncounterDirector;
class UAbilitySystemComponent;

/** One slot on the skill bar (icon + cooldown darken overlay + remaining-seconds text). */
USTRUCT()
struct FLProjectSkillSlot
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UImage> Icon = nullptr;
	UPROPERTY()
	TObjectPtr<UImage> CooldownDark = nullptr;
	UPROPERTY()
	TObjectPtr<UTextBlock> CooldownText = nullptr;

	FGameplayTag CooldownTag;
	bool bAwakening = false;
};

/**
 * In-fight raid HUD, built in C++ over the PNG-brush UI style (no WBP). Polls the EncounterDirector +
 * boss + player each tick to drive: boss name plate + multi-bar HP + stagger/groggy, the enrage clock,
 * the player vitals, and the center counter prompt. Shown by the flow owner during the Encounter state.
 */
UCLASS()
class LPROJECT_API ULProjectRaidHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;

protected:
	void BuildHUD();
	ULProjectEncounterDirector* Director() const;

	UProgressBar* MakeBar(const TCHAR* FillTex, FLinearColor FillTint);
	UTextBlock* MakeText_Internal(const FString& Str, int32 Size, FLinearColor Color);

	UPROPERTY()
	TObjectPtr<class UCanvasPanel> Root;

	// Boss block.
	UPROPERTY()
	TObjectPtr<UTextBlock> BossNameText;
	UPROPERTY()
	TObjectPtr<UTextBlock> BossBarCountText;
	UPROPERTY()
	TObjectPtr<UProgressBar> BossHealthBar;
	UPROPERTY()
	TObjectPtr<UProgressBar> StaggerBar;
	UPROPERTY()
	TObjectPtr<UTextBlock> PhaseText;
	UPROPERTY()
	TObjectPtr<UTextBlock> EnrageText;
	UPROPERTY()
	TObjectPtr<UImage> GroggyBanner;
	UPROPERTY()
	TObjectPtr<UTextBlock> GroggyText;

	// Player block.
	UPROPERTY()
	TObjectPtr<UProgressBar> PlayerHealthBar;
	UPROPERTY()
	TObjectPtr<UTextBlock> PlayerHealthText;
	UPROPERTY()
	TObjectPtr<UProgressBar> IdentityBar;

	// Skill bar.
	UPROPERTY()
	TArray<FLProjectSkillSlot> SkillSlots;

	void AddSkillSlot(UHorizontalBox* Row,
	    const TCHAR* IconTex,
	    const FString& KeyLabel,
	    FGameplayTag CooldownTag,
	    bool bAwakening);
	UAbilitySystemComponent* PlayerASC() const;

	// Center counter prompt.
	UPROPERTY()
	TObjectPtr<UImage> CounterBurst;
	UPROPERTY()
	TObjectPtr<UTextBlock> CounterText;

	float PulseTime = 0.0f;
};
