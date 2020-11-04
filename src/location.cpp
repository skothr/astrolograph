#include "location.hpp"
using namespace astro;

#include "dateTime.hpp"
#include "date/tz.h"

//// LOCATION ////
Location::Location(double lat, double lon, double alt)
  : latitude(lat), longitude(lon), altitude(alt)
{ }
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
  const std::size_t totalBytes(size * num);
  out->append(in, totalBytes);
  return totalBytes;
}

// hacky timezone query
std::string Location::getTimezoneCurl(const astro::Location &loc)
{
  CURL *curl = curl_easy_init();
  if(curl)
    {
      std::string url = (std::string(GEONAMES_URL GEONAMES_TIMEZONE_PREFIX) +
                         "lat=" + to_string(loc.latitude, 12) + "&lng=" + to_string(loc.longitude, 12) +
                         "&username=" + GEONAMES_USERNAME);
      // std::cout << "URL: " << url << "\n";
      // std::cout << "-----------------------\n\n";

      std::cout << "Querying timezone using Geonames...\n";
      std::cout << "  --> URL: " << url << "\n";
      
      std::unique_ptr<std::string> httpData(new std::string());
      
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());
      CURLcode res = curl_easy_perform(curl);
      
      long httpCode = 0L;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
      
      // std::cout << "CURL HTTP CODE: " << httpCode << "\n";
      // std::cout << "CURL HTTP DATA: " << *httpData.get() << "\n";
      curl_easy_cleanup(curl);

      // TODO: proper JSON parsing
      if(httpCode == 200)
        {
          std::string tzLabel = "timezoneId";
          // find "timezoneId" label
          std::size_t labelPos = httpData.get()->find(tzLabel);
          if(labelPos != std::string::npos)
            { // separate value from label
              std::string rest = httpData.get()->substr(labelPos+tzLabel.size()+1);
              std::size_t colonPos = rest.find(":");
              std::size_t commaPos = rest.find(",");
              std::string valueStr = rest.substr(colonPos+2, commaPos-(colonPos+2)-1);
              // std::cout << "  VALUE: " << valueStr << "\n";
              return valueStr;
            }
        }
    }
  return "";
}

void Location::updateTimezone()
{
  timezoneId = getTimezoneCurl(*this);
  updateUtcOffset();
}

void Location::updateUtcOffset()
{
  utcOffset = getTimezoneOffset(DateTime::now());
}

double Location::getTimezoneOffset(const DateTime &dt) const
{
  if(timezoneId.empty()) { return 0.0; }

  const date::time_zone* tz = date::locate_zone(timezoneId);

  std::string dateStr = (std::to_string(dt.year())+"-"+std::to_string(dt.month())+"-"+std::to_string(dt.day()) +
                         "T"+std::to_string(dt.hour())+":"+std::to_string(dt.minute())+":"+std::to_string(dt.second())+"Z");
  std::istringstream ss(dateStr);
  date::sys_time<std::chrono::milliseconds> t;
  ss >> date::parse("%a %b %d %T %z %Y", t);
  date::sys_info tzinfo = tz->get_info(t);

  // std::cout << t.count() <<"\n";
  
  //std::cout << "(offset --> " << tzinfo.offset.count() << " | save --> " << tzinfo.save.count() << ")\n";
  return (tzinfo.offset).count() / (60.0*60.0); // convert from seconds to hours
  
  // // (longitude-based estimation)
  // return std::floor(longitude*24.0/360.0) + (longitude < 0.0 ? 1 : 0.0); // add 1 if longitude below 0 due to floor()
}
