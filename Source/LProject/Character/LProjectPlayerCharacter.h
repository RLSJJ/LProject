// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/LProjectCharacterBase.h"
#include "LProjectPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class ULProjectGameplayAbility;
struct FInputActionValue;

/**
 * Player avatar: quarterview camera, world-relative movement, GAS-driven dash.
 *
 * Input assets (UInputMappingContext + UInputAction) and ability classes are assigned in the
 * editor on a Blueprint subclass / the CDO — this class only owns the logic.
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
	void Input_Dash(const FInputActionValue& Value);

	/** Grants DefaultAbilities + DashAbility. Authority only. */
	void GrantDefaultAbilities();

	// --- Camera ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> TopDownCamera;

	// --- Input (assign the assets in the editor) ---
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> DashAction;

	// --- Abilities ---
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TArray<TSubclassOf<ULProjectGameplayAbility>> DefaultAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<ULProjectGameplayAbility> DashAbility;
};
