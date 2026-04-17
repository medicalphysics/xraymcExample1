# xraymcExample1 — Pencil Beam Depth-Dose Simulation

This example demonstrates the basic workflow of [xraymclib](https://github.com/medicalphysics/XRayMClib) by simulating a monoenergetic pencil beam directed onto an aluminum cylinder and scoring the resulting depth-dose profile.

## What the example does

1. Defines a cylindrical geometry item configured as a depth-dose scorer.
2. Assigns aluminum as the cylinder material (looked up by atomic number).
3. Configures a 70 keV pencil beam aimed along the cylinder axis.
4. Runs the Monte Carlo photon transport and prints the depth-dose table to the console.
5. Renders two PNG images of the scene — plain geometry and geometry colored by deposited dose.

## Simulation geometry

```
z = +10  ●  beam source, pencil beam directed in -z
          |
          ↓
     ┌────────┐   z = 0 (top face of cylinder)
     │        │
     │  Al    │   radius = 2 cm, length = 10 cm, 100 depth bins
     │        │
     └────────┘   z = -10
```

The surrounding world is filled with air at standard density.

## Building

The library is fetched automatically at CMake configure time via `FetchContent`. No manual installation is required.

**Requirements**
- CMake ≥ 3.20
- C++23-capable compiler (MSVC ≥ 16.8 or Clang ≥ 13.0)
- Internet access at configure time (to clone xraymclib and download the EPICS physics database)

**Configure and build**

```bash
cmake -B build
cmake --build build
```

The `CMakeLists.txt` already sets `XRAYMCLIB_EPICS_DOWNLOAD ON`, so CMake will download the required EPICS 2025 physics data (EPDL/EADL) from the IAEA automatically during configuration.

### CMakeLists.txt overview

```cmake
include(FetchContent)
FetchContent_Declare(
    libxraymc
    GIT_REPOSITORY https://github.com/medicalphysics/XRayMClib.git
    GIT_TAG master
)
set(XRAYMCLIB_EPICS_DOWNLOAD "ON")  # auto-download physics database
set(XRAYMCLIB_USE_LOADPNG "ON")     # enable PNG output

FetchContent_MakeAvailable(libxraymc)

add_executable(xraymcexample1 main.cpp)
target_link_libraries(xraymcexample1 PRIVATE libxraymc)
xraymclib_add_physics_list(xraymcexample1)  # copies physics data next to the binary
```

The call to `xraymclib_add_physics_list()` is required — it copies the binary physics database to the build output directory so the executable can find it at runtime.

## Console output

```
Depth[cm], Dose[mGy], NumberOfEvents, Relative uncertainty[%]
0.05, ...
0.15, ...
...
```

100 depth bins are printed, one per line.

## PNG output

| File | Description |
|---|---|
| `pencilbeam.png` | Geometry render with the beam direction shown as a line |
| `pencilbeam_color.png` | Same geometry, cylinder colored by deposited dose (0 → peak) |

## Key xraymclib concepts illustrated

| Concept | Example |
|---|---|
| Template type aliases | `using World = xraymc::World<DepthDose>` |
| Adding a geometry/scorer item | `world.addItem<DepthDose>()` |
| Material by atomic number | `Material::byZ(13)` |
| Material by NIST name | `Material::byNistName("Polymethyl Methacralate (Lucite, Perspex)")` |
| Material by chemical formula | `Material::byChemicalFormula("H2O")` |
| Building the world | `world.build(padding_cm)` — call after all items are configured |
| Running transport | `transport.runConsole(world, beam)` |
| Reading scored results | `item.depthDoseScored()` |
| Visualization | `xraymc::VisualizeWorld` |

## About xraymclib

[xraymclib](https://github.com/medicalphysics/XRayMClib) is a C++ Monte Carlo dose scoring library for diagnostic radiology energy levels. It supports voxel, mesh, and basic shape geometries, uses cross-section data from the EPICS 2025 dataset (IAEA), and generates x-ray spectra based on the Poludniowski–Evans methodology. Licensed under GPL-3.0.