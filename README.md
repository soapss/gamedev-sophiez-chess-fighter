# Game Title

<!-- One-paragraph description of the game and its core loop. -->

## Project Structure

```text
gamedev-template/
├── src/                  Game project source — where the game itself lives.
│                         Full engine project (Godot/raylib/etc): code,
│                         scenes, assets, and build config. This is what the
│                         agent edits and what compiles into the binaries.
├── screenshots/          Screenshots of the running game.
│   └── screen-01.png     Referenced from task.toml; capture key states
│                         (gameplay, UI, win/loss) for quick review.
├── binaries/             Compressed binaries of the compiled game for
│   ├── game.app.zip      reviewers to run. Provide an OSX build — most
│   └── game.exe.zip      researchers run Macs. Add Windows if available.
├── instruction.md        Product Requirements Document — the detailed game
│                         spec that also serves as the prompt for the agent.
├── task.toml             Task metadata: tags, screenshot paths, video px
│                         links, and binary paths. See task.toml.example.
├── task.toml.example     Template for task.toml with example values.
└── README.md             This file — overview, observations, links.
```

Notes:

- **`src/`** holds the complete, buildable game project, not just loose scripts.
- **`binaries/`** must contain archived (zip) builds — at minimum an OSX build.
- **Videos** are not stored in the repo; upload them to bunnylol px and link
  them from `task.toml` (see the Videos section below).

## Core Features

<!-- Describe the standout features of this implementation. -->

- Example: smooth, responsive shooting experience
- Example: fast level transitions with no loading hitches

## Observations

<!-- Optional. Notes comparing implementations (e.g. Avocado vs Claude),
     design trade-offs, or anything notable about how the game was built.
     Example table below — keep, edit, or remove as needed. -->

> **Note:** The observations below are provided for illustration only — replace
> them with notes specific to your own implementation.

| Evaluation | Claude (Opus 4.8) | Avocado | Track opportunity |
| --- | --- | --- | --- |
| Self-validation (closed loop) | Runs build, screenshots, analyzes, self-corrects before finalizing | No validation; ships broken code (wrong APIs, missing files) | Validate-then-fix trajectories; reward verifying before "done" |
| Visual / image understanding | Parses rendered output and assets (e.g. 9-slice UI) | Cannot analyze images at all | Multimodal read-and-verify tasks against PRD/reference |
| Speed | Slower | 5-7x faster (minutes on raylib) | Preserve the speed while closing correctness gaps |

## Videos

Do not commit video files to the repository. Upload gameplay and teaser
videos to **bunnylol px** and reference the resulting `px` links in
`task.toml.example` (the `videos` and `teaser` fields).

Example: https://www.internalfb.com/intern/px/p/bd8gT

## Trajectories

<!-- Optional. Agent run trajectories that produced this game. -->

Upload trajectories to **bunnylol paste** and link them here. If a
trajectory is short, inline it directly in this section instead.
