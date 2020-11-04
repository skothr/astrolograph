#include "ephemeris.hpp"
using namespace astro;

#include <cmath>
#include <iostream>
#include <iomanip>


const std::vector<int> Ephemeris::SWE_IDS = { SE_SUN, SE_MOON,
                                              SE_MERCURY, SE_VENUS, SE_MARS, SE_JUPITER, SE_SATURN, SE_URANUS, SE_NEPTUNE, SE_PLUTO, 60000,//SE_QUAOAR,
                                              SE_CHIRON, SE_CERES, SE_JUNO, SE_PALLAS, SE_VESTA,
                                              SE_MEAN_APOG,             // (Dark Moon Lilith)
                                              //(SE_AST_OFFSET + 19),   // (Fortuna)
                                              SE_TRUE_NODE, SE_TRUE_NODE }; // (south node calculated from north node)



Ephemeris::Ephemeris()
{
  swe_set_ephe_path(EPHEM_PATH);
}

void Ephemeris::setDate(const DateTime &dt, bool daylightSavings)
{
  mJulDay_et = getJulianDayET(dt, mLocation, daylightSavings);
  mJulDay_ut = getJulianDayUT(dt, mLocation, daylightSavings);
}

void Ephemeris::setLocation(const Location &loc)
{
  mLocation = loc;
}

double Ephemeris::getJulianDayUT(const DateTime &dt, const Location &loc, bool daylightSavings)
{
  // calculate timezone
  double d_timezone = dt.tzOffset();
  int y, mo, d, h, mi;
  double s;

  swe_set_topo(loc.longitude, loc.latitude, loc.altitude);
  swe_utc_time_zone(dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second(), d_timezone, &y, &mo, &d, &h, &mi, &s);
  
  // compute julian day
  char serr[AS_MAXCH];
  double dret[2];
  int ret = swe_utc_to_jd(y, mo, d, h, mi, s, SE_GREG_CAL, dret, serr);
  if(ret < 0) { std::cout << "SWE ERROR: " << serr << "\n"; }
  
  return dret[1];
}

double Ephemeris::getJulianDayET(const DateTime &dt, const Location &loc, bool daylightSavings)
{
  // calculate timezone
  double d_timezone = dt.tzOffset();
  int y, mo, d, h, mi;
  double s;

  swe_set_topo(loc.longitude, loc.latitude, loc.altitude);
  swe_utc_time_zone(dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second(), d_timezone, &y, &mo, &d, &h, &mi, &s);
  
  // compute julian day
  char serr[AS_MAXCH];
  double dret[2];
  int ret = swe_utc_to_jd(y, mo, d, h, mi, s, SE_GREG_CAL, dret, serr);
  if(ret < 0) { std::cout << "SWE ERROR: " << serr << "\n"; }
  
  return dret[0];
}

DateTime Ephemeris::getProgressed(const DateTime &ndt, const Location &nloc, const DateTime &tdt, const Location &tloc, bool nds, bool tds)
{
  // // transit-natal differences
  // DateTime diff = transit - natal;
  // // start with natal date
  // DateTime pdt = natal;
  // // offset progressed
  // pdt += diff/365.25;

  double jdNatal = getJulianDayUT(ndt, nloc, nds);
  double jdTransit = getJulianDayUT(tdt, tloc, tds);

  double dayDiff = jdTransit - jdNatal;
  
  if(dayDiff <= 0.0) // transit should be greater than natal
    { return ndt; }
  else
    {
      double jdProg = jdNatal + dayDiff/365.25;
      
      int y, mo, d, h, mi;
      double s;
      swe_jdut1_to_utc(jdProg, SE_GREG_CAL, &y, &mo, &d, &h, &mi, &s);
      return DateTime(y, mo, d, h, mi, s, 0.0);
    }
}


ObjData Ephemeris::getObjData(ObjType obj) const
{
  ObjData objData;

  if(obj >= ANGLE_OFFSET)
    {
      
    }
  else
    {
      char serr[AS_MAXCH];
      //long iflag = SEFLG_SPEED | SEFLG_TOPOCTR | SEFLG_TRUEPOS;// | SEFLG_EQUATORIAL;// | SEFLG_TRUEPOS;
  
      int p = getSweIndex(obj);
      if(p < 0) { return ObjData{}; }

      // set geographic position for calculations
      swe_set_topo(mLocation.longitude, mLocation.latitude, mLocation.altitude);
      
      // swe calc
      double data[6];
      long iflgret = swe_calc(mJulDay_et, p, mSweFlags, data, serr);
      if(iflgret < 0) { std::cout << "SWE ERROR: " << serr << "\n"; }
  
      objData = *reinterpret_cast<ObjData*>(data);
  
      if(obj == OBJ_SOUTHNODE)
        { // calculate south lunar node from true node
          objData.latitude  = fmod(objData.latitude+90.0, 180.0);
          objData.longitude = fmod(objData.longitude+180.0, 360.0);
          objData.latSpeed  *= -1;
          objData.lonSpeed *= -1;
        }
    }
  return objData;
}

double Ephemeris::getObjAngle(ObjType obj) const
{
  if(obj >= ANGLE_OFFSET)
    {
      switch(obj)
        {
        case ANGLE_ASC:
          return mAsc;
        case ANGLE_DSC:
          return mDsc;
        case ANGLE_MC:
          return mMc;
        case ANGLE_IC:
          return mIc;
        default:
          return -1.0;
        }
    }
  else
    {
      ObjData data = getObjData(obj);
      return data.longitude;
    }
}

double Ephemeris::getAngle(ObjType angle) const
{
  switch(angle)
    {
    case ANGLE_ASC:
      return mAsc;
    case ANGLE_DSC:
      return mDsc;
    case ANGLE_MC:
      return mMc;
    case ANGLE_IC:
      return mIc;
    default:
      return -1.0;
    }
}

void Ephemeris::calcHouses()
{
  swe_set_topo(mLocation.longitude, mLocation.latitude, mLocation.altitude);
  swe_houses(mJulDay_ut, mLocation.latitude, mLocation.longitude, mHouseSystem, mCusps, mAscmc);
  mAsc = mAscmc[SE_ASC];
  mMc  = mAscmc[SE_MC];
  mDsc = fmod(mAsc + 180.0, 360.0);
  mIc  = fmod(mMc + 180.0, 360.0);
}

double Ephemeris::getHouseCusp(int house) const
{
  return mCusps[house];// + mAsc;
}

// return index of the sign containing the given ecliptic angle
int Ephemeris::getSign(double longitude) const
{ return (int)std::floor(fmod(longitude, 360.0)/30.0); }
// return number of the house containing the given ecliptic angle
int Ephemeris::getHouse(double longitude) const
{
  for(int i = 1; i < 12; i++)
    {
      int ni = (i == 12 ? 1 : i+1);
      double h1 = getHouseCusp(i);
      double h2 = getHouseCusp(ni);
      if(h2 < h1)
        {
          //if(longitude < h2) { longitude += 2.0*M_PI; }
          h2 = h1 + angleDiff(h2, h1);
        }
      if(longitude >= h1 && longitude <= h2) { return i; }
    }
}

double Ephemeris::getSignCusp(int sign) const
{
  if(sign >= 0 && sign < 12)
    { return sign*30.0; } // aries=0
  else
    { return -1.0; }
}


double Ephemeris::getSignCusp(const std::string &name) const
{ return getSignCusp(getSignIndex(name)); }


void Ephemeris::printHouses() const
{
  // print ascendant
  int deg = (int)std::floor(mAsc);
  int min = ((mAsc - deg)*60.0f);
  std::cout << "ASC: " << deg << "°" << min << "'\n";
  
  // print house cusps  
  std::cout << "CUSPS:\n";
  for(int i = 1; i <= 12; i++)
    {
      double angle = getHouseCusp(i);
      std::cout << i << ": " << angle << " | " << SIGN_NAMES[getSign(angle)] << "\n";
    }
  std::cout << "\n";
}

void Ephemeris::printObjects(const astro::DateTime &dt, const astro::Location &loc) const
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
      ObjData obj = getObjData((ObjType)o);
      double angle = getObjAngle((ObjType)o);
      std::string name = getObjName((ObjType)o);
      std::cout << std::fixed << std::setprecision(6)
                << "|" << std::setw(12) << name << " | " << std::setw(12) << angle << " | "
                << std::setw(12) << obj.latitude << " | " << std::setw(12) << obj.longitude << " | " << std::setw(12) << obj.distance << " | "
                << std::setw(12) << obj.latSpeed << " | " << std::setw(12) << obj.lonSpeed << " | " << std::setw(12) << obj.distSpeed << " |\n";
    }
  std::cout << "------------------------------------------------------------------------------------------------------------------------\n";
}
