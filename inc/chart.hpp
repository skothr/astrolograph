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

    DateTime    mDate;
    Location    mLocation;
    HouseSystem mHouseSystem = HOUSE_PLACIDUS;
    bool        mSidereal    = false;
    bool        mTruePos     = false;
    bool        mDraconic    = false;
    
    std::vector<ChartObject*> mObjects;
    std::vector<ObjData>      mObjectData;
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
    
    void setHouseSystem(HouseSystem hs) { mNeedUpdate |= (mHouseSystem != hs); mHouseSystem = hs; }
    HouseSystem getHouseSystem() const  { return mHouseSystem; }
    // position calculation
    void setSidereal(bool state)        { mNeedUpdate |= (state != mSidereal); mSidereal = state; }
    bool getSidereal() const            { return mSidereal; }
    void setTruePos(bool state)         { mNeedUpdate |= (state != mTruePos);  mTruePos = state; }
    bool getTruePos() const             { return mTruePos; }
    void setDraconic(bool state)        { mNeedUpdate |= (state != mDraconic); mDraconic = state; }
    bool getDraconic() const            { return mDraconic; }

    void calcAspects();
    void update();

    double getSingleAngle(ObjType obj);

    bool changed() const { return mNeedUpdate; }
    
    int aspectCount(AspectType a);

    void setAspectOrb(AspectType asp, double orb);
    void setAspectFocus(AspectType asp, bool focus);
    void setAspectVisible(AspectType asp, bool visible);
    double getAspectOrb(AspectType asp);
    bool getAspectFocus(AspectType asp);
    bool getAspectVisible(AspectType asp);

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
