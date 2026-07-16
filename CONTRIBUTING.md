# Contributing — GameDev Track

This template follows the ADO GameDev track process. GameDev is about building
**real, polished, engaging games with Avocado**, and deriving training tasks from
that work. Tasks are chunks of that real game-building effort.

## Workflow

1. **Pick a game.** Choose something that pushes Avocado beyond its comfort zone —
   complex enough to be interesting. Dedupe across people to keep genre variety.
2. **Break it into todo items** (tasks). Each is a larger, complex chunk of work —
   a real piece of game-building. If Avocado breezes through a chunk and it's
   trivial, just commit it and move on; it isn't task-worthy.
3. **Build the gold solution.** Implement the task manually or with Avocado (1P).
   **No 3P models** for the gold source. Export the trajectory and link it in the
   task README.
4. **Write `instruction.md`.** Capture the instructions needed for a model to
   reproduce what you built. Give enough detail without spelling out the solution.
   Keep tuning it until a model prompted with it returns your gold solution.
5. **Avocado vs Claude (exploration).** One-shot the instruction with both models
   separately. Note what worked / didn't in the task README. **Do not commit any
   code from this step.**
6. **Wrap up the task.** Record the start and end commit hashes, grab exciting
   screenshots/videos, fill in `task.toml`, then pick the next todo item.

## Model policy (1P / 3P)

- **Gold source, assets, and `instruction.md`:** 1P only. No 3P models. Do not
  rely heavily on 1P either for `instruction.md` — use 1P for grammar/technical
  help only.
- **`README.md` and `task.toml`:** 3P models are allowed (same as other tracks).
- **Brainstorming / task ideas:** neither 3P nor 1P — the ideas should be yours.
- **No 3P tokens in the repo.** Do not commit Claude (or other 3P) one-shot code.

## Constraints

- **Engine/library:** open-source / non-commercial-license only. Unity and Unreal
  are unsupported for now.
- **Platform:** build for **Mac** (temporary — so TBD researchers can run it).
- **Model:** always use the latest Avocado model.
- **Binaries:** do **not** commit binaries. Only the oracle (gold) solution goes
  in the repo; share behavior via screenshots and PixelCloud videos.
- **Repo layout:** one repo per game, named `gamedev-{game-name}` (unixname
  agnostic — multiple people may work on one game).

## Notes

- Codimango integration for this structure is a work in progress; some fields
  (e.g. `starting-commit-hash`) and review flows may change.
- Grading rubrics (behavior rubrics, reference media, golden tests) are being
  defined and may become a required per-task artifact later.
