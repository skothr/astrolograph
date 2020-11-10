#ifndef CHART_HPP
#define CHART_HPP

#include "astro.hpp"
#include "vector.hpp"
#include "ephemeris.hpp"

#include <vector>

namespace astro
{
  // represents an object in the chart
  struct ChartObject
  {
    ObjType type = OBJ_INVALID;
    double angle = 0.0;
    bool visible = true;
    bool focused = false;
  };
  // represents an aspect between chart objects
  struct Aspect
  {
    AspectType   type = ASPECT_INVALID;
    ChartObject *obj1 = nullptr;
    ChartObject *obj2 = nullptr;
    
    double orb      = 0.0; // angle difference from perfectly aligned aspect (orb)
    double strength = 0.0; // aspect strength ([0.0, 1.0] -- currently squared)
    bool   visible = true;
    bool   focused = false;

    Aspect() { }
    Aspect(ChartObject *o1, ChartObject *o2, AspectType type_, double orb_, double strength_)
      : obj1(o1), obj2(o2), type(type_), orb(orb_), strength(strength_) { }
  };
  
  class Chart
  {
  private:

    ///////// INSIDE DEGREES TEST //////////
#define INSIDE_DEGREES_PATH "./res/inside-degrees.txt"
    static std::array<std::array<std::string, 30>, 12> insideDegreesShort; // ACCESS: arr[SIGN_INDEX][floor(DEGREE)]
    static std::array<std::array<std::string, 30>, 12> insideDegreesLong;  // ACCESS: arr[SIGN_INDEX][floor(DEGREE)]
    static bool mInsideDegreesLoaded;
    static bool loadInsideDegrees();

    DateTime mDate;
    Location mLocation;
    
    std::vector<ChartObject*> mObjects;
    std::vector<Aspect>       mAspects;
    // global per-aspect params
    std::array<double, ASPECT_COUNT> mAspectOrbs;
    std::array<bool,   ASPECT_COUNT> mAspectFocus;
    std::array<bool,   ASPECT_COUNT> mAspectVisible;
    
    Ephemeris mSwe;
    bool mNeedUpdate = true;
    
  public:
    Chart();
    Chart(const DateTime &dt, const Location &loc);
    ~Chart();

    static std::string getInsideDegreeTextShort(int sign, int degree);
    static std::string getInsideDegreeTextLong(int sign, int degree);

    void setDate(const DateTime &dt);
    void setLocation(const Location &loc);
    
    void calcAspects();
    void update();

    bool changed() const { return mNeedUpdate; }
    
    int aspectCount(AspectType a);

    void setAspectOrb(AspectType asp, double orb)
    { if(asp > ASPECT_INVALID && asp < ASPECT_COUNT) { mAspectOrbs[(int)asp] = orb; mNeedUpdate = true; } }
    double getAspectOrb(AspectType asp)
    { if(asp > ASPECT_INVALID && asp < ASPECT_COUNT) { return mAspectOrbs[(int)asp]; mNeedUpdate = true; } }
    void setAspectFocus(AspectType asp, bool focus)
    { if(asp > ASPECT_INVALID && asp < ASPECT_COUNT) { mAspectFocus[(int)asp] = focus; mNeedUpdate = true; } }
    void setAspectVisible(AspectType asp, bool visible)
    { if(asp > ASPECT_INVALID && asp < ASPECT_COUNT) { mAspectVisible[(int)asp] = visible; mNeedUpdate = true; } }
    bool getAspectFocus(AspectType asp)   { return (asp > ASPECT_INVALID && asp < ASPECT_COUNT) ? mAspectFocus[(int)asp]   : false; }
    bool getAspectVisible(AspectType asp) { return (asp > ASPECT_INVALID && asp < ASPECT_COUNT) ? mAspectVisible[(int)asp] : false; }

    Ephemeris& swe() { return mSwe; }
    const std::vector<Aspect>&       aspects() const { return mAspects; }
    const std::vector<ChartObject*>& objects() const { return mObjects; }

    ChartObject* getObject(ObjType o)         { return mObjects[(o<OBJ_COUNT ? o-OBJ_SUN : OBJ_COUNT+o-ANGLE_OFFSET)]; }
    void showObject(ObjType o, bool visible)  { getObject(o)->visible = visible; }
    void setObjFocus(ObjType o, bool focused) { getObject(o)->focused = focused; }

    const DateTime& date() const     { return mDate; }
    const Location& location() const { return mLocation; }
    
  };
  
}

#endif // CHART_HPP
