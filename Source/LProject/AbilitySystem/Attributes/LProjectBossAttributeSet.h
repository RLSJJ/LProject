// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attributes/LProjectAttributeSet.h"
#include "LProjectBossAttributeSet.generated.h"

/**
 * Boss-only attributes, granted on top of ULProjectAttributeSet. Holds the stagger/무력화 gauge that
 * the player depletes to force a groggy window. Reuses the ATTRIBUTE_ACCESSORS macro from the base set.
 *
 * Stagger model: StaggerCurrent starts at StaggerMax and is reduced by StaggerDamage from hits; when it
 * reaches 0 the boss enters groggy (driven by ALProjectBossCharacter, which watches the attribute), then
 * refills on recovery.
 */
UCLASS()
class LPROJECT_API ULProjectBossAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	ULProjectBossAttributeSet();

	//~ Begin UAttributeSet interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	//~ End UAttributeSet interface

	/** Remaining stagger before groggy. Reaches 0 -> groggy/무력화 window opens. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_StaggerCurrent, Category = "Stagger")
	FGameplayAttributeData StaggerCurrent;
	ATTRIBUTE_ACCESSORS(ULProjectBossAttributeSet, StaggerCurrent);

	/** Maximum stagger; StaggerCurrent refills to this after a groggy window ends. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_StaggerMax, Category = "Stagger")
	FGameplayAttributeData StaggerMax;
	ATTRIBUTE_ACCESSORS(ULProjectBossAttributeSet, StaggerMax);

protected:
	UFUNCTION()
	void OnRep_StaggerCurrent(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_StaggerMax(const FGameplayAttributeData& OldValue);
};
