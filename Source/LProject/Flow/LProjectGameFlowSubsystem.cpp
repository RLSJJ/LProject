// Copyright Epic Games, Inc. All Rights Reserved.

#include "Flow/LProjectGameFlowSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Boss/LProjectBossCharacter.h"
#include "Boss/LProjectPartBreakComponent.h"
#include "Core/LProjectGameplayTags.h"
#include "Encounter/LProjectEncounterDirector.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/LProjectRaidHUD.h"
#include "UI/Screens/LProjectReadyWidget.h"
#include "UI/Screens/LProjectResultWidget.h"
#include "UI/Screens/LProjectTitleWidget.h"

void ULProjectGameFlowSubsystem::Deinitialize()
{
	// Unbind from the director and tear down our viewport widgets so nothing dangles across a world
	// reload or GameInstance shutdown (the review's PIE-restart soft-lock / widget-leak fix).
	if (ULProjectEncounterDirector* D = BoundDirector.Get())
	{
		D->OnEncounterEnded.RemoveDynamic(this, &ULProjectGameFlowSubsystem::HandleEncounterEnded);
	}
	BoundDirector = nullptr;

	if (ActiveScreen)
	{
		ActiveScreen->RemoveFromParent();
		ActiveScreen = nullptr;
	}
	if (RaidHUD)
	{
		RaidHUD->RemoveFromParent();
		RaidHUD = nullptr;
	}

	Super::Deinitialize();
}

void ULProjectGameFlowSubsystem::BeginFlow()
{
	RequestState(ELProjectGameFlowState::Title);
}

bool ULProjectGameFlowSubsystem::CanEnter(ELProjectGameFlowState From, ELProjectGameFlowState To) const
{
	using E = ELProjectGameFlowState;
	if (From == To)
	{
		return true; // idempotent refresh (re-applies presentation); callers guard against re-entrancy
	}
	switch (From)
	{
	case E::Boot:
		return To == E::Title || To == E::Encounter; // normal boot, or the dev SkipFrontEnd fast-path
	case E::Title:
		return To == E::Ready;
	case E::Ready:
		return To == E::Encounter || To == E::Title;
	case E::Encounter:
		return To == E::ResultVictory || To == E::ResultDefeat || To == E::Title;
	case E::ResultVictory:
	case E::ResultDefeat:
		return To == E::Encounter || To == E::Title;
	default:
		return false;
	}
}

APlayerController* ULProjectGameFlowSubsystem::GetPC() const
{
	return GetGameInstance() ? GetGameInstance()->GetFirstLocalPlayerController() : nullptr;
}

ULProjectEncounterDirector* ULProjectGameFlowSubsystem::GetDirector() const
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

void ULProjectGameFlowSubsystem::ApplyPresentation(bool bUIOnly, bool bShowCursor, bool bPaused)
{
	APlayerController* C = GetPC();
	if (!C)
	{
		return;
	}

	if (bUIOnly)
	{
		FInputModeUIOnly Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		C->SetInputMode(Mode);
	}
	else
	{
		FInputModeGameAndUI Mode;
		Mode.SetHideCursorDuringCapture(false);
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		C->SetInputMode(Mode);
	}
	C->SetShowMouseCursor(bShowCursor);

	if (UWorld* W = C->GetWorld())
	{
		UGameplayStatics::SetGamePaused(W, bPaused);
	}
}

void ULProjectGameFlowSubsystem::ShowScreenForState(ELProjectGameFlowState State)
{
	if (ActiveScreen)
	{
		ActiveScreen->RemoveFromParent();
		ActiveScreen = nullptr;
	}

	TSubclassOf<UUserWidget> WidgetClass = nullptr;
	switch (State)
	{
	case ELProjectGameFlowState::Title:
		WidgetClass = ULProjectTitleWidget::StaticClass();
		break;
	case ELProjectGameFlowState::Ready:
		WidgetClass = ULProjectReadyWidget::StaticClass();
		break;
	case ELProjectGameFlowState::ResultVictory:
	case ELProjectGameFlowState::ResultDefeat:
		WidgetClass = ULProjectResultWidget::StaticClass();
		break;
	default:
		break; // Boot / Encounter: no full-screen menu
	}

	if (WidgetClass)
	{
		if (APlayerController* C = GetPC())
		{
			ActiveScreen = CreateWidget<UUserWidget>(C, WidgetClass);
			if (ActiveScreen)
			{
				ActiveScreen->AddToViewport(100);
			}
		}
	}
}

bool ULProjectGameFlowSubsystem::RequestState(ELProjectGameFlowState NewState)
{
	if (!CanEnter(CurrentState, NewState))
	{
		UE_LOG(LogTemp,
		    Warning,
		    TEXT("[Flow] Rejected illegal transition %d -> %d"),
		    static_cast<int32>(CurrentState),
		    static_cast<int32>(NewState));
		return false;
	}

	CurrentState = NewState;
	ShowScreenForState(NewState);

	// In-fight HUD lives only during the Encounter state.
	if (NewState == ELProjectGameFlowState::Encounter)
	{
		if (!RaidHUD)
		{
			if (APlayerController* C = GetPC())
			{
				RaidHUD = CreateWidget<UUserWidget>(C, ULProjectRaidHUD::StaticClass());
				if (RaidHUD)
				{
					RaidHUD->AddToViewport(10);
				}
			}
		}
	}
	else if (RaidHUD)
	{
		RaidHUD->RemoveFromParent();
		RaidHUD = nullptr;
	}

	switch (NewState)
	{
	case ELProjectGameFlowState::Encounter:
		ApplyPresentation(/*bUIOnly*/ false, /*bCursor*/ true, /*bPaused*/ false);
		break;
	default:
		ApplyPresentation(/*bUIOnly*/ true, /*bCursor*/ true, /*bPaused*/ true);
		break;
	}

	OnFlowStateChanged.Broadcast(NewState);
	return true;
}

void ULProjectGameFlowSubsystem::GoToReady()
{
	if (CurrentState != ELProjectGameFlowState::Title)
	{
		return;
	}
	RequestState(ELProjectGameFlowState::Ready);
}

void ULProjectGameFlowSubsystem::StartOrResetEncounter()
{
	RequestState(ELProjectGameFlowState::Encounter);

	if (const UGameInstance* GI = GetGameInstance())
	{
		if (UWorld* W = GI->GetWorld())
		{
			EncounterStartTime = W->GetTimeSeconds();
		}
	}

	if (ULProjectEncounterDirector* D = GetDirector())
	{
		// Bind per-director: on a world reload the director is recreated, so rebind to the live instance
		// instead of trusting a one-shot flag (which would leave Encounter->Result permanently unbound).
		if (BoundDirector.Get() != D)
		{
			if (ULProjectEncounterDirector* Old = BoundDirector.Get())
			{
				Old->OnEncounterEnded.RemoveDynamic(this, &ULProjectGameFlowSubsystem::HandleEncounterEnded);
			}
			D->OnEncounterEnded.AddDynamic(this, &ULProjectGameFlowSubsystem::HandleEncounterEnded);
			BoundDirector = D;
		}
		// RetryEncounter restores both combatants to full and (re)starts — a clean fight every time.
		D->RetryEncounter();
	}
}

void ULProjectGameFlowSubsystem::EnterRaid()
{
	// Only from the Ready screen (or the dev Boot fast-path). Guards against double-clicking ENTER RAID,
	// which would otherwise wipe RunStats and restart a live fight.
	if (CurrentState != ELProjectGameFlowState::Ready && CurrentState != ELProjectGameFlowState::Boot)
	{
		return;
	}
	RunStats = FLProjectRunStats();
	StartOrResetEncounter();
}

void ULProjectGameFlowSubsystem::Retry()
{
	// Retry is only meaningful from a Result screen.
	if (CurrentState != ELProjectGameFlowState::ResultVictory && CurrentState != ELProjectGameFlowState::ResultDefeat)
	{
		return;
	}
	RunStats.Attempts += 1;
	StartOrResetEncounter();
}

void ULProjectGameFlowSubsystem::ReturnToTitle()
{
	if (CurrentState == ELProjectGameFlowState::Boot || CurrentState == ELProjectGameFlowState::Title)
	{
		return;
	}
	// Abandon any live/finished encounter so a stale boss isn't ticking behind the Title menu.
	if (ULProjectEncounterDirector* D = GetDirector())
	{
		D->AbortEncounter();
	}
	RequestState(ELProjectGameFlowState::Title);
}

void ULProjectGameFlowSubsystem::QuitGame()
{
	if (APlayerController* C = GetPC())
	{
		UKismetSystemLibrary::QuitGame(C, C, EQuitPreference::Quit, false);
	}
}

void ULProjectGameFlowSubsystem::HandleEncounterEnded(bool bWon)
{
	// Only resolve a result while actually in the fight — guards against a stale/duplicate broadcast
	// re-running the result screen after we've already left Encounter.
	if (CurrentState != ELProjectGameFlowState::Encounter)
	{
		return;
	}
	CaptureRunStats();
	RequestState(bWon ? ELProjectGameFlowState::ResultVictory : ELProjectGameFlowState::ResultDefeat);
}

void ULProjectGameFlowSubsystem::CaptureRunStats()
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (UWorld* W = GI->GetWorld())
		{
			RunStats.ClearTimeSeconds = FMath::Max(0.0f, (float)(W->GetTimeSeconds() - EncounterStartTime));
		}
	}

	if (ULProjectEncounterDirector* D = GetDirector())
	{
		const FGameplayTag Phase = D->GetActivePhaseTag();
		if (Phase == TAG_Phase_3)
		{
			RunStats.PhaseReached = 3;
		}
		else if (Phase == TAG_Phase_2)
		{
			RunStats.PhaseReached = 2;
		}
		else
		{
			RunStats.PhaseReached = 1;
		}

		if (ALProjectBossCharacter* B = D->GetBoss())
		{
			const float MaxHP = B->GetMaxHealth();
			RunStats.BossHealthPctAtEnd = MaxHP > 0.0f ? B->GetHealth() / MaxHP : 0.0f;
			if (ULProjectPartBreakComponent* PB = B->GetPartBreak())
			{
				RunStats.PartsBroken = PB->GetBrokenPartCount();
			}
		}
	}
}
