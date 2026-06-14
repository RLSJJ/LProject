# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project state

A **UE5.7 C++ project** built as a portfolio piece: a **single-player recreation of a Lost Ark-style
boss raid** (north-star boss: **Behemoth** — chosen because it is the canonical "no-support" raid, so
solo balancing is natural). Multiplayer is intentionally out of scope; that budget goes into
raid-mechanic depth. Originally from the Blank template (hence the `TP_Blank` → `LProject` game-name
redirects in `Config/DefaultEngine.ini`).

**Current state — Phase 0-1 + Phase 1 done, build-verified.** GAS is enabled and a working foundation
exists: a quarterview player character with Enhanced Input movement and a GAS dash ability (i-frames).
`Content/` is still empty — input/mesh assets are a TODO in the editor (see *Gameplay architecture*).
Roadmap: **Phase 2** boss core (multi-bar HP, stagger, data-driven patterns, telegraphs) → **Phase 3**
raid systems (encounter director, stagger check, part-break, counter, wipe-gimmick framework) →
**Phase 4** Behemoth encounter → **Phase 5** UI/HUD + polish.

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
  (asset manager, game mode, native gameplay tags), `AbilitySystem/` (+ `Abilities/`, `Attributes/`),
  `Character/`, `Player/`.
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
- **`ULProjectAttributeSet`** (`AbilitySystem/Attributes/`) — base attributes (`Health`, `MaxHealth`)
  with full replication (`OnRep` + `DOREPLIFETIME_CONDITION_NOTIFY(... REPNOTIFY_Always)`) and clamping
  in `PreAttributeChange`. Uses the standard `ATTRIBUTE_ACCESSORS` macro (defined in the header). Add
  Stagger/resource/part-durability as separate `UAttributeSet` subclasses later.
- **`ULProjectGameplayAbility`** (`AbilitySystem/Abilities/`) — ability base (`InstancedPerActor`,
  `LocalPredicted`).
- **`ULProjectGA_Dash`** — first ability: `LaunchCharacter` along move/facing direction + i-frames via
  the `TAG_State_Invulnerable` loose tag, ended on a timer.
- **`ALProjectPlayerCharacter`** (`Character/`) — quarterview avatar: SpringArm + Camera (fixed angle),
  world-relative WASD via Enhanced Input, `bOrientRotationToMovement`, plus a placeholder cube mesh
  (`DevVisualMesh`). Driven by a `ULProjectPawnData`; input is bound by tag. If no PawnData is assigned,
  `EnsureDefaultPawnData()` builds a WASD+Space default in code so it is playable with zero editor assets.
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

**To play:** works out of the box (code-default WASD + Space + cube mesh). Just ensure the play map uses
`LProjectGameMode` (no GameMode override) and has a PlayerStart. **For production**, author real
`IA_*`/`IMC_*` assets + an AbilitySet/PawnData and assign PawnData on a BP subclass — the code default
is only a stopgap.

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
