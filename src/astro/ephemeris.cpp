#include "ephemeris.hpp"
using namespace astro;

#include <cmath>
#include <iostream>
#include <iomanip>
#include <fstream>


const std::vector<int> Ephemeris::SWE_IDS = { SE_SUN, SE_MOON, SE_MERCURY, SE_VENUS, SE_MARS,
                                              SE_JUPITER, SE_SATURN, SE_URANUS, SE_NEPTUNE, SE_PLUTO,
                                              (SE_AST_OFFSET+50000), // (Quaoar)
                                              (SE_AST_OFFSET+120),   // (Lachesis)
                                              
                                              SE_CHIRON, SE_PHOLUS, SE_CERES, SE_JUNO, SE_PALLAS, SE_VESTA,
                                              SE_MEAN_APOG,             // (Lilith)
                                              (SE_AST_OFFSET + 19),     // (Fortuna),
                                              (SE_AST_OFFSET + 136199), // (Eris)
                                              SE_TRUE_NODE, SE_TRUE_NODE }; // (south node calculated from north node)

//// INSIDE DEGREE TEXT ////
std::array<std::array<std::string, 30>, 12> Ephemeris::insideDegreesShort; // ACCESS: arr[SIGN_INDEX][floor(DEGREE)]
std::array<std::array<std::string, 30>, 12> Ephemeris::insideDegreesLong;  // ACCESS: arr[SIGN_INDEX][floor(DEGREE)]
bool Ephemeris::mInsideDegreesLoaded = false;
bool Ephemeris::loadInsideDegrees()
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

          std::transform(sign.begin(), sign.end(), sign.begin(), [](unsigned char c){ return std::tolower(c); }); // lowercase sign
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

std::string Ephemeris::getInsideDegreeTextShort(int sign, int degree)
{
  if(sign < 0 || sign >= 12 || degree < 0 || degree >= 30) { return ""; }
  else                                                     { return insideDegreesShort[sign][degree]; }
}

std::string Ephemeris::getInsideDegreeTextLong(int sign, int degree)
{
  if(sign < 0 || sign >= 12 || degree < 0 || degree >= 30) { return ""; }
  else                                                     { return insideDegreesLong[sign][degree]; }
}



Ephemeris::Ephemeris()
{
  char ephemPath[512] = EPHEM_PATH;
  swe_set_ephe_path(ephemPath);
  loadInsideDegrees();
}

void Ephemeris::setDate(const DateTime &dt)
{
  double jdUT = getJulianDayUT(dt, mLocation);
  // TEST PRINTOUT
  if(jdUT != mJulDay_ut)
    {
      const DateTime dut0 = getDateFromJUT(mJulDay_ut, mLocation);
      const DateTime dut1 = getDateFromJUT(jdUT,       mLocation);
      std::cout << "====> CHANGING EPHEMERIS DATE:  " << dut0 << " (JDUT: " << mJulDay_ut
                <<                         ")  -->  " << dut1 << " (JDUT: " << jdUT << ")\n";
    }
  
  mJulDay_et = getJulianDayET(dt, mLocation);
  mJulDay_ut = jdUT;
}

void Ephemeris::setDateUT(double jdUT)
{
  // TEST PRINTOUT
  if(jdUT != mJulDay_ut)
    {
      const DateTime dut0 = getDateFromJUT(mJulDay_ut, mLocation);
      const DateTime dut1 = getDateFromJUT(jdUT,       mLocation);
      std::cout << "====> CHANGING EPHEMERIS DATE (UT):  " << dut0 << " (JDUT: " << mJulDay_ut
                <<                              ")  -->  " << dut1 << " (JDUT: " << jdUT << ")\n";
    }
  mJulDay_ut = jdUT;
  mJulDay_et = jdUT + swe_deltat(jdUT);
}

void Ephemeris::setDateET(double jdET)
{
  int    year, month, day, hour, min;
  double sec;
  double dret[2];
  char   serr[AS_MAXCH];
  
  swe_jdet_to_utc(jdET, GREG_FLAG, &year, &month, &day, &hour, &min, &sec);
  int ret = swe_utc_to_jd(year, month, day, hour, min, sec, GREG_FLAG, dret, serr);
  if(ret < 0) { std::cout << "SWE ERROR: " << serr << "\n"; }

  // TEST PRINTOUT
  if(jdET != mJulDay_et)
    {
      const DateTime det0 = getDateFromJET(mJulDay_et, mLocation);
      const DateTime det1 = getDateFromJET(jdET,       mLocation);
      std::cout << "====> CHANGING EPHEMERIS DATE (ET):  " << det0 << " (JDET: " << mJulDay_et
                <<                              ")  -->  " << det1 << " (JDET: " << jdET << ")\n";
    }
  
  mJulDay_et = jdET;
  mJulDay_ut = dret[1];
}

void Ephemeris::setLocation(const Location &loc)
{
  mLocation = loc;
}


void Ephemeris::setAyanamsa(int index)
{
  mAyanamsaIndex = index;
  swe_set_sid_mode(mAyanamsaIndex, 0.0, 0.0);
}

std::string Ephemeris::getAyanamsaName(int index)
{
  return std::string(swe_get_ayanamsa_name(index));
}

double Ephemeris::getSiderealTime(const DateTime &dt, const Location &loc)
{
  return swe_sidtime(getJulianDayUT(dt, loc));
}


DateTime Ephemeris::getDateFromJUT(double jd_UT, const Location &loc)
{
  int y, mo, d, h, mi; double s;
  swe_jdut1_to_utc(jd_UT, GREG_FLAG, &y, &mo, &d, &h, &mi, &s);
  return DateTime(y, mo, d, h, mi, s, mLocation.utcOffset);
}

DateTime Ephemeris::getDateFromJET(double jd_ET, const Location &loc)
{
  int y, mo, d, h, mi; double s;
  swe_jdet_to_utc(jd_ET, GREG_FLAG, &y, &mo, &d, &h, &mi, &s);
  return DateTime(y, mo, d, h, mi, s, loc.utcOffset);
}


double Ephemeris::getJulianDayUT(const DateTime &dt, const Location &loc)
{
  // calculate timezone offset
  double d_timezone = loc.utcOffset - (double)dt.dstOffset();
  int y, mo, d, h, mi; double s;
  swe_set_topo(loc.longitude, loc.latitude, loc.altitude);
  swe_utc_time_zone(dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second(), d_timezone, &y, &mo, &d, &h, &mi, &s);
  
  // compute julian day
  char serr[AS_MAXCH];
  double dret[2];
  int ret = swe_utc_to_jd(y, mo, d, h, mi, s, GREG_FLAG, dret, serr);
  if(ret < 0) { std::cout << "SWE ERROR: " << serr << "\n"; }
  
  return dret[1];
}

double Ephemeris::getJulianDayET(const DateTime &dt, const Location &loc)
{
  // calculate timezone offset
  double d_timezone = loc.utcOffset - (double)dt.dstOffset();
  int y, mo, d, h, mi; double s;
  swe_set_topo(loc.longitude, loc.latitude, loc.altitude);
  swe_utc_time_zone(dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second(), d_timezone, &y, &mo, &d, &h, &mi, &s);
  
  // compute julian day
  char serr[AS_MAXCH];
  double dret[2];
  int ret = swe_utc_to_jd(y, mo, d, h, mi, s, GREG_FLAG, dret, serr);
  if(ret < 0) { std::cout << "SWE ERROR: " << serr << "\n"; }
  
  return dret[0];
}

DateTime Ephemeris::getProgressed(const DateTime &ndt, const Location &nloc, const DateTime &tdt, const Location &tloc)
{
  // transit-natal differences
  double jdNatal   = getJulianDayUT(ndt, nloc);
  double jdTransit = getJulianDayUT(tdt, tloc);
  double dayDiff = jdTransit - jdNatal;
  
  if(dayDiff <= 0.0) // transit should be greater than natal
    { return ndt; }
  else
    {
      double jdProg = jdNatal + dayDiff/DAYS_PER_JULIAN_YEAR;
      int y, mo, d, h, mi; double s;
      swe_jdut1_to_utc(jdProg, GREG_FLAG, &y, &mo, &d, &h, &mi, &s);
      return DateTime(y, mo, d, h, mi, s, 0.0);
    }
}

DateTime Ephemeris::getUnprogressed(const DateTime &ndt, const Location &nloc, const DateTime &pdt, const Location &ploc)
{
  // transit-natal differences
  double jdNatal = getJulianDayUT(ndt, nloc);
  double jdProg  = getJulianDayUT(pdt, ploc);
  double dayDiff = jdProg - jdNatal;
  
  if(dayDiff <= 0.0) // progressed should be greater than natal
    { return ndt; }
  else
    {
      double jdTransit = jdNatal + dayDiff*DAYS_PER_JULIAN_YEAR;
      int y, mo, d, h, mi; double s;
      swe_jdut1_to_utc(jdTransit, GREG_FLAG, &y, &mo, &d, &h, &mi, &s);
      return DateTime(y, mo, d, h, mi, s, 0.0);
    }
}


ObjData Ephemeris::getObjData(ObjType o) const
{
  ObjData objData;
  objData.type = o;
  if(o >= ANGLE_OFFSET)
    {
      objData.valid = true;
      switch(o)
        {
        case ANGLE_ASC:
          objData.longitude = mAscmc[SE_ASC];
          objData.lonSpeed = mAscmcSpeed[SE_ASC];
          break;
        case ANGLE_MC:
          objData.longitude = mAscmc[SE_MC];
          objData.lonSpeed = mAscmcSpeed[SE_MC];
          break;
        case ANGLE_DSC:
          objData.longitude = fmod(mAscmc[SE_ASC]+180.0, 360.0);
          objData.lonSpeed = mAscmcSpeed[SE_ASC];
          break;
        case ANGLE_IC:
          objData.longitude = fmod(mAscmc[SE_MC]+180.0, 360.0);
          objData.lonSpeed = mAscmcSpeed[SE_MC];
          break;
        case ANGLE_VERTEX:
          objData.longitude = mAscmc[SE_VERTEX];
          objData.lonSpeed = mAscmcSpeed[SE_VERTEX];
          break;
        }
    }
  else
    {
      bool isSouthNode = (o == OBJ_SOUTHNODE);
      if(isSouthNode) { o = OBJ_NORTHNODE; }
      
      int p = getSweIndex(o);
      if(p < 0) { return ObjData{}; }

      // set geographic position for calculations
      swe_set_topo(mLocation.longitude, mLocation.latitude, mLocation.altitude);
      
      // calculate
      double data[6];
      char serr[AS_MAXCH];
      long iflgret = swe_calc(mJulDay_et, p, mSweFlags, data, serr);
      if(iflgret < 0)
        {
          std::cout << "SWE ERROR(" << getObjNameLong(o) << "): " << serr << "\n";
          objData.valid = false;
        }
      else { objData.valid = true; }
  
      objData.longitude = data[0];
      objData.latitude  = data[1];
      objData.distance  = data[2];
      objData.lonSpeed  = data[3];
      objData.latSpeed  = data[4];
      objData.distSpeed = data[5];
      
      if(isSouthNode)
        { // calculate south lunar node from true node
          objData.latitude  *= -1;
          objData.longitude = fmod(objData.longitude+180.0, 360.0);
          objData.latSpeed  *= -1;
        }
    }
  return objData;
}

double Ephemeris::getAngle(ObjType angle) const
{
  switch(angle)
    {
    case ANGLE_ASC:
      return mAscmc[SE_ASC];
    case ANGLE_MC:
      return mAscmc[SE_MC];
    case ANGLE_DSC:
      return fmod(mAscmc[SE_ASC]+180.0, 360.0);
    case ANGLE_IC:
      return fmod(mAscmc[SE_MC]+180.0, 360.0);
    case ANGLE_VERTEX:
      return mAscmc[SE_VERTEX];
    default:
      return -1.0;
    }
}

OrbitInfo Ephemeris::getOrbitInfo(ObjType obj) const
{
  OrbitInfo info;
  if(obj == OBJ_SUN)
    { // sun (doesn't orbit itself)
      info.orbitalPeriod   = 1.0;                  // 1 year
      info.meanDailyMotion = 360.0/DAYS_PER_JULIAN_YEAR; // daily motion
      info.tropicalPeriod  = 1.0;                  // 1 year
      info.synodicPeriod   = DAYS_PER_JULIAN_YEAR; // 1 year (365.25 days)
      info.valid = true;
    }
  else if(obj == OBJ_NORTHNODE || obj == OBJ_SOUTHNODE)
    { // lunar nodes
      // ObjData data = getObjData(obj);
      info.orbitalPeriod   = LUNAR_NODE_PERIOD;                               // ~18.6 years
      info.meanDailyMotion = -360.0/(DAYS_PER_JULIAN_YEAR*LUNAR_NODE_PERIOD); // daily motion (always retrograde)
      info.tropicalPeriod  = LUNAR_NODE_PERIOD;                               // period in years
      info.synodicPeriod   = (DAYS_PER_JULIAN_YEAR*LUNAR_NODE_PERIOD);        // period in days
      info.valid = true;
    }
  else if(obj == OBJ_LILITH)
    { // lunar nodes
      // ObjData data = getObjData(obj);
      info.orbitalPeriod   = LILITH_PERIOD;                               // ~18.6 years
      info.meanDailyMotion = -360.0/(DAYS_PER_JULIAN_YEAR*LILITH_PERIOD); // daily motion (always retrograde)
      info.tropicalPeriod  = LILITH_PERIOD;                               // period in years
      info.synodicPeriod   = (DAYS_PER_JULIAN_YEAR*LILITH_PERIOD);        // period in days
      info.valid = true;
    }
 else if(obj < OBJ_COUNT)
    { // objects
      char serr[AS_MAXCH];
      int ret = swe_get_orbital_elements(mJulDay_et, getSweIndex(obj), mSweFlags, (double*)&info, serr);
      info.valid = (ret >= 0);
      if(!info.valid) { std::cout << "SWE ERROR: " << serr << "\n"; }
    }
  else
    { // angles
      info.orbitalPeriod    = 1.0/DAYS_PER_JULIAN_YEAR; // period is 1 day
      info.meanDailyMotion  = 360.0; // 360 degrees per day
      info.tropicalPeriod   = 1.0/DAYS_PER_JULIAN_YEAR; // period is 1 day
      info.synodicPeriod    = 1.0;   // period is 1 day
      info.valid = true;
    }
  
  return info;
}


void Ephemeris::calcHouses(HouseSystem hsys)
{
  swe_set_topo(mLocation.longitude, mLocation.latitude, mLocation.altitude);
  char serr[AS_MAXCH];
  swe_houses_ex2(mJulDay_ut, mSweFlags, mLocation.latitude, mLocation.longitude, hsys, mCusps, mAscmc, mCuspSpeed, mAscmcSpeed, serr);
}

double Ephemeris::getHouseCusp(int house) const
{ return ((house >= 1 && house <= 12) ? mCusps[house] : -1.0); }

void Ephemeris::printHouses() const
{
  // print ascendant
  int deg = (int)std::floor(mAscmc[SE_ASC]);
  int min = (int)std::floor((mAscmc[SE_ASC] - deg)*60.0);
  std::cout << "ASC: " << deg << "°" << min << "'\n";
  
  // print house cusps  
  std::cout << "CUSPS:\n";
  for(int i = 1; i <= 12; i++)
    {
      double angle = getHouseCusp(i);
      std::cout << i << ": " << angle << " | " << SIGN_NAMES[(int)std::floor(fmod(angle, 360.0)/30.0)] << "\n";
    }
  std::cout << "\n";
}

void Ephemeris::printObjects(const DateTime &dt, const Location &loc) const
{
  std::cout << "------------------------------------------------------------------------------------------------------------------------\n";
  std::cout << "|  " << dt << "\n";
  std::cout << "|  " << loc << "\n";
  std::cout << "|----------------------------------------------------------------------------------------------------------------------|\n";
  std::cout << "|      OBJECT |        ANGLE |     LATITUDE |    LONGITUDE |     DISTANCE |    LAT SPEED |    LON SPEED |   DIST SPEED |\n";
  std::cout << "|             |    (degrees) |    (degrees) |    (degrees) |         (AU) |    (degrees) |    (degrees) |     (AU/day) |\n";
  std::cout << "|=============|==============|==============|==============|==============|==============|==============|==============|\n";
  for(int o = OBJ_SUN; o < OBJ_COUNT; o++)
    {
      ObjData obj = getObjData(o);
      double angle = obj.longitude;
      std::string name = getObjName(o);
      std::cout << std::fixed << std::setprecision(6)
                << "|" << std::setw(12) << name  << " | " << std::setw(12) << angle << " | "
                << std::setw(12) << obj.latitude << " | " << std::setw(12) << obj.longitude << " | " << std::setw(12) << obj.distance  << " | "
                << std::setw(12) << obj.latSpeed << " | " << std::setw(12) << obj.lonSpeed  << " | " << std::setw(12) << obj.distSpeed << " |\n";
    }
  std::cout << "------------------------------------------------------------------------------------------------------------------------\n";
}
