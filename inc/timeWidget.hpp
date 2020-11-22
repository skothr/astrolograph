#ifndef TIME_WIDGET_HPP
#define TIME_WIDGET_HPP

#include "astro.hpp"


namespace astro
{
#define DATE_SAVE_DIR "./saved/"
#define DATE_SAVE_PATH DATE_SAVE_DIR "dates.txt"
#define DATE_NAME_BUFLEN 128

  struct DateSave
  {
    std::string name;
    DateTime date;
  };

  class TimeWidget
  {
  private:
    DateTime mDate;
    DateTime mSavedDate;
    char mName[DATE_NAME_BUFLEN] = "";
    char mSavedName[DATE_NAME_BUFLEN] = "";
    bool mDST = false; // daylight savings time
    
  public:
    TimeWidget();
    TimeWidget(const DateTime &date);
    TimeWidget(const TimeWidget &other);
    TimeWidget& operator=(const TimeWidget &other);
    
    DateTime& get()                     { return mDate; }
    const DateTime& get() const         { return mDate; }
    void set(const DateTime &date)      { mDate = date; }
    DateTime& getSaved()                { return mSavedDate; }
    const DateTime& getSaved() const    { return mSavedDate; }
    void setSaved(const DateTime &date) { mSavedDate = date; }
    
    std::string getName() const { return mName; }
    void setName(const std::string &n) { sprintf(mName, "%s", n.c_str()); }
    
    bool save(const std::string &name);
    bool load(const std::string &name);
    bool remove(const std::string &name);
    bool reload() { mDate = mSavedDate; return true; }
    std::vector<DateSave> loadAll();
    
    void draw(const std::string &id, float scale, bool blocked);
  };
}

#endif // TIME_WIDGET_HPP
