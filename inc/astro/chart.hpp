#ifndef CHART_HPP
#define CHART_HPP

#include "astro.hpp"
#include "vector.hpp"
#include "ephemeris.hpp"

#include <vector>
#include <string>
#include <iostream>

#define MAX_STEPS 2048

namespace astro
{
  struct ObjRx
  {
    DateTime preShadow;
    DateTime rxStation;
    DateTime dxStation;
    DateTime postShadow;
    bool preValid  = false;
    bool rxValid   = false;
    bool dxValid   = false;
    bool postValid = false;
  };
  
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
    
    std::vector<ObjData*>     mObjectData;
    std::vector<ChartObject*> mObjects;
    std::array<double, 12>    mHouseCusps;
    
    bool mNeedUpdate = true;
    bool mDebug = false;
    
  public:
    Chart();
    Chart(const DateTime &dt, const Location &loc);
    ~Chart();

    void outputToFile(const std::string &path, const std::string &name="", bool append=true, bool validAngles=true);
    
    void setDebug(bool debug) { mDebug = debug; }
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
      setZodiac(zodiac);
    }
    ZodiacType getZodiac() const { return mZodiac; }
    void setTruePos(bool state)  { mNeedUpdate |= (state != mTruePos);  mTruePos = state; }
    bool getTruePos() const      { return mTruePos; }
    
    void setAyanamsa(int index) { mNeedUpdate |= (mAyanamsa != index); mAyanamsa = index; }
    
    double getSiderealTime() const { return mSiderealTime; }

    std::vector<ChartAspect> calcAspects(const ChartParams &params, bool all=false);
    std::vector<ChartAspect> calcAspects(Chart *other, const ChartParams &params, bool all=false);
    void update();
    double getSingleAngle(ObjType o);
    ChartObject* getSingleObject(ObjType o);
    ChartAspect getAspect(ObjType o1, ObjType o2, const ChartParams &params);

    bool hasChanged() const { return mNeedUpdate; }

    double getHouseCusp(int house) const;
    double getSignCusp(int sign) const;
    double getSignCusp(const std::string &name) const;    
    int getHouse(double longitude) const; // returns house number (1-12)
    int getSign(double longitude) const;  // returns sign index   (0-11)
    std::string getSignChar(double longitude) const;
    std::string getObjString(double longitude) const;

    const ChartParams* getParams() const { return mParams; }
    ChartParams* getParams()             { return mParams; }
    void setParams(const ChartParams &params) { *mParams = params; }

    Ephemeris& swe() { return mSwe; }
    const std::vector<ChartObject*>& objects() const { return mObjects; }

    ChartObject* getObject(ObjType o) { return (o < mObjects.size() ? mObjects[o] : nullptr); }
    ObjData* getObjectData(ObjType o) { return (o < mObjectData.size() ? mObjectData[o] : nullptr); }

    const DateTime& date() const     { return mDate; }
    const Location& location() const { return mLocation; }
    DateTime& date()     { return mDate; }
    Location& location() { return mLocation; }
    
    bool findRxStations(const DateTime &start, const DateTime &end, ObjType o, std::vector<ObjRx> &stations,
                        double maxError, double minTimeRange, int *stepsTaken=nullptr, int level=0);
    
    std::vector<DateTime> findAspects(const DateTime &start, const DateTime &end, ObjType o1, ObjType o2, AspectType a, double maxAngle=(1.0/3600.0/60.0),
                                      double *error=nullptr, int *stepsTaken=nullptr);

    /// NOTE: doesn't use end date
    DateTime findAspectPeak(const DateTime &start, const DateTime &end, ObjType o1, ObjType o2, AspectType a, double maxAngle=(1.0/3600.0/60.0),
                            double *error=nullptr, int *stepsTaken=nullptr);
    
  };
  
}

#endif // CHART_HPP
