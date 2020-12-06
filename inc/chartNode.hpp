#ifndef CHART_NODE_HPP
#define CHART_NODE_HPP

#include "astro.hpp"
#include "chart.hpp"
#include "node.hpp"

namespace astro
{
  // forward declarations
  class SettingForm;

  //// node connector indices ////
  // inputs
#define CHARTNODE_INPUT_DATE        0
#define CHARTNODE_INPUT_LOCATION    1
  // outputs
#define CHARTNODE_OUTPUT_CHART      0
  ////////////////////////////////

  class ChartNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()
    { return {new Connector<DateTime>("Date"), new Connector<Location>("Location")}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS()
    { return {new Connector<Chart>("Chart")}; }
    
    Chart        *mChart = nullptr;
    SettingForm *mSettings = nullptr;
    
    bool mTruePos = false; // 
    std::vector<std::string> mZNames;
    std::vector<std::string> mHsNames;
    int mHouseSystem = 0;  // combo index
    int mZodiac      = ZODIAC_TROPICAL; // combo index

    // bool mOptionsOpen = false;
    virtual void onUpdate() override;
    virtual void onDraw() override;
    
    virtual void getSaveParams(std::map<std::string, std::string> &params) const override;
    virtual void setSaveParams(std::map<std::string, std::string> &params) override;    
  public:
    ChartNode(Chart *chart=nullptr);
    ChartNode(const DateTime &dt, const Location &loc);
    ~ChartNode();
    virtual std::string type() const { return "ChartNode"; }
    
    virtual bool onConnect(ConnectorBase *con) override;
    
    virtual bool copyTo(Node *other) override
    { // copy settings
      if(Node::copyTo(other))
        {
          // set date and location if inputs not connected
          if(!inputs()[CHARTNODE_INPUT_DATE]->get<DateTime>())
            { ((ChartNode*)other)->mChart->setDate(mChart->date()); }
          else
            { ((ChartNode*)other)->mChart->setDate(*inputs()[CHARTNODE_INPUT_DATE]->get<DateTime>()); }
          if(!inputs()[CHARTNODE_INPUT_LOCATION]->get<Location>())
            { ((ChartNode*)other)->mChart->setLocation(mChart->location()); }
          else
            { ((ChartNode*)other)->mChart->setLocation(*inputs()[CHARTNODE_INPUT_LOCATION]->get<Location>()); }
          
          ((ChartNode*)other)->mChart->setHouseSystem(mChart->getHouseSystem());
          ((ChartNode*)other)->mChart->setZodiac(mChart->getZodiac());
          // (everything else set by connections)
          return true;
        }
      else { return false; }
    }
    
    Chart* chart() { return mChart; }
  };
}

#endif // CHART_NODE_HPP
