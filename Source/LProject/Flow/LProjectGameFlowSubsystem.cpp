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
#include "UI/Screens/LProjectReadyWidget.h"
#include "UI/Screens/LProjectResultWidget.h"
#include "UI/Screens/LProjectTitleWidget.h"

void ULProjectGameFlowSubsystem::BeginFlow()
{
	RequestState(ELProjectGameFlowState::Title);
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

void ULProjectGameFlowSubsystem::RequestState(ELProjectGameFlowState NewState)
{
	CurrentState = NewState;
	ShowScreenForState(NewState);

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
}

void ULProjectGameFlowSubsystem::GoToReady()
{
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
		if (!bBoundEnded)
		{
			D->OnEncounterEnded.AddDynamic(this, &ULProjectGameFlowSubsystem::HandleEncounterEnded);
			bBoundEnded = true;
		}
		// RetryEncounter restores both combatants to full and (re)starts — a clean fight every time.
		D->RetryEncounter();
	}
}

void ULProjectGameFlowSubsystem::EnterRaid()
{
	RunStats = FLProjectRunStats();
	StartOrResetEncounter();
}

void ULProjectGameFlowSubsystem::Retry()
{
	RunStats.Attempts += 1;
	StartOrResetEncounter();
}

void ULProjectGameFlowSubsystem::ReturnToTitle()
{
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
