# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project state

A **UE5.7 C++ project** built as a portfolio piece: a **single-player recreation of a Lost Ark-style
boss raid** (north-star boss: **Behemoth** — chosen because it is the canonical "no-support" raid, so
solo balancing is natural). Multiplayer is intentionally out of scope; that budget goes into
raid-mechanic depth. Originally from the Blank template (hence the `TP_Blank` → `LProject` game-name
redirects in `Config/DefaultEngine.ini`).

**Current state — full vertical slice, build- and smoke-verified.** On top of the base systems sits a
complete front-to-back experience: a **GameInstance flow state machine** (Boot→Title→Ready→Encounter→
Result, guarded legal transitions) driving **C++ UMG screens** + an in-fight **UMG raid HUD** (boss
multi-bar HP, stagger/groggy, identity gauge, QWER cooldowns, enrage clock) over a **PNG-texture UI**
pipeline. Combat: a single SetByCaller/exec-calc **damage pipeline** (incl. **true damage**), player
**basic attack** + a **QWER skill kit** with an **Identity/awakening** resource (all via GEs), a
**counter** window (decoupled through the combatant interface), and a **combat-feel layer**
(`Feedback/`): camera shake, hit-stop, floating damage numbers, and a mesh-squash hit reaction, all
fanned out from the one damage seam. The **boss** moves (chases/repositions, no longer a turret), reads
its attacks with a procedural rear-up→slam **tell**, runs **stagger→groggy→2×**, **positional part-break**
(Head/Core/Tail by facing), and a **mechanic vocabulary** beyond dodge-the-AoE: **knockback** and
**safe-zone (stand-in)** patterns. The **EncounterDirector** runs **real phases** (phase-gated movesets +
an untargetable **phase-transition roar**) and a **soft-enrage** (Enraged buff + faster cadence) before a
hard wipe. Characters use imported glTF test meshes; UI uses generated PNGs. Visuals are greybox-plus
(debug-draw telegraphs, procedural feel) — **authored montages, Niagara VFX, SFX, and a Behemoth-grade
mechanic script are the remaining production work** (hooks are in place: GameplayCue-ready feedback,
`HitSound`/shake-class data slots, montage-shaped ability timing).

Remaining roadmap: **art pass** (authored attack montages + hit-react anims, telegraph/impact materials,
Niagara VFX, SFX), **authored encounter content** (signature Behemoth gimmicks, scripted phase sequences),
and a demo video. See `ARCHITECTURE_REVIEW.md` for the standing self-assessment.

## Directory layout gotcha

The repo root and the actual UE project are **nested**: the `.uproject` lives at
`D:\Claude\LProject\LProject\LProject.uproject`, one level below the outer folder. Run all UE
tooling from `D:\Claude\LProject\LProject` (the folder containing `LProject.sln`).

This **inner** folder is a git repository on branch `main`, tracking
`origin` = `https://github.com/RLSJJ/LProject.git`, with `.gitignore` + Git LFS configured
(`Binaries/`, `Intermediate/`, `DerivedDataCache/`, `Saved/`, and `compile_commands.json` are ignored;
`.uasset`/`.umap`/media go through LFS). The **outer** folder (`D:\Claude\LProject`, the Claude Code
working dir) is NOT the repo root — run git from the inner folder, or use `git -C`.

## Build, run, and regenerate

Engine is installed at `C:\Program Files\Epic Games\UE_5.7`. Use the engine's BatchFiles.
`<UE>` below = `C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles`.

```powershell
# Build the editor target (most common during development)
& "<UE>\Build.bat" LProjectEditor Win64 Development -Project="D:\Claude\LProject\LProject\LProject.uproject" -WaitMutex

# Build the standalone game target
& "<UE>\Build.bat" LProject Win64 Development -Project="D:\Claude\LProject\LProject\LProject.uproject" -WaitMutex

# Regenerate the Visual Studio solution after adding/removing source files or modules
& "<UE>\Build.bat" -projectfiles -project="D:\Claude\LProject\LProject\LProject.uproject" -game

# Regenerate clangd's compile database after adding/removing source files (drives IDE intelligence)
& "<UE>\Build.bat" LProjectEditor Win64 Development -Project="D:\Claude\LProject\LProject\LProject.uproject" -mode=GenerateClangDatabase -OutputDir="D:\Claude\LProject\LProject"
```

- After editing `.cpp`/`.h` while the editor is open, prefer **Live Coding** (Ctrl+Alt+F11). Changing
  headers/`UCLASS`/`UPROPERTY` layout or adding new `UObject` types needs a full rebuild with the
  editor closed.
- After adding/removing source files or changing a `.Build.cs`/`.Target.cs`: regenerate project files
  **and** the clang database (commands above).
- No automated tests, linter, or CI are configured.

## Module & target structure

- Single runtime module **`LProject`** (`Source/LProject/`), primary game module via
  `IMPLEMENT_PRIMARY_GAME_MODULE` in `LProject.cpp`. Source is organized into folders: `Core/`
  (asset manager, game modes, native gameplay tags, pawn data), `AbilitySystem/` (+ `Abilities/`,
  `Attributes/`, `Calculations/` exec calcs, `Effects/` GEs), `Character/`, `Player/`, `Input/`,
  `Combat/` (combatant interface), `Boss/` (boss character + pattern/part-break components + pattern
  data), `Telegraph/` (AoE warning actor), `Encounter/` (director subsystem + encounter game mode),
  `Flow/` (game-flow GameInstance subsystem), `Feedback/` (camera shake + hit-stop + damage numbers +
  hit-react combat-feel layer), `UI/` (UMG raid HUD + `Screens/` menu widgets + `Style/` palette).
- Two build targets: `LProject` (Game) and `LProjectEditor` (Editor), pinned to
  `BuildSettingsVersion.V6` and `EngineIncludeOrderVersion.Unreal5_7`.
- Public deps in `LProject.Build.cs`: `Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`,
  and the GAS trio `GameplayAbilities`, `GameplayTags`, `GameplayTasks`. Add new module deps here
  (not via `#include` alone) or the link fails.
- `LProject.Build.cs` adds `PublicIncludePaths.Add(ModuleDirectory)` so subfolder files use
  **module-root-relative includes** (e.g. `#include "AbilitySystem/Attributes/LProjectAttributeSet.h"`).
  Without it a `""` include only searches the including file's own directory → subfolder files fail
  with `C1083`.

## Gameplay architecture (GAS)

Combat is built on the **Gameplay Ability System**. Key classes (all prefixed `LProject`):

- **`ALProjectCharacterBase`** (`Character/`) — shared base for player + boss. Owns the
  `AbilitySystemComponent` and a `ULProjectAttributeSet` (both default subobjects → auto-registered),
  implements `IAbilitySystemInterface`, calls `InitAbilityActorInfo` in `PossessedBy`/`BeginPlay`.
  ASC replication mode is `Minimal` (single-player).
- **`ULProjectAbilitySystemComponent`** (`AbilitySystem/`) — ASC subclass. `AbilityInputTagPressed(tag)`
  activates the granted ability whose spec carries that input tag (tag-driven input). Future home for
  raid-mechanic helpers.
- **`ULProjectAttributeSet`** (`AbilitySystem/Attributes/`) — shared combat attributes: `Health`,
  `MaxHealth`, `AttackPower`, `Defense` (all replicated), plus a transient **`Damage` meta-attribute**
  (NOT replicated). `PostGameplayEffectExecute` turns incoming `Damage` into a `Health` subtraction and
  tags the owner `State.Dead` at 0. Clamps in `PreAttributeChange`. `ATTRIBUTE_ACCESSORS` macro in the
  header. **`ULProjectBossAttributeSet`** adds boss-only `StaggerCurrent`/`StaggerMax`.
- **`ULProjectGameplayAbility`** (`AbilitySystem/Abilities/`) — ability base (`InstancedPerActor`,
  `LocalPredicted`).
- **`ULProjectGA_Dash`** — first ability: `LaunchCharacter` along move/facing direction + i-frames via
  the `TAG_State_Invulnerable` loose tag, ended on a timer.
- **`ALProjectPlayerCharacter`** (`Character/`) — quarterview avatar: SpringArm + Camera (fixed angle),
  **RMB click-to-move** (steer-while-held + click-to-point auto-run) via Enhanced Input,
  `bOrientRotationToMovement`, an imported glTF test mesh (CesiumMan) over a fallback cube. Driven by a
  `ULProjectPawnData`; input is bound by tag. If no PawnData asset is assigned, `EnsureDefaultPawnData()`
  builds the default control scheme in code (RMB move, LMB 평타, Space dash, F counter, Q/W/E/R skills) so
  it is playable with zero editor assets.
- **`ALProjectPlayerController`** / **`ALProjectGameMode`** (`Player/`, `Core/`) — controller skeleton;
  game mode wires pawn + controller and is the `GlobalDefaultGameMode`.
- **`ULProjectAssetManager`** (`Core/`) — forces `UAbilitySystemGlobals::Get().InitGlobalData()` in
  `StartInitialLoading` (deterministic GAS init; auto-called in 5.3+ but explicit here). Registered via
  `[/Script/Engine.Engine] AssetManagerClassName`.
- **Native gameplay tags** — `UE_DECLARE_GAMEPLAY_TAG_EXTERN` / `UE_DEFINE_GAMEPLAY_TAG` in
  `Core/LProjectGameplayTags.{h,cpp}` (state + `InputTag.*`). Prefer these over string lookups.

**Data-driven content pipeline** — kits and input are data, not hardcoded:
- **`ULProjectAbilitySet`** (`AbilitySystem/`) — grantable bundle of abilities (each with an `InputTag`)
  + attribute sets; `GiveToAbilitySystem(ASC)` grants them.
- **`ULProjectInputConfig`** (`Input/`) — maps `UInputAction` ↔ `InputTag` (native + ability inputs).
- **`ULProjectPawnData`** (`Core/`) — bundles AbilitySet + InputConfig + mapping context = one pawn
  archetype; the player and each boss reference one.
- **`ILProjectCombatant`** (`Combat/`) — health/alive contract on `ALProjectCharacterBase`.
- **Adding an ability = make a `UGameplayAbility` + add it to an AbilitySet + map an input in an
  InputConfig. No character code changes.** Activation: input (InputTag) → `AbilityInputTagPressed` →
  the spec tagged with that InputTag activates.

**Combat & boss systems** — all routed through one GAS damage seam:
- **Damage pipeline:** every attacker (player 평타, boss patterns, counter, DoT) builds a SetByCaller
  `ULProjectGE_Damage` and applies it; `ULProjectExecCalc_Damage` (`AbilitySystem/Calculations/`) is the
  one formula — `Base * AttackPower/100 * (1 - clamp(Defense/100, 0, .95))`, ×2 vs a groggy boss. It
  outputs the `Damage` meta-attribute (HP) and a negative `StaggerCurrent` modifier (only if the target
  has the boss set). GEs live in `AbilitySystem/Effects/` (`…GE_Damage`, `…GE_DamageOverTime`,
  `…GE_AttackUp`, base `…GameplayEffect`).
- **Player attacks:** `ULProjectGA_BasicAttack` (left mouse) box-overlaps Pawns in front (no custom
  collision channel — `OverlapMultiByObjectType(ECC_Pawn)`), de-dupes, applies the damage GE.
  `ULProjectGA_Counter` (F) only lands while a nearby counterable combatant has `State.Boss.Counterable`:
  it overlap-finds the target via the `ILProjectCombatant` interface, interrupts its pattern
  (`NotifyCountered`), bursts stagger, applies a bleed DoT, deals true (Defense-ignoring) chip, self-buffs,
  and grants i-frames. Dash i-frames (`State.Invulnerable`) make hits whiff — checked by every attacker
  (basic attack, skills, boss strikes). A **QWER skill kit** (`LProjectSkills`) builds/spends an
  **Identity** awakening resource, all routed through GEs (`GE_IdentityGain`, no raw attribute writes).
- **Boss** (`Boss/LProjectBossCharacter`) — subclasses `ALProjectCharacterBase`; grants the boss
  attribute set in code, faces the player each tick AND **repositions** (chases/backs off to a preferred
  range while free), plays a procedural **attack tell** (rears up through the telegraph, slams on the
  strike), exposes multi-bar-HP helpers, and runs **groggy/무력화** (stagger→0 pauses the runner + 2×
  damage for `GroggyDuration`, then refills). Health loss feeds **`ULProjectPartBreakComponent`** —
  **positional** part-break: damage routes to the part facing the attacker (Head/Core/Tail), and a break
  permanently lowers Defense (`ResetParts()` re-arms on retry).
- **Patterns** — `ULProjectBossPatternData` (`FLProjectBossAttackPattern` rows: shape, size, target mode,
  timings, damage/stagger, counterable, weight, required phase tags) is run by
  **`ULProjectBossPatternRunnerComponent`**, an Idle→Telegraph→Strike→Recovery FSM that spawns a
  greybox **`ALProjectTelegraphActor`** (debug-draw circle/box/cone, fill animates to the strike) then
  overlaps + applies damage. Falls back to built-in default patterns when no DataAsset is set.
- **Encounter** — **`ULProjectEncounterDirector`** (`UTickableWorldSubsystem`) registers boss+player,
  fires HP-gated **phases** (accumulating phase tags unlock phase-gated movesets; crossing a phase plays
  an untargetable **roar**), runs the enrage timer (**soft-enrage** Enraged buff + faster cadence at
  `SoftEnrageSeconds`, hard wipe at 0), and resolves win/lose/retry/abort (exposes `GetOutcome()`,
  delegates). **`ALProjectEncounterGameMode`** spawns + freezes the boss on BeginPlay and hands first-frame
  control to the **flow subsystem** (it does NOT auto-start the fight; `lp.SkipFrontEnd 1` drops straight
  into combat for dev).
- **`ULProjectGameFlowSubsystem`** (`Flow/`) — the single flow owner (GameInstance subsystem, survives
  world reload): a guarded state machine (Boot→Title→Ready→Encounter→Result), the only caller of the
  director's Start/Retry/Abort, swaps the full-screen menu widget + the UMG raid HUD per state, and applies
  the per-state input/cursor/pause contract.
- **`ULProjectRaidHUD`** (`UI/`) — C++ UMG over a PNG-texture style (`UI/Style`): boss name plate +
  multi-bar HP, stagger/groggy, identity gauge, 7-slot skill bar with cooldown readouts, enrage clock,
  player vitals, counter prompt. Shown by the flow during Encounter. (The old `ALProjectDebugHUD` was
  removed.)

**To play:** press Play — the **encounter game mode** (`GlobalDefaultGameMode`) spawns the boss + freezes
it, then the **flow subsystem** runs Title→Ready→Encounter→Result. Controls: **RMB** move, **LMB** 평타,
**Space** dash (i-frames), **F** counter (during the cyan COUNTER! prompt), **Q/W/E/R** skills (R =
awakening at full Identity), **R-key** retry **on the Result screen**. `lp.SkipFrontEnd 1` drops straight
into a live fight. Everything works with **zero authored assets**. Swap `GlobalDefaultGameMode` back to
`LProjectGameMode` for boss-free movement testing. **For production**, author real meshes/montages/VFX +
tuned `UBossPatternData`/`PawnData`/phase DataAssets; the code defaults are a stopgap.

## Engine configuration that shapes the project

`Config/DefaultEngine.ini` opts into a high-end, modern-desktop rendering stack. Keep new content and
code compatible — these are intentional choices, not defaults:

- **Substrate materials** (`r.Substrate=True`) — authoring differs from the legacy shading model.
- **Lumen** dynamic GI + reflections, with `r.AllowStaticLighting=False` — fully dynamic lighting; no
  baked lightmaps.
- **Hardware ray tracing** (`r.RayTracing=True`) and **Virtual Shadow Maps** (`r.Shadow.Virtual.Enable=1`).
- **DirectX 12 / Shader Model 6** RHI (Vulkan SM6 on Linux, Metal SM6 on Mac).
- Default map is the **Open World** template, so **World Partition** is in play — design streamable /
  partitioned levels.
- **Enhanced Input** is the input system. Use Input Mapping Contexts / Input Actions, not legacy
  `PlayerInput` mappings.
- `LProjectGameMode` is the `GlobalDefaultGameMode`; `LProjectAssetManager` is the `AssetManagerClassName`.

## Plugins

Enabled plugins (`Plugins` array in `LProject.uproject`):

- **`GameplayAbilities`** — the Gameplay Ability System (GAS). Enabling it auto-enables its
  dependencies (GameplayTagsEditor, Niagara, DataRegistry).
- **`ModelingToolsEditorMode`** — editor-only (`TargetAllowList: ["Editor"]`).

When adding plugins, edit that array; keep an `Editor` allow-list for editor-only tooling so it does
not ship in the game build.

## Local tooling (formatting & clangd intelligence)

Configured in the **outer** Claude Code dir (`D:\Claude\LProject\.claude\`) plus project-root files:

- **clang-format on save** — a PostToolUse hook formats every edited C/C++ file using `.clang-format`
  (UE style: Allman braces, hard tabs, `SortIncludes: false`). clang-format resolves from PATH, falling
  back to the VS-bundled LLVM.
- **clangd LSP** — the `clangd-lsp` plugin provides code intelligence, driven by `compile_commands.json`
  at the project root (git-ignored; **regenerate after adding source files** — command above). clangd is
  LLVM 22.x at `C:\Program Files\LLVM\bin`.

## UE C++ conventions (for new code)

- Prefixes: `U` (UObject), `A` (AActor), `F` (struct/plain), `E` (enum), `I` (interface). Project
  classes are prefixed `LProject`.
- Reflected members need `UCLASS`/`USTRUCT`/`UENUM`/`UPROPERTY`/`UFUNCTION`; non-reflected changes can
  hot-reload, reflection-layout changes cannot.
- Use `TObjectPtr<>` for `UPROPERTY` object refs in new headers (not raw pointers).
- **GAS-first**: model combat as abilities / effects / attributes / tags. Use native gameplay tags, not
  strings. New attribute sets / abilities live under `AbilitySystem/`.
- **Data-driven**: prefer `UDataAsset` / `UDataTable` for tunable content (boss patterns, ability params)
  so it can be extended without code.
- Place new files in the right module folder and use module-root-relative includes.
