# THE LAST SIGNAL — Project Guide

First-person survival horror, Unreal Engine 5.4, Windows PC.

## 0. Status — read this first

This project was written without access to Unreal Engine. The environment that produced it had no
UE install, no GPU and no Windows toolchain, so:

- **Nothing has been compiled.** The first build will likely surface a handful of errors. Fix them
  in the order the compiler reports them; the code is structured so each file stands on its own.
- **No packaged Windows build has been produced.** Section 3 explains how to create one.
- **The level builder has not been run.** It is written against the UE 5.4 Python API. Check its
  output in the viewport before trusting it.
- **There is no art, audio or music.** Geometry is engine cubes; interactables are placeholder
  cubes; creatures have no mesh. Every sound cue slot exists in the C++ and is empty.
- **The game is a vertical slice, not a full release.** It has four maps, one playable path from the
  forest to the vault, and the systems the brief asks for. Section 6 lists what is not built.

What is in place: the full C++ gameplay core (controller, character, flashlight, inventory,
interaction, doors, pickups, documents, puzzles, checkpoints and saves, creature AI with three
behaviour types, horror director, menus, settings, endings), level layouts for all maps, and the
config for input, rendering and collision.

## 1. Requirements

- Windows 10/11 64-bit, an RTX-class or equivalent GPU (Lumen and Nanite need one)
- Unreal Engine 5.4 (Epic Games Launcher)
- Visual Studio 2022 with the "Game development with C++" workload
- Enable in the Editor: **Python Script Plugin**, **Editor Scripting Utilities** (both are listed in
  `TheLastSignal.uproject`)

## 2. Project layout

```
TheLastSignal.uproject
Config/
  DefaultEngine.ini          Default map, game mode, collision profile, renderer flags
  DefaultInput.ini           Legacy input axes and actions
Content/Python/
  build_levels.py            Builds MainMenu, Forest_01, Facility_01, Underground_01
Source/TheLastSignal/
  Core/        GameMode, GameInstance (story flags, chosen ending)
  Player/      Character (movement, stamina, health, focus), Flashlight, Inventory, PlayerController
  Gameplay/    Interactables, doors, pickups, documents, puzzle terminal, checkpoints,
               level exits, ending terminal
  AI/          Creature (tuning, cosmetics) and CreatureController (state machine, perception)
  Horror/      Horror director (tension, flicker, ambient scares), per-level data actor
  Save/        Save game class and subsystem (checkpoints, cross-level restore)
  UI/          Menu, HUD, inventory widgets; graphics and audio settings library
Docs/README.md               This file
```

## 3. Setup and run

1. Right-click `TheLastSignal.uproject` > **Generate Visual Studio project files**.
2. Open the generated `.sln`, set the configuration to **Development Editor**, build.
   Alternatively, open the `.uproject` and let the editor prompt to rebuild.
3. Open the editor. Confirm the plugins from section 1 are enabled
   (Edit > Plugins).
4. Run the level builder: **Tools > Execute Python Script** > `Content/Python/build_levels.py`.
   This creates `/Game/Maps/MainMenu`, `Forest_01`, `Facility_01` and `Underground_01`, each with
   lighting, a navmesh volume, doors, pickups, documents, creatures and checkpoints.
   Alternatively, from a command prompt:
   ```
   "<UE>\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "<path>\TheLastSignal.uproject" ^
     -ExecutePythonScript="<path>\Content\Python\build_levels.py" -unattended -nop4 -nosplash
   ```
5. Complete the Blueprint and asset steps in section 5.
6. Play: **Play In Editor** from `MainMenu`.
7. Package for Windows: **Platforms > Windows > Package Project** (Shipping or Development).
   Use Development first; it keeps the console and logs visible.

Log category `LogTLS` prints from the C++ systems. Filter the Output Log by it when debugging.

## 4. Controls

| Action | Key |
|---|---|
| Move | W A S D |
| Look | Mouse |
| Run (drains stamina) | Left Shift (hold) |
| Crouch | Left Ctrl (toggle) |
| Jump | Space |
| Interact / read / use keypad | E |
| Flashlight on/off | F |
| Inventory and journal | Tab |
| Pause | Escape |

Light is the main tool against creatures: a sustained beam on a Stalker or Lurker makes it back away.
Hunters ignore light and follow sound, so keep noise down around them. Lurkers only strike in the dark.

## 5. Manual steps (Blueprints and assets)

The Python builder places actors. These steps need the editor because they involve assets, art or
Blueprint graphs. Each one is required for the game to be complete.

### 5.1 Game flow Blueprints

- **BP_TLSCharacter** (child of `TLSCharacter`). Holds the audio and surface settings in 5.4.
- **BP_TLSGameMode** (child of `TLSGameMode`). Set `Default Pawn Class` = BP_TLSCharacter. Then set
  it as the project GameMode (Project Settings > Maps & Modes > Default GameMode). The config file
  points at the C++ class, so the Blueprint child must replace it.
- **BP_TLSPlayerController** (child of `TLSPlayerController`). Set in its defaults:
  - `HUDWidgetClass` = WBP_HUD
  - `PauseMenuClass` = WBP_Menu (with `bIsPauseMenu` true on the instance, or leave it; the controller sets it)
  - `InventoryWidgetClass` = WBP_Inventory
  - `MainMenuClass` = WBP_MainMenu (used only on the MainMenu map)
  - Set this as the controller in the project: Project Settings > Maps & Modes > Player Controller
    Class = BP_TLSPlayerController. The GameMode is already `TLSGameMode`.

### 5.2 Widgets (UMG)

Create these Widget Blueprints. Names must match exactly, because C++ binds them by name.

- **WBP_Menu** (parent `TLSMenuWidget`). Add buttons named `NewGameButton`, `ContinueButton`,
  `ResumeButton`, `SettingsButton`, `MainMenuButton`, `QuitButton`, `BackButton`. Add a panel
  `MainPanel` holding the main buttons, and a panel `SettingsPanel` holding:
  `MasterVolumeSlider` (USlider), `QualitySlider` (USlider), `QualityLabel` (TextBlock),
  `FullscreenCheck` and `VSyncCheck` (CheckBox). Set `FirstLevelName` to `Forest_01`.
  Any button you leave out is simply not wired.
- **WBP_MainMenu** (parent `TLSMenuWidget`). Same as WBP_Menu but without `ResumeButton`
  and `MainMenuButton`. `NewGameButton` and `ContinueButton` are the important ones.
- **WBP_HUD** (parent `TLSHUDWidget`). Add `HealthBar`, `StaminaBar`, `BatteryBar` (ProgressBar),
  `PromptText`, `ObjectiveText`, `TensionText` (TextBlock). All optional.
- **WBP_Inventory** (parent `TLSInventoryWidget`). Add `ItemsText` and `JournalText` (TextBlock).
  Fill `ItemDisplayNames` and `DocumentTitles` in the defaults with your display strings.
- **WBP_Keypad** (plain widget). Build a 0–9 pad and a text readout. On submit, find the
  `ATLSPuzzleTerminal` and call `SubmitCode(Code)`. A `true` return means solved. Bind it from
  the terminal's `OnOpenKeypad` event in its Blueprint child.
- **WBP_Document** (plain widget). Shows `Title` and `Body`. Bind `OnDocumentRead` on each
  `TLSDocument` (or on a Blueprint child) to open it.
- **WBP_EndingChoice**. Built from `ATLSEndingTerminal::GetAvailableOptions` in its
  `OnShowChoices` event. Calls `ChooseEnding(Ending)`.

### 5.3 Blueprint children of gameplay actors

- **BP_Door** (child of `TLSDoor`): assign a door mesh, set the pivot to the hinge, set
  `OpenSound` and `LockedSound`. Placed doors in `Facility_01` use the placeholder cube for now.
- **BP_Pickup**: meshes for battery and key items. Set `PickupSound`.
- **BP_Creature** (child of `TLSCreature`): assign a skeletal mesh and an animation Blueprint, set
  `Vocals` (growls and breaths), `RecoilSound`, `AttackSound`. Implement `OnBehaviourChanged` to
  drive animation states (Dormant, Patrol, Investigate, Hunt, Search, Retreat). The enum is passed as
  a string, so `Hunt` is the string `"Hunt"`. Drop the BP onto the placeholder creatures in the
  level, or change the `creature()` calls in the builder to use BP_Creature.
- **BP_LevelDirector** is not needed: each level has a `TLSLevelDirector` actor placed by the
  builder. Add `AmbientScares` (distant metal, dripping, door creaks) and `NamedScares` there.

### 5.4 Settings the editor must hold

- **Physical surfaces** (Project Settings > Physics > Physical Surfaces). Name `SurfaceType1..7` as
  Concrete, Metal, Wood, Water, Gravel, Carpet, Glass in that order. The footstep code maps
  `SurfaceType1..7` to `ETLSSurface` by position. Assign physical materials to floor meshes.
- **Footsteps**: on `BP_TLSCharacter` (or the character's defaults), fill `FootstepSounds` with
  one cue per `ETLSSurface` value, including `Default`. Also set `BreathingLoop` and `HurtSound`.
- **Flashlight**: set `ClickSound` on the flashlight component.
- **Ending terminal**: in `Underground_01`, select the `TLSEndingTerminal` and fill `Options`:
  - Escape: `LevelName` = a level you build (e.g. `Ending_Escape`), `RequiredFlags` = `security_unlocked`
  - Transmit: `LevelName` = `Ending_Transmit`, `RequiredFlags` = `generator_on`, `security_unlocked`
  - Stay: `LevelName` = `Ending_Stay`, `RequiredFlags` = `generator_hint`
  Create the three ending maps (a cinematic and credits each) with a Level Sequence.
- **Keypad flags**: Facility's generator keypad uses code `2207`, the security keypad `4071`.
  The generator note's `FlagOnRead` sets `generator_hint`.
- **Lighting and rendering** (Project Settings > Engine > Rendering): Dynamic Global Illumination
  = Lumen, Reflections = Lumen, Virtual Shadow Maps on, Nanite on for imported meshes. The renderer
  defaults are in `DefaultEngine.ini`.

### 5.5 Art and audio replacement

Every interactable and creature uses a placeholder. Replace meshes in their Blueprint children
(section 5.3). Import Nanite-enabled meshes for the environment (`Build > Nanite` on the static
mesh asset), and use Lumen-friendly materials. For audio, import cues and assign them to the slots
above. Attenuation and reverb are per-cue settings: set spatialisation on each SoundCue or
MetaSound for the 3D positional audio the brief asks for.

## 6. What is implemented vs. not

**Implemented in C++ (needs first-compile verification):**

- First-person character with walk, run (stamina with lockout and recovery), crouch, interaction
  focus trace, footstep surfaces, breathing tied to tension
- Flashlight with battery drain, low-battery flicker, and light-based creature repulsion
- Inventory with stacking, slot limit, and a document journal
- Doors (open/close, key-locked, noise on use), pickups (items and batteries), documents, keypad
  puzzles with flag prerequisites, checkpoints, level exits with autosave
- Save and checkpoint system surviving level changes (flags, inventory, vitals, position)
- Death: fade, reload from last checkpoint, restart level if none
- Creature AI: state machine (Dormant, Patrol, Investigate, Hunt, Search, Retreat), sight and
  hearing perception, navmesh movement, three behaviour types, lurker attack only in darkness
- Horror director: tension from proximity, being hunted, health and darkness; flicker waves; ambient
  scares placed behind the player; named scares for story beats
- Menus: main, pause, inventory, HUD, settings (graphics quality, fullscreen, VSync, master volume),
  persisted to config
- Ending selection with flag requirements

**Not built (explicitly out of scope for this pass):**

- Art, meshes, materials, animation, and audio assets (slots exist; contents do not)
- Dynamic music system (no music manager; a MetaSound or music controller would hook into the
  director's `Tension` value)
- Cinematic sequences (Level Sequences are required; the director and story flags are ready to
  trigger them)
- Full story content. The documents in the builder are a sketch. The brief asks for a complete
  storyline; that is writing work, placed through `TLSDocument` and story flags
- Ending maps `Ending_Escape`, `Ending_Transmit` and `Ending_Stay`
- Packaged Windows build (produced by you, section 3)
- Surface-specific particle effects, controller rumble, and accessibility options beyond the
  settings listed above
- Automated tests

## 7. Troubleshooting

- **"Not compiling"**: the first build is the real test. Most likely issues are API names that have
  moved between minor UE 5.x versions (`UAISense::GetSenseID`, `UNavigationSystemV1` helpers,
  `FText::AsPercent`). Fix them against the engine headers in your install.
- **Keys do nothing**: confirm `DefaultInput.ini` is in `Config/` and the project uses legacy input
  classes (Edit > Project Settings > Input > Default Classes).
- **Creatures ignore the player**: the player needs `PerceptionSource` (added in code). Check the
  creature's perception in the editor debugger and that the navmesh bounds volume covers the area.
- **Interactables cannot be focused**: check the actor's collision profile is `Interactable` and
  Visibility is set to Block.
- **Level builder errors**: run it from Output Log and note the first failing call. Each helper is
  small, so the failing one is easy to fix. Set any enum or struct field that the Python API rejects
  by hand in the Details panel.
