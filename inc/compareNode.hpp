#ifndef COMPARE_NODE_HPP
#define COMPARE_NODE_HPP

#include "astro.hpp"
#include "chartView.hpp"
#include "node.hpp"
#include "chartCompare.hpp"

namespace astro
{
  class CompareNode : public Node
  {
  private:
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
      // if(mCompare->getInnerChart())
      //   {
      //     params.emplace("iDate",            mCompare->getInnerChart()->date().toSaveString());
      //     params.emplace("iLocation",        mCompare->getInnerChart()->location().toSaveString());
      //   }
      // if(mCompare->getOuterChart())
      //   {
      //     params.emplace("oDate",            mCompare->getOuterChart()->date().toSaveString());
      //     params.emplace("oLocation",        mCompare->getOuterChart()->location().toSaveString());
      //   }
      params.emplace("width",            std::to_string(mChartWidth));
      return params;
    };
    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params) override
    {
      // auto dateIter = params.find("iDate");
      // auto locIter = params.find("iDate");
      // if(dateIter != params.end() && locIter != params.end())
      //   {
      //     mCompare->getInnerChart()->setDate(dateIter->second);
      //     mCompare->getInnerChart()->setLocation(locIter->second);
      //   }
      // dateIter = params.find("iLocation");
      // locIter = params.find("oLocation");
      // if(dateIter != params.end() && locIter != params.end())
      //   {
      //     mCompare->getOuterChart()->setDate(dateIter->second);
      //     mCompare->getOuterChart()->setLocation(locIter->second);
      //   }
      std::stringstream ss(params["width"]);
      ss >> mChartWidth;
      return params;
    };
    
  public:
    CompareNode();
    ~CompareNode();
    virtual std::string type() const { return "ChartCompareNode"; }

    Chart* outerChart() { return mCompare->getOuterChart(); }
    Chart* innerChart() { return mCompare->getInnerChart(); }
  };
}

#endif // COMPARE_NODE_HPP
