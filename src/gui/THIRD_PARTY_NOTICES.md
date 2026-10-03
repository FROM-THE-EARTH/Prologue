# GUI demo dependencies

Prologue's own source remains MIT licensed. The GUI links Qt and JKQTPlotter dynamically; keep their libraries replaceable. No Qt Charts or Qt WebEngine module is used. This development demo is not a release package.

| Dependency | Version/source | License | Use |
| --- | --- | --- | --- |
| Qt | Tested with 6.8.3, https://code.qt.io/qt/qtbase.git/ and https://code.qt.io/qt/qtsvg.git/ (tag v6.8.3) | LGPL-3.0-only for the selected modules | Widgets, network tiles, SVG/PDF output, worker execution |
| JKQTPlotter | v4.0.3, commit `0d4e3bff7dfac7eb40ec7b14eda1f188b448a119`, https://github.com/jkriege2/JKQtPlotter | LGPL-2.1-or-later | Native XY graphs and figure export |
| XITS fonts | Bundled with JKQTMathText at the above commit | SIL Open Font License 1.1 | Mathematical labels |
| miniz | 3.1.2, commit `77d0dce8627735138c51770d1799a1ef48f2117d`, https://github.com/richgel999/miniz | MIT | Local KMZ archive reading |

Full license texts are in `licenses/`. The GUI dependency source archives and hashes are pinned in `CMakeLists.txt`; upstream source links above provide the corresponding sources. Qt's binary SDK includes its third-party SPDX notices in `sbom/`; retain those notices when distributing a binary built with that SDK. Redistribution requires the notices for the actual Qt build, its corresponding source, and the applicable LGPL obligations. The repository does not include the SDK itself.

JKQTPlotter copyright (c) 2008–2020 Jan W. Krieger. It is provided without warranty under LGPL-2.1-or-later. Qt copyright belongs to The Qt Company Ltd. and its contributors; consult the SDK's module and third-party notices. The XITS copyright notices accompany its full OFL text.

## Map data

Tiles are fetched directly over HTTPS from the Geospatial Information Authority of Japan:
`https://cyberjapandata.gsi.go.jp/xyz/pale/{z}/{x}/{y}.png`.
The demo uses zoom levels 9–18. Tile requests include tile coordinates; local rocket inputs and KML/KMZ contents are not uploaded.

The map and exported figures display GSI attribution and the processing notice. Tile descriptions and terms:
https://maps.gsi.go.jp/development/ichiran.html
https://www.gsi.go.jp/kikakuchousei/kikakuchousei40182.html

KML/KMZ overlays stay local. The demo renders points, line strings and polygon boundaries/fills, including holes and normal styles. It does not execute network links or HTML, or fetch remote icons. Embedded icon artwork and complex 3D styles are not rendered.
