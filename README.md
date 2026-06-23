# Game Title

<!-- One-paragraph description of the game and its core loop. -->

## Core Features

<!-- Describe the standout features of this implementation. -->

- Example: smooth, responsive shooting experience
- Example: fast level transitions with no loading hitches

## Observations

<!-- Optional. Notes comparing implementations (e.g. Avocado vs Claude),
     design trade-offs, or anything notable about how the game was built.
     Example table below — keep, edit, or remove as needed. -->

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

<!-- Optional. Links to agent run trajectories that produced this game. -->
