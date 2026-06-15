// Copyright Epic Games, Inc. All Rights Reserved.

#include "Boss/LProjectBossPatternData.h"

const FLProjectBossAttackPattern* ULProjectBossPatternData::SelectPattern(const FGameplayTagContainer& ActivePhaseTags,
    FRandomStream& Stream) const
{
	// Gather phase-valid patterns and sum their weights.
	TArray<const FLProjectBossAttackPattern*> Valid;
	float TotalWeight = 0.0f;
	for (const FLProjectBossAttackPattern& Pattern : Patterns)
	{
		const bool bPhaseOK = Pattern.RequiredPhaseTags.IsEmpty() || ActivePhaseTags.HasAll(Pattern.RequiredPhaseTags);
		if (bPhaseOK && Pattern.SelectionWeight > 0.0f)
		{
			Valid.Add(&Pattern);
			TotalWeight += Pattern.SelectionWeight;
		}
	}

	if (Valid.Num() == 0 || TotalWeight <= 0.0f)
	{
		return nullptr;
	}

	// Weighted pick.
	float Roll = Stream.FRandRange(0.0f, TotalWeight);
	for (const FLProjectBossAttackPattern* Pattern : Valid)
	{
		Roll -= Pattern->SelectionWeight;
		if (Roll <= 0.0f)
		{
			return Pattern;
		}
	}
	return Valid.Last();
}
