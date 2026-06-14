// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/LProjectCharacterBase.h"
#include "LProjectPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UInputMappingContext;
class UInputAction;
class ULProjectGameplayAbility;
struct FInputActionValue;

/**
 * Player avatar: quarterview camera, world-relative movement, GAS-driven dash.
 *
 * Input assets (UInputMappingContext + UInputAction) and ability classes can be assigned in the
 * editor on a Blueprint subclass / the CDO. If the input slots are left empty, EnsureDefaultInput()
 * builds a code-driven WASD + Space mapping at runtime so the pawn is immediately playable — the
 * production path is still to assign proper IMC/IA assets, which take priority when present.
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

	/** If the IMC/action slots are empty, construct a default WASD + Space mapping in code. */
	void EnsureDefaultInput();

	/** Grants DefaultAbilities + DashAbility. Authority only. */
	void GrantDefaultAbilities();

	// --- Camera ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> TopDownCamera;

	/** Placeholder visible body (engine cube) so the pawn is visible before a real mesh exists. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UStaticMeshComponent> DevVisualMesh;

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
