#include "plotWidget.hpp"
using namespace astro;

#include <fstream>
#include <sstream>
#include <cfloat>        // (for DBL_MAX)
#include <bits/stdc++.h> // (for INT_MAX)
#include <algorithm>
#include <cctype>
#include <string>

#include <imgui.h>
#include "glfwKeys.hpp"
#include "viewSettings.hpp"


PlotWidget::PlotWidget()
{
  mMarketParams = new PlotDataParams();
  mVolumeParams = new PlotDataParams();
  mVolumeParams->plotType  = PLOT_BAR;
  mPlanetPosAxis.range   = Vec2d(0.0, 360.0);
  mAspectAxis.range      = Vec2d(0.0, 180.0);
  mPlanetSpeedAxis.range = Vec2d(-20.0, 20.0);
  
  mDateAxis.xAxis                = true;
  mDateAxis.drawLabels           = true;
  mDateAxis.labelInterval        = 365.0;

  mPlanetPosAxis.visible         = false;
  mPlanetPosAxis.drawLabels      = false;
  mPlanetPosAxis.labelMin        = 0.0;
  mPlanetPosAxis.labelMax        = 360.0;
  mPlanetPosAxis.labelInterval   = 30.0;
  
  mPlanetSpeedAxis.visible       = false;
  mPlanetSpeedAxis.drawLabels    = false;
  mPlanetSpeedAxis.labelMin      = -20.0;
  mPlanetSpeedAxis.labelMax      = 20.0;
  mPlanetSpeedAxis.labelInterval = 5.0;

  mAspectAxis.visible            = true;
  mAspectAxis.drawLabels         = false;
  mAspectAxis.drawLabelLines     = false;
  mAspectAxis.labelMin           = 0.0;
  mAspectAxis.labelMax           = 90.0;
  mAspectAxis.labelInterval      = 10.0;

  mMarketAxis.visible         = true;
  mMarketAxis.drawLabels      = true;
  mMarketAxis.drawLabelLines  = false;
  mMarketAxis.labelInterval   = 100.0;
  
  mVolumeAxis.visible         = false;
  mVolumeParams->visible      = false;
  mVolumeAxis.drawLabels      = false;
  
  mYAxes      = { &mPlanetPosAxis, &mPlanetSpeedAxis, &mAspectAxis, &mMarketAxis, &mVolumeAxis };
  mDataParams = { mMarketParams, mVolumeParams };
  mNeedUpdate = true;
}

PlotWidget::~PlotWidget()
{
  if(mMarketParams) { delete mMarketParams; }
  if(mVolumeParams) { delete mVolumeParams; }
}

void PlotWidget::addPlanet(const std::vector<PlanetPoint> *points, const std::vector<RxBox> *rx, PlanetDataParams *params)
{
  mPlanetData.push_back(points);
  mPlanetParams.push_back(params); mDataParams.push_back(params);
  mRetrogrades.push_back(rx);
  
  mPlanetPosAxis.dataMin   = 0.0;   mPlanetPosAxis.dataMax   = 360.0;
  mPlanetSpeedAxis.dataMin = 360.0; mPlanetSpeedAxis.dataMax = 0.0;
  for(int j = 0; j < mPlanetData.back()->size(); j++)
    {
      mPlanetSpeedAxis.dataMin = std::min(mPlanetSpeedAxis.dataMin, (*mPlanetData.back())[j].speed);
      mPlanetSpeedAxis.dataMax = std::max(mPlanetSpeedAxis.dataMax, (*mPlanetData.back())[j].speed);
      // NOTE: planet positions should always be 0 <= p <= 360
    }

  mNeedUpdate = true;
}

void PlotWidget::addAspect(const std::vector<double> *points, AspectDataParams *params)
{
  mAspectData.push_back(points);
  mAspectParams.push_back(params); mDataParams.push_back(params);
  // mAspectParams.back()->visible = false;
  mAspectAxis.dataMin = DBL_MAX; mAspectAxis.dataMax = 0.0;
  for(int j = 0; j < mAspectData.back()->size(); j++)
    {
      mAspectAxis.dataMax = std::max(mAspectAxis.dataMax, (*mAspectData.back())[j]);
      mAspectAxis.dataMin = std::min(mAspectAxis.dataMin, (*mAspectData.back())[j]);
    }
  mNeedUpdate = true;
}

void PlotWidget::clearPlanets()
{
  mPlanetData.clear();
  for(int i = 0; i < mPlanetParams.size(); i++)
    {
      for(int j = 0; j < mDataParams.size(); j++)
        {
          if(mDataParams[j] == mPlanetParams[i])
            { mDataParams.erase(mDataParams.begin() + j); break; }
        }
    }
  mPlanetParams.clear();
  mRetrogrades.clear();
  mNeedUpdate = true;
}

void PlotWidget::clearAspects()
{
  mAspectData.clear();
  for(int i = 0; i < mAspectParams.size(); i++)
    {
      for(int j = 0; j < mDataParams.size(); j++)
        {
          if(mDataParams[j] == mAspectParams[i])
            { mDataParams.erase(mDataParams.begin() + j); break; }
        }
    }
  mAspectParams.clear();
}

void PlotWidget::setDateRange(const DateTime &dtStart, const DateTime &dtEnd)
{
  mStartDate = dtStart; mEndDate = dtEnd; mNeedUpdate = true;
  updateDates();
}

void PlotWidget::updateDates()
{
  if(mNeedUpdate)
    {
      mStartDate.setHour(0); mStartDate.setMinute(0); mStartDate.setSecond(0);
      mEndDate.setHour(0); mEndDate.setMinute(0); mEndDate.setSecond(0);
      mNumDays = (int)mEndDate.diffDays(mStartDate);
      
      mDateAxis.dataMin = 0; mDateAxis.dataMax = mNumDays;
      if(chartCenter && chartDate >= mStartDate && chartDate <= mEndDate)
        {
          mDateAxis.center = chartDate.diffDays(mStartDate);
          if(mDateAxis.logScale) { mDateAxis.center = log(mDateAxis.center); }
        }
      mNeedUpdate = false;
    }
}

void PlotWidget::resetView()
{
  // calculate data max/min
  // mDateAxis.dataMin = 0; mDateAxis.dataMax = mNumDays;
  mDateAxis.range.x = 0; mDateAxis.range.y = mNumDays;
  // for(auto ax : mYAxes) { ax->dataMin = -DBL_MAX; ax->dataMax = DBL_MAX; }
  
  // for(int i = 0; i < mNumDays; i++)
  //   {
  //   }


  // for(int i = 0; i < mNumDays; i++)
  //   {
  // mDateAxis.dataMin        = 0.0;     mDateAxis.dataMax        = mNumDays;
  // mPlanetPosAxis.dataMin   = 0.0;     mPlanetPosAxis.dataMax   =  360.0;
  // mPlanetSpeedAxis.dataMin = 0.0;     mPlanetSpeedAxis.dataMax = -360.0;
  // mAspectAxis.dataMin      = 0.0;     mAspectAxis.dataMax      =  360.0;
  // {
  //   for(int j = 0; j < mPlanetData.size(); j++)
  //     {
  //       const PlanetPoint &p = (*mPlanetData[j])[i];
  //       mPlanetPosAxis.dataMax   = std::max(mPlanetPosAxis.dataMax,   p.angle);
  //       mPlanetPosAxis.dataMin   = std::min(mPlanetPosAxis.dataMin,   p.angle);
  //       mPlanetSpeedAxis.dataMax = std::max(mPlanetSpeedAxis.dataMax, p.speed);
  //       mPlanetSpeedAxis.dataMin = std::min(mPlanetSpeedAxis.dataMin, p.speed);
  //     }

  //   for(int j = 0; j < mAspectData.size(); j++)
  //     {
  //       const double &p = (*mAspectData[j])[i];
  //       mAspectAxis.dataMax = std::max(mAspectAxis.dataMax, p);
  //       mAspectAxis.dataMin = std::min(mAspectAxis.dataMin, p);
  //     }
  // }
  double p1 = mDateAxis.range.x - mDateAxis.rangeSize()*mPadRatio.x + mDateAxis.offset;
  double p2 = mDateAxis.range.y + mDateAxis.rangeSize()*mPadRatio.x + mDateAxis.offset;
  mDateAxis.center    = (chartCenter ? chartDate.diffDays(mStartDate) : (p1+p2)/2.0);
  mDateAxis.center    = std::max(0.0, std::min((double)mNumDays, mDateAxis.center));
  mDateAxis.viewSize  = p2 - p1;
  mDateAxis.viewScale = mScreenRect.size().x / mDateAxis.viewSize;
  //newXAxHeight = getXAxisHeight(); // TODO: recalculate axis sizes?
  
  for(auto ax : mYAxes)
    {
      p1 = ax->range.x - ax->rangeSize()*mPadRatio.y + ax->offset;
      p2 = ax->range.y + ax->rangeSize()*mPadRatio.y + ax->offset;
      ax->center    = (ax->logScale ? (p1+p2)-2.0 : (p1+p2)/2.0);
      ax->viewSize  = p2 - p1;
      ax->viewScale = mScreenRect.size().y / ax->viewSize;
    }
}

bool PlotWidget::handleIO()
{
  // io / control
  ImGuiIO &io = ImGui::GetIO();
  bool changed = false;
  
  if(mHovered && !ImGui::IsKeyDown(GLFW_KEY_LEFT_CONTROL) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    { mClicked = true; }
  else if(ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    { mClicked = false; }      
  if(mClicked && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
      Vec2d dmp = -Vec2d(ImGui::GetMouseDragDelta(ImGuiMouseButton_Left));
      ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);

      if(mLockX) { dmp.x = 0.0f; }
      if(mLockY) { dmp.y = 0.0f; }
      
      double dp = screenToPlotVecAx(dmp.x, &mDateAxis);
      mDateAxis.center = (mDateAxis.logScale ? log(exp(mDateAxis.center) + dp) : mDateAxis.center + dp);
      
      for(int i = 0; i < mYAxes.size(); i++)
        {
          dp = screenToPlotVecAx(dmp.y, mYAxes[i]);
          mYAxes[i]->center = (mYAxes[i]->logScale ? log(exp(mYAxes[i]->center) + dp) : mYAxes[i]->center + dp);
        }
      changed = true;
    }
  
  if(mHovered && !io.KeyCtrl && std::abs(io.MouseWheel) > 0.0f)
    {
      double  vel     = 0.95;
      Vec2f   mp      = ImGui::GetMousePos();
      // zoom/scale axes
      if(!mDateAxis.locked && !mLockX)      { mDateAxis.scale(io.MouseWheel > 0.0f ? vel : 1.0/vel, screenToPlotPosAx(mp.x, &mDateAxis)); }
      
      for(int i = 0; i < mYAxes.size(); i++)
        {
          if(!mYAxes[i]->locked && !mLockY) { mYAxes[i]->scale(io.MouseWheel > 0.0f ? vel : 1.0/vel, screenToPlotPosAx(mp.y, mYAxes[i])); }
        }
      changed = true;
    }
      
  if(mHovered && ImGui::IsKeyPressed(GLFW_KEY_ESCAPE)) { resetView(); }
  
  bool showPos     = false;
  bool showSpeeds  = false;
  bool showRx      = false;
  bool showShadows = false;
  bool showRxBoxes = false;
  for(int i = 0; i < mPlanetParams.size(); i++)
    {
      PlanetDataParams *params = mPlanetParams[i];
      showPos     |= params->showPos;
      showSpeeds  |= params->showSpeed;
      showRx      |= params->showRx;
      showShadows |= params->showShadows;
      showRxBoxes |= params->showRxBoxes;
    }

  std::string contextName = "##plotContext";
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,     TOOLTIP_PADDING);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,      TOOLTIP_SPACING);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, TOOLTIP_SPACING);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    TOOLTIP_PADDING*2.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,      TOOLTIP_PADDING);
  ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing,    TOOLTIP_SPACING.x);
  if(mHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    { ImGui::OpenPopup(contextName.c_str()); }

  if(ImGui::BeginPopup(contextName.c_str()))
    {
      mContextOpen = true;
      ImGui::SetWindowFontScale(1.0/mScale);
      ImGui::TextUnformatted("Planets");
      ImGui::Indent();
      if(ImGui::Checkbox("Show Positions",             &showPos))     { for(auto p : mPlanetParams) { p->showPos     = showPos;     } }
      if(ImGui::Checkbox("Show Speeds",                &showSpeeds))  { for(auto p : mPlanetParams) { p->showSpeed   = showSpeeds;  } }
      if(ImGui::Checkbox("Show Retrogrades",           &showRx))      { for(auto p : mPlanetParams) { p->showRx      = showRx;      } }
      if(ImGui::Checkbox("Show Shadow Periods",        &showShadows)) { for(auto p : mPlanetParams) { p->showShadows = showShadows; } }
      if(ImGui::Checkbox("Highlight Retrograde Boxes", &showRxBoxes)) { for(auto p : mPlanetParams) { p->showRxBoxes = showRxBoxes; } }
      ImGui::Checkbox("Show Aspects", &mAspectAxis.visible);
      
      ImGui::Unindent();
      ImGui::TextUnformatted("Market");
      ImGui::Indent();
      ImGui::Checkbox("Show Prices", &mMarketAxis.visible);
      ImGui::Checkbox("Show Volume", &mVolumeAxis.visible);
      ImGui::Unindent();
      ImGui::TextUnformatted("Axes");
      ImGui::Indent();
      ImGui::Checkbox("Lock X", &mLockX);
      ImGui::Checkbox("Lock Y", &mLockY);
      ImGui::Unindent();
      ImGui::EndPopup();
    }
  else if(mPlotHovered)
    { // draw tooltip
      mContextOpen = false;
      Vec2f mOffset(-1.0f, 0.0f);
      Vec2f mp = Vec2f(ImGui::GetMousePos()) + mOffset; mp = Vec2d(screenToPlotPosAx(mp.x, &mDateAxis), screenToPlotPosAx(mp.y, &mPlanetPosAxis));
      int dateIndex = std::floor(mp.x);
      DateTime dt = mStartDate;
      dt.setDay(dt.day() + dateIndex); dt.fix();
      DateTime dt2 = dt;
      dt2.setSecond((mp.x-dateIndex)*24.0f*60.0f*60.0f); dt2.fix();
      ImGui::BeginTooltip();
      ImGui::Text("%s", dt2.toString().c_str());
      if(dateIndex >= 0 && dateIndex < mNumDays)
        {
          for(int i = 0; i < mPlanetData.size(); i++)
            {
              PlanetDataParams *params = mPlanetParams[i];
              if(!mPlanetData[i] || dateIndex >= mPlanetData[i]->size()) { continue; }
              const PlanetPoint &p = (*mPlanetData[i])[dateIndex];
              ImGui::Text("%-16s %4s %11f°", (params->label + ":").c_str(), (p.speed < 0.0 ? "(Rx)" : ""), p.angle);
              ImGui::SameLine(); ImGui::Text(" | %11f °/day", p.speed);
            }
          for(int i = 0; i < mAspectData.size(); i++)
            {
              AspectDataParams *params = mAspectParams[i];
              if(!mAspectData[i] || dateIndex >= mAspectData[i]->size()) { continue; }
              double p = (*mAspectData[i])[dateIndex];

              std::string o1Name = getObjName(params->obj1);
              std::string o2Name = getObjName(params->obj2);
              std::string aspName = getAspectName(params->asp);
              std::string o1Rx = "";
              std::string o2Rx = "";
              
              if((int)params->obj1 > 0 && (int)params->obj1 < mPlanetData.size())
                { o1Rx = ((*mPlanetData[params->obj1])[dateIndex].speed < 0.0 ? "(Rx)" : ""); }
              if((int)params->obj2 > 0 && (int)params->obj2 < mPlanetData.size())
                { o2Rx = ((*mPlanetData[params->obj2])[dateIndex].speed < 0.0 ? "(Rx)" : ""); }
              ImGui::Text("%-16s%4s %-16s %-16s%4s (orb: %11f°)", o1Name.c_str(), o1Rx.c_str(), aspName.c_str(), o2Name.c_str(), o2Rx.c_str(), p);
            }
        }

      if(mMarketData && mMarketAxis.visible)
        {
          std::string dateStr = marketDateStr(dt);
          std::cout << "DATE STR: " << dateStr << "\n";
          const auto &iter = mMarketData->find(dateStr);
          if(iter != mMarketData->end())
            {
          
          // int marketIndex = std::round(dt.diffDays(mMarketStartDate));
          //for(int i = 0; i < mMarketData->size(); i++)
          // if(marketIndex > 0 && marketIndex < mMarketData->size())
          //   {
              const MarketPoint &p = iter->second; //mMarketData[marketIndex];
              // const MarketPoint &q = mMarketData[marketIndex == 0 ? marketIndex : marketIndex-1];

              std::cout << "OPEN: " << p.open << ", CLOSE: " << p.close << "\n";

              double before = p.open;
              if(p.prevDate > marketDateStr(mMarketStartDate))
                {
                  std::string prevDateStr = marketDateStr(p.prevDate);
                  DateTime prevDate       = marketDate(prevDateStr);
                  auto prevIter           = mMarketData->find(prevDateStr);

                  while(prevIter == mMarketData->end() && prevDate > mMarketStartDate)
                    {
                      prevDate.setDay(prevDate.day()-1); prevDate.fix();
                      prevDateStr = marketDateStr(prevDate);
                      prevIter    = mMarketData->find(prevDateStr);
                    }
                  
                  if(prevIter != mMarketData->end())
                    {
                      const MarketPoint &q = prevIter->second;
                      before = q.close;
                      
                      std::cout << "BEFORE CLOSE: " << before << "\n";
                    }
                }
                  
              ImGui::Text("Market Change: %f%%", 100.0f*(p.close - before)/before);
              ImGui::Text("Market Open:   %f",   p.open);
              ImGui::Text("Market Close:  %f",   p.close);
              ImGui::Text("Market High:   %f",   p.high);
              ImGui::Text("Market Low:    %f",   p.low);
              ImGui::Text("Market Volume: %.0f", p.volume);
              // if(debug) { ImGui::Text("marketIndex=%d, p.dateIndex=%d", marketIndex, p.dateIndex); }
            }
        }
      ImGui::EndTooltip();
    }
  else { mContextOpen = false; }
  ImGui::PopStyleVar(6);
  mPlanetPosAxis.visible   = showPos;
  mPlanetSpeedAxis.visible = showSpeeds;
  
  return changed;
}

Vec2f PlotWidget::drawLegend(ViewSettings *vs, bool blocked, float scale)
{
  ImDrawList *nodeDrawList = ImGui::GetWindowDrawList();
  Vec2f p0 = ImGui::GetCursorScreenPos();
  float symSize       = 16.0f*scale;
  Vec2f framePadding  = LEGEND_FRAME_PADDING*scale;
  Vec2f itemSpacing   = Vec2f(LEGEND_ITEM_SPACING, LEGEND_ITEM_SPACING)*scale;
  Vec2f legendSize;
  
  int numLabels = 0;
  for(auto p : mPlanetParams) { numLabels += (p->label.empty() ? 0 : 1); }

  ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, itemSpacing);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, itemSpacing);
  
  if(numLabels > 0)
    { // draw legend

      // title
      ImGui::PushFont(vs->titleFont);
      Vec2f titleSize = ImGui::CalcTextSize(LEGEND_TITLE);
      ImGui::PopFont();

      // checkbox labels
      std::string cbLabels = "P";
    
      legendSize = Vec2f(titleSize.x, 2.0f*framePadding.y + titleSize.y);
      for(int i = 0; i < mPlanetData.size(); i++)
        {
          Vec2f tSize = ImGui::CalcTextSize(mPlanetParams[i]->label.c_str());
          legendSize.x = std::max(legendSize.x, tSize.x);
          legendSize.y += tSize.y + itemSpacing.y;
        }
      legendSize += Vec2f(2.0f*framePadding.x, 0.0f);

      Vec2f colorSquareSize = Vec2f(symSize, symSize);
      legendSize.x += itemSpacing.x*3.0f + colorSquareSize.x + symSize + 5.0f*(symSize + itemSpacing.x);

      Vec2f p1 = p0 + framePadding;
    
      nodeDrawList->AddRectFilled(p1, p1+legendSize, ImColor(mBgColor));

      ImGui::SetCursorScreenPos(p1 + Vec2f((legendSize.x - titleSize.x)/2.0f, framePadding.y));
      ImGui::PushFont(vs->titleFont);
      ImGui::TextUnformatted(LEGEND_TITLE);
      ImGui::PopFont();
      
      ImGui::SetCursorScreenPos(Vec2f(ImGui::GetCursorScreenPos())+Vec2f(2.0f*framePadding.x, 0));//p1 + Vec2f(0, titleSize.y) + framePadding);
      ImGui::BeginGroup();
      {
        mAnyFocused = false;
        for(int i = 0; i < mPlanetParams.size(); i++)
          {
            PlanetDataParams *params = mPlanetParams[i];
            Vec2f p = ImGui::GetCursorScreenPos();
            ImGui::BeginGroup();
            {
              ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0,0));
              ImGui::SetNextItemWidth(symSize);
              // ImGui::Checkbox(("##" + params->label + "Vis").c_str(), &params->visible);
              // ImGui::SameLine(); ImGui::SetNextItemWidth(symSize);
              ImGui::Checkbox(("##" + std::to_string(i) + "Pos").c_str(),    &params->showPos);
              ImGui::SameLine(); ImGui::SetNextItemWidth(symSize);
              ImGui::Checkbox(("##" + std::to_string(i) + "Speed").c_str(),  &params->showSpeed);
              ImGui::SameLine(); ImGui::SetNextItemWidth(symSize);
              ImGui::Checkbox(("##" + std::to_string(i) + "Rx").c_str(),     &params->showRx);
              ImGui::SameLine(); ImGui::SetNextItemWidth(symSize);
              ImGui::Checkbox(("##" + std::to_string(i) + "Shadow").c_str(), &params->showShadows);
              ImGui::SameLine(); ImGui::SetNextItemWidth(symSize);
              ImGui::Checkbox(("##" + std::to_string(i) + "Box").c_str(),    &params->showRxBoxes);
              ImGui::PopStyleVar();
              ImGui::SameLine();
              p = ImGui::GetCursorScreenPos();
              nodeDrawList->AddRectFilled(p, p + colorSquareSize, ImColor(params->color));
              ImGui::SetCursorScreenPos(p + Vec2f(colorSquareSize.x + itemSpacing.x, 0));
              ChartImage *img = getWhiteImage(params->label);
              if(img)
                {
                  ImGui::Image(img->id(), Vec2f(symSize, symSize), Vec2f(0,0), Vec2f(1,1), ImColor(params->color), Vec4f(0,0,0,0));
                  ImGui::SameLine();
                }
              ImGui::TextUnformatted(params->label.c_str());
              // ImGui::SetCursorScreenPos(Vec2f(ImGui::GetCursorScreenPos()) + Vec2f(colorSquareSize.x + itemSpacing.x, 0));
            }
            ImGui::EndGroup();
            
            params->focused = (ImGui::IsItemHovered() && (ImGui::IsKeyDown(GLFW_KEY_LEFT_SHIFT) || ImGui::IsKeyDown(GLFW_KEY_RIGHT_SHIFT)));
            mAnyFocused |= params->focused;
          }
      }
      ImGui::EndGroup();
      legendSize = Vec2f(ImGui::GetItemRectMax()) - ImGui::GetItemRectMin() + 2.0f*framePadding;
    }
  ImGui::PopStyleVar(2);
  return legendSize;
}

float PlotWidget::getXAxisHeight(ViewSettings *vs, float scale, const DateAxis *axis)
{
  Vec2f framePadding  = AXIS_FRAME_PADDING*scale;

  int rows = 3;
  if(!axis->dayLabels)
    {
      rows--;
      if(!axis->monthLabels)
        {
          rows--;
          if(!axis->yearLabels) { rows--; }
        }
    }
  
  return (float)rows*(vs->mainTextSize*scale + framePadding.y) + 2.0f*framePadding.y;
}

float PlotWidget::getYAxisWidth(ViewSettings *vs, float scale, const Axis *axis)
{
  Vec2f framePadding  = AXIS_FRAME_PADDING*scale;
  std::stringstream ss; ss << (int)std::max(axis->dataMax, axis->labelMax);
  return ImGui::CalcTextSize(ss.str().c_str()).x + framePadding.x;
}


std::vector<AxisLabel> PlotWidget::getXAxisLabels(ViewSettings *vs, float scale, DateAxis *axis)
{
  if(!axis || !axis->drawLabels) { return { }; }
  ImDrawList *axDrawList = ImGui::GetWindowDrawList();
  Vec2f framePadding  = AXIS_FRAME_PADDING*scale;
  
  double interval  = axis->labelInterval;
  double start     = axis->p1();
  double end       = axis->p2();//std::ceil(axis->p2()/interval)*interval;
  // start = std::floor(start/interval)*interval;
  // end   = std::ceil(end/interval)*interval;
  if(axis->labelMax != 0.0) { end = std::min(end, std::ceil(axis->labelMax/interval)*interval); }

  //DateTime dtStart = mStartDate; dt0.setDay(dt0.day() + std::floor(start)); dt0.setHour(0); dt0.setMinute(0); dt0.setSecond(0.0); dt0.fix();
  DateTime dt0 = mStartDate; dt0.setDay(dt0.day() + std::floor(start)); dt0.fix();//dt0.setHour(0); dt0.setMinute(0); dt0.setSecond(0.0); dt0.fix();
  //dt0.setSecond(dt0.second() + (start - std::floor(start))); dt0.fix();
  DateTime dt1 = mStartDate; dt1.setDay(dt1.day() + std::ceil(end));    dt1.fix(); //dt1.setHour(0); dt1.setMinute(0); dt1.setSecond(0.0);
  //dt1.setSecond(dt1.second() + (end   - std::ceil(end)));    dt1.fix();

  int numDays   = (int)dt1.diffDays(dt0);
  int numMonths = (dt1.year() - dt0.year())*12 + (dt1.month() - dt0.month());
  int numYears  = (dt1.year() - dt0.year());

  if(numYears <= 0) { numYears = 1; }
  float dtMaxW = Vec2f(ImGui::CalcTextSize("00")).x;
  float mtMaxW = Vec2f(ImGui::CalcTextSize("XXX")).x;
  float ytMaxW = Vec2f(ImGui::CalcTextSize("0000")).x + framePadding.x;
  axis->dayLabels   = mScreenRect.size().x/numDays   > dtMaxW;
  axis->monthLabels = mScreenRect.size().x/numMonths > mtMaxW;
  axis->yearLabels  = mScreenRect.size().x/numYears  > ytMaxW;

  int yearStep = 1;
  while(!axis->yearLabels)
    {
      yearStep++;
      axis->yearLabels = mScreenRect.size().x/(numYears/yearStep) > ytMaxW;
    }
  while(dt0.year() % yearStep != 0) { dt0.setYear(dt0.year() + 1); }
  
  int numLabels = 0;
  if(axis->dayLabels)        { numLabels = numDays; }
  else if(axis->monthLabels) { numLabels = numMonths; }
  else if(axis->yearLabels)  { numLabels = numYears/yearStep; }
  DateTime dt = dt0;
  if(axis->dayLabels)                                     // set to beginning of day
    { dt.set(dt0.year(), dt0.month(), dt0.day(), 0, 0, 0.0); }
  if(axis->monthLabels && !axis->dayLabels)                     // set to beginning of month
    { dt.set(dt0.year(), dt0.month(), 1, 0, 0, 0.0); }
  else if(axis->yearLabels && !axis->monthLabels && !axis->dayLabels) // set to beginning of year
    { dt.set(dt0.year(), 1, 1, 0, 0, 0.0); }

  std::vector<AxisLabel> labels; labels.reserve(numLabels);
  for(int i = 0; i < numLabels; i++)
    {
      int year = dt0.year(); int month = dt0.month(); int day = dt0.day();
      if(axis->dayLabels)        { dt.setDay(dt.day() + 1);     dt.fix(); }
      else if(axis->monthLabels) { dt.setMonth(dt.month() + 1); dt.fix(); dt.setDay(1); }
      else if(axis->yearLabels)  { dt.setYear(dt.year() + yearStep); dt.setMonth(1); dt.setDay(1); }
      year = dt.year(); month = dt.month(); day = dt.day();
      
      bool ly = ((axis->yearLabels && !axis->monthLabels && !axis->dayLabels) ||
                 ((month == 1 && day == 1) && (axis->monthLabels || axis->dayLabels)));       // need year label
      bool lm = ((axis->monthLabels && !axis->dayLabels) || (axis->dayLabels  && day == 1)); // need month label
      bool ld = axis->dayLabels;                                                              // need day label
      
      double pp = std::round(dt.diffDays(mStartDate));
      Vec2f  sp = Vec2f(plotToScreenPosAx(pp, axis), mScreenRect.p2.y + framePadding.y);

      std::stringstream ss;
      Vec2f ttSize = 2.0f*framePadding;
      std::vector<std::string> text;
      if(ld)
        {
          std::stringstream ss; ss << day;
          text.push_back(ss.str());
          Vec2f dtSize = ImGui::CalcTextSize(ss.str().c_str());
          // ImGui::SetCursorScreenPos(sp + Vec2f(-dtSize.x/2.0f, ttSize.y));
          // ImGui::TextUnformatted(ss.str().c_str());
          ttSize = Vec2f(std::max(ttSize.y, dtSize.y), ttSize.y + dtSize.y + framePadding.y);
        }
      if(lm)
        {
          std::stringstream ss; ss << DateTime::monthNameAbbrev(month);
          text.push_back(ss.str());
          Vec2f mtSize = ImGui::CalcTextSize(ss.str().c_str());
          // ImGui::SetCursorScreenPos(sp + Vec2f(-mtSize.x/2.0f, ttSize.y));
          // ImGui::TextUnformatted(ss.str().c_str());
          ttSize = Vec2f(std::max(ttSize.y, mtSize.y), ttSize.y + mtSize.y + framePadding.y);
        }
      if(ly)
        {
          std::stringstream ss; ss << std::setfill('0') << std::setw(4) << year << std::setfill(' ');
          text.push_back(ss.str());
          Vec2f ytSize = ImGui::CalcTextSize(ss.str().c_str());
          // ImGui::SetCursorScreenPos(sp + Vec2f(-ytSize.x/2.0f, ttSize.y));
          // ImGui::TextUnformatted(ss.str().c_str());
          ttSize = Vec2f(std::max(ttSize.x, ytSize.x), ttSize.y + ytSize.y + framePadding.y);
        }

      // add tick mark
      // axDrawList->AddLine(sp, Vec2f(sp.x, mScreenRect.p1.y), ImColor(1.0f, 1.0f, 1.0f, 1.0f), 1.0f);
      
      AxisLabel l = AxisLabel{ text, pp, sp, ttSize };
      labels.push_back(l);
    }
  return labels;
}


void PlotWidget::drawXAxisLabels(ViewSettings *vs, float scale, DateAxis *axis, const std::vector<AxisLabel> &labels)
{
  if(!axis || !axis->drawLabels) { return; }
  ImDrawList *axDrawList = ImGui::GetWindowDrawList();
  Vec2f framePadding  = AXIS_FRAME_PADDING*scale;
  
  int numLabels = labels.size();
  for(int i = 0; i < numLabels; i++)
    {
      const AxisLabel &l = labels[i];

      Vec2f ttSize = framePadding;
      for(const auto &line : l.text)
        {
          Vec2f tSize = ImGui::CalcTextSize(line.c_str());
          ImGui::SetCursorScreenPos(l.screenPos + Vec2f(-tSize.x/2.0f, ttSize.y));
          ImGui::TextUnformatted(line.c_str());
          ttSize = Vec2f(std::max(ttSize.x, tSize.x), ttSize.y + tSize.y + framePadding.y);
        }
      // add tick mark
      axDrawList->AddLine(l.screenPos, Vec2f(l.screenPos.x, mScreenRect.p1.y), ImColor(1.0f, 1.0f, 1.0f, 1.0f), 1.0f);
    }
}


std::vector<AxisLabel> PlotWidget::getYAxisLabels(ViewSettings *vs, float scale, float xOffset, Axis *axis, bool right)
{
  if(!axis || !axis->drawLabels) { return { }; }
  ImDrawList *axDrawList = ImGui::GetWindowDrawList();
  Vec2f axisSize     = Vec2f(getYAxisWidth(vs, scale, axis), mScreenRect.size().y);
  Vec2f framePadding = AXIS_FRAME_PADDING*scale;
  
  double start     = axis->labelMin;
  double interval  = axis->labelInterval;
  double end       = std::ceil(axis->p2()/interval)*interval;
  if(std::isnan(start) || std::isnan(end) || std::isnan(end)) { return { }; }
  if(axis->labelMax != 0.0) { end = std::min(end, std::ceil(axis->labelMax/interval)*interval); }
  while(start < axis->p1()) { start += interval; }
  if(start > end) { return { }; }
  
  int numLabels = std::ceil((end-start)/interval) + 1;

  std::vector<AxisLabel> labels; labels.reserve(numLabels);  
  for(int i = 0; i < numLabels; i++)
    {      
      double pp = start + i*interval;
      Vec2f  sp = Vec2f(mOuterRect.p1.x + framePadding.x + xOffset+axisSize.x, plotToScreenPosAx(pp, axis));

      std::stringstream ss; ss << (int)pp;
      Vec2f ttSize = Vec2f(ImGui::CalcTextSize(ss.str().c_str())) + Vec2f(framePadding.x, 0);
      
      // ImGui::SetCursorScreenPos(sp - Vec2f(ttSize.x, ttSize.y/2.0f));
      // ImGui::TextUnformatted(ss.str().c_str());

      // // add tick mark
      // axDrawList->AddLine(sp, Vec2f(mScreenRect.p1.x, sp.y), ImColor(1.0f, 1.0f, 1.0f, 1.0f), 1.0f);
      
      AxisLabel l = AxisLabel{ {ss.str()}, pp, sp, ttSize };
      labels.push_back(l);
    }
  return labels;//axisSize;
}

void PlotWidget::drawYAxisLabels(ViewSettings *vs, float scale, Axis *axis, const std::vector<AxisLabel> &labels, bool right)
{
  if(!axis || !axis->drawLabels) { return; }
  ImDrawList *axDrawList = ImGui::GetWindowDrawList();
  Vec2f axisSize     = Vec2f(getYAxisWidth(vs, scale, axis), mScreenRect.size().y);
  Vec2f framePadding = AXIS_FRAME_PADDING*scale;
  
  // double start     = axis->labelMin;
  // double interval  = axis->labelInterval;
  // double end       = std::ceil(axis->p2()/interval)*interval;
  // if(axis->labelMax != 0.0) { end = std::min(end, std::ceil(axis->labelMax/interval)*interval); }
  // int    numLabels = std::ceil((end-start)/interval) + 1;
  int numLabels = labels.size();
  for(int i = 0; i < numLabels; i++)
    {
      const AxisLabel &l = labels[i];
      // double pp = start + i*interval;
      // Vec2f  sp = Vec2f(mOuterRect.p1.x + framePadding.x + xOffset+axisSize.x, plotToScreenPosAx(pp, axis));

      // std::stringstream ss; ss << (int)pp;
      // Vec2f ttSize = Vec2f(ImGui::CalcTextSize(ss.str().c_str())) + Vec2f(framePadding.x, 0);

      for(const auto &line : l.text)
        {
          ImGui::SetCursorScreenPos(l.screenPos - Vec2f(l.textSize.x, l.textSize.y/2.0f));
          ImGui::TextUnformatted(line.c_str());
        }
      // add tick mark
      axDrawList->AddLine(l.screenPos, Vec2f(mScreenRect.p1.x, l.screenPos.y), ImColor(1.0f, 1.0f, 1.0f, 1.0f), 1.0f);
    }
}

Vec2f PlotWidget::draw(ViewSettings *vs, bool blocked, float scale, const Vec2f &size)
{
  mHovered      = false;
  mPlotHovered  = false;
  mXAxisHovered = false;
  mYAxisHovered = false;
  mScale = scale;
  
  ImDrawList *nodeDrawList = ImGui::GetWindowDrawList();
  Vec2f framePadding  = FRAME_PADDING*mScale;
  Vec2f p0 = ImGui::GetCursorScreenPos();

  Vec2f newSize = size;
  
  // draw title
  Vec2f titleSize;
  if(!mTitle.empty())
    {
      ImGui::PushFont(vs->titleFontB);
      titleSize = ImGui::CalcTextSize(mTitle.c_str());
      ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f((size.x*mScale-titleSize.x)/2.0f, framePadding.y));
      ImGui::TextUnformatted(mTitle.c_str());
      ImGui::PopFont();
      ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()));
      p0 = ImGui::GetCursorScreenPos();
    }
  
  // draw legend
  Vec2f legendSize = drawLegend(vs, blocked, mScale);

  // calculate scaling / rects
  mOuterRect = Rect2d(p0 + Vec2d(2.0f*framePadding.x + legendSize.x, 0.0f),
                      p0 + size*mScale - Vec2d(framePadding.x + legendSize.x, 1.5f*framePadding.y)); // graph+axes
  mScreenRect = mOuterRect;
  float xAxHeight = getXAxisHeight(vs, mScale, &mDateAxis);
  mScreenRect.p2.y -= xAxHeight;
  mScreenRect.p1.x += AXIS_FRAME_PADDING.x*mScale;
  for(auto ax : mYAxes) { if(ax->drawLabels) { mScreenRect.p1.x += getYAxisWidth(vs, mScale, ax) + AXIS_FRAME_PADDING.x*mScale; } }
  mScreenRect.p1.x += framePadding.x;
  // adjust axis scale so data stays centered
  if(mScreenRect.size() != mLastScreenSize*mScale && !mFirstFrame)
    {
      Vec2d dSize = (mScreenRect.size() - mLastScreenSize*mScale);
      double diff = screenToPlotVec(dSize, &mDateAxis).x;
      mDateAxis.viewSize += diff; mDateAxis.center += diff/2.0;
      // mDateAxis.setSize(mDateAxis.viewSize - diff);
      //mScreenRect.p2.y -= (getXAxisHeight(vs, mScale, &mDateAxis) - xAxHeight);
      
      for(auto ax : mYAxes)
        {
          // ax->setSize(ax->viewSize + diff);
          if(ax->logScale)
            {
              diff = screenToPlotVec(dSize, ax).y;
              ax->viewSize -= diff;
              // ax->center = log(ax->center + exp(diff))/2.0;
            }
          else
            {
              diff = screenToPlotVec(dSize, ax).y;
              ax->viewSize -= diff;
              ax->center += diff/2.0;
            }
        }
    }
  mDateAxis.center    = std::max(-8000.0*DAYS_PER_JULIAN_YEAR,  std::min(8000.0*DAYS_PER_JULIAN_YEAR,  mDateAxis.center));   // [-8000years, 8000years]
  mDateAxis.viewSize  = std::max(1.0,                           std::min(16000.0*DAYS_PER_JULIAN_YEAR, mDateAxis.viewSize)); // [1day, 16000years]
  mDateAxis.viewScale = mScreenRect.size().x / mDateAxis.viewSize;
  for(auto ax : mYAxes)
    {
      // if(ax->logScale) { ax->viewScale = mScreenRect.size().y / ax->viewSize; }
      // else             { ax->viewScale = mScreenRect.size().y / ax->viewSize; }
      ax->viewScale = mScreenRect.size().y / ax->viewSize;
    }
  mLastScreenSize = mScreenRect.size()/mScale;
  mFirstFrame = false;

  newSize.x = std::max(newSize.x, (float)(mScreenRect.size().x+2.0f*framePadding.x));
  newSize.y = std::max(newSize.y, std::max((float)mScreenRect.size().y, legendSize.y));
  
  // get X labels
  std::vector<AxisLabel> xLabels = getXAxisLabels(vs, mScale, &mDateAxis);
  xAxHeight = 0.0f; for(auto &l : xLabels) { xAxHeight = std::max(xAxHeight, l.textSize.y); }
  // get Y axis labels
  std::vector<std::vector<AxisLabel>> yLabels(mYAxes.size());
  float yAxWidth  = 4.0*AXIS_FRAME_PADDING.x*mScale;
  for(int i = 0; i < mYAxes.size(); i++)
    {
      yLabels[i] = getYAxisLabels(vs, mScale, yAxWidth, mYAxes[i]);
      float maxSize = 0.0f;
      for(auto &l : yLabels[i]) { maxSize = std::max(maxSize, l.textSize.x); }
      if(mYAxes[i]->drawLabels) { yAxWidth += maxSize + AXIS_FRAME_PADDING.x*mScale; } //getYAxisWidth(vs, mScale, ax) + AXIS_FRAME_PADDING.x*mScale; }
    }
  
  if(chartCenter) { mDateAxis.center = chartDate.diffDays(mStartDate); }
    
  ImGui::SetCursorScreenPos(mOuterRect.p1);
  ImGui::PushStyleColor(ImGuiCol_ChildBg, mAxisBgColor);
  bool visible = ImGui::BeginChild("##plotChild", mOuterRect.size(), true, ImGuiWindowFlags_NoDecoration);
  ImGui::PopStyleColor();
  if(visible)
   { 
      ImGui::SetWindowFontScale(mScale);
      mHovered |= ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
      mXAxisHovered = !mScreenRect.contains(ImGui::GetMousePos()) && ImGui::GetMousePos().y >= mScreenRect.p2.y;
      mYAxisHovered = !mScreenRect.contains(ImGui::GetMousePos()) && ImGui::GetMousePos().x <= mScreenRect.p1.x;

      // draw axis labels
      drawXAxisLabels(vs, mScale, &mDateAxis, xLabels);
      for(int i = 0; i < mYAxes.size(); i++)
        { drawYAxisLabels(vs, mScale, mYAxes[i], yLabels[i]); }
      
      ImGui::SetCursorScreenPos(mScreenRect.p1);
      ImGui::PushStyleColor(ImGuiCol_ChildBg, mBgColor);
      bool graphVisible = ImGui::BeginChild("##graphChild", mScreenRect.size(), false, ImGuiWindowFlags_NoDecoration);
      ImGui::PopStyleColor();
      if(graphVisible)
        {
          ImGui::SetWindowFontScale(mScale);
          mPlotHovered = ImGui::IsWindowHovered();
          ImDrawList *graphDrawList = ImGui::GetWindowDrawList();

          float dateLineW = 2.0f;
          float yLineW    = 1.0f;
          
          // draw axis lines
          int numLines = xLabels.size();
          for(int i = 0; i < numLines; i++)
            {
              AxisLabel &l = xLabels[i];
              graphDrawList->AddLine(Vec2f(l.screenPos.x, mScreenRect.p1.y),
                                     Vec2f(l.screenPos.x, mScreenRect.p2.y),
                                     ImColor(Vec4f(0.5f, 0.5f, 0.5f, 0.5f)), dateLineW);
            }
          
          for(int i = 0; i < mYAxes.size(); i++)
            {
              if(!mYAxes[i]->drawLabelLines || !mYAxes[i]->drawLabels) { continue; }
              numLines = yLabels[i].size();
              for(int j = 0; j < numLines; j++)
                {
                  AxisLabel &l = yLabels[i][j];
                  Vec2f pp = Vec2f(mScreenRect.p1.x, plotToScreenPosAx(l.plotPos, mYAxes[i]));
                  graphDrawList->AddLine(Vec2f(mScreenRect.p1.x, pp.y),
                                         Vec2f(mScreenRect.p2.x, pp.y),
                                         ImColor(Vec4f(0.5f, 0.5f, 0.5f, 1.0f)), yLineW);
                }
            }

          int startIndex = std::max((int)std::floor(mDateAxis.p1()), 0);
          int endIndex   = std::min((int)std::ceil(mDateAxis.p2()), mNumDays-1);
          
          // draw planet positions/speeds
          Vec4f defaultColor = Vec4f(0.9, 0.9, 0.9, 1.0);
          for(int i = 0; i < mPlanetData.size(); i++)
            {
              PlanetDataParams *params = mPlanetParams[i];
              Vec4f col = params->color;
              float wid = params->width/2.0f;
              if(!params->visible && !params->focused) { continue; }
              if(mPlanetData[i] && mPlanetData[i]->size() > 0 && (!mAnyFocused || params->focused))
                {
                  for(int j = startIndex; j < endIndex; j++)
                    {
                      const PlanetPoint &p = (*mPlanetData[i])[j];
                      const PlanetPoint &q = (*mPlanetData[i])[j+1];
                      Vec2d pp0 = Vec2d(j, p.angle);
                      Vec2d pp1 = Vec2d(j+1, q.angle);
                      
                      if(params->showPos && mPlanetPosAxis.visible)
                        {
                          if(std::abs(pp1.y - pp0.y) > 180.0f)
                            { // draw dotted line if angle passes 0 degrees
                              if(params->connectBreaks)
                                {
                                  Vec2d diff = pp1 - pp0;
                                  for(int k = 0; k < 32; k++)
                                    {
                                      Vec2d ppp0 = pp0 + (double)k/32.0f*diff;
                                      Vec2d ppp1 = pp0 + (double)(k+1)/32.0f*diff;
                                      Vec2d diff2 = ppp1-ppp0;
                                      ppp0 += diff2*0.3f;
                                      ppp1 -= diff2*0.3f;
                                      graphDrawList->AddLine(plotToScreenPos(ppp0, &mPlanetPosAxis),
                                                             plotToScreenPos(ppp1, &mPlanetPosAxis), ImColor(col), wid/2.0f);
                                    }
                                }
                            }
                          else
                            { graphDrawList->AddLine(plotToScreenPos(pp0, &mPlanetPosAxis), plotToScreenPos(pp1, &mPlanetPosAxis), ImColor(col), wid); }
                        }
                      if(params->showSpeed && mPlanetSpeedAxis.visible)
                        { // dashed line
                          Vec2d pp0 = Vec2d(j, p.speed);
                          Vec2d pp1 = Vec2d(j+1, q.speed);
                          Vec2d diff = pp1 - pp0;
                          pp0 += diff*0.3f;
                          pp1 -= diff*0.3f;
                          graphDrawList->AddLine(plotToScreenPos(pp0, &mPlanetSpeedAxis), plotToScreenPos(pp1, &mPlanetSpeedAxis),
                                                 ImColor(params->color), params->width/2.0f);
                        }
                    }
                }

              for(int j = 0; j < mRetrogrades[i]->size(); j++)
                {
                  const RxBox &rx = (*mRetrogrades[i])[j];
                  
                  // highlight shadow boxes
                  if(params->showRxBoxes) 
                    {
                      if(!rx.flipped)
                        {
                          graphDrawList->AddRectFilled(plotToScreenPos(rx.shadowBox.p1, &mPlanetPosAxis),
                                                       plotToScreenPos(rx.shadowBox.p2, &mPlanetPosAxis),
                                                       ImColor(col.x, col.y, col.z, 0.3f));
                        }
                      else
                        { // rectangle overlaps 0 degrees -- draw 2 rects
                          Rect2d r1(rx.shadowBox.p1, Vec2d(rx.shadowBox.p2.x, 360.0f));
                          Rect2d r2(Vec2d(rx.shadowBox.p1.x, 0.0f), rx.shadowBox.p2);
                          graphDrawList->AddRectFilled(plotToScreenPos(r1.p1, &mPlanetPosAxis),
                                                       plotToScreenPos(r1.p2, &mPlanetPosAxis),
                                                       ImColor(col.x, col.y, col.z, 0.3f));
                          graphDrawList->AddRectFilled(plotToScreenPos(r2.p1, &mPlanetPosAxis),
                                                       plotToScreenPos(r2.p2, &mPlanetPosAxis),
                                                       ImColor(col.x, col.y, col.z, 0.3f));
                        }
                    }
                  Rect2d sr(screenToPlotPos(mScreenRect.p1, &mPlanetPosAxis), screenToPlotPos(mScreenRect.p2, &mPlanetPosAxis));
                  
                  // draw shadow periods
                  if(params->showShadows)
                    { 
                      Rect2d s1r(Vec2d(rx.rxBox.p1.x, sr.p1.y),     Vec2d(rx.shadowBox.p1.x, sr.p2.y));
                      Rect2d s2r(Vec2d(rx.shadowBox.p2.x, sr.p1.y), Vec2d(rx.rxBox.p2.x,     sr.p2.y));

                          graphDrawList->AddRectFilled(plotToScreenPos(s1r.p1, &mPlanetPosAxis), plotToScreenPos(s1r.p2, &mPlanetPosAxis),
                                                       ImColor(col.x, col.y, col.z, 0.15f));
                          graphDrawList->AddRectFilled(plotToScreenPos(s2r.p1, &mPlanetPosAxis), plotToScreenPos(s2r.p2, &mPlanetPosAxis),
                                                       ImColor(col.x, col.y, col.z, 0.15f));
                    }
                  
                  // draw retrograde
                  if(params->showRx)
                    {
                      Rect2d s1r(Vec2d(rx.rxBox.p1.x, sr.p1.y), Vec2d(rx.rxBox.p2.x, sr.p2.y));
                      
                      graphDrawList->AddRectFilled(plotToScreenPos(s1r.p1, &mPlanetPosAxis),
                                                   plotToScreenPos(s1r.p2, &mPlanetPosAxis),
                                                   ImColor(col.x, col.y, col.z, 0.25f));
                    }
                }
            }

          for(int i = 0; i < mAspectData.size(); i++)
            {
              AspectDataParams *params = mAspectParams[i];
              if(!mAspectData[i] || !params) { continue; }
              if(!(params->visible && mAspectAxis.visible) && !params->focused) { continue; }
              for(int j = startIndex; j < endIndex-1; j++)
                {
                  if(j >= mAspectData[i]->size()) { continue; }
                  std::string o1Name = getObjName(params->obj1);
                  std::string o2Name = getObjName(params->obj2);
              
                  double p = (*mAspectData[i])[j];
                  double q = (*mAspectData[i])[j+1];
                  Vec2d pp0 = Vec2d(j,   p);
                  Vec2d pp1 = Vec2d(j+1, q);
                  Vec4f col = params->color;
                  float wid = params->width/2.0f;
                  Vec2d spp0 = plotToScreenPos(pp0, &mAspectAxis);
                  Vec2d spp1 = plotToScreenPos(pp1, &mAspectAxis);
                  if(params->showOrb)
                    { graphDrawList->AddLine(plotToScreenPos(pp0, &mAspectAxis), plotToScreenPos(pp1, &mAspectAxis), ImColor(col), wid); }
                }
            }
          
          if(drawChartDate)
            { // indicate date of connected chart
              Vec4f chartDateCol(1.0, 1.0, 1.0, 1.0);
              Vec2f pp1(plotToScreenPosAx(chartDate.diffDays(mStartDate), &mDateAxis), mScreenRect.p1.y);
              Vec2f pp2(pp1.x, mScreenRect.p2.y);
              graphDrawList->AddLine(pp1, pp2, ImColor(chartDateCol), 2.0f);
            }

          if(mMarketData)
            {
              // plot market data
              Vec4f marketBullColor(0.0f, 1.0f,  0.0f,  1.0f); // market candle color on bullish days
              Vec4f marketBearColor(1.0f, 0.0f,  0.0f,  1.0f); // market candle color on bearish days
              Vec4f marketFlatColor(0.8f, 0.8f,  0.8f,  1.0f); // market candle color on flat days
              Vec4f marketWickColor(1.0f, 1.0f,  1.0f,  0.5f); // market wick color
              Vec4f volumeBullColor(0.2f, 1.0f,  0.2f,  0.6f); // volume bar color on bullish days
              Vec4f volumeBearColor(1.0f, 0.15f, 0.15f, 0.8f); // volume bar color on bearish days

              DateTime dtStart = mStartDate; dtStart.setDay(dtStart.day()+startIndex); dtStart.fix();
              DateTime dtEnd   = mStartDate; dtEnd.setDay(dtEnd.day()+endIndex);       dtEnd.fix();
              std::string startDate = marketDateStr(dtStart);
              std::string endDate   = marketDateStr(dtEnd);
              // // DateTime dtEnd   = mStartDate; dtEnd.setDay(dtEnd.day()+endIndex);       dtEnd.fix();
              // int marketStart = std::floor(dtStart.diffDays(mMarketStartDate)) - 1;
              // int marketEnd   = marketStart + (endIndex - startIndex); //mMarketData->size();//std::ceil(dtEnd.diffDays(mMarketStartDate)) + 1;
              // int diff        = marketEnd - marketStart;
              // marketStart     = std::max(0, std::min(marketStart, (int)mMarketData->size()-1));
              // //int marketEnd   = marketStart + (endIndex - startIndex) + 1;
              // marketEnd       = std::max(0, std::min(marketEnd,   (int)mMarketData->size()));
              // if(marketStart >= 0 && marketStart < mMarketData->size() && marketEnd > 0 && marketEnd <= mMarketData->size())
              {
          
                // int lastIndex = marketStart;
                // while(lastIndex > 0 && mMarketData[lastIndex].volume == 0) { lastIndex--; }
                double daySize = plotToScreenVecAx(1.0, &mDateAxis);
            
                // for(int j = marketStart; j < marketEnd; j++)
                DateTime dtLast = dtStart; dtLast.setDay(dtLast.day()-1); dtLast.fix();
                DateTime dt     = dtStart;

                double dateOffset = dtStart.diffDays(mMarketStartDate);
                
                int dtIndex     = std::max(0, (int)(startIndex));//-std::floor(dateOffset)));
                while(dt < dtEnd)
                  {
                    std::string dtStr = marketDateStr(dt);
                    const auto &iter = mMarketData->find(dtStr);
                    if(iter != mMarketData->end())
                      {
                        const MarketPoint &d = iter->second; //mMarketData[j];
                        int dateIndex = dtIndex;  //marketStart-j;
                        if(mMarketAxis.visible)
                          {
                            Vec2f  po = plotToScreenPos(Vec2d(dateIndex+0.1f, d.open),  &mMarketAxis)+Vec2f(1,0);
                            Vec2f  pc = plotToScreenPos(Vec2d(dateIndex+0.9f, d.close), &mMarketAxis);
                            Vec2f  ph = plotToScreenPos(Vec2d(dateIndex+0.5f, d.high),  &mMarketAxis);
                            Vec2f  pl = plotToScreenPos(Vec2d(dateIndex+0.5f, d.low),   &mMarketAxis);
                            double dT = std::max(d.open, d.close); // top of candle stick
                            double dB = std::min(d.open, d.close); // bottom of candle stick
                      
                            // draw wick
                            //double wickSizeUpper = plotToScreenVecAx(d.high-dT, &mMarketAxis);
                            //if(wickSizeUpper > 0.5) // don't draw wick if < 0.5 pixels
                            //double wickSizeLower = plotToScreenVecAx(dB-d.low,  &mMarketAxis);
                            { graphDrawList->AddLine(ph, plotToScreenPos(Vec2d(dateIndex+0.5, dT), &mMarketAxis), ImColor(marketWickColor)); }
                            //if(wickSizeLower > 0.5) // don't draw wick if < 0.5 pixels
                            { graphDrawList->AddLine(plotToScreenPos(Vec2d(dateIndex+0.5, dB), &mMarketAxis), pl, ImColor(marketWickColor)); }
                      
                            // draw candle
                            if(daySize >= 1.5 || d.close == d.open)
                              { // draw rects
                                if(d.close != d.open || (d.open != 0.0 && d.close != 0.0))
                                  {
                                    if(d.close > d.open)      { graphDrawList->AddRectFilled(po, pc, ImColor(marketBullColor)); } // bull candle
                                    else if(d.close < d.open) { graphDrawList->AddRectFilled(po, pc, ImColor(marketBearColor)); } // bear candle
                                    else                      { graphDrawList->AddLine(po, pc, ImColor(marketFlatColor)); } // flat candle (no intraday data)
                                  }
                                else
                                  { // flat candle
                                    graphDrawList->AddLine(po, pc, ImColor(marketFlatColor));
                                  }
                              }
                            else
                              { // draw vertical line instead of rect
                                po = plotToScreenPos(Vec2d(dateIndex+0.5f, d.open),  &mMarketAxis);
                                pc = plotToScreenPos(Vec2d(dateIndex+0.5f, d.close), &mMarketAxis);
                                if(d.close != d.open || (d.open != 0.0 && d.close != 0.0))
                                  {
                                    if(d.close > d.open)      { graphDrawList->AddLine(po, pc, ImColor(marketBullColor)); } // bull candle
                                    else if(d.close < d.open) { graphDrawList->AddLine(po, pc, ImColor(marketBearColor)); } // bear candle
                                    // else                      { graphDrawList->AddLine(po, pc, ImColor(marketFlatColor)); } // flat candle (no intraday data)
                                  }
                              }
                          }
                        if(mVolumeAxis.visible)
                          { // plot bar data
                            Vec2f lineOffset = Vec2f(0.5f, mVolumeAxis.range.y*0.1f);
                            PlotDataParams *params = mVolumeParams;

                            std::string dtLastStr = marketDateStr(dtLast);
                            const auto &iterLast  = mMarketData->find(dtLastStr);
                            MarketPoint dLast;
                            bool haveLast = false;
                            if(iterLast != mMarketData->end())
                              { dLast = iterLast->second; haveLast = true; }

                            Vec2d pp0; Vec2d spp0;
                            if(haveLast) { pp0 = Vec2d(dateIndex, dLast.volume); spp0 = plotToScreenPos(pp0, &mVolumeAxis); }
                        
                            Vec2d pp1  = Vec2d(dateIndex+1, d.volume);
                            Vec2d spp1 = plotToScreenPos(pp1, &mVolumeAxis);
                            Vec4f col  = (d.close >= d.open) ? volumeBullColor : volumeBearColor;
                            float wid  = params->width/2.0f;
                  
                            // draw volume bar
                            Rect2d sr(screenToPlotPos(mScreenRect.p1, &mVolumeAxis), screenToPlotPos(mScreenRect.p2, &mVolumeAxis));
                            if(d.volume != 0 && haveLast) { graphDrawList->AddRectFilled(spp0, plotToScreenPos(Vec2f(pp1.x, 0), &mVolumeAxis), ImColor(col)); }
                            else              { pp1.y = dLast.volume; }
                      
                            // draw line
                            if(drawVolumeLine && haveLast)
                              {
                                col = Vec4f(0.5f, 0.5f, 0.5f, 1.0f);
                                graphDrawList->AddLine(plotToScreenPos(pp0+lineOffset, &mVolumeAxis),
                                                       plotToScreenPos(pp1+lineOffset, &mVolumeAxis), ImColor(col), 1.0f);
                              }
                            // if(d.volume != 0) { lastIndex = j; }
                          }
                      }
                    dtLast = dt;
                    dt.setDay(dt.day()+1); dt.fix();
                    dtIndex++;
                  }
              }
            }
        }
      ImGui::EndChild();
      if(!blocked) { handleIO(); }
   }
  ImGui::EndChild();
  return newSize;
}



void PlotWidget::setMarketData(const StockData *marketData, bool updateView)
{
  mMarketData = marketData;
  
  if(mMarketData)
    {
      std::cout << "--> PlotWidget set with " << mMarketData->size() << " market points.\n";
      mMarketStartDate = minDate(*mMarketData);
      mMarketEndDate   = maxDate(*mMarketData);
      
      std::cout << "    --> Start date: " << mMarketStartDate << "\n";
      std::cout << "    --> End date:   " << mMarketEndDate   << "\n";
    }

  if(mMarketData->size() > 0)
    {
      mMarketAxis.visible = true;
      // mVolumeAxis.visible = true;
      mMarketAxis.dataMin = DBL_MAX; mMarketAxis.dataMax = 0.0;
      mVolumeAxis.dataMin = DBL_MAX; mVolumeAxis.dataMax = 0.0;
      for(auto iter : *mMarketData)
        {
          const MarketPoint &d = iter.second;
          mMarketAxis.dataMax = std::max(mMarketAxis.dataMax, d.high);
          mVolumeAxis.dataMax = std::max(mVolumeAxis.dataMax, d.volume);
          if(d.low != 0)
            {
              mMarketAxis.dataMin = std::min(mMarketAxis.dataMin, d.low);
              mVolumeAxis.dataMin = std::min(mVolumeAxis.dataMin, d.volume);
            }
          // else if(i > 0) // no data (weekend/holiday) -- copy previous day's close data
          //   {
          //     MarketPoint &dLast = mMarketData[i-1];
          //     d.open   = dLast.close;
          //     d.close  = dLast.close;
          //     d.high   = dLast.close;
          //     d.low    = dLast.close;
          //     d.volume = 0;
          //   }
        }
      mMarketAxis.range.x = mMarketAxis.dataMin - (3.0*mPadRatio.y)*(mMarketAxis.dataMax-mMarketAxis.dataMin);
      mMarketAxis.range.y = mMarketAxis.dataMax + mPadRatio.y*(mMarketAxis.dataMax-mMarketAxis.dataMin);
      // volume bar chart stays at bottom of plot (TODO: make more flexible)
      mVolumeAxis.range.x = 0;//volumeMin - mPadRatio.y*(volumeMax-volumeMin);
      mVolumeAxis.range.y = 4.0*mVolumeAxis.dataMax + mPadRatio.y*(mVolumeAxis.dataMax-mVolumeAxis.dataMin);

      mVolumeAxis.labelMin      = 0.0;
      mVolumeAxis.labelMax      = mVolumeAxis.dataMax*1.5f;
      mVolumeAxis.labelInterval = mVolumeAxis.dataMax / 10.0;
      if(mVolumeAxis.logScale)
        {
          mVolumeAxis.labelMin      = mVolumeAxis.labelMin <= 0 ? 1.0 : log(mVolumeAxis.labelMin);
          mVolumeAxis.labelMax      = mVolumeAxis.labelMax <= 0 ? 1.0 : log(mVolumeAxis.labelMax);
          mVolumeAxis.labelInterval = mVolumeAxis.labelInterval <= 0 ? 1.0 : log(mVolumeAxis.labelInterval);
        }
      mMarketAxis.labelMin      = 0.0;
      mMarketAxis.labelMax      = 0.0;
      if(mMarketAxis.labelInterval <= 0) { mMarketAxis.labelInterval = mMarketAxis.dataMax / 10.0; }
      if(mMarketAxis.logScale)
        {
          mMarketAxis.labelMin      = mMarketAxis.labelMin <= 0 ? 1.0 : log(mMarketAxis.labelMin);
          mMarketAxis.labelMax      = mMarketAxis.labelMax <= 0 ? 1.0 : log(mMarketAxis.labelMax);
          mMarketAxis.labelInterval = mMarketAxis.labelInterval <= 0 ? 1.0 : log(mMarketAxis.labelInterval);
        }
    }
  else
    {
      // mMarketAxis.visible = false;
      // mVolumeAxis.visible = false;
      mMarketAxis.dataMin = 0.0; mMarketAxis.dataMax = 0.0;
      mVolumeAxis.dataMin = 0.0; mVolumeAxis.dataMax = 0.0;
    }
  if(updateView) { resetView(); }
}
