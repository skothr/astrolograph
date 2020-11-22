#ifndef LOCATION_WIDGET_HPP
#define LOCATION_WIDGET_HPP

#include "astro.hpp"
#include <bits/stdc++.h>

namespace astro
{
#define LOCATION_SAVE_DIR "./saved/"
#define LOCATION_SAVE_PATH LOCATION_SAVE_DIR "locations.txt"
#define LOCATION_NAME_BUFLEN 128

  struct LocationSave
  {
    std::string name;
    Location location;
  };
  
  class LocationWidget
  {
  private:
    Location mLocation;
    Location mSavedLocation;
    char mName[LOCATION_NAME_BUFLEN] = "";
    char mSavedName[LOCATION_NAME_BUFLEN] = "";
    // bool mDST = false; // daylight savings time
    
  public:
    LocationWidget();
    LocationWidget(const Location &location);
    LocationWidget(const LocationWidget &other);
    LocationWidget& operator=(const LocationWidget &other);
    
    Location& get()                    { return mLocation; }
    const Location& get() const        { return mLocation; }
    void set(const Location &loc)      { mLocation = loc; }
    Location& getSaved()               { return mSavedLocation; }
    const Location& getSaved() const   { return mSavedLocation; }
    void setSaved(const Location &loc) { mSavedLocation = loc; }

    // bool getDST() const   { return mDST; }
    // void setDST(bool dst) { mDST = dst; }
    
    std::string getName() const { return mName; }
    void setName(const std::string &n) { sprintf(mName, "%s", n.c_str()); }
    void setSaveName(const std::string &n) { sprintf(mSavedName, "%s", n.c_str()); }
    
    bool save(const std::string &name);
    bool load(const std::string &name);
    bool remove(const std::string &name);
    std::vector<LocationSave> loadAll();
    
    void draw(float scale, bool blocked);
  };
}


#endif // LOCATION_WIDGET_HPP
