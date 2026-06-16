// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/LProjectGameplayTags.h"

// Combat states
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Invulnerable, "State.Invulnerable");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Dead, "State.Dead");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Stunned, "State.Stunned");

// Boss states
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Boss_Groggy, "State.Boss.Groggy");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Boss_Counterable, "State.Boss.Counterable");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Boss_Enraged, "State.Boss.Enraged");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Boss_PartBroken, "State.Boss.PartBroken");

// SetByCaller magnitude keys
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Damage, "SetByCaller.Damage");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_StaggerDamage, "SetByCaller.StaggerDamage");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_CooldownDuration, "SetByCaller.CooldownDuration");

// Damage types
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Type_Physical, "Damage.Type.Physical");
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Type_True, "Damage.Type.True");

// Encounter phases
UE_DEFINE_GAMEPLAY_TAG(TAG_Phase_1, "Phase.1");
UE_DEFINE_GAMEPLAY_TAG(TAG_Phase_2, "Phase.2");
UE_DEFINE_GAMEPLAY_TAG(TAG_Phase_3, "Phase.3");

// Buffs / debuffs
UE_DEFINE_GAMEPLAY_TAG(TAG_Buff_AttackUp, "Buff.AttackUp");
UE_DEFINE_GAMEPLAY_TAG(TAG_Debuff_DefenseDown, "Debuff.DefenseDown");
UE_DEFINE_GAMEPLAY_TAG(TAG_Debuff_DoT_Bleed, "Debuff.DoT.Bleed");

// Input tags
UE_DEFINE_GAMEPLAY_TAG(TAG_InputTag_Move, "InputTag.Move");
UE_DEFINE_GAMEPLAY_TAG(TAG_InputTag_Dash, "InputTag.Dash");
UE_DEFINE_GAMEPLAY_TAG(TAG_InputTag_BasicAttack, "InputTag.BasicAttack");
UE_DEFINE_GAMEPLAY_TAG(TAG_InputTag_Counter, "InputTag.Counter");
UE_DEFINE_GAMEPLAY_TAG(TAG_InputTag_SkillQ, "InputTag.SkillQ");
UE_DEFINE_GAMEPLAY_TAG(TAG_InputTag_SkillW, "InputTag.SkillW");
UE_DEFINE_GAMEPLAY_TAG(TAG_InputTag_SkillE, "InputTag.SkillE");
UE_DEFINE_GAMEPLAY_TAG(TAG_InputTag_SkillR, "InputTag.SkillR");

// Cooldown tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Cooldown_Dash, "Cooldown.Dash");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cooldown_Charge, "Cooldown.Charge");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cooldown_Cleave, "Cooldown.Cleave");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cooldown_Bolt, "Cooldown.Bolt");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cooldown_Awakening, "Cooldown.Awakening");
