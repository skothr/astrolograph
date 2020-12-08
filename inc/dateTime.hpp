#ifndef DATETIME_HPP
#define DATETIME_HPP

#include <array>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <type_traits>

#include "astro.hpp"

namespace astro
{
  static const std::vector<std::string> MONTH_NAMES =
    { "January", "February", "March",     "April",   "May",      "June",
      "July",    "August",   "September", "October", "November", "December"};
  static const std::vector<std::string> WEEK_NAMES =
    { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

  // DateTime -- defines a point in time (date and time of day) //
  class DateTime
  {
  private:
    int mYear         = 1;
    int mMonth        = 1;
    int mDay          = 1;
    int mHour         = 1;
    int mMinute       = 0;
    double mSecond    = 0.0;
    double mUtcOffset = 0.0;
    bool mDstOffset   = false;
    
  public:
    //// STATIC ////
    static const int MAX_YEAR;
    static const int MIN_YEAR;
    static const std::array<int, 12> MONTH_DAYS;
    static DateTime now();
    static bool isValid(int year, int month, int day, int hour, int minute, double second);         // returns whether given date is valid
    static DateTime correctDate(int year, int month, int day, int hour, int minute, double second); // returns corrected date
    
    float diffDays(const DateTime &other);
    
    DateTime() { }
    DateTime(const std::string &saveStr) { fromSaveString(saveStr); }
    DateTime(int year, int month, int day, int hour, int minute, double second, double utcOffset=0.0);
    DateTime(const DateTime &other);
    DateTime(const std::array<int, 6> &arr);
    DateTime& operator=(const DateTime &other);

    bool valid() const;
    void fix();
    DateTime fixed() const;
    
    std::string toString(bool pdate=true, bool ptime=true) const
    {
      std::ostringstream os;
      if(pdate)          { printDate(os); }
      if(pdate && ptime) { os << " | "; }
      if(ptime)          { printTime(os); }
      return os.str();
    }
    std::string toSaveString() const
    {
      std::ostringstream os;
      os << mYear << " " << mMonth << " " << mDay << " " << mHour << " " << mMinute << " " << mSecond << " " << mUtcOffset << " " << mDstOffset;
      return os.str();
    }
    std::string fromSaveString(const std::string &str)
    {
      std::istringstream is(str);
      std::string name;
      is >> mYear; is >> mMonth; is >> mDay; is >> mHour; is >> mMinute; is >> mSecond; is >> mUtcOffset; is >> mDstOffset;
      // return remaining string
      std::stringstream tmp; tmp << is.rdbuf();
      return tmp.str();
    }
    
    void setYear(double year);
    void setMonth(double month);
    void setDay(double day);
    void setHour(double hour);
    void setMinute(double minute);
    void setSecond(double second);
    void setUtcOffset(double offset);
    void setDstOffset(bool dst);
    
    bool set(int year, int month, int day, int hour, int minute, double second);

    int year() const         { return mYear;   }
    int month() const        { return mMonth;  }
    int day() const          { return mDay;    }
    int hour() const         { return mHour;   }
    int minute() const       { return mMinute; }
    double second() const    { return mSecond; }
    double utcOffset() const { return mUtcOffset; }
    bool dstOffset() const   { return mDstOffset; }

    bool operator==(const DateTime &other) const
    { return (mYear == other.mYear && mMonth == other.mMonth && mDay == other.mDay &&
              mHour == other.mHour && mMinute == other.mMinute && mSecond == other.mSecond &&
              mUtcOffset == other.mUtcOffset && mDstOffset == other.mDstOffset); }
    bool operator!=(const DateTime &other) const
    { return !(*this == other); }
    bool operator<(const DateTime &other) const
    {return (mYear < other.mYear ||
             (mYear == other.mYear && mMonth < other.mMonth) ||
             (mYear == other.mYear && mMonth == other.mMonth && mDay < other.mDay) ||
             (mYear == other.mYear && mMonth == other.mMonth && mDay == other.mDay && mHour < other.mHour) ||
             (mYear == other.mYear && mMonth == other.mMonth && mDay == other.mDay && mHour == other.mHour && mMinute < other.mMinute) ||
             (mYear == other.mYear && mMonth == other.mMonth && mDay == other.mDay && mHour == other.mHour && mMinute == other.mMinute && mSecond < other.mSecond));}
    bool operator>(const DateTime &other) const
    {return (mYear > other.mYear ||
             (mYear == other.mYear && mMonth > other.mMonth) ||
             (mYear == other.mYear && mMonth == other.mMonth && mDay > other.mDay) ||
             (mYear == other.mYear && mMonth == other.mMonth && mDay == other.mDay && mHour > other.mHour) ||
             (mYear == other.mYear && mMonth == other.mMonth && mDay == other.mDay && mHour == other.mHour && mMinute > other.mMinute) ||
             (mYear == other.mYear && mMonth == other.mMonth && mDay == other.mDay && mHour == other.mHour && mMinute == other.mMinute && mSecond > other.mSecond));}
    bool operator<=(const DateTime &other) const
    { return ((*this < other) || (*this == other)); }
    bool operator>=(const DateTime &other) const
    { return ((*this > other) || (*this == other)); }

    DateTime& operator+=(const DateTime &other)
    {
      mYear += other.mYear; mMonth  += other.mMonth;  mDay    += other.mDay;
      mHour += other.mHour; mMinute += other.mMinute; mSecond += other.mSecond;
      fix(); return *this;
    }
    DateTime& operator-=(const DateTime &other)
    {
      mYear -= other.mYear; mMonth  -= other.mMonth;  mDay    -= other.mDay;
      mHour -= other.mHour; mMinute -= other.mMinute; mSecond -= other.mSecond;
      fix(); return *this;
    }
    DateTime operator+(const DateTime &other) const
    {
      DateTime dt(mYear+other.mYear, mMonth+other.mMonth,   mDay+other.mDay,
                  mHour+other.mHour, mMinute+other.mMinute, mSecond+other.mSecond);
      dt.fix(); return dt+=other;
    }
    DateTime operator-(const DateTime &other) const
    {
      DateTime dt(mYear, mMonth,  mDay,
                  mHour, mMinute, mSecond);
      dt.fix(); return dt-=other;
    }

    DateTime& operator*=(double scalar)
    {
      mYear *= scalar; mMonth  *= scalar; mDay    *= scalar;
      mHour *= scalar; mMinute *= scalar; mSecond *= scalar;
      fix(); return *this;
    }
    DateTime& operator/=(double scalar)
    {
      // convert to seconds
      double s = mSecond;
      s += mMinute * 60.0;
      s += mHour   * 60.0*60.0;
      s += mDay    * 60.0*60.0*24.0;
      s += mMonth  * 60.0*60.0*24.0*30.0;
      s += mYear   * 60.0*60.0*24.0*365.0;
      
      s /= scalar;

      mYear = 0;
      mMonth = 0;
      mDay = 0;
      mHour = 0;
      mMinute = 0;
      mSecond = s;
      
      fix(); return *this;
    }
    DateTime operator*(double scalar) const
    {
      DateTime dt(mYear, mMonth,  mDay, mHour, mMinute, mSecond);
      return dt*=scalar;
    }
    DateTime operator/(double scalar) const
    {
      DateTime dt(mYear, mMonth,  mDay, mHour, mMinute, mSecond);
      return dt/=scalar;
    }

    void printDate(std::ostream &os) const
    { os << MONTH_NAMES[mMonth-1] << " " << mDay << ", " << mYear; }
    void printTime(std::ostream &os) const
    {
      int hour12 = mHour % 12;
      hour12 = (hour12 == 0 ? 12 : hour12);
      bool am = (mHour < 12);
      os << (hour12 < 10 ? "0" : "") << hour12 << ":" << (mMinute < 10 ? "0" : "") << mMinute
         << ":" << (mSecond < 10 ? "0" : "") << (int)mSecond << (am ? " AM" : " PM");
    }

    friend std::ostream& operator<<(std::ostream &os, const DateTime &date);
    friend std::istream& operator>>(std::istream &is, DateTime &date);
  };
  
  inline std::ostream& operator<<(std::ostream &os, const DateTime &date)
  {
    os << date.toSaveString();
    // date.printDate(os); os << " | "; date.printTime(os);
    return os;
  }

  inline std::istream& operator>>(std::istream &is, DateTime &date)
  {
    int year, month, day, hour, minute;
    double second, utcOffset;
    bool dstOffset;
    is >> year; is >> month; is >> day; is >> hour; is >> minute; is >> second; is >> utcOffset; is >> dstOffset;
    date.set(year, month, day, hour, minute, second);
    date.setUtcOffset(utcOffset);
    date.setDstOffset(dstOffset);
    return is;
  }
  
  // treat each year as a day
  inline DateTime progressed(const DateTime &natal, const DateTime &transit)
  {
    // transit-natal differences
    DateTime diff = transit - natal;
    // start with natal date
    DateTime pdt = natal;
    // offset progressed
    pdt += diff/365.25;
    return pdt;
  }
}


#endif // DATETIME_HPP
