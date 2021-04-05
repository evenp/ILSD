# ILSD: Interactive Linear Structures Detector

Supervised extraction of linear structures
(ridge or hollow structures, carriage tracks)
from LiDAR raw data, and visualization of cross and longitudinal profiles.

![Figure](Images/ccx02.png?raw=true)
**Extraction of a holloway structure : on left, a profile of 3D points ; at center, analyzed scans ; on right, the extracted structure displayed over the DTM view (in green, the user input ; in black, current analyzed profile position ; in blue, selected area for the holloway volume estimation).**

Ensured functionalities are surely far from complete,
but we hope that this first set will already help LiDAR analysis tasks.
Suggestions are welcome.

Authors: Philippe Even and Phuc Ngo, LORIA/ADAGIo, Université de Lorraine,
and Pierre Even, Université de Strasbourg.

Reference : Even, P., Grzesznik, A., Gebhardt, A., Chenal, T., Even, P. and
Ngo, P., 2021. Fast extraction of linear structures fromLiDAR raw data
for archaeomorphological structure prospection.
In International Archives of the Photogrammetry, Remote Sensing and Spatial
Information Sciences (proc. of the 2021 edition of the XXIVth ISPRS Congress).
<a href="https://hal.archives-ouvertes.fr/hal-03189641">hal-03189641</a>

## QUICK SETUP / COMPILATION GUIDE
### windows

1. download and install visual studio 2019 and cmake (required to compile glfw)

2. optionally (if sources are not present in "deps" sub-directories),
download submodules --
From the depository root, run: `git submodule update --init --recursive`

3. compile and install libraries --
Go to the tools directory, then execute: `buildLibs.bat`
then `copyLibraryFiles.bat`

4. generate project files -- Run: `GenerateProjectFiles.bat`

5. compile "ILSD.sln" within visual studio
(use the release configuration for performance reasons),
or compile and run by: `ILSD.bat` (compiles both DEBUG and RELEASE),
then simple run by: `ILSD\multi\binaries\ILSD\Release\ILSD.exe`

Always run from "resources" folder (already configured for visual studio).

### linux

1. instal glfw dependencies --
More informations on https://www.glfw.org/docs/latest/compile.html,
section "Dependencies" (apt install xorg-dev for ubuntu)

2. optionally (if sources are not present in "deps" sub-directories),
download submodules --
From the depository root, run: `git submodule update --init --recursive`

3. compile and install libraries --
Go to the tools directory, then execute `buildLibs.sh`
then `copyLibraryFiles.sh`

4. generate project files -- Run `GenerateProjectFiles.sh`

5. compile and start program: `ILSD.sh`

The application can be compiled in debug mode on linux with:
`make config="debug"` (can cause heavy performance issues).

### MacOs

1. instal glfw dependencies --
More informations on https://www.glfw.org/docs/latest/compile.html,
or install with brew (brew install glfw)

2. optionally (if sources are not present in "deps" sub-directories),
download submodules --
From the depository root, run: `git submodule update --init --recursive`

3. compile and install libraries --
Go to the tools directory, then execute (in sudo mode) `bash buildLibs.sh`
then `bash copyLibraryFiles.sh`

4. generate project files -- Run `bash GenerateProjectFiles.sh`

5. compile and start program with xCode: `ILSD.xcodeproj`

## HOW TO TEST

Warning: the demo tiles must only be used for testing ILSD. For any other use,
please contact Charles.Kraemer(at)univ-lorraine.fr

1. Ridge detection: just start ILSD; you can test the detection of
the platform edge on default tiles (draw a stroke across the edge).
Then switch "Profile view visible" from "X-Profile" or "L-profile" menu
to navigate through the detected profiles.

2. Forest track detection: switch the detection mode to "track" from
the "Detection" menu. Then draw a stroke across one of the visible
tracks and test also the profiles.

3. From "Selection" menu, load a structure or a measure in "selections/ridges"
directory: wall1, wall2, or STR2 correspond to experiments on archaeological
structures described in the 2021 ISPRS Congress paper ("archeo" tiles are
loaded by default).

4. From "Files" menu, select the tile "ccx0", then from "Selection" menu,
load a structure or a measure in "selections/hollows" directory:
ccx02, ccx03, ccx04 and ccx05 correspond to experiments on hollow structures
described in the 2021 ISPRS Congress paper.
Do the same with "ccx1", "ccx2", "ccx3" and "ccx4" for the other structures.

## TILE FORMATS HANDLED

ASC format used for DTM tiles.

ASCII XYZ format for point tiles, that can be obtained from LAZ using
lastools (https::rapidlasso.com/lastools)

* to unzip LAS file: `laszip -i file.laz -o file.las`
* to extract ground points: `las2txt -i file.las -o file.xyz -keep_class 2`

### Point cloud access modes

* Mode "Top" : fast access but heavy memory consumption,
* Mode "Mid" : medium time and memory performance,
* Mode "Eco" : longer execution time but less memory required.

No difference in accuracy between these modes.
The access mode is set from "Files" menu.

__Hint:__ Initially, ILSD uses "Eco" mode for test tile size optimization.
We recommend to switch to "top" mode.
Tile files are automatically converted (this may take some time...).

### Importing new tiles

To import a new tile (from "Files" menu):

1. select the xyz point file (xyz format) and "validate";
2. select at first the DTM tile (asc format), then its 8 (or less)
neighbours and "validate";
3. new tiles are built and stored in "resources/nvm" and "resources/til"
directories (takes a bit of time).

NB: selecting neighbour tiles is necessary to preserve the continuity
between tiles.

## APPLICATION SETTINGS

User settings are stored when exiting ILSD and loaded back at next start.
It is always possible to come back to the default settings by removing
the configuration file: "resources/config/ILSD.ini"

### Extraction modes

* Mode "Ridge" : ridge structures with possible volume measures
when local trend is detected on each side.
* Mode "Hollow" : hollow structures with possible volume measures
when local trend is detected on each side.
* Mode "Track" : mountain carriage tracks with bounded horizontal cross profile.
* Mode "None" : simple straight strip to analyse cross and longitudinal
altimetric profiles with controlable width.

The selected mode is set from "Detection" menu or Ctrl-M key.
It appears in the main window title.

### Straight strip extraction

In extraction mode "None" (for no structure to detect),
the input stroke indicates the strip central line.
Longitudinal and cross profiles can be displayed
("L profiles" and "X profiles" menus).

### Ridge or hollow structure extraction

The extracted structure can be displayed as:

* the disconnected set of detected cross profiles,
* the connected set of detected cross profiles,
* the hull of the cross profiles ("bounds"),
* the line connecting cross profile summits,
* the line connecting cross profile centers (more stable than summits).

Cross profile "centers" can be defined as the center of the cross profile area
or as its projection on the cross profile hull.

Detection options and parameters are left under user control ("Detection" menu)
to adapt them to different topographic contexts.
Generally, when setting them, more control improves detection accuracy
but produces shorter structures.

"Trend detection": local trends on each side of the cross profile must first
be detected. Local trends ensure more accurate section area measure.
"Trend roughness" parameter controls the larger thickness value accepted.

"Mass-center reference" selects the cross profile area center rather than
its projection on the point surface to control deviations between successive
profiles.
This choice is less subject to surface fluctuations, but it requires a more
stable structure shape than using the projected center.

Profiles can be validated according to the deviation of:

* the cross profile center (side) position ("Position"),
* the cross profile center altitude ("Altitude"),
* the cross profile estimated width ("Width").
* the cross profile estimated height ("Height").

The deviation thresholds can be set using absolute metric values ("Absolute")
or proportional values to input stroke ("Relative") .

"Direction control" and "Slope control" refer to the center side position
and altitude shifts integrated over several successive profiles. They are
used to try to detect abrupt changes.

"Bump lack tolerance" refers to the maximal number of successive undetected
profiles accepted (*bump* is the name given to ridge cross profiles).
Fails due to insufficient ground points are not counted, so that occluded
areas by a dense vegetation cover may be crossed.

"Bump minimal width" and "Bump minimal height" values can also be set to
adapt to different application contexts.

### Carriage track structure extraction

"Pre-detection": a fast initial detection can be performed to provide
a better search direction.

"Plateau density test": discards profiles with too few plateau points
(*plateau* is the name given to the horizontal part of track cross profiles,
between side cuts).

"Plateau max tilt": refers to the tolerance to deviation from horizontality
used to discriminate roads from side slopes.

Fixing reasonable minimal and maximal values for "plateau length"
(track width) can help the detection.

Tolerance values must also be set to detect deviations of the plateau
thickness or of its center (side) position and altitude between successive
cross profiles.

"Direction control" and "Slope control" refer to the plateau (side) position
and altitude shifts integrated over several successive profiles. They are
used to try to detect abrupt changes.

"Plateau lack tolerance" refers to the maximal number of successive undetected
profiles accepted.
Fails due to insufficient ground points are not counted, so that occluded
areas by a dense vegetation cover may be crossed.

"Plateau stability control": discards ridges with hectic behavior
(unstable plateau side position). 
"Max cumulated shift" parameter refers to the extra length of the polyline
joining plateau centers compared to the distance between ridge end plateaux.

"Ridge density control": discards ridges with too many missing profiles.
"Min ridge density" parameter is the percentage of missing plateaux tolered.

"Sparse tail pruning": discards ridge ends insufficiently connected.
Tail minimal size is the minimal number of connected plateau the end
parts may count.

### Measuring ridge and hollow structures

Ridge and hollow measures can be obtained via the cross profile view.
In "X profile" menu, "Area measure" should be set.

Cross profiles through the structure layout can be inspected using the
menu or up and down arrows over the cross profile view.
On each examined profile, the area value displayed corresponds to the
surface closed by the green measure line and the polyline joining surface
points.

The blue triangle (template) connects baseline end points to the surface
center, and a blue line is drawn between the surface center and the mass
center to show their position.
The ridge width is the baseline length and the ridge height is the distance
of the surface center to the baseline.

To achieve finer area measures, the measure line can be manually set
for each profile ("Lower line translation" and "Lower line rotation").
But a new input in the main view or any detector setting will cause a
new structure detection, and the former structure will be lost.

So we recommend to perform structure measurements only at the end of the
extraction task, once a correct extraction is obtained, and to save current
measures ("Save measure" menu) as soon as possible.
All measures are written in a text file (".msr" suffixed) placed in
"resources/selections/ridges" directory by default.

Volume measures between selected profiles can also be extracted using
"Start measure" and "Stop measure" buttons. The measured structure section
is displayed with blue marks on each view.

## MOUSE CONTROLS

Click and drag using left mouse button to draw an input stroke on DTM map.
Once released, the detection is immediately performed and the found linear
structure is displayed.

## KEYBOARD CONTROLS

On Mac systems, Cmd keys should be used instead of Ctrl keys.
All the commands can also be accessed via the menu system.

### Main view controls (for azerty keyboards)

| Key | Description |
|-----|-------------|
| Arrows | Shifts DTM map |
| Page up/down | Zooms DTM map |
| Ctrl-M | Toggles extraction mode (ridge -> hollow -> track -> none) |
| Ctrl-V | Switches DTM view shading (hill-shading or slope-shading) |
| a | Adds detected structure to selection list |
| A | Displays selection list |
| b(B) | Sets light intensity for hill-shading |
| v(V) | Sets light direction for hill-shading |
| & | Switches on/off cross profile view |
| ) | Switches on/off longitudinal profile view |
| : | Inverts input stroke direction |
| Enter | Saves input stroke in "resources/tests/test.txt" |
| 0 | Loads saved stroke in "resources/test/test.txt" and runs detection |
| 9 | Runs performance test on last input stroke |
