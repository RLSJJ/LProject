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
 * Player avatar: quarterview camera, Lost Ark-style click-to-move (right mouse), and a data-driven
 * GAS kit.
 *
 * Movement: hold/tap right mouse to move toward the cursor (straight-line; no NavMesh needed). The
 * kit and input bindings come from a ULProjectPawnData; an empty PawnData falls back to a code
 * default (RMB move + Space dash) so the pawn is playable with zero editor assets.
 */
UCLASS()
class LPROJECT_API ALProjectPlayerCharacter : public ALProjectCharacterBase
{
	GENERATED_BODY()

public:
	ALProjectPlayerCharacter();

	virtual void Tick(float DeltaSeconds) override;

protected:
	//~ Begin AActor/APawn interface
	virtual void PossessedBy(AController* NewController) override;
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	//~ End AActor/APawn interface

	// Click-to-move: while right mouse is held, steer toward the cursor; a short tap walks to the point.
	void OnSetDestinationTriggered(const FInputActionValue& Value);
	void OnSetDestinationReleased(const FInputActionValue& Value);

	void Input_AbilityTagPressed(const FInputActionValue& Value, FGameplayTag InputTag);
	void Input_AbilityTagReleased(const FInputActionValue& Value, FGameplayTag InputTag);

	/** Grants PawnData's ability set, or the code-default dash when no PawnData is set. Authority only. */
	void GrantAbilities();

	/** If PawnData is unset, build a code-driven default (RMB move + Space dash) so the pawn is playable. */
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

	// --- Click-to-move tuning ---
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float ShortPressThreshold = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float DestinationAcceptanceRadius = 80.0f;

	// Runtime click-to-move state.
	FVector CachedDestination = FVector::ZeroVector;
	float FollowTime = 0.0f;
	bool bAutoRunToDestination = false;
};
