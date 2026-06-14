// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "LProjectCharacterBase.generated.h"

class ULProjectAbilitySystemComponent;
class ULProjectAttributeSet;

/**
 * Shared base for all GAS-driven characters (player avatar + raid boss).
 *
 * Owns the AbilitySystemComponent and the base attribute set, and implements
 * IAbilitySystemInterface so any GAS query can resolve the ASC from the actor.
 * ASC lives on the pawn (fine for single-player); revisit if respawn persistence is ever needed.
 */
UCLASS(Abstract)
class LPROJECT_API ALProjectCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ALProjectCharacterBase();

	//~ Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface

	ULProjectAbilitySystemComponent* GetLProjectAbilitySystemComponent() const
	{
		return AbilitySystemComponent;
	}
	const ULProjectAttributeSet* GetAttributeSet() const
	{
		return AttributeSet;
	}

protected:
	//~ Begin AActor/APawn interface
	virtual void PossessedBy(AController* NewController) override;
	virtual void BeginPlay() override;
	//~ End AActor/APawn interface

	/** Binds the ASC to this actor as both owner and avatar. Safe to call more than once. */
	void InitAbilityActorInfo();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<ULProjectAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<ULProjectAttributeSet> AttributeSet;
};
