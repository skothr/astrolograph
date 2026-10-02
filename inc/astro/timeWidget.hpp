#ifndef TIME_WIDGET_HPP
#define TIME_WIDGET_HPP

#include "astro.hpp"

// forward declarations
class NodeGraph;
class FileDialog;

namespace astro
{
  
#define DATE_SAVE_DIR "./dates"
#define DATE_NAME_BUFLEN 128
  
  struct DateSave
  {
    std::string name;
    DateTime date;
  };

  class TimeWidget
  {
  private:
    NodeGraph  *mGraph      = nullptr;
    FileDialog *mFileDialog = nullptr;
    DateTime mDate;
    DateTime mSavedDate;
    std::string mName = "";
    //bool mDST = false; // daylight savings time
    bool saveDirCheck();
    
  public:
    TimeWidget();
    TimeWidget(const DateTime &date);
    TimeWidget(const TimeWidget &other);
    TimeWidget& operator=(const TimeWidget &other);
    ~TimeWidget();
    
    DateTime& get()                     { return mDate; }
    const DateTime& get() const         { return mDate; }
    void set(const DateTime &date)      { mDate = date; }
    DateTime& getSaved()                { return mSavedDate; }
    const DateTime& getSaved() const    { return mSavedDate; }
    void setSaved(const DateTime &date) { mSavedDate = date; }
    
    const std::string& getName() const { return mName; }
    std::string& getName()             { return mName; }
    void setName(const std::string &n) { mName = n; }

    void setGraph(NodeGraph *graph) { mGraph = graph; }
    
    bool checkFileDialog();
    bool draw(const std::string &id, float scale, bool blocked);
  };
}

#endif // TIME_WIDGET_HPP
