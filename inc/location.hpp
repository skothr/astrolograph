#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <array>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <type_traits>

#include "astro.hpp"
#include <curl/curl.h>

///////////////////////////////////////////////////////////////////////////
// TODO: TIMEZONES:
//    e.g. -->   http://api.geonames.org/timezoneJSON?lat=38.652100839431&lng=-90.502670955469&date=1993-12-27&username=skothr
//       (can return string id, e.g. "America/New York") --> plug into date/tz
//
#define GEONAMES_USERNAME "skothr" // geonames API username
///////////////////////////////////////////////////////////////////////////
#define GEONAMES_URL             "http://api.geonames.org/"
#define GEONAMES_TIMEZONE_PREFIX "timezoneJSON?"
///////////////////////////////////////////////////////////////////////////


// location of New York Stock Exchange (used as default coordinates)
#define NYSE_LAT      40.706833333333    // degrees NORTH
#define NYSE_LON     -74.011027777778    // degrees WEST
#define NYSE_ALT      200.00000000000    // meters (approx.)
#define NYSE_TZNAME   "America/New_York" // NYSE timezone name
#define NYSE_OFFSET   -5.0               // UTC offset

namespace astro
{
  class DateTime;
  
  // LOCATION -- defines a location with latitude, longitude, and altitude //
  struct Location
  {
    double      latitude   = NYSE_LAT;    // degrees (+N/-S)
    double      longitude  = NYSE_LON;    // degrees (+E/-W)
    double      altitude   = NYSE_ALT;    // meters
    std::string timezoneId = NYSE_TZNAME; // timezone label
    double      utcOffset  = NYSE_OFFSET; // timezone UTC offset
    
    Location();
    Location(const std::string &saveStr) { fromSaveString(saveStr); }
    Location(double lat, double lon, double alt);
    Location(const Location &other);
    Location& operator=(const Location &other);

    static std::size_t curlCallback(const char* in, std::size_t size, std::size_t num, std::string *out);
    static std::string getTimezoneCurl(const astro::Location &loc);

    void updateTimezone(); // updates timezone member via getTimezoneCurl() (NOTE: use sparingly --> ~2500 free queries per username per day)
    void updateUtcOffset();
    
    bool valid() const;
    void fix();
    Location fixed() const;
    
    double getTimezoneOffset(const DateTime &dt) const;

    std::string toString() const
    {
      std::ostringstream os;
      os << (*this);
      return os.str();
    }
    std::string toSaveString() const
    {
      std::ostringstream os;
      os << to_string(latitude, 12) << " " << to_string(longitude, 12) << " " << altitude << " " << std::quoted(timezoneId) << " " << utcOffset;
      return os.str();
    }
    std::string fromSaveString(const std::string &str)
    {
      std::istringstream is(str);
      std::string name;
      is >> latitude; is >> longitude; is >> altitude; is >> std::quoted(timezoneId); is >> utcOffset;
      updateUtcOffset();
      
      // return remaining string
      std::stringstream tmp; tmp << is.rdbuf();
      return tmp.str();
    }
    
    bool operator==(const Location &other) const
    { return (latitude == other.latitude && longitude == other.longitude && altitude == other.altitude); }
    bool operator!=(const Location &other) const
    { return !(*this == other); }
    
    friend std::ostream& operator<<(std::ostream &os, const Location &loc);
    friend std::basic_ostream<wchar_t>& operator<<(std::basic_ostream<wchar_t> &os, const Location &loc);
    //friend std::istream& operator>>(std::istream &is, DateTime &date);
  };

  inline std::ostream& operator<<(std::ostream &os, const Location &loc)
  {
    os << std::fixed << std::setprecision(6) << loc.latitude << "°" << (loc.latitude < 0 ? "S" : "N") << "  "
       << loc.longitude << "°" << (loc.longitude < 0 ? "W" : "E") << "  "
       << std::setprecision(2) << loc.altitude << " m";
    
    // os << "[" << loc.latitude << ", " << loc.longitude << ", " << loc.altitude << "]";
    return os;
  }
  
  inline std::basic_ostream<wchar_t>& operator<<(std::basic_ostream<wchar_t> &os, const Location &loc)
  {
    os << std::fixed << std::setprecision(6) << loc.latitude << L"°" << (loc.latitude < 0 ? L"S" : L"N") << L" / "
       << loc.longitude << L"°" << (loc.longitude < 0 ? L"W" : L"E") << L" / "
       << std::setprecision(2) << loc.altitude << L"m";
    
    // os << "[" << loc.latitude << ", " << loc.longitude << ", " << loc.altitude << "]";
    return os;
  }

  // inline std::istream& operator>>(std::istream &is, Location &loc)
  // {
  //   is >> loc.latitude;
  //   is >> loc.longitude;
  //   is >> loc.altitude;
  //   return is;
  // }
}


#endif // LOCATION_HPP
