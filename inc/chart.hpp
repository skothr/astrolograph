#ifndef CHART_HPP
#define CHART_HPP

#include "astro.hpp"
#include "vector.hpp"
#include "ephemeris.hpp"

#include <vector>
#include <string>
#include <iostream>

namespace astro
{
  struct BoolStruct
  {
    bool data;
    
    BoolStruct(bool val = false) : data(val) { }
    //BoolStruct& operator=(bool val) { data = val; return *this; }
    
    //bool operator!() const { return !data; }
    operator bool() const { return data; }
      
    friend std::ostream& operator<<(std::ostream &os, const BoolStruct &b);
    friend std::istream& operator>>(std::istream &is, BoolStruct &b);
  };
  inline std::ostream& operator<<(std::ostream &os, const BoolStruct &b)
  {
    os << (b.data ? "1" : "0") << " ";
    return os;
  }
  inline std::istream& operator>>(std::istream &is, BoolStruct &b)
  {
    std::string str;
    is >> str;
    b.data = (str != "0");
    return is;
  }

  struct ChartParams
  {    
    std::array<BoolStruct, (OBJ_COUNT+OBJ_END-ANGLE_OFFSET)> objVisible;
    std::array<BoolStruct, (OBJ_COUNT+OBJ_END-ANGLE_OFFSET)> objFocused;
    std::array<double,     (OBJ_COUNT+OBJ_END-ANGLE_OFFSET)> objOrbs;
    std::array<BoolStruct, ASPECT_COUNT>                     aspVisible;
    std::array<BoolStruct, ASPECT_COUNT>                     aspFocused;
    std::array<double,     ASPECT_COUNT>                     aspOrbs;
    ChartParams()
    { // initialize defaults
      for(int i = OBJ_SUN; i < OBJ_END; i++)
        {
          objVisible[i] = (i < OBJ_COUNT); // angles initially hidden
          objFocused[i] = false;
          objOrbs[i] = (i < ANGLE_OFFSET ? OBJECT_ORBS_DEFAULT[i] : 1.0);
        }
      for(int i = 0; i < ASPECT_COUNT; i++)
        {
          aspVisible[i] = true;
          aspFocused[i] = false;
          aspOrbs[i] = getAspectInfo((AspectType)i)->orb;
        }
    }
  };
  
  // represents the position of an object in the chart
  struct ChartObject
  {
    ObjType type = OBJ_INVALID;
    double angle = 0.0;
    bool visible = true;
    bool focused = false;

    bool valid      = false; // valid data
    bool retrograde = false; // object is in retrograde motion
  };
  // represents an aspect between chart objects
  struct ChartAspect
  {
    AspectType   type = ASPECT_INVALID;
    ChartObject *obj1 = nullptr;
    ChartObject *obj2 = nullptr;
    
    double orb      = 0.0; // angle difference from perfectly aligned aspect (orb)
    double strength = 0.0; // aspect strength ([0.0, 1.0] -- currently squared)

    bool   valid   = false;
    bool   visible = true;
    bool   focused = false;

    ChartAspect() { }
    ChartAspect(ChartObject *o1, ChartObject *o2, AspectType type_, double orb_, double strength_)
      : obj1(o1), obj2(o2), type(type_), orb(orb_), strength(strength_), valid(true) { }
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
    ZodiacType  mZodiac      = ZODIAC_TROPICAL;
    bool        mTruePos     = false;
    
    std::vector<ChartObject*> mObjects;
    std::vector<ObjData>      mObjectData;
    std::vector<ChartAspect>  mAspects;

    std::array<double, 12>    mHouseCusps;
    
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
    void setZodiac(ZodiacType zodiac)   { mNeedUpdate |= (zodiac != mZodiac); mZodiac = zodiac; }
    void setZodiac(const std::string &zodiacStr)
    { // (string should be a number -- value of zodiac type enum)
      int zodiac;
      std::istringstream(zodiacStr) >> zodiac;
      setZodiac((ZodiacType)zodiac);
    }
    ZodiacType getZodiac() const   { return mZodiac; }
    void setTruePos(bool state)    { mNeedUpdate |= (state != mTruePos);  mTruePos = state; }
    bool getTruePos() const        { return mTruePos; }

    std::vector<ChartAspect> calcAspects(const ChartParams &params);
    void update();
    double getSingleAngle(ObjType obj);
    ChartAspect getAspect(ObjType obj1, ObjType obj2);

    bool hasChanged() const { return mNeedUpdate; }

    double getHouseCusp(int house) const;
    double getSignCusp(int sign) const;
    double getSignCusp(const std::string &name) const;    
    int getHouse(double longitude) const; // returns house number (1-12)
    int getSign(double longitude) const;  // returns sign index   (0-11)

    int aspectCount(AspectType a);

    void setAspectOrb(AspectType asp, double orb);
    void setAspectFocus(AspectType asp, bool focus);
    void setAspectVisible(AspectType asp, bool visible);
    double getAspectOrb(AspectType asp);
    bool getAspectFocus(AspectType asp);
    bool getAspectVisible(AspectType asp);

    Ephemeris& swe() { return mSwe; }
    const std::vector<ChartAspect>&  aspects() const { return mAspects; }
    const std::vector<ChartObject*>& objects() const { return mObjects; }

    ChartObject* getObject(ObjType o)         { return mObjects[o]; }    //(o<OBJ_COUNT ? o-OBJ_SUN : OBJ_COUNT+o-ANGLE_OFFSET)]; }
    ObjData getObjectData(ObjType o)          { return mObjectData[o]; } //(o<OBJ_COUNT ? o-OBJ_SUN : OBJ_COUNT+o-ANGLE_OFFSET)]; }
    void showObject(ObjType o, bool visible)  { getObject(o)->visible = visible; }
    void setObjFocus(ObjType o, bool focused) { getObject(o)->focused = focused; }

    const DateTime& date() const     { return mDate; }
    const Location& location() const { return mLocation; }
    DateTime& date()     { return mDate; }
    Location& location() { return mLocation; }
  };
  
}

#endif // CHART_HPP
