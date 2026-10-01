# The exercises

Before you start, setup must have printed **READY** and the agent must have
opened: a bordered box with a cursor at the bottom of the screen. If you see a
line ending in `$` instead, type `openclaw chat`.

**The Intent Gap.** Agents act on our behalf, but they do not share the intent
we carry in our heads. There is always a gap between what we meant and what the
agent actually did. The engineering work is to make that gap visible, testable
and recoverable: watch it, check it, give it a rule.

## Try it yourself

| File | What it is | Time |
|---|---|---|
| [`flight-software.md`](flight-software.md) | **Start here, Dan.** The agent on `flightlib/`, a C99 avionics-style library: goal, check, rule, then a menu of experiments (an independent Fortran oracle, a CBMC proof, MISRA triage, determinism, Ada, JSBSim). | 15 min, then as long as you like |
| [`hands-on.md`](hands-on.md) | The original workshop exercise. One goal, five moves, three short lines, on a few documents of your own in `mine/`. | 15 min |
| [`site-tutor.md`](site-tutor.md) | Have the agent build a small public teaching website from public sources, then check its facts and give it a rule. Optionally make it permanent with `bash publish-site.sh`, an approval gate you answer yourself. Includes a classroom version. | 20-30 min |

## From the original workshop, for educators

| File | What it is |
|---|---|
| [`adapt-one-line.md`](adapt-one-line.md) | Write one change to one course in four lines. |
| [`starter-assignment.md`](starter-assignment.md) | A one-week assignment: delegate a task to an AI assistant, verify it, document it. |
| [`starter-build-agent.md`](starter-build-agent.md) | A first building assignment: students write a small agent with a trace, a check, an approval gate and a rollback story. |
| [`rubric-template.md`](rubric-template.md) | The grading grid for both assignments. Five criteria, four levels, plus a build variant. |
| [`what-went-wrong.md`](what-went-wrong.md) | The candid list from a four-week agentic systems course: what broke, and what we would do again. |

## Three habits that save time

- Click once inside the terminal before you type. Typing in the editor changes a
  file and reaches no agent.
- Read the **Exec** cards above each reply: they show what the agent opened and
  ran. **Check its evidence, not its summary.**
- If the agent asks you a question and you do not know the answer, tell it to
  choose and to say what it assumed.
