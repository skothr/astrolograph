#include "dateTime.hpp"
using namespace astro;

#include <chrono>
#include <ctime>
#include "date/tz.h"

// static
const std::array<int, 12> DateTime::MONTH_DAYS { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }; // (non-leap years)
const int DateTime::MAX_YEAR = 999999999; //8000;
const int DateTime::MIN_YEAR = -999999999; //-8000;

int monthDays(int month) { return DateTime::MONTH_DAYS[month-1]; }

bool DateTime::isValid(int year, int month, int day, int hour, int minute, double second)
{
  if(year < MIN_YEAR || year > MAX_YEAR)      { return false; }
  else if(month < 1 || month > 12)            { return false; }
  else if(day < 1 || day > monthDays(month))  { return false; }
  else if(hour < 0 || hour >= 24)             { return false; }
  else if(minute < 0 || minute >= 60)         { return false; }
  else if(second < 0 || second >= 60)         { return false; }
  else                                        { return true;  }
}

DateTime DateTime::correctDate(int year, int month, int day, int hour, int minute, double second)
{
  // seconds
  while(second < 0.0)
    { second += 60.0; minute--; }
  while(second >= 60.0)
    { second -= 60.0; minute++; }
  // minutes
  while(minute < 0)
    { minute += 60; hour--; }
  while(minute >= 60)
    { minute -= 60; hour++; }
  // hours
  while(hour < 0)
    { hour += 24; day--; }
  while(hour >= 24)
    { hour -= 24; day++; }
  
  // months (preliminary)
  while(month <= 0) { month += 12; year--; }           // step month to correct range
  while(month > 12) { month -= 12; year++; }
  
  // days (negative)
  while(day <= 0)
    {
      day += monthDays(month==1 ? 12 : month-1);
      month--;
      while(month <= 0)   { month += 12; year--; }     // step month to correct range
    }
  
  // months (final)
  while(month <= 0) { month += 12; year--; }           // step month to correct range
  while(month > 12) { month -= 12; year++; }
  
  // days (positive -- final)
  while(day > monthDays(month))
    {
      day -= monthDays(month);
      month++;
      if(month > 12) { month = month%12; year++; }
    }
  
  // years
  year = std::max(MIN_YEAR, std::min(MAX_YEAR, year)); // clamp year to range

  return DateTime(year, month, day, hour, minute, second);
}

float DateTime::diffDays(const DateTime &other)
{
  DateTime diff = (other - *this);
  float ddays = 0.0f;

  ddays += diff.year()*360.25f;
  ddays += diff.month()*30;
  ddays += diff.day();
  ddays += diff.hour()/24.0f;
  ddays += diff.minute()/24.0f/60.0f;
  ddays += diff.second()/24.0f/60.0f/60.0f;
  
  return ddays;
}

DateTime DateTime::now()
{
  auto current = std::chrono::system_clock::now();
  time_t tt = std::chrono::system_clock::to_time_t(current); // local time_t (rounded to nearest second)
  
  auto rounded = std::chrono::system_clock::from_time_t(tt);
  if(rounded > current)
    {
      tt--;
      rounded -= std::chrono::seconds(1);
    }
  // calculate milliseconds for sub-second time
  int ms = std::chrono::duration_cast<std::chrono::duration<int,std::milli>>(current - rounded).count();
  
  tm *local = localtime(&tt);
  DateTime d(local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min, (double)local->tm_sec + (double)ms / 1000.0);

  auto zoned = date::make_zoned(date::current_zone(), current);
  int offset = zoned.get_info().offset.count();
  d.mUtcOffset = ((double)offset) / 60.0/60.0;
  return d;
}

////////



DateTime::DateTime(int year, int month, int day, int hour, int minute, double second, double utcOffset)
  : mYear(year), mMonth(month), mDay(day), mHour(hour), mMinute(minute), mSecond(second), mUtcOffset(utcOffset)
{
  //fix();
}

DateTime::DateTime(const DateTime &other)
  : mYear(other.mYear), mMonth(other.mMonth), mDay(other.mDay),
    mHour(other.mHour), mMinute(other.mMinute), mSecond(other.mSecond),
    mUtcOffset(other.mUtcOffset), mDstOffset(other.mDstOffset)
{ }

DateTime::DateTime(const std::array<int, 6> &arr)
  : mYear(arr[0]), mMonth(arr[1]), mDay(arr[2]),
    mHour(arr[3]), mMinute(arr[4]), mSecond(arr[5])
{ }

DateTime& DateTime::operator=(const DateTime &other)
{
  mYear      = other.mYear;
  mMonth     = other.mMonth;
  mDay       = other.mDay;
  mHour      = other.mHour;
  mMinute    = other.mMinute;
  mSecond    = other.mSecond;
  mUtcOffset = other.mUtcOffset;
  mDstOffset = other.mDstOffset;
  return *this;
}
    
bool DateTime::valid() const
{
  return isValid(mYear, mMonth, mDay, mHour, mMinute, mSecond);
}

void DateTime::fix()
{
  DateTime dt = correctDate(mYear, mMonth, mDay, mHour, mMinute, mSecond);
  mYear   = dt.year();
  mMonth  = dt.month();
  mDay    = dt.day();
  mHour   = dt.hour();
  mMinute = dt.minute();
  mSecond = dt.second();
}

DateTime DateTime::fixed() const
{
  DateTime dt = *this;
  dt.fix();
  return dt;
}

void DateTime::setYear(double year)
{
  mYear = (int)year;
  double remaining = year - mYear;
  if(remaining > 0.0) { setDay(mDay + remaining*365.25); }
}
void DateTime::setMonth(double month)
{
  mMonth = (int)month;
  double remaining = month - mMonth;
  if(remaining > 0.0) { setDay(mDay + remaining*monthDays(mMonth)); }
}
void DateTime::setDay(double day)
{
  mDay = (int)day;
  double remaining = day - mDay;
  if(remaining > 0.0) { setHour(mHour + remaining*24.0); }
}
void DateTime::setHour(double hour)
{
  mHour = (int)hour;
  double remaining = hour - mHour;
  if(remaining > 0.0) { setMinute(mMinute + remaining*60.0); }
}
void DateTime::setMinute(double minute)
{
  mMinute = (int)minute;
  double remaining = minute - mMinute;
  if(remaining > 0.0) { setSecond(mSecond + remaining*60.0); }
}
void DateTime::setSecond(double second)
{ mSecond = second; }
void DateTime::setUtcOffset(double offset)
{ mUtcOffset = offset; }
void DateTime::setDstOffset(double offset)
{ mDstOffset = offset; }

bool DateTime::set(int year, int month, int day, int hour, int minute, double second)
{
  if(isValid(year, month, day, hour, minute, second))
    {
      mYear   = year;
      mMonth  = month;
      mDay    = day;
      mHour   = hour;
      mMinute = minute;
      mSecond = second;
      return true;
    }
  else
    {
      DateTime correct = correctDate(year, month, day, hour, minute, second);
      mYear   = correct.mYear;
      mMonth  = correct.mMonth;
      mDay    = correct.mDay;
      mHour   = correct.mHour;
      mMinute = correct.mMinute;
      mSecond = correct.mSecond;
      return true;
    }
}
