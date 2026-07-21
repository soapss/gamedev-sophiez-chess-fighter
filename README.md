# Game Title

<!-- One-paragraph description of the game as a whole and its core loop. This is
     the game-level README — it describes the complete game that lives in src/.
     Each task under tasks/ has its own README describing that task specifically. -->

This repo follows the ADO **GameDev track** task structure: one repo per game
(`gamedev-{game-name}`), a single shared gold source in `src/`, and one folder
per task under `tasks/`.

## Project Structure

```text
gamedev-{game-name}/
├── src/                  Gold game source — the single, shared, buildable game
│                         project (code, scenes, assets, build config). One
│                         source of truth for the whole game; tasks do NOT copy
│                         it. Built with 1P (Avocado) or by hand — never 3P.
├── tasks/                One folder per task (a "todo item" — a larger, complex
│   └── <task-name>/      chunk of work toward the game). Name it descriptively;
│       │                 do not prefix with a date (record the completion date
│       │                 in task.toml / the task README instead).
│       ├── instruction.md    Detailed game task description — the prompt used to
│       │                     reproduce this task's golden feature. No 3P models.
│       ├── task.toml         Task metadata (see the example task's task.toml).
│       ├── screenshots/      Screenshots of the running game for this task.
│       │   └── screen-01.png Referenced from task.toml; capture key states.
│       └── README.md         Task description, Avocado vs Claude comparison,
│                             and trajectory links.
└── README.md             This file — the game-level overview.
```

Notes:

- **`src/`** holds the complete, buildable gold game — not loose scripts, and not
  copied into each task. Tasks reference it and build on top of it.
- **Videos** are not stored in the repo; upload them to **PixelCloud** and
  reference the links from each task's `task.toml` and README.

## Game types

The template is generic. Use it for either kind of GameDev task:

- **One-shot game** — a small, complex game buildable from a single PRD. Add it
  as a single task under `tasks/`; no need to break it into multiple steps.
- **Large game** — an ongoing game built as many todo-items. Each task under
  `tasks/` is a larger, complex chunk of work. Not every todo-item is
  task-worthy: if a model flies through it, just commit the code and move on; if
  it's purely non-visual source code, submit it as a t-bench / swe-bench task
  instead.

## Constraints

- **Engines** — only libraries/engines without commercial licences (Unity and
  Unreal are unsupported).
- **Model** — always use the latest model.
- **1P / 3P models** — the gold solution and any in-game assets must not come
  from 3P models. `README.md` and `task.toml` may use 3P; `instruction.md` must
  not use 3P and must not lean heavily on 1P (grammar / technical help only).
  Never use any model to brainstorm the game or task idea.

## Engine & Framework

<!-- The engine/framework the game is built on, its version, and its license.
     The license must permit commercial/internal use (no commercial-use restriction). -->

- **Engine / framework:** <!-- e.g. raylib 5.0 (C) -->
- **License:** <!-- e.g. Zlib -->

## Dependencies

<!-- Every third-party library/package used in src/, pinned to an exact version,
     with its source and license. All must permit commercial/internal use.
     Write "None." if nothing is used beyond the engine. Delete the example row. -->

| Library | Version | Source | License |
| --- | --- | --- | --- |
| <!-- flame --> | <!-- 1.18.0 --> | <!-- https://pub.dev/packages/flame --> | <!-- MIT --> |

## Assets & Attribution

<!-- Every asset shipped in src/ (art, sprites, audio/music, fonts, models). Give
     source + license + attribution for ALL third-party AND Meta-internal assets.
     Group by folder when a whole directory shares one source/license (e.g. all
     sprites in src/assets/char_sprites/ are X license from Y source); list
     individual files only where they differ. If an asset is original or generated,
     say so and by what. If the game uses only primitives, write: "All assets are
     original primitives. No third-party or Meta-internal assets." Delete the
     example rows. -->

| Asset / Folder | Type | Source | License / Attribution |
| --- | --- | --- | --- |
| <!-- src/assets/char_sprites/ --> | <!-- sprites --> | <!-- kenney.nl --> | <!-- CC0 --> |
| <!-- src/assets/music/theme.ogg --> | <!-- music --> | <!-- original (generated) --> | <!-- internal-use --> |

No third-party tokens, proprietary code, or IP appear in the code, assets, or the
model-visible environment; all third-party material is attributed above.

## Building & Running

<!-- Prerequisites (compiler/SDK/browser + versions) and exact steps to build src/
     from source and run it, per platform. Must be sufficient for a clean build in
     a fresh environment (dependency versions are pinned above). -->

**Prerequisites:** <!-- e.g. clang + CMake 3.28; Emscripten for web; a modern browser -->

```text
<!-- e.g.
cd src
<build command>
<run command>
-->
```

## Core Features

<!-- Describe the standout features of the game as a whole. -->

- Example: smooth, responsive controls
- Example: fast level transitions with no loading hitches

## Gold Version

<!-- Model + harness are per-task and live in each task's task.toml (avocado-model,
     harness), the single source of truth. Do not restate the model here. -->

- See each task's `task.toml` for the exact `avocado-model` (use the exact, latest
  model) and `harness` used.

## Tasks

<!-- Index the tasks in this repo. -->

| Task | Description | Completed |
| --- | --- | --- |
| [example-task](./tasks/example-task/) | Full TidEscape one-shot build | <!-- YYYY-MM-DD --> |
