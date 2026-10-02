#ifndef SIMPLE_PLOT_HPP
#define SIMPLE_PLOT_HPP


#include <vector>
#include <imgui.h>
#include <imgui_internal.h>
#include "vector.hpp"
#include "range.hpp"
#include "colors.hpp"

#define TITLE_SPACING 5.0f
#define AUTOSCALE_PADDING 0.05 // ratio of total data range

static const std::vector<std::string> LINE_STYLES = { "solid", "dotted", "dashed", "dashdot" };

template<typename XT, typename YT>
struct PlotPoint { XT x = XT(); YT y = YT(); };
  
template<typename XT, typename YT>
struct PlotData
{
  std::string label = "";
  std::string style = "solid";
  Vec4f       color = COLOR("white");
  float       width = 1.0f;
  const std::vector<PlotPoint<XT, YT>> *data;
};

template<typename XT, typename YT>
struct Plot
{
  std::string title;
  std::vector<PlotData<XT, YT>> data;
  Vec4f bgColor = COLOR("grey30"); // Vec4f(0.03f, 0.03f, 0.03f, 1.0f);
  float width   = 2.0f;
    
  Plot(const std::string &title_="") : title(title_) { }
    
  void setTitle(const std::string &title_)  { title = title_; }          // set plot title
  void setBgColor(const std::string &color) { bgColor = COLOR(color); } // set background color
  void setBgColor(const Vec4f &color)       { bgColor = color; }
    
  void addData(const std::vector<PlotPoint<XT, YT>> *points,
               const Vec4f &color=COLOR("white"), float width=1.0f,
               const std::string &style="", const std::string &label="")
  {
    if(points)
      { data.push_back(PlotData<XT, YT>{label, style, color, width, points}); }
  }
    
  void addData(const PlotData<XT, YT> &d)
  {
    if(d)
      {
        auto &iter = std::find(data.begin(), data.end(), d);
        if(iter != data.end()) { data.push_back(d); }
      }
  }

  // remove dataset by pointer
  void removeData(const PlotData<XT, YT> *d)
  {
    if(d)
      {
        auto &iter = std::find(data.begin(), data.end(), d);
        if(iter != data.end()) { data.erase(iter); }
      }
  }
  // remove dataset by label
  bool removeData(const std::string &label)
  {
    if(!label.empty())
      {
        for(int i = 0; i < data.size(); i++)
          { if(data[i].label == label) { data.erase(data.begin()+1); return true; } }
      }
    return false;
  }

  Vec2f plotToScreen(const PlotPoint<XT, YT> &p, const Range<XT> &vRangeX, const Range<YT> &vRangeY, const Vec2f &screenPos, const Vec2f &screenSize)
  { return screenPos + screenSize*Vec2f((p.x-vRangeX.lower)/(float)vRangeX.span(), (vRangeY.upper-p.y)/(float)vRangeY.span()); }
    
  void draw(float scale, Range<XT> &vRangeX, Range<YT> &vRangeY, Vec2f screenPos, Vec2f screenSize, bool autoScaleX=false, bool autoScaleY=false)
  {
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    screenSize *= scale;

    // draw title
    Vec2f titleSize = ImGui::CalcTextSize(title.c_str());
    ImGui::SetCursorScreenPos(screenPos + Vec2f((screenSize.x-titleSize.x)/2.0f, TITLE_SPACING));
    ImGui::TextUnformatted(title.c_str());
    Vec2f titleSpace = Vec2f(0.0f, ImGui::GetItemRectMax().y-ImGui::GetItemRectMin().y + 2.0f*TITLE_SPACING);
    screenPos += titleSpace; screenSize -= titleSpace;
    // draw background
    ImGui::PushClipRect(screenPos, screenPos+screenSize, true); // clip to plot area
    drawList->AddRectFilled(screenPos, screenPos+screenSize, ImColor(bgColor), width);

    Range<XT> dRangeX; Range<YT> dRangeY;
    for(const auto &d : data) { for(const auto &p : *d.data) { dRangeX.fit(p.x); dRangeY.fit(p.y); } }
    XT dSpanX = dRangeX.span(); YT dSpanY = dRangeY.span(); // total span of data (max-min)

    // autoscale -- adjust view to cover data range
    if(autoScaleX) { vRangeX = dRangeX.extended(AUTOSCALE_PADDING*dSpanX); }
    if(autoScaleY) { vRangeY = dRangeY.extended(AUTOSCALE_PADDING*dSpanY); }
      
    for(const auto &d : data)
      {
        for(int i = 1; i < d.data->size(); i++) //const auto &p : *d.data)
          {
            const PlotPoint<XT, YT> &p0 = (*d.data)[i-1];
            const PlotPoint<XT, YT> &p1 = (*d.data)[i];
            Vec2f sp0 = plotToScreen(p0, vRangeX, vRangeY, screenPos, screenSize);
            Vec2f sp1 = plotToScreen(p1, vRangeX, vRangeY, screenPos, screenSize);

            drawList->AddLine(sp0, sp1, ImColor(d.color), width);

            // indicate clipping
            bool clipLow0  = (sp0.y >= screenPos.y + screenSize.y); bool clipHigh0 = (sp0.y <= screenPos.y);
            bool clipLow1  = (sp1.y >= screenPos.y + screenSize.y); bool clipHigh1 = (sp1.y <= screenPos.y);
            Vec2f clipL1; Vec2f clipH1; Vec2f clipL0; Vec2f clipH0; // screen points at which data clips
            if(sp0.y >= screenPos.y + screenSize.y)  { clipL0 = Vec2f(sp0.x, screenPos.y + screenSize.y); }
            if(sp0.y <= screenPos.y)                 { clipH0 = Vec2f(sp0.x, screenPos.y); }
            if(sp1.y >= screenPos.y + screenSize.y)  { clipL1 = Vec2f(sp1.x, screenPos.y + screenSize.y); }
            if(sp1.y <= screenPos.y)                 { clipH1 = Vec2f(sp1.x, screenPos.y); }
          
            if(clipLow0)  { drawList->AddLine(clipL0, clipL0 - Vec2f(0, 5.0f), ImColor(Vec4f(1.0f, 0.0f, 0.0f, 1.0f)), d.width*2.0f); }
            if(clipHigh0) { drawList->AddLine(clipH0, clipH0 + Vec2f(0, 5.0f), ImColor(Vec4f(1.0f, 0.0f, 0.0f, 1.0f)), d.width*2.0f); }
            if(clipLow1)  { drawList->AddLine(clipL1, clipL1 - Vec2f(0, 5.0f), ImColor(Vec4f(1.0f, 0.0f, 0.0f, 1.0f)), d.width*2.0f); }
            if(clipHigh1) { drawList->AddLine(clipH1, clipH1 + Vec2f(0, 5.0f), ImColor(Vec4f(1.0f, 0.0f, 0.0f, 1.0f)), d.width*2.0f); }
          }
      }

    ImGui::SetCursorScreenPos(screenPos);
    ImGui::InvisibleButton("##plot", screenSize, ImGuiButtonFlags_Disabled);
    ImGui::PopClipRect();
  }
};

#endif // SIMPLE_PLOT_HPP
