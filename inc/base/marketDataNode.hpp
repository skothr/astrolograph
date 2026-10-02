#ifndef MARKET_DATA_NODE_HPP
#define MARKET_DATA_NODE_HPP

#include <thread>
#include "marketData.hpp"
#include "node.hpp"

class FileDialog;

//// node connector indices ////
// inputs
#define MARKETNODE_INPUT_STARTDATE   0
#define MARKETNODE_INPUT_ENDDATE     1
// outputs
#define MARKETNODE_OUTPUT_MARKETDATA 0
////////////////////////////////

  
class MarketDataNode : public Node
{
private:
  static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return { new Connector<DateTime>("Start Date Input"), new Connector<DateTime>("End Date Input") }; }
  static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return { new Connector<MarketData>("Market Data Output") }; }
    
  MarketData  mMarketData;
  StockData   mStockData;
  std::string mSymbol  = "";
  std::string mCsvPath = "";
  std::thread mLoadThread;
  bool        mThreadRunning = false;
  bool        mDoneLoading   = false;

  FileDialog  *mFileDialog = nullptr;
  DateTime mStartDate = DateTime::now();
  DateTime mEndDate   = DateTime::now();
    
  void downloadWorker();
  bool checkFileDialog();
    
  // virtual bool onConnect(ConnectorBase *con) override;
  virtual void onUpdate() override;
  virtual void onDraw() override;

public:
  MarketDataNode();
  ~MarketDataNode();
  virtual std::string type() const { return "MarketDataNode"; }
};


#endif // MARKET_DATA_NODE_HPP
