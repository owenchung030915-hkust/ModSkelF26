# Modeler — Hierarchical Cat

Computer Graphics (Fall 2026) Project 2. A hierarchical 3D cat built with OpenGL and FLTK.

**Repo:** https://github.com/owenchung030915-hkust/ModSkelF26

For the 30-pt oral / video questions, see [`DEMO_QA.md`](DEMO_QA.md).

## Requirements

- Windows
- Visual Studio 2022 with **Desktop development with C++**
- Platform **Win32** (x86), not x64

FLTK is already in this repo (`fltk-1.3.3`). You do not need to install it separately.

## Run from Visual Studio

1. Open `modeler.sln`.
2. In the toolbar, set **Debug** and **Win32**.
3. Build: **Build → Build Solution** (or `Ctrl+Shift+B`).
4. Run: **Debug → Start Without Debugging** (`Ctrl+F5`), or **Start Debugging** (`F5`).

The executable is written to `Debug\modeler.exe`.

If Visual Studio asks to retarget the toolset, accept the default for VS 2022 (`v143` is fine).

## Run from a terminal

```powershell
cd path\to\ModSkelF26

# Build (optional if already built in Visual Studio)
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" modeler.sln /p:Configuration=Debug /p:Platform=Win32

# Run
.\Debug\modeler.exe
```

If `MSBuild.exe` is not at that path, open a **Developer PowerShell for VS 2022** and run:

```powershell
msbuild modeler.sln /p:Configuration=Debug /p:Platform=Win32
.\Debug\modeler.exe
```

## Camera and UI

| Input | Action |
| --- | --- |
| Left-drag | Orbit |
| Shift + left-drag | Twist camera |
| Middle-drag | Pan |
| Right-drag | Zoom |
| Control sliders | Pose, appearance, scene props |
| **Control → Animate** | Play animation (~30 fps) |

## Demo path (for showing the model)

1. Start the program with default sliders.
2. Scrub **Head Yaw** (single joint).
3. Scrub **Sit (multi)** then **Stretch (multi)** (coupled joints).
4. Set **Anim** to `1` (walk), turn on **Control → Animate**.
5. Try moods `0–3` (calm / playful / sleepy / alert) with **Yarn Ball Toy** on.
6. Flip **Frame All (trigger)** from `0` to `1` to reframe the camera.

## Troubleshooting

- **Nothing runs / “not a valid Win32 application”** — the build must be **Win32**, not x64.
- **Toolset v145 not found** — retarget the project to **v143** (VS 2022).
- **Blank window / missing libs** — rebuild Debug Win32 so `Debug\modeler.exe` is regenerated.
- **Animate does nothing** — set **Anim Speed** above 0 and enable **Control → Animate**.
