#ifndef COMPARE_NODE_HPP
#define COMPARE_NODE_HPP

#include "astro.hpp"
#include "chartView.hpp"
#include "node.hpp"
#include "chartCompare.hpp"

namespace astro
{
  //// node connector indices ////
  // inputs
#define COMPARENODE_INPUT_CHART_INNER 0
#define COMPARENODE_INPUT_CHART_OUTER 1
  // outputs
  ////////////////////////////////
  
  class CompareNode : public Node
  {
  private:
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return {new Connector<Chart>("Inner Chart"), new Connector<Chart>("Outer Chart")}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return {}; }

    ChartView mView;
    
    ChartCompare *mCompare = nullptr;

    DateTime mDateOuter;
    DateTime mDateInner;
    Location mLocOuter;
    Location mLocInner;
    
    float mChartWidth = CHART_SIZE_DEFAULT;

    virtual bool onDraw() override;
    
    virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const override
    {
      params.emplace("width", std::to_string(mChartWidth));
      return params;
    };
    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params) override
    {
      std::stringstream ss(params["width"]);
      ss >> mChartWidth;
      return params;
    };
    
  public:
    CompareNode();
    ~CompareNode();
    virtual std::string type() const { return "ChartCompareNode"; }
    virtual bool copyTo(Node *other) override
    { // copy settings
      if(Node::copyTo(other))
        {
          ((CompareNode*)other)->mChartWidth = mChartWidth;
          // (everything else set by connections)
          return true;
        }
      else { return false; }
    }

    Chart* outerChart() { return mCompare->getOuterChart(); }
    Chart* innerChart() { return mCompare->getInnerChart(); }
  };
}

#endif // COMPARE_NODE_HPP
