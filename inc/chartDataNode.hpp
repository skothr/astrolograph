#ifndef CHART_DATA_NODE_HPP
#define CHART_DATA_NODE_HPP

#include "astro.hpp"
#include "chart.hpp"
#include "node.hpp"

namespace astro
{
  //// node connector indices ////
  // inputs
#define DATANODE_INPUT_CHART 0
  // outputs
  ////////////////////////////////

  class ChartDataNode : public Node
  {
  private:
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()
    { return {new Connector<Chart>("Chart")}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS()
    { return {}; }
    
    bool mAngOpen = true;
    bool mObjOpen = true;
    bool mOrbOpen = false;

    std::vector<bool> mShowObjects;
    std::vector<bool> mFocusObjects;
    
    virtual bool onDraw() override;
    
    virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const override
    {
      // save object visibility
      std::string showObjs = "";
      for(auto show : mShowObjects)
        { showObjs += (show ? "1" : "0"); }
      params.emplace("showObjs", showObjs);
      return params;
    };
    
    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params) override
    {
      Chart *chart = inputs()[DATANODE_INPUT_CHART]->get<Chart>();
      auto iter = params.find("showObjs");
      if(iter != params.end())
        {
          for(int o = OBJ_SUN; o < OBJ_COUNT; o++)
            {
              if(iter->second.size() < o) { break; }
              else
                {
                  mShowObjects[o] = (iter->second[o] == '0');
                  if(chart) { chart->getObject((ObjType)o)->visible = mShowObjects[o]; }
                }
            }
          for(int a = ANGLE_OFFSET; a < ANGLE_END; a++)
            {
              if(iter->second.size() < a) { break; }
              else
                {
                  mShowObjects[a] = (iter->second[a] == '0');
                  if(chart) { chart->getObject((ObjType)a)->visible = mShowObjects[a]; }
                }
            }
        }
      // mChart->update();
      return params;
    };

    
  public:
    ChartDataNode();
    ~ChartDataNode();
    virtual std::string type() const { return "ChartDataNode"; }
    virtual bool copyTo(Node *other) override
    { // copy settings
      if(Node::copyTo(other))
        {
          ((ChartDataNode*)other)->mAngOpen = mAngOpen;
          ((ChartDataNode*)other)->mObjOpen = mObjOpen;
          ((ChartDataNode*)other)->mOrbOpen = mOrbOpen;
          for(int o = OBJ_SUN; o < OBJ_COUNT; o++) // start with objects visible
            { ((ChartDataNode*)other)->mShowObjects[o] = mShowObjects[o]; }
          for(int a = ANGLE_OFFSET; a < ANGLE_END; a++) // start with angles invisible
            { ((ChartDataNode*)other)->mShowObjects[OBJ_COUNT+a-ANGLE_OFFSET] = mShowObjects[OBJ_COUNT+a-ANGLE_OFFSET]; }
          // (everything else set by connections)
          return true;
        }
      else { return false; }
    }
    
  };
}

#endif // CHART_DATA_NODE_HPP
