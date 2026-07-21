# Task: example-task

<!-- Rename this folder and heading to a descriptive task name (no date prefix).
     This README describes ONE task — a larger, complex chunk of work toward the
     game. The whole-game overview lives in the repo-root README.md. -->

## Description

<!-- What does this task deliver? For a one-shot game build, summarize the game;
     for an incremental task, describe the golden feature added in this task and
     how it builds on the existing gold source. -->

Full one-shot build of **TidEscape** — a 2D crab-on-a-beach maze survival game
where you feed at low tide, grow into a bigger shell, and climb back to higher
ground before the rising tide reaches you. See [instruction.md](./instruction.md)
for the full spec used to build it.

## Avocado vs Claude

<!-- Strongly encouraged. Once the golden feature is built, one-shot the
     instruction.md with BOTH Avocado (1P) and Claude (3P) separately and record
     what worked and what didn't. Do NOT commit any code from these one-shots —
     this section is exploration for researchers to see where Avocado
     underperforms. Replace the illustrative rows below with your own findings. -->

> **Note:** The rows below are illustrative — replace them with observations
> specific to this task's one-shots.

| Evaluation | Claude (Opus 4.8) | Avocado | Track opportunity |
| --- | --- | --- | --- |
| Self-validation (closed loop) | Runs build, screenshots, analyzes, self-corrects before finalizing | No validation; ships broken code (wrong APIs, missing files) | Validate-then-fix trajectories; reward verifying before "done" |
| Visual / image understanding | Parses rendered output and assets (e.g. 9-slice UI) | Cannot analyze images at all | Multimodal read-and-verify tasks against the instruction/reference |
| Speed | Slower | 5-7x faster (minutes on raylib) | Preserve the speed while closing correctness gaps |

## Human-Tuned Areas

<!-- The nuanced, taste-making pieces you hand-tuned that a model would not nail by
     default. Call these out so reviewers can judge the human craft. Examples:
     input timings / jump buffer, animation speeds, color palette, difficulty ramp,
     enemy pacing, audio/music timing and mix, camera feel. Remove any that do not
     apply and add your own. -->

- <!-- e.g. tide rise-rate curve tuned so escape feels tense but fair -->
- <!-- e.g. crab movement acceleration + input buffer tuned for responsiveness -->
- <!-- e.g. palette and shell-growth animation timing hand-adjusted for readability -->

## Screenshots

<!-- Capture key states (gameplay, UI, win/loss). Referenced from task.toml. -->

![screen-01](./screenshots/screen-01.png)

## Videos

Do not commit video files. Upload gameplay and teaser videos to **PixelCloud**
and reference the links here and in [task.toml](./task.toml) (`videos`, `teaser`
— teaser is a ~10 sec highlight).

- Gameplay: <!-- PixelCloud link -->
- Teaser: <!-- PixelCloud link -->

## Trajectories

<!-- Add the paste link to the trajectory used when building this task's golden
     feature (Avocado / manual — never 3P). Skip if you built it entirely by hand.
     Inline it here if it's short. -->

- Gold build trajectory: <!-- bunnylol paste link -->

## Tag reference

`task.toml` uses controlled vocabularies. Pick from these lists:

**game-tags:** Arcade/action · Puzzle/board/card · Simulation/management ·
RPG/adventure/story · Sports/racing/vehicle · Casual/avatar/decor ·
Educational/serious · 3D/VR scene-like · Interactive scene/cinematic ·
Other game/unclear

**tech-stack-tags:** Web JS/DOM · Vanilla JS canvas · React/TS canvas ·
Three.js/WebGL · Phaser 3 · Pixi.js · Godot · Unity/Roblox/native game engine ·
Unspecified/other

**assets-used:** Primitives · ImportedAssets
