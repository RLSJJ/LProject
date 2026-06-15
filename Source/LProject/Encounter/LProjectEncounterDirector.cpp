// Copyright Epic Games, Inc. All Rights Reserved.

#include "Encounter/LProjectEncounterDirector.h"

#include "AbilitySystem/Attributes/LProjectAttributeSet.h"
#include "AbilitySystem/Attributes/LProjectBossAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Boss/LProjectBossCharacter.h"
#include "Boss/LProjectBossPatternRunnerComponent.h"
#include "Character/LProjectCharacterBase.h"
#include "Core/LProjectGameplayTags.h"

void ULProjectEncounterDirector::Tick(float DeltaTime)
{
	if (!bEncounterActive)
	{
		return;
	}

	ALProjectBossCharacter* B = Boss.Get();
	ALProjectCharacterBase* P = Player.Get();
	if (!B || !P)
	{
		return;
	}

	// Resolve win/lose first.
	if (!P->IsAlive())
	{
		EndEncounter(false);
		return;
	}
	if (!B->IsAlive())
	{
		EndEncounter(true);
		return;
	}

	// Phase gates: advance to every newly-crossed threshold (phases are in descending order).
	const float MaxHP = B->GetMaxHealth();
	const float Pct = MaxHP > 0.0f ? B->GetHealth() / MaxHP : 0.0f;
	while (Phases.IsValidIndex(CurrentPhaseIndex + 1) && Pct <= Phases[CurrentPhaseIndex + 1].HealthPctThreshold)
	{
		EnterPhase(CurrentPhaseIndex + 1);
	}

	// Enrage DPS check.
	EnrageSecondsRemaining = FMath::Max(EnrageSecondsRemaining - DeltaTime, 0.0f);
	OnEnrageTick.Broadcast(EnrageSecondsRemaining);
	if (EnrageSecondsRemaining <= 0.0f)
	{
		EndEncounter(false);
	}
}

TStatId ULProjectEncounterDirector::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULProjectEncounterDirector, STATGROUP_Tickables);
}

void ULProjectEncounterDirector::RegisterBoss(ALProjectBossCharacter* InBoss)
{
	Boss = InBoss;
}

void ULProjectEncounterDirector::RegisterPlayer(ALProjectCharacterBase* InPlayer)
{
	Player = InPlayer;
}

ALProjectBossCharacter* ULProjectEncounterDirector::GetBoss() const
{
	return Boss.Get();
}

ALProjectCharacterBase* ULProjectEncounterDirector::GetPlayer() const
{
	return Player.Get();
}

void ULProjectEncounterDirector::StartEncounter()
{
	if (Phases.Num() == 0)
	{
		BuildDefaultPhases();
	}

	bEncounterActive = true;
	Outcome = ELProjectEncounterOutcome::InProgress;
	CurrentPhaseIndex = -1;
	EnrageSecondsRemaining = EnrageDuration;
	ActivePhaseTag = FGameplayTag();
	ActivePhaseTags.Reset();

	// Phase 0 starts immediately.
	if (Phases.Num() > 0)
	{
		EnterPhase(0);
	}

	if (ALProjectBossCharacter* B = Boss.Get())
	{
		if (ULProjectBossPatternRunnerComponent* Runner = B->GetPatternRunner())
		{
			Runner->SetPaused(false);
		}
	}

	OnEncounterStarted.Broadcast();
}

void ULProjectEncounterDirector::EnterPhase(int32 PhaseIndex)
{
	if (!Phases.IsValidIndex(PhaseIndex))
	{
		return;
	}

	CurrentPhaseIndex = PhaseIndex;
	const FLProjectEncounterPhase& Phase = Phases[PhaseIndex];

	ActivePhaseTag = Phase.PhaseTag;
	ActivePhaseTags.Reset();
	if (Phase.PhaseTag.IsValid())
	{
		ActivePhaseTags.AddTag(Phase.PhaseTag);
	}

	if (ALProjectBossCharacter* B = Boss.Get())
	{
		if (ULProjectBossPatternRunnerComponent* Runner = B->GetPatternRunner())
		{
			Runner->SetActivePhaseTags(ActivePhaseTags);
			if (Phase.PhasePatterns)
			{
				Runner->SetPatternData(Phase.PhasePatterns);
			}
		}
	}

	OnPhaseChanged.Broadcast(ActivePhaseTag);
}

void ULProjectEncounterDirector::EndEncounter(bool bWon)
{
	if (!bEncounterActive)
	{
		return;
	}
	bEncounterActive = false;
	Outcome = bWon ? ELProjectEncounterOutcome::Won : ELProjectEncounterOutcome::Lost;

	if (ALProjectBossCharacter* B = Boss.Get())
	{
		if (ULProjectBossPatternRunnerComponent* Runner = B->GetPatternRunner())
		{
			Runner->InterruptCurrentPattern();
			Runner->SetPaused(true);
		}
	}

	OnEncounterEnded.Broadcast(bWon);
}

void ULProjectEncounterDirector::RetryEncounter()
{
	// Restore the boss.
	if (ALProjectBossCharacter* B = Boss.Get())
	{
		if (UAbilitySystemComponent* ASC = B->GetAbilitySystemComponent())
		{
			ASC->RemoveLooseGameplayTag(TAG_State_Dead);
			ASC->RemoveLooseGameplayTag(TAG_State_Boss_Groggy);
			ASC->SetNumericAttributeBase(ULProjectAttributeSet::GetHealthAttribute(), B->GetMaxHealth());
			if (const ULProjectBossAttributeSet* BossSet = B->GetBossAttributeSet())
			{
				ASC->SetNumericAttributeBase(ULProjectBossAttributeSet::GetStaggerCurrentAttribute(),
				    BossSet->GetStaggerMax());
			}
		}
	}

	// Restore the player.
	if (ALProjectCharacterBase* P = Player.Get())
	{
		if (UAbilitySystemComponent* ASC = P->GetAbilitySystemComponent())
		{
			ASC->RemoveLooseGameplayTag(TAG_State_Dead);
			ASC->SetNumericAttributeBase(ULProjectAttributeSet::GetHealthAttribute(), P->GetMaxHealth());
		}
	}

	StartEncounter();
}

void ULProjectEncounterDirector::BuildDefaultPhases()
{
	auto MakePhase = [](float Threshold, const FGameplayTag& Tag)
	{
		FLProjectEncounterPhase Phase;
		Phase.HealthPctThreshold = Threshold;
		Phase.PhaseTag = Tag;
		return Phase;
	};

	Phases.Add(MakePhase(1.0f, TAG_Phase_1));
	Phases.Add(MakePhase(0.66f, TAG_Phase_2));
	Phases.Add(MakePhase(0.33f, TAG_Phase_3));
}
