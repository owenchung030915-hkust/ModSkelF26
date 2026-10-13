# Project 2 Modeler — Simple Implementation Q&A (30 pts)

Solo: ~45 seconds. Two students: split Q1 / Q2 (~90 seconds).

---

## Q1. How did you implement `Camera::lookAt`?

**Show:** `camera.cpp` — `Camera::lookAt` and `applyViewingTransform`.

**Say:**

We were not allowed to call `gluLookAt`. We built the same view transform ourselves.

`applyViewingTransform` passes the camera `eye`, look-at point, and up vector into `lookAt`.

```cpp
void Camera::lookAt(double *eye, double *at, double *up)
{
	// Build an orthonormal basis matching gluLookAt, without calling it.
	Vec3f eyeV(...), atV(...), upV(...);

	Vec3f forward = atV - eyeV;
	forward /= fLen;                 // 1) look direction

	Vec3f side = forward ^ upV;      // 2) camera right (cross product)
	side /= sLen;

	Vec3f upN = side ^ forward;      // 3) true up (orthonormal)

	// 4) load rotation matrix, then 5) translate by -eye
	glMultMatrixf(m);
	glTranslated(-eye[0], -eye[1], -eye[2]);
}
```

**Explain in order:**

1. **Forward** = normalize(`at − eye`) — camera look direction.
2. **Side** = `forward × up` — camera right.
3. **True up** = `side × forward` so the three axes are orthonormal.
4. Put those axes into a 4×4 matrix (`glMultMatrixf`) so world axes line up with camera axes.
5. `glTranslated(-eye)` moves the world so the camera sits at the origin.

That is the same as `gluLookAt`: rotate, then translate the world into camera space. Then briefly show `applyViewingTransform` calling `lookAt` instead of `gluLookAt`.

---

## Q2. Bonus: extra animated sequences (walk / pounce / groom)

**This is the [bell] “additional animated sequence” item.**

### Demo setup (do this first on screen)

1. Set **Anim Speed** ≈ `1`.
2. Set **Anim 0idle/1walk/2pounce/3groom** to **`1`** (walk).
3. Turn on **Control → Animate**.
4. Optional: switch to **`2`** (pounce) then **`3`** (groom) for 2–3 seconds each.

You should see alternating legs (walk), crouch→spring (pounce), or head tuck + raised paw (groom).

### Show code: `sample.cpp`

Each frame we advance a phase, then a `switch` on `ANIM_MODE` writes offsets into many joints:

```cpp
m_animPhase += 0.08 * VAL(ANIM_SPEED);

float walk    = sin(m_animPhase * 2.2 * moodAnim);
float walkOpp = sin(m_animPhase * 2.2 * moodAnim + M_PI);

switch (animMode)
{
case 1: // Walk — opposite-phase legs
	modeLegFL = walk * 28.0f;
	modeLegFR = walkOpp * 28.0f;
	modeLegHL = walkOpp * 22.0f;
	modeLegHR = walk * 22.0f;
	modeKneeF = 8.0f + fabs(walk) * 12.0f;
	modeKneeH = 10.0f + fabs(walkOpp) * 14.0f;
	modeBodyBob = fabs(walk) * 0.06f;
	break;
case 2: // Pounce — crouch then spring
	// t from anim drives spine, hips, knees together
	break;
case 3: // Groom — head down + front paw up
	modeHeadPitch = -35.0f + anim * 8.0f;
	modeLegFL = 55.0f + anim * 15.0f;
	break;
}
```

Those `modeLeg*` / `modeKnee*` values are added into the normal joint angles before `drawLeg`, so one anim mode drives the whole pose over time when Animate is on (~30 redraws/sec).

**Say (brief):**

> For our bonus we added animated sequences. With Animate on, `draw()` increments a time phase. Anim mode 1 is a walk cycle: opposite sine waves on front and hind legs so the cat walks. Mode 2 is pounce, mode 3 is groom. So instead of one slider, the animation automatically drives many joints each frame.

---

## 45-second solo script (read aloud)

> **Q1:** We replaced `gluLookAt` with our own `Camera::lookAt`. From eye, at, and up we build an orthonormal basis: forward is at minus eye, side is forward cross up, up is side cross forward. We load that as a rotation matrix, then translate by minus eye so the camera is at the origin.
>
> **Q2:** Our bonus is an extra animation sequence. With Control→Animate on and Anim mode set to walk, each draw call advances a phase angle. We use sine for one diagonal pair of legs and the opposite phase for the other pair, plus knee bend, so the cat walks. We also have pounce and groom modes in the same switch.

**On camera:**

| Time | What to show |
| --- | --- |
| 0–18s | `camera.cpp` `lookAt` (Q1) |
| 18–30s | Live demo: Animate ON, Anim = 1 (walk), maybe flip to 2 |
| 30–45s | `sample.cpp` walk `switch` case, point at `walk` / `walkOpp` |

---

## Backup Q2 (if walk is awkward): L-system plant

Scrub **Plant L-System Depth** `1 → 4`. Show recursive `drawLSystemPlant` in `sample.cpp` (cylinder + 3 recursive branches with `glPushMatrix` / `glPopMatrix`). Say: “procedural modeling bonus — depth slider controls recursion.”
