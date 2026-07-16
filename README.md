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
- **No binaries in the repo.** Per the latest track guidance, only the oracle
  (gold) solution is submitted; there is no reliable way to run peers' binaries,
  so builds are not committed. Share behavior via screenshots and videos instead.
- **Videos** are not stored in the repo; upload them to **PixelCloud** and
  reference the links from each task's `task.toml` and README.

## Core Features

<!-- Describe the standout features of the game as a whole. -->

- Example: smooth, responsive controls
- Example: fast level transitions with no loading hitches

## Gold Version

- Built with <!-- e.g. muse-spark-1.1-aai2 (Avocado) --> — see each task's
  `task.toml` for the exact `avocado-model` and `harness` used.

## Tasks

<!-- Index the tasks in this repo. -->

| Task | Description | Completed |
| --- | --- | --- |
| [example-task](./tasks/example-task/) | Full TidEscape one-shot build | <!-- YYYY-MM-DD --> |
