# Native GUI demo

Build the optional GUI with C++20, CMake 3.24 or newer, Qt 6.5 or newer and a matching C++ toolchain. CLI/core builds do not need Qt and do not download GUI dependencies.

```powershell
cmake -S . -B build -DPROLOGUE_BUILD_GUI=ON -DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/msvc2022_64
cmake --build build --config Debug --parallel 4
ctest --test-dir build/test -C Debug --output-on-failure
```

CMake uses an installed `JKQTPlotterSharedLib` package if present (`JKQTPlotterSharedLib_DIR` points at its `lib/cmake` folder); otherwise it downloads the pinned shared-library source. It also downloads pinned miniz. Windows builds deploy Qt plugins, JKQTPlotter DLLs, license texts, and the explicit repository sample inputs beside `build/gui/PrologueGUI.exe`. Debug binaries require a development machine with the matching MSVC debug runtime. See [third-party notices](THIRD_PARTY_NOTICES.md).

Open `PrologueGUI.exe`, or specify `--project path/to/project.json --settings path/to/prologue.settings.json`. The sample project opens by default. The project and settings tabs edit the same version-2 JSON and settings format used by the CLI. Save As keeps referenced thrust/aerodynamic/wind files pointing to the original resources. It does not copy those resources. Interactive execution conditions (detail/scatter, descent mode, detachment and wind) are selected above the tabs, as in the CLI.

Run a simulation, select any numerical X and Y quantities in the graph tab, select one/all bodies, and export PNG, SVG or PDF. Presets cover time history, downrange/altitude, stability margin, pressure and airspeed. Scatter summaries can show ENU landing rings grouped by wind speed. Labels use explicit units; angle in radians and degrees are separate choices.

The map tab shows launch, trajectories/landing points or scatter rings. Add local KML/KMZ safety areas, toggle their visibility, pan/zoom, then export a map figure after all visible tiles load. Figures include a legend, scale and GSI attribution. Internet access is required for the base map; overlays and graphs use local data.

`CSV/KMLを保存` writes the shared CLI result formats into a new timestamped folder. Input saves are atomic. Figures support Japanese output paths. Detailed polar heatmaps and automatic report assembly remain outside this demo's scope.
