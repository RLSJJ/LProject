// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/LProjectGA_Dash.h"

#include "AbilitySystemComponent.h"
#include "Core/LProjectGameplayTags.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

ULProjectGA_Dash::ULProjectGA_Dash()
{
}

void ULProjectGA_Dash::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!Character || !ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// i-frames for the duration of the dash.
	ASC->AddLooseGameplayTag(TAG_State_Invulnerable);

	// Dash along the current movement input, falling back to facing direction.
	FVector Direction = Character->GetLastMovementInputVector().GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		Direction = Character->GetActorForwardVector();
	}
	Character->LaunchCharacter(Direction * DashStrength, true, true);

	if (UWorld* World = Character->GetWorld())
	{
		World->GetTimerManager().SetTimer(DashTimerHandle, this, &ULProjectGA_Dash::FinishDash, DashDuration, false);
	}
	else
	{
		FinishDash();
	}
}

void ULProjectGA_Dash::FinishDash()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(TAG_State_Invulnerable);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
