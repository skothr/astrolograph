#ifndef LOCATION_WIDGET_HPP
#define LOCATION_WIDGET_HPP

#include "astro.hpp"
// #include <bits/stdc++.h>

namespace astro
{
  // forward declarations
  class NodeGraph;
  class FileDialog;
  
#define LOCATION_SAVE_DIR "locations/"
// #define LOCATION_SAVE_PATH LOCATION_SAVE_DIR "locations.txt"
#define LOCATION_NAME_BUFLEN 128

  struct LocationSave
  {
    std::string name;
    Location location;
  };
  
  class LocationWidget
  {
  private:
    NodeGraph  *mGraph      = nullptr;
    FileDialog *mFileDialog = nullptr;
    Location mLocation;
    Location mSavedLocation;
    std::string mName       = "";
    bool mExpanded          = false; // expanded positional view (WIP)

    bool saveDirCheck();
    
  public:
    LocationWidget();
    LocationWidget(const Location &location);
    LocationWidget(const LocationWidget &other);
    LocationWidget& operator=(const LocationWidget &other);
    ~LocationWidget();
    void setGraph(NodeGraph *graph) { mGraph = graph; }
    
    Location& get()                    { return mLocation; }
    const Location& get() const        { return mLocation; }
    Location& getSaved()               { return mSavedLocation; }
    const Location& getSaved() const   { return mSavedLocation; }
    std::string& getName()             { return mName; }
    const bool& getExpanded() const    { return mExpanded; }
    bool& getExpanded()                { return mExpanded; }
    void set(const Location &loc)      { mLocation = loc; }
    void setSaved(const Location &loc) { mSavedLocation = loc; }
    void setName(const std::string &n) { mName = n; }
    void setExpanded(bool expanded)    { mExpanded = expanded; }

#ifndef ENABLE_CUDA // not needed for building CUDA files (std::quoted undefined)
    bool checkFileDialog();
#endif // ENABLE_CUDA
    
    void update();
    void draw(float scale, bool blocked);
  };
}


#endif // LOCATION_WIDGET_HPP
