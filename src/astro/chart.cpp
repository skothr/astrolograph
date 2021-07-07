#include "chart.hpp"
using namespace astro;

#include <array>
#include <string>
#include <cctype>
#include <fstream>

//// CHART ////
Chart::Chart(const DateTime &dt, const Location &loc)
  : mDate(dt), mLocation(loc), mParams(new ChartParams())
{
  for(int o = 0; o < OBJ_END; o++)
    { mObjectData.push_back(new ObjData{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, o, false}); }
  for(int o = 0; o < OBJ_END; o++)
    { mObjects.push_back(new ChartObject{mObjectData[o], o, 0.0, 0.0, false, false, false}); }
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

void Chart::outputToFile(const std::string &path, const std::string &name, bool append, bool validAngles)
{
  std::ofstream f;
  if(append)
    { f.open(path, std::ofstream::out | std::ofstream::app); }
  else
    { f.open(path, std::ofstream::out); }

  if(!fileExists(path) || !append)
    { // output header
      std::string header = "name,year,month,day,time,utc,dst,latitude,longitude,";
      for(int i = ANGLE_OFFSET; i < OBJ_END; i++) // ANGLES
        { header += getObjName(i) + ","; }
      for(int i = 1; i <= 12; i++)                // HOUSES
        {
          header += "house " + std::to_string(i) + ",";
          header += "house " + std::to_string(i) + " ruler,";
        }
      for(int i = 0; i < ANGLE_OFFSET; i++)      // OBJECTS
        {
          if(i != OBJ_QUAOAR)
            {
              header += getObjName(i) + ",";
              header += getObjName(i) + " Rx,";
              header += getObjName(i) + " house,";
            }
        }
      f << header << "\n";
    }

  double utc = mLocation.utcOffset;
  double dst = mDate.dstOffset();
  
  f << name << ","
    << mDate.year() << "," << mDate.month() << "," << mDate.day() << "," << mDate.toShortString(false, true, false)
    << "," << (utc < 0 ? "" : "+") << (int)utc << "," << (dst == 0.0 ? "0" : "1") << ","
    << mLocation.latitude << "," << mLocation.longitude << ",";

  for(int i = ANGLE_OFFSET; i < OBJ_END; i++)  // ANGLES
    { f << (validAngles ? getObjString(getObjectData(i)->longitude) : "") << ","; }
  
  for(int i = 1; i <= 12; i++)                 // HOUSES
    {
      f << (validAngles ? getObjString(getHouseCusp(i)) : "") << ",";
      std::vector<std::string> rulers = getSignRulers(getSign(getHouseCusp(i)));
      std::string rulerStr = ""; for(int j = 0; j < rulers.size()-1; j++) { rulerStr += rulers[j] + " / "; } rulerStr += rulers.back();
      f << rulerStr << ",";
    }
  
  for(int i = 0; i < ANGLE_OFFSET; i++)        // OBJECTS
    {
      if(i != OBJ_QUAOAR)
        {
          double angle = getObjectData(i)->longitude;
          int    house = getHouse(angle);
          f << getObjString(angle) << "," << (getObjectData(i)->lonSpeed <= 0.0 ? 1 : 0) << "," << (validAngles ? std::to_string(house) : "") << ",";
        }
    }
  f << "\n";
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
      std::string name1 = getObjName(o1);

      for(int o2 = o1+1; o2 < OBJ_END; o2++)
        {
          if(!all && !params.objVisible[o2]) { continue; } // skip if switched off
          double angle2 = mObjects[o2]->angle;
          std::string name2 = getObjName(o2);
          double diff = angleDiffDegrees(angle1, angle2);
          for(auto &iter : ASPECTS)
            {
              if(!all && !params.aspVisible[iter.second.type]) { continue; } // skip if switched off
              double aDiff = angleDiffDegrees(diff, iter.second.angle);
              double orb = std::min(params.orbs.objOrbs[o1][iter.second.type], params.orbs.objOrbs[o2][iter.second.type]);
              if(std::abs(aDiff) <= orb)
                {
                  double strength = 1.0 - (std::abs(aDiff) / orb);
                  aspects.emplace_back(mObjects[o1], mObjects[o2], iter.second.type, aDiff, strength,
                                       true, params.aspVisible[iter.second.type], params.aspFocused[iter.second.type]); // valid, visible, focused
                }
            }
        }
    }
  // // sort aspects by orb (reverse?)
  // std::sort(aspects.begin(), aspects.end(),
  //           [](const ChartAspect &a, const ChartAspect &b) -> bool
  //           {
  //             if(std::abs(a.orb - b.orb) < 0.001)
  //               { // differentiate by aspect type, then object types
  //                 if(a.type < b.type)      { return true; }
  //                 else if(a.obj1 < b.obj1) { return true; }
  //                 else if(a.obj2 < b.obj2) { return true; }
  //                 else { return false; }
  //               }
  //             else // return smaller orb
  //               { return (a.orb < b.orb); }
  //           } ); // sort by orb (ascending)
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
      std::string name1 = getObjName(o1);

      // object aspects
      for(int o2 = o1+1; o2 < OBJ_END; o2++)
        {
          if(!all && !params.objVisible[o2]) { continue; } // skip if switched off
          int i2 = o2;
          double angle2 = objects()[o2]->angle;
          std::string name2 = getObjName(o2);
          double diff = astro::angleDiffDegrees(angle1, angle2);
          
          for(auto &iter : astro::ASPECTS)
            {
              if(!all && !params.aspVisible[iter.second.type]) { continue; } // skip if switched off
              double aDiff = astro::angleDiffDegrees(diff, iter.second.angle);
              double orb = std::min(params.orbs.objOrbs[o1][iter.second.type], params.orbs.objOrbs[o2][iter.second.type]);
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
          ObjType o = i;//(OBJ_SUN + i);
          //if(o >= OBJ_COUNT) { o = (o-OBJ_COUNT+ANGLE_OFFSET); } // correct for angles
          ChartObject *obj = mObjects[i];
          *obj->data = mSwe.getObjData(o);
          obj->valid = obj->data->valid;
          obj->angle = obj->data->longitude;
          obj->speed = obj->data->lonSpeed;
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

double Chart::getSingleAngle(ObjType o)
{
  if(!mNeedUpdate)
    { return getObject(o)->angle; }
  else
    {
      // update chart info (via Swiss Ephemeris wrapper)
      mLocation.fix();
      mDate.fix();
      mSwe.setLocation(mLocation);
      mSwe.setDate(mDate);
      mSwe.setSidereal(mZodiac == ZODIAC_SIDEREAL);
      mSwe.setTruePos(mTruePos);
      
      if(o >= ANGLE_OFFSET) { mSwe.calcHouses(mHouseSystem); }
      
      double angle = mSwe.getObjData(o).longitude;
      if(mZodiac == ZODIAC_DRACONIC) // set aries 0-degrees to true node
        { angle = fmod(angle - mSwe.getObjData(OBJ_NORTHNODE).longitude + 360.0, 360.0); }
      
      return angle;
    }
}

ChartObject* Chart::getSingleObject(ObjType o)
{
  if(!mNeedUpdate)
    { return getObject(o); }
  else
    {
      // update chart info (via Swiss Ephemeris wrapper)
      mLocation.fix();
      mDate.fix();
      mSwe.setLocation(mLocation);
      mSwe.setDate(mDate);
      mSwe.setSidereal(mZodiac == ZODIAC_SIDEREAL);
      mSwe.setTruePos(mTruePos);
      
      if(o >= ANGLE_OFFSET) { mSwe.calcHouses(mHouseSystem); }
      
      ChartObject *obj = mObjects[o];
      *obj->data = mSwe.getObjData(o);
      obj->valid = obj->data->valid;
      obj->angle = obj->data->longitude;
      obj->speed = obj->data->lonSpeed;
      obj->retrograde = (obj->data->lonSpeed < 0.0);
      
      if(mZodiac == ZODIAC_DRACONIC)
        { // set aries 0-degrees to true node
          ChartObject *nnObj = obj;
          if(o != OBJ_NORTHNODE) { nnObj = getSingleObject(OBJ_NORTHNODE); }
          obj->angle = fmod(obj->angle - nnObj->angle + 360.0, 360.0);
        }
      
      return obj;
    }
}

ChartAspect Chart::getAspect(ObjType o1, ObjType o2, const ChartParams &params)
{
  int i1 = o1;
  int i2 = o2;
  
  double angle1 = mObjects[o1]->angle;
  double angle2 = mObjects[o2]->angle;
  double diff = angleDiffDegrees(angle1, angle2);
  for(auto &iter : ASPECTS)
    {
      double aDiff = angleDiffDegrees(diff, iter.second.angle);
      double orb = std::min(params.orbs.objOrbs[o1][iter.second.type], params.orbs.objOrbs[o2][iter.second.type]);
      if(std::abs(aDiff) <= orb)
        {
          // aspects sorted from strongest to weakest
          double strength = 1.0 - (std::abs(aDiff) / orb);
          return ChartAspect(mObjects[i1], mObjects[i2], iter.second.type, aDiff, strength,
                             true, params.aspVisible[iter.second.type], params.aspFocused[iter.second.type]); // valid, visible, focused
        }
    }
  return ChartAspect();
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

std::string utf8_substr(const std::string& str, unsigned int start, unsigned int leng)
{
    if (leng==0) { return ""; }
    unsigned long c, i, ix, q, min=std::string::npos, max=std::string::npos;
    for (q=0, i=0, ix=str.length(); i < ix; i++, q++)
    {
        if (q==start){ min=i; }
        if (q<=start+leng || leng==std::string::npos){ max=i; }

        c = (unsigned char) str[i];
        if      (
                 //c>=0   &&
                 c<=127) i+=0;
        else if ((c & 0xE0) == 0xC0) i+=1;
        else if ((c & 0xF0) == 0xE0) i+=2;
        else if ((c & 0xF8) == 0xF0) i+=3;
        //else if (($c & 0xFC) == 0xF8) i+=4; // 111110bb //byte 5, unnecessary in 4 byte UTF-8
        //else if (($c & 0xFE) == 0xFC) i+=5; // 1111110b //byte 6, unnecessary in 4 byte UTF-8
        else return "";//invalid utf8
    }
    if (q<=start+leng || leng==std::string::npos){ max=i; }
    if (min==std::string::npos || max==std::string::npos) { return ""; }
    return str.substr(min,max-min);
}

std::string Chart::getSignChar(double longitude) const
{
  int s = getSign(longitude);
  return utf8_substr(std::string(SIGNS_EXTRA_CHARS), s, 1);
}

std::string Chart::getObjString(double longitude) const
{
  return getSignChar(longitude) + " " + angle_string(longitude-getSign(longitude)*30.0, true, false, true, 2);
}

#define MAX_RX_DEPTH 50
bool Chart::findRxStations(const DateTime &start, const DateTime &end, ObjType o, std::vector<ObjRx> &stations,
                           double maxError, double minTimeRange, int *stepsTaken, int level)
{
  if(level == 0) { mSwe.setLocation(mLocation); mSwe.setSidereal(mZodiac == ZODIAC_SIDEREAL); mSwe.setTruePos(mTruePos); if(stepsTaken) { *stepsTaken = 0; } }
  
  if(stepsTaken) { *stepsTaken++; }
  
  DateTime d0 = start.fixed();
  DateTime d1 = end.fixed();

  // check base cases
  mSwe.setDate(d0);
  ObjData obj = mSwe.getObjData(o);
  if(std::abs(obj.lonSpeed) <= maxError)
    { // object at station
      mSwe.setDate(d1);
      ObjData obj1 = mSwe.getObjData(o);
      bool rx = ((obj.lonSpeed > 0.0 && obj1.lonSpeed < 0.0) ||
                 (obj.lonSpeed > 0.0 && std::abs(obj.lonSpeed) > std::abs(obj1.lonSpeed)));

      if(mDebug) { std::cout << " ==> FOUND " << (rx ? "RX" : "DX") << " STATION: " << d0 << "\n"; }

      bool found = false;
      for(auto &s : stations)
        {
          if(rx)
            {
              if(!s.rxValid) { s.rxValid = true; s.rxStation = d0; found = true; break; }
            }
          else
            {
              if(!s.dxValid) { s.dxValid = true; s.dxStation = d0; found = true; break; }
            }
        }
      if(!found)
        {
          ObjRx orx;
          if(rx) { orx.rxStation = d0; }
          else   { orx.dxStation = d0; }
          stations.push_back(orx);
        }
      return true;
    }
  
  double rangeDiff = d1.diffDays(d0);
  //std::cout << " ==> RANGE DIFF: " << rangeDiff << "\n";
  if(level > MAX_RX_DEPTH || rangeDiff < minTimeRange) { return false; } // no point in recursing further

  double half = rangeDiff/2.0;
  DateTime dMid = d0;
  dMid.setDay(dMid.day() + (int)std::floor(half)); dMid.fix();
  half = (half - std::floor(half))*24.0;
  dMid.setHour(dMid.hour() + (int)std::floor(half)); dMid.fix();
  half = (half - std::floor(half))*60.0;
  dMid.setMinute(dMid.minute() + (int)std::floor(half)); dMid.fix();
  half = (half - std::floor(half))*60.0;
  dMid.setSecond(dMid.second() + half); dMid.fix();

  bool found = false;
  if(findRxStations(start, dMid, o, stations, maxError, minTimeRange, stepsTaken, level+1))
    { found = true; }
  if(!found || rangeDiff > 30.0)
    {
      if(findRxStations(dMid,  end,  o, stations, maxError, minTimeRange, stepsTaken, level+1))
        { found = true; }
    }
  return found;
}

// TODO //
std::vector<DateTime> Chart::findAspects(const DateTime &start, const DateTime &end, ObjType o1, ObjType o2, AspectType a,
                                         double maxAngle, double *error, int *stepsTaken)
{
  int printPrecision = 8;
  std::cout << std::fixed << std::setprecision(printPrecision);

  DateTime d0    = start;  d0.fix();
  DateTime d1    = end;    d1.fix();
  double   jdUT  = mSwe.getJulianDayUT(d0, mLocation);
  double   jdUT0 = jdUT;
  double   jdUT1 = mSwe.getJulianDayUT(d1, mLocation);
  mLocation.fix();
  mSwe.setLocation(mLocation);
  mSwe.setDate(d0);
  mSwe.setSidereal(mZodiac == ZODIAC_SIDEREAL);
  mSwe.setTruePos(mTruePos);

  ObjData obj1 = mSwe.getObjData(o1);
  ObjData obj2 = mSwe.getObjData(o2);
  std::string o1Name = getObjName(o1);
  std::string o2Name = getObjName(o2);
  OrbitInfo o1Orbit = mSwe.getOrbitInfo(o1);
  OrbitInfo o2Orbit = mSwe.getOrbitInfo(o2);

  double diff  = angleDiffDegrees(obj1.longitude, obj2.longitude);
  int steps = 0;

  double minErr      = 360.0;
  double direction   = 1.0; // forward or backward in time
  double damping     = 1.0;
  double lastNumDays = 0.0;
  double diffStepped = diff;
  if(mDebug) { std::cout << "  OBJ1 = " << o1Name << "\n  OBJ2 = " << o2Name << "\n  ASP=" << getAspectName(a) << "\n"; }

  std::vector<DateTime> aspectPeaks;
  double aspAngle = getAspectInfo(a)->angle;
  while((std::abs(diff - aspAngle) > maxAngle) && (steps < MAX_STEPS))
    {
      double speed1 = (o1Orbit.meanDailyMotion); // degrees/day
      double speed2 = (o2Orbit.meanDailyMotion); // degrees/day

      double o1Stepped = fmod(obj1.longitude + (speed1 > 0 ? 0.1 : -0.1) + 360.0, 360.0);
      double o2Stepped = fmod(obj2.longitude + (speed2 > 0 ? 0.1 : -0.1) + 360.0, 360.0);

      double dampMult = 1.0;
      double err = std::abs(diff-aspAngle);
      double errStepped = std::abs(diffStepped-aspAngle);
      minErr = std::min(minErr, err);
      if(err > minErr && errStepped > err)//(speed1 < 0) != (speed2 < 0)) // switch direction if aspect was passed
        { direction *= -1.0; dampMult = 0.7; }

      if(jdUT < jdUT0) { direction = 1.0; } // don't go back in time
      
      double numDays = damping*dampMult*direction*err/(std::max(std::abs(speed1), std::abs(speed2)));
      // if((speed1 < 0) != (speed2 < 0)) { numDays /= 2.0; }
      lastNumDays = numDays;
      diffStepped = angleDiffDegrees(o1Stepped, o2Stepped);
      
      if(mDebug)
        {
          std::cout << mSwe.getDateFromJUT(jdUT, mLocation)
                    << " -- DIFF=" << std::left << std::setw(4+printPrecision) << diff
                    << "  O1A=" << std::left << std::setw(4+printPrecision) << obj1.longitude
                    << "  O1S="  << std::left << std::setw(4+printPrecision) << speed1
                    << "  O2A=" << std::left << std::setw(4+printPrecision) << obj2.longitude
                    << "  O2S="  << std::left << std::setw(4+printPrecision) << speed2
                    << "  |  ERROR=" << std::left << std::setw(4+printPrecision) << err
                    << "   -->   NDAYS=" << std::left << std::setw(6+printPrecision) << numDays << "   -->   ";
        }
      
      jdUT += numDays;
      mSwe.setDateUT(jdUT);
      if(mDebug) { std::cout << mSwe.getDateFromJUT(jdUT, mLocation) << "\n"; }
      
      if(o1 >= ANGLE_OFFSET || o2 >= ANGLE_OFFSET) { mSwe.calcHouses(mHouseSystem); }
      obj1 = mSwe.getObjData(o1); obj2 = mSwe.getObjData(o2);
      diff = angleDiffDegrees(obj1.longitude, obj2.longitude);
      o1Orbit = mSwe.getOrbitInfo(o1);
      o2Orbit = mSwe.getOrbitInfo(o2);

      steps++;
    }
  if(stepsTaken) { *stepsTaken = steps; }
  if(error) { *error = minErr; }
  
  d0 = mSwe.getDateFromJUT(jdUT, mLocation);
  
  
  std::cout << std::setprecision(6);
  return aspectPeaks;
}



// OLD (incremental/slow)
// DateTime Chart::findRxStationOnDay(const DateTime &dt, ObjType o, double maxError, double *error, int *stepsTaken, bool *success)
// {
//   if(success) { *success = false; }
  
//   DateTime d0 = dt; d0.fix();
//   d0.setHour(0); d0.setMinute(0); d0.setSecond(0.0);

//   mSwe.setDate(d0);
//   ObjData obj = mSwe.getObjData(o);
//   if(obj.lonSpeed == 0.0) { if(success) { *success = true; } return d0; } // station at exactly 00:00 on date
  
//   DateTime d = d0;
//   double direction = (obj.lonSpeed < 0.0 ? -1.0 : 1.0);
//   int hms = 0; // 0=hour, 1=minute, 1=second
  
//   while(d.day() == d0.day() && std::abs(obj.lonSpeed) > maxError)
//     {
//       DateTime last = d;
//       // step dt
//       if(stepsTaken)    { *stepsTaken++; }
//       if(hms == 0)      { d.setHour(d.hour()+1);       }
//       else if(hms == 1) { d.setMinute(d.minute()+1);   }
//       else if(hms == 2) { d.setSecond(d.second()+1.0); }
//       else              { d.setSecond(d.second() + 1.0/pow(10, hms-2)); }
//       d.fix();
      
//       if(mDebug) { std::cout << "STEP (" << last << " --> " << d << ")\n"; }

//       mSwe.setDate(d);
//       obj = mSwe.getObjData(o);
//       if((obj.lonSpeed < 0.0) != (direction < 0.0))
//         {
//           if(hms == 0)      { d.setHour(d.hour()-1);       }
//           else if(hms == 1) { d.setMinute(d.minute()-1);   }
//           else if(hms == 2) { d.setSecond(d.second()-1.0); }
//           else              { d.setSecond(d.second() - 1.0/pow(10, hms-2)); }
//           hms++;
//           direction *= -1.0;
//         }
//     }

//   std::cout << "  ==>  D0: " << d0  << "  |  D: " << d << "\n";
//   if(success) { *success = ((obj.lonSpeed < 0.0) == (direction < 0.0)); }
//   if(error)   { *error   = std::abs(obj.lonSpeed); }
//   return d;
// }



DateTime Chart::findAspectPeak(const DateTime &start, const DateTime &end, ObjType o1, ObjType o2, AspectType a, double maxAngle, double *error, int *stepsTaken)
{
  int printPrecision = 8;
  std::cout << std::fixed << std::setprecision(printPrecision);

  DateTime d0 = start;  d0.fix();
  DateTime d1 = end;    d1.fix();
  mLocation.fix();
  mSwe.setLocation(mLocation);
  mSwe.setDate(d0);
  mSwe.setSidereal(mZodiac == ZODIAC_SIDEREAL);
  mSwe.setTruePos(mTruePos);

  ObjData obj1 = mSwe.getObjData(o1);
  ObjData obj2 = mSwe.getObjData(o2);
  std::string o1Name = getObjName(o1);
  std::string o2Name = getObjName(o2);
  OrbitInfo o1Orbit = mSwe.getOrbitInfo(o1);
  OrbitInfo o2Orbit = mSwe.getOrbitInfo(o2);

  if(mDebug)
    {
      std::cout << "OBJECT 1 ORBIT INFO:\n"
                << "  semimajorAxis    = " << o1Orbit.semimajorAxis    << "\n"
                << "  eccentricity     = " << o1Orbit.eccentricity     << "\n"
                << "  inclination      = " << o1Orbit.inclination      << "\n"
                << "  ascendingNode    = " << o1Orbit.ascendingNode    << "\n"
                << "  periapsisArg     = " << o1Orbit.periapsisArg     << "\n"
                << "  periapsisLon     = " << o1Orbit.periapsisLon     << "\n"
                << "  meanEpochAnomaly = " << o1Orbit.meanEpochAnomaly << "\n"
                << "  trueEpochAnomaly = " << o1Orbit.trueEpochAnomaly << "\n"
                << "  eccEpochAnomaly  = " << o1Orbit.eccEpochAnomaly  << "\n"
                << "  meanEpochLon     = " << o1Orbit.meanEpochLon     << "\n"
                << "  orbitalPeriod    = " << o1Orbit.orbitalPeriod    << "\n"
                << "  meanDailyMotion  = " << o1Orbit.meanDailyMotion  << "\n"
                << "  tropicalPeriod   = " << o1Orbit.tropicalPeriod   << "\n"
                << "  synodicPeriod    = " << o1Orbit.synodicPeriod    << "\n"
                << "  perihelionTime   = " << o1Orbit.perihelionTime   << "\n"
                << "  perihelionDist   = " << o1Orbit.perihelionDist   << "\n"
                << "  aphelionDist     = " << o1Orbit.aphelionDist     << "\n\n";

      std::cout << "OBJECT 2 ORBIT INFO:\n"
                << "  semimajorAxis    = " << o2Orbit.semimajorAxis    << "\n"
                << "  eccentricity     = " << o2Orbit.eccentricity     << "\n"
                << "  inclination      = " << o2Orbit.inclination      << "\n"
                << "  ascendingNode    = " << o2Orbit.ascendingNode    << "\n"
                << "  periapsisArg     = " << o2Orbit.periapsisArg     << "\n"
                << "  periapsisLon     = " << o2Orbit.periapsisLon     << "\n"
                << "  meanEpochAnomaly = " << o2Orbit.meanEpochAnomaly << "\n"
                << "  trueEpochAnomaly = " << o2Orbit.trueEpochAnomaly << "\n"
                << "  eccEpochAnomaly  = " << o2Orbit.eccEpochAnomaly  << "\n"
                << "  meanEpochLon     = " << o2Orbit.meanEpochLon     << "\n"
                << "  orbitalPeriod    = " << o2Orbit.orbitalPeriod    << "\n"
                << "  meanDailyMotion  = " << o2Orbit.meanDailyMotion  << "\n"
                << "  tropicalPeriod   = " << o2Orbit.tropicalPeriod   << "\n"
                << "  synodicPeriod    = " << o2Orbit.synodicPeriod    << "\n"
                << "  perihelionTime   = " << o2Orbit.perihelionTime   << "\n"
                << "  perihelionDist   = " << o2Orbit.perihelionDist   << "\n"
                << "  aphelionDist     = " << o2Orbit.aphelionDist     << "\n\n";
    }

  double diff  = angleDiffDegrees(obj1.longitude, obj2.longitude);

  double jdUT  = mSwe.getJulianDayUT(d0, mLocation);
  double jdUT0 = jdUT;
  int steps = 0;

  double minErr      = 360.0;
  double direction   = 1.0; // forward or backward in time
  double damping     = 1.0;
  double lastNumDays = 0.0;
  double diffStepped = diff;
  if(mDebug) { std::cout << "  OBJ1 = " << o1Name << "\n  OBJ2 = " << o2Name << "\n  ASP=" << getAspectName(a) << "\n"; }

  double aspAngle = getAspectInfo(a)->angle;
  while((std::abs(diff - aspAngle) > maxAngle) && (steps < MAX_STEPS))
    {
      double speed1 = (o1Orbit.meanDailyMotion); // degrees/day
      double speed2 = (o2Orbit.meanDailyMotion); // degrees/day

      double o1Stepped = fmod(obj1.longitude + (speed1 > 0 ? 0.1 : -0.1) + 360.0, 360.0);
      double o2Stepped = fmod(obj2.longitude + (speed2 > 0 ? 0.1 : -0.1) + 360.0, 360.0);

      double dampMult = 1.0;
      double err = std::abs(diff-aspAngle);
      double errStepped = std::abs(diffStepped-aspAngle);
      minErr = std::min(minErr, err);
      if(err > minErr && errStepped > err)//(speed1 < 0) != (speed2 < 0)) // switch direction if aspect was passed
        { direction *= -1.0; dampMult = 0.7; }

      if(jdUT < jdUT0) { direction = 1.0; } // don't go back in time
      
      double numDays = damping*dampMult*direction*err/(std::max(std::abs(speed1), std::abs(speed2)));
      // if((speed1 < 0) != (speed2 < 0)) { numDays /= 2.0; }
      lastNumDays = numDays;
      diffStepped = angleDiffDegrees(o1Stepped, o2Stepped);
      
      if(mDebug)
        {
          std::cout << mSwe.getDateFromJUT(jdUT, mLocation)
                    << " -- DIFF=" << std::left << std::setw(4+printPrecision) << diff
                    << "  O1A=" << std::left << std::setw(4+printPrecision) << obj1.longitude
                    << "  O1S="  << std::left << std::setw(4+printPrecision) << speed1
                    << "  O2A=" << std::left << std::setw(4+printPrecision) << obj2.longitude
                    << "  O2S="  << std::left << std::setw(4+printPrecision) << speed2
                    << "  |  ERROR=" << std::left << std::setw(4+printPrecision) << err
                    << "   -->   NDAYS=" << std::left << std::setw(6+printPrecision) << numDays << "   -->   ";
        }
      
      jdUT += numDays;
      mSwe.setDateUT(jdUT);
      if(mDebug) { std::cout << mSwe.getDateFromJUT(jdUT, mLocation) << "\n"; }
      
      if(o1 >= ANGLE_OFFSET || o2 >= ANGLE_OFFSET) { mSwe.calcHouses(mHouseSystem); }
      obj1 = mSwe.getObjData(o1); obj2 = mSwe.getObjData(o2);
      diff = angleDiffDegrees(obj1.longitude, obj2.longitude);
      o1Orbit = mSwe.getOrbitInfo(o1);
      o2Orbit = mSwe.getOrbitInfo(o2);

      steps++;
    }
  if(stepsTaken) { *stepsTaken = steps; }
  if(error) { *error = minErr; }
  
  d0 = mSwe.getDateFromJUT(jdUT, mLocation);
  
  
  std::cout << std::setprecision(6);
  return d0;
}






// DateTime Chart::findAspectPeak(const DateTime &start, const DateTime &end, ObjType o1, ObjType o2, AspectType a, double maxAngle, double *error, int *stepsTaken)
// {
//   int printPrecision = 8;
//   std::cout << std::fixed << std::setprecision(printPrecision);

//   DateTime d0 = start;  d0.fix();
//   DateTime d1 = end;    d1.fix();
//   mLocation.fix();
//   mSwe.setLocation(mLocation);
//   mSwe.setDate(d0);
//   mSwe.setSidereal(mZodiac == ZODIAC_SIDEREAL);
//   mSwe.setTruePos(mTruePos);

//   ObjData obj1 = mSwe.getObjData(o1);
//   ObjData obj2 = mSwe.getObjData(o2);
//   std::string o1Name = getObjName(o1);
//   std::string o2Name = getObjName(o2);
//   OrbitInfo o1Orbit = mSwe.getOrbitInfo(o1);
//   OrbitInfo o2Orbit = mSwe.getOrbitInfo(o2);

//   if(mDebug)
//     {
//       std::cout << "OBJECT 1 ORBIT INFO:\n"
//                 << "  semimajorAxis    = " << o1Orbit.semimajorAxis    << "\n"
//                 << "  eccentricity     = " << o1Orbit.eccentricity     << "\n"
//                 << "  inclination      = " << o1Orbit.inclination      << "\n"
//                 << "  ascendingNode    = " << o1Orbit.ascendingNode    << "\n"
//                 << "  periapsisArg     = " << o1Orbit.periapsisArg     << "\n"
//                 << "  periapsisLon     = " << o1Orbit.periapsisLon     << "\n"
//                 << "  meanEpochAnomaly = " << o1Orbit.meanEpochAnomaly << "\n"
//                 << "  trueEpochAnomaly = " << o1Orbit.trueEpochAnomaly << "\n"
//                 << "  eccEpochAnomaly  = " << o1Orbit.eccEpochAnomaly  << "\n"
//                 << "  meanEpochLon     = " << o1Orbit.meanEpochLon     << "\n"
//                 << "  orbitalPeriod    = " << o1Orbit.orbitalPeriod    << "\n"
//                 << "  meanDailyMotion  = " << o1Orbit.meanDailyMotion  << "\n"
//                 << "  tropicalPeriod   = " << o1Orbit.tropicalPeriod   << "\n"
//                 << "  synodicPeriod    = " << o1Orbit.synodicPeriod    << "\n"
//                 << "  perihelionTime   = " << o1Orbit.perihelionTime   << "\n"
//                 << "  perihelionDist   = " << o1Orbit.perihelionDist   << "\n"
//                 << "  aphelionDist     = " << o1Orbit.aphelionDist     << "\n\n";

//       std::cout << "OBJECT 2 ORBIT INFO:\n"
//                 << "  semimajorAxis    = " << o2Orbit.semimajorAxis    << "\n"
//                 << "  eccentricity     = " << o2Orbit.eccentricity     << "\n"
//                 << "  inclination      = " << o2Orbit.inclination      << "\n"
//                 << "  ascendingNode    = " << o2Orbit.ascendingNode    << "\n"
//                 << "  periapsisArg     = " << o2Orbit.periapsisArg     << "\n"
//                 << "  periapsisLon     = " << o2Orbit.periapsisLon     << "\n"
//                 << "  meanEpochAnomaly = " << o2Orbit.meanEpochAnomaly << "\n"
//                 << "  trueEpochAnomaly = " << o2Orbit.trueEpochAnomaly << "\n"
//                 << "  eccEpochAnomaly  = " << o2Orbit.eccEpochAnomaly  << "\n"
//                 << "  meanEpochLon     = " << o2Orbit.meanEpochLon     << "\n"
//                 << "  orbitalPeriod    = " << o2Orbit.orbitalPeriod    << "\n"
//                 << "  meanDailyMotion  = " << o2Orbit.meanDailyMotion  << "\n"
//                 << "  tropicalPeriod   = " << o2Orbit.tropicalPeriod   << "\n"
//                 << "  synodicPeriod    = " << o2Orbit.synodicPeriod    << "\n"
//                 << "  perihelionTime   = " << o2Orbit.perihelionTime   << "\n"
//                 << "  perihelionDist   = " << o2Orbit.perihelionDist   << "\n"
//                 << "  aphelionDist     = " << o2Orbit.aphelionDist     << "\n\n";
//     }

//   double diff  = angleDiffDegrees(obj1.longitude, obj2.longitude);

//   double jdUT  = mSwe.getJulianDayUT(d0, mLocation);
//   double jdUT0 = jdUT;
//   int steps = 0;

//   double minErr      = 360.0;
//   double direction   = 1.0; // forward or backward in time
//   double damping     = 1.0;
//   double lastNumDays = 0.0;
//   double diffStepped = diff;
//   if(mDebug) { std::cout << "  OBJ1 = " << o1Name << "\n  OBJ2 = " << o2Name << "\n  ASP=" << getAspectName(a) << "\n"; }
  
//   double aspAngle = getAspectInfo(a)->angle;
//   while((std::abs(diff - aspAngle) > maxAngle) && (steps < MAX_STEPS))
//     {
//       double speed1 = (o1Orbit.meanDailyMotion); // degrees/day
//       double speed2 = (o2Orbit.meanDailyMotion); // degrees/day

//       double o1Stepped = fmod(obj1.longitude + (speed1 > 0 ? 0.1 : -0.1) + 360.0, 360.0);
//       double o2Stepped = fmod(obj2.longitude + (speed2 > 0 ? 0.1 : -0.1) + 360.0, 360.0);

//       double dampMult = 1.0;
//       double err = std::abs(diff-aspAngle);
//       double errStepped = std::abs(diffStepped-aspAngle);
//       minErr = std::min(minErr, err);
//       if(err > minErr && errStepped > err)//(speed1 < 0) != (speed2 < 0)) // switch direction if aspect was passed
//         { direction *= -1.0; dampMult = 0.7; }

//       if(jdUT < jdUT0) { direction = 1.0; } // don't go back in time
      
//       double numDays = damping*dampMult*direction*err/(std::max(std::abs(speed1), std::abs(speed2)));
//       // if((speed1 < 0) != (speed2 < 0)) { numDays /= 2.0; }
//       lastNumDays = numDays;
//       diffStepped = angleDiffDegrees(o1Stepped, o2Stepped);
      
//       if(mDebug)
//         {
//           std::cout << mSwe.getDateFromJUT(jdUT, mLocation)
//                     << " -- DIFF=" << std::left << std::setw(4+printPrecision) << diff
//                     << "  O1A=" << std::left << std::setw(4+printPrecision) << obj1.longitude
//                     << "  O1S="  << std::left << std::setw(4+printPrecision) << speed1
//                     << "  O2A=" << std::left << std::setw(4+printPrecision) << obj2.longitude
//                     << "  O2S="  << std::left << std::setw(4+printPrecision) << speed2
//                     << "  |  ERROR=" << std::left << std::setw(4+printPrecision) << err
//                     << "   -->   NDAYS=" << std::left << std::setw(6+printPrecision) << numDays << "   -->   ";
//         }
      
//       jdUT += numDays;
//       mSwe.setDateUT(jdUT);
//       if(mDebug) { std::cout << mSwe.getDateFromJUT(jdUT, mLocation) << "\n"; }
      
//       if(o1 >= ANGLE_OFFSET || o2 >= ANGLE_OFFSET) { mSwe.calcHouses(mHouseSystem); }
//       obj1 = mSwe.getObjData(o1); obj2 = mSwe.getObjData(o2);
//       diff = angleDiffDegrees(obj1.longitude, obj2.longitude);
//       o1Orbit = mSwe.getOrbitInfo(o1);
//       o2Orbit = mSwe.getOrbitInfo(o2);

//       steps++;
//     }
//   if(stepsTaken) { *stepsTaken = steps; }
//   if(error) { *error = minErr; }
  
//   d0 = mSwe.getDateFromJUT(jdUT, mLocation);
//   std::cout << std::setprecision(6);
//   return d0;
// }
