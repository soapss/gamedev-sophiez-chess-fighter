# Chess Fighter - Bot Battle: Chess + Street Fighter Capture Duels

One-paragraph description: Small, polished, complex one-shot for ADO GameDev Track. Chess where every capture requires winning a street-fighter-style duel. Foundation phase: chess board only (8x8, legal move gen, check/mate detection). This will grow into full chess+fighter with bot minimax depth 3 and per-piece specials. Bot-first to keep scope task-worthy for one-shot — captures push Avocado beyond simple chess. Human-designed per track hard rule (no models for ideation).

This repo follows the ADO **GameDev track** task structure: one repo per game (`gamedev-{game-name}`), a single shared gold source in `src/` + `project.godot`, and one folder per task under `tasks/`.

## Project Structure

```text
gamedev-sophiez-chess-fighter/
├── project.godot         Godot 4.7 project config (root for res://)
├── src/                  Gold game source — buildable game (single source of truth)
│   ├── main.gd           Main Control, chess board UI, selection, legal highlights
│   ├── chess/            board.gd, piece.gd, move.gd (full chess logic)
│   ├── assets/           icon.svg placeholder
│   └── scenes/Main.tscn  Main scene (StatusLabel + BoardContainer)
├── tasks/                Future one-shot task(s) will live here
└── README.md             This file — game-level overview
```

Notes: `src/` + `project.godot` holds the complete buildable gold game; tasks do NOT copy it. Videos go to PixelCloud, not repo.

## Game Loop (current foundation)
1. Chess board 8x8, White vs Black turn alternation
2. Click piece to select, see legal moves highlighted (green empty, red capture)
3. Legal move filtering includes king safety, en passant, castling with check squares
4. Check/checkmate/stalemate detection ends game

Future one-shot loop:
5. Attempt capture → triggers fighter arena overlay
6. Fighter: attacker vs defender with per-piece HP & special (Pawn Rush, Knight Leap, etc)
7. Controls: A/D move, J/K/U/I attacks, Shift block, Space special
8. Attacker must win fighter to capture succeed

## Engine & Framework

- **Engine / framework:** Godot 4.7 (GDScript)
- **License:** MIT (https://github.com/godotengine/godot/blob/master/LICENSE.txt) — open-source, permits commercial/internal use, compliant with track (no Unity/Unreal)

## Dependencies

| Library | Version | Source | License |
| --- | --- | --- | --- |
| Godot Engine | 4.7.x | https://godotengine.org/download/archive/4.7 | MIT |
| None beyond engine | - | - | - |

All pinned: Godot 4.7 per project.godot `config_version=5` and `config/features=4.7`. No third-party libs.

## Assets & Attribution

| Asset / Folder | Type | Source | License / Attribution |
| --- | --- | --- | --- |
| `src/assets/icon.svg` | icon | original (placeholder triangle primitive) | internal-use, original |
| `src/` chess visuals | primitives | original — ColorRect tiles, Label Unicode chess symbols ♔♕♖♗♘♙ | original |
| Fighter UI (future) | primitives | original — ProgressBar HP, Label, ColorRect | original |
| No external art/audio | - | All assets are original primitives | No third-party assets — complies with [Asset Guidelines](https://fb.workplace.com/groups/1939643316691589/permalink/1966187604037160/) |

No third-party tokens, proprietary code, or IP appear in the code, assets, or model-visible environment; all third-party material attributed above. No 3P-generated assets (per Manuela FAQ — no ai_asset_gen etc). Built with latest 1P model (Muse Spark 1.1 / Avocado) or by hand — never 3P.

## Building & Running

**Prerequisites:** Godot 4.7.x (https://godotengine.org/download/archive), modern desktop OS (Mac/Win/Linux). For Mac dev, optional Santa exemption per onboarding: https://www.internalfb.com/wiki/AppVote/Requesting_Opt-out/

```text
# Clone (HTTPS)
git clone https://github.com/soapss/gamedev-sophiez-chess-fighter.git
cd gamedev-sophiez-chess-fighter

# Open in Godot
Open project.godot in Godot 4.7 editor: Project Manager -> Import -> select project.godot

# Run
Press F5 or Run Main scene: res://src/scenes/Main.tscn

# CLI headless validation
godot --path . src/scenes/Main.tscn
```

Mac build required per track, but Godot 4.7 cross-platform export via export presets (not committed per Ivan "no binaries").

## Core Features (Foundation Phase)

- smooth, responsive chess click-to-move with legal move highlights
- full pseudo-legal + legal filtering: pawn 1/2, captures, en passant target tracking, knight L, bishop/rook/queen sliding with path clear, king 1 + castling kingside/queenside with intermediate check validation
- state: check, checkmate +- material, stalemate detection
- original primitives only, no external deps — easy clean build

Future one-shot:
- bot-first minimax depth 3 alpha-beta, capture-first sorting, material+positional eval
- fighter integration with per-piece specials and cooldowns, 45s round timeout, FIGHTER_PENDING state

## Gold Version

- See each task's `task.toml` for exact `avocado-model` (use latest) and `harness`.
- Current foundation: built manually + Avocado Muse Spark 1.1 — no 3P tokens in src/* per rule. Engine updated to 4.7 per local requirement.

## Tasks

| Task | Description | Completed |
| --- | --- | --- |
| capture-fight (WIP) | Full one-shot: chess bot + fighter duels on capture, per-piece specials | Foundation done — board only, 2026-07-22 |

## References
- Template: [gamedev-template](https://github.com/codimango/gamedev-template)
- Track doc: ADO GameDev Track (Godot 4.4+ -> 4.7)
- Asset Guidelines: https://fb.workplace.com/groups/1939643316691589/permalink/1966187604037160/
- How I Work Through a Task: https://fb.workplace.com/groups/1939643316691589/permalink/1967780980544489/
