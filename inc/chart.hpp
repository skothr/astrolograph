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
  class Chart
  {
  private:
    Ephemeris    mSwe;
    ChartParams *mParams      = nullptr;
    DateTime     mDate;
    Location     mLocation;
    HouseSystem  mHouseSystem = HOUSE_PLACIDUS;
    ZodiacType   mZodiac      = ZODIAC_TROPICAL;
    bool         mTruePos     = false;

    int mAyanamsa = 0;

    double mSiderealTime = 0.0;
    
    std::vector<ObjData*>  mObjectData;
    std::vector<ChartObject*> mObjects;
    
    std::array<double, 12>    mHouseCusps;
        
    bool mNeedUpdate = true;
    
  public:
    Chart();
    Chart(const DateTime &dt, const Location &loc);
    ~Chart();

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
    ZodiacType getZodiac() const { return mZodiac; }
    void setTruePos(bool state)  { mNeedUpdate |= (state != mTruePos);  mTruePos = state; }
    bool getTruePos() const      { return mTruePos; }
    
    void setAyanamsa(int index) { mNeedUpdate |= (mAyanamsa != index); mAyanamsa = index; }
    
    double getSiderealTime() const { return mSiderealTime; }

    std::vector<ChartAspect> calcAspects(const ChartParams &params, bool all=false);
    std::vector<ChartAspect> calcAspects(Chart *other, const ChartParams &params, bool all=false);
    void update();
    double getSingleAngle(ObjType obj);
    ChartAspect getAspect(ObjType obj1, ObjType obj2, const ChartParams &params);

    bool hasChanged() const { return mNeedUpdate; }

    double getHouseCusp(int house) const;
    double getSignCusp(int sign) const;
    double getSignCusp(const std::string &name) const;    
    int getHouse(double longitude) const; // returns house number (1-12)
    int getSign(double longitude) const;  // returns sign index   (0-11)

    const ChartParams* getParams() const { return mParams; }
    ChartParams* getParams()             { return mParams; }
    void setParams(const ChartParams &params) { *mParams = params; }

    Ephemeris& swe() { return mSwe; }
    const std::vector<ChartObject*>& objects() const { return mObjects; }

    ChartObject* getObject(ObjType o) { return mObjects[o]; }
    ObjData* getObjectData(ObjType o) { return mObjectData[o]; }

    const DateTime& date() const     { return mDate; }
    const Location& location() const { return mLocation; }
    DateTime& date()     { return mDate; }
    Location& location() { return mLocation; }
  };
  
}

#endif // CHART_HPP
