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

  // DateTime -- defines a point in time (date and time of day) //
  class DateTime
  {
  private:
    int mYear      = 1;
    int mMonth     = 1;
    int mDay       = 1;
    int mHour      = 1;
    int mMinute    = 0;
    double mSecond = 0.0;
    double mTzOffset = 0.0;
    
  public:
    //// STATIC ////
    static constexpr int MAX_YEAR = 8000;
    static constexpr int MIN_YEAR = -8000;
    static const std::array<int, 12> MONTH_DAYS;
    static DateTime now();
    static bool isValid(int year, int month, int day, int hour, int minute, double second);         // returns whether given date is valid
    static DateTime correctDate(int year, int month, int day, int hour, int minute, double second); // returns corrected date
    
    DateTime() { }
    DateTime(const std::string &saveStr) { fromSaveString(saveStr); }
    DateTime(int year, int month, int day, int hour, int minute, double second, double tz=0.0);
    DateTime(const DateTime &other);
    DateTime(const std::array<int, 6> &arr);
    DateTime& operator=(const DateTime &other);

    bool valid() const;
    void fix();
    DateTime fixed() const;
    
    std::string toString() const
    {
      std::ostringstream os;
      os << (*this);
      return os.str();
    }
    std::string toSaveString() const
    {
      std::ostringstream os;
      os << mYear << " " << mMonth << " " << mDay << " " << mHour << " " << mMinute << " " << mSecond << " " << mTzOffset;
      return os.str();
    }
    std::string fromSaveString(const std::string &str)
    {
      std::istringstream is(str);
      std::string name;
      is >> mYear; is >> mMonth; is >> mDay; is >> mHour; is >> mMinute; is >> mSecond; is >> mTzOffset;
      // return remaining string
      std::stringstream tmp; tmp << is.rdbuf();
      return tmp.str();
    }
    
    void setYear(int year);
    void setMonth(int month);
    void setDay(int day);
    void setHour(int hour);
    void setMinute(int minute);
    void setSecond(int second);
    void setTzOffset(double offset);
    
    bool set(int year, int month, int day, int hour, int minute, double second);

    int year() const      { return mYear;   }
    int month() const     { return mMonth;  }
    int day() const       { return mDay;    }
    int hour() const      { return mHour;   }
    int minute() const    { return mMinute; }
    double second() const { return mSecond; }
    double tzOffset() const { return mTzOffset; }

    bool operator==(const DateTime &other) const
    { return (mYear == other.mYear && mMonth == other.mMonth && mDay == other.mDay &&
              mHour == other.mHour && mMinute == other.mMinute && mSecond == other.mSecond &&
              mTzOffset == other.mTzOffset); }
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
    { return (*this < other || *this == other); }
    bool operator>=(const DateTime &other) const
    { return (*this > other || *this == other); }

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
    //friend std::istream& operator>>(std::istream &is, DateTime &date);
  };
  
  inline std::ostream& operator<<(std::ostream &os, const DateTime &date)
  {
    date.printDate(os);
    os << "  |  ";
    date.printTime(os);    
    //os << "[" << date.mYear << "/" << date.mMonth << "/" << date.mDay << " | " << date.mHour << ":" << date.mMinute << ":" << date.mSecond << "]";
    return os;
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
