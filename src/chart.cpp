#include "chart.hpp"
using namespace astro;

#include <array>
#include <string>
#include <cctype>

//// CHART ////
Chart::Chart(const DateTime &dt, const Location &loc)
  : mDate(dt), mLocation(loc), mParams(new ChartParams())
{
  for(int o = 0; o < OBJ_END; o++)
    { mObjectData.push_back(new ObjData{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, (ObjType)o, false}); }
  for(int o = 0; o < OBJ_END; o++)
    { mObjects.push_back(new ChartObject{mObjectData[o], (ObjType)o, 0.0, false, false, false}); }

  // for(int asp = 0; asp < ASPECT_COUNT; asp++)
  //   {
  //     mAspectOrbs[asp]    = getAspectInfo((AspectType)asp)->orb;
  //     mAspectVisible[asp] = true;
  //     mAspectFocus[asp]   = false;
  //   }
}
Chart::Chart()
  : Chart(DateTime(), Location())
{ }

Chart::~Chart()
{
  if(mParams) { delete mParams; }
  for(auto obj : mObjects)    { delete obj; }
  for(auto obj : mObjectData) { delete obj; }
  mObjects.clear();
  mObjectData.clear();
}

void Chart::setDate(const DateTime &dt)
{
  if(!dt.valid()) { std::cout << "WARNING: Chart::setDate --> Invalid date: " << dt << "\n"; }
  if(dt != mDate)
    {
      mDate = dt;
      mDate.fix();
      mNeedUpdate = true;
    }
}

void Chart::setLocation(const Location &loc)
{
  if(!loc.valid()) { std::cout << "WARNING: Chart::setLocation --> Invalid location: " << loc << "\n"; }
  if(loc != mLocation)
    {
      mLocation = loc;
      mLocation.fix();
      mNeedUpdate = true;
    }
}

std::vector<ChartAspect> Chart::calcAspects(const ChartParams &params, bool all)
{
  std::vector<ChartAspect> aspects;
  for(int o1 = 0; o1 < OBJ_END; o1++)
    {
      if(!all && !params.objVisible[o1]) { continue; } // skip if switched off
      double angle1 = mObjects[o1]->angle;
      std::string name1 = getObjName((ObjType)o1);

      for(int o2 = o1+1; o2 < OBJ_END; o2++)
        {
          if(!all && !params.objVisible[o2]) { continue; } // skip if switched off
          double angle2 = mObjects[o2]->angle;
          std::string name2 = getObjName((ObjType)o2);
          double diff = angleDiffDegrees(angle1, angle2);
          for(auto &iter : ASPECTS)
            {
              if(!all && !params.aspVisible[iter.second.type]) { continue; } // skip if switched off
              double aDiff = angleDiffDegrees(diff, iter.second.angle);
              double orb = std::min(params.aspOrbs[(int)iter.second.type], std::min(params.objOrbs[o1], params.objOrbs[o2]));
              if(std::abs(aDiff) <= orb)
                {
                  double strength = 1.0 - (std::abs(aDiff) / orb);
                  aspects.emplace_back(mObjects[o1], mObjects[o2], iter.second.type, aDiff, strength,
                                       true, params.aspVisible[iter.second.type], params.aspFocused[iter.second.type]); // valid, visible, focused
                }
            }
        }
    }
  // sort aspects by orb (reverse?)
  std::sort(aspects.begin(), aspects.end(),
            [](const ChartAspect &a, const ChartAspect &b) -> bool
            {
              if(std::abs(a.orb - b.orb) < 0.001)
                { // differentiate by aspect type, then object types
                  if(a.type < b.type)      { return true; }
                  else if(a.obj1 < b.obj1) { return true; }
                  else if(a.obj2 < b.obj2) { return true; }
                  else { return false; }
                }
              else // return smaller orb
                { return (a.orb < b.orb); }
            } ); // sort by orb (ascending)  
  return aspects;
}

std::vector<ChartAspect> Chart::calcAspects(Chart *other, const ChartParams &params, bool all)
{
  if(!other) { return { }; }

  std::vector<ChartAspect> aspects;  
  for(int o1 = 0; o1 < OBJ_END; o1++)
    {
      if(!all && !params.objVisible[o1]) { continue; } // skip if switched off
      int i1 = o1;
      double angle1 = other->objects()[o1]->angle;
      std::string name1 = getObjName((ObjType)o1);

      // object aspects
      for(int o2 = o1+1; o2 < OBJ_END; o2++)
        {
          if(!all && !params.objVisible[o2]) { continue; } // skip if switched off
          int i2 = o2;
          double angle2 = objects()[o2]->angle;
          std::string name2 = getObjName((ObjType)o2);
          double diff = astro::angleDiffDegrees(angle1, angle2);
          
          for(auto &iter : astro::ASPECTS)
            {
              if(!all && !params.aspVisible[iter.second.type]) { continue; } // skip if switched off
              double aDiff = astro::angleDiffDegrees(diff, iter.second.angle);
              double orb = std::min(params.aspOrbs[(int)iter.second.type], std::min(params.objOrbs[o1], params.objOrbs[o2]));
              if(std::abs(aDiff) <= orb)
                {
                  double strength = 1.0 - (std::abs(aDiff) / orb);
                  aspects.emplace_back(other->objects()[o1], mObjects[o2], iter.second.type, aDiff, strength,
                                       true, params.aspVisible[iter.second.type], params.aspFocused[iter.second.type]); // valid, visible, focused
                }
            }
        }
    }

  // sort aspects by orb (reverse?)
  std::sort(aspects.begin(), aspects.end(),
            [](const ChartAspect &a, const ChartAspect &b) -> bool
            {
              if(std::abs(a.orb - b.orb) < 0.001)
                { // differentiate by aspect type, then object types
                  if(a.type < b.type)      { return true; }
                  else if(a.obj1 < b.obj1) { return true; }
                  else if(a.obj2 < b.obj2) { return true; }
                  else { return false; }
                }
              else // return smaller orb
                { return (a.orb < b.orb); }
            } ); // sort by orb (ascending)
  return aspects;
}










void Chart::update()
{
  if(mNeedUpdate)
    {
      //std::cout << "UPDATING CHART!!\n";
      
      // update chart info (via Swiss Ephemeris wrapper)
      mLocation.fix();
      mDate.fix();
      // DateTime minDate = mDate;
      // minDate.setYear(-8000); minDate.setMonth(1); minDate.setDay(1); minDate.setHour(0); minDate.setMinute(0); minDate.setSecond(0.0); minDate.fix();
      // if(mDate < DateTime()) { mDate = minDate; }
      //std::cout << mDate.year() << "\n";
      mSwe.setLocation(mLocation);
      mSwe.setDate(mDate);
      mSwe.setSidereal(mZodiac == ZODIAC_SIDEREAL);
      if(mZodiac == ZODIAC_SIDEREAL) { mSwe.setAyanamsa(mAyanamsa); } // TEMP
      mSwe.setTruePos(mTruePos);
      mSwe.calcHouses(mHouseSystem);
      mSiderealTime = mSwe.getSiderealTime(mDate, mLocation);

      for(int hi = 0; hi < 12; hi++) // get house cusps
        { mHouseCusps[hi] = mSwe.getHouseCusp(hi+1); }
      
      for(int i = 0; i < mObjects.size(); i++)
        { // calc objects
          ObjType o = (ObjType)i;//(ObjType)(OBJ_SUN + i);
          //if(o >= OBJ_COUNT) { o = (ObjType)(o-OBJ_COUNT+ANGLE_OFFSET); } // correct for angles
          ChartObject *obj = mObjects[i];
          *obj->data = mSwe.getObjData((ObjType)o);
          obj->valid = obj->data->valid;
          obj->angle = obj->data->longitude;
          obj->retrograde = (obj->data->lonSpeed < 0.0);
        }

      if(mZodiac == ZODIAC_DRACONIC)
        { // set aries 0-degrees to true node 
          double nnAngle = mObjects[OBJ_NORTHNODE]->angle;
          for(auto obj : mObjects)        // orient object positions
            { obj->angle =  fmod(obj->angle - nnAngle + 360.0, 360.0); }
          for(int hi = 0; hi < 12; hi++) // orient houses
            { mHouseCusps[hi] = fmod(mHouseCusps[hi] - nnAngle + 360.0, 360.0); }
        }
      
      //calcAspects();
      mNeedUpdate = false;
    }
}

double Chart::getSingleAngle(ObjType obj)
{
  if(!mNeedUpdate)
    { return getObject(obj)->angle; }
  else
    {
      // update chart info (via Swiss Ephemeris wrapper)
      mLocation.fix();
      mDate.fix();
      mSwe.setLocation(mLocation);
      mSwe.setDate(mDate);
      mSwe.setSidereal(mZodiac == ZODIAC_SIDEREAL);
      mSwe.setTruePos(mTruePos);
      
      if(obj >= ANGLE_OFFSET) { mSwe.calcHouses(mHouseSystem); }
      
      double angle = mSwe.getObjData(obj).longitude;
      if(mZodiac == ZODIAC_DRACONIC) // set aries 0-degrees to true node
        { angle = fmod(angle - mSwe.getObjData(OBJ_NORTHNODE).longitude + 360.0, 360.0); }
      
      return angle;
    }
}

ChartAspect Chart::getAspect(ObjType obj1, ObjType obj2, const ChartParams &params)
{
  // TODO: check if need update?
  int i1 = obj1;//-OBJ_SUN;
  int i2 = obj2;//-OBJ_SUN;
  
  double angle1 = mObjects[obj1]->angle;
  double angle2 = mObjects[obj2]->angle;
  double diff = angleDiffDegrees(angle1, angle2);
  for(auto &iter : ASPECTS)
    {
      double aDiff = angleDiffDegrees(diff, iter.second.angle);
      double orb = params.aspOrbs[(int)iter.second.type];
      if(std::abs(aDiff) <= orb)
        {
          // aspects sorted from strongest to weakest
          double strength = 1.0 - (std::abs(aDiff) / orb);
          return ChartAspect(mObjects[i1], mObjects[i2], iter.second.type, aDiff, strength,
                             true, params.aspVisible[iter.second.type], params.aspFocused[iter.second.type]); // valid, visible, focused
        }
    }
  return ChartAspect(); // (valid = false)
}

double Chart::getHouseCusp(int house) const
{ return mHouseCusps[house-1]; }
double Chart::getSignCusp(int sign) const // (aries = 0)
{ return ((sign >= 0 && sign < 12) ? sign*30.0 : -1.0); }
double Chart::getSignCusp(const std::string &name) const
{ return getSignCusp(getSignIndex(name)); }

// return index of the sign containing the given ecliptic angle
int Chart::getSign(double longitude) const
{ return (int)std::floor(fmod(longitude, 360.0)/30.0); }
// return number of the house containing the given ecliptic angle
int Chart::getHouse(double longitude) const
{
  for(int i = 1; i <= 12; i++)
    {
      int ni = (i == 12 ? 1 : i+1);
      double h1 = getHouseCusp(i);
      double h2 = getHouseCusp(ni);
      if(anglesContainDegrees(h1, h2, longitude)) { return i; }
    }
  return -1;
}
