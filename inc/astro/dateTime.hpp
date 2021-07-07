#ifndef DATETIME_HPP
#define DATETIME_HPP

#include <array>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <type_traits>

#include "tools.hpp"
//#include "astro.hpp"

namespace astro
{
  static const std::vector<std::string> MONTH_NAMES = { "January", "February", "March",     "April",   "May",      "June",
                                                        "July",    "August",   "September", "October", "November", "December" };
  static const std::vector<std::string> WEEK_NAMES = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

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
    
    double diffDays(const DateTime &other) const;
    
    DateTime() { }
    DateTime(const std::string &saveStr) { std::istringstream is(saveStr); is >> *this; }//fromSaveString(saveStr); }
    DateTime(int year, int month, int day, int hour, int minute, double second, double utcOffset=0.0, double dstOffset=0.0);
    DateTime(const DateTime &other);
    DateTime(const std::array<int, 6> &arr);
    DateTime& operator=(const DateTime &other);

    bool valid() const;
    void fix();
    DateTime fixed() const;
    
    std::string toShortString(bool pdate=true, bool ptime=true, bool pseconds=true) const
    {
      std::ostringstream os;
      if(pdate)          { os << std::setw(2) << std::setfill('0') << mMonth << "/"
                              << std::setw(2) << std::setfill('0') << mDay << "/"
                              << std::setw(4) << std::setfill('0') << mYear; }
      if(pdate && ptime) { os << " | "; }
      if(ptime)          { os << std::setw(2) << std::setfill('0') << mHour << ":" << std::setw(2) << std::setfill('0') << mMinute; }
      if(pseconds)       { os << ":" << std::setw(2) << std::setfill('0') << mSecond; }
      
      return os.str();
    }
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
      is >> mYear; is >> mMonth; is >> mDay; is >> mHour; is >> mMinute; is >> mSecond; is >> mUtcOffset; is >> mDstOffset;
      // return remaining string
      std::stringstream tmp; tmp << is.rdbuf();
      return tmp.str();
    }

    static int monthDays(int month, int year);
    static std::string monthName(int month);
    static std::string monthNameAbbrev(int month);

    int daysInYear() const  { return (mYear % 4 == 0) ? 366 : 365; }
    int daysInMonth() const { return monthDays(mMonth, mYear); }
    int toTimestamp() const;
    
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
    { // TODO: operator+ necessary/meaningful?
      mYear  += other.mYear;
      mMonth += other.mMonth;  mDay    += other.mDay;
      mHour  += other.mHour; mMinute += other.mMinute; mSecond += other.mSecond;
      fix(); return *this;
    }
    DateTime& operator-=(const DateTime &other)
    {
      // mYear -= other.mYear;
      // mMonth -= other.mMonth;
      // mDay   -= other.mDay;
      // mHour  -= other.mHour; mMinute -= other.mMinute; mSecond -= other.mSecond;
      double ddays = diffDays(other);
      mDay    -= (int)ddays; ddays -= (int)ddays; ddays *= 24.0;
      mHour   -= (int)ddays; ddays -= (int)ddays; ddays *= 60.0;
      mMinute -= (int)ddays; ddays -= (int)ddays; ddays *= 60.0;
      mSecond -= (int)ddays; ddays -= (int)ddays;
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
      DateTime dt(mYear, mMonth,  mDay, mHour, mMinute, mSecond);
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
    { if(mMonth > 0 && mMonth <= 12) { os << MONTH_NAMES[mMonth-1]; } os << " " << mDay << ", " << mYear; }
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
    // os << date.toSaveString();
    os << std::fixed << std::setprecision(2);
    os << std::setw(2) << std::right << std::setfill('0') << date.mMonth << "/"
       << std::setw(2) << std::right << std::setfill('0') << date.mDay << "/"
       << std::setw(4) << std::right << std::setfill('0') << date.mYear << " "
       << std::setw(2) << std::right << std::setfill('0') << date.mHour << ":"
       << std::setw(2) << std::right << std::setfill('0') << date.mMinute << ":"
       << std::setw(5) << std::right << std::setfill('0') << date.mSecond << " "
       << date.mUtcOffset << " " << date.mDstOffset;
    os << std::setprecision(6) << std::setfill(' '); // reset precision/fill
    return os;
  }

  inline std::istream& operator>>(std::istream &is, DateTime &date)
  {
    // int year, month, day, hour, minute;
    //double second, utcOffset;
    //bool dstOffset;
    // is >> year; is >> month; is >> day; is >> hour; is >> minute; is >> second; is >> utcOffset; is >> dstOffset;
    // date.set(year, month, day, hour, minute, second);
    // date.setUtcOffset(utcOffset);
    // date.setDstOffset(dstOffset);

    // tokenize
    std::string dateArg = ""; is >> dateArg;
    std::string timeArg = ""; is >> timeArg;
    std::string utcArg  = ""; is >> utcArg;
    std::string dstArg  = ""; is >> dstArg;
    // parse date 'MM/DD/YYYY'
    std::stringstream ss(dateArg);
    std::string line = "";
    int i = 0; // 0 --> month(int), 1 --> day(int), 2 --> year(int)
    while(std::getline(ss, line, '/'))
      {
        // get value
        std::stringstream iss(line);
        int val; iss >> val;
        // set date
        if(i == 0)      { date.setMonth(val); }
        else if(i == 1) { date.setDay  (val); }
        else if(i == 2) { date.setYear (val); }
        else { return is; } i++;
      }
    // parse time 'HH:MM:SS.SSS'
    ss.clear();
    ss.str(timeArg);
    line = "";
    i = 0; // 0 --> hour(int), 1 --> minute(int), 2 --> second(double)
    while(std::getline(ss, line, ':'))
      {
        // get value
        std::stringstream iss(line);
        // set date
        if(i == 0)      { int    val; iss >> val; date.setHour(val);   }
        else if(i == 1) { int    val; iss >> val; date.setMinute(val); }
        else if(i == 2) { double val; iss >> val; date.setSecond(val); }
        else { return is; } i++;
      }

    ss.clear(); ss.str(utcArg);
    ss >> date.mUtcOffset;
    ss.clear(); ss.str(dstArg);
    ss >> date.mDstOffset;
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
