// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/LProjectCharacterBase.h"
#include "GameplayTagContainer.h"
#include "LProjectPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class ULProjectPawnData;
struct FInputActionValue;

/**
 * Player avatar: quarterview camera, world-relative movement, and a data-driven GAS kit.
 *
 * Behaviour is defined by a ULProjectPawnData (ability set + input config + mapping context).
 * If PawnData is left empty, EnsureDefaultPawnData() builds a code-driven WASD + Space default so
 * the pawn is immediately playable; assigning a PawnData asset takes priority.
 */
UCLASS()
class LPROJECT_API ALProjectPlayerCharacter : public ALProjectCharacterBase
{
	GENERATED_BODY()

public:
	ALProjectPlayerCharacter();

protected:
	//~ Begin AActor/APawn interface
	virtual void PossessedBy(AController* NewController) override;
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	//~ End AActor/APawn interface

	void Move(const FInputActionValue& Value);
	void Input_AbilityTagPressed(const FInputActionValue& Value, FGameplayTag InputTag);
	void Input_AbilityTagReleased(const FInputActionValue& Value, FGameplayTag InputTag);

	/** Grants PawnData's ability set, or the code-default dash when no PawnData is set. Authority only. */
	void GrantAbilities();

	/** If PawnData is unset, build a code-driven default (WASD move + Space dash) so the pawn is playable. */
	void EnsureDefaultPawnData();

	// --- Camera ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> TopDownCamera;

	/** Placeholder visible body (engine cube) so the pawn is visible before a real mesh exists. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UStaticMeshComponent> DevVisualMesh;

	/** Data-driven definition of this pawn (abilities + input). Leave empty for the code default. */
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<ULProjectPawnData> PawnData;
};
