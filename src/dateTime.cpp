#include "dateTime.hpp"
using namespace astro;

#include <chrono>
#include <ctime>

// static
const std::array<int, 12> DateTime::MONTH_DAYS { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }; // (non-leap years)

int monthDays(int month)
{ return DateTime::MONTH_DAYS[month-1]; }

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
  
  return d;
}

////////



DateTime::DateTime(int year, int month, int day, int hour, int minute, double second, double tz)
  : mYear(year), mMonth(month), mDay(day), mHour(hour), mMinute(minute), mSecond(second), mTzOffset(tz)
{
  //fix();
}

DateTime::DateTime(const DateTime &other)
  : mYear(other.mYear), mMonth(other.mMonth), mDay(other.mDay),
    mHour(other.mHour), mMinute(other.mMinute), mSecond(other.mSecond),
    mTzOffset(other.mTzOffset)
{ }

DateTime::DateTime(const std::array<int, 6> &arr)
  : mYear(arr[0]), mMonth(arr[1]), mDay(arr[2]),
    mHour(arr[3]), mMinute(arr[4]), mSecond(arr[5])
{ }

DateTime& DateTime::operator=(const DateTime &other)
{
  mYear   = other.mYear;
  mMonth  = other.mMonth;
  mDay    = other.mDay;
  mHour   = other.mHour;
  mMinute = other.mMinute;
  mSecond = other.mSecond;
  mTzOffset = other.mTzOffset;
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

void DateTime::setYear(int year)
{ mYear = year; }
void DateTime::setMonth(int month)
{ mMonth = month; }
void DateTime::setDay(int day)
{ mDay = day; }
void DateTime::setHour(int hour)
{ mHour = hour; }
void DateTime::setMinute(int minute)
{ mMinute = minute; }
void DateTime::setSecond(int second)
{ mSecond = second; }
void DateTime::setTzOffset(double offset)
{ mTzOffset = offset; }

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
