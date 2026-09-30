# Agent Handoff — shore.landing_stage — Visual Critic and Gameplay Critic

Filled from `AGENT_HANDOFF_TEMPLATE.md`. **For an agent that did not build the stage**, for example Gemini as Visual Critic and Cursor/Grok as Gameplay Critic. Run the two independently: neither reads the other's findings, and neither needs the builder's notes, before writing its own.

Read first: `CLAUDE.md`, `Design/POIs/README.md`, `Design/POIs/shore.landing_stage.md`, `Design/POIs/reviews/shore.landing_stage_REVIEW_PACKET.md`.

## Assigned Role

One per agent:

- Visual Critic
- Gameplay Critic

## Allowed Scope

- Read the spec, the review packet, the captures, the scripts, the tests, and the test logs.
- Visual Critic: judge each landing review view against its written expectation, and the four states against each other from the same camera.
- Gameplay Critic: judge the routes, the state rules, persistence, blocking, and the inspect text against the spec, by reading the scripts and tests and, if the agent can run them, `Tools\RunTests.bat Map.Boathouse.LandingStage`.
- Write findings. Nothing else.

## Files / Content Owned

- Visual Critic: `Design/POIs/reviews/shore.landing_stage_visual.md` (new)
- Gameplay Critic: `Design/POIs/reviews/shore.landing_stage_gameplay.md` (new)

## Files That Must Not Be Modified

Everything else, in particular every script, map, asset, test, plan, and the spec. A critic that edits content has stopped being a critic.

## Input Specification

- **POI spec:** `Design/POIs/shore.landing_stage.md` at the commit named in the review packet
- **Upstream artifacts:** the capture run folder named in the review packet (`Saved/Review/<run>/`: PNGs, `contact_sheet.png`, `manifest.json`); the test logs `Saved/Logs/RunTests*.log`
- **Baseline:** the commit and test count in the review packet
- **Approved decisions from Anthony:** `PRODUCTION_PILOT.md` §12; `WorldStatePhasePlan.txt` §2 (the state table)

## Expected Deliverables

One findings file with, for each finding:

- an id (`V-01` or `G-01` onward), a severity (`blocker`, `should fix`, or `nit`), the review view or the file and line, what is wrong, why it matters against the spec or the plan, and a suggested fix in one sentence
- a short "passes" list of what was checked and found right
- answers to the review packet's questions

Done when every question in the packet has an answer and every finding has a severity.

## Required Tests

None to add. The Gameplay Critic may run `Tools\RunTests.bat Map.Boathouse.LandingStage` (editor closed) and report the result.

## Required Review Artifacts

The findings file only.

## Known Dependencies

The capture run and the test logs named in the review packet must exist.

## Stop Conditions

- The captures or logs named in the packet are missing: stop and say so in the findings file.
- A finding needs a creative decision (a lore claim, the stage's place on the shore): write it as a question for Anthony, not as a fix.

## Handoff Notes

- **Status:** not started
- **Return to:** the builder (revision, WS-09), then Anthony
