# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project state

This is a **fresh Unreal Engine 5.7 C++ project**, generated from the Blank template (note the
`TP_Blank` → `LProject` game-name redirects in `Config/DefaultEngine.ini`). There is no gameplay
code yet beyond the default primary game module, and `Content/` is empty. Treat new work as
greenfield — establish patterns here.

## Directory layout gotcha

The repo root and the actual UE project are **nested**: the `.uproject` lives at
`D:\Claude\LProject\LProject\LProject.uproject`, one level below the outer folder. Run all UE
tooling from `D:\Claude\LProject\LProject` (the folder containing `LProject.sln`).

This **inner** folder (`D:\Claude\LProject\LProject`) is a git repository on branch `main`,
tracking `origin` = `https://github.com/RLSJJ/LProject.git`, with `.gitignore` + Git LFS
configured (`Binaries/`, `Intermediate/`, `DerivedDataCache/`, `Saved/`, and
`compile_commands.json` are ignored). Note the **outer** folder (`D:\Claude\LProject`, the Claude
Code working dir) is NOT the repo root — run git from the inner folder (or use `git -C`).

## Build, run, and regenerate

Engine is installed at `C:\Program Files\Epic Games\UE_5.7`. Use the engine's BatchFiles.
`<UE>` below = `C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles`.

```powershell
# Build the editor target (most common during development)
& "<UE>\Build.bat" LProjectEditor Win64 Development -Project="D:\Claude\LProject\LProject\LProject.uproject" -WaitMutex

# Build the standalone game target
& "<UE>\Build.bat" LProject Win64 Development -Project="D:\Claude\LProject\LProject\LProject.uproject" -WaitMutex

# Regenerate the Visual Studio solution after adding/removing source files or modules
& "<UE>\Build.bat" -projectfiles -project="D:\Claude\LProject\LProject\LProject.uproject" -game -engine
```

- After editing `.cpp`/`.h` while the editor is open, prefer **Live Coding** (Ctrl+Alt+F11 in the
  editor) for fast iteration. Changing headers/`UCLASS`/`UPROPERTY` layout or adding new
  `UObject` types generally requires a full rebuild with the editor closed.
- After adding/removing source files or changing a `.Build.cs`/`.Target.cs`, regenerate project
  files (command above) so the solution and UnrealBuildTool pick them up.
- There are **no automated tests, linter, or CI** configured in this project.

## Module & target structure

- Single runtime module **`LProject`** (`Source/LProject/`), declared as the primary game module
  via `IMPLEMENT_PRIMARY_GAME_MODULE` in `LProject.cpp`.
- Two build targets: `LProject` (Game) and `LProjectEditor` (Editor), both pinned to
  `BuildSettingsVersion.V6` and `EngineIncludeOrderVersion.Unreal5_7`.
- Module dependencies are declared in `Source/LProject/LProject.Build.cs`. Current public deps:
  `Core`, `CoreUObject`, `Engine`, `InputCore`, **`EnhancedInput`**. Add new module dependencies
  here (not via `#include` alone) or the link will fail.

## Engine configuration that shapes the project

`Config/DefaultEngine.ini` opts into a high-end, modern-desktop rendering stack. Keep new content
and code compatible with these — they are intentional choices, not defaults:

- **Substrate materials** enabled (`r.Substrate=True`) — material authoring differs from the
  legacy shading model.
- **Lumen** dynamic GI + reflections (`r.DynamicGlobalIlluminationMethod=1`, `r.ReflectionMethod=1`),
  with `r.AllowStaticLighting=False` — this is a fully dynamic-lighting project; do not rely on
  baked lightmaps.
- **Hardware ray tracing** (`r.RayTracing=True`) and **Virtual Shadow Maps**
  (`r.Shadow.Virtual.Enable=1`).
- **DirectX 12 / Shader Model 6** is the targeted RHI (Vulkan SM6 on Linux, Metal SM6 on Mac).
- Default map is the **Open World** template (`/Engine/Maps/Templates/OpenWorld`), so
  **World Partition** is in play — design levels as streamable/partitioned rather than monolithic.
- **Enhanced Input** is the input system (matching the `EnhancedInput` module dep). Use Input
  Mapping Contexts / Input Actions, not the legacy `PlayerInput` axis/action mappings.

## Plugins

Only `ModelingToolsEditorMode` is enabled, and **editor-only** (`TargetAllowList: ["Editor"]`).
When adding plugins, edit the `Plugins` array in `LProject.uproject`; for editor-only tooling
keep the `Editor` target allow-list so it does not ship in the game build.

## UE C++ conventions (for new code)

- Use Unreal type/prefix conventions: `U` for `UObject`-derived, `A` for `AActor`-derived,
  `F` for structs/plain types, `E` for enums, `I` for interfaces.
- Reflected members need `UCLASS`/`USTRUCT`/`UENUM`/`UPROPERTY`/`UFUNCTION` macros; non-reflected
  changes can hot-reload, reflection-layout changes cannot (see Build notes above).
- Use `TObjectPtr<>` for `UPROPERTY` object references in new headers (UE5 convention) rather than
  raw pointers.
