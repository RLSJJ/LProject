// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

// Native gameplay tags for LProject. Prefer these over string-based FGameplayTag lookups.
// Add new tags here as systems grow (states, abilities, cues, mechanic phases).

// --- Combat states ---------------------------------------------------------------------------------
/** Actor is immune to incoming damage (e.g. during a dash i-frame or a successful counter window). */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Invulnerable);
/** Actor is dead (Health reached 0): blocks input/abilities and pattern execution. */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Dead);
/** Generic crowd-control / flinch state (e.g. boss briefly stunned by a counter). */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Stunned);

// --- Boss states -----------------------------------------------------------------------------------
/** Boss stagger gauge depleted -> groggy/무력화: pattern runner paused, takes bonus damage. */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Boss_Groggy);
/** Set while a counterable telegraph window is open; required by the counter ability. */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Boss_Counterable);
/** Boss enrage active (post-timer escalation). */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Boss_Enraged);
/** Boss has at least one broken part (HUD/loot hook). */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Boss_PartBroken);

// --- SetByCaller magnitude keys (assigned on a damage GE spec at runtime) ---------------------------
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SetByCaller_Damage);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SetByCaller_StaggerDamage);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SetByCaller_CooldownDuration);

// --- Damage types ----------------------------------------------------------------------------------
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_Type_Physical);
/** True damage ignores Defense (used by the counter burst, executes, etc.). */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_Type_True);

// --- Encounter phases (swapped by the EncounterDirector, read by the pattern runner) ---------------
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Phase_1);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Phase_2);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Phase_3);

// --- Buffs / debuffs (granted by stat-modifier GEs, shown on the HUD) ------------------------------
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Buff_AttackUp);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Debuff_DefenseDown);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Debuff_DoT_Bleed);

// --- Input tags: an InputConfig maps a UInputAction to one of these; abilities activate by tag. ----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InputTag_Move);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InputTag_Dash);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InputTag_BasicAttack);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InputTag_Counter);
// QWER skill kit.
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InputTag_SkillQ);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InputTag_SkillW);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InputTag_SkillE);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InputTag_SkillR);

// --- Cooldown tags: each skill's cooldown GE grants its tag; the HUD reads remaining time by tag. ---
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cooldown_Dash);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cooldown_Charge);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cooldown_Cleave);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cooldown_Bolt);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cooldown_Awakening);
