#ifndef EPHEMERIS_HPP
#define EPHEMERIS_HPP

#include <array>
#include <vector>
#include <algorithm>
#include "swephexp.h"

#include "astro.hpp"

// path to ephemeris data
#define EPHEM_PATH "./libs/swe/ephe"

namespace astro
{
  class Ephemeris
  {
  private:
    DateTime mDateTime;
    Location mLocation;
    
    double mJulDay_ut = 2269000.0; // TODO: Proper defaults?
    double mJulDay_et = 2269000.0;

    // HouseSystem mHouseSystem = HOUSE_PLACIDUS;//HOUSE_WHOLESIGN;
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //       In the array ascmc, the function returns the following values:
    //       ascmc[0] = Ascendant
    //       ascmc[1] = MC
    //       ascmc[2] = ARMC
    //       ascmc[3] = Vertex
    //       ascmc[4] = "equatorial ascendant"
    //       ascmc[5] = "co-ascendant" (Walter Koch)
    //       ascmc[6] = "co-ascendant" (Michael Munkasey)
    //       ascmc[7] = "polar ascendant" (M. Munkasey)
    //       The following defines can be used to find these values:
    // #define SE_ASC       0
    // #define SE_MC        1
    // #define SE_ARMC      2
    // #define SE_VERTEX         3
    // #define SE_EQUASC         4    /* "equatorial ascendant" */
    // #define SE_COASC1         5    /* "co-ascendant" (W. Koch) */
    // #define SE_COASC2         6    /* "co-ascendant" (M. Munkasey) */
    // #define SE_POLASC         7    /* "polar ascendant" (M. Munkasey) */
    // #define SE_NASCMC         8
    //       ascmc must be an array of 10 doubles. ascmc[8... 9] are 0 and may be used for additional points in future releases.
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    double mAscmc[10];
    double mAscmcSpeed[10];
    double mCusps[13];
    double mCuspSpeed[13];
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////// SWE FLAGS ////////
    // #define SEFLG_JPLEPH       1L               // use JPL ephemeris
    // #define SEFLG_SWIEPH       2L               // use SWISSEPH ephemeris, default
    // #define SEFLG_MOSEPH       4L               // use Moshier ephemeris
    // #define SEFLG_HELCTR       8L               // return heliocentric position
    // #define SEFLG_TRUEPOS      16L              // return true positions, not apparent
    // #define SEFLG_J2000        32L              // no precession, i.e. give J2000 equinox
    // #define SEFLG_NONUT        64L              // no nutation, i.e. mean equinox of date
    // #define SEFLG_SPEED3       128L             // speed from 3 positions (do not use it, SEFLG_SPEED is faster and more precise.)
    // #define SEFLG_SPEED        256L             // high precision speed (analyt. comp.)
    // #define SEFLG_NOGDEFL      512L             // turn off gravitational deflection
    // #define SEFLG_NOABERR      1024L            // turn off 'annual' aberration of light
    // #define SEFLG_ASTROMETRIC (SEFLG_NOABERR|SEFLG_NOGDEFL) // astrometric positions
    // #define SEFLG_EQUATORIAL   2048L            // equatorial positions are wanted
    // #define SEFLG_XYZ          4096L            // cartesian, not polar, coordinates
    // #define SEFLG_RADIANS      8192L            // coordinates in radians, not degrees
    // #define SEFLG_BARYCTR      16384L           // barycentric positions
    // #define SEFLG_TOPOCTR      (32*1024L)       // topocentric positions
    // #define SEFLG_SIDEREAL     (64*1024L)       // sidereal positions
    // #define SEFLG_ICRS         (128*1024L)      // ICRS (DE406 reference frame)
    // #define SEFLG_DPSIDEPS_1980 (256*1024)      /* reproduce JPL Horizons
    // * 1962 - today to 0.002 arcsec. */
    // #define SEFLG_JPLHOR SEFLG_DPSIDEPS_1980
    // #define SEFLG_JPLHOR_APPROX (512*1024)      /* approximate JPL Horizons 1962 - today */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    long mSweFlags = SEFLG_SWIEPH | SEFLG_SPEED | SEFLG_TOPOCTR;
    
  public:
    static const std::vector<int> SWE_IDS;
    
    Ephemeris();
    
    static int getSweIndex(ObjType obj)
    {
      if(obj < OBJ_COUNT)
        { return SWE_IDS[obj]; }
      else
        { return -1; }
    }
    static ObjType getObjType(int sweIndex)
    {
      auto it = std::find(SWE_IDS.begin(), SWE_IDS.end(), sweIndex);
      if(it != SWE_IDS.end()) { return (ObjType)(std::distance(SWE_IDS.begin(), it)); }
      else                    { return OBJ_INVALID; }
    }

    void setSidereal(bool state)
    {
      if(state) { mSweFlags |= SEFLG_SIDEREAL; }
      else      { mSweFlags &= ~SEFLG_SIDEREAL; }
    }
    bool getSidereal() const { return (mSweFlags & SEFLG_SIDEREAL); }
    void setTruePos(bool state)
    {
      if(state) { mSweFlags |= SEFLG_TRUEPOS; }
      else      { mSweFlags &= ~SEFLG_TRUEPOS; }
    }
    bool getTruePos() const { return (mSweFlags & SEFLG_TRUEPOS); }

    double getJulianDay() const { return mJulDay_ut; }
    double getJulianDayUT(const DateTime &dt, const Location &loc);
    double getJulianDayET(const DateTime &dt, const Location &loc);
    // treat each year as a day
    DateTime getProgressed(const DateTime &ndt, const Location &nloc, const DateTime &tdt, const Location &tloc);
    DateTime getUnprogressed(const DateTime &ndt, const Location &nloc, const DateTime &pdt, const Location &ploc);
    
    void setLocation(const Location &loc);
    void setDate(const DateTime &dt);
    ObjData getObjData(ObjType obj) const;
    double getAngle(ObjType angle) const;

    void calcHouses(HouseSystem hsys);
    double getHouseCusp(int house) const;
    
    void printHouses() const;
    void printObjects(const astro::DateTime &dt, const astro::Location &loc) const;
  };
}

#endif // EPHEMERIS_HPP
