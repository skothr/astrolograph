#ifndef CHART_VIEW_NODE_HPP
#define CHART_VIEW_NODE_HPP

#include "astro.hpp"
#include "chartView.hpp"
#include "node.hpp"

namespace astro
{
  //// node connector indices ////
  // inputs
#define CHARTVIEWNODE_INPUT_CHART        0
  // outputs
#define CHARTVIEWNODE_OUTPUT_CHART       0
  ////////////////////////////////

  class ChartViewNode : public Node
  {
  private:
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()
    { return {new Connector<Chart>("Chart Input")}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS()
    { return {}; }//new Connector<Chart>("Chart Ouput (Copy)")}; }

    ChartView mView;
    float mChartWidth = (CHART_SIZE_MIN + CHART_SIZE_DEFAULT)/2.0f; // start halfways between minimum size and "default"

    ChartParams mParams;
    bool mOptionsOpen = false;
    bool mDisplayOpen = false;
    bool mOrbsOpen    = false;

    bool mAlignAsc    = false; // align chart so ascendant points to the left
    bool mShowHouses  = true;  // show houses on chart
    
    // date modify flags (not saved)
    bool mEditYear   = false; // toggled with 1 key
    bool mEditMonth  = false; // toggled with 2 key
    bool mEditDay    = false; // toggled with 3 key
    bool mEditHour   = false; // toggled with 4 key
    bool mEditMinute = false; // toggled with 5 key
    bool mEditSecond = false; // toggled with 6 key
    // location modify flags
    bool mEditLat    = false; // toggled with Q key
    bool mEditLon    = false; // toggled with W key
    bool mEditAlt    = false; // toggled with E key

    virtual void onUpdate() override;
    virtual void onDraw() override;
    
    virtual void getSaveParams(std::map<std::string, std::string> &params) const override
    {
      params.emplace("width",       std::to_string(mChartWidth));
      params.emplace("optionsOpen", (mOptionsOpen ? "1" : "0"));
      params.emplace("displayOpen", (mDisplayOpen ? "1" : "0"));
      params.emplace("orbsOpen",    (mOrbsOpen    ? "1" : "0"));
      params.emplace("alignAsc",    (mAlignAsc    ? "1" : "0"));
      params.emplace("showHouses",  (mShowHouses  ? "1" : "0"));
      std::string visStr = "";
      std::string orbStr = "";
      for(int i = OBJ_SUN; i < OBJ_END; i++)
        {
          visStr.append(mParams.objVisible[i] ? "1" : "0");
          orbStr.append(to_string(mParams.objOrbs[i], 6) + ((i != OBJ_END-1) ? "|" : ""));
        }
      params.emplace("objVisible",  visStr);
      params.emplace("objOrbs",     orbStr);
      visStr = "";
      orbStr = "";
      for(int i = 0; i < ASPECT_COUNT; i++)
        {
          visStr.append(mParams.aspVisible[i] ? "1" : "0");
          orbStr.append(to_string(mParams.aspOrbs[i], 6) + ((i != ASPECT_COUNT-1) ? "|" : ""));
        }
      params.emplace("aspVisible",  visStr);
      params.emplace("aspOrbs",     orbStr);
    };
    
    virtual void setSaveParams(std::map<std::string, std::string> &params) override
    {
      auto iter = params.find("width");
      if(iter != params.end()) { std::istringstream(iter->second) >> mChartWidth; }
      iter = params.find("optionsOpen");
      if(iter != params.end()) { mOptionsOpen = (iter->second != "0"); }
      iter = params.find("displayOpen");
      if(iter != params.end()) { mDisplayOpen = (iter->second != "0"); }
      iter = params.find("orbsOpen");
      if(iter != params.end()) { mOrbsOpen = (iter->second != "0"); }
      iter = params.find("alignAsc");
      if(iter != params.end()) { mAlignAsc = (iter->second != "0"); }
      iter = params.find("showHouses");
      if(iter != params.end()) { mShowHouses = (iter->second != "0"); }

      iter = params.find("objVisible");
      if(iter != params.end())
        { 
          std::string visStr = iter->second;
          for(int i = OBJ_SUN; i < OBJ_END && i < visStr.size(); i++)
            { mParams.objVisible[i] = (visStr[i] != '0'); }
        }
      iter = params.find("objOrbs");
      if(iter != params.end())
        {
          int i = 0;
          std::string orbStr;
          std::stringstream ss(iter->second);
          while(std::getline(ss, orbStr, '|'))
            {
              std::istringstream is(orbStr);
              is >> mParams.objOrbs[i++];
            }
        }

      iter = params.find("aspVisible");
      if(iter != params.end())
        { 
          std::string visStr = iter->second;
          for(int i = 0; i < ASPECT_COUNT && i < visStr.size(); i++)
            { mParams.aspVisible[i] = (visStr[i] != '0'); }
        }
      iter = params.find("aspOrbs");
      if(iter != params.end())
        {
          int i = 0;
          std::string orbStr;
          std::stringstream ss(iter->second);
          while(std::getline(ss, orbStr, '|'))
            {
              std::istringstream is(orbStr);
              is >> mParams.aspOrbs[i++];
            }
        }
    };
    
  public:
    ChartViewNode();
    virtual std::string type() const { return "ChartViewNode"; }
    virtual bool copyTo(Node *other) override
    { // copy settings
      if(Node::copyTo(other))
        {
          ((ChartViewNode*)other)->mChartWidth  = mChartWidth;
          ((ChartViewNode*)other)->mOptionsOpen = mOptionsOpen;
          ((ChartViewNode*)other)->mDisplayOpen = mDisplayOpen;
          ((ChartViewNode*)other)->mOrbsOpen    = mOrbsOpen;
          ((ChartViewNode*)other)->mAlignAsc    = mAlignAsc;
          ((ChartViewNode*)other)->mShowHouses  = mShowHouses;
          
          ((ChartViewNode*)other)->mView.setAlignAsc(mAlignAsc);
          ((ChartViewNode*)other)->mView.setShowHouses(mShowHouses);

          for(int i = OBJ_SUN; i < OBJ_END; i++)
            {
              ((ChartViewNode*)other)->mParams.objVisible[i] = mParams.objVisible[i];
              ((ChartViewNode*)other)->mParams.objOrbs[i]    = mParams.objOrbs[i];
            }
          for(int i = 0; i < ASPECT_COUNT; i++)
            {
              ((ChartViewNode*)other)->mParams.aspVisible[i] = mParams.aspVisible[i];
              ((ChartViewNode*)other)->mParams.aspOrbs[i]    = mParams.aspOrbs[i];
            }
          
          // (everything else set by connections)
          return true;
        }
      else { return false; }
    }

    void processInput(Chart *chart);
  };
}

#endif // CHART_VIEW_NODE_HPP
