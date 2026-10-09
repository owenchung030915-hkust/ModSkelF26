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

## Q2. Required extension: coupled **Sit** slider

**Demo first:** drag **Sit (multi)** from 0 to 1. Hind hips fold, knees bend, body drops.

**Show:** `sample.cpp` — the `VAL(SIT)` mapping.

**Say:**

One slider drives several joints. We read `VAL(SIT)` once, then map it to hip fold, knee bend, and spine:

```cpp
float sit = (float)VAL(SIT);

float spineArch = stretch * 25.0f - sit * 10.0f + ...;
float hindFold  = sit * 55.0f + stretch * (-15.0f);
float hindKnee  = sit * 70.0f;

float legHL = (float)VAL(LEG_HL) + hindFold + ...;
float legHR = (float)VAL(LEG_HR) + hindFold + ...;
float kneeH = 18.0f + hindKnee + ...;
```

`drawLeg` applies hip then knee with `glRotated` / `glPushMatrix`, so one Sit value moves hip **and** knee together — same idea as the spec’s “straighten a leg.”

---

## 45-second solo script (read aloud)

> **Q1:** We replaced `gluLookAt` with our own `Camera::lookAt`. From eye, at, and up we build an orthonormal basis: forward is at minus eye, side is forward cross up, up is side cross forward. We load that as a rotation matrix, then translate by minus eye so the camera is at the origin.
>
> **Q2:** Our coupled control is Sit. One slider value is added into hind hip angles, hind knees, and a small spine change, so the cat sits instead of moving one joint.

**On camera:** ~20s on `lookAt` in `camera.cpp`, ~25s scrubbing Sit while pointing at the `sit * 55` / `sit * 70` lines.
