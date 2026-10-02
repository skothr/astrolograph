#ifndef EPHEMERIS_HPP
#define EPHEMERIS_HPP

#include <array>
#include <vector>
#include <algorithm>
#include "swephexp.h"

#include "astro.hpp"

// path to ephemeris data
#define EPHEM_PATH "./libs/swe/ephe"

#define AYANAMSA_COUNT       46
namespace astro
{
  struct OrbitInfo
  {
    double semimajorAxis    = 0.0; // dret[0]  -- semimajor axis (a)
    double eccentricity     = 0.0; // dret[1]  -- eccentricity (e)
    double inclination      = 0.0; // dret[2]  -- inclination (in)
    double ascendingNode    = 0.0; // dret[3]  -- longitude of ascending node (upper case omega OM)
    double periapsisArg     = 0.0; // dret[4]  -- argument of periapsis (lower case omega om)
    double periapsisLon     = 0.0; // dret[5]  -- longitude of periapsis (peri)
    double meanEpochAnomaly = 0.0; // dret[6]  -- mean anomaly at epoch (M0)
    double trueEpochAnomaly = 0.0; // dret[7]  -- true anomaly at epoch (N0)
    double eccEpochAnomaly  = 0.0; // dret[8]  -- eccentric anomaly at epoch (E0)
    double meanEpochLon     = 0.0; // dret[9]  -- mean longitude at epoch (LM)
    double orbitalPeriod    = 0.0; // dret[10] -- sidereal orbital period in tropical years
    double meanDailyMotion  = 0.0; // dret[11] -- mean daily motion
    double tropicalPeriod   = 0.0; // dret[12] -- tropical period in years
    double synodicPeriod    = 0.0; // dret[13] -- synodic period in days, negative, if inner planet (Venus, Mercury, Aten asteroids) or Moon
    double perihelionTime   = 0.0; // dret[14] -- time of perihelion passage
    double perihelionDist   = 0.0; // dret[15] -- perihelion distance
    double aphelionDist     = 0.0; // dret[16] -- aphelion distance
    bool   valid            = false;
  };

#define GREG_FLAG SE_GREG_CAL
  
  class Ephemeris
  {
  private:
    ///////// INSIDE DEGREES TEST //////////
#define INSIDE_DEGREES_PATH "./res/inside-degrees.txt"
    static std::array<std::array<std::string, 30>, 12> insideDegreesShort; // ACCESS: arr[SIGN_INDEX][floor(DEGREE)]
    static std::array<std::array<std::string, 30>, 12> insideDegreesLong;  // ACCESS: arr[SIGN_INDEX][floor(DEGREE)]
    static bool mInsideDegreesLoaded;
    static bool loadInsideDegrees();

    DateTime mDateTime;
    Location mLocation;
    double mJulDay_ut = 2269000.0; // TODO: Proper defaults
    double mJulDay_et = 2269000.0;
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////// SWE ASCMC ARRAY ////////    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //       In the array ascmc, the function returns the following values:
    //       ascmc[0] = Ascendant                          (SE_ASC)   
    //       ascmc[1] = MC                                 (SE_MC)    
    //       ascmc[2] = ARMC                               (SE_ARMC)  
    //       ascmc[3] = Vertex                             (SE_VERTEX)
    //       ascmc[4] = "equatorial ascendant"             (SE_EQUASC)
    //       ascmc[5] = "co-ascendant" (Walter Koch)       (SE_COASC1)
    //       ascmc[6] = "co-ascendant" (Michael Munkasey)  (SE_COASC2)
    //       ascmc[7] = "polar ascendant" (M. Munkasey)    (SE_POLASC)
    //       ascmc must be an array of 10 doubles. ascmc[8... 9] are 0 and may be used for additional points in future releases.
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    double mAscmc[10]; double mAscmcSpeed[10];
    double mCusps[13]; double mCuspSpeed[13];
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////// SWE FLAGS ////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // SEFLG_JPLEPH          - use JPL ephemeris
    // SEFLG_SWIEPH          - use SWISSEPH ephemeris, default
    // SEFLG_MOSEPH          - use Moshier ephemeris
    // SEFLG_HELCTR          - return heliocentric position
    // SEFLG_TRUEPOS         - return true positions, not apparent
    // SEFLG_J2000           - no precession, i.e. give J2000 equinox
    // SEFLG_NONUT           - no nutation, i.e. mean equinox of date
    // SEFLG_SPEED3          - speed from 3 positions (do not use it, SEFLG_SPEED is faster and more precise.)
    // SEFLG_SPEED           - high precision speed (analyt. comp.)
    // SEFLG_NOGDEFL         - turn off gravitational deflection
    // SEFLG_NOABERR         - turn off 'annual' aberration of light
    // SEFLG_ASTROMETRIC     - astrometric positions
    // SEFLG_EQUATORIAL      - equatorial positions are wanted
    // SEFLG_XYZ             - cartesian, not polar, coordinates
    // SEFLG_RADIANS         - coordinates in radians, not degrees
    // SEFLG_BARYCTR         - barycentric positions
    // SEFLG_TOPOCTR         - topocentric positions
    // SEFLG_SIDEREAL        - sidereal positions
    // SEFLG_ICRS            - ICRS (DE406 reference frame)
    // SEFLG_DPSIDEPS_1980   - reproduce JPL Horizons 1962 - today to 0.002 arcsec.
    // SEFLG_JPLHOR_APPROX   - approximate JPL Horizons 1962 - today
    // SEFLG_CENTER_BODY     - calculate position of center of body (COB) of planet, not barycenter of its system
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    long mSweFlags = SEFLG_SWIEPH | SEFLG_SPEED | SEFLG_TOPOCTR;
    int mAyanamsaIndex = SE_SIDM_GALCENT_0SAG; // default -- 0 degrees saggitarius aligns with galactic center
    // TODO: custom ayanamsas (?)    
    
  public:
    static const std::vector<int> SWE_IDS;
    static std::string getInsideDegreeTextShort(int sign, int degree);
    static std::string getInsideDegreeTextLong(int sign, int degree);
    
    Ephemeris();
    static int getSweIndex(ObjType obj)
    {
      if(obj < OBJ_COUNT) { return SWE_IDS[obj]; }
      else                { return -1; }
    }
    static ObjType getObjType(int sweIndex)
    {
      auto it = std::find(SWE_IDS.begin(), SWE_IDS.end(), sweIndex);
      if(it != SWE_IDS.end()) { return std::distance(SWE_IDS.begin(), it); }
      else                    { return OBJ_INVALID; }
    }

    void setSidereal(bool state)
    {
      if(state) { mSweFlags |= SEFLG_SIDEREAL; }
      else      { mSweFlags &= ~SEFLG_SIDEREAL; }
    }
    void setAyanamsa(int index);
    std::string getAyanamsaName(int index);
    double getSiderealTime(const DateTime &dt, const Location &loc);
    bool getSidereal() const { return (mSweFlags & SEFLG_SIDEREAL); }
    void setTruePos(bool state)
    {
      if(state) { mSweFlags |= SEFLG_TRUEPOS; }
      else      { mSweFlags &= ~SEFLG_TRUEPOS; }
    }
    bool getTruePos() const { return (mSweFlags & SEFLG_TRUEPOS); }

    double getJulianDay() const { return mJulDay_ut; }
    double getJulianDayUT(const DateTime &dt, const Location &loc);
    DateTime getDateFromJUT(double jd_UT, const Location &loc);
    DateTime getDateFromJET(double jd_ET, const Location &loc);
    double getJulianDayET(const DateTime &dt, const Location &loc);
    // treat each year as a day (365.25 julian days)
    DateTime getProgressed(const DateTime &ndt, const Location &nloc, const DateTime &tdt, const Location &tloc);
    DateTime getUnprogressed(const DateTime &ndt, const Location &nloc, const DateTime &pdt, const Location &ploc);
    
    void setLocation(const Location &loc);
    void setDate(const DateTime &dt);
    void setDateUT(double jdUT);
    void setDateET(double jdET);
    ObjData getObjData(ObjType obj) const;
    double getAngle(ObjType angle) const;

    OrbitInfo getOrbitInfo(ObjType obj) const;

    void calcHouses(HouseSystem hsys);
    double getHouseCusp(int house) const;
    
    void printHouses() const;
    void printObjects(const DateTime &dt, const Location &loc) const;
  };
}

#endif // EPHEMERIS_HPP
