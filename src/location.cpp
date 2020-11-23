#include "location.hpp"
using namespace astro;

#include "dateTime.hpp"
#include "date/tz.h"
#include <curl/curl.h>

//// LOCATION ////
// Location::Location() : Location(NYSE_LAT, NYSE_LON, NYSE_ALT) { }

Location::Location(double lat, double lon, double alt)
  : latitude(lat), longitude(lon), altitude(alt)
{ }
//   // use OS time zone as default
//   auto current = std::chrono::system_clock::now();
//   auto zoned = date::make_zoned(date::current_zone(), current);
//   int offset = zoned.get_info().offset.count();
//   timezoneId = date::current_zone()->name();
//   utcOffset  = ((double)offset) / 60.0/60.0;
// }

  Location::Location(const Location &other)
  : latitude(other.latitude), longitude(other.longitude), altitude(other.altitude), timezoneId(other.timezoneId), utcOffset(other.utcOffset)
{ }
Location& Location::operator=(const Location &other)
{
  latitude   = other.latitude;
  longitude  = other.longitude;
  altitude   = other.altitude;
  timezoneId = other.timezoneId;
  utcOffset  = other.utcOffset;
  return *this;
}

bool Location::valid() const
{
  return (latitude >= -90.0f && latitude <= 90.0f &&
          longitude > -180.0f && longitude <= 180.0f); 
}

void Location::fix()
{
  while(latitude < -90.0f)    { latitude += 180.0; }
  while(latitude > 90.0f)     { latitude -= 180.0; }
  while(longitude <= -180.0f) { longitude += 360.0; }
  while(longitude > 180.0f)   { longitude -= 360.0; }
}

Location Location::fixed() const
{
  Location loc(*this);
  loc.fix();
  return loc;
}

// TIMEZONE //
std::size_t Location::curlCallback(const char* in, std::size_t size, std::size_t num, std::string *out)
{
  const std::size_t totalBytes(size*num);
  out->append(in, totalBytes);
  return totalBytes;
}

// hacky timezone query
std::string Location::getTimezoneCurl(const astro::Location &loc)
{
  std::string timezone = "";
  CURL *curl = curl_easy_init();
  if(curl)
    {
      std::string url = (std::string(GEONAMES_URL GEONAMES_TIMEZONE_PREFIX) +
                         "lat=" + to_string(loc.latitude, 12) + "&lng=" + to_string(loc.longitude, 12) +
                         "&username=" + GEONAMES_USERNAME);

      std::cout << "Querying timezone using Geonames...\n";
      std::cout << "  (URL: " << url << ")\n";
      
      std::unique_ptr<std::string> httpData(new std::string());
      
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());
      std::cout << "...";
      CURLcode res = curl_easy_perform(curl);
      
      std::cout << "DONE\n";
      
      long httpCode = 0L;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
      curl_easy_cleanup(curl);

      // TODO: proper JSON parsing
      if(httpCode == 200)
        {
          std::cout << "  --> Result: " << *httpData.get() << "\n";
          // find "timezoneId" section
          std::string tzLabel = "timezoneId";
          std::size_t labelPos = httpData.get()->find(tzLabel);
          if(labelPos != std::string::npos)
            { // separate value from label
              std::string rest = httpData.get()->substr(labelPos+tzLabel.size()+1);
              std::size_t colonPos = rest.find(":");
              std::size_t commaPos = rest.find(",");
              std::string valueStr = rest.substr(colonPos+2, commaPos-(colonPos+2)-1);
              timezone = valueStr;
            }
        }
    }
  return timezone;
}

void Location::updateTimezone()
{
  timezoneId = getTimezoneCurl(*this);
  updateUtcOffset();
}

void Location::updateUtcOffset()
{
  if(timezoneId.empty())
    {
      utcOffset = 0.0;
      dstOffset = 0.0;
      return;
    }
  else
    {
      DateTime dt = DateTime::now();
      const date::time_zone* tz = date::locate_zone(timezoneId);
      auto sysTime = date::sys_time<date::days>{date::year(dt.year()) / date::month(dt.month()) / date::day(dt.day())};
      // + (dt.hour())h + (dt.minute())n + (dt.second())s;
      date::sys_info info = tz->get_info(sysTime);
      utcOffset = ((info.offset).count()/60.0 + info.save.count())/60.0;
      dstOffset = (info.save.count())/60.0;
    }
}

double Location::getTimezoneOffset(DateTime &dt) const
{
  if(timezoneId.empty()) { return 0.0; }
  
  const date::time_zone* tz = date::locate_zone(timezoneId);
  auto sysTime = date::sys_time<date::days>{date::year(dt.year()) / date::month(dt.month()) / date::day(dt.day())};
  // + (dt.hour())h + (dt.minute())n + (dt.second())s;
  date::sys_info info = tz->get_info(sysTime);

  // offset in seconds, save in minutes --> convert to hours
  dt.setUtcOffset((info.offset.count()/60.0 - info.save.count())/60.0);
  dt.setDstOffset(info.save.count()/60.0);
  return info.offset.count() / (60.0*60.0);
}
