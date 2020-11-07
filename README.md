# AstroloGraph
Node graph-based astrological data viewer

NOTE: This software is still in development and features may be missing/buggy/broken. The node interface is currently a bit clunky, and calculations should be double-checked for accuracy (especially time zones).

## Install
### Linux (tested on Ubuntu 18.04 | TODO: VERIFY)
* sudo apt-get update
* sudo apt-get install build-essential cmake libglfw3-dev libglew-dev libswe-dev
* mkdir build && cd build
* ./make-debug.sh

### Windows
* TODO

### Mac
* TODO

# Basic Instructions
* The program will start empty. To add a node, right-click on the background and select the type of node.
* Nodes have inputs(left) and/or outputs(right) that can be connected to other nodes by clicking and dragging.
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
  
