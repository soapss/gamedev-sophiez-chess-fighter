# Product Requirements Document

<!-- This document is the detailed game task description. It serves directly as
     the prompt for Avocado to build the game. Write it as a complete spec, not
     a fill-in-the-blank template. Keep scope small and playable. -->

## Overview

<!-- What is the game? What is the core loop the agent must implement? -->

2D game and the challenge is navigating a maze up and down the shore. The maze may be narrow in some areas forcing you to take different paths as you grow. When you get larger your will be able to open door ways (move a rock) to access higher ground.

The mazes are procedurally generated based on a seed value. So you can get a different challenge each time but also revisit a particularly challenging level.

Visuals
Use a similar aesthetic style to the arcade game Dig-Dug with respect to color pallette, pixel art for characters, textures, and backgrounds. Everything should match this aesthetic including the UI, title screen and logo.
The sky should indicate day night cycle with the sun and move passing from right to left.

Perhaps have 5 tiers for the initial level. So you must grow 5 times and return to higher ground 5 times.

The terrain should appear to curve near the horizon by squashing vertically similar to a retractable blind or roll of paper. This makes the sky always visible even when the maze you are navigating doesn't fit on the screen.

You are a crab on a rocky beach. You go down to the water at low tide to feed and must escape back to higher ground as the tide rises again. You get a little bigger each time you feed. You must find a new shell for the larger you. The tide get higher each time.

Stages
- You navigate a simple maze down to the water line at low tide
- You feed in between waves eatting little shrimps from the wet sand.
- Once you feed enough, a new shell washes up. It is left behind by one of the waves. It must be above the water line.
- You must swap into the new larger shell.
- The tide starts to rise again
- You must navigate back up the maze to higher ground.
- Once safe from the tide you can rest. The night cycle should pass more quickly and start the next day.
- The tide goes back out and you start again.

Tides
- The tides will get progressively higher at high tide requiring you to climb farther each time.
- You will have the same amount of time for each tide cycle even though you will have to climb higher each day.
- Technically there are 2 high and 2 low tides per day, but we will stick to 1 a day for simplicity. This allows for a day night cycle to coincide with the play time with high tide at night when the moon is out and low tide at daytime.
- Will need to tune the tide duration to make it challenging but achieveable.
- There are waves in addition to tides. At low tide the waves come in and out 5 times before the tide rises again.
- If you do not feed and grow, you will be unable to reach the next higher section of the maze and will be consummed by the tide.

## Assets
All graphics are sprite pixel art. These should be solid and detailed. Don't just use lines for legs or a circle for the body. Give them actual shape. Use DigDug characters as a reference.

The main items that need 2D assets are:
- Crab that can grow in size. Plays a generic walking animation when moving in any direction. He should have 4 legs and 2 arms with claws. He should have eyes that extend above his head.
- Shells that the crab can wear in 5 sizes. Should attach to the crab visibly once collected. Should be spiral shaped and solid except for the opening the crab extends from.
- Shrimps to eat. Simple animation to show them emerging from the sand.
- Sun. Stylized cartoon sun.
- Moon. Full moon is fine. Add some texture, not just solid.
- Rocks that block paths in the maze. Can vary in size.
- Wave edge pattern that can repeat across the screen with sea foam.
- Logo for title screen. This should be large arcade style font with the bottom half of the letters filled with water with foam separating from the top. Bold outlines around the letters. Keep the letter spacing tight and use all capital lettering. The logo should fill 80% the width of the screen. Make the lettering cartoonish and chunky.

## Maze Design
- The maze is a series of labyrinths generated from a seed value.
- There are 5 sections to each maze.
- Between each section there is an open space that is safe from the tide for that day and contains the entranceto the next section.
- The mazes are traversed up and down. The entrance and exit are always on top and bottom of each section.
- There should only be a single path for the largest crab through each section, but include shortcuts for smaller crabs in the lower sections. A crab may take a shorter path on the way down than on the way back up due to no longer fitting through the downward path.
- The entire 5 sections of maze should be generated at the start of gameplay.
- Only the bottom section of the maze is accessible at the start. You must unlock each section by moving a rock. You must grow one size to open each rock. So, you will start each stage of the game below a section blocked by a rock that you cannot move yet.
- Mazes should have continuous paths with corners and intersections and dead ends. There should be only one correct path for the largest crab to take.
- Maze sections should get progressively more difficult the higher you get.
- Mazes should appear to be carved from a solid piece of earth. The are tunnels through the solid material, not walls on a flat plane.
- Required paths may require traversing horizontally or going down and then back up. These complexities should increase in the higher sections.


## Presentation
- The game should start at a title screen which shows the logo over an autoplaying level.
- Below the logo is a start button which initiates a new game.
- If the game has been played before, below the start button there should be a level select button. This takes you to a screen with a list of the maze seed values you have played before and whether each of them has been completed or not.

## Goals

- Deliver a playable single-player game with a clear win, loss, or completion state.
- Document controls and expected behavior.
- Keep the implementation readable for future agent edits.

## Non-Goals

<!-- Features intentionally excluded from scope. -->
- Sound effects and music. May add later.

## Requirements

### Display

<!-- Window size, camera, layout, rendering style, assets. -->

- Render in 2D
- Window size 1080x720
- Window title "TidEscape"

### Controls

<!-- Keyboard, mouse, controller, touch controls. -->

- WASD movement only
- Menus should be navigated with WASD and spacebar or enter
- You should not need to use the mouse at all

### Gameplay

<!-- Entities, rules, collisions, scoring, game states. -->

### Win / Loss / Completion

<!-- Conditions that end or reset the game. -->

- Win condition - You survive 5 high tides.
- Lose condition - The tide touches you. If you are touched by the tide a game over screen appears with options to retry, new game, or level select.
- Upon losing the crab image should be turned upside down.

## Technical Constraints

<!-- Engine, language, dependencies, supported platforms. -->

Engine: Raylib
Language: C
Platform: Mac
