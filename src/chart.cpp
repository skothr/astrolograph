#include "chart.hpp"
using namespace astro;

#include <fstream>
#include <array>
#include <string>
#include <cctype>


//// INSIDE DEGREE TEXT ////
std::array<std::array<std::string, 30>, 12> Chart::insideDegreesShort; // ACCESS: arr[SIGN_INDEX][floor(DEGREE)]
std::array<std::array<std::string, 30>, 12> Chart::insideDegreesLong;  // ACCESS: arr[SIGN_INDEX][floor(DEGREE)]
bool Chart::mInsideDegreesLoaded = false;
bool Chart::loadInsideDegrees()
{
  if(!mInsideDegreesLoaded)
    {
      std::ifstream file(INSIDE_DEGREES_PATH, std::ios::in);

      std::string line;
      while(std::getline(file, line))
        {
          if(line.empty() || line == "\n" || line[0] == '#') { continue; } // skip blank and commented lines
      
          std::stringstream ss(line);
          std::string sign;
          int degree;
          ss >> sign;
          ss >> degree;

          // lowercase sign
          std::transform(sign.begin(), sign.end(), sign.begin(), [](unsigned char c){ return std::tolower(c); });

          int signIdx = getSignIndex(sign);
          if(std::getline(file, line))
            { insideDegreesShort[signIdx][degree-1] = line; }
          while(std::getline(file, line) && !line.empty() && line != "\n")
            { insideDegreesLong[signIdx][degree-1] += line; }
        }
      mInsideDegreesLoaded = true;
    }
  return true;
}

std::string Chart::getInsideDegreeTextShort(int sign, int degree)
{
  if(sign < 0 || sign >= 12 || degree < 0 || degree >= 30) { return ""; }
  else                                                     { return insideDegreesShort[sign][degree]; }
}

std::string Chart::getInsideDegreeTextLong(int sign, int degree)
{
  if(sign < 0 || sign >= 12 || degree < 0 || degree >= 30) { return ""; }
  else                                                     { return insideDegreesLong[sign][degree]; }
}



//// CHART ////
Chart::Chart(const DateTime &dt, const Location &loc)
  : mDate(dt), mLocation(loc)
{
  for(int o = OBJ_SUN; o < OBJ_COUNT; o++)
    { mObjects.push_back(new ChartObject{(ObjType)o, 0.0, true, false}); }

  for(int o = ANGLE_OFFSET; o < ANGLE_END; o++)
    { mObjects.push_back(new ChartObject{(ObjType)o, 0.0, true, false}); }

  for(int asp = ASPECT_CONJUNCTION; asp < ASPECT_COUNT; asp++)
    {
      mAspectOrbs[asp]    = getAspect((AspectType)asp)->orb;
      mAspectVisible[asp] = true;
      mAspectFocus[asp]   = false;
    }
  loadInsideDegrees();
}
Chart::Chart()
  : Chart(DateTime(), Location())
{ }

Chart::~Chart()
{
  for(auto obj : mObjects) { delete obj; }
  mObjects.clear();
  // for(auto asp : mAspects) { delete asp; }
  // mApects.clear();
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

int Chart::aspectCount(AspectType a)
{
  int count = 0;
  for(auto asp : mAspects)
    { count += (asp.type == a ? 1 : 0); }
  return count;
}

void Chart::calcAspects()
{
  mAspects.clear();
  for(int o1 = OBJ_SUN; o1 < OBJ_COUNT; o1++)
    {
      int i1 = o1-OBJ_SUN;
      double angle1 = mObjects[o1-OBJ_SUN]->angle;
      std::string name1 = getObjName((ObjType)o1);

      for(int o2 = OBJ_SUN; o2 < OBJ_COUNT; o2++)
        {
          if(o1 >= o2) { continue; } // skip duplicates
          int i2 = o2-OBJ_SUN;
          
          double angle2 = mObjects[o2-OBJ_SUN]->angle;
          std::string name2 = getObjName((ObjType)o2);
          
          double diff = angleDiffDegrees(angle1, angle2);
          
          for(auto &iter : ASPECTS)
            {
              double aDiff = angleDiffDegrees(diff, iter.second.angle);
              double orb = mAspectOrbs[(int)iter.second.type];
              if(std::abs(aDiff) <= orb)
                {
                  // sort aspects from strongest to weakest
                  double strength = 1.0 - (std::abs(aDiff) / orb);
                  mAspects.emplace_back(mObjects[i1], mObjects[i2], iter.second.type, aDiff, strength);
                }
            }
        }
      //if(mAngleAspects)
      {
        for(int o2 = ANGLE_OFFSET; o2 < ANGLE_END; o2++)
          {
            if(o1 >= o2) { continue; } // skip duplicates
            int i2 = OBJ_COUNT + o2-ANGLE_OFFSET;
              
            double angle2 = mObjects[o2-ANGLE_OFFSET+OBJ_COUNT]->angle;
            std::string name2 = getObjName((ObjType)o2);
            double diff = angleDiffDegrees(angle1, angle2);
            for(auto &iter : ASPECTS)
              {
                double aDiff = angleDiffDegrees(diff, iter.second.angle);
                double orb = mAspectOrbs[(int)iter.second.type];
                if(std::abs(aDiff) <= orb)
                  {
                    // sort aspects from strongest to weakest
                    double strength = 1.0 - (std::abs(aDiff) / orb);
                    mAspects.emplace_back(mObjects[i1], mObjects[i2], iter.second.type, aDiff, strength);
                  }
              }
          }
      }
    }

  // sort aspects by orb (reverse?)
  std::sort(mAspects.begin(), mAspects.end(),
            [](const Aspect &a, const Aspect &b) -> bool
            {
              if(std::abs(a.orb - b.orb) < 0.001)
                { // differentiate by aspect type, then object types
                  if(a.type < b.type) { return true; }
                  else if(a.obj1 < b.obj1) { return true; }
                  else if(a.obj2 < b.obj2) { return true; }
                  else { return false; }
                }
              else // return smaller orb
                { return (a.orb < b.orb); }
            } ); // sort by orb (ascending)

}

// // convert longitude(degrees) to angle on screen (radians) based on chart orientation
// float Chart::screenAngle(float longitude)
// { return M_PI/180.0f * (longitude - (mAlignAsc ? mSwe.getDsc() : 0.0f)); }

void Chart::update()
{
  if(mNeedUpdate)
    {
      // update chart info (via Swiss Ephemeris wrapper)
      mLocation.fix();
      mDate.fix();
      mSwe.setLocation(mLocation);
      mSwe.setDate(mDate);
      mSwe.setHouseSystem(mHouseSystem);
      mSwe.setSidereal(mZodiac == ZODIAC_SIDEREAL);
      mSwe.setTruePos(mTruePos);
      mSwe.calcHouses();
      
      mObjectData.clear();
      for(int i = 0; i < mObjects.size(); i++)
        { // calc objects
          ObjType o = (ObjType)(OBJ_SUN + i);
          if(o >= OBJ_COUNT) { o = (ObjType)(o-OBJ_COUNT+ANGLE_OFFSET); } // correct for angles
          ChartObject *obj = mObjects[i];
          mObjectData.push_back(mSwe.getObjData((ObjType)o));
          obj->valid = mObjectData.back().valid;
          obj->angle = mObjectData.back().longitude;
          obj->retrograde = (mObjectData.back().lonSpeed < 0.0);
        }

      if(mZodiac == ZODIAC_DRACONIC)
        { // set aries 0-degrees to true node 
          double nnAngle = mObjects[OBJ_NORTHNODE]->angle;
          for(auto obj : mObjects)
            { obj->angle =  fmod(obj->angle - nnAngle + 360.0, 360.0); }
        }
      
      calcAspects();
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
      mSwe.setHouseSystem(mHouseSystem);
      mSwe.setSidereal(mZodiac == ZODIAC_SIDEREAL);
      mSwe.setTruePos(mTruePos);
      mSwe.calcHouses();
      
      double angle = mSwe.getObjData(obj).longitude;
      if(mZodiac == ZODIAC_DRACONIC) // set aries 0-degrees to true node
        { angle = fmod(angle - mSwe.getObjData(OBJ_NORTHNODE).longitude + 360.0, 360.0); }
      
      return angle;
    }
}


void Chart::setAspectOrb(AspectType asp, double orb)
{
  if(asp > ASPECT_INVALID && asp < ASPECT_COUNT)
    {
      mAspectOrbs[(int)asp] = orb;
      mNeedUpdate = true;
    }
}
void Chart::setAspectFocus(AspectType asp, bool focus)
{
  if(asp > ASPECT_INVALID && asp < ASPECT_COUNT)
    {
      mAspectFocus[(int)asp] = focus;
      mNeedUpdate = true;
    }
}
void Chart::setAspectVisible(AspectType asp, bool visible)
{
  if(asp > ASPECT_INVALID && asp < ASPECT_COUNT)
    {
      mAspectVisible[(int)asp] = visible;
      mNeedUpdate = true;
    }
}
double Chart::getAspectOrb(AspectType asp)
{ return ((asp > ASPECT_INVALID && asp < ASPECT_COUNT) ? mAspectOrbs[(int)asp]   : -1.0); }
bool Chart::getAspectFocus(AspectType asp)
{ return (asp > ASPECT_INVALID && asp < ASPECT_COUNT) ? mAspectFocus[(int)asp]   : false; }
bool Chart::getAspectVisible(AspectType asp)
{ return (asp > ASPECT_INVALID && asp < ASPECT_COUNT) ? mAspectVisible[(int)asp] : false; }
