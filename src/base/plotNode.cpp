#include "plotNode.hpp"
using namespace astro;

#include <curl/curl.h>
#include <fstream>

#include "imgui.h"
#include "tools.hpp"
#include "setting.hpp"
#include "settingForm.hpp"
#include "plotWidget.hpp"
#include "fileDialog.hpp"
#include "nodeGraph.hpp"


#define PLOT_OBJ_START OBJ_SUN
#define PLOT_OBJ_END   OBJ_PLUTO

PlotNode::PlotNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Plot Node", true), mOldStartDate(DateTime::now()), mOldEndDate(DateTime::now())
{
  mOldChart.setDate(DateTime::now());
  mFileDialog = new FileDialog();
  mWidget     = new PlotWidget();
  for(int i = PLOT_OBJ_START; i <= PLOT_OBJ_END; i++)
    { mParams.push_back(new PlanetDataParams(getObjName(i), getObjColor(i), 1.0f)); }
  mAspectParams.label = (getObjName(mAspectObj1) + " " + getAspectName(mAspectType) + " " + getObjName(mAspectObj2));
  mAspectParams.color = getAspectInfo(mAspectType)->color;
  mAspectParams.width = 2.0f;
  
  mSettings.push_back(new Setting<Vec2f>("Plot Size", "plotSize", &mPlotSize));
  mSettings.push_back(new Setting<std::string>("Plot Title", "plotTitle", &mWidget->mTitle));
  mSettings.push_back(new Setting<bool>("Settings Open", "settingsOpen", &mSettingsOpen));
  
  // axis settings
  for(int i = 0; i < mParams.size(); i++)
    {
      std::string pid = std::to_string(i);
      mSettings.push_back(new Setting<bool>  ("Visible",       "visible"+pid,     &mParams[i]->visible));
      mSettings.push_back(new Setting<bool>  ("Show Pos",      "showPos"+pid,     &mParams[i]->showPos));
      mSettings.push_back(new Setting<bool>  ("Show Speed",    "showSpeed"+pid,   &mParams[i]->showSpeed));
      mSettings.push_back(new Setting<bool>  ("Show Rx",       "showRx"+pid,      &mParams[i]->showRx));
      mSettings.push_back(new Setting<bool>  ("Show Shadows",  "showShadows"+pid, &mParams[i]->showShadows));
      mSettings.push_back(new Setting<bool>  ("Show Rx Boxes", "showRxBoxes"+pid, &mParams[i]->showRxBoxes));
    }
  
  mSettings.push_back(new Setting<Vec2d> ("Date Range",    "dRange",  &mWidget->mDateAxis.range));
  mSettings.push_back(new Setting<double>("Date Size",     "dOffset", &mWidget->mDateAxis.offset));
  mSettings.push_back(new Setting<double>("Date Center",   "dCenter", &mWidget->mDateAxis.center));
  mSettings.push_back(new Setting<double>("Date Size",     "dSize",   &mWidget->mDateAxis.viewSize));
  
  mSettings.push_back(new Setting<Vec2d> ("Pos Range",     "pRange",  &mWidget->mPlanetPosAxis.range));
  mSettings.push_back(new Setting<double>("Pos Offset",    "pOffset", &mWidget->mPlanetPosAxis.offset));
  mSettings.push_back(new Setting<double>("Pos Center",    "pCenter", &mWidget->mPlanetPosAxis.center));
  mSettings.push_back(new Setting<double>("Pos Size",      "pSize",   &mWidget->mPlanetPosAxis.viewSize));
  
  mSettings.push_back(new Setting<Vec2d> ("Speed Range",   "sRange",  &mWidget->mPlanetSpeedAxis.range));
  mSettings.push_back(new Setting<double>("Speed Offset",  "sOffset", &mWidget->mPlanetSpeedAxis.offset));
  mSettings.push_back(new Setting<double>("Speed Center",  "sCenter", &mWidget->mPlanetSpeedAxis.center));
  mSettings.push_back(new Setting<double>("Speed Size",    "sSize",   &mWidget->mPlanetSpeedAxis.viewSize));
  
  mSettings.push_back(new Setting<Vec2d> ("Market Range",  "mRange",  &mWidget->mMarketAxis.range));
  mSettings.push_back(new Setting<double>("Market Offset", "mOffset", &mWidget->mMarketAxis.offset));
  mSettings.push_back(new Setting<double>("Market Center", "mCenter", &mWidget->mMarketAxis.center));
  mSettings.push_back(new Setting<double>("Market Size",   "mSize",   &mWidget->mMarketAxis.viewSize));
  
  mSettings.push_back(new Setting<Vec2d> ("Volume Range",  "vRange",  &mWidget->mVolumeAxis.range));
  mSettings.push_back(new Setting<double>("Volume Offset", "vOffset", &mWidget->mVolumeAxis.offset));
  mSettings.push_back(new Setting<double>("Volume Center", "vCenter", &mWidget->mVolumeAxis.center));
  mSettings.push_back(new Setting<double>("Volume Size",   "vSize",   &mWidget->mVolumeAxis.viewSize));

  //mSettings.push_back(new Setting<std::string>("Market Data Path",   "marketDataPath",   &mMarketDataPath));
  
  // flag settings
  mSettings.push_back(new Setting<bool>("Debug",           "plotDebug",   &mWidget->debug));
  mSettings.push_back(new Setting<bool>("Update View",     "viewUpdate",  &mUpdateView));
  mSettings.push_back(new Setting<bool>("Chart Center",    "chartCenter", &mWidget->chartCenter));
  mSettings.push_back(new Setting<bool>("Draw Chart Date", "cDate",       &mWidget->drawChartDate));
  mSettings.push_back(new Setting<bool>("Lock X",          "lockX",       &mWidget->mLockX));
  mSettings.push_back(new Setting<bool>("Lock Y",          "lockY",       &mWidget->mLockY));


  for(int i = PLOT_OBJ_START; i <= PLOT_OBJ_END; i++) { mObjNames.push_back(getObjName(i));    }
  for(int i = 0; i < ASPECT_COUNT; i++)              { mAspNames.push_back(getAspectName(i)); }
  
  SettingBase *aTypeSetting = new ComboSetting ("Aspect Type",   "aspType", &mAspectType, mAspNames, ASPECT_SQUARE);
  SettingBase *a1Setting    = new ComboSetting ("Object 1",   "aspObj1",    &mAspectObj1, mObjNames, OBJ_SATURN);
  SettingBase *a2Setting    = new ComboSetting ("Object 2",   "aspObj2",    &mAspectObj2, mObjNames, OBJ_URANUS);
  SettingGroup *group = new SettingGroup("Aspect", "aspGroup", { a1Setting, a2Setting, aTypeSetting }, false, false);
  mSettingForm = new SettingForm(150.0f, 150.0f);
  mSettingForm->add(group);

  setMinSize(Vec2f(690, 420));
}

PlotNode::~PlotNode()
{
  if(mWidget)      { delete mWidget; }
  if(mFileDialog)  { delete mFileDialog; }
  if(mSettingForm) { delete mSettingForm; }
}

void PlotNode::onUpdate()
{
  Chart      *chart      = inputs()[PLOTNODE_INPUT_CHART]->get<Chart>();
  MarketData *marketData = inputs()[PLOTNODE_INPUT_MARKETDATA]->get<MarketData>();
  DateTime   *startDate  = inputs()[PLOTNODE_INPUT_STARTDATE]->get<DateTime>();
  DateTime   *endDate    = inputs()[PLOTNODE_INPUT_ENDDATE]->get<DateTime>();
  if(chart)
    {
      DateTime dtOrig  = chart->date();
      DateTime dtStart = dtOrig;
      DateTime dtEnd   = dtOrig;
      if(startDate) { dtStart = *startDate; } else { dtStart.setDay(dtStart.day() - 180); dtStart.fix(); }
      if(endDate)   { dtEnd   = *endDate;   } else { dtEnd.setDay(dtEnd.day() + 180);     dtEnd.fix();   }
      int numDays = std::floor(dtEnd.diffDays(dtStart));
      
      if(mReloadData ||
         (dtStart.year() != mOldStartDate.year()) || (dtStart.month() != mOldStartDate.month()) || (dtStart.day() != mOldStartDate.day()) ||
         (dtEnd.year()   != mOldEndDate.year())   || (dtEnd.month()   != mOldEndDate.month())   || (dtEnd.day()   != mOldEndDate.day())   ||
         (chart->getHouseSystem() != mOldChart.getHouseSystem()) ||
         (chart->getZodiac()      != mOldChart.getZodiac())      ||
         (chart->getTruePos()     != mOldChart.getTruePos()))
        {
          // find date period overlap
          DateTime overlapStart = dtStart; DateTime overlapEnd = dtEnd;
          bool datesOverlap = false;

          if(!mReloadData)
            {
              bool startOverlap = false;
              bool endOverlap   = false;
              if(dtStart >= mOldStartDate && dtStart <= mOldEndDate)      { overlapStart = dtStart;       startOverlap = true; }
              else if(mOldStartDate >= dtStart && mOldStartDate <= dtEnd) { overlapStart = mOldStartDate; startOverlap = true; }
              if(dtEnd >= mOldStartDate && dtEnd <= mOldEndDate)          { overlapEnd   = dtEnd;         endOverlap = true; }
              else if(mOldEndDate >= dtStart && mOldEndDate <= dtEnd)     { overlapEnd   = mOldEndDate;   endOverlap = true; }
              datesOverlap = startOverlap && endOverlap;
              datesOverlap &= (overlapEnd.diffDays(overlapStart) >= 1.0);
              std::cout << "-----------------------------------------------------------\n"
                        << "Old START: " << mOldStartDate << " | Old END: " << mOldEndDate << "\n"
                        << "New START: " << dtStart       << " | New END: " << dtEnd       << "\n"
                        << "Overlap(S" << startOverlap << "|E" << endOverlap
                        << ") --> START: " << overlapStart  << " | END: " << overlapEnd  << "\n"
                        << "Overlap days: " << overlapEnd.diffDays(overlapStart) << "\n"
                        << "-----------------------------------------------------------\n";
            }
          else
            {
              std::cout << "Reloading data...\n";
            }

          mOldChart.setDate(dtStart);
          mOldChart.setLocation(chart->location());
          mOldChart.setHouseSystem(chart->getHouseSystem());
          mOldChart.setZodiac(chart->getZodiac());
          mOldChart.setTruePos(chart->getTruePos());

          DateTime dt = dtStart;
          
          std::vector<std::vector<PlanetPoint>> newData;
          int numCopied     = 0;
          int numCalculated = 0;
          
          for(int i = PLOT_OBJ_START; i <= PLOT_OBJ_END; i++)
            { newData.push_back({}); newData.back().reserve(numDays); }
          
          for(int j = 0; j < numDays; j++)
            {
              if(datesOverlap && dt >= overlapStart && dt < overlapEnd)
                { // copy from overlapping data
                  int index = std::abs(std::round(dt.diffDays(mOldStartDate)));
                  for(int i = PLOT_OBJ_START; i <= PLOT_OBJ_END; i++)
                    {
                      if(mPlanetData.size() > i && mPlanetData[i].size() > index)
                        {
                          newData[i].emplace_back(mPlanetData[i][index]);
                          numCopied++;
                        }
                      else
                        { // calculate from ephemeris
                          mOldChart.setDate(dt);
                          ChartObject *obj = mOldChart.getSingleObject(i);
                          newData[i].emplace_back(obj->angle, obj->speed);
                          numCalculated++;
                        }
                    }
                }
              else
                { // calculate from ephemeris
                  mOldChart.setDate(dt);
                  for(int i = PLOT_OBJ_START; i <= PLOT_OBJ_END; i++)
                    {
                      ChartObject *obj = mOldChart.getSingleObject(i);
                      newData[i].emplace_back(obj->angle, obj->speed);
                      numCalculated++;
                    }
                }
              dt.setDay(dt.day()+1); dt.fix();
            }
          std::cout << " --> Done\n";
          std::cout << " --> Copied: " << numCopied << "  |  Calculated: " << numCalculated << "\n";

          mWidget->mDateAxis.center += (mOldStartDate.diffDays(dtStart));
          
          mOldChart.setDate(dtOrig);
          mOldStartDate = dtStart;
          mOldEndDate   = dtEnd;
          
          mWidget->clearPlanets();
          mWidget->setDateRange(dtStart, dtEnd);
          mWidget->updateDates();
          
          mPlanetData = newData;
          if(newData.size() > 0) { mRxData = findRetrogrades(mPlanetData, dtStart, dtEnd); }
          for(int i = 0; i < mPlanetData.size(); i++) { mWidget->addPlanet(&mPlanetData[i], &mRxData[i], mParams[i]); }

          if(marketData && marketData->size() > 0) { mWidget->setMarketData(&marketData->begin()->second, mUpdateView); }
          // if(!mMarketDataPath.empty()) { mWidget->loadMarketData(mMarketDataPath, mUpdateView); }
          
          mReloadAspects = true;
          mReloadData    = false;
        }
      
      if(mReloadAspects && numDays > 0)
        {
          std::cout << "Calculating aspects...\n";
          mAspectData.clear(); mAspectData.reserve(numDays);
          DateTime dt = dtStart;
          const std::vector<PlanetPoint> &p1 = mPlanetData[mAspectObj1-PLOT_OBJ_START];
          const std::vector<PlanetPoint> &p2 = mPlanetData[mAspectObj2-PLOT_OBJ_START];
          for(int j = 0; j < numDays; j++)
            {
              mAspectData.push_back(angleDiffDegrees(angleDiffDegrees(p2[j].angle, p1[j].angle), getAspectInfo(mAspectType)->angle));
              dt.setDay(dt.day()+1); dt.fix();
            }
          mAspectParams.obj1 = mAspectObj1;
          mAspectParams.obj2 = mAspectObj2;
          mAspectParams.asp  = mAspectType;
          mWidget->clearAspects();
          mWidget->addAspect(&mAspectData, &mAspectParams);
          mReloadAspects = false;
          std::cout << " --> Done\n";
        }
    }
}



void PlotNode::onDraw()
{
  float scale = getScale();

  Chart      *chart      = inputs()[PLOTNODE_INPUT_CHART]->get<Chart>();
  MarketData *marketData = inputs()[PLOTNODE_INPUT_MARKETDATA]->get<MarketData>();
  DateTime   *dtStart    = inputs()[PLOTNODE_INPUT_STARTDATE]->get<DateTime>();
  DateTime   *dtEnd      = inputs()[PLOTNODE_INPUT_ENDDATE]->get<DateTime>();

  if(marketData && marketData->size() > 0 && &marketData->begin()->second != mWidget->mMarketData) { mWidget->setMarketData(&marketData->begin()->second, mUpdateView); }
  
  ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth);
  ImGui::SetNextTreeNodeOpen(mSettingsOpen);
  if(ImGui::CollapsingHeader("Settings", nullptr, flags))
    {
      mSettingsOpen = true;
      bool resetView = false; // reset if update

      ImGui::BeginGroup();
      {
        // // to open market data
        // ImGui::TextUnformatted("Market Data: ");

        // ImGui::SameLine(); ImGui::TextUnformatted("Ticker: ");
        // ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
        // ImGui::InputText("##tickerInput", mTicker, 8);

        // bool loadData = false;
        // ImGui::SameLine(); if(ImGui::Button("Download")) { mMarketDataPath = loadMarketDataCurl(mTicker); loadData = true; }

        // if(ImGui::Button("Load")) { mFileDialog->open("Open Location", DIALOG_LOAD, ".", {".csv"}); }
        // checkFileDialog();
        // if(marketData) //!mMarketDataPath.empty())
        //   {
        //     ImGui::SameLine();
        //     if((ImGui::Button("Reload") || loadData) && marketData) { mWidget->setMarketData(marketData, true); } //mWidget->resetView(); }
        //     ImGui::SameLine(); ImGui::TextUnformatted(mMarketDataPath.c_str());
        //     ImGui::Text("Dates: [%s : %s]", mWidget->mMarketStartDate.toString(true, false).c_str(), mWidget->mMarketEndDate.toString(true, false).c_str());
        //     ImGui::SameLine(); ImGui::Text("   Data: [%f, %f]", mWidget->mMarketAxis.dataMin, mWidget->mMarketAxis.dataMax);
        //   }
        // ImGui::Separator();
  
        char title[256] = "";
        sprintf(title, "%s", mWidget->mTitle.c_str());
        ImGui::TextUnformatted("Plot Title:  ");
        ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
        if(ImGui::InputText("##plotTitle", title, 256)) { mWidget->mTitle = title; }
  
        ImGui::SameLine();
        ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos())+Vec2f(42.0f, 0.0f)*scale);
        ImGui::TextUnformatted("   Debug");
        ImGui::SameLine(); ImGui::Checkbox("##debugCb", &mWidget->debug);
        ImGui::SameLine(); ImGui::TextUnformatted("   Update View");
        ImGui::SameLine(); if(ImGui::Checkbox("##updateCb", &mUpdateView)) { resetView |= mUpdateView; }
        ImGui::SameLine(); ImGui::TextUnformatted("   Chart Center");
        ImGui::SameLine(); ImGui::Checkbox("##chartCenterCb", &mWidget->chartCenter);
        ImGui::SameLine(); ImGui::TextUnformatted("   Chart Date");
        ImGui::SameLine(); ImGui::Checkbox("##chartDateCb", &mWidget->drawChartDate);

        ImGui::SameLine(); ImGui::TextUnformatted("   Connect Breaks");
        bool connect = false;  for(auto &p : mParams) { if(p->connectBreaks) { connect = true; break; } }
        ImGui::SameLine(); if(ImGui::Checkbox("##pConnect", &connect))
                             { for(auto &p : mParams) { p->connectBreaks = connect; } }

        ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos())+Vec2f(0.0f, 5.0f));
        ImGui::TextUnformatted("Y Labels:");
        ImGui::SameLine(); ImGui::TextUnformatted(" Planet Pos");
        ImGui::SameLine(); ImGui::Checkbox("##pLabels",   &mWidget->mPlanetPosAxis.drawLabels);
        ImGui::SameLine(); ImGui::TextUnformatted(" Planet Speed");
        ImGui::SameLine(); ImGui::Checkbox("##sLabels",   &mWidget->mPlanetSpeedAxis.drawLabels);
        ImGui::SameLine(); ImGui::TextUnformatted(" Aspects");
        ImGui::SameLine(); ImGui::Checkbox("##aLabels",   &mWidget->mAspectAxis.drawLabels);
        ImGui::SameLine(); ImGui::TextUnformatted(" Market");
        ImGui::SameLine(); ImGui::Checkbox("##mLabels",   &mWidget->mMarketAxis.drawLabels);
        ImGui::SameLine(); ImGui::TextUnformatted(" Volume");
        ImGui::SameLine(); ImGui::Checkbox("##vLabels",   &mWidget->mVolumeAxis.drawLabels);
      
        ImGui::TextUnformatted("Y Lines: ");
        ImGui::SameLine(); ImGui::TextUnformatted(" Planet Pos");
        ImGui::SameLine(); ImGui::Checkbox("##pLines",   &mWidget->mPlanetPosAxis.drawLabelLines);
        ImGui::SameLine(); ImGui::TextUnformatted(" Planet Speed");
        ImGui::SameLine(); ImGui::Checkbox("##sLines",   &mWidget->mPlanetSpeedAxis.drawLabelLines);
        ImGui::SameLine(); ImGui::TextUnformatted(" Aspects");
        ImGui::SameLine(); ImGui::Checkbox("##aLines",   &mWidget->mAspectAxis.drawLabelLines);
        ImGui::SameLine(); ImGui::TextUnformatted(" Market");
        ImGui::SameLine(); ImGui::Checkbox("##mLines",   &mWidget->mMarketAxis.drawLabelLines);
        ImGui::SameLine(); ImGui::TextUnformatted(" Volume");
        ImGui::SameLine(); ImGui::Checkbox("##vLines",   &mWidget->mVolumeAxis.drawLabelLines);

        ImGui::Separator();
        if(ImGui::Button("Recalculate Planets")) { mReloadData    = true; }
        ImGui::SameLine();
        if(ImGui::Button("Recalculate Aspects")) { mReloadAspects = true; }
        ImGui::Separator();

        ImGui::SetNextTreeNodeOpen(mAspectsOpen);
        if(ImGui::CollapsingHeader("Aspects", nullptr, flags))
          {
            mAspectsOpen = true;
            AspectType oldAsp  = mAspectType;
            ObjType    oldObj1 = mAspectObj1;
            ObjType    oldObj2 = mAspectObj2;
            mSettingForm->draw(scale, false, isBodyVisible());
            mReloadAspects |= (mAspectType != oldAsp || mAspectObj1 != oldObj1 || mAspectObj2 != oldObj2);
            ImGui::Separator();
          } else { mAspectsOpen = false; }


        double volStep1 = mWidget->scalePlotPosAx(mWidget->mVolumeAxis.dataMax/50.0, &mWidget->mVolumeAxis); // small step size (Click +/-)
        double volStep2 = mWidget->scalePlotPosAx(mWidget->mVolumeAxis.dataMax/5.0,  &mWidget->mVolumeAxis); // large step size (Ctrl+Click +/-)
        // volStep1 = std::max(volStep1, 1000.0);
        // volStep2 = std::max(volStep2, 1000000.0);
        
        // VIEW PARAMS //
        ImGui::SetNextTreeNodeOpen(mViewParamsOpen);
        if(ImGui::CollapsingHeader("View Params", nullptr, flags))
          {
            mViewParamsOpen = true;
            ImGui::BeginGroup();
            {
              // ImGui::TextUnformatted("View Params:");
              // ImGui::Separator(); 
              ImGui::Indent();
              ImGui::TextUnformatted("(X) Dates           | ");
              ImGui::SameLine(); ImGui::TextUnformatted(" Center");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##dcenter", &mWidget->mDateAxis.center,          1.0, 7.0);
              ImGui::SameLine(); ImGui::TextUnformatted("   Size");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##dsize",   &mWidget->mDateAxis.viewSize,        1.0, 7.0);

              ImGui::TextUnformatted("(Y) Astro Positions | ");
              ImGui::SameLine(); ImGui::TextUnformatted(" Center");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##pcenter", &mWidget->mPlanetPosAxis.center,     1.0, 10.0);
              ImGui::SameLine(); ImGui::TextUnformatted("   Size");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##psize",   &mWidget->mPlanetPosAxis.viewSize,   1.0, 10.0);

              ImGui::TextUnformatted("(Y) Astro Speeds    | ");
              ImGui::SameLine(); ImGui::TextUnformatted(" Center");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##scenter", &mWidget->mPlanetSpeedAxis.center,   1.0,  5.0);
              ImGui::SameLine(); ImGui::TextUnformatted("   Size");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##ssize",   &mWidget->mPlanetSpeedAxis.viewSize, 1.0,  5.0);
        
              ImGui::TextUnformatted("(Y) Astro Aspects   | ");
              ImGui::SameLine(); ImGui::TextUnformatted(" Center");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##acenter", &mWidget->mAspectAxis.center,        1.0, 10.0);
              ImGui::SameLine(); ImGui::TextUnformatted("   Size");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##asize",   &mWidget->mAspectAxis.viewSize,      1.0, 10.0);

              ImGui::TextUnformatted("(Y) Market Price    | ");
              ImGui::SameLine(); ImGui::TextUnformatted(" Center");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##mcenter", &mWidget->mMarketAxis.center,        1.0, 10.0);
              ImGui::SameLine(); ImGui::TextUnformatted("   Size");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##msize",   &mWidget->mMarketAxis.viewSize,      1.0, 10.0);
  
              ImGui::TextUnformatted("(Y) Market Volume   | ");
              ImGui::SameLine(); ImGui::TextUnformatted(" Center");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##vcenter", &mWidget->mVolumeAxis.center,   volStep1, volStep2);
              ImGui::SameLine(); ImGui::TextUnformatted("   Size");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##vsize",   &mWidget->mVolumeAxis.viewSize, volStep1, volStep2);
              ImGui::SameLine(); ImGui::TextUnformatted("   Line");
              ImGui::SameLine(); ImGui::Checkbox("##vline", &mWidget->drawVolumeLine);
              ImGui::Unindent();
            }
            ImGui::EndGroup();
            //ImGui::SameLine();
          } else { mViewParamsOpen = false; }

        // RESET PARAMS //
        ImGui::SetNextTreeNodeOpen(mResetParamsOpen);
        if(ImGui::CollapsingHeader("Reset Params", nullptr, flags))
          {
            mResetParamsOpen = true;
            ImGui::BeginGroup();
            {
              ImGui::TextUnformatted("Reset Params:");
              ImGui::Separator(); ImGui::Indent();
              ImGui::TextUnformatted("(X) Dates           | ");
              ImGui::SameLine(); ImGui::TextUnformatted(" Min");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              if(ImGui::InputDouble("##dmin", &mWidget->mDateAxis.range.x,  1.0f, 7.0f) && mUpdateView) { resetView = true; }
              ImGui::SameLine(); ImGui::TextUnformatted(" Max");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              if(ImGui::InputDouble("##dmax", &mWidget->mDateAxis.range.y,  1.0f, 7.0f) && mUpdateView) { resetView = true; }
              ImGui::SameLine(); ImGui::TextUnformatted(" Offset");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              if(ImGui::InputDouble("##doff", &mWidget->mDateAxis.offset,   1.0f, 7.0f) && mUpdateView) { resetView = true; }
              ImGui::SameLine(); ImGui::TextUnformatted(" Log Scale");
              ImGui::SameLine(); if(ImGui::Checkbox("##dlog", &mWidget->mDateAxis.logScale))
                                   {
                                     if(mWidget->mDateAxis.logScale)
                                       {
                                         mWidget->mDateAxis.range.x = mWidget->mDateAxis.range.x == 0.0 ? 1.0 : log(mWidget->mDateAxis.range.x);
                                         mWidget->mDateAxis.range.y = mWidget->mDateAxis.range.y == 0.0 ? 1.0 : log(mWidget->mDateAxis.range.y);
                                       }
                                     else
                                       {
                                         mWidget->mDateAxis.range.x = mWidget->mDateAxis.range.x == 1.0 ? 0.0 : exp(mWidget->mDateAxis.range.x);
                                         mWidget->mDateAxis.range.y = mWidget->mDateAxis.range.y == 1.0 ? 0.0 : exp(mWidget->mDateAxis.range.y);
                                       }
                                   }
              ImGui::SameLine(); ImGui::TextUnformatted(" Auto Scale");
              ImGui::SameLine(); ImGui::Checkbox("##dauto", &mWidget->mDateAxis.autoScale);

              ImGui::TextUnformatted("(Y) Astro Positions | ");
              ImGui::SameLine(); ImGui::TextUnformatted(" Min");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              if(ImGui::InputDouble("##pmin", &mWidget->mPlanetPosAxis.range.x,  1.0, 10.0f) && mUpdateView) { resetView = true; }
              ImGui::SameLine(); ImGui::TextUnformatted(" Max");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              if(ImGui::InputDouble("##pmax", &mWidget->mPlanetPosAxis.range.y,  1.0, 10.0f) && mUpdateView) { resetView = true; }
              ImGui::SameLine(); ImGui::TextUnformatted(" Offset");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              if(ImGui::InputDouble("##poff", &mWidget->mPlanetPosAxis.offset,   1.0, 10.0f) && mUpdateView) { resetView = true; }
              ImGui::SameLine(); ImGui::TextUnformatted(" Log Scale");
              ImGui::SameLine(); if(ImGui::Checkbox("##plog", &mWidget->mPlanetPosAxis.logScale))
                                   {
                                     if(mWidget->mPlanetPosAxis.logScale)
                                       {
                                         mWidget->mPlanetPosAxis.range.x = mWidget->mPlanetPosAxis.range.x == 0.0 ? 1.0 : log(mWidget->mPlanetPosAxis.range.x);
                                         mWidget->mPlanetPosAxis.range.y = mWidget->mPlanetPosAxis.range.y == 0.0 ? 1.0 : log(mWidget->mPlanetPosAxis.range.y);
                                       }
                                     else
                                       {
                                         mWidget->mPlanetPosAxis.range.x = mWidget->mPlanetPosAxis.range.x == 1.0 ? 0.0 : exp(mWidget->mPlanetPosAxis.range.x);
                                         mWidget->mPlanetPosAxis.range.y = mWidget->mPlanetPosAxis.range.y == 1.0 ? 0.0 : exp(mWidget->mPlanetPosAxis.range.y);
                                       }
                                   }
              ImGui::SameLine(); ImGui::TextUnformatted(" Auto Scale");
              ImGui::SameLine(); ImGui::Checkbox("##pauto", &mWidget->mPlanetPosAxis.autoScale);

              ImGui::TextUnformatted("(Y) Astro Speeds    | ");
              ImGui::SameLine(); ImGui::TextUnformatted(" Min");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              if(ImGui::InputDouble("##smin", &mWidget->mPlanetSpeedAxis.range.x,  1.0, 10.0) && mUpdateView) { resetView = true; }
              ImGui::SameLine(); ImGui::TextUnformatted(" Max");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              if(ImGui::InputDouble("##smax", &mWidget->mPlanetSpeedAxis.range.y,  1.0, 10.0) && mUpdateView) { resetView = true; }
              ImGui::SameLine(); ImGui::TextUnformatted(" Offset");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              if(ImGui::InputDouble("##soff", &mWidget->mPlanetSpeedAxis.offset,   1.0, 10.0) && mUpdateView) { resetView = true; }
              ImGui::SameLine(); ImGui::TextUnformatted(" Log Scale");
              ImGui::SameLine(); if(ImGui::Checkbox("##slog", &mWidget->mPlanetSpeedAxis.logScale))
                                   {
                                     if(mWidget->mPlanetSpeedAxis.logScale)
                                       {
                                         mWidget->mPlanetSpeedAxis.range.x = mWidget->mPlanetSpeedAxis.range.x == 0.0 ? 1.0 : log(mWidget->mPlanetSpeedAxis.range.x);
                                         mWidget->mPlanetSpeedAxis.range.y = mWidget->mPlanetSpeedAxis.range.y == 0.0 ? 1.0 : log(mWidget->mPlanetSpeedAxis.range.y);
                                       }
                                     else
                                       {
                                         mWidget->mPlanetSpeedAxis.range.x = mWidget->mPlanetSpeedAxis.range.x == 1.0 ? 0.0 : exp(mWidget->mPlanetSpeedAxis.range.x);
                                         mWidget->mPlanetSpeedAxis.range.y = mWidget->mPlanetSpeedAxis.range.y == 1.0 ? 0.0 : exp(mWidget->mPlanetSpeedAxis.range.y);
                                       }
                                   }
              ImGui::SameLine(); ImGui::TextUnformatted(" Auto Scale");
              ImGui::SameLine(); ImGui::Checkbox("##sauto", &mWidget->mPlanetSpeedAxis.autoScale);

              ImGui::TextUnformatted("(Y) Astro Aspects   | ");
              ImGui::SameLine(); ImGui::TextUnformatted(" Min");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              if(ImGui::InputDouble("##amin", &mWidget->mAspectAxis.range.x,  1.0, 10.0f) && mUpdateView) { resetView = true; }
              ImGui::SameLine(); ImGui::TextUnformatted(" Max");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              if(ImGui::InputDouble("##amax", &mWidget->mAspectAxis.range.y,  1.0, 10.0f) && mUpdateView) { resetView = true; }
              ImGui::SameLine(); ImGui::TextUnformatted(" Offset");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              if(ImGui::InputDouble("##aoff", &mWidget->mAspectAxis.offset,   1.0, 10.0f) && mUpdateView) { resetView = true; }
              ImGui::SameLine(); ImGui::TextUnformatted(" Log Scale");
              ImGui::SameLine(); if(ImGui::Checkbox("##alog", &mWidget->mAspectAxis.logScale))
                                   {
                                     if(mWidget->mAspectAxis.logScale)
                                       {
                                         mWidget->mAspectAxis.range.x = mWidget->mAspectAxis.range.x == 0.0 ? 1.0 : log(mWidget->mAspectAxis.range.x);
                                         mWidget->mAspectAxis.range.y = mWidget->mAspectAxis.range.y == 0.0 ? 1.0 : log(mWidget->mAspectAxis.range.y);
                                       }
                                     else
                                       {
                                         mWidget->mAspectAxis.range.x = mWidget->mAspectAxis.range.x == 1.0 ? 0.0 : exp(mWidget->mAspectAxis.range.x);
                                         mWidget->mAspectAxis.range.y = mWidget->mAspectAxis.range.y == 1.0 ? 0.0 : exp(mWidget->mAspectAxis.range.y);
                                       }
                                   }
              ImGui::SameLine(); ImGui::TextUnformatted(" Auto Scale");
              ImGui::SameLine(); ImGui::Checkbox("##aauto", &mWidget->mAspectAxis.autoScale);

              ImGui::TextUnformatted("(Y) Market Price    | ");
              ImGui::SameLine(); ImGui::TextUnformatted(" Min");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              if(ImGui::InputDouble("##mmin", &mWidget->mMarketAxis.range.x, 1.0, 10.0) && mUpdateView) { resetView = true; }
              ImGui::SameLine(); ImGui::TextUnformatted(" Max");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              if(ImGui::InputDouble("##mmax", &mWidget->mMarketAxis.range.y, 1.0, 10.0) && mUpdateView) { resetView = true; }
              ImGui::SameLine(); ImGui::TextUnformatted(" Offset");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              if(ImGui::InputDouble("##moff", &mWidget->mMarketAxis.offset,  1.0, 10.0) && mUpdateView) { resetView = true; }
              ImGui::SameLine(); ImGui::TextUnformatted(" Log Scale");
              ImGui::SameLine(); if(ImGui::Checkbox("##mlog", &mWidget->mMarketAxis.logScale))
                                   {
                                     if(mWidget->mMarketAxis.logScale)
                                       {
                                         mWidget->mMarketAxis.range.x = mWidget->mMarketAxis.range.x == 0.0 ? 1.0 : log(mWidget->mMarketAxis.range.x);
                                         mWidget->mMarketAxis.range.y = mWidget->mMarketAxis.range.y == 0.0 ? 1.0 : log(mWidget->mMarketAxis.range.y);
                                       }
                                     else
                                       {
                                         mWidget->mMarketAxis.range.x = mWidget->mMarketAxis.range.x == 1.0 ? 0.0 : exp(mWidget->mMarketAxis.range.x);
                                         mWidget->mMarketAxis.range.y = mWidget->mMarketAxis.range.y == 1.0 ? 0.0 : exp(mWidget->mMarketAxis.range.y);
                                       }
                                   }
              ImGui::SameLine(); ImGui::TextUnformatted(" Auto Scale");
              ImGui::SameLine(); ImGui::Checkbox("##mauto", &mWidget->mMarketAxis.autoScale);

              ImGui::TextUnformatted("(Y) Market Volume   | ");
              ImGui::SameLine(); ImGui::TextUnformatted(" Min");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              if(ImGui::InputDouble("##vmin", &mWidget->mVolumeAxis.range.x, volStep1, volStep2) && mUpdateView) { resetView = true; }
              ImGui::SameLine(); ImGui::TextUnformatted(" Max");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              if(ImGui::InputDouble("##vmax", &mWidget->mVolumeAxis.range.y, volStep1, volStep2) && mUpdateView) { resetView = true; }
              ImGui::SameLine(); ImGui::TextUnformatted(" Offset");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              if(ImGui::InputDouble("##voff", &mWidget->mVolumeAxis.offset,  volStep1, volStep2) && mUpdateView) { resetView = true; }
              ImGui::SameLine(); ImGui::TextUnformatted(" Log Scale");
              ImGui::SameLine(); if(ImGui::Checkbox("##vlog", &mWidget->mVolumeAxis.logScale))
                                   {
                                     if(mWidget->mVolumeAxis.logScale)
                                       {
                                         mWidget->mVolumeAxis.range.x = mWidget->mVolumeAxis.range.x == 0.0 ? 1.0 : log(mWidget->mVolumeAxis.range.x);
                                         mWidget->mVolumeAxis.range.y = mWidget->mVolumeAxis.range.y == 0.0 ? 1.0 : log(mWidget->mVolumeAxis.range.y);
                                       }
                                     else
                                       {
                                         mWidget->mVolumeAxis.range.x = mWidget->mVolumeAxis.range.x == 1.0 ? 0.0 : exp(mWidget->mVolumeAxis.range.x);
                                         mWidget->mVolumeAxis.range.y = mWidget->mVolumeAxis.range.y == 1.0 ? 0.0 : exp(mWidget->mVolumeAxis.range.y);
                                       }
                                   }
              ImGui::SameLine(); ImGui::TextUnformatted(" Auto Scale");
              ImGui::SameLine(); ImGui::Checkbox("##vauto", &mWidget->mVolumeAxis.autoScale);
              ImGui::Unindent();
            }
            ImGui::EndGroup();
          } else { mResetParamsOpen = false; }
  
        // LABELING //
        ImGui::SetNextTreeNodeOpen(mLabelParamsOpen);
        if(ImGui::CollapsingHeader("Label Params", nullptr, flags))
          {
            mLabelParamsOpen = true;
            ImGui::BeginGroup();
            {
              ImGui::TextUnformatted("Axis Labeling:");
              ImGui::Separator(); ImGui::Indent();
              ImGui::TextUnformatted("(X) Dates           | ");
              ImGui::SameLine(); ImGui::TextUnformatted(" Min");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##dminint", &mWidget->mDateAxis.labelMin,        1.0,  10.0);
              ImGui::SameLine(); ImGui::TextUnformatted(" Max");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##dmaxint", &mWidget->mDateAxis.labelMax,        1.0,  10.0);
              ImGui::SameLine(); ImGui::TextUnformatted(" Interval");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##dlabelint", &mWidget->mDateAxis.labelInterval, 1.0,  10.0);

              ImGui::TextUnformatted("(Y) Astro Positions | ");
              ImGui::SameLine(); ImGui::TextUnformatted(" Min");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##pminint", &mWidget->mPlanetPosAxis.labelMin,        1.0,  10.0);
              ImGui::SameLine(); ImGui::TextUnformatted(" Max");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##pmaxint", &mWidget->mPlanetPosAxis.labelMax,        1.0,  10.0);
              ImGui::SameLine(); ImGui::TextUnformatted(" Interval");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##plabelint", &mWidget->mPlanetPosAxis.labelInterval, 1.0,  10.0);

              ImGui::TextUnformatted("(Y) Astro Speeds    | ");
              ImGui::SameLine(); ImGui::TextUnformatted(" Min");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##sminint", &mWidget->mPlanetSpeedAxis.labelMin,        1.0,  10.0);
              ImGui::SameLine(); ImGui::TextUnformatted(" Max");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##smaxint", &mWidget->mPlanetSpeedAxis.labelMax,        1.0,  10.0);
              ImGui::SameLine(); ImGui::TextUnformatted(" Interval");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##slabelint", &mWidget->mPlanetSpeedAxis.labelInterval, 1.0,  10.0);

              ImGui::TextUnformatted("(Y) Astro Aspects   | ");
              ImGui::SameLine(); ImGui::TextUnformatted(" Min");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##aminint", &mWidget->mAspectAxis.labelMin,        1.0,  10.0);
              ImGui::SameLine(); ImGui::TextUnformatted(" Max");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##amaxint", &mWidget->mAspectAxis.labelMax,        1.0,  10.0);
              ImGui::SameLine(); ImGui::TextUnformatted(" Interval");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##alabelint", &mWidget->mAspectAxis.labelInterval, 1.0,  10.0);

              ImGui::TextUnformatted("(Y) Market Price    | ");
              ImGui::SameLine(); ImGui::TextUnformatted(" Min");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##mminint", &mWidget->mMarketAxis.labelMin,        1.0,  10.0);
              ImGui::SameLine(); ImGui::TextUnformatted(" Max");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##mmaxint", &mWidget->mMarketAxis.labelMax,        1.0,  10.0);
              ImGui::SameLine(); ImGui::TextUnformatted(" Interval");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##mlabelint", &mWidget->mMarketAxis.labelInterval, 1.0,  10.0);

              ImGui::TextUnformatted("(Y) Market Volume   | ");
              ImGui::SameLine(); ImGui::TextUnformatted(" Min");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##vminint", &mWidget->mVolumeAxis.labelMin,        1.0,  10.0);
              ImGui::SameLine(); ImGui::TextUnformatted(" Max");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##vmaxint", &mWidget->mVolumeAxis.labelMax,        1.0,  10.0);
              ImGui::SameLine(); ImGui::TextUnformatted(" Interval");
              ImGui::SameLine(); ImGui::SetNextItemWidth(150*scale);
              ImGui::InputDouble("##vlabelint", &mWidget->mVolumeAxis.labelInterval, volStep1, volStep2);

              ImGui::Unindent();
            }
            ImGui::EndGroup();
          } else { mLabelParamsOpen = false; }
      }
      ImGui::EndGroup();
      Vec2f uiSize = getGraph()->screenToGraphVec(Vec2f(ImGui::GetItemRectMax()) - ImGui::GetItemRectMin());
      mPlotSize.x = std::max(mPlotSize.x, uiSize.x);
      mPlotSize.x = std::max(mPlotSize.x, mMinSize.x);
      mPlotSize.y = std::max(mPlotSize.y, mMinSize.y);
      
      if(mUpdateView && resetView) { mWidget->resetView(); }
    }
  else { mSettingsOpen = false; }
  
  mWidget->chartDate = (chart ? chart->date() : DateTime::now());
  Vec2f pSize = mWidget->draw(getViewSettings(), mResizeClicked, scale, mPlotSize);
  // mPlotSize.x = pSize.x;//std::max(mPlotSize.x, pSize.x);
  // mPlotSize.y = pSize.y;//std::max(mPlotSize.y, pSize.y);
  
  if((ImGui::IsItemHovered() && !mClicked && !ImGui::IsKeyDown(GLFW_KEY_LEFT_CONTROL) && !ImGui::IsKeyDown(GLFW_KEY_RIGHT_CONTROL)) ||
     mWidget->contextOpen())
    { mActive = true; }
}

void PlotNode::onResize(const Vec2f &dSize)
{
  mPlotSize += dSize;
}



std::vector<std::vector<RxBox>> PlotNode::findRetrogrades(const std::vector<std::vector<PlanetPoint>> &planetData, const DateTime &dtStart, const DateTime &dtEnd)
{
  std::cout << "Calculating retrogrades...\n";
  
  std::vector<std::vector<RxBox>> rxData;
  for(int i = PLOT_OBJ_START; i <= PLOT_OBJ_END; i++) { rxData.push_back({}); }
  if(dtEnd.diffDays(dtStart) < 1.0) { std::cout << "Zero date span! (nothing to calculate)\n"; return rxData; }
  
  // find retrogrades
  for(int i = PLOT_OBJ_START; i <= PLOT_OBJ_END; i++)
    {
      rxData.push_back({});
      const std::vector<PlanetPoint> &points  = planetData[i];
      std::vector<RxBox>             &rxBoxes = rxData[i];
      int    numDays   = points.size();
      double lastSpeed = 0.0;//(points.size() > 0 ? points[0].speed : 0.0);
      for(int j = 0; j < points.size()-1; j++)
        {
          int next = j+1;
          const PlanetPoint &p = points[j];
          const PlanetPoint &q = points[next];
          if(lastSpeed > 0.0 && p.speed <= 0.0)
            { // make sure it's the closest point to rx station (TODO: make more visually accurate! (< 1 day)
              double x       = j;                 double y       = p.angle;
              double xLower  = (j > 0 ? j-1 : 0); double yLower  = points[(int)xLower].angle;
              double xHigher = next;              double yHigher = q.angle;
              Vec2d pRx(x, y);
              if(yLower  > pRx.y) { pRx.x = xLower;  pRx.y = yLower;  }
              if(yHigher > pRx.y) { pRx.x = xHigher; pRx.y = yHigher; }
              rxBoxes.emplace_back(Rect2d(pRx, pRx));
              rxBoxes.back().rxValid = true;
            }
          else if(j > 0 && lastSpeed <= 0.0 && p.speed >= 0.0)
            { // make sure it's the closest point to dx station (TODO: make more visually accurate! (< 1 day)
              double x       = j;                 double y       = p.angle;
              double xLower  = (j > 0 ? j-1 : 0); double yLower  = points[(int)xLower].angle;
              double xHigher = next;              double yHigher = q.angle;
              Vec2d pRx(x, y);
              if(yLower  < y) { pRx.x = xLower;  pRx.y = yLower;  }
              if(yHigher < y) { pRx.x = xHigher; pRx.y = yHigher; }
          
              if(rxBoxes.size() == 0)
                { // rx station not captured in data
                  rxBoxes.emplace_back(Rect2d(pRx, pRx));
                  rxBoxes.back().rxValid = false;
                }
              rxBoxes.back().rxBox.p2 = pRx;
              rxBoxes.back().dxValid = true;
            }
          else if(p.speed <= 0.0)
            { // update rx rect
              Vec2d pRx(j, p.angle);
              if(rxBoxes.size() == 0)
                {
                  rxBoxes.emplace_back(Rect2d(Vec2d(0.0f, points[0].angle), pRx));
                }
              // else if(j > 0)
              //   { rxBoxes.back().rxValid = true; }
              rxBoxes.back().rxBox.p2 = pRx;
            }
          lastSpeed = p.speed;
        }

      if(rxBoxes.size() > 0 && !rxBoxes.front().rxValid)
        { // retrograde station not captured in data -- extend search backward from start date
          DateTime dt2 = dtStart;
          PlanetPoint p2 = points.front();
          double lastSpeed2 = p2.speed;
          int offset = 0;
          while(p2.speed < 0.0)
            { // calculate from ephemeris
              dt2.setDay(dt2.day()-1); dt2.fix();
              mOldChart.setDate(dt2);
              ChartObject *obj = mOldChart.getSingleObject(i);
              p2.angle = obj->angle; p2.speed = obj->speed;
              lastSpeed2 = p2.speed;
              offset--;
            }
          rxBoxes.front().rxBox.p1 = Vec2d(offset, p2.angle);//points[0].angle), pRx));
          rxBoxes.front().rxValid = true;
        }
      if(rxBoxes.size() > 0 && !rxBoxes.back().dxValid)
        { // direct station not captured in data -- extend search forward from end date
          DateTime dt2 = dtEnd;
          PlanetPoint p2 = points.back();
          double lastSpeed2 = p2.speed;
          int offset = 0;
          while(p2.speed < 0.0)
            { // calculate from ephemeris
              dt2.setDay(dt2.day()+1); dt2.fix();
              mOldChart.setDate(dt2);
              ChartObject *obj = mOldChart.getSingleObject(i);
              p2.angle = obj->angle; p2.speed = obj->speed;
              lastSpeed2 = p2.speed;
              offset++;
            }
          rxBoxes.back().rxBox.p2 = Vec2d(points.size()+offset, p2.angle);
          rxBoxes.back().dxValid = true;
        }

      // find shadow periods
      for(int j = 0; j < rxBoxes.size(); j++)
        {
          RxBox &rx = rxBoxes[j];
          if(!rx.dxValid || !rx.rxValid) { rxBoxes.erase(rxBoxes.begin()+j); j--; continue; }
          // find shadow period
          double xRx = rx.rxBox.p1.x;
          double xDx = rx.rxBox.p2.x;
          double yRx = rx.rxBox.p1.y;
          double yDx = rx.rxBox.p2.y;
                  
          double x0  = xRx;
          double x1  = xDx;
          double y0  = yRx;//points[(int)x1].angle;
          double y1  = yDx;//points[(int)x0].angle;
          bool flipped = (points[(int)xRx-1].angle < points[(int)xDx+1].angle); // crosses 0/360 degrees within post-shadow period

          // pre-shadow
          bool found   = false;
          double lastY = points[(int)xRx-1].angle;
          for(int k = xRx-1; k >= 0; k--)
            {
              y0 = points[k].angle;
              if(angleDiffDegrees(y0, yDx) > angleDiffDegrees(lastY, yDx)) { x0 = k; found = true; break; }
              lastY = y0;
            }
          if(!found) { x0 = 0; y0 = points[0].angle; }
          else
            { // make sure closest point was found (+/- 1 day)
              double diffx0     = angleDiffDegrees(points[(int)x0].angle, yDx);
              double diffLower  = (x0 == 0               ? diffx0 : angleDiffDegrees(points[(int)x0-1].angle, yDx));
              double diffHigher = (x0 == numDays-1 ? diffx0 : angleDiffDegrees(points[(int)x0+1].angle, yDx));
              if(diffLower  < diffx0) { x0--; }
              if(diffHigher < diffx0) { x0++; }
            }
          rx.preShadowComplete = found;

          // post-shadow       
          found   = false;
          lastY = points[(int)xDx+1].angle;
          for(int k = xDx+1; k < numDays; k++)
            {
              y1 = points[k].angle;
              if(angleDiffDegrees(y1, yRx) > angleDiffDegrees(lastY, yRx)) { x1 = k; found = true; break; }
              lastY = y1;
            }
          if(!found) { x1 = numDays-1; y1 = points[numDays-1].angle; }
          else
            { // make sure closest point was found (+/- 1 day)
              double diffx1     = angleDiffDegrees(points[(int)x1].angle, yRx);
              double diffLower  = (x1 == 0               ? diffx1 : angleDiffDegrees(points[(int)x1-1].angle, yRx));
              double diffHigher = (x1 == numDays-1 ? diffx1 : angleDiffDegrees(points[(int)x1+1].angle, yRx));
              if(diffLower  < diffx1) { x1--; }
              if(diffHigher < diffx1) { x1++; }
            }

          rx.postShadowComplete = found;
          rx.shadowBox = Rect2d(Vec2d(x0, yDx), Vec2d(x1, yRx));
          rx.flipped   = flipped;
        }
    }
  std::cout << " --> Done\n";
  return rxData;
}

