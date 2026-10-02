#include "marketDataNode.hpp"

#include <imgui.h>
#include <iostream>

#include "fileDialog.hpp"
#include "marketData.hpp"

MarketDataNode::MarketDataNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Market Data Node", true)
{
  mFileDialog = new FileDialog();
  outputs()[MARKETNODE_OUTPUT_MARKETDATA]->set(&mMarketData);
  setTitle("Market Data");
}


MarketDataNode::~MarketDataNode()
{
  if(mFileDialog) { delete mFileDialog; }
}


void MarketDataNode::downloadWorker()
{
  std::cout << "===> Downloading data for " << mSymbol << "...\n";
  mCsvPath = loadStockDataCurl(mSymbol); // loadData = true;
  std::cout << "===> Downloading Complete Loading data in: " << mCsvPath << "...\n";
  mStockData = loadStockDataCsv(mCsvPath);
  std::cout << "===> Loading Complete --> " << mStockData.size() << " data points.\n";

  mStartDate = minDate(mStockData);
  mEndDate   = maxDate(mStockData);
  mDoneLoading = true;
}


void MarketDataNode::onUpdate()
{  
  // DateTime *startDate = inputs()[MARKETNODE_INPUT_STARTDATE]->get<DateTime>();
  // DateTime *endDate   = inputs()[MARKETNODE_INPUT_ENDDATE]->get<DateTime>();

  // if(startDate) { mStartDate = *startDate; }
  // if(endDate)   { mEndDate   = *endDate;   }
}

void MarketDataNode::onDraw()
{
  float scale = getScale();
  mFileDialog->setGraph(mGraph);
    
  ImGui::TextUnformatted("Symbol: ");
  ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);

  char symbol[16] = "";
  strcpy(symbol, mSymbol.c_str());
  
  if(ImGui::InputText("##symbolInput", symbol, 16))
    { mSymbol = std::string(symbol); }
  
  bool loadData = false;
  ImGui::SameLine();
  if(ImGui::Button("Download") && !mThreadRunning)
    {
      // mThreadRunning = true;
      // mDoneLoading   = false;
      // mLoadThread = std::thread(std::bind(&MarketDataNode::downloadWorker, this));
      downloadWorker();
      mMarketData.emplace(mSymbol, mStockData);
      mSymbol = "";
    }

  // if(mThreadRunning && mDoneLoading)
  //   {
  //     mLoadThread.join();
  //     mThreadRunning = false;
  //     mMarketData = mMarketData2;
  //   }

  if(ImGui::Button("Load")) { mFileDialog->open("Open Location", DIALOG_LOAD, ".", {".csv"}); }
  checkFileDialog();
  if(!mCsvPath.empty())
    {
      ImGui::SameLine();
      if(!mCsvPath.empty() && (ImGui::Button("Reload") || loadData) && !mThreadRunning) { loadStockDataCsv(mCsvPath); }
      ImGui::SameLine(); ImGui::TextUnformatted(mCsvPath.c_str());
      ImGui::Text("Dates: [%s : %s]", mStartDate.toString(true, false).c_str(), mEndDate.toString(true, false).c_str());
    }
  ImGui::Separator();
}

bool MarketDataNode::checkFileDialog()
{
  bool success = false;
  if(mFileDialog->check())
    {
      std::string path = mFileDialog->getPath();
      std::cout << "File dialog path --> " << path << "\n";
      if(!path.empty())
        {
          mCsvPath = path;
          if(!mCsvPath.empty()) { loadStockDataCsv(mCsvPath); }
        }
      else { std::cout << "Empty path string!\n"; }
    }
  return success;
}
