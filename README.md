# AstroloGraph
Node graph-based astrological data viewer.

![Simple chart setup with date and location inputs](images/simple-example.png?raw=true "Simple Example")
![More complex setup with progressed chart and comparison](images/complex-example.png?raw=true "More Complex Example")

NOTE: This software is still in development and features may be missing/buggy/broken. Calculations should be double-checked for accuracy (especially time zones).

## Build
### Linux (tested on Ubuntu 18.04)
* `sudo apt-get update`
* `sudo apt-get install build-essential cmake libglfw3-dev libglew-dev libcurl4-openssl-dev`
* `./make-release.sh`
  * Alternatively:
    * `mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release ..`
### Windows
* TODO
### Mac
* TODO

# Basic Instructions
* TODO: Improve Instructions
* Nodes have inputs(left side) and/or outputs(right side) that can be connected to other nodes by clicking and dragging.
* To add a node, right click on the graph background and select from the menu, or press the associated key (denoted in parentheses below). Then click where you want to place it (shift+click for multiple).

## Node Types
### Data Sources
* (T) Time Node (date and time)
* (S) Time Span Node (plays over a range of time)
* (L) Location Node (latitude/longitude/altitude)
### Calculation
* (C) Chart Node (calculates positions via the Swiss Ephemeris)
* (P) Progress Node (calculates secondary progressed date)
### Data/Visualization
* (V) Chart View Node (view a chart)
  * Interactive chart display.
    * Hover over any aspect symbol to show its orb and any objects involved.
    * Hover over any object to see its placement.
      * Hold SHIFT to highlight object aspects.
      * Hold ALT to see inside degrees text for its placement.
        * Hold CTRL+ALT show inside degrees text (long, with explanation).
    * Hover over any sign or house to see the objects it contains.
  * With the mouse over the view, hold a key and scroll the mouse to manually adjust time and location.
    * TIME: number keys 1-6
      * 1 --> adjust by year
      * 2 --> adjust by month
      * 3 --> adjust by day
      * 4 --> adjust by hour
      * 5 --> adjust by minute
      * 6 --> adjust by second
    * LOCATION: keys Q/W/E
      * Q --> adjust latitude
      * W --> adjust longitude
      * E --> adjust altitude
    * By default, each parameter steps by one unit. Modifier keys will apply a multiplier:
      * SHIFT --> x0.1
      * CTRL  --> x10
      * ALT   --> x60
* (X) Chart Compare Node (view comparison between two charts)
* (D) Chart Data Node (view raw positional data)
* (A) Aspect Node (view in-depth aspect data)
* (M) Moon Node (render moon phase)
* ()  Plot Node (plot chart positions over a time range)

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
* libcurl (to query GeoNames --> getting timezone from coordinates)  
  * https://curl.se/download.html
  
# Contact
* skothr@gmail.com.