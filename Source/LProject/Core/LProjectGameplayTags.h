// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

// Native gameplay tags for LProject. Prefer these over string-based FGameplayTag lookups.
// Add new tags here as systems grow (states, abilities, cues, mechanic phases).

/** Actor is immune to incoming damage (e.g. during a dash i-frame window). */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Invulnerable);
