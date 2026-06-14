// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/LProjectCharacterBase.h"

#include "AbilitySystem/LProjectAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/LProjectAttributeSet.h"

ALProjectCharacterBase::ALProjectCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	AbilitySystemComponent = CreateDefaultSubobject<ULProjectAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	// Single-player: cheapest mode. Switch to Mixed if ever extended to multiplayer with an owned ASC.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	// Created as a subobject of the ASC owner -> auto-registered with the ASC at init time.
	AttributeSet = CreateDefaultSubobject<ULProjectAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* ALProjectCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ALProjectCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitAbilityActorInfo();
}

void ALProjectCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	InitAbilityActorInfo();
}

void ALProjectCharacterBase::InitAbilityActorInfo()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}
