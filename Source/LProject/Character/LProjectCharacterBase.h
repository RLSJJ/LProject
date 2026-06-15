// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Combat/LProjectCombatInterface.h"
#include "LProjectCharacterBase.generated.h"

class ULProjectAbilitySystemComponent;
class ULProjectAttributeSet;
class USkeletalMesh;

/**
 * Shared base for all GAS-driven characters (player avatar + raid boss).
 *
 * Owns the AbilitySystemComponent and the base attribute set; implements IAbilitySystemInterface
 * (GAS lookup) and ILProjectCombatant (combat queries). ASC lives on the pawn (fine for
 * single-player); revisit if respawn persistence is ever needed.
 */
UCLASS(Abstract)
class LPROJECT_API ALProjectCharacterBase : public ACharacter, public IAbilitySystemInterface, public ILProjectCombatant
{
	GENERATED_BODY()

public:
	ALProjectCharacterBase();

	//~ Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface

	//~ Begin ILProjectCombatant
	virtual float GetHealth() const override;
	virtual float GetMaxHealth() const override;
	virtual bool IsAlive() const override;
	//~ End ILProjectCombatant

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

	/**
	 * Test-visual helper: puts a skeletal mesh on GetMesh(), auto-fitting its scale to TargetHeightCm
	 * from the asset bounds (handles arbitrary glTF import scale) and aligning the feet to the capsule
	 * bottom. Returns true if applied.
	 */
	bool ConfigureTestVisualMesh(USkeletalMesh* VisualMesh, float TargetHeightCm, const FRotator& MeshRotation);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<ULProjectAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<ULProjectAttributeSet> AttributeSet;
};
