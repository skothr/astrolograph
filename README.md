# AstroloGraph
Node graph-based astrological data viewer.

![Simple chart setup with date and location inputs](images/simple-example.png?raw=true "Simple Example")
![More complex setup with progressed chart and comparison](images/complex-example.png?raw=true "More Complex Example")

NOTE: This software is still in development and features may be missing/buggy/broken. Calculations should be double-checked for accuracy (especially time zones).

## Build
### Linux (tested on Ubuntu 18.04)
* `sudo apt-get update`
* `sudo apt-get install build-essential cmake libglfw3-dev libglew-dev`
* `./make-release.sh`
  * Alternatively:
    * `mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release ..`

### Windows
* TODO (Possible process: import into a visual studio cmake project)

### Mac
* TODO

# Basic Instructions
* Nodes have inputs(left side) and/or outputs(right side) that can be connected to other nodes by clicking and dragging.
* To add a node, right click on the graph background and select from the menu, or press the associated key (denoted in parentheses below).
* TODO: Improve Instructions
* TODO: Add example save files

## Node Types
### Data Sources
* (T) Time Node (date and time)
* (S) Time Span Node (start/end time)
* (L) Location Node (latitude/longitude/altitude)
### Calculation
* (C) Chart Node (calculates positions via the Swiss Ephemeris)
* (P) Progress Node (calculates secondary progressed date)
### Data/Visualization
* (V) Chart View Node (view a chart)
* (X) Chart Compare Node (view comparison between two charts)
* (D) Chart Data Node (view in-depth positional chart data)
* (A) Aspect Node (view in-depth aspect data)


## Dependencies
* GLFW3
  * https://www.glfw.org/
* GLEW
  * http://glew.sourceforge.net/
* Dear ImGui,
  * https://github.com/ocornut/imgui
* stb_image
  * https://github.com/nothings/stb
* Swiss Ephemeris
  * https://www.astro.com/swisseph/swephinfo_e.htm
* Date / tz (for timezone calculations)
  * https://github.com/HowardHinnant/date
  
# Contact
* skothr@gmail.com.